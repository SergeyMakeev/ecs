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


#ifndef _UNUSED
#define _UNUSED(T) (void)(T)
#endif


namespace ecs
{
	namespace memory
	{
		void* Alloc(size_t bytesCount, size_t align);
		void Free(void* p);
	}



	template <typename T, uint32_t Alignment>
	class aligned_allocator
	{
	public:

		// The following will be the same for virtually all allocators.
		typedef T * pointer;
		typedef const T * const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		T * address(T& r) const
		{
			return &r;
		}

		const T * address(const T& s) const
		{
			return &s;
		}

		size_type max_size() const
		{
			// The following has been carefully written to be independent of
			// the definition of size_type and to avoid signed/unsigned warnings.
			return (static_cast<size_type>(0) - static_cast<size_type>(1)) / sizeof(T);
		}

		// The following must be the same for all allocators.
		template <typename U>
		struct rebind
		{
			typedef aligned_allocator<U, Alignment> other;
		};

		bool operator!=(const aligned_allocator& other) const
		{
			return !(*this == other);
		}

		void construct(T * const p, const T& t) const
		{
			void * const pv = static_cast<void *>(p);
			new (pv) T(t);
		}

		void destroy(T * const p) const
		{
			_UNUSED(p);
			p->~T();
		}

		// Returns true if and only if storage allocated from *this
		// can be deallocated from other, and vice versa.
		// Always returns true for stateless allocators.
		bool operator==(const aligned_allocator&) const
		{
			return true;
		}

		// Default constructor, copy constructor, rebinding constructor, and destructor.
		// Empty for stateless allocators.
		aligned_allocator() { }
		aligned_allocator(const aligned_allocator&) { }
		template <typename U> aligned_allocator(const aligned_allocator<U, Alignment>&) { }
		~aligned_allocator() { }


		// The following will be different for each allocator.
		T * allocate(const size_type n) const
		{
			// The return value of allocate(0) is unspecified.
			// Mallocator returns NULL in order to avoid depending
			// on malloc(0)'s implementation-defined behavior
			// (the implementation can define malloc(0) to return NULL,
			// in which case the bad_alloc check below would fire).
			// All allocators can return NULL in this case.
			if (n == 0) 
				return nullptr;	

			// All allocators should contain an integer overflow check.
			// The Standardization Committee recommends that std::length_error
			// be thrown in the case of integer overflow.
			if (n > max_size())
				return nullptr;

			// Mallocator wraps malloc().
			void * const pv = memory::Alloc(n * sizeof(T), Alignment);

			// Allocators should throw std::bad_alloc in the case of memory allocation failure.
	/*
			if (pv == NULL)
			{
				throw std::bad_alloc();
			}
	*/

			return static_cast<T *>(pv);
		}

		void deallocate(T * const p, const size_type) const
		{
			memory::Free(p);
		}


		// The following will be the same for all allocators that ignore hints.
		template <typename U>
		T * allocate(const size_type n, const U * /* const hint */) const
		{
			return allocate(n);
		}

		// Allocators are not required to be assignable, so
		// all allocators should have a private unimplemented
		// assignment operator. Note that this will trigger the
		// off-by-default (enabled under /Wall) warning C4626
		// "assignment operator could not be generated because a
		// base class assignment operator is inaccessible" within
		// the STL headers, but that warning is useless.
	private:
		aligned_allocator& operator=(const aligned_allocator&);
	};


	template<class T>
	class vector : public std::vector<T, aligned_allocator<T, 16>>
	{
	public:
		~vector()
		{
			static_assert( (sizeof(T) % 16 == 0) || (16 % sizeof(T) == 0), "Can't use aligned vector with not aligned type" );
		}
	};

}


#undef _UNUSED
