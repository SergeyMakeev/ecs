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



SUITE(SimpleTests)
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(BitSetSimple)
{
	ecs::bitset bset;

	CHECK(bset.get(1) == false);
	bset.set(1);
	CHECK(bset.get(1) == true);

	CHECK(bset.get(9) == false);
	bset.set(9);
	CHECK(bset.get(9) == true);

	CHECK(bset.get(10) == false);
	bset.set(10);
	CHECK(bset.get(10) == true);

	CHECK(bset.get(33) == false);
	bset.set(33);
	CHECK(bset.get(33) == true);

	CHECK(bset.get(38) == false);
	bset.set(38);
	CHECK(bset.get(38) == true);

	CHECK(bset.get(153) == false);
	bset.set(153);
	CHECK(bset.get(153) == true);

	CHECK(bset.get(300) == false);
	bset.set(300);
	CHECK(bset.get(300) == true);


	CHECK(bset.get(1) == true);
	CHECK(bset.get(9) == true);
	CHECK(bset.get(10) == true);
	CHECK(bset.get(33) == true);
	CHECK(bset.get(38) == true);
	CHECK(bset.get(153) == true);
	CHECK(bset.get(300) == true);

	bset.reset(1);
	CHECK(bset.get(1) == false);
	CHECK(bset.get(9) == true);
	CHECK(bset.get(10) == true);
	CHECK(bset.get(33) == true);
	CHECK(bset.get(38) == true);
	CHECK(bset.get(153) == true);
	CHECK(bset.get(300) == true);

	bset.reset(9);
	CHECK(bset.get(1) == false);
	CHECK(bset.get(9) == false);
	CHECK(bset.get(10) == true);
	CHECK(bset.get(33) == true);
	CHECK(bset.get(38) == true);
	CHECK(bset.get(153) == true);
	CHECK(bset.get(300) == true);

	bset.reset(10);
	CHECK(bset.get(1) == false);
	CHECK(bset.get(9) == false);
	CHECK(bset.get(10) == false);
	CHECK(bset.get(33) == true);
	CHECK(bset.get(38) == true);
	CHECK(bset.get(153) == true);
	CHECK(bset.get(300) == true);

	bset.reset(33);
	CHECK(bset.get(1) == false);
	CHECK(bset.get(9) == false);
	CHECK(bset.get(10) == false);
	CHECK(bset.get(33) == false);
	CHECK(bset.get(38) == true);
	CHECK(bset.get(153) == true);
	CHECK(bset.get(300) == true);

	bset.reset(38);
	CHECK(bset.get(1) == false);
	CHECK(bset.get(9) == false);
	CHECK(bset.get(10) == false);
	CHECK(bset.get(33) == false);
	CHECK(bset.get(38) == false);
	CHECK(bset.get(153) == true);
	CHECK(bset.get(300) == true);

	bset.reset(153);
	CHECK(bset.get(1) == false);
	CHECK(bset.get(9) == false);
	CHECK(bset.get(10) == false);
	CHECK(bset.get(33) == false);
	CHECK(bset.get(38) == false);
	CHECK(bset.get(153) == false);
	CHECK(bset.get(300) == true);

	bset.reset(300);
	CHECK(bset.get(1) == false);
	CHECK(bset.get(9) == false);
	CHECK(bset.get(10) == false);
	CHECK(bset.get(33) == false);
	CHECK(bset.get(38) == false);
	CHECK(bset.get(153) == false);
	CHECK(bset.get(300) == false);


	CHECK(bset.get(2) == false);
	bset.flip(2);
	CHECK(bset.get(2) == true);

	CHECK(bset.get(3) == false);
	bset.flip(3);
	CHECK(bset.get(3) == true);

	CHECK(bset.get(33) == false);
	bset.flip(33);
	CHECK(bset.get(33) == true);


	CHECK(bset.get(2) == true);
	bset.flip(2);
	CHECK(bset.get(2) == false);
	CHECK(bset.get(3) == true);
	CHECK(bset.get(33) == true);


	CHECK(bset.get(3) == true);
	bset.flip(3);
	CHECK(bset.get(2) == false);
	CHECK(bset.get(3) == false);
	CHECK(bset.get(33) == true);

	CHECK(bset.get(33) == true);
	bset.flip(33);
	CHECK(bset.get(2) == false);
	CHECK(bset.get(3) == false);
	CHECK(bset.get(33) == false);


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(BitSetIterator)
{
	ecs::bitset bset;

	CHECK(bset.get(0) == false);
	bset.set(0);
	CHECK(bset.get(0) == true);

	CHECK(bset.get(39) == false);
	bset.set(39);
	CHECK(bset.get(39) == true);

	CHECK(bset.get(45) == false);
	bset.set(45);
	CHECK(bset.get(45) == true);

	CHECK(bset.get(69) == false);
	bset.set(69);
	CHECK(bset.get(69) == true);

	CHECK(bset.get(70) == false);
	bset.set(70);
	CHECK(bset.get(70) == true);

	CHECK(bset.get(130) == false);
	bset.set(130);
	CHECK(bset.get(130) == true);

	CHECK(bset.get(300) == false);
	bset.set(300);
	CHECK(bset.get(300) == true);

	CHECK(bset.get(313) == false);
	bset.set(313);
	CHECK(bset.get(313) == true);

	CHECK(bset.get(383) == false);
	bset.set(383);
	CHECK(bset.get(383) == true);


	std::vector<uint32_t> enabledBits;

	for (auto it = bset.begin(); it != bset.end(); ++it)
	{
		uint32_t bitIndex = *it;
		enabledBits.push_back(bitIndex);
	}

	CHECK(enabledBits.size() == 9);

	CHECK(enabledBits[0] == 0);
	CHECK(enabledBits[1] == 39);
	CHECK(enabledBits[2] == 45);
	CHECK(enabledBits[3] == 69);
	CHECK(enabledBits[4] == 70);
	CHECK(enabledBits[5] == 130);
	CHECK(enabledBits[6] == 300);
	CHECK(enabledBits[7] == 313);
	CHECK(enabledBits[8] == 383);


	bset.clear();

	std::vector<uint32_t> randomSequence;
	for (int bitIndex = 0; bitIndex < ecs::bitset::MaxBitCount::value; bitIndex++)
	{
		if ((rand() % 1024) < 512)
		{
			randomSequence.push_back(bitIndex);
		}
	}

	for (auto it = randomSequence.begin(); it != randomSequence.end(); it++)
	{
		bset.set(*it);
	}


	enabledBits.clear();
	for (auto it = bset.begin(); it != bset.end(); ++it)
	{
		uint32_t bitIndex = *it;
		enabledBits.push_back(bitIndex);
	}


	CHECK(enabledBits.size() == randomSequence.size());

	for (size_t idx = 0; idx < randomSequence.size(); idx++)
	{
		CHECK(randomSequence[idx] == enabledBits[idx]);
	}

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(BitSetContains)
{
	ecs::bitset bset1;
	ecs::bitset bset2;

	bset1.set(2);
	CHECK(bset1.get(2) == true);
	bset1.set(3);
	CHECK(bset1.get(3) == true);
	bset1.set(7);
	CHECK(bset1.get(7) == true);
	bset1.set(65);
	CHECK(bset1.get(65) == true);
	bset1.set(93);
	CHECK(bset1.get(93) == true);
	bset1.set(120);
	CHECK(bset1.get(120) == true);
	bset1.set(121);
	CHECK(bset1.get(121) == true);
	bset1.set(223);
	CHECK(bset1.get(223) == true);
	bset1.set(224);
	CHECK(bset1.get(224) == true);
	bset1.set(319);
	CHECK(bset1.get(319) == true);
	bset1.set(320);
	CHECK(bset1.get(320) == true);

	bset2.set(3);
	CHECK(bset2.get(3) == true);
	bset2.set(65);
	CHECK(bset2.get(65) == true);
	bset2.set(120);
	CHECK(bset2.get(120) == true);
	bset2.set(223);
	CHECK(bset2.get(223) == true);
	bset2.set(319);
	CHECK(bset2.get(319) == true);

	bool res1 = bset1.contains(bset2);
	CHECK(res1 == true);

	bool res2 = bset2.contains(bset1);
	CHECK(res2 == false);

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(CreateAndDestroy)
{
	EntityId id1 = ecs::CreateEntity();
	CHECK(ecs::IsValid(id1));
	ecs::DestroyEntity(id1);
	CHECK(!ecs::IsValid(id1));

	EntityId id2 = ecs::CreateEntity();
	CHECK(id1 != id2);
	CHECK(!ecs::IsValid(id1));
	CHECK(ecs::IsValid(id2));
	ecs::DestroyEntity(id2);

	CHECK(!ecs::IsValid(id1));
	CHECK(!ecs::IsValid(id2));

	id1 = ecs::CreateEntity();
	id2 = ecs::CreateEntity();

	CHECK(ecs::IsValid(id1));
	CHECK(ecs::IsValid(id2));

	ecs::DestroyEntity(id1);
	ecs::DestroyEntity(id2);

	CHECK(!ecs::IsValid(id1));
	CHECK(!ecs::IsValid(id2));
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(CreateAndDestroyMassive)
{
	std::vector<EntityId> ids;

#ifdef _DEBUG
	int entitiesCount = 10000;
#else
	int entitiesCount = 1000000;
#endif

	printf("Create and Destroy %d entities\n", entitiesCount);

	ids.reserve(entitiesCount);
	for (int i = 0; i < entitiesCount; i++)
	{
		ids.push_back(ecs::CreateEntity());
	}

	for (auto it = ids.begin(); it != ids.end(); ++it)
	{
		CHECK(ecs::IsValid(*it));
	}

	for (auto it = ids.begin(); it != ids.end(); ++it)
	{
		ecs::DestroyEntity(*it);
	}

	for (auto it = ids.begin(); it != ids.end(); ++it)
	{
		CHECK(!ecs::IsValid(*it));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(SimpleComponentTest)
{
	ecs::DestroyAll();

	EntityId id1 = ecs::CreateEntity();
	CHECK(ecs::IsValid(id1));

	PositionComponent* pPosComponent = ecs::GetComponent<PositionComponent>(id1);
	CHECK(pPosComponent == nullptr);

	RotationComponent* pRotComponent = ecs::GetComponent<RotationComponent>(id1);
	CHECK(pRotComponent == nullptr);

	ecs::AddComponent(id1, PositionComponent(13.6f, 19.79f));
	ecs::AddComponent(id1, RotationComponent(136.0f));

	pPosComponent = ecs::GetComponent<PositionComponent>(id1);
	CHECK(pPosComponent != nullptr);

	CHECK_CLOSE(pPosComponent->x, 13.6f, 0.0001f);
	CHECK_CLOSE(pPosComponent->y, 19.79f, 0.0001f);

	pRotComponent = ecs::GetComponent<RotationComponent>(id1);
	CHECK(pRotComponent != nullptr);

	CHECK_CLOSE(pRotComponent->angle, 136.0f, 0.0001f);

	ecs::RemoveComponent<PositionComponent>(id1);

	pPosComponent = ecs::GetComponent<PositionComponent>(id1);
	CHECK(pPosComponent == nullptr);

	CHECK_CLOSE(pRotComponent->angle, 136.0f, 0.0001f);

	ecs::RemoveComponent<RotationComponent>(id1);

	pRotComponent = ecs::GetComponent<RotationComponent>(id1);
	CHECK(pRotComponent == nullptr);

	ecs::AddComponent(id1, RotationComponent(13.0f));

	pRotComponent = ecs::GetComponent<RotationComponent>(id1);
	CHECK(pRotComponent != nullptr);

	CHECK_CLOSE(pRotComponent->angle, 13.0f, 0.0001f);

	//
	CHECK(!ecs::GetComponentStorage<RotationComponent>().empty());

	ecs::DestroyEntity(id1);

	CHECK(!ecs::IsValid(id1));

	//
	CHECK(ecs::GetComponentStorage<PositionComponent>().empty());
	CHECK(ecs::GetComponentStorage<RotationComponent>().empty());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(EntityAspectTest)
{

	EntityId id1 = ecs::CreateEntity(PositionComponent(1.0f, 2.0f), RotationComponent(11.0f));
	CHECK(ecs::IsValid(id1));

	EntityId id2 = ecs::CreateEntity(PositionComponent(3.0f, 4.0f), RotationComponent(12.0f));
	CHECK(ecs::IsValid(id2));

	EntityId id3 = ecs::CreateEntity(PositionComponent(5.0f, 6.0f));
	CHECK(ecs::IsValid(id3));

	typedef ecs::Aspect<const PositionComponent, const RotationComponent> TAspect;

	TAspect aspect1 = TAspect::Create(id1);
	TAspect aspect2 = TAspect::Create(id2);
	TAspect aspect3 = TAspect::Create(id3);

	CHECK(aspect1.id == id1);
	CHECK(aspect2.id == id2);
	CHECK(aspect3.id == id3);

	CHECK(aspect1.c0 != nullptr);
	CHECK(aspect1.c1 != nullptr);
	CHECK(aspect2.c0 != nullptr);
	CHECK(aspect2.c1 != nullptr);
	CHECK(aspect3.c0 != nullptr);
	CHECK(aspect3.c1 == nullptr);

	CHECK_CLOSE(aspect1.c0->x, 1.0f, 0.00001f);
	CHECK_CLOSE(aspect1.c0->y, 2.0f, 0.00001f);
	CHECK_CLOSE(aspect1.c1->angle, 11.0f, 0.00001f);

	CHECK_CLOSE(aspect2.c0->x, 3.0f, 0.00001f);
	CHECK_CLOSE(aspect2.c0->y, 4.0f, 0.00001f);
	CHECK_CLOSE(aspect2.c1->angle, 12.0f, 0.00001f);

	CHECK_CLOSE(aspect3.c0->x, 5.0f, 0.00001f);
	CHECK_CLOSE(aspect3.c0->y, 6.0f, 0.00001f);


	ecs::DestroyEntity(id1);
	ecs::DestroyEntity(id2);
	ecs::DestroyEntity(id3);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(GetActiveListTestOrdering)
{
	ecs::DestroyAll();

	std::vector<EntityId> ids;

	for (int i = 0; i < 200; i++)
	{
		EntityId id = ecs::CreateEntity();
		ids.push_back(id);
	}

	{
		const ecs::ConstEntityList& constList = ecs::GetActiveListConst();
		uint32_t prevIndex = constList[0].u.index;
		for (size_t i = 1; i < constList.size(); i++)
		{
			uint32_t currentIndex = constList[i].u.index;
			CHECK(prevIndex < currentIndex);
			prevIndex = currentIndex;
		}
	}


	for (int i = 0; i < 40; i++)
	{
		ecs::DestroyEntity(ids[90 + i]);
	}


	{
		const ecs::ConstEntityList& constList = ecs::GetActiveListConst();
		uint32_t prevIndex = constList[0].u.index;
		for (size_t i = 1; i < constList.size(); i++)
		{
			uint32_t currentIndex = constList[i].u.index;
			CHECK(prevIndex < currentIndex);
			prevIndex = currentIndex;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST(GetActiveListTest)
{
	ecs::DestroyAll();

	{
		const ecs::ConstEntityList& constList = ecs::GetActiveListConst();
		const ecs::EntityList& list = ecs::GetActiveList();

		CHECK(constList.empty());
		CHECK(list.empty());
	}

	EntityId id1 = ecs::CreateEntity();
	CHECK(ecs::IsValid(id1));

	EntityId id2 = ecs::CreateEntity();
	CHECK(ecs::IsValid(id2));

	EntityId id3 = ecs::CreateEntity();
	CHECK(ecs::IsValid(id3));

	EntityId id4 = ecs::CreateEntity();
	CHECK(ecs::IsValid(id4));


	{
		const ecs::ConstEntityList& constList = ecs::GetActiveListConst();
		const ecs::EntityList& list = ecs::GetActiveList();

		CHECK(constList.size() == 4);
		CHECK(list.size() == 4);

		for (size_t i = 0; i < constList.size(); i++)
		{
			CHECK(constList[i] == list[i]);
			CHECK(constList[i] == id1 || constList[i] == id2 || constList[i] == id3 || constList[i] == id4);
		}
	}

	ecs::DestroyEntity(id3);

	{
		const ecs::ConstEntityList& constList = ecs::GetActiveListConst();
		const ecs::EntityList& list = ecs::GetActiveList();

		CHECK(constList.size() == 3);
		CHECK(list.size() == 3);

		for (size_t i = 0; i < constList.size(); i++)
		{
			CHECK(constList[i] == list[i]);
			CHECK(constList[i] == id1 || constList[i] == id2 || constList[i] == id4);
		}
	}


	ecs::DestroyEntity(id2);

	{
		const ecs::ConstEntityList& constList = ecs::GetActiveListConst();
		const ecs::EntityList& list = ecs::GetActiveList();

		CHECK(constList.size() == 2);
		CHECK(list.size() == 2);

		for (size_t i = 0; i < constList.size(); i++)
		{
			CHECK(constList[i] == list[i]);
			CHECK(constList[i] == id1 || constList[i] == id4);
		}
	}

	ecs::DestroyEntity(id1);

	{
		const ecs::ConstEntityList& constList = ecs::GetActiveListConst();
		const ecs::EntityList& list = ecs::GetActiveList();

		CHECK(constList.size() == 1);
		CHECK(list.size() == 1);

		constList[0] == id4;
	}


	ecs::DestroyEntity(id4);


	{
		const ecs::ConstEntityList& constList = ecs::GetActiveListConst();
		const ecs::EntityList& list = ecs::GetActiveList();

		CHECK(constList.empty());
		CHECK(list.empty());
	}



}



}




