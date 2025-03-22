#ifndef QYK_LOKI_ALLOCATOR
#define QYK_LOKI_ALLOCATOR
#include<cstdlib> //malloc free
#include<new> //new


#if !defined(__THROW_BAD_ALLOC)
#include<iostream>
#define __THROW_BAD_ALLOC std::cerr<<"out of memery"<<std::endl; exit(1);
#endif

namespace qyk {
	namespace detail {
	
		template<bool threads, int inst> //threads用来表示是单线程还是多线程 非型别参数inst并没有使用 后面分别定义为false和0
		class __qyk_loki_fixed_alloc { //loki_allocate的第二级，处理确定大小的内存块，对chunks【】数组的管理
		public:
			class Chunk {  //fixedalloc内声明的chunk类，作用域仅在fixedalloc
			public:
				explicit Chunk(size_t blockSize);

				~Chunk() {
					//std::cout << "Chunk析构" << std::endl;
					::free((void*)pData);
				}

				
				void* chunkallocate(const size_t blockSize);

				void chunkdeallocate(void* ptr, const size_t blockSize);

			public:
				char numOfLeftBlock;
				char firstToAlloc;
				char* pData;
				//size_t blockSize;
			};// end of chunk 

		
			static constexpr int MAX_BLOCK_SIZE = 64;		//每一个chunk中含有的block数
			const size_t blockSize;					//这种fixedalloc负责管理的内存块大小，即每个block的大小,fixedalloc生成时初始化

		private:
			size_t maxChunksNow;				//现在为chunks数组最多存放多少个chunk
			size_t chunksNow;				//现在chunks数组里有多少个chunk
			size_t chunkToAlloc;		//上次进行分配的chunk编号
			size_t chunkToDealloc;		//上次回收的chunk编号
			Chunk** chunks;			//chunks数组

			void chunks_reset();		//当前chunks放满了，进行扩容
			void chunk_swap(size_t chunkToDalloc);		//chunk_dealloc后，当前chunk的空间全部回收，将这个chunk放到最后当作缓存，
														//当下一次有chunk全回收时，执行chunk_free
			void chunk_free();			//现在有两块chunk都全回收了，释放缓存的chunk
			void* chunk_add();			//目前所有chunk都满了，增加新的chunk


		public:
			__qyk_loki_fixed_alloc(size_t blockSize);

			~__qyk_loki_fixed_alloc()
			{
				for (int i = 0; i < chunksNow; i++) {
					delete chunks[i];
				}
				::free(chunks);
			}

			void* allocate();
			void deallocate(void* ptr);
		};//end of fixed_alloc


		//第三级allocate，查找目标chunk里是否还有空间，
		// 如果有，返回优先级最高的那一块地址，将这块地址内的编号对应的地址作为下一次的最高优先级地址
		//如果没有，返回空指针
		template<bool threads, int inst>
		void* __qyk_loki_fixed_alloc<threads, inst>::Chunk::chunkallocate(const size_t blockSize) {
			if (!numOfLeftBlock) {
				return nullptr;
			}
			numOfLeftBlock--;
			char* result = pData + firstToAlloc * blockSize;
			firstToAlloc = *result;
			return (void*)result;
		}

	
		//第二级allocate，fixedalloc里有chunks[]数组，chunkToAlloc记录的是上一次分配的chunk的编号，由于存在数据聚集现象，
		//优先在这个chunk内查找是否有空间分配，如果有就交给这个chunk分配；
		//如果没有，或者chunkToAlloc处于初始状态，就从头遍历寻找有空间的chunk
		//如果都没有就new一个新的chunk；
		template<bool threads, int inst>
		void* __qyk_loki_fixed_alloc<threads, inst>::allocate() {
			void* result;
			if (0 == chunkToAlloc || 0==chunks[chunkToAlloc-1]->numOfLeftBlock) {//chunkToAlloc未命中
				size_t i = 0;
				for (; i<chunksNow; i++) { //遍历寻找满足的chunk
					//std::cout << "寻找可分配的chunk" << std::endl;
					result = chunks[i]->chunkallocate(blockSize);
					
					if (result) {
						chunkToAlloc = i+1;
						//std::cout << "找到了  " << (int)i<< std::endl;
						return result;
					}
					//std::cout << "meizhaodao" << std::endl;
				}
				chunkToAlloc = i+1;  //所有chunk都空了，创建新的chunk
				
				//std::cout <<(int)i << std::endl;
				return (chunk_add());

			}
			result = chunks[chunkToAlloc - 1]->chunkallocate(blockSize);  //chunkToAlloc命中，直接从这个chunk里取
			return (result);
		}

