// The MIT License (MIT)
//
// 	Copyright (c) 2017 Sergey Makeev
//
// 	Permission is hereby granted, free of charge, to any person obtaining a copy
// 	of this software and associated documentation files (the "Software"), to deal
// 	in the Software without restriction, including without limitation the rights
// 	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// 	copies of the Software, and to permit persons to whom the Software is
// 	furnished to do so, subject to the following conditions:
//
//      The above copyright notice and this permission notice shall be included in
// 	all copies or substantial portions of the Software.
//
// 	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// 	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// 	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// 	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// 	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// 	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// 	THE SOFTWARE.
#pragma once


#include <vector>
#include <array>
#include "Memory.h"
#include "EntityId.h"
#include "BitSet.h"



namespace ecs
{
	class IComponentsStorage;

	// forward decl
	std::array<IComponentsStorage*, bitset::MaxBitCount::value>& GetStorageDirectory();
	ecs::vector<IComponentsStorage*>& GetStorageLinearDirectory();


	typedef ecs::vector<uint32_t> ComponentsIterator;


	class IComponentsStorage
	{
	public:

		virtual ~IComponentsStorage() {}
		virtual void erase_v(const EntityId id) = 0;
		virtual void optimize_v() = 0;
		virtual void push_back_v(const EntityId id, void* pMem, size_t sizeOf, size_t alignOf) = 0;
	};






	template<typename T>
	class ComponentsStorage : public IComponentsStorage
	{
		ecs::vector<T> dataBuffer;

		// translate EntityId to component index or -1 if no component present for this entity.
		ecs::vector<int32_t> forwardIndex;

		// translate component index to EntityId
		ecs::vector<EntityId> backIndex;


	private:

		// non copyable
		ComponentsStorage(ComponentsStorage&);
		void operator=(ComponentsStorage&);


		T* get_element(const EntityId id)
		{
			assert(dataBuffer.size() == backIndex.size());

			// invalid entity index
			if (id.u.index >= forwardIndex.size())
			{
				return nullptr;
			}

			// convert EntityId to component index
			int32_t index = forwardIndex.at(id.u.index);

			//no component of this type for this EntityId
			if (index < 0)
			{
				return nullptr;
			}

			// return component pointer
			return &dataBuffer[index];
		}


		const T* get_element(const ConstEntityId id) const
		{
			assert(dataBuffer.size() == backIndex.size());

			// convert EntityId to component index
			int32_t index = forwardIndex.at(id.u.index);

			//no component of this type for this EntityId
			if (index < 0)
			{
				return nullptr;
			}

			// return component pointer
			return &dataBuffer[index];
		}


	public:

		ComponentsStorage()
		{
			// register storage
			uint32_t componentTypeIndex = ecs::GetComponentTypeIndex<T>();

			std::array<IComponentsStorage*, ecs::bitset::MaxBitCount::value>& storageDir = ecs::GetStorageDirectory();
			storageDir[componentTypeIndex] = this;

			ecs::vector<IComponentsStorage*>& linearStorageDir = ecs::GetStorageLinearDirectory();
			linearStorageDir.push_back(this);
		}


		void push_back(const EntityId id, T&& v)
		{
			assert(dataBuffer.size() == backIndex.size());

			if (id.u.index >= forwardIndex.size())
			{
				forwardIndex.resize(id.u.index + 1, -1);
			}

			uint32_t componentIndex = size();
			dataBuffer.push_back(std::move(v));

			backIndex.push_back(id);
			forwardIndex[id.u.index] = componentIndex;
		}


		uint32_t size() const
		{
			assert(dataBuffer.size() == backIndex.size());
			return narrow_cast<uint32_t>(dataBuffer.size());
		}

		bool empty() const
		{
			assert(dataBuffer.size() == backIndex.size());
			return dataBuffer.empty();
		}

		T* get(const EntityId id)
		{
			return get_element(id);
		}

		const T* get(const ConstEntityId id) const
		{
			return get_element(id);
		}

		void erase(const EntityId id)
		{
			assert(dataBuffer.size() == backIndex.size());
			if (id.u.index >= forwardIndex.size())
			{
				return;
			}

			// convert EntityId to component index
			uint32_t index = forwardIndex.at(id.u.index);
			assert(index < dataBuffer.size());

			uint32_t lastIndex = size() - 1;

			// Remove an element without preserving order of components.
			// If the element is not the last element transfer the last element into its position
			if (index != lastIndex)
			{
				//
				backIndex[index] = std::move(backIndex[lastIndex] );
				dataBuffer[index] = std::move(dataBuffer[lastIndex] );

				// update forward index for moved component
				EntityId movedEntityId = backIndex[index];
				forwardIndex[movedEntityId.u.index] = index;
			}

			// destroy component
			backIndex.pop_back();
			dataBuffer.pop_back();
			forwardIndex[id.u.index] = -1;
		}


		//
		// Optimize storage data layout
		//    The order of the components corresponds to the order of entities.
		//    This will improve the use of the CPU cache while iterating through components array.
		//
		//  Worst/Best/Average-case performance is O(n)
		//    where is n is the number of components
		//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void optimize()
		{
			//TODO: create and check for fragmentation flag
			//      second consecutive call (OptimizeLayoutForCache) should not do anything
			//
			// set framgentation flags if entity or component created/destroyed
			//    and reset this flag afrer defragmentation

			uint32_t tgtComponentIndex = 0;
			uint32_t maxEntityIndex = narrow_cast<uint32_t>(forwardIndex.size());
			for (uint32_t tgtEntityIndex = 0; tgtEntityIndex < maxEntityIndex; tgtEntityIndex++)
			{
				// Get the index of the component currently used for the Entity[tgtEntityIndex]
				int32_t srcComponentIndex = forwardIndex[tgtEntityIndex];

				// If component index is invalid, Entity[tgtEntityIndex] does not contain component of such type, ignoring...
				if (srcComponentIndex < 0)
				{
					continue;
				}

				// Sanity check
				assert(backIndex[srcComponentIndex].u.index == tgtEntityIndex);

				// Query entity index that is using our component slot.
				uint32_t srcEntityIndex = backIndex[tgtComponentIndex].u.index;

				// No need to optimize (component already stored in right place)
				if (tgtEntityIndex == srcEntityIndex)
				{
					tgtComponentIndex++;
					continue;
				}

				// Swap components data
				std::swap(dataBuffer[srcComponentIndex], dataBuffer[tgtComponentIndex]);

				// Update forward and back index
				std::swap(forwardIndex[srcEntityIndex], forwardIndex[tgtEntityIndex]);
				std::swap(backIndex[tgtComponentIndex], backIndex[srcComponentIndex]);

				//
				tgtComponentIndex++;
			}
		}


		virtual void erase_v(const EntityId id) override
		{
			erase(id);
		}

		virtual void optimize_v() override
		{
			optimize();
		}

		virtual void push_back_v(const EntityId id, void* pMem, size_t sizeOf, size_t alignOf)
		{
			assert(sizeOf == sizeof(T));
			assert(alignOf == __alignof(T));
			T& value = *(T*)pMem;
			push_back(id, std::move(value));
		}






	};



}


