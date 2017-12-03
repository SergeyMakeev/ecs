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
#include <stdint.h>
#include <array>
#include <vector>
#include <BitSet.h>
#include <Entity.h>
#include <Process.h>


class IComponentsStorage;

namespace ecs
{

	namespace memory
	{
		/////////////////////////////////////////////////////////////////////////////////
		void* Alloc(size_t bytesCount, size_t align)
		{
			return _mm_malloc(bytesCount, align);
		}

		/////////////////////////////////////////////////////////////////////////////////
		void Free(void* p)
		{
			_mm_free(p);
		}
	}



	/////////////////////////////////////////////////////////////////////////////////
	uint32_t& GlobalIdCounter()
	{
		static uint32_t globalCounter = 0;
		return globalCounter;
	}

	/////////////////////////////////////////////////////////////////////////////////
	std::array<IComponentsStorage*, bitset::MaxBitCount::value>& GetStorageDirectory()
	{
		static std::array<IComponentsStorage*, bitset::MaxBitCount::value> storageDir;
		return storageDir;
	}

	/////////////////////////////////////////////////////////////////////////////////
	ecs::vector<IComponentsStorage*>& GetStorageLinearDirectory()
	{
		static ecs::vector<IComponentsStorage*> storageLinearDir;
		return storageLinearDir;
	}


	static const size_t dispatcherBufferSize = 4 * 1024 * 1024;

	/////////////////////////////////////////////////////////////////////////////////
	internal::Context::Context()
		: dispatcher(dispatcherBufferSize)
	{
		state = ContextState::MUTABLE;

		needRebuildOrderedList = false;

		// make initial memory reservation
		const size_t initialEntitiesCount = 1024;

		entitiesDesc.reserve(initialEntitiesCount);
		entitiesMasks.reserve(initialEntitiesCount);

		unorderedUsedEntitiesIds.reserve(initialEntitiesCount);

		changedEntitiesIds.reserve(initialEntitiesCount * 4);

		processList.reserve(128);
	}

	/////////////////////////////////////////////////////////////////////////////////
	internal::Context& internal::GetContext()
	{
		static internal::Context context;
		return context;
	}

	/////////////////////////////////////////////////////////////////////////////////
	void RegisterProcess(IProcessBase* pProcess)
	{
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);

		internal::ProcessList& newProcessList = internal::GetContext().newProcessList;
		newProcessList.push_back(pProcess);
	}

	/////////////////////////////////////////////////////////////////////////////////
	void UnregisterProcess(IProcessBase* pProcess)
	{
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);

		internal::ProcessList& processList = internal::GetContext().processList;
		processList.erase(std::remove(processList.begin(), processList.end(), pProcess), processList.end());

		internal::ProcessList& newProcessList = internal::GetContext().newProcessList;
		newProcessList.erase(std::remove(newProcessList.begin(), newProcessList.end(), pProcess), newProcessList.end());
	}

	/////////////////////////////////////////////////////////////////////////////////
	void Update(float deltaTime)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		
		dispatcher.lock();
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		internal::GetContext().state = internal::ContextState::REMAP;

		internal::ProcessList& processList = internal::GetContext().processList;

		uint32_t maxEntityIndex = narrow_cast<uint32_t>(internal::GetContext().entitiesDesc.size());

		// remap updated entities
		{
			ConstEntityList& changedEntitiesIds = internal::GetContext().changedEntitiesIds;
			if (!changedEntitiesIds.empty())
			{
				for (auto it = processList.begin(); it != processList.end(); ++it)
				{
					IProcessBase* pProcess = *it;
					pProcess->ReMap(changedEntitiesIds, maxEntityIndex);
				}
				changedEntitiesIds.clear();
			}
		}


		// call initial ReMap for new processes
		internal::ProcessList& newProcessList = internal::GetContext().newProcessList;
		if (!newProcessList.empty())
		{
			const ConstEntityList& entitiesList = ecs::GetActiveListConst();

			for (auto it = newProcessList.begin(); it != newProcessList.end(); ++it)
			{
				IProcessBase* pProcess = *it;
				pProcess->ReMap(entitiesList, maxEntityIndex);
				processList.push_back(pProcess);
			}
			newProcessList.clear();
		}

		assert(internal::GetContext().state == internal::ContextState::REMAP);
		internal::GetContext().state = internal::ContextState::UPDATE;


		// update all registred processes
		{
			for (auto it = processList.begin(); it != processList.end(); ++it)
			{
				IProcessBase* pProcess = *it;
				pProcess->Update(deltaTime);
			}
		}

		assert(internal::GetContext().state == internal::ContextState::UPDATE);
		internal::GetContext().state = internal::ContextState::MUTABLE;
		dispatcher.unlock();
	}


	/////////////////////////////////////////////////////////////////////////////////
	void FoldAndReorder(const RemapList& input, EntityList& output, BucketsList& buckets)
	{
		std::array<uint32_t, 255> histogramm;
		std::memset(&histogramm[0], 0, histogramm.size() * sizeof(uint32_t));

		// build histogramm
		uint32_t elementsCount = narrow_cast<uint32_t>(input.size());
		for (uint32_t i = 0; i < elementsCount; i++)
		{
			MapKey key = input[i].key;
			if (key == ecs::UnusedMapKey)
			{
				continue;
			}
			assert(i == input[i].id.u.index);
			histogramm[key]++;
		}


		buckets.clear();

		// convert histogramm to starting offsets
		std::array<uint32_t, 255> offsets;
		uint32_t currentOffset = 0;

		uint32_t histogramSize = narrow_cast<uint32_t>(histogramm.size());
		for (uint32_t i = 0; i < histogramSize; i++)
		{
			offsets[i] = currentOffset;

			uint32_t prevOffset = currentOffset;
			currentOffset += histogramm[i];

			if (histogramm[i] > 0)
			{
				buckets.emplace_back(Bucket(prevOffset, currentOffset - 1));
			}
		}

		output.clear();
		output.resize(currentOffset);

		// build ordered results
		for (uint32_t i = 0; i < elementsCount; i++)
		{
			const MapTuple& tuple = input[i];
			if (tuple.key == ecs::UnusedMapKey)
			{
				continue;
			}

			uint32_t& writeIndex = offsets[tuple.key];
			output[writeIndex] = EntityId::internal::CreateFromConst(tuple.id);
			writeIndex++;
		}
	}






}


