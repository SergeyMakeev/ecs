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


#include <stdint.h>

#include "Component.h"
#include "EntityId.h"
#include "ComponentsStorage.h"
#include "BitSet.h"
#include "Utils.h"
#include "Memory.h"
#include "Dispatcher.h"


#define ecs_force_inline __forceinline



namespace ecs
{


	// forward decl
	uint32_t& GlobalIdCounter();
	struct IProcessBase;

	//list of constant entites id
	typedef ecs::vector<ConstEntityId> ConstEntityList;

	//list of entites id
	typedef ecs::vector<EntityId> EntityList;

	/////////////////////////////////////////////////////////////////////////////////
	struct Bucket
	{
		uint32_t from;
		uint32_t to;

		Bucket(uint32_t _from, uint32_t _to)
			: from(_from)
			, to(_to)
		{
		}
	};

	// list of buckets
	typedef ecs::vector<Bucket> BucketsList;


	//
	// Entity enumerator
	//
	/////////////////////////////////////////////////////////////////////////////////
	template<typename TAspect, typename TEntityList>
	class TEntityEnumerator
	{
	protected:
		const TEntityList& entityList;
		uint32_t firstValidElementIndex;
		uint32_t firstInvalidElementIndex;

	public:

		/////////////////////////////////////////////////////////////////////////////////
		struct iterator
		{
			const TEntityList& entityList;
			int32_t index;

			iterator(const TEntityList& list, int32_t _index)
				: entityList(list)
				, index(_index)
			{
			}

			ecs_force_inline bool operator != (const iterator& other) const
			{
				if (&entityList != &other.entityList)
					return true;

				if (index != other.index)
					return true;

				return false;
			}

			ecs_force_inline iterator& operator++ ()
			{
				index++;
				return *this;
			}

			ecs_force_inline typename TAspect operator* () const
			{
				const auto& id = entityList[index];
				return TAspect::Create(id);
			}
		};

		TEntityEnumerator(const TEntityList& list, const Bucket* bucket)
			: entityList(list)
		{
			if (bucket)
			{
				firstValidElementIndex = bucket->from;
				firstInvalidElementIndex = bucket->to + 1;
			} else
			{
				firstValidElementIndex = 0;
				firstInvalidElementIndex = narrow_cast<uint32_t>(list.size());
			}

			assert(firstInvalidElementIndex >= firstValidElementIndex);
		}

		inline iterator begin()
		{
			return iterator(entityList, firstValidElementIndex);
		}

		inline iterator end()
		{
			return iterator(entityList, firstInvalidElementIndex);
		}
	};


	//
	// internal
	//
	////////////////////////////////////////////////////////////////////////////////////
	namespace internal
	{

		namespace ContextState
		{
			enum Type
			{
				MUTABLE = 0,
				REMAP = 1,
				UPDATE = 2,
			};
		}

		class EntityDesc
		{
		public:

			// id
			EntityId id;

			// index inside usedEntitiesIds
			uint32_t usedIndex;

			EntityDesc(EntityId _id, uint32_t _usedIndex)
				: id(_id)
				, usedIndex(_usedIndex)
			{
			}
		};
		static_assert(sizeof(EntityDesc) <= 64, "sizeof(EntityDesc) > 64");

		typedef ecs::vector<EntityDesc> EntityStorage;
		typedef ecs::vector<bitset> EntityMaskStorage;
		typedef ecs::vector<IProcessBase*> ProcessList;

		struct Context
		{
			ContextState::Type state;
			Dispatcher dispatcher;
			EntityStorage entitiesDesc;
			EntityMaskStorage entitiesMasks;
			EntityList unorderedUsedEntitiesIds;
			ConstEntityList changedEntitiesIds;
			ProcessList processList;
			ProcessList newProcessList;

			EntityList orderedUsedEntitiesIds;
			bool needRebuildOrderedList;

			Context();

			inline void NeedRebuildOrderedList()
			{
				needRebuildOrderedList = true;
			}

			inline bool IsOrderedListNeedRebuild()
			{
				return needRebuildOrderedList;
			}

