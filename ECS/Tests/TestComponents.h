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

#include <EntityId.h>

struct DummyComponent
{
	float x;
	char padding[200];
	float y;

	DummyComponent(float _x, float _y)
		: x(_x)
		, y(_y)
	{
	}

};


struct DummyComponent2
{
	int v;

	DummyComponent2(int _v)
		: v(_v)
	{
	}

};



struct Pos
{
	float x;
	float y;

	Pos(float _x, float _y)
		: x(_x)
		, y(_y)
	{
	}

};


struct Velocity
{
	float x;
	float y;

	Velocity(float _x, float _y)
		: x(_x)
		, y(_y)
	{
	}

};


struct Timer
{
	int time;

	Timer(int _time)
		: time(_time)
	{
	}

};



struct Dummy
{
	int val;

	Dummy(int _val)
		: val(_val)
	{
	}
};



struct PositionComponent
{
	float x;
	float y;

	PositionComponent(float _x, float _y)
		: x(_x)
		, y(_y)
	{
	}

};



struct RotationComponent
{
	float angle;

	RotationComponent(float _angle)
		: angle(_angle)
	{
	}
};



struct ParentComponent
{
	ConstEntityId id;

	ParentComponent(ConstEntityId _id)
		: id(_id)
	{
	}
};

