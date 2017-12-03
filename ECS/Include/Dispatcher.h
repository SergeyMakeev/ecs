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

#include <atomic>
#include <memory>
#include "Memory.h"
#include "EntityID.h"


#ifndef _UNUSED
#define _UNUSED(T) (void)(T)
#endif



namespace ecs
{
	//fwd decls
	void NotifyChanges(const ConstEntityId id);
	void DestroyEntity(EntityId id);
	void DestroyAll();

	namespace internal
	{
		void InitEntityDesc(EntityId id);
		void SetComponentBit(EntityId id, uint32_t componentTypeIndex);
		void ResetComponentBit(EntityId id, uint32_t componentTypeIndex);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class IdGenerator
	{
		ecs::vector<EntityId> unusedIds;
		std::atomic<uint32_t> reusedElementsCount;
		std::atomic<uint32_t> poolSize;
		std::atomic<uint32_t> firstUnusedId;
		std::atomic<bool> isLocked;

	public:

		IdGenerator()
			: reusedElementsCount(0)
			, poolSize(0)
			, firstUnusedId(0)
			, isLocked(false)
		{
			unusedIds.reserve(1024);
		}

		//
		// not thread safe!
		//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void release(EntityId id)
		{
			assert(isLocked.load() == false && "IdsPool is locked!");
			unusedIds.push_back(id);
		}

		//
		// not thread safe!
		//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void clear()
		{
			assert(isLocked.load() == false && "IdsPool is locked!");
			unusedIds.clear();
			firstUnusedId = 0;
		}

		//
		// not thread safe!
		//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void lock()
		{
			assert(isLocked.load() == false && "IdsPool is already locked!");
			isLocked.store(true);
			poolSize.store(narrow_cast<uint32_t>(unusedIds.size()));
			reusedElementsCount.store(0);
		}

		//
		// not thread safe!
		//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		void unlock()
		{
			assert(isLocked.load() == true && "IdsPool is not locked!");
			isLocked.store(false);

			assert(poolSize == unusedIds.size());

			//strip used ids
			uint32_t usedIdsCount = reusedElementsCount.load(std::memory_order_relaxed);
			if (!unusedIds.empty())
			{
				if (usedIdsCount > unusedIds.size())
					unusedIds.clear();
				else
					unusedIds.erase(unusedIds.end() - usedIdsCount, unusedIds.end());

			}
		}


		// thread safe!
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool IsLocked() const
		{
			return isLocked.load();
		}


		// thread safe!
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		EntityId acquire()
		{
			// thread-safe path for locked pool
			if (isLocked.load() == true)
			{
				uint32_t index = reusedElementsCount.fetch_add(1);
				uint32_t maxIndex = poolSize.load();
				if (index < maxIndex)
				{
					uint32_t reuseIndex = maxIndex - index - 1;
					EntityId id = unusedIds[reuseIndex];

					assert(id.u.generation != 0);
					id.u.generation++;

					// wrapping fixup
					if (id.u.generation == 0)
					{
						id.u.generation++;
					}
					return id;
				}

				EntityId newId;
				newId.u.generation = 1;
				newId.u.index = firstUnusedId.fetch_add(1);
				return newId;
			}

			// fast route (single thread)
			if (unusedIds.empty())
			{
				EntityId newId;
				newId.u.generation = 1;
				newId.u.index = firstUnusedId;
				firstUnusedId++;
				return newId;
			}

			EntityId id = unusedIds.back();
			assert(id.u.generation != 0);
			id.u.generation++;

			// wrapping fixup
			if (id.u.generation == 0)
			{
				id.u.generation++;
			}

			unusedIds.pop_back();
			return id;
		}
	};


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	class Dispatcher
	{
		enum Opcode
		{
			NOTIFY_CHANGES,
			CREATE_ENTITY,
			DESTROY_ENTITY,
			DESTROY_ALL,
			ADD_COMPONENT,
			REMOVE_COMPONENT,
		};


		struct Header
		{
			Opcode opcode;
			EntityId id;
		};


		struct SimpleCmd
		{
			Header head;
		};

		struct RemoveComponentCmd
		{
			Header header;
			uint32_t componentTypeIndex;
			IComponentsStorage* storage;
		};

		struct AddComponentBase
		{
			Header header;
			IComponentsStorage* storage;
			void(*destroyFunc)(void*);
			void* pComponent;
			uint32_t componentTypeIndex;
			uint32_t sizeOf;
			uint32_t alignOf;
			uint32_t commandSizeInBytes;
		};

		template<typename T>
		struct AddComponentCmd
		{
			AddComponentBase base;
			std::aligned_storage<sizeof(T), __alignof(T)> storage;

			static void CallDtor(void* p)
			{
				_UNUSED(p);
				T* _this = (T*)p;
				_UNUSED(_this);
				_this->~T();
			}
		};

		IdGenerator idGen;

		//to prevent false cache sharing
		static const uint32_t defaultAlignment = 128;

		struct BufferDeleter { void operator()(uint8_t* p) { memory::Free(p); } };
		std::unique_ptr<uint8_t, BufferDeleter> pBuffer;
		std::atomic<uint32_t> offset;
		size_t bufferSize;

		uint32_t Align(uint32_t val, uint32_t alignment)
		{
			return (val + (alignment - 1)) & ~(alignment - 1);
		}

		uint8_t* alloc(uint32_t bytesCount)
		{
			uint32_t blockSize = Align(bytesCount, defaultAlignment);

			uint32_t currentOffset = offset.fetch_add(blockSize);
			assert(currentOffset + blockSize < bufferSize && "Dispatcher: Out of memory!");
			uint32_t alignedOffset = Align(currentOffset, defaultAlignment);

			uint8_t* pMemory = pBuffer.get() + alignedOffset;
			return pMemory;
		}

		void PutSimpleCommand(ConstEntityId id, Opcode opcode)
		{
			SimpleCmd* cmd = (SimpleCmd*)alloc(sizeof(SimpleCmd));
			cmd->head.opcode = opcode;
			cmd->head.id = EntityId::internal::CreateFromConst(id);
		}

		void Execute()
		{
			assert(!IsLocked() == true && "Dispatcher is locked! Can't execute!");

			uint32_t maxOffset = offset.load(std::memory_order_relaxed);
			if (maxOffset == 0)
			{
				return;
			}

			uint32_t currentOffset = 0;
			uint8_t* buffer = pBuffer.get();

			while (currentOffset < maxOffset)
			{
				Header* head = (Header*)(buffer + currentOffset);
				switch (head->opcode)
				{
				case NOTIFY_CHANGES:
					{
						SimpleCmd* cmd = (SimpleCmd*)head;
						currentOffset += sizeof(SimpleCmd);
						ecs::NotifyChanges(cmd->head.id);
					}
					break;
				case CREATE_ENTITY:
					{
						SimpleCmd* cmd = (SimpleCmd*)head;
						currentOffset += sizeof(SimpleCmd);
						ecs::internal::InitEntityDesc(cmd->head.id);
					}
					break;
				case DESTROY_ENTITY:
					{
						SimpleCmd* cmd = (SimpleCmd*)head;
						currentOffset += sizeof(SimpleCmd);
						ecs::DestroyEntity(cmd->head.id);
					}
					break;
				case DESTROY_ALL:
					{
						//SimpleCmd* cmd = (SimpleCmd*)head;
						currentOffset += sizeof(SimpleCmd);
						ecs::DestroyAll();
					}
					break;
				case ADD_COMPONENT:
					{
						AddComponentBase* cmd = (AddComponentBase*)head;
						currentOffset += cmd->commandSizeInBytes;

						IComponentsStorage* storage = cmd->storage;

						storage->push_back_v(cmd->header.id, cmd->pComponent, cmd->sizeOf, cmd->alignOf);

						//call dtor
						cmd->destroyFunc(cmd->pComponent);

						ecs::internal::SetComponentBit(cmd->header.id, cmd->componentTypeIndex);
					}
					break;
				case REMOVE_COMPONENT:
					{
						RemoveComponentCmd* cmd = (RemoveComponentCmd*)head;
						currentOffset += sizeof(RemoveComponentCmd);
						IComponentsStorage* storage = cmd->storage;
						storage->erase_v(cmd->header.id);

						ecs::internal::ResetComponentBit(cmd->header.id, cmd->componentTypeIndex);
					}
					break;
				default:
					assert(false && "Unknown opcode");
				}

				currentOffset = Align(currentOffset, defaultAlignment);
			}

			offset.store(0);
		}


	public:

		Dispatcher(size_t bytesCount)
			: pBuffer( (uint8_t*)memory::Alloc(bytesCount, 128) )
			, offset(0)
			, bufferSize(bytesCount)
		{
			std::memset(pBuffer.get(), 0xFE, bytesCount);
		}

		~Dispatcher()
		{
		}

		IdGenerator& GetIdGenerator()
		{
			return idGen;
		}

		void lock()
		{
			assert(!IsLocked() == true && "Dispatcher is already locked!");
			idGen.lock();
		}

		void unlock()
		{
			assert(IsLocked() == true && "Dispatcher is not locked!");
			idGen.unlock();
			Execute();
		}

		bool IsLocked()
		{
			return idGen.IsLocked();
		}

		void Invoke_NotifyChanges(ConstEntityId id)
		{
			assert(IsLocked() == true && "Dispatcher is not locked!");
			PutSimpleCommand(id, NOTIFY_CHANGES);
		}

		void Invoke_DestroyEntity(EntityId id)
		{
			assert(IsLocked() == true && "Dispatcher is not locked!");
			PutSimpleCommand(id, DESTROY_ENTITY);
		}

		void Invoke_DestroyAll()
		{
			assert(IsLocked() == true && "Dispatcher is not locked!");
			PutSimpleCommand(ConstEntityId::Invalid(), DESTROY_ALL);
		}

		EntityId Invoke_CreateEntity()
		{
			assert(IsLocked() == true && "Dispatcher is not locked!");
			EntityId id = idGen.acquire();
			PutSimpleCommand(id, CREATE_ENTITY);
			return id;
		}

		template<typename T>
		void Invoke_AddComponent(EntityId id, T&& v0)
		{
			assert(IsLocked() == true && "Dispatcher is not locked!");

			ComponentsStorage<T>& storage = ecs::GetComponentStorage<std::remove_const<T>::type>();

			typedef AddComponentCmd<T> CommandType;

			CommandType* cmd = (CommandType*)alloc(sizeof(CommandType));
			cmd->base.header.opcode = ADD_COMPONENT;
			cmd->base.header.id = EntityId::internal::CreateFromConst(id);
			cmd->base.storage = &storage;
			cmd->base.sizeOf = sizeof(T);
			cmd->base.alignOf = __alignof(T);
			cmd->base.commandSizeInBytes = sizeof(CommandType);
			cmd->base.componentTypeIndex = ecs::GetComponentTypeIndex<std::remove_const<T>::type>();

			// store func (unique for each type T) to deffered call dtor
			cmd->base.destroyFunc = &CommandType::CallDtor;

			cmd->base.pComponent = (void *)::std::addressof(cmd->storage);
			
			// placement move ctor
			::new (cmd->base.pComponent) T(std::move(v0));
			
		}

		template<typename T>
		void Invoke_RemoveComponent(EntityId id)
		{
			assert(IsLocked() == true && "Dispatcher is not locked!");
			ComponentsStorage<T>& storage = ecs::GetComponentStorage<std::remove_const<T>::type>();

			RemoveComponentCmd* cmd = (RemoveComponentCmd*)alloc(sizeof(RemoveComponentCmd));
			cmd->header.opcode = REMOVE_COMPONENT;
			cmd->header.id = EntityId::internal::CreateFromConst(id);
			cmd->storage = &storage;
			cmd->componentTypeIndex = ecs::GetComponentTypeIndex<std::remove_const<T>::type>();
		}



	};

}



#undef _UNUSED
