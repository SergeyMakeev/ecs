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
#include <assert.h>
#include <cstring>
#include "Utils.h"

/* gcc/clang - support

unsigned char _BitScanForward(unsigned long* Index, unsigned long Mask)
{
	if (Mask == 0)
	{
		return 0;
	}

	*Index = __builtin_ctz(Mask);
	return 1;
}


*/


namespace ecs
{
	// betset for 384 bits (48 bytes)
	class bitset
	{
		// 12 * 4 = 48 bytes = 384 bits
		union
		{
			__m128i sse_storage[3];
			uint32_t storage[12];
		};

	public:

		enum MaxBitCount
		{
			value = 384,
		};

		class const_iterator
		{
			static const int32_t dwordsCount = 12;

			const bitset* const object;
			uint32_t currentDwordIndex;
			uint32_t currentMask;
			int32_t val;

			int32_t FindNextEnabledBit()
			{
				char res = 0;
				uint32_t bitIndex = (uint32_t)-1;
				for (; currentDwordIndex < dwordsCount; currentDwordIndex++, currentMask = 0xFFFFFFFF)
				{
					uint32_t v = object->storage[currentDwordIndex] & currentMask;
					res = _BitScanForward((unsigned long*)&bitIndex, v);
					if (res != 0)
					{
						break;
					}
				}

				// no more enabled bits
				if (res == 0)
				{
					return -1;
				}

				assert(bitIndex < 32);

				// update hide mask
				uint32_t hideAlreadyVisitedMask = ~(1 << bitIndex);
				currentMask &= hideAlreadyVisitedMask;
				return narrow_cast<int32_t>(currentDwordIndex * 32 + bitIndex);
			}

		public:

			const_iterator(const bitset* const p, uint32_t firstDwordIndex = 0)
				: object(p)
				, currentDwordIndex(firstDwordIndex)
				, currentMask(0xFFFFFFFF)
			{
				if (firstDwordIndex < dwordsCount)
					val = FindNextEnabledBit();
				else
					val = -1;
			}

			bool operator != (const const_iterator& other) const
			{
				// different bitsets
				if (object != other.object)
					return true;

				// different values
				if (val != other.val)
					return true;

				return false;
			}

			const_iterator& operator++ ()
			{
				val = FindNextEnabledBit();
				return *this;
			}

			uint32_t operator* () const
			{
				assert(val >= 0);
				return narrow_cast<uint32_t>(val);
			}
		};


		bitset()
		{
			clear();
		}

		bitset(const bitset& other)
		{
			sse_storage[0] = other.sse_storage[0];
			sse_storage[1] = other.sse_storage[1];
			sse_storage[2] = other.sse_storage[2];
		}

		bitset& operator=(const bitset& other)
		{
			sse_storage[0] = other.sse_storage[0];
			sse_storage[1] = other.sse_storage[1];
			sse_storage[2] = other.sse_storage[2];
			return *this;
		}


		void clear()
		{
			__m128i zero = _mm_setzero_si128();
			_mm_store_si128(&sse_storage[0], zero);
			_mm_store_si128(&sse_storage[1], zero);
			_mm_store_si128(&sse_storage[2], zero);
		}

		void set(uint32_t index)
		{
			assert(index < MaxBitCount::value);

			uint32_t dwordIndex = (index >> 5); //div 32
			uint32_t bitIndex = index - (dwordIndex << 5);
			assert(bitIndex < 32);
			uint32_t mask = 1 << bitIndex;

			// turn on the actual bit
			storage[dwordIndex] |= mask;
		}

		void reset(uint32_t index)
		{
			assert(index < MaxBitCount::value);

			uint32_t dwordIndex = (index >> 5); //div 32
			uint32_t bitIndex = index - (dwordIndex << 5);
			assert(bitIndex < 32);
			uint32_t mask = 1 << bitIndex;

			// turn off the actual bit
			storage[dwordIndex] &= ~mask;
		}


		void flip(uint32_t index)
		{
			assert(index < MaxBitCount::value);

			uint32_t dwordIndex = (index >> 5); //div 32
			uint32_t bitIndex = index - (dwordIndex << 5);
			assert(bitIndex < 32);
			uint32_t mask = 1 << bitIndex;

			// flip the actual bit
			storage[dwordIndex] ^= mask;
		}

		bool get(uint32_t index) const
		{
			assert(index < MaxBitCount::value);

			uint32_t dwordIndex = (index >> 5); //div 32
			uint32_t bitIndex = index - (dwordIndex << 5);
			assert(bitIndex < 32);
			uint32_t mask = 1 << bitIndex;

			// check the actual bit
			return (storage[dwordIndex] & mask) != 0;
		}

		bool contains(const bitset& other) const
		{
			// and
			__m128i a = _mm_and_si128(sse_storage[0], other.sse_storage[0] );
			__m128i b = _mm_and_si128(sse_storage[1], other.sse_storage[1] );
			__m128i c = _mm_and_si128(sse_storage[2], other.sse_storage[2] );

			// compare and move compare results into integer register
			int v0 = _mm_movemask_epi8(_mm_cmpeq_epi32(a, other.sse_storage[0]));
			int v1 = _mm_movemask_epi8(_mm_cmpeq_epi32(b, other.sse_storage[1]));
			int v2 = _mm_movemask_epi8(_mm_cmpeq_epi32(c, other.sse_storage[2]));

			// merge results
			int res = v0 & v1 & v2;

			return (res == 0xFFFF);

		}

		const_iterator begin() const
		{
			return const_iterator(this);
		}

		const_iterator end() const
		{
			return const_iterator(this, 0xFFFFFFFF);
		}


	};


}