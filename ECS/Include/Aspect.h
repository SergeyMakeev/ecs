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

#include <vector>
#include <functional>




//
// Generate bitset from template params
//
#define GENERATE_MASK_FOR_PARAM(NUM)                                                                            \
{                                                                                                               \
	const uint32_t componentTypeIndex = ecs::GetComponentTypeIndex<std::remove_const<T##NUM>::type>();          \
	componentsMask.set(componentTypeIndex);                                                                     \
	                                                                                                            \
	const bool isReadOnlyAccess = std::is_const< T##NUM >::value;                                               \
	if (isReadOnlyAccess)                                                                                       \
	{                                                                                                           \
		accesTypeReadOnlyMask.set(componentTypeIndex);                                                          \
	}                                                                                                           \
}                                                                                                               \


// T0 - source type (int)
// P0 - pointer type  (int*)
// CP0 - const pointer type (const int*)
#define DECLARE_ADDITIONAL_TYPES(NUM)                                                                                             \
	static_assert(!std::is_reference< T##NUM >::value, "Type can't be reference!");                                               \
	static_assert(!std::is_pointer< T##NUM >::value, "Type can't be pointer!");                                                   \
	typedef typename std::add_pointer< T##NUM >::type P##NUM;                                                                     \
	typedef typename std::add_pointer<  typename std::add_const< T##NUM >::type >::type CP##NUM;                                  \







namespace ecs
{
	typedef uint8_t UnusedComponentTypeValue;
	typedef char FinalComponentTypeValue;



	//
	//
	// Appearance of the entity for the external viewer/editor
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<
		typename T0 = UnusedComponentTypeValue,
		typename T1 = UnusedComponentTypeValue,
		typename T2 = UnusedComponentTypeValue,
		typename TEND = FinalComponentTypeValue
	>
	struct Aspect
	{
		Aspect()
		{
			static_assert(false, "Partial template specialization goes wrong (too many params?)");
		}
	};

	//
	// Template specialization for 1 param.
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T0>
	struct Aspect<T0, UnusedComponentTypeValue, UnusedComponentTypeValue, FinalComponentTypeValue>
	{
		typedef typename Aspect<T0, UnusedComponentTypeValue, UnusedComponentTypeValue, FinalComponentTypeValue> ThisType;
		DECLARE_ADDITIONAL_TYPES(0);

		struct Const
		{
			ConstEntityId id;
			CP0 c0;

			static ecs_force_inline typename ThisType::Const Create(const ConstEntityId id)
			{
				ThisType::Const r;
				r.id = id;
				r.c0 = ecs::GetComponent<T0>(id);
				return r;
			}
		};


		EntityId id;
		P0 c0;
		
		static inline void GenerateMask(bitset& componentsMask, bitset& accesTypeReadOnlyMask)
		{
			GENERATE_MASK_FOR_PARAM(0);
		}

		static ecs_force_inline ThisType Create(const EntityId id)
		{
			ThisType r;
			r.id = id;
			r.c0 = ecs::GetComponent<T0>(id);
			return r;
		}


	};

	//
	// Template specialization for 2 params.
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T0, typename T1>
	struct Aspect<T0, T1, UnusedComponentTypeValue, FinalComponentTypeValue>
	{
		typedef typename Aspect<T0, T1, UnusedComponentTypeValue, FinalComponentTypeValue> ThisType;

		DECLARE_ADDITIONAL_TYPES(0);
		DECLARE_ADDITIONAL_TYPES(1);

		struct Const
		{
			ConstEntityId id;
			CP0 c0;
			CP1 c1;

			static ecs_force_inline typename ThisType::Const Create(const ConstEntityId id)
			{
				ThisType::Const r;
				r.id = id;
				r.c0 = ecs::GetComponent<T0>(id);
				r.c1 = ecs::GetComponent<T1>(id);
				return r;
			}
		};


		EntityId id;
		P0 c0;
		P1 c1;

		static inline void GenerateMask(bitset& componentsMask, bitset& accesTypeReadOnlyMask)
		{
			GENERATE_MASK_FOR_PARAM(0);
			GENERATE_MASK_FOR_PARAM(1);
		}

		static ecs_force_inline ThisType Create(const EntityId id)
		{
			ThisType r;
			r.id = id;
			r.c0 = ecs::GetComponent<T0>(id);
			r.c1 = ecs::GetComponent<T1>(id);
			return r;
		}
	};


	//
	// Template specialization for 3 params.
	//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T0, typename T1, typename T2>
	struct Aspect<T0, T1, T2, FinalComponentTypeValue>
	{
		typedef typename Aspect<T0, T1, T2, FinalComponentTypeValue> ThisType;

		DECLARE_ADDITIONAL_TYPES(0);
		DECLARE_ADDITIONAL_TYPES(1);
		DECLARE_ADDITIONAL_TYPES(2);

		struct Const
		{
			ConstEntityId id;
			CP0 c0;
			CP1 c1;
			CP2 c2;

			static ecs_force_inline typename ThisType::Const Create(const ConstEntityId id)
			{
				ThisType::Const r;
				r.id = id;
				r.c0 = ecs::GetComponent<T0>(id);
				r.c1 = ecs::GetComponent<T1>(id);
				r.c2 = ecs::GetComponent<T2>(id);
				return r;
			}
		};


		EntityId id;
		P0 c0;
		P1 c1;
		P2 c2;

		static inline void GenerateMask(bitset& componentsMask, bitset& accesTypeReadOnlyMask)
		{
			GENERATE_MASK_FOR_PARAM(0);
			GENERATE_MASK_FOR_PARAM(1);
			GENERATE_MASK_FOR_PARAM(2);
		}

		static ecs_force_inline ThisType Create(const EntityId id)
		{
			ThisType r;
			r.id = id;
			r.c0 = ecs::GetComponent<T0>(id);
			r.c1 = ecs::GetComponent<T1>(id);
			r.c2 = ecs::GetComponent<T2>(id);
			return r;
		}
	};

	



}

#undef GENERATE_MASK_FOR_PARAM
#undef DECLARE_ADDITIONAL_TYPES

