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

//
// Entity Identifier, acts as smart pointer (WeakPtr) for entities
//
// If the entity is destroyed, this identifier automatically
// becomes invalid using generations mechanism.
//
//////////////////////////////////////////////////////////////////////////////
struct ConstEntityId
{
	union
	{
		struct
		{
			uint32_t generation : 12;  // 0 .. 4095
			uint32_t index : 20;       // 0 .. 1048575
		} u;
		uint32_t _dw;
	};

	inline bool operator== (const ConstEntityId& other) const
	{
		return (_dw == other._dw);
	}

	inline bool operator!= (const ConstEntityId& other) const
	{
		return (_dw != other._dw);
	}

	inline bool IsValid() const
	{
		return (_dw != NotUsed);
	}

	inline static ConstEntityId Invalid()
	{
		ConstEntityId r { NotUsed };
		return r;
	}

	static const uint32_t NotUsed = 0;
};

// Identifier must have the smallest possible size
static_assert(sizeof(ConstEntityId) == 4, "sizeof(ConstEntityId) != 4");


struct EntityId : public ConstEntityId
{
	inline void Invalidate()
	{
		_dw = NotUsed;
	}

	struct internal
	{
		static inline EntityId CreateFromConst(ConstEntityId other)
		{
			EntityId r;
			r._dw = other._dw;
			return r;
		}
	};

};
static_assert(sizeof(EntityId) == 4, "sizeof(EntityId) != 4");


