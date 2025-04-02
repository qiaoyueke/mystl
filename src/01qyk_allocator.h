#ifndef QYK_ALLOCATOR
#define QYK_ALLOCATOR


#include"01pool_allocator.h"
#include"01loki_allocator.h"


namespace qyk {

	typedef detail::__qyk_pool_alloc<false, 0> alloc;  //默认非多线程环境,使用pool_allocator

	template<typename T, class Alloc = alloc>
	class allocator {
	public:
		typedef T				value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t			size_type;
		typedef ptrdiff_t		difference_type;


		//rebind allocator of type U
		//这一段的作用是 ：使用容器时，实际申请的内存不只用来存放元素，比如链表还包括两个指针，可以转换到对应类型的分配器
		template<class U>
		struct rebind {
			typedef allocator<U, Alloc> other;
		};

		static pointer allocate(size_t n) {
			return 0 == n ? nullptr : (pointer)Alloc::allocate(n * sizeof(T));
		}

		static pointer allocate(void) {
			return (pointer)Alloc::allocate(sizeof(T));
		}

		static void deallocate(const_pointer ptr) {
			if (!ptr)Alloc::deallocate((void*)ptr, sizeof(T));
		}

		static void deallocate(const_pointer ptr, size_t n) {
			if (!ptr)Alloc::deallocate((void*)ptr, n * sizeof(T));
		}

		template<class U>
		static void construct(pointer ptr, const U n) {
			new(ptr) value_type(std::forward<U>(n));
		}

		static void destory(pointer ptr) {
			ptr->~T();
		}
	};
}


#endif // !QYK_ALLOCATOR