		//chunks数组里new一个新的chunk，如果当前为chunks分配的空间不足，就进行扩容
		template<bool threads, int inst>
		void* __qyk_loki_fixed_alloc<threads, inst>::chunk_add() {
			//std::cout << "chunk_add" << std::endl;
			if (chunksNow == maxChunksNow) { //如果现在chunks数组已经满了，就需要对chunks扩容
				chunks_reset();
			}
			chunksNow++; //现有的chunk数加一
			//std::cout << "new end" << std::endl;
			chunks[chunksNow - 1] = new Chunk(blockSize); //new新的chunk
			//std::cout << "new end" << std::endl;
			return chunks[chunksNow - 1]->chunkallocate(blockSize); 
		}

		//为chunks【】申请新的空间，并将原来的数据转移过来
		template<bool threads, int inst>
		void __qyk_loki_fixed_alloc<threads, inst>::chunks_reset() {
			//std::cout << "chunk_reset" << std::endl;
			maxChunksNow = maxChunksNow << 1; //每次扩容时，容量变为原来的两倍
			Chunk** temp = chunks;
			chunks = (Chunk**)::malloc(maxChunksNow * sizeof(Chunk*)); //新的chunks数组
			for (int i = 0; i < chunksNow; i++) {
				chunks[i] = temp[i]; //将原来的数组里放的指针转移过来
			}
			::free(temp); //释放原来的chunks数组的空间
			//std::cout << "end of chunk_reset" << std::endl;
		}


		//fixed_alloc的dealloc,chunkToDealloc是上次回收内存的chunnk的编号，由于数据集中，先从这个chunk开始往两边
		//寻找，找到一个chunk使回收的指针指向的地址包含在这个chunk中，调用这个chunk的dealloc进行回收
		//回收完毕后检查是否整个chunk都回收了，如果都回收了，则判断是否需要释放，为避免频繁的地址申请与释放，设置一个chunk作为缓存
		//当有两个chunk都全回收了之后菜释放一个chunk
		template<bool threads, int inst>
		void __qyk_loki_fixed_alloc<threads, inst>::deallocate(void* ptr) {
			//std::cout << "fixed_dealloc" << std::endl;
			size_t low = chunkToDealloc;
			size_t lowbond = 0;
			size_t hi = low + 1;
			size_t hibond = chunksNow;
			
			while(true){//从chunksToDealloc往两边遍历，更新chunksToDealloc
				if (low > lowbond && (chunks[low - 1]->pData) <= (char*)ptr && (chunks[low - 1]->pData) + blockSize * MAX_BLOCK_SIZE > (char*)ptr) {
					chunkToDealloc = low;
					break;
				}
				low = (--low >= 0) ? low : 0;
				if (hi <= hibond && (chunks[hi - 1]->pData) <= (char*)ptr && (chunks[hi - 1]->pData) + blockSize * MAX_BLOCK_SIZE > (char*)ptr) {
					chunkToDealloc = hi;
					break;
				}
				hi++;
				if (low <= lowbond && hi > hibond) { //所有的chunk内都不包含这一块地址，说明这一块地址不是loki_alloc分配的
					std::cerr << "这块地址不是油loki_allocator分配的，不应使用loki_deallocate" << std::endl;
					exit(1);
				}
			}
			//std::cout << chunkToDealloc << std::endl;
			chunks[chunkToDealloc-1]->chunkdeallocate(ptr,blockSize);  //调用找到的chunk的chunkdeallocate
			if (chunks[chunkToDealloc - 1]->numOfLeftBlock == MAX_BLOCK_SIZE) {
				//当现在回收的chunk全部回收完毕，把这个chunk换到最后面当作释放缓存
				if (chunks[chunksNow-1]->numOfLeftBlock== MAX_BLOCK_SIZE) {//如果chunks【】的最后一个chunk已经全空了，就释放最后一个chunk
					chunk_free();
				}
				chunk_swap(chunkToDealloc);
			}
			//std::cout << "fixed_dealloc return" << std::endl;
		}

		//chunk的释放，由于将chunks中最后一个已构造的chunk作为缓存，每一次都是释放chunks[chunksNow-1]；
		template<bool threads, int inst>
		void __qyk_loki_fixed_alloc<threads, inst>::chunk_free() {
			//std::cout << "chunk_free" << std::endl;
			chunks[chunksNow - 1]->~Chunk();
			chunksNow--;
		}

		//交换两个chunk，将全回收的那个放到最后
		template<bool threads, int inst>
		void __qyk_loki_fixed_alloc<threads, inst>::chunk_swap(size_t chunkToDealloc) {
			//std::cout << "chunk_swap" << std::endl;
			if (chunkToDealloc == chunksNow) return;
			Chunk* temp = chunks[chunkToDealloc - 1];
			chunks[chunkToDealloc - 1] = chunks[chunksNow - 1];
			chunks[chunksNow - 1] = temp;
		}


