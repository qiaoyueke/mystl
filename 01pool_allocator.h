#ifndef _QYK_POOL_ALLOCATOR_
#define _QYK_POOL_ALLOCATOR_

#include<cstdlib> //malloc free

#if !defined(__THROW_BAD_ALLOC)
#include<iostream>
#define __THROW_BAD_ALLOC std::cerr<<"out of memery"<<std::endl; exit(1);
#endif

namespace qyk { //外部接口
	namespace detail { //内部实现

		constexpr int MIN_POOL_SIZE = 8;	//内存池分配内存块大小的下限
		constexpr int MAX_POOL_SIZE = 128;	//内存池分配内存块大小的上限
		constexpr int NEMB_OF_FREELIST = MAX_POOL_SIZE / MIN_POOL_SIZE; ////设定需要的空闲链表个数


		template<bool threads, int inst> //threads用来表示是单线程还是多线程 非型别参数inst并没有使用 后面分别定义为false和0
		class __qyk_pool_alloc {
		private:
			union obj{  
				//使用union结构，在内存未被分配时使用前面的位置作为指针指向下一块空间地址
				//分配后覆盖原来存放下一块地址的部分，将这块空间分给用户使用，以此节省cookie消耗
				obj* next; //指向下一块空闲地址
				char data;
			};

			static obj* volatile freeLists[NEMB_OF_FREELIST];

		public:
			static void* allocate(size_t);
			
			static void deallocate(void* ,size_t);

		private:
			static obj* freeStart ;  //空闲内存的起始位置
			static obj* freeEnd ;	//空闲内存的结束位置
			static size_t heapSize ;		//内存池申请的内存总量

			static void(*myHandler)();

			typedef void(*H)();
			static H setNewHandler(H h) {
				H old = myHandler;
				myHandler = h;
				return (old);
			};

			//计算目标内存块在空闲链表的序号（从0开始）
			inline static size_t find_freelist_numb(size_t targateSize) {
				return ((targateSize + MIN_POOL_SIZE - 1) / MIN_POOL_SIZE - 1);
			};

			//上调到8的倍数
			inline static size_t round_up(size_t targateSize) {
				return ((targateSize + MIN_POOL_SIZE - 1) & ~(MIN_POOL_SIZE - 1));
			};

			//目标内存大小对应的空闲链表空时，进行填充
			static void* refile(size_t needSize);

			static constexpr int NOBJS = 20; //每次填充时尝试填充的内存块个数上限

			//实际申请内存
			static char* chunk_alloc(size_t needSize, int& nobjs);  

			static void* oom_malloc(size_t needSize);

		};