			inline void BuildOrderedListIfNeed()
			{
				if (needRebuildOrderedList == false)
					return;

				orderedUsedEntitiesIds = unorderedUsedEntitiesIds;
				needRebuildOrderedList = false;

				//TODO: replace to radix sort (index is only 20 bits)
				std::sort(orderedUsedEntitiesIds.begin(), orderedUsedEntitiesIds.end(), [](const EntityId &a, const EntityId &b)
				{
					return a.u.index < b.u.index;
				});
			}
		};

		Context& GetContext();

		////////////////////////////////////////////////////////////////////////////////////
		template<bool NEED_UPDATE_REVERSE_INDEX>
		inline void Destroy(uint32_t index)
		{
			assert(internal::GetContext().state == internal::ContextState::MUTABLE);

			internal::EntityStorage& entitiesDesc = internal::GetContext().entitiesDesc;
			internal::EntityMaskStorage& entitiesMasks = internal::GetContext().entitiesMasks;

			// Copy actual entity Id
			EntityId id = entitiesDesc[index].id;

			// Invalidate index
			entitiesDesc[index].id.Invalidate();

			// Destroy from usedEntitiesIds (reverse index to O(1) mapping between EntityId -> usedEntitiesIds[])
			if (NEED_UPDATE_REVERSE_INDEX)
			{
				EntityList& unorderedUsedEntitiesIds = internal::GetContext().unorderedUsedEntitiesIds;

				uint32_t usedIndex = entitiesDesc[index].usedIndex;
				assert(!unorderedUsedEntitiesIds.empty());
				uint32_t lastUsedIndex = narrow_cast<uint32_t>(unorderedUsedEntitiesIds.size()) - 1;

				// Remove an element without preserving order of components.
				// If the element is not the last element transfer the last element into its position
				if (usedIndex != lastUsedIndex)
				{
					EntityId lastEntityId = unorderedUsedEntitiesIds[lastUsedIndex];
					unorderedUsedEntitiesIds[usedIndex] = lastEntityId;

					// update reverse index (usedIndex) to affected entity
					entitiesDesc[lastEntityId.u.index].usedIndex = usedIndex;
				}
				unorderedUsedEntitiesIds.pop_back();

				internal::GetContext().NeedRebuildOrderedList();
			}

			// Destroy all entity components
			std::array<IComponentsStorage*, bitset::MaxBitCount::value>& storageDir = GetStorageDirectory();

			const bitset& componentsMask = entitiesMasks[index];
			for (auto it = componentsMask.begin(); it != componentsMask.end(); ++it)
			{
				uint32_t componentTypeIndex = *it;
				assert(componentTypeIndex < bitset::MaxBitCount::value && "Invalid component type index.");
				IComponentsStorage* storage = storageDir[componentTypeIndex];
				assert(storage);
				storage->erase_v(id);
			}

			// Call dtor's
			entitiesDesc[index].~EntityDesc();
			entitiesMasks[index].~bitset();
		}

		////////////////////////////////////////////////////////////////////////////////////
		inline void InitEntityDesc(EntityId id)
		{
			assert(internal::GetContext().state == internal::ContextState::MUTABLE);

			EntityList& unorderedUsedEntitiesIds = internal::GetContext().unorderedUsedEntitiesIds;
			internal::EntityStorage& entitiesDesc = internal::GetContext().entitiesDesc;
			internal::EntityMaskStorage& entitiesMasks = internal::GetContext().entitiesMasks;

			// first free entity index
			uint32_t maxEntityIndex = narrow_cast<uint32_t>(entitiesDesc.size());

			// number of used entities id's
			uint32_t maxUsedEntitiesIds = narrow_cast<uint32_t>(unorderedUsedEntitiesIds.size());

			// sanity check
			assert(entitiesDesc.size() == entitiesMasks.size());

			if (id.u.index < maxEntityIndex)
			{
				// index was reused

				//update entity data
				entitiesDesc[id.u.index] = internal::EntityDesc(id, maxUsedEntitiesIds);
				entitiesMasks[id.u.index].clear();

				unorderedUsedEntitiesIds.push_back(id);
				internal::GetContext().NeedRebuildOrderedList();
				return;
			}

			// index was created
			
			// entities id's index must be incremented in order
			assert(id.u.index == maxEntityIndex && "IdGenerator and entitiesDesc is out of sync!");

			//create new entity data
			entitiesDesc.push_back(internal::EntityDesc(id, maxUsedEntitiesIds));
			entitiesMasks.push_back(bitset());

			unorderedUsedEntitiesIds.push_back(id);

			if (!internal::GetContext().IsOrderedListNeedRebuild())
			{
				// No need to refresh ordered list, since we addiding the entity id with highest index to the end of list (ordering is preserved)
				EntityList& orderedUsedEntitiesIds = internal::GetContext().orderedUsedEntitiesIds;
				orderedUsedEntitiesIds.push_back(id);
			}
		}


