// 定义了iterator，分为5种，只读、只写、前向读写、双向读写、随机存储迭代器
// 其中，前向读写迭代器继承自只写寄存器，双向读写继承自前向读写迭代器器，随机存储迭代器继承自双向读写寄存器

#ifndef QYK_ITERATOR
#define QYK_ITERATOR

#include <cstddef>

namespace qyk
{
	// 只读迭代器
	struct output_iterator_tag
	{
	};
	// 只写迭代器
	struct input_iterator_tag
	{
	};
	// 读写迭代器
	struct forward_iterator_tag : public input_iterator_tag
	{
	};
	// 双向迭代器
	struct bidirectional_iterator_tag : public forward_iterator_tag
	{
	};
	// 随机读写迭代器
	struct random_access_iterator_tag : public bidirectional_iterator_tag
	{
	};

	// 所有的迭代器都继承自这一种基类
	template <class Category,
			  class T,
			  class Distance = ptrdiff_t,
			  class Pointer = T *,
			  class Reference = T &>
	struct iterator
	{
		typedef Category iterator_category;
		typedef T value_type;
		typedef Distance difference_type;
		typedef Pointer pointer;
		typedef Reference reference;
	};

	// traits，萃取出所有迭代器的种类，经过traits之后，所有迭代器将自动使用适合自身属性的操作函数

	template <typename... Ts>
	struct dmake_void
	{
		using type = void;
	};
	template <typename... Ts>
	using dvoid_t = typename dmake_void<Ts...>::type;
	
	template <class Iterator,typename=void>
	struct iterator_traits{

	};


	template <class Iterator>
	struct iterator_traits<Iterator,dvoid_t<typename Iterator::iterator_category>>
	{
		typedef typename Iterator::iterator_category iterator_category;
		typedef typename Iterator::value_type value_type;
		typedef typename Iterator::difference_type difference_type;
		typedef typename Iterator::pointer pointer;
		typedef typename Iterator::reference reference;
	};

	// iterator_traits针对原生指针的特化
	template <class T>
	struct iterator_traits<T *>
	{
		typedef random_access_iterator_tag iterator_category;
		typedef T value_type;
		typedef ptrdiff_t difference_type;
		typedef T *pointer;
		typedef T &reference;
	};

	// iterator_traits针对原生常量指针的特化
	template <class T>
	struct iterator_traits<const T *>
	{
		typedef random_access_iterator_tag iterator_category;
		typedef T value_type;
		typedef ptrdiff_t difference_type;
		typedef const T *pointer;
		typedef const T &reference;
	};

	// 返回difference_type的临时对象，目的是获取迭代器的difference类型
	template <class Iterator>
	inline typename iterator_traits<Iterator>::difference_type *
	distance_type(const Iterator &)
	{
		return static_cast<typename iterator_traits<Iterator>::difference_type *>(0);
	}

	// 同上，获取iterator_tag
	template <class Iterator>
	inline typename iterator_traits<Iterator>::iterator_category
	iterator_category(const Iterator &)
	{
		typedef typename iterator_traits<Iterator>::iterator_category category;
		return category(); // 返回一个临时对象
	}

	// 同上，获取value_type,返回的是一个指针
	template <class Iterator>
	inline typename iterator_traits<Iterator>::value_type *
	value_type(const Iterator &)
	{
		return static_cast<typename iterator_traits<Iterator>::value_type *>(0);
	}

	// 内部使用的迭代器操作：distance，advance，分别是获取两个迭代器之间的距离与对一个迭代器进行移动
	namespace detail
	{
		// 所有拥有写入功能的迭代器可用使用，它们有自增的能力
		template <class InputIterator>
		inline typename iterator_traits<InputIterator>::difference_type
		__distance(InputIterator first, InputIterator last, input_iterator_tag)
		{
			typename iterator_traits<InputIterator>::difference_type n = 0;
			while (first != last)
			{
				++first;
				++n;
			}
			return n;
		}

		// 针对随机读写迭代器，这种迭代器可以相减
		template <class RandomAccessIterator>
		inline typename iterator_traits<RandomAccessIterator>::difference_type
		__distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
		{

			return last - first;
		}

		// input_iterator，只能通过自增移动
		template <class InputIterator, class Distance>
		inline void __advance(InputIterator &iter, Distance n, input_iterator_tag)
		{
			while (n--)
				++iter;
		}

		// 双向迭代器，可以自增向后移动，也可以自减向前移动
		template <class BidirectionalIterator, class Distance>
		inline void __advance(BidirectionalIterator &iter, Distance n, bidirectional_iterator_tag)
		{
			if (n >= 0)
				while (n--)
					++iter;
			else
				while (n++)
					--iter;
		}

		// 可以加减距离移动
		template <class RandomAccessIterator, class Distance>
		inline void __advance(RandomAccessIterator &iter, Distance n, random_access_iterator_tag)
		{
			iter += n;
		}
	} // end of detail

	// distance标准接口
	template <class InputIterator>
	inline typename iterator_traits<InputIterator>::difference_type
	distance(InputIterator first, InputIterator last)
	{
		typedef typename iterator_traits<InputIterator>::iterator_category category;
		return detail::__distance(first, last, category());
	}

	// advance标准接口
	template <class InputIterator, class Distance>
	inline void advance(InputIterator &iter, Distance n)
	{
		detail::__advance(iter, n, iterator_category(iter));
	}

	template <class T, T v>
	struct integral_constant
	{
		static constexpr T value = v;									 // 值
		using value_type = T;											 // 值的类型
		using type = integral_constant;									 // 类型自身
		constexpr operator value_type() const noexcept { return value; } // 隐式转换为值
	};

	// true_type = integral_constant<bool, true>
	using true_type = integral_constant<bool, true>;

	// false_type = integral_constant<bool, false>
	using false_type = integral_constant<bool, false>;

	template <typename... Ts>
	struct make_void
	{
		using type = void;
	};
	template <typename... Ts>
	using void_t = typename make_void<Ts...>::type;

	template <typename T, typename = void>
	struct is_iterator 
	{		
		static constexpr bool value = false;
	};

	template <typename T>
	struct is_iterator<T, void_t<typename iterator_traits<T>::iterator_category>> 
	{
		static constexpr bool value = true;
	};

	
	// template <typename T,typename = void>
	// struct is_iterator
	// {

	// 	template <typename U>
	// 	static auto test(int) -> decltype(void(typename iterator_traits<U>::iterator_category()), true_type{});

	// 	template <typename U>
	// 	static false_type test(...);

	// 	static constexpr bool value = decltype(test<T>(0))::value;
	// };

} // end of qyk

#endif // !QYK_ITERATOR