		//第三级deallocate，，这块chunk的可分配block+1，将回收的这块block在chunk里的编号作为下一次分配的bolck，并且在这块block里记录原来的firstToAlloc
		template<bool threads, int inst>
		void __qyk_loki_fixed_alloc<threads, inst>::Chunk::chunkdeallocate(void* ptr, const size_t blockSize) {
			//std::cout << "chunk_dealloc" << std::endl;
			numOfLeftBlock++;
			char* temp = (char*)ptr;
			*temp = (char)firstToAlloc;
			firstToAlloc = (temp - pData) / blockSize ;
			//std::cout << "chunk_dealloc  return" << std::endl;

		}



		template<bool threads, int inst> //threads用来表示是单线程还是多线程 非型别参数inst并没有使用 后面分别定义为false和0
		class __qyk_loki_alloc {
			static constexpr int MAX_LOKI_SIZE = 128;
			static constexpr int MIN_LOKI_SIZE = 8;
			static constexpr int SIZE_OF_LOKI = MAX_LOKI_SIZE / MIN_LOKI_SIZE;

			static void(*myHandler)();  //这是一个函数指针，仿new handler，用于处理异常

			typedef __qyk_loki_fixed_alloc<threads, inst>* fixedAlloc;
			static fixedAlloc fixedAllocs[SIZE_OF_LOKI];

			static inline size_t round_up(size_t n) {
				return ((n + MIN_LOKI_SIZE - 1) & ~(MIN_LOKI_SIZE - 1));
			}


			static inline size_t find_fix_num(size_t n) {
				return ((n + MIN_LOKI_SIZE - 1) / MIN_LOKI_SIZE - 1);
			}

		public:
			static void* allocate(size_t n);

			static void deallocate(void* ptr, size_t n);

			static void* oom_malloc(size_t needSize);

			typedef void(*H)();
			static H setNewHandler(H h) { //设置新的handler函数指针，返回原来的handler指针
				H old = myHandler;
				myHandler = h;
				return (old);
			}

		};

		


		template<bool threads, int inst>
		typename __qyk_loki_alloc<threads, inst>::fixedAlloc  __qyk_loki_alloc<threads, inst>::fixedAllocs[__qyk_loki_alloc<threads, inst>::SIZE_OF_LOKI] =
		{
			nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
		};

		template<bool threads, int inst>
		void(*__qyk_loki_alloc<threads, inst>::myHandler)() = nullptr;

		template<bool threads, int inst>
		void* __qyk_loki_alloc<threads, inst>::allocate(size_t n) {
			if (n > MAX_LOKI_SIZE) return (::malloc(n));
			size_t num = find_fix_num(n);
			if (nullptr == fixedAllocs[num])
			{
				fixedAllocs[num] = new __qyk_loki_fixed_alloc<threads, inst>(round_up(n));
			}
			return (fixedAllocs[num]->allocate());
		}

		template<bool threads, int inst>
		void __qyk_loki_alloc<threads, inst>::deallocate(void* ptr,size_t n) {
			//std::cout << "loki_dealloc" << std::endl;
			if (n > MAX_LOKI_SIZE) {
				::free(ptr);
				return;
			}
			size_t num = find_fix_num(n);
			fixedAllocs[num]->deallocate(ptr);
			//std::cout << "loki_dealloc return " << std::endl;
		}

		//oom_malloc
		template<bool threads, int inst>
		void* __qyk_loki_alloc<threads, inst>::oom_malloc(size_t n) {
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

		template<bool threads, int inst>
		__qyk_loki_fixed_alloc<threads, inst>::Chunk::Chunk(size_t blockSize) : numOfLeftBlock(MAX_BLOCK_SIZE), firstToAlloc(0) {
			//std::cout << "构造chunk" << std::endl;
			pData = (char*)::malloc(blockSize * MAX_BLOCK_SIZE);
			if (!pData) {
				pData = (char*)__qyk_loki_alloc<threads, inst>::oom_malloc(blockSize * MAX_BLOCK_SIZE);
			}
			char* temp = pData;
			for (int i = 1; i <= MAX_BLOCK_SIZE; i++) {
				*temp = i;
				temp = (temp + blockSize);
			}
		}

		template<bool threads, int inst>
		__qyk_loki_fixed_alloc<threads, inst>::__qyk_loki_fixed_alloc(size_t blockSize) :
			blockSize(blockSize), maxChunksNow(2), chunksNow(1), chunkToAlloc(0), chunkToDealloc(0)
			//初始化一个fixed_alloc时，构造一个chunk，只为chunks数组申请两个chunk指针的空间，等待后续扩容
		{
			chunks = (Chunk**)::malloc(maxChunksNow * sizeof(Chunk*));
			if (!chunks) {
				chunks = (Chunk**)__qyk_loki_alloc<threads, inst>::oom_malloc(maxChunksNow * sizeof(Chunk*));
			}
			chunks[0] = new Chunk(blockSize);
		}

		
	}//end of detail

	typedef detail::__qyk_loki_alloc<false, 0> loki_alloc;

}


#endif // !QYK_LOKI_ALLOCATOR