		////////////////////////////////////////////////////////////////////////////////////
		inline EntityId CreateEntity()
		{
			assert(internal::GetContext().state == internal::ContextState::MUTABLE);

			IdGenerator& idGen = internal::GetContext().dispatcher.GetIdGenerator();
			EntityId id = idGen.acquire();

			InitEntityDesc(id);

			return id;
		}

		////////////////////////////////////////////////////////////////////////////////////
		inline void SetComponentBit(EntityId id, uint32_t componentTypeIndex)
		{
			assert(componentTypeIndex < bitset::MaxBitCount::value && "Invalid component type index.");
			internal::EntityMaskStorage& entitiesMasks = internal::GetContext().entitiesMasks;
			bitset& componentsMask = entitiesMasks[id.u.index];
			assert(!componentsMask.get(componentTypeIndex) && "Component of this type already present in this entity.");
			componentsMask.set(componentTypeIndex);
		}

		////////////////////////////////////////////////////////////////////////////////////
		inline void ResetComponentBit(EntityId id, uint32_t componentTypeIndex)
		{
			assert(componentTypeIndex < bitset::MaxBitCount::value && "Invalid component type index.");
			internal::EntityMaskStorage& entitiesMasks = internal::GetContext().entitiesMasks;
			bitset& componentsMask = entitiesMasks[id.u.index];
			assert(componentsMask.get(componentTypeIndex) && "Component of this type is not present in this entity.");
			componentsMask.reset(componentTypeIndex);
		}



		////////////////////////////////////////////////////////////////////////////////////
		template<typename T>
		inline void AddComponent(EntityId id, T&& defaultValue)
		{
			assert(internal::GetContext().state == internal::ContextState::MUTABLE);

			uint32_t componentTypeIndex = ecs::GetComponentTypeIndex<std::remove_const<T>::type>();
			internal::SetComponentBit(id, componentTypeIndex);

			ComponentsStorage<T>& storage = ecs::GetComponentStorage<std::remove_const<T>::type>();
			storage.push_back(id, std::move(defaultValue));
		}

		////////////////////////////////////////////////////////////////////////////////////
		template<typename T>
		inline void RemoveComponent(EntityId id)
		{
			assert(internal::GetContext().state == internal::ContextState::MUTABLE);

			uint32_t componentTypeIndex = ecs::GetComponentTypeIndex<std::remove_const<T>::type>();
			internal::ResetComponentBit(id, componentTypeIndex);

			ComponentsStorage<T>& storage = ecs::GetComponentStorage<std::remove_const<T>::type>();
			storage.erase(id);
			
		}

	} // namespace internal


	//
	// Check for valid EntityId or not
	//
	////////////////////////////////////////////////////////////////////////////////////
	inline bool IsValid(const ConstEntityId id)
	{
		internal::EntityStorage& entitiesDesc = internal::GetContext().entitiesDesc;

		//check for valid index
		if (id.u.index >= entitiesDesc.size())
		{
			return false;
		}

		EntityId& entId = entitiesDesc[id.u.index].id;

		// check for valid generation
		if (entId.u.generation != id.u.generation)
		{
			return false;
		}

		return true;
	}