		template<bool threads, int inst>
		typename __qyk_pool_alloc<threads, inst>::obj*
		volatile __qyk_pool_alloc<threads, inst>::freeLists[NEMB_OF_FREELIST]=
		{nullptr,nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

		template<bool threads, int inst>
		typename __qyk_pool_alloc<threads, inst>::obj* __qyk_pool_alloc<threads, inst>::freeStart = nullptr;  //空闲内存的起始位置

		template<bool threads, int inst>
		typename __qyk_pool_alloc<threads, inst>::obj* __qyk_pool_alloc<threads, inst>::freeEnd = nullptr;	//空闲内存的结束位置

		template<bool threads, int inst>
		size_t __qyk_pool_alloc<threads, inst>::heapSize = 0;		//内存池申请的内存总量

		template<bool threads, int inst>
		void(*__qyk_pool_alloc<threads, inst>::myHandler)() = nullptr;  //初始myHhandler
		

		template<bool threads, int inst>
		void* __qyk_pool_alloc<threads, inst>::allocate(size_t targetSize) {
			//std::cout << "pool_alloc" << std::endl;
			void* result; //分配给用户的内存地址

			if (MAX_POOL_SIZE < targetSize) { //直接交给malloc
				result = ::malloc(targetSize);
				if (nullptr == result) result = oom_malloc(targetSize); //如果分配失败就调用oom_malloc
				return (result);
			}

			//寻找需要的内存块在哪一个空闲链表下
			obj* volatile* targetList = freeLists + find_freelist_numb(targetSize);

			//如果目标链表非空，则从中分配一块给用户
			if (nullptr != *targetList) {
				result = *targetList;
				*targetList = (*targetList)->next;
				return (result);
			}

			//目标链表下没有可用的内存块，进行填充
			result = refile(round_up(targetSize));

			return (result);
		}//end of allocate

		//调用此函数时，空闲链表下没有可用空间，此函数将申请一块空间并对其进行分割管理
		template<bool threads, int inst>
		void* __qyk_pool_alloc<threads, inst>::refile(size_t needSize) {
			//std::cout << "refile" << std::endl;
			int nobjs = NOBJS; //本次填充获得的内存块个数

			//调用chunk_alloc申请内存，除这个chunk_alloc外的操作用于分割管理这块申请到的内存
			//nobjs传入时是通过refference的方式，实际申请到的块数会返回
			obj* result = (obj*)chunk_alloc(needSize, nobjs);

			//如果只分配了一块内存，则直接返回给用户，这个空闲链表下仍然为空
			if (1 == nobjs) {
				return ((void*)result);
			}

			//分配了超过一块内存时，对这块内存进行处理，挂在对应的空闲链表下
			//由于已经分配给用户了一块，只需要处理剩下的部分
			obj* volatile* targetList = freeLists + find_freelist_numb(needSize);
			*targetList = result + needSize;
			obj* cur = *targetList;
			for (int i = 2; i < nobjs; i++) {
				cur->next = cur + needSize;
				cur = cur->next;
			}
			cur->next = nullptr;

			return ((void*)result);
		} //end of refile

		template<bool threads, int inst>
		char* __qyk_pool_alloc<threads, inst>::chunk_alloc(size_t needSize,int &nobjs) {
			//std::cout << "chunk_alloc" << std::endl;
			size_t freeSize = freeEnd - freeStart;  //目前剩下没有挂载到空闲链表的内存大小
			size_t maxSize = needSize * nobjs;
			void* result;

			//如果剩下的部分不够分配一个对象，就将剩下的内存挂载到对应的空闲链表上
			//然后申请nobjs个目标内存块
			if (freeSize < needSize) {	
				//回收上次剩下的空闲内存
				if (freeSize > 0) {
					obj* volatile* listToAdd = freeLists + find_freelist_numb(freeSize);
					freeStart->next = *listToAdd;
					*listToAdd = freeStart;
				}
				
				size_t sizeToGet = 2 * maxSize + round_up(heapSize >> 2);

				result = ::malloc(sizeToGet);
				if (nullptr == result) {  //::malloc分配失败，为防止内存池占用所有内存资源，转为从目前可用的较大的内存块中取出使用
					
					for (size_t i = needSize; i <= MAX_POOL_SIZE; i += MIN_POOL_SIZE) {
						obj* volatile* listToFree = freeLists + find_freelist_numb(i);
						if (nullptr != *listToFree) {
							freeStart = *listToFree;
							freeEnd = freeStart + i;
							*listToFree = (*listToFree)->next;
							return (chunk_alloc(needSize, nobjs));
						}
					}
					//内存池里也没有可以分配的内存了，检查是否设置有解决方法（new_handler）；
					
					return ((char*)oom_malloc(needSize));
				}

				freeStart = (obj*)result + maxSize;
				freeEnd = (obj*)result +sizeToGet;
				heapSize += sizeToGet;
				
				return ((char*)result);
			}
			else if (freeSize >= maxSize) {	//当前剩下的内存足够分配nobjs个对象
				result = (void*)freeStart;
				freeStart += maxSize;
				return ((char*)result);
			}
			else {//剩下的部分足够分配一个，但不够分配nobjs个
				result = (void*)freeStart;
				nobjs = (freeEnd - freeStart) / needSize;
				freeStart += (nobjs * needSize);
				return ((char*)result);
			}
		}//end of chunk_alloc

		//oom_malloc
		template<bool threads, int inst>
		void* __qyk_pool_alloc<threads, inst>::oom_malloc(size_t n) {
			void* result;
			if (nullptr == myHandler) {
				__THROW_BAD_ALLOC;
			}
			for (int i = 0; i < 36; i++) {
				(*myHandler)();
				result = ::malloc(n);
				if (nullptr != result) {
					return result;
				}
			}
			__THROW_BAD_ALLOC;
		}

		

		//deallocate ,将对象的空间回收，重新挂载到对应的空闲链表
		template<bool threads, int inst>
		void __qyk_pool_alloc<threads, inst>::deallocate(void* ptr, size_t size) {
			obj* volatile* listToAdd = freeLists + round_up(size);
			obj* p = (obj*)ptr;
			p->next = *listToAdd;
			*listToAdd = p;
		}

		
	}//end of detail


	typedef detail::__qyk_pool_alloc<false, 0> alloc;
	

	template<typename T, class Alloc=alloc>
	class allocator {
	public:
		typedef T				value_type;
		typedef T*				pointer;
		typedef const T*		const_pointer;
		typedef T&				reference;
		typedef const T&		const_reference;
		typedef size_t			size_type;
		typedef ptrdiff_t		difference_type;


		//rebind allocator of type U
		//这一段的作用是  T中内涵别的元素比如指针时，可以隐式的转换到对应元素类型的分配器
		template<class U>
		struct rebind {
			typedef allocator<U,Alloc> other;
		};

		static pointer allocate(size_t n) {
			return 0==n ? 0 : (pointer)Alloc::allocate(n*sizeof(T));
		}

		static pointer allocate(void) {
			return (pointer)Alloc::allocate(sizeof(T));
		}

		static void deallocate(const_pointer ptr) {
			Alloc::deallocate((void*)ptr, sizeof(T));
		}
	};

}



#endif // !_QYK_POOL_ALLOCATOR_








