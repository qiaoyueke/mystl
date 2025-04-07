#ifndef QYK_CONSTRUCT
#define QYK_CONSTRUCT

#include"02iterator.h"
#include"02type_traits.h"
#include<new>

namespace qyk {

	template<class T1, class T2>
	inline void construct(T1* p, const T2& value) {
		new(p) T1(value);
	}

	template<class T1, class T2>
	inline void construct(T1* p, T2&& value) {
		new(p) T1(value);
	}

	template<class T1, class...Argus>
	inline void construct(T1* p, Argus&&... argus) {
		new(p) T1(std::forward<Argus>(argus)...);
	}

	template<typename T>
	void destroy(T* p) {
		p->~T();
	}

	namespace detail {
		//对于传入参数中可以判断T的类型的，根据type_traits获取has_trivial_destructor判断是否需要调用析构函数
		template<typename ForwardIterator, typename T>
		inline void __destroy(ForwardIterator first, ForwardIterator last, T*) {
			typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
			__destroy_aux(first, last, trivial_destructor());
		}

		//如果传入的trivial_destructor()是__false_type类型，则使用下面这个 __destroy_aux，依次调用T的destory
		template<typename ForwardIterator>
		inline void __destroy_aux(ForwardIterator first, ForwardIterator last, __false_type) {
			for (; first != last; ++first) {
				destroy(&*first); //传入的是一个指针，指向first这个迭代器指向的元素，这个函数执行：(&*first)->~T();
			}
		}

		//如果传入的trivial_destructor()是___true_type类型，则什么都不做
		template<typename ForwardIterator>
		inline void __destroy_aux(ForwardIterator, ForwardIterator, __true_type) { }


	}//end of detail

	template<typename ForwardIterator>
	inline void destroy(ForwardIterator first, ForwardIterator last) {
		detail::__destroy(first, last, value_type(first));	//value_type函数实现于02iterator.h，通过迭代器获取元素的类型
	}


	//以下是destroy第二版本针对迭代器为char*和wchar_t*的特化版（他们什么也不用做）
	inline void destroy(char*, char*) { }
	inline void destroy(wchar_t*, wchar_t*) { }

}//end of qyk






#endif // !QYK_CONSTRUCT


