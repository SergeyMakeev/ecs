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
#include "EntityId.h"
#include "BitSet.h"
#include "Aspect.h"


namespace ecs
{
	//
	typedef uint8_t MapKey;
	static const MapKey MinMapKey = 0x0;
	static const MapKey MaxMapKey = 0xFE;
	static const MapKey UnusedMapKey = 0xFF;

	struct MapTuple
	{
		MapKey key;
		ConstEntityId id;

		static MapTuple Invalid()
		{
			MapTuple r;
			r.key = UnusedMapKey;
			r.id = ConstEntityId::Invalid();
			return r;
		}

		static MapTuple Create(MapKey key, ConstEntityId id)
		{
			MapTuple r;
			r.key = key;
			r.id = id;
			return r;
		}

	};

	typedef ecs::vector<MapTuple> RemapList;


	//
	// Returns an array of Entity Id's ordered by MapKey
	//    all Id's where MapKey == UnusedMapKey rejected.
	//
	//  Ordering is stable, ie if two objects with equal keys appear in the output array in the same order in as in input array
	//
	//  Worst/Best/Average-case performance is O(n)
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void FoldAndReorder(const RemapList& input, EntityList& output, BucketsList& buckets);


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct IProcessBase
	{
		virtual ~IProcessBase() {}

		virtual void ReMap(const ConstEntityList& entities, uint32_t maxEntityIndex) = 0;

		//
		virtual void Update(float deltaTime) = 0;
	};


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	struct Process : public IProcessBase
	{
		typedef T TAspect;

		Process()
		{
			ecs::RegisterProcess(this);
		}

		virtual ~Process()
		{
			ecs::UnregisterProcess(this);
		}


	};






}
