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
//  The above copyright notice and this permission notice shall be included in
// 	all copies or substantial portions of the Software.
//
// 	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// 	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// 	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// 	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// 	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// 	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// 	THE SOFTWARE.

#include <UnitTest++.h>
#include <ECS.h>
#include "TestComponents.h"


SUITE(ProcessTests)
{

	
	class TestProcess : public ecs::Process< ecs::Aspect<Pos, const Velocity> >
	{
		ecs::RemapList remap;
		ecs::bitset aspectMask;
		ecs::EntityList workingSet;
		ecs::BucketsList buckets;

	public:

		TestProcess()
		{
			ecs::bitset tmp;
			TAspect::GenerateMask(aspectMask, tmp);
		}

		virtual void ReMap(const ecs::ConstEntityList& entities, uint32_t maxEntityIndex) override
		{
			//printf("TestProcess::ReMap()\n");

			remap.resize(maxEntityIndex, ecs::MapTuple::Invalid());

			for (auto it = entities.cbegin(); it != entities.cend(); ++it)
			{
				const ConstEntityId id = *it;
				remap[it->u.index] = ecs::IsMatchAspect(id, aspectMask) ? ecs::MapTuple::Create(0, id) : ecs::MapTuple::Invalid();
			}

			ecs::FoldAndReorder(remap, workingSet, buckets);
		}


		virtual void Update(float deltaTime) override
		{
			//printf("TestProcess::Update()\n");

			auto enumerator = ecs::CreateEnumerator<TAspect>(workingSet);
			for (auto it = enumerator.begin(); it != enumerator.end(); ++it)
			{
				TAspect entityAspect = *it;
				const ConstEntityId id = entityAspect.id;
				Pos* pos = entityAspect.c0;
				const Velocity* vel = entityAspect.c1;

				//printf("ID: gen(%d), idx(%d)\n", (uint32_t)id.u.generation, (uint32_t)id.u.index);
				//printf("  pos : %f, %f\n", pos->x, pos->y);
				//printf("  vel : %f, %f\n", vel->x, vel->y);

				//mutate state
				pos->x += vel->x * deltaTime;
				pos->y += vel->y * deltaTime;

			} // loop
		}

	};

	class TestProcess2 : public ecs::Process< ecs::Aspect<Pos> >
	{
		ecs::RemapList remap;
		ecs::bitset aspectMask;
		ecs::EntityList workingSet;
		ecs::BucketsList buckets;

	public:

		TestProcess2()
		{
			ecs::bitset tmp;
			TAspect::GenerateMask(aspectMask, tmp);
		}

		virtual void ReMap(const ecs::ConstEntityList& entities, uint32_t maxEntityIndex) override
		{
			//printf("TestProcess2::ReMap()\n");

			remap.resize(maxEntityIndex, ecs::MapTuple::Invalid());

			for (auto it = entities.cbegin(); it != entities.cend(); ++it)
			{
				const ConstEntityId id = *it;
				remap[it->u.index] = ecs::IsMatchAspect(id, aspectMask) ? ecs::MapTuple::Create(0, id) : ecs::MapTuple::Invalid();
			}

			ecs::FoldAndReorder(remap, workingSet, buckets);
		}

		virtual void Update(float /*deltaTime*/) override
		{
			//printf("TestProcess2::Update()\n");

			auto enumerator = ecs::CreateEnumerator<TAspect::Const>(workingSet);
			for (auto it = enumerator.begin(); it != enumerator.end(); ++it)
			{
				TAspect::Const entityAspect = *it;
				const ConstEntityId id = entityAspect.id;
				const Pos* pos = entityAspect.c0;

				float v[2];
				v[0] = pos->x;
				v[1] = pos->y;
				//printf("ID: gen(%d), idx(%d)\n", (uint32_t)id.u.generation, (uint32_t)id.u.index);
				//printf("  pos : %f, %f\n", pos->x, pos->y);
			}
		}

	};





////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(SimpleProcess)
{
	const float deltaTime = 0.016666f;

	ecs::DestroyAll();
	ecs::Update(deltaTime);

	EntityId id1 = ecs::CreateEntity(Pos(1.0f, 2.0f), Velocity(1.0f, 0.0f));
	EntityId id2 = ecs::CreateEntity(Pos(3.0f, 4.0f));
	EntityId id3 = ecs::CreateEntity(Pos(5.0f, 6.0f), Velocity(-1.0f, 0.0f));

	const ecs::ConstEntityList& list = ecs::GetActiveListConst();
	CHECK(list.size() == 3);

	TestProcess mmm;

	for (int frame = 0; frame < 16; frame++)
	{
		//printf("+++ [ frame %d ] +++++++++++++++++++++++++++++\n", frame);
		ecs::Update(deltaTime);

		if (frame == 10)
		{
			ecs::AddComponent(id2, Velocity(1.0f, 0.0f));
		} else if (frame == 14)
		{
			ecs::RemoveComponent<Velocity>(id2);
		}
	}


	//printf("+++ [ done ] +++++++++++++++++++++++++++++\n");

	TestProcess2 nnnn;
	ecs::Update(deltaTime);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(FoldAndReorder)
{
	ecs::DestroyAll();
	ecs::Update(1.0f);

	std::vector<ecs::MapTuple> referenceValues;

	ecs::RemapList remap;
	for (int32_t i = 256; i >= 0; i--)
	{
		for (uint32_t n = 0; n < 100; n++)
		{
			ecs::MapKey key = ecs::narrow_cast<ecs::MapKey>(i);
			EntityId id = ecs::CreateEntity();
			ecs::MapTuple tuple = ecs::MapTuple::Create(key, id);
			remap.push_back(tuple);

			if (key != ecs::UnusedMapKey)
			{
				referenceValues.push_back(tuple);
			}
		}
	}

	ecs::BucketsList buckets;
	ecs::EntityList results;
	ecs::FoldAndReorder(remap, results, buckets);
	CHECK(buckets.size() == 255);

	std::stable_sort(referenceValues.begin(), referenceValues.end(), [](const ecs::MapTuple &a, const ecs::MapTuple &b)
		{
			return a.key < b.key;
		});

	CHECK(results.size() == referenceValues.size());

	for (size_t j = 0; j < referenceValues.size(); j++)
	{
		CHECK(results[j] == referenceValues[j].id);
	}

}


class TestProcess3 : public ecs::Process< ecs::Aspect<Timer> >
{
	ecs::RemapList remap;
	ecs::bitset aspectMask;
	ecs::EntityList workingSet;
	ecs::BucketsList buckets;

public:

	TestProcess3()
	{
		ecs::bitset tmp;
		TAspect::GenerateMask(aspectMask, tmp);
	}

	virtual void ReMap(const ecs::ConstEntityList& entities, uint32_t maxEntityIndex) override
	{
		remap.resize(maxEntityIndex, ecs::MapTuple::Invalid());

		for (auto it = entities.cbegin(); it != entities.cend(); ++it)
		{
			const ConstEntityId id = *it;
			remap[it->u.index] = ecs::IsMatchAspect(id, aspectMask) ? ecs::MapTuple::Create(0, id) : ecs::MapTuple::Invalid();
		}

		ecs::FoldAndReorder(remap, workingSet, buckets);
	}

	virtual void Update(float /*deltaTime*/) override
	{
		auto enumerator = ecs::CreateEnumerator<TAspect>(workingSet);
		for (auto it = enumerator.begin(); it != enumerator.end(); ++it)
		{
			TAspect entityAspect = *it;
			const EntityId id = entityAspect.id;
			Timer* timer = entityAspect.c0;

			timer->time -= 1;

			// add component if need
			if (timer->time < 2 && ecs::GetComponent<Dummy>(id) == nullptr)
			{
				ecs::AddComponent(id, Dummy(timer->time));
			}

			if (timer->time <= 0)
			{
				// destroy entity 
				ecs::DestroyEntity(id);

				//create two new entities
				ecs::CreateEntity(Timer(5));
				ecs::CreateEntity(Timer(5));
			}
		}
	}

};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(MutateGlobalStateDuringUpdate)
{
	ecs::DestroyAll();
	ecs::Update(1.0f);

	TestProcess3 process;

	std::vector<EntityId> ids;
	for (uint32_t i = 0; i < 100; i++)
	{
		EntityId id = ecs::CreateEntity(Timer(4));
		ids.push_back(id);
	}

	CHECK(ecs::GetActiveList().size() == 100);

	//destroy some entities before update
	ecs::DestroyEntity(ids[20]);
	ecs::DestroyEntity(ids[30]);
	ecs::DestroyEntity(ids[33]);
	ecs::DestroyEntity(ids[39]);
	ecs::DestroyEntity(ids[89]);

	CHECK(ecs::GetActiveList().size() == 95);
	

	//simulate
	const float deltaTime = 0.016666f;
	for (int frame = 0; frame < 3; frame++)
	{
		ecs::Update(deltaTime);
	}

	{
		const ecs::EntityList& aliveEntities = ecs::GetActiveList();
		CHECK(aliveEntities.size() == 95);

		auto entities = ecs::CreateEnumerator<TestProcess3::TAspect>(aliveEntities);
		for (TestProcess3::TAspect view : entities)
		{
			// all entities with life time less than 2, must have Dummy component
			Dummy* pDummy = ecs::GetComponent<Dummy>(view.id);
			if (view.c0->time < 2)
			{
				CHECK(pDummy != nullptr);
				CHECK(pDummy->val == 1);
			}
			else
			{
				CHECK(pDummy == nullptr);
			}
		}
	}

	//simulate
	for (int frame = 0; frame < 5; frame++)
	{
		ecs::Update(deltaTime);
	}

	{
		const ecs::EntityList& aliveEntities = ecs::GetActiveList();
		CHECK(aliveEntities.size() == 190);

		auto entities = ecs::CreateEnumerator<TestProcess3::TAspect>(aliveEntities);
		for (TestProcess3::TAspect view : entities)
		{
			// all entities with life time less than 2, must have Dummy component
			Dummy* pDummy = ecs::GetComponent<Dummy>(view.id);
			if (view.c0->time < 2)
			{
				CHECK(pDummy != nullptr);
				CHECK(pDummy->val == 1);
			}
			else
			{
				CHECK(pDummy == nullptr);
			}
		}

	}

	//simulate
	for (int frame = 0; frame < 3; frame++)
	{
		ecs::Update(deltaTime);
	}


	{
		const ecs::EntityList& aliveEntities = ecs::GetActiveList();
		CHECK(aliveEntities.size() == 380);

		auto entities = ecs::CreateEnumerator<TestProcess3::TAspect>(aliveEntities);
		for (TestProcess3::TAspect view : entities)
		{
			// all entities with life time less than 2, must have Dummy component
			Dummy* pDummy = ecs::GetComponent<Dummy>(view.id);
			if (view.c0->time < 2)
			{
				CHECK(pDummy != nullptr);
				CHECK(pDummy->val == 1);
			}
			else
			{
				CHECK(pDummy == nullptr);
			}
		}

	}
}


class OrderedProcess : public ecs::Process< ecs::Aspect<Pos, Dummy> >
{
	ecs::RemapList remap;
	ecs::bitset aspectMask;
	ecs::EntityList workingSet;
	ecs::BucketsList buckets;

	int currentFrame;

public:

	OrderedProcess()
	{
		currentFrame = 1;
		ecs::bitset tmp;
		TAspect::GenerateMask(aspectMask, tmp);
	}

	int Map(const ConstEntityId id)
	{
		// can't map invalid id
		assert(ecs::IsValid(id));

		// get parent component
		const ParentComponent* parent = ecs::GetComponent<ParentComponent>(id);
		if (parent == nullptr)
		{
			return 1;
		}

		// check if parent is valid
		if (!ecs::IsValid(parent->id))
		{
			return 1;
		}

		//
		return Map(parent->id) + 1;
	}

	virtual void ReMap(const ecs::ConstEntityList& entities, uint32_t maxEntityIndex) override
	{
		remap.resize(maxEntityIndex, ecs::MapTuple::Invalid());

		// Can use parallel_for for this loop
		for (auto it = entities.cbegin(); it != entities.cend(); ++it)
		{
			const ConstEntityId id = *it;
			if (ecs::IsMatchAspect(id, aspectMask))
			{
				int key = Map(id);
				assert(key >= ecs::MinMapKey && key <= ecs::MaxMapKey);
				remap[it->u.index] = ecs::MapTuple::Create(ecs::narrow_cast<ecs::MapKey>(key), id);
			}
			else
			{
				remap[it->u.index] = ecs::MapTuple::Invalid();
			}
		}

		ecs::FoldAndReorder(remap, workingSet, buckets);
	}

	virtual void Update(float /*deltaTime*/) override
	{
		int bucketIndex = 0;

		// buckets must be updated in order
		for (const ecs::Bucket& bucket : buckets)
		{
			// but we can run a parallel_for for each bucket
			auto enumerator = ecs::CreateEnumerator<TAspect>(workingSet, &bucket);
			for (auto it = enumerator.begin(); it != enumerator.end(); ++it)
			{
				TAspect entityAspect = *it;
				const EntityId id = entityAspect.id;
				Pos* pos = entityAspect.c0;
				Dummy* dummy = entityAspect.c1;
				//printf("bucket: %d, gen(%d), idx(%d)\n", bucketIndex, (uint32_t)id.u.generation, (uint32_t)id.u.index);

				const ParentComponent* parent = ecs::GetComponent<ParentComponent>(id);
				if (parent)
				{
					const Dummy* parentDummy = ecs::GetComponent<Dummy>(parent->id);
					// parent entity must be already updated in this frame
					CHECK(parentDummy->val == currentFrame);

					// current entity must be update in previous frame
					CHECK(dummy->val == (currentFrame-1));

					// parent position is already updated in this frame, so we can use the parent pos
					const Pos* parentPos = ecs::GetComponent<Pos>(parent->id);
					pos->x = parentPos->x * 1.5f;
				}
				else
				{
					pos->x += 1.0f;
				}

				// update current entity frame
				dummy->val = currentFrame;

			}

			bucketIndex++;
		}

		currentFrame++;
	}



};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(OrderedUpdateInsideProcess)
{
	ecs::DestroyAll();
	ecs::Update(1.0f);

	OrderedProcess process;

	for (int i = 0; i < 3; i++)
	{
		EntityId root = ecs::CreateEntity(Pos(0.0f, 0.0f), Dummy(0));
		EntityId child1 = ecs::CreateEntity(Pos(1.0f, 0.0f), Dummy(0), ParentComponent(root));
		EntityId child2 = ecs::CreateEntity(Pos(-1.0f, 0.0f), Dummy(0), ParentComponent(root));
		EntityId grandchild1 = ecs::CreateEntity(Pos(2.0f, 0.0f), Dummy(0), ParentComponent(child1));
		EntityId grandchild2 = ecs::CreateEntity(Pos(-2.0f, 0.0f), Dummy(0), ParentComponent(child2));
	}

	const float deltaTime = 0.016666f;
	for (int frame = 0; frame < 16; frame++)
	{
		ecs::Update(deltaTime);
	}

}


}
