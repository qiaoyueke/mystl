#ifndef QYK_ALGOBASE
#define QYK_ALGOBASE

#include "02iterator.h"
#include "02type_traits.h"
#include <string.h> //for memmove
#include <utility>	//for move

// 仿函数
namespace
{
	template <class T = void>
	struct less
	{
		constexpr bool operator()(const T &first, const T &last) const
		{
			return first < last;
		}
	};

	template <class T = void>
	struct Self
	{
		constexpr T operator()(const T &first) const
		{
			return first;
		}
	};

	template <class T = void>
	struct first
	{
		constexpr T operator()(const T &first) const
		{
			return first.first;
		}
	};

}

// copy,接受三个迭代器，将前两个迭代器之间的元素复制到第三个迭代器后面，返回复制后最后一个元素的下一个迭代器
namespace qyk
{
	namespace detail
	{

		// 根据迭代器种类，input_iterator_tag调用下面的实现
		template <class InputIterator, class OutputIterator, class Distance>
		inline OutputIterator __copy(InputIterator first, InputIterator last, OutputIterator result, Distance *, input_iterator_tag)
		{
			for (; first != last; ++first, ++result) // 以迭代器作为循环条件，速度慢
				*result = *first;
			return result;
		}

		// 根据迭代器种类，random_access_iterator_tag调用下面的实现
		template <class InputIterator, class OutputIterator, class Distance>
		inline OutputIterator __copy(InputIterator first, InputIterator last, OutputIterator result, Distance *, random_access_iterator_tag)
		{
			// std::cout << "copy start" << std::endl;
			Distance n = last - first;
			for (; n > 0; n--, ++first, ++result) // 以n作为循环条件，速度快
			{
				*result = *first;
			}

			return result;
		}
	}

	// 传入迭代器种类
	template <class InputIterator, class OutputIterator>
	inline OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
	{
		return detail::__copy(first, last, result, distance_type(first), iterator_category(first));
	}

	// 针对const char*的全特化版本
	inline char *copy(const char *first, const char *last, char *result)
	{
		memmove(result, first, last - first);
		return result + (last - first);
	}
}

// move,接受三个迭代器，将前两个迭代器之间的元素移动到第三个迭代器后面，
namespace qyk
{
	namespace detail
	{

		// 根据迭代器种类，input_iterator_tag调用下面的实现
		template <class InputIterator, class OutputIterator>
		inline OutputIterator __move(InputIterator first, InputIterator last, OutputIterator result, input_iterator_tag)
		{
			for (; first != last; ++first, ++result) // 以迭代器作为循环条件，速度慢
				*result = std::move(*first);
			return result;
		}

		// 根据迭代器种类，random_access_iterator_tag调用下面的实现
		template <class InputIterator, class OutputIterator>
		inline OutputIterator __move(InputIterator first, InputIterator last, OutputIterator result, random_access_iterator_tag)
		{
			auto n = last - first;
			for (; n > 0; n--, ++first, ++result) // 以n作为循环条件，速度快
				*result = std::move(*first);
			return result;
		}

	} // end of detail move

	// 传入迭代器种类
	template <class InputIterator, class OutputIterator>
	inline OutputIterator move(InputIterator first, InputIterator last, OutputIterator result)
	{
		return detail::__move(first, last, result, iterator_category(first));
	}

	template <class InputIterator>
	inline InputIterator move(InputIterator first, InputIterator last, InputIterator result)
	{
		return detail::__move(first, last, result, iterator_category(first));
	}
} // end of qyk move

// fill_n，接受一个迭代器、一个size常量 n一个常量 x，使用这个常量对这个迭代器之后的n个元素进行初始化，返回下一个迭代器
namespace qyk
{
	template <class ForwardIterator, typename size, typename T>
	ForwardIterator fill_n(ForwardIterator first, const size n, const T x)
	{
		for (size m = n; m != 0; --m, ++first)
		{
			*first = x;
		}
		return first;
	}
} // end of qyk

// fill，接受两个迭代器和一个常量，使用这个常量对两个迭代器之间的元素进行初始化
namespace qyk
{
	template <class ForwardIterator, typename T>
	void fill(ForwardIterator first, ForwardIterator last, const T x)
	{
		for (; first != last; ++first)
		{
			*first = x;
		}
	}

} // end of qyk

// copy_back, 将first到last的内容从尾部开始一一复制到result后面（顺序不变）
namespace qyk
{
	namespace detail
	{

		template <class BidirectionalIterator1, class BidirectionalIterator2, class Distance>
		BidirectionalIterator2 __copy_back(BidirectionalIterator1 first, BidirectionalIterator1 last, BidirectionalIterator2 result, Distance *, bidirectional_iterator_tag)
		{
			BidirectionalIterator2 cur = result;
			size_t step = 0;
			for (; first != last; ++cur, ++first)
				step++;
			BidirectionalIterator2 pur = cur;
			for (; step != 0;)
			{
				--cur;
				--first;
				*cur = *first;
			}
			return pur;
		}

