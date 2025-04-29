#ifndef QYK_VECTOR
#define QYK_VECTOR

#include"01qyk_allocator.h"
#include"02iterator.h"
#include"02construct.h"
#include"02type_traits.h"
#include"02alogbase.h"
#include"02uninitialized.h"
#include<initializer_list>


namespace qyk {


	template<class T, class Alloc=alloc>
	class vector {
	public:
		
		typedef T					value_type;
		typedef value_type*			pointer;
		typedef const value_type*	const_pointer;
		typedef value_type*			iterator;
		typedef const value_type*	const_iterator;
		typedef value_type&			reference;
		typedef const value_type&	const_reference;
		typedef size_t				size_type;
		typedef ptrdiff_t			difference_type;
		typedef vector<T, Alloc> 	self;

	private:
		typedef allocator<T> data_allocator;
		iterator start;
		iterator finish;
		iterator endOfStorage;

		

		//申请n个元素的内存，并构造对象
		void fill_initialized(size_type n, const_reference value) {
			start = data_allocator::allocate(n);
			finish = start + n;
			endOfStorage = finish;
			uninitialized_fill_n(start, n, value);
		}


	public:
		//默认构造
		vector() :start(nullptr), finish(nullptr), endOfStorage(nullptr) {};
		//构造n个value
		vector(size_type n, value_type value) {
			fill_initialized(n, value);
		}
		//构造n个默认值
		vector(size_type n) {
			fill_initialized(n, T());
		}

		//通过两个迭代器构造
		template<class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
		vector(Iterator first, Iterator last) {
			size_type n = qykDistance(first, last);
			start = data_allocator::allocate(n);
			uninitialized_copy(first, last, start);
			finish = start + n;
			endOfStorage = finish;
		}

		//拷贝构造
		vector(const vector<T>& other) {
			start = data_allocator::allocate(other.size());
			uninitialized_copy(other.begin(), other.end(), start);
			finish = start + other.size();
			endOfStorage = finish;
		}

		//列表初始化
		vector(std::initializer_list<T> init_list) {
			start = data_allocator::allocate(init_list.size());
			finish = start + init_list.size();
			endOfStorage = finish;
			uninitialized_copy(init_list.begin(), init_list.end(), start);
		}

		//移动构造
		vector(vector<T>&& other)  noexcept  :start(other.start), finish(other.finish), endOfStorage(other.endOfStorage) {
			other.start = nullptr;
			other.finish = nullptr;
			other.endOfStorage = nullptr;
		};

		//析构
		~vector() {
			if (start) {
				qykDestroy(start, finish);
				data_allocator::deallocate(start, endOfStorage - start);
			}
			start = nullptr;
			finish = nullptr;
			endOfStorage = nullptr;
		}

		//返回管理的数组
		pointer data() { return empty() ? nullptr : (&*start); }

		iterator begin() {return start;}

		const_iterator begin() const { return start; }

		const_iterator cbegin() const { return start; }

		iterator end() {return finish;}

		const_iterator end() const { return finish; }

		const_iterator cend() const {return finish;}
		//访问元素
		reference operator[](size_type n) {return *(start + n);}
		//带有边界检查的元素访问
		reference at(size_type n) {
			if (n >= 0 && start + n < finish) {
				return *(start + n);
			}
			throw std::out_of_range();
		}

		reference front() { return *start; }

		const_reference front() const { return *start; }

		reference back() { return *(finish - 1); }
		
		const_reference back() const { return *(finish - 1); }

		bool empty() const { return start == finish ? true : false; }

		size_type size() const { return finish - start; }

		size_type capacity() const { return endOfStorage - start; }

		//进行扩容
		void reserve(size_type new_cap) {
			if (new_cap <= capacity()) return; //如果本来的大小就大于接受的值，则说明都不做
			iterator newStart = data_allocator::allocate(new_cap);
			uninitialized_move(start, finish, newStart);
			if (start) {
				qykDestroy(start, finish);
			}
			size_type old = endOfStorage - start;
			if(start) data_allocator::deallocate(start, old);
			start = newStart;
			finish = start + old;
			endOfStorage = start + new_cap;
		}

