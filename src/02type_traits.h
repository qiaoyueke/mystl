/*	type_traits的作用是：分辨出容器内的实例在构造、销毁、移动时是否需要调用他们自身的函数
* 如果不需要做特殊处理，就可以直接使用memcpy、malloc等效率更高的做法，
* 如果对象内部含有指针之类的成员，就需要调用他们自身的成员函数
*/

#ifndef QYK_TYPE_TRAITS
#define QYK_TYPE_TRAITS

namespace qyk {
	namespace detail {
		/*根据iterator_traits的经验我们希望这样使用__type_traits:
	 *__type_traits<T>::has_trivial_default_constructor
	 *__type_traits<T>::is_POD_type
	 *
	 *他们返回“真”或“假”，以便我们决定采取什么策略，但bool值不能进行函数的重载与决议，
	 *因此我们定义以下两个具有真假性质的类，通过对象来进行函数的重载与决议	*/


		struct __true_type { }; //意思是这种类型可以直接进行操作，不需要考虑对象自己的构造或者析构函数，
		struct __false_type { }; //这种类型的元素需要使用自己的构造或析构函数，自定义的类型默认为这种类型


		/* 为了达成上述式子，__type_traits内必须定义一些typedefs，其值不是__true_type就是__false_type */
		template<class type>
		struct __type_traits {
			typedef __true_type this_dummy_member_must_be_first;
			/* 不要移除这个成员，他通知“有能力将 __type_traits 特化”的编译器说：我们现在所看到的这个		*/
			/* __type_traits template 是特殊的。这是为了确保万一编译器也使用一个名为 __type_traits 而其		*/
			/* 实与此处定义并无任何关联的 template，所有事情都仍将顺利运作。								*/

		/* 以下条件应该被遵守，因为编译器有可能自动为各型别产生专属的 __type_traits 特化版本： */
			/* 你可以重新排列以下成员的次序										*/
			/* 你可以移除以下任何成员											*/
			/* 绝对不可以将以下成员重新命名却没有改变编译器中的对应名称			*/
			/* 新加入的成员会被视为一般成员，除非你在编译器中加上适当的支持		*/
			typedef __false_type	has_trivial_default_constructor;
			typedef __false_type	has_trivial_copy_constructor;
			typedef __false_type	has_trivial_assignment_operator;
			typedef __false_type	has_trivial_destructor;
			typedef __false_type	is_POD_type;
			//SGI把所有内嵌型别都定义为__false_type，这是最保守的值（调用其ctor、detor），
			//对于每一个内置型别再定义其特化版本。
		};


		/* 以下是对所有c++标量型别所定义的__type_traits特化版本。
			 *这些定义对于内建有__type_traits支持能力的编译器并无伤害，对于无该等支持能力的编译器则属于必要。
			 *
			 *以下为C++基本型别提供特化版本，每一个成员值都是__true_type，
			 *表示这些型别都可以采用最快速方式（例如memcpy）进行拷贝、赋值等操作。
			 */
		template<> struct __type_traits<char> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<signed char> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<unsigned char> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<short> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<unsigned short> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<int> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<unsigned int> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<long> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<unsigned long> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<float> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<double> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		template<> struct __type_traits<long double> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

		//以下是针对原生指针设计的偏特化版本，原生指针也被视为一种标量型别
		template<class T>
		struct __type_traits<T*> {
			typedef __true_type		has_trivial_default_constructor;
			typedef __true_type		has_trivial_copy_constructor;
			typedef __true_type		has_trivial_assignment_operator;
			typedef __true_type		has_trivial_destructor;
			typedef __true_type		is_POD_type;
		};

	}//end of detail


	template<typename T1, typename T2>
	struct my_is_same{
		static constexpr bool value = false;
	};

	template<typename T>
	struct my_is_same<T,T>{
		static constexpr bool value = true;
	};

	template<bool B, class T=void>
	struct my_enable_if {};

	template<class T>
	struct my_enable_if<true, T> {
		using type = T;
	};
}//end of qyk


#endif // !QYK_TYPE_TRAITS