	//
	// Notify of Changes (These notifications will be received for all the processes using ReMap() method.)
	//
	////////////////////////////////////////////////////////////////////////////////////
	inline void NotifyChanges(const ConstEntityId id)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_NotifyChanges(id);
			return;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);

		ConstEntityList& changedEntitiesIds = internal::GetContext().changedEntitiesIds;
		changedEntitiesIds.push_back(id);
	}


	//
	// Destroy entity
	//
	////////////////////////////////////////////////////////////////////////////////////
	inline void DestroyEntity(EntityId id)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_DestroyEntity(id);
			return;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		assert(IsValid(id) && "Invalid entity ID");

		IdGenerator& idGen = internal::GetContext().dispatcher.GetIdGenerator();
		internal::Destroy<true>(id.u.index);
		idGen.release(id);
		NotifyChanges(id);
	}

	//
	// Destroy all enteties and clear internal caches (restart id's)
	//
	////////////////////////////////////////////////////////////////////////////////////
	inline void DestroyAll()
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_DestroyAll();
			return;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);

		internal::EntityStorage& entitiesDesc = internal::GetContext().entitiesDesc;
		internal::EntityMaskStorage& entitiesMasks = internal::GetContext().entitiesMasks;
		IdGenerator& idGen = internal::GetContext().dispatcher.GetIdGenerator();

		EntityList& unorderedUsedEntitiesIds = internal::GetContext().unorderedUsedEntitiesIds;
		EntityList& orderedUsedEntitiesIds = internal::GetContext().orderedUsedEntitiesIds;

		ConstEntityList& changedEntitiesIds = internal::GetContext().changedEntitiesIds;

		// if available using ordered list of entities (best use of CPU cache)
		bool orderedListIsValid = (internal::GetContext().IsOrderedListNeedRebuild() == false);
		EntityList& list = orderedListIsValid ? orderedUsedEntitiesIds : unorderedUsedEntitiesIds;

		if (orderedListIsValid)
		{
			assert(orderedUsedEntitiesIds.size() == unorderedUsedEntitiesIds.size());
		}

		// Destroy all entities
		for (size_t index = 0; index < list.size(); index++)
		{
			EntityId& id = list[index];
			internal::Destroy<false>(id.u.index);

			//massive changes notification
			changedEntitiesIds.push_back(id);
		}

		// destroy
		idGen.clear();
		unorderedUsedEntitiesIds.clear();
		orderedUsedEntitiesIds.clear();
		entitiesDesc.clear();
		entitiesMasks.clear();
	}

	//
	// Create new entity
	//
	////////////////////////////////////////////////////////////////////////////////////
	inline EntityId CreateEntity()
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			EntityId id = dispatcher.Invoke_CreateEntity();
			dispatcher.Invoke_NotifyChanges(id);
			return id;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);

		EntityId id = internal::CreateEntity();
		NotifyChanges(id);
		return id;
	}

	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0>
	inline EntityId CreateEntity(T0&& v0)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			EntityId id = dispatcher.Invoke_CreateEntity();
			dispatcher.Invoke_AddComponent<T0>(id, std::move(v0));
			dispatcher.Invoke_NotifyChanges(id);
			return id;
		}

		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		EntityId id = internal::CreateEntity();
		internal::AddComponent<T0>(id, std::move(v0));
		NotifyChanges(id);
		return id;
	}

	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0, typename T1>
	inline EntityId CreateEntity(T0&& v0, T1&& v1)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			EntityId id = dispatcher.Invoke_CreateEntity();
			dispatcher.Invoke_AddComponent<T0>(id, std::move(v0));
			dispatcher.Invoke_AddComponent<T1>(id, std::move(v1));
			dispatcher.Invoke_NotifyChanges(id);
			return id;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);

		EntityId id = internal::CreateEntity();
		internal::AddComponent<T0>(id, std::move(v0));
		internal::AddComponent<T1>(id, std::move(v1));
		NotifyChanges(id);
		return id;
	}

	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0, typename T1, typename T2>
	inline EntityId CreateEntity(T0&& v0, T1&& v1, T2&& v2)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			EntityId id = dispatcher.Invoke_CreateEntity();
			dispatcher.Invoke_AddComponent<T0>(id, std::move(v0));
			dispatcher.Invoke_AddComponent<T1>(id, std::move(v1));
			dispatcher.Invoke_AddComponent<T2>(id, std::move(v2));
			dispatcher.Invoke_NotifyChanges(id);
			return id;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);

		EntityId id = internal::CreateEntity();
		internal::AddComponent<T0>(id, std::move(v0));
		internal::AddComponent<T1>(id, std::move(v1));
		internal::AddComponent<T2>(id, std::move(v2));
		NotifyChanges(id);
		return id;
	}



	//
	// Get entity component (return nullptr if component of such type is not added to entity)
	//
	////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline T* GetComponent(EntityId id)
	{
		assert(IsValid(id) && "Invalid entity ID");

		typedef std::remove_const<T>::type TYPE;

		ComponentsStorage<TYPE>& storage = ecs::GetComponentStorage<TYPE>();
		T* v = storage.get(id);
		return v;
	}


	//
	// Get entity component (return nullptr if component of such type is not added to entity)
	//
	////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	inline const T* GetComponent(const ConstEntityId id)
	{
		assert(IsValid(id) && "Invalid entity ID");

		typedef std::remove_const<T>::type TYPE;

		ComponentsStorage<TYPE>& storage = ecs::GetComponentStorage<TYPE>();
		const T* v = storage.get(id);
		return v;
	}

	//
	// Add component to the entity
	//
	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0>
	inline void AddComponent(EntityId id, T0&& v0)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_AddComponent(id, std::move(v0));
			dispatcher.Invoke_NotifyChanges(id);
			return;
		}

		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		assert(IsValid(id) && "Invalid entity ID");

		internal::AddComponent<T0>(id, std::move(v0));

		NotifyChanges(id);
	}

	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0, typename T1>
	inline void AddComponents(EntityId id, T0&& v0, T1&& v1)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_AddComponent(id, std::move(v0));
			dispatcher.Invoke_AddComponent(id, std::move(v1));
			dispatcher.Invoke_NotifyChanges(id);
			return;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		assert(IsValid(id) && "Invalid entity ID");

		internal::AddComponent<T0>(id, std::move(v0));
		internal::AddComponent<T1>(id, std::move(v1));

		NotifyChanges(id);
	}


	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0, typename T1, typename T2>
	inline void AddComponents(EntityId id, T0&& v0, T1&& v1, T2&& v2)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_AddComponent(id, std::move(v0));
			dispatcher.Invoke_AddComponent(id, std::move(v1));
			dispatcher.Invoke_AddComponent(id, std::move(v2));
			dispatcher.Invoke_NotifyChanges(id);
			return;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		assert(IsValid(id) && "Invalid entity ID");

		internal::AddComponent<T0>(id, std::move(v0));
		internal::AddComponent<T1>(id, std::move(v1));
		internal::AddComponent<T2>(id, std::move(v2));

		NotifyChanges(id);
	}



	//
	// Remove component from entity
	//
	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0>
	inline void RemoveComponent(EntityId id)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_RemoveComponent<T0>(id);
			dispatcher.Invoke_NotifyChanges(id);
			return;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		assert(IsValid(id) && "Invalid entity ID");

		internal::RemoveComponent<T0>(id);
		NotifyChanges(id);
	}

	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0, typename T1>
	inline void RemoveComponents(EntityId id)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_RemoveComponent<T0>(id);
			dispatcher.Invoke_RemoveComponent<T1>(id);
			dispatcher.Invoke_NotifyChanges(id);
			return;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		assert(IsValid(id) && "Invalid entity ID");

		internal::RemoveComponent<T0>(id);
		internal::RemoveComponent<T1>(id);
		NotifyChanges(id);
	}

	////////////////////////////////////////////////////////////////////////////////////
	template<typename T0, typename T1, typename T2>
	inline void RemoveComponents(EntityId id)
	{
		Dispatcher& dispatcher = internal::GetContext().dispatcher;
		if (dispatcher.IsLocked())
		{
			dispatcher.Invoke_RemoveComponent<T0>(id);
			dispatcher.Invoke_RemoveComponent<T1>(id);
			dispatcher.Invoke_RemoveComponent<T2>(id);
			dispatcher.Invoke_NotifyChanges(id);
			return;
		}
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);
		assert(IsValid(id) && "Invalid entity ID");

		internal::RemoveComponent<T0>(id);
		internal::RemoveComponent<T1>(id);
		internal::RemoveComponent<T2>(id);
		NotifyChanges(id);
	}




	//
	// Optimize the location of all components
	//
	//  the order of the components corresponds to the order of entities,
	//  this improves the use of the CPU cache
	//
	//
	////////////////////////////////////////////////////////////////////////////////////
	inline void OptimizeLayoutForCache()
	{
		assert(internal::GetContext().state == internal::ContextState::MUTABLE);

		ecs::vector<IComponentsStorage*>& componentStorages = GetStorageLinearDirectory();
		for (auto it = componentStorages.begin(); it != componentStorages.end(); ++it)
		{
			IComponentsStorage* storage = *it;
			storage->optimize_v();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	void RegisterProcess(IProcessBase* pProcess);
	////////////////////////////////////////////////////////////////////////////////////
	void UnregisterProcess(IProcessBase* pProcess);
	////////////////////////////////////////////////////////////////////////////////////
	void Update(float deltaTime);

	////////////////////////////////////////////////////////////////////////////////////
	inline bool IsMatchAspect(const ConstEntityId id, const bitset& aspectMask)
	{
		if (!IsValid(id))
		{
			return false;
		}

		internal::EntityMaskStorage& entitiesMasks = internal::GetContext().entitiesMasks;
		return entitiesMasks[id.u.index].contains(aspectMask);
	}


	//
	// The EntityList is guaranteed to be ordered by the entity index.
	//   CPU cache friendly
	//
	////////////////////////////////////////////////////////////////////////////////////
	inline const EntityList& GetActiveList()
	{
		internal::GetContext().BuildOrderedListIfNeed();
		const EntityList& orderedUsedEntitiesIds = internal::GetContext().orderedUsedEntitiesIds;
		return orderedUsedEntitiesIds;
	}

	//
	// The ConstEntityList is guaranteed to be ordered by the entity index.
	//   CPU cache friendly
	//
	////////////////////////////////////////////////////////////////////////////////////
	inline const ConstEntityList& GetActiveListConst()
	{
		// some static checks for reinterpret cast
		static_assert(sizeof(ConstEntityId) == sizeof(EntityId), "ConstEntityId and EntityId must be equal size!");
		static_assert(std::is_pod<ConstEntityId>::value, "ConstEntityId must be pod type!");
		static_assert(std::is_pod<EntityId>::value, "EntityId must be pod type!");

		const EntityList& orderedUsedEntitiesIds = GetActiveList();

		// HACK! container reinterpret cast!
		return reinterpret_cast<const ConstEntityList&>(orderedUsedEntitiesIds);
	}


	////////////////////////////////////////////////////////////////////////////////////
	template<typename TAspect>
	inline TEntityEnumerator<TAspect, EntityList> CreateEnumerator(const EntityList& list, const Bucket* bucket = nullptr)
	{
		return TEntityEnumerator<TAspect, EntityList>(list, bucket);
	}

	////////////////////////////////////////////////////////////////////////////////////
	template<typename TAspect>
	inline TEntityEnumerator<TAspect, ConstEntityList> CreateEnumerator(const ConstEntityList& list, const Bucket* bucket = nullptr)
	{
		return TEntityEnumerator<TAspect, ConstEntityList>(list, bucket);
	}


	// fwd decl (implementation in macro ECS_IMPLEMENT_COMPONENT_META)
	////////////////////////////////////////////////////////////////////////////////////
	template<typename TComponent>
	ecs::ComponentsStorage<TComponent>& GetComponentStorage();

	// fwd decl (implementation in macro ECS_IMPLEMENT_COMPONENT_META)
	////////////////////////////////////////////////////////////////////////////////////
	template<typename TComponent>
	uint32_t GetComponentTypeIndex();




}