		void clear() {
			qykDestroy(start, finish);
			finish = start;
		}

		void insert(const_iterator pos, const T& value) {
			insert(pos, 1, value);
		}

		void insert(const_iterator pos, size_type n, const T& value) {
			size_type npos = pos - start;
			if (n > endOfStorage - finish) reserve(2 * ((finish - start) + n));
			finish=move_back(start + npos, finish, start + npos + n);
			fill_n(start + npos, n, value);
		}

		template<class InputIterator, typename = typename my_enable_if<is_iterator<InputIterator>::value>::type>
		void insert(const_iterator pos, InputIterator begin, InputIterator end) {
			iterator vpos = const_cast<iterator>(pos);
			if ((&*pos) < (&*end) && (&*finish) >= (&*end)) {
				//满足条件则说明begin和end都是这个vector的迭代器，并且覆盖了pos，对pos操作会使begin到end
				//之间的数据被覆盖，需要进行备份,
				iterator build_start= data_allocator::allocate(qykDistance(begin, end));
				auto build_finish = build_start + qykDistance(begin, end);
				copy(begin, end, build_start);
				insert(pos, build_start, build_finish);
				if (build_start) {
					qykDestroy(build_start, build_finish);
				}
				data_allocator::deallocate(build_start, qykDistance(begin, end));
				return;
			}
			auto n = qykDistance(begin, end);
			size_type npos = pos - start;
			if (endOfStorage - finish < n) {
				iterator newStart = data_allocator::allocate(2 * ((finish - start) + n));
				iterator newEndOfStorage = newStart + 2 * ((finish - start) + n);
				iterator newFinish = newStart + n + size();
				
				move(start, vpos, newStart);
				move(vpos, finish, newStart + n + npos);
				uninitialized_copy(begin, end, newStart + npos);
				if (start) {
					qykDestroy(start, finish);
				}
				data_allocator::deallocate(start, endOfStorage - start);
				start = newStart;
				finish = newFinish;
				endOfStorage = newEndOfStorage;
				return;
			}
			finish=move_back(vpos, finish, vpos + n);
			uninitialized_copy(begin, end, vpos);
		}

		void push_back(const T& value) {
			//std::cout << "push_back" << std::endl;
			insert(finish, value);
		}

		//在pos处直接构造元素
		template<class... Args>
		void emplace(const_iterator pos, Args&&... args) {
			size_type npos = pos - start;
			if (finish == endOfStorage) {
				reserve(size() << 1);
			}
			move_back(start+npos, finish, start+npos+1);
			new(start + npos) T(std::forward<Args>(args)...);
			finish += 1;
		}

		template<class... Args>
		void emplace_back(Args&&... args) {
			emplace(finish, std::forward<Args>(args)...);
		}

		iterator erase(const_iterator begin, const_iterator end) {
			auto result=move(const_cast<iterator>(end), finish, const_cast<iterator>(begin));
			finish -= qykDistance(begin, end);
			return result;
		}

		iterator erase(const_iterator pos) {
			if (pos == finish) {
				qykDestroy(pos);
				finish--;
				return finish;
			}
			else {
				return erase(pos, pos + 1);
			}
		}

		void pop_back() {
			if(!empty())
			qykDestroy(--finish);
		}

	
		void resize(size_type n, value_type value = T()) {
			if (n <= finish - start) {
				qykDestroy(start + n, finish);
				finish = start + n;
				return;
			}
			if (n > endOfStorage - start) reserve(2 * (n + (finish - start)));
			iterator newfinish = start + n;
			fill(finish, newfinish, value);
			finish = newfinish;
		}

		void swap(self& other){
			qyk::swap(start, other.start);
			qyk::swap(finish, other.finish);
			qyk::swap(endOfStorage, other.endOfStorage);
			return;
		}
		
	};//end of vector
}//end of qyk




#endif // QYK_qyk_vector


