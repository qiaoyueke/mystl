#ifndef QYK_UNINITIALIZED
#define QYK_UNINITIALIZED

#include"02iterator.h"
#include"02type_traits.h"
#include"02construct.h"
#include"02alogbase.h"
#include<utility>
#include<string.h>

namespace qyk {
	namespace detail {
		//模板函数偏特化，传入的is_POD_type()是__true_type类型时调用这个函数，说明是普通的类型，可以直接copy
		//copy时根据迭代器种类的不同和元素种类的不同，可有不同的copy方式，算法放到02alogbase.h中实现
		template<class InputIterator, class ForwardIteartor>
		ForwardIteartor __uninitalized_copy_aux (InputIterator first, InputIterator last, ForwardIteartor result, __true_type) {
			return copy(first,last,result);
		}

		//传入的is_POD_type()是__false_type类型时调用这个函数,依次调用构造函数
		template<class InputIterator, class ForwardIteartor>
		ForwardIteartor __uninitalized_copy_aux(InputIterator first, InputIterator last, ForwardIteartor result, __false_type) {
			ForwardIteartor cur = result;
			try{
				for (; first != last;) {
					construct(&*cur, *first);
					++first;
					++cur;
				}
			}
			catch (...) {
				ForwardIteartor pur = result;
				for (; pur != cur; pur++) {
					qykDestroy(&*pur);
				}
				throw;
			}
			return cur;
		}

		//根据传入的value_type(result)这跟指针（T*）推出T的类型，用type_traits<T>::判断这种类型是否需要进行构造与销毁
		template<class InputIterator, class ForwardIteartor, class T>
		ForwardIteartor __uninitalized_copy(InputIterator first, InputIterator last, ForwardIteartor result, T*) {
			typedef typename __type_traits<T>::is_POD_type is_POD_type;
			return __uninitalized_copy_aux(first, last, result, is_POD_type());
		}

		

		//模板函数偏特化，传入的is_POD_type()是__true_type类型时调用这个函数，说明是普通的类型，可以直接copy
		//copy时根据迭代器种类的不同和元素种类的不同，可有不同的copy方式，算法放到02alogbase.h中实现
		template<class InputIterator, class ForwardIteartor>
		ForwardIteartor __uninitalized_move_aux(InputIterator first, InputIterator last, ForwardIteartor result, __true_type) {
			return move(first, last, result);
		}

		//传入的is_POD_type()是__false_type类型时调用这个函数,依次调用构造函数
		template<class InputIterator, class ForwardIteartor>
		ForwardIteartor __uninitalized_move_aux(InputIterator first, InputIterator last, ForwardIteartor result, __false_type) {
			ForwardIteartor cur = result;
			InputIterator cfirst = first;
			try {
				for (; first != last;++first,++cur) {
					construct(*cur, std::move(*first));
				}
			}
			catch (...) {
				ForwardIteartor pur = result;
				for (; pur != cur; pur++, cfirst++) {
					*cfirst = std::move(*pur);
					qykDestroy(&*pur);
				}
				throw;
			}
			return cur;
		}

		//根据传入的value_type(result)这跟指针（T*）推出T的类型，用type_traits<T>::判断这种类型是否需要进行构造与销毁
		template<class InputIterator, class ForwardIteartor, class T>
		ForwardIteartor __uninitalized_move(InputIterator first, InputIterator last, ForwardIteartor result, T*) {
			typedef typename __type_traits<T>::is_POD_type is_POD_type;
			return __uninitalized_move_aux(first, last, result, is_POD_type());
		}


		//根据传入的迭代器判断调用哪一种__uninitialized_fill_aux
		template<typename ForwardIterator, typename T, typename U>
		inline void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x, U*) {
			typedef typename __type_traits<U>::is_POD_type is_POD_type;
			__uninitialized_fill_aux(first, last, x, is_POD_type());
		}

		template<typename ForwardIterator, typename T>
		inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __true_type) {
			fill(first, last, x);
		}

		template<typename ForwardIterator, typename T>
		inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __false_type) {
			ForwardIterator cur = first;
			try{
				for (; first != last; ++first) {
					construct(&*first, x);
				}
			}
			catch (...) {
				ForwardIterator pur = first;
				for (; pur != cur;) {
					qykDestroy(&*pur);
				}
				throw;
			}
			
		}


		//根据传入的迭代器判断调用哪一种__uninitialized_fill_n_aux
		template<typename ForwardIterator, typename size, typename T, typename U>
		inline ForwardIterator __uninitialized_fill_n(ForwardIterator first, size n, const T& x, U*) {
			typedef typename __type_traits<U>::is_POD_type is_POD_type;
			return __uninitialized_fill_n_aux(first, n, x, is_POD_type());
		}

		template<typename ForwardIterator, typename size, typename T>
		inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, size n, const T& x, __true_type) {
			return fill_n(first, n, x);
		}

		template<typename ForwardIterator, typename size, typename T>
		inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, size n, const T& x, __false_type) {
			ForwardIterator cur = first;
			try{
				for (; n != 0; --n, ++cur) {
					construct(&*cur, x);
				}
			}
			catch (...) {
				ForwardIterator pur = first;
				for (; pur != cur; ++pur) {
					qykDestroy(&*pur);
				}
				throw;
			}
			return cur;
		}




	}//end of detail


	//这个函数目的是从result开始，用first到last之间的对象进行填充
	//需要判别的是，是否需要挨个调用这些对象的构造函数，还是可以通过直接赋值进行填充，
	//通过value_type判别迭代器管理的元素的类型，value_type(result)返回的是iterator_traits<iterator>::value_type*(0)，是一根指针;
	template<class InputIterator, class ForwardIteartor>
	inline ForwardIteartor uninitialized_copy(InputIterator first, InputIterator last, ForwardIteartor result) {
		return detail::__uninitalized_copy(first, last, result, value_type(result));
	}

	//针对 const char* 的特化版本，采用高效的memmove 
	inline char* uninitialized_copy(const char* first, const char* last, char* result) {
		memmove(result, first, last - first);
		return result + (last - first);
	}

	//这个函数目的是将first到last之间的对象移动到result之后
	//需要判别的是，是否需要挨个调用这些对象的构造函数，还是可以通过直接赋值进行移动，
	//通过value_type判别迭代器管理的元素的类型，value_type(result)返回的是iterator_traits<iterator>::value_type*(0)，是一根指针;
	template<class InputIterator, class ForwardIteartor>
	inline ForwardIteartor uninitialized_move(InputIterator first, InputIterator last, ForwardIteartor result) {
		return detail::__uninitalized_move(first, last, result, value_type(result));
	}

	//将first到last之间用x构造
	template<typename ForwardIterator, typename T>
	inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x) {
		detail::__uninitialized_fill(first, last, x, value_type(first));
		
	}

	//从first开始的n个用x构造
	template<typename ForwardIterator, typename Size, typename T>
	inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x) {
		return detail::__uninitialized_fill_n(first, n, x, value_type(first));
		
	}


}




	
#endif // !QYK_UNINITIALIZED