		template <class BidirectionalIterator1, class BidirectionalIterator2, class Distance>
		BidirectionalIterator2 __copy_back(BidirectionalIterator1 first, BidirectionalIterator1 last, BidirectionalIterator2 result, Distance *, random_access_iterator_tag)
		{
			BidirectionalIterator2 cur = result;
			size_t step = 0;
			for (; first != last; ++cur, ++first)
				step++;
			BidirectionalIterator2 pur = cur + step;
			for (; step != 0;)
			{
				--cur;
				--first;
				*cur = *first;
			}
			return pur;
		}

	} // end of detail copy_back

	template <class BidirectionalIterator1, class BidirectionalIterator2>
	BidirectionalIterator2 copy_back(BidirectionalIterator1 first, BidirectionalIterator1 last, BidirectionalIterator2 result)
	{
		return detail::__copy_back(first, last, result, distance_type(first), iterator_category(result));
	}

} // end of qyk copy_back

// move_back, 将first到last的内容从尾部开始一一移动到result后面（顺序不变）
namespace qyk
{
	namespace detail
	{

		template <class BidirectionalIterator1, class BidirectionalIterator2>
		BidirectionalIterator2 __move_back(BidirectionalIterator1 first, BidirectionalIterator1 last, BidirectionalIterator2 result)
		{
			BidirectionalIterator2 cur = result;
			size_t step = 0;
			for (; first != last; ++cur, ++first)
				step++;
			BidirectionalIterator2 pur = cur;
			for (; step != 0; step--)
			{
				--cur;
				--first;
				*cur = std::move(*first);
			}
			return pur;
		}

	} // end of detail move_back

	template <class BidirectionalIterator1, class BidirectionalIterator2>
	BidirectionalIterator2 move_back(BidirectionalIterator1 first, BidirectionalIterator1 last, BidirectionalIterator2 result)
	{
		return detail::__move_back(first, last, result);
	}

} // end of qyk move_back

//_lower_bound 与_upper_bound 分别返回范围内第一个小于/大于x的迭代器
namespace qyk
{
	namespace detail
	{
		template <class ForwardIterator, class T, class Compare , class Distance>
		ForwardIterator _lower_bound(ForwardIterator first, ForwardIterator last, const T &x, Compare comp, forward_iterator_tag, Distance*)
		{
			Distance len = distance(first, last);
			Distance half = 0;
			ForwardIterator mid = first;

			while(len>0){
				half = len>>1;
				mid = first;
				advance(mid, half);

				if(comp(*mid, x)){
					first = ++mid;
					len = len - half - 1;
				}else{
					len = half;
				}
			}
			return first;
		}

		template <class RandomIterator, class T, class Compare , class Distance>
		RandomIterator _lower_bound(RandomIterator first, RandomIterator last, const T &x, Compare comp, random_access_iterator_tag, Distance*)
		{
			Distance len = last - first;
			Distance half = 0;
			RandomIterator mid = first;

			while(len>0){
				half = len>>1;
				mid = first;
				mid = mid + half;

				if(comp(*mid, x)){
					first = ++mid;
					len = len - half - 1;
				}else{
					len = half;
				}
			}
			return first;
		}

		template <class ForwardIterator, class T, class Compare , class Distance>
		ForwardIterator _upper_bound(ForwardIterator first, ForwardIterator last, const T &x, Compare comp, forward_iterator_tag, Distance*)
		{
			Distance len = distance(first, last);
			Distance half = 0;
			ForwardIterator mid = first;

			while(len>0){
				half = len>>1;
				mid = first;
				advance(mid, half);

				if(comp(x, *mid)){
					len = half;
				}else{
					first = ++mid;
					len = len - half - 1;
				}
			}
			return first;
		}

		template <class RandomIterator, class T, class Compare , class Distance>
		RandomIterator _upper_bound(RandomIterator first, RandomIterator last, const T &x, Compare comp, random_access_iterator_tag, Distance*)
		{
			Distance len = last - first;
			Distance half = 0;
			RandomIterator mid = first;

			while(len>0){
				half = len>>1;
				mid = first + half;

				if(comp(x, *mid)){
					len = half;
				}else{
					first = ++mid;
					len = len - half - 1;
				}
			}
			return first;
		}
	}//end of detail

	template <class Iterator, class T, class Compare = less<T>, typename my_enable_if<is_iterator<Iterator>::value>::type>
	Iterator lower_bound(Iterator first, Iterator last, const T &x, Compare comp = Compare())
	{
		return _lower_bound(first, last, x, comp, iterator_category(first), distance_type(first));
	}

	template <class Iterator, class T, class Compare = less<T>, typename my_enable_if<is_iterator<Iterator>::value>::type>
	Iterator upper_bound(Iterator first, Iterator last, const T &x, Compare comp = Compare())
	{
		return _upper_bound(first, last, x, comp, iterator_category(first), distance_type(first));
	}

}//end of lower_bound/ upper_bound


#endif
