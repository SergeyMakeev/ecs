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
#include <algorithm>
#include "TestComponents.h"






SUITE(DataOrientedDesignTests)
{


TEST(BasicSort)
{
	std::vector<int> data;

	// generate partialy sorted sequence
	int val = 512;
	for (int i = 0; i < 100000; i++)
	{
		data.push_back(val);
		int delta = (rand() % 128) - 16;
		val += delta;
	}

	// check sort implementation (need only for custom sort algorithms)
	// TODO: move implementation to timsort
	std::sort(data.begin(), data.end());


	// check results
	int prev = -1;
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		int v = *it;

		if (prev >= 0)
		{
			CHECK(prev <= v);
		}
		prev = v;
	}
}

TEST(BasicSortStorage)
{
	ecs::DestroyAll();

	std::vector<EntityId> ids;

	const int count = 10000;
	for (int i = 0; i < count; i++)
	{
		EntityId id = ecs::CreateEntity(DummyComponent(float(i), -float(i)));
		ids.push_back(id);
	}
	
	for (auto it = ids.rbegin(); it != ids.rend(); ++it)
	{
		EntityId id = *it;
		CHECK(ecs::IsValid(id));
	}

	
	ecs::ComponentsStorage<DummyComponent>& storage = ecs::GetComponentStorage<DummyComponent>();
	storage.optimize();

	for (auto it = ids.begin(); it != ids.end(); ++it)
	{
		ecs::DestroyEntity(*it);
	}

	CHECK(ecs::GetComponentStorage<DummyComponent>().empty());
}

TEST(CacheFriendly)
{
	ecs::DestroyAll();

	// Create
	EntityId id1 = ecs::CreateEntity();
	EntityId id2 = ecs::CreateEntity();
	EntityId id3 = ecs::CreateEntity();
	EntityId id4 = ecs::CreateEntity();
	EntityId id5 = ecs::CreateEntity();
	EntityId id6 = ecs::CreateEntity();
	EntityId id7 = ecs::CreateEntity();

	// Add components in reverse order
	ecs::AddComponent(id7, DummyComponent(7.0f, -7.0f));
	ecs::AddComponent(id6, DummyComponent(6.0f, -6.0f));
	ecs::AddComponent(id5, DummyComponent(5.0f, -5.0f));
	ecs::AddComponent(id4, DummyComponent(4.0f, -4.0f));
	ecs::AddComponent(id3, DummyComponent(3.0f, -3.0f));
	ecs::AddComponent(id2, DummyComponent(2.0f, -2.0f));
	ecs::AddComponent(id1, DummyComponent(1.0f, -1.0f));


	ecs::AddComponent(id5, DummyComponent2(5));
	ecs::AddComponent(id1, DummyComponent2(1));
	ecs::AddComponent(id3, DummyComponent2(3));

	

	for (int i = 0; i < 3; i++)
	{
		DummyComponent* pComp1 = ecs::GetComponent<DummyComponent>(id1);
		DummyComponent* pComp2 = ecs::GetComponent<DummyComponent>(id2);
		DummyComponent* pComp3 = ecs::GetComponent<DummyComponent>(id3);
		DummyComponent* pComp4 = ecs::GetComponent<DummyComponent>(id4);
		DummyComponent* pComp5 = ecs::GetComponent<DummyComponent>(id5);
		DummyComponent* pComp6 = ecs::GetComponent<DummyComponent>(id6);
		DummyComponent* pComp7 = ecs::GetComponent<DummyComponent>(id7);

		CHECK(pComp1 != nullptr);
		CHECK(pComp2 != nullptr);
		CHECK(pComp3 != nullptr);
		CHECK(pComp4 != nullptr);
		CHECK(pComp5 != nullptr);
		CHECK(pComp6 != nullptr);
		CHECK(pComp7 != nullptr);

		CHECK_CLOSE(pComp1->x, 1.0f, 0.0001f);
		CHECK_CLOSE(pComp1->y, -1.0f, 0.0001f);

		CHECK_CLOSE(pComp2->x, 2.0f, 0.0001f);
		CHECK_CLOSE(pComp2->y, -2.0f, 0.0001f);

		CHECK_CLOSE(pComp3->x, 3.0f, 0.0001f);
		CHECK_CLOSE(pComp3->y, -3.0f, 0.0001f);

		CHECK_CLOSE(pComp4->x, 4.0f, 0.0001f);
		CHECK_CLOSE(pComp4->y, -4.0f, 0.0001f);

		CHECK_CLOSE(pComp5->x, 5.0f, 0.0001f);
		CHECK_CLOSE(pComp5->y, -5.0f, 0.0001f);

		CHECK_CLOSE(pComp6->x, 6.0f, 0.0001f);
		CHECK_CLOSE(pComp6->y, -6.0f, 0.0001f);

		CHECK_CLOSE(pComp7->x, 7.0f, 0.0001f);
		CHECK_CLOSE(pComp7->y, -7.0f, 0.0001f);

		DummyComponent2* piComp1 = ecs::GetComponent<DummyComponent2>(id1);
		DummyComponent2* piComp2 = ecs::GetComponent<DummyComponent2>(id2);
		DummyComponent2* piComp3 = ecs::GetComponent<DummyComponent2>(id3);
		DummyComponent2* piComp4 = ecs::GetComponent<DummyComponent2>(id4);
		DummyComponent2* piComp5 = ecs::GetComponent<DummyComponent2>(id5);
		DummyComponent2* piComp6 = ecs::GetComponent<DummyComponent2>(id6);
		DummyComponent2* piComp7 = ecs::GetComponent<DummyComponent2>(id7);

		CHECK(piComp1 != nullptr);
		CHECK(piComp2 == nullptr);
		CHECK(piComp3 != nullptr);
		CHECK(piComp4 == nullptr);
		CHECK(piComp5 != nullptr);
		CHECK(piComp6 == nullptr);
		CHECK(piComp7 == nullptr);

		CHECK(piComp1->v == 1);
		CHECK(piComp3->v == 3);
		CHECK(piComp5->v == 5);


		if (i == 0)
		{
			CHECK(pComp7 < pComp6);
			CHECK(pComp6 < pComp5);
			CHECK(pComp5 < pComp4);
			CHECK(pComp4 < pComp3);
			CHECK(pComp3 < pComp2);
			CHECK(pComp2 < pComp1);

			CHECK(piComp5 < piComp1);
			CHECK(piComp1 < piComp3);
		} else if (i >= 1)
		{
			CHECK(pComp1 < pComp2);
			CHECK(pComp2 < pComp3);
			CHECK(pComp3 < pComp4);
			CHECK(pComp4 < pComp5);
			CHECK(pComp5 < pComp6);
			CHECK(pComp6 < pComp7);

			CHECK(piComp1 < piComp3);
			CHECK(piComp3 < piComp5);

		}


		ecs::OptimizeLayoutForCache();
	}

}




}

