// deque的结构是很多固定大小的空间buffer，它们连续存放元素T，每个vector都有一个指针指向它，
// 这些指针node被按顺序存放在一块连续空间map里，以此实现元素连续存放的表象

// deque的迭代器里含有一个指向存放的元素的指针、一个指向当前buffer起点的指针、一个指向当前buffer在map中的位置的指针
// 在对迭代器移动时，先根据当前buffer的起点和移动的距离得到需要跳过的buffer数量，在map中找到目标buffer，然后跳到目标位置处

// 应当有一个全局的函数来得知deque存放这个种类的对象时，希望一个buffer内存放多少个这种对象

#ifndef QYK_DEQUE
#define QYK_DEQUE

#include "02iterator.h"
#include "01qyk_allocator.h"
#include "02uninitialized.h"
#include "02type_traits.h"

namespace qyk
{
    namespace detail
    {

        /* 全局函数: 用于辅助决定缓冲区大小
         * 如果buf_size不为0，传回由用户定义的buf_size，表示一个缓冲区可以容纳 buf_size 个元素
         * 如果 buf_size 为0，表示 buffer size 使用默认值，那么：
         * 	 如果 val_size(sizeof(value_type))小于512，返回 512 / val_size，一个缓冲区可容纳 512 / val_size 个元素
         * 	 如果 val_size >= 512，返回 1，一个缓冲区可容纳 1 个元素
         */
        inline size_t __deque_buf_size(size_t bufSize, size_t valSize)
        {
            return (bufSize != 0) ? bufSize
                                  : ((valSize < 512) ? size_t(512 / valSize) : size_t(1));
        }

        // deque的迭代器
        template <class T, class ref, class ptr, size_t BufSize>
        class deque_iterator
        {
        public:
            static size_t buffer_size() { return __deque_buf_size(BufSize, sizeof(T)); }

            using iterator = deque_iterator<T, T &, T *, BufSize>;
            using const_iterator = deque_iterator<T, T &, T *, BufSize>;

            using iterator_category = random_access_iterator_tag;
            using value_type = T;
            using pointer = ptr;
            using const_pointer = const ptr;
            using reference = ref;
            using const_reference = const ref;
            using difference_type = ptrdiff_t;
            using size_type = size_t;

            using map_pointer = T **;
            using self = deque_iterator;

            T *cur;
            T *first;
            T *last;
            T **node;

            // 解迭代器
            reference operator*() const { return *cur; }
            pointer operator->() const { return cur; }

            // 跳转到另一个buffer
            void set_node(map_pointer newNode)
            {
                node = newNode;
                first = *newNode;
                last = first + difference_type(buffer_size());
            }

            // 两个迭代器之间的距离
            difference_type operator-(const self &other) const
            {
                return (node - other.node) * buffer_size() + (cur - first) - (other.cur - other.first);
            }

            // 自增 前置++
            self &operator++()
            {
                ++cur; // 切换至下一个元素
                if (cur == last)
                {                       // 如果已经到达缓冲区的尾端（最后一个元素的下一位置）
                    set_node(node + 1); // 就切换至下一个缓冲区节点的第一个元素
                    cur = first;
                }
                return *this;
            }
            // 自增 后置++
            self operator++(int)
            {
                self tmp = *this;
                ++*this; // 调用前置++
                return tmp;
            }

            // 自减 前置--
            self &operator--()
            {
                if (cur == first)
                {                       // 如果已经到达缓冲区的头端
                    set_node(node - 1); // 就切换至前一个缓冲区节点
                    cur = last;         // cur 指向最后一个元素的下一位置
                }
                --cur; // 切换至前一个元素
                return *this;
            }
            // 自减 后置--
            self operator--(int)
            {
                self tmp = *this;
                --*this; // 调用前置--
                return tmp;
            }

            //+=
            self &operator+=(difference_type n)
            {
                difference_type disn = n + (cur - first);
                if (disn >= 0)
                {
                    set_node(node + disn / difference_type(buffer_size()));
                    cur = first + disn % buffer_size();
                }
                else
                {
                    set_node(node - ((-disn - 1) / difference_type(buffer_size()) + 1));
                    cur = first + difference_type(buffer_size()) - (-disn - 1) % buffer_size() - 1;
                }
                return *this;
            }

            //+=
            self &operator-=(difference_type n)
            {
                *this += -n;
                return *this;
            }

            //+ ，不改变迭代器自身
            self operator+(difference_type n) const
            {
                self tmp = *this;
                return tmp += n;
            }
            //- ，不改变迭代器自身
            self operator-(difference_type n) const
            {
                self tmp = *this;
                return tmp -= n;
            }

            // 实现随机访问，
            reference operator[](difference_type n) const { return *(*this + n); }

            bool operator==(const self &x) const { return cur == x.cur; }
            bool operator!=(const self &x) const { return cur != x.cur; }
            bool operator<(const self &x) const
            {
                return (node == x.node) ? (cur < x.cur) : (node < x.node);
            }
            bool operator>(const self &x) const
            {
                return x < *this;
            }
        };

    } // end of detail

    template <class T, size_t BufSize = 0, class Alloc = alloc>
    class deque
    {
    public:
        using iterator = typename detail::deque_iterator<T, T &, T *, BufSize>::iterator;
        using const_iterator = typename detail::deque_iterator<T, T &, T *, BufSize>::const_iterator;

        using category = typename detail::deque_iterator<T, T &, T *, BufSize>::iterator_category;
        using value_type = typename detail::deque_iterator<T, T &, T *, BufSize>::value_type;
        using pointer = typename detail::deque_iterator<T, T &, T *, BufSize>::pointer;
        using const_pointer = typename detail::deque_iterator<T, T &, T *, BufSize>::const_pointer;
        using reference = typename detail::deque_iterator<T, T &, T *, BufSize>::reference;
        using const_reference = typename detail::deque_iterator<T, T &, T *, BufSize>::const_reference;
        using difference_type = typename detail::deque_iterator<T, T &, T *, BufSize>::difference_type;
        using size_type = typename detail::deque_iterator<T, T &, T *, BufSize>::size_type;

    private:
        using map_pointer = T **;
        iterator start;
        iterator finish;
        map_pointer map;
        size_type mapSize;

        using buff_allocator = allocator<T, Alloc>;
        using map_allocator = allocator<pointer, Alloc>;

        size_t (*buffer_size)() = detail::deque_iterator<T, T &, T *, BufSize>::buffer_size;

        // 创建一个含有n个元素的deque
        void creat_deque_n(size_t n = 0)
        {
            size_t bufferNeedeD = n / buffer_size() + 1;
            // 除了存放元素的buffer外，还需要头尾两个buffer作为缓存，
            mapSize = bufferNeedeD + 2;
            // 申请map空间
            map = map_allocator::allocate(mapSize);
            for (auto i = map + 1; i != map + bufferNeedeD + 1; ++i)
            {
                // 申请buffer空间，并传入map
                *i = buff_allocator::allocate(buffer_size());
            }
            start.set_node(map + 1);
            start.cur = start.first;
            finish.set_node(map + bufferNeedeD);
            finish.cur = finish.first + n % buffer_size();
        }

        // 重设map，只有需要插入元素的时候才会调用，ture表示需要在前端插入时申请，false为后端插入时申请
        // 如果此时map占用不超过可用空间的一半，则说明实际空间是足够的，只是数据都挤在一端，需要重新分配
        // 否则需要申请一个新的更大的map
        void reset_map_front(bool flag, size_type nodeToAdd)
        {
            size_type oldNumNode = finish.node - start.node + 1;
            size_type newNumNode = oldNumNode + nodeToAdd;
            map_pointer newstart;
            if (mapSize > 2 * newNumNode)
            { // 增加buffer后map占用不超过一半
                newstart = map + (mapSize - newNumNode) / 2 + (flag ? nodeToAdd : 0);
                // 上面的目的是使重新分配后的map使用的node在map的中间
                if (newstart < start.node)
                { // 新的buffer起点在原来的前面，则将node信息前移
                    move(start.node, finish.node, newstart);
                }
                else
                { // 否则后移
                    move_back(start.node, finish.node, newstart);
                }
            }
            else
            { // map占用超过一半，需要申请更大的map
                map_pointer newMap = map_allocator::allocate(2 * newNumNode);
                newstart = newMap + (2 * newNumNode - newNumNode) / 2 + (flag ? nodeToAdd : 0);
                move(start.node, finish.node + 1, newstart);
                map_allocator::deallocate(map);
                map = newMap;
                mapSize = 2 * newNumNode;
            }
            start.set_node(newstart);
            finish.set_node(newstart + oldNumNode - 1);
        }

    public:
        // 默认构造
        deque()
        {
            creat_deque_n();
        }

        // 填充构造
        deque(const size_type n, const value_type &value)
        {
            creat_deque_n(n);
            uninitialized_fill(start, finish, value);
        }

        // 默认值构造
        deque(const size_type n)
        {
            creat_deque_n(n);
            uninitialized_fill(start, finish, 0);
        }

        // 范围构造
        template <class InputIterator, typename =
                                           typename my_enable_if<is_iterator<InputIterator>::value>::type>
        deque(InputIterator first, InputIterator last)
        {
            size_t dis = qykDistance(first, last);
            creat_deque_n(dis);
            uninitialized_copy(first, last, start);
        }

        // 拷贝构造
        deque(const deque<T, BufSize, Alloc> &other)
        {
            size_t size = other.size();
            creat_deque_n(size);
            uninitialized_copy(other.begin(), other.end(), start);
        }

        // 拷贝构造
        deque(deque<T, BufSize, Alloc> &&other) : start(other.start), finish(other.finish), map(other.map), mapSize(other.mapSize)
        {
            other.creat_deque_n();
        }

        // 析构函数
        ~deque()
        {
            qykDestroy(start, finish);
            map_pointer temp = start.node;
            for (auto i = start.node; i <= finish.node; ++i)
            {
                buff_allocator::deallocate(*i, buffer_size());
            }
            map_allocator::deallocate(map, mapSize);
        }

        bool empty() const { return start == finish; }
        iterator begin() { return start; }
        const_iterator begin() const { return start; }
        const_iterator cbegin() const { return start; }
        iterator end() { return finish; }
        const_iterator end() const { return finish; }
        const_iterator cend() const { return finish; }
        size_t size() const { return size_t(finish - start); }
        reference front() { return *start; }
        const_reference front() const { return *start; }
        reference back()
        {
            iterator temp = finish;
            --temp;
            return *temp;
        }
        const_reference back() const
        {
            iterator temp = finish;
            --temp;
            return *temp;
        }
        reference operator[](size_type n)
        {
            return start[distance_type(n)]; // 调用iterator的operator[]
        }

        // pop_front
        void pop_front()
        {
            if (start.cur == start.last - 1)
            { // 如果当前buffer只有一个元素，则将其析构，然后释放buffer,更新start
                qykDestroy(start.cur);
                buff_allocator::deallocate(start.first);
                start.set_node(start.node + 1);
                start.cur = start.first;
            }
            else
            {
                qykDestroy(start.cur++);
            }
        }

        // push_front
        void push_front(const value_type &value);

        // emplace_frone
        template <class... Argus>
        void emplace_front(Argus &&...argus);

        // pop_back
        void pop_back()
        {
            if (finish.cur == finish.first)
            { // 如果当前buffer为空,释放buffer,更新finish
                buff_allocator::deallocate(finish.first);
                finish.set_node(finish.node - 1);
                finish.cur = finish.last - 1;
                qykDestroy(finish.cur);
            }
            else
            {
                qykDestroy(--finish.cur);
            }
        }

        // push_back
        void push_back(const value_type &value);

        // emplace_back
        template <class... Argus>
        void emplace_back(Argus &&...argus);

        // clear
        void clear();

        // insert
        iterator insert(iterator pod, const value_type &value);
        iterator insert(iterator pod, const size_t n, const value_type &value);
        template <class InputIterator, typename = typename my_enable_if<is_iterator<InputIterator>::value>::type>
        iterator insert(iterator pod, InputIterator first, InputIterator last);

        // erase 删除pod处的元素
        iterator erase(iterator pod);
        iterator erase(iterator first, iterator last);

    }; // end of deque

    // clear 需要将管理的元素释放，申请的buffer释放，但是需要保留一块buffer，这就是deque的初始状态
    template <class T, size_t BufSize, class Alloc>
    void deque<T, BufSize, Alloc>::clear()
    {
        // 处理除头尾外的buffer，这些是满的
        for (auto i = start.node + 1; i != finish.node; i++)
        {
            qykDestroy(i.first, i.last);
            buff_allocator::deallocate(i.first, buffer_size());
        }
        if (start.node != finish.node) // 如果头尾不在同一块buffer中，则分别析构然后释放尾部的buffer
        {
            qykDestroy(start.cur, start.last);
            qykDestroy(finish.first, finish.cur);
            buff_allocator::deallocate(finish.first, buffer_size());
        }
        else
        {
            qykDestroy(start.cur, finish.cur);
        }
        start.cur = start.first; // 更新状态，但是没有释放map，mapSize没变
        finish = start;
    }

    // push_front
    template <class T, size_t BufSize, class Alloc>
    void deque<T, BufSize, Alloc>::push_front(const value_type &value)
    {
        if (start.cur != start.first)
        { // 当前buffer还有空余
            construct(--start.cur, value);
            return;
        }
        else if (map != start.node)
        {                                                                // 当前map还有空余位置可以挂载新的buffer
            *(start.node - 1) = buff_allocator::allocate(buffer_size()); // 在start前面申请新的buffer
            start.set_node(start.node - 1);
            start.cur = start.last - 1;
            construct(start.cur, value);
            return;
        }
        else
        { // 整个deque为前端留的空间已经用完，需要重设map
            reset_map_front(true, 1);
            push_front(value);
        }
    }

    template <class T, size_t BufSize, class Alloc>
    template <class... Argus>
    void deque<T, BufSize, Alloc>::emplace_front(Argus &&...argus)
    {
        if (start.cur != start.first)
        { // 当前buffer还有空余
            construct(--start.cur, std::forward<Argus>(argus)...);
            return;
        }
        else if (map != start.node)
        {                                                                // 当前map还有空余位置可以挂载新的buffer
            *(start.node - 1) = buff_allocator::allocate(buffer_size()); // 在start前面申请新的buffer
            start.set_node(start.node - 1);
            start.cur = start.last - 1;
            construct(start.cur, std::forward<Argus>(argus)...);
            return;
        }
        else
        { // 整个deque为前端留的空间已经用完，需要重设map
            reset_map_front(true, 1);
            emplace_front(std::forward<Argus>(argus)...);
        }
    }

    // push_back
    template <class T, size_t BufSize, class Alloc>
    void deque<T, BufSize, Alloc>::push_back(const value_type &value)
    {
        if (finish.cur != finish.last - 1)
        { // 当前buffer还有空余
            construct(finish.cur++, value);
            return;
        }
        else if (map + mapSize - 1 != finish.node)
        {                                                                 // 当前map还有空余位置可以挂载新的buffer
            *(finish.node + 1) = buff_allocator::allocate(buffer_size()); // 在finish后面申请新的buffer
            construct(finish.cur, value);
            finish.set_node(finish.node + 1);
            finish.cur = finish.first;
            return;
        }
        else
        { // 整个deque为尾端留的空间已经用完，需要重设map
            reset_map_front(false, 1);
            push_back(value);
        }
    }

    // emplace_back
    template <class T, size_t BufSize, class Alloc>
    template <class... Argus>
    void deque<T, BufSize, Alloc>::emplace_back(Argus &&...argus)
    {
        {
            if (finish.cur != finish.last - 1)
            { // 当前buffer还有空余
                construct(finish.cur++, std::forward<Argus>(argus)...);
                return;
            }
            else if (map + mapSize - 1 != finish.node)
            {                                                                 // 当前map还有空余位置可以挂载新的buffer
                *(finish.node + 1) = buff_allocator::allocate(buffer_size()); // 在finish后面申请新的buffer
                construct(finish.cur, std::forward<Argus>(argus)...);
                finish.set_node(finish.node + 1);
                finish.cur = finish.first;
                return;
            }
            else
            { // 整个deque为尾端留的空间已经用完，需要重设map
                reset_map_front(false, 1);
                emplace_back(std::forward<Argus>(argus)...);
            }
        }
    }

    // insert(iterator pod,const value_type& value) 在pod处插入一个元素value
    template <class T, size_t BufSize, class Alloc>
    typename deque<T, BufSize, Alloc>::iterator
    deque<T, BufSize, Alloc>::insert(iterator pod, const value_type &value)
    {
        if (pod == start)
        { // 在开头插入，调用emplace_front
            emplace_front(value);
            return start + 1;
        }
        if (pod == finish)
        { // 在最后插入，调用emplace_back
            emplace_back(value);
            return finish;
        }

        // 在中间插入
        size_t npod = pod - start; // 插入位置前面的元素个数

        if (npod < finish - pod)
        { // 前面的元素个数少于后面的
            emplace_front();
            move(start + 1, start + npod + 1, start); // 插入位置前的元素前移
        }
        else
        { // 前面的元素个数多于后面的
            emplace_back();
            move_back(start + npod, finish, start + npod + 1); // 插入位置后的元素后移
        }
        construct((start + npod).cur, value);
        return start + npod + 1;
    }

    // 在pod处插入n个value
    template <class T, size_t BufSize, class Alloc>
    typename deque<T, BufSize, Alloc>::iterator
    deque<T, BufSize, Alloc>::insert(iterator pod, const size_t n, const value_type &value)
    {
        size_t npod = pod - start;
        size_t npod_back = finish - start;

        if (npod < finish - pod)
        {                                               // 将原来的数据往前移
            size_t freeNode = size_t(start.node - map); // 插入前start.node前面的可用节点数
            size_t addNode = (n - (start.cur - start.first)) / buffer_size() + 1;

            if (freeNode < addNode)
                reset_map_front(true, addNode); // 节点不够则重构map

            for (auto i = addNode; i > 0; i--)
            {
                *(start.node - i) = buff_allocator::allocate(buffer_size()); // 申请足够的buffer存放新加入的元素
            }
            start = start - n;
            move(start + n, start + n + npod, start);
        }
        else
        {                                                                // 将原来的数据往后移
            size_t freeNode = size_t(mapSize - (finish.node - map + 1)); // 插入前start.node前面的可用节点数
            size_t addNode = (n - (finish.last - finish.cur)) / buffer_size() + 1;

            if (freeNode < addNode)
                reset_map_front(false, addNode); // 节点不够则重构map

            for (auto i = addNode; i > 0; i--)
            {
                *(finish.node + i) = buff_allocator::allocate(buffer_size()); // 申请足够的buffer存放新加入的元素
            }

            finish = finish + n;

            move_back(finish - n - npod_back, finish - n, finish - npod_back);
        }
        uninitialized_fill(start + npod, start + n + npod, value);
        return start + n + npod + 1;
    }

    // insert(iterator pod, InputIterator first, InputIterator last),用迭代器在pod处插入
    template <class T, size_t BufSize, class Alloc>
    template <class InputIterator, typename>
    typename deque<T, BufSize, Alloc>::iterator
    deque<T, BufSize, Alloc>::insert(iterator pod, InputIterator first, InputIterator last)
    {
        size_t npod = pod - start;
        size_t npodBack = finish - start;
        size_t n = qykDistance(first, last);

        if (npod < finish - pod)
        {                                               // 将原来的数据往前移
            size_t freeNode = size_t(start.node - map); // 插入前start.node前面的可用节点数
            size_t addNode = (n - (start.cur - start.first)) / buffer_size() + 1;

            if (freeNode < addNode)
                reset_map_front(true, addNode); // 节点不够则重构map

            for (auto i = addNode; i > 0; i--)
            {
                *(start.node - i) = buff_allocator::allocate(buffer_size()); // 申请足够的buffer存放新加入的元素
            }
            start = start - n;
            move(start + n, start + n + npod, start);
        }
        else
        {                                                                // 将原来的数据往后移
            size_t freeNode = size_t(mapSize - (finish.node - map + 1)); // 插入前start.node前面的可用节点数
            size_t addNode = (n - (finish.last - finish.cur)) / buffer_size() + 1;

            if (freeNode < addNode)
                reset_map_front(false, addNode); // 节点不够则重构map

            for (auto i = addNode; i > 0; i--)
            {
                *(finish.node + i) = buff_allocator::allocate(buffer_size()); // 申请足够的buffer存放新加入的元素
            }

            finish = finish + n;

            move_back(finish - n - npodBack, finish - n, finish - npodBack);
        }
        uninitialized_copy(first, last, start + npod);
        return start + n + npod + 1;
    }

    // erase(iterator pod)移除pod处的元素
    template <class T, size_t BufSize, class Alloc>
    typename deque<T, BufSize, Alloc>::iterator
    deque<T, BufSize, Alloc>::erase(iterator pod)
    {
        size_t npod = pod - start;
        if (npod < (finish - pod))
        {
            move_back(start, pod, pod + 1);
            pop_front();
        }
        else
        {
            copy(pod+1, finish, pod);
            pop_back();
        }
        return start + npod;
    }

    // 删除 [first, last) 内的所有元素
    template <class T, size_t BufSize, class Alloc>
    typename deque<T, BufSize, Alloc>::iterator
    deque<T, BufSize, Alloc>::erase(iterator first, iterator last)
    {
        size_t npod = first - start;
        size_t npodBack = finish - last;
        size_t dis=last-first;
        if (npod < npodBack)
        {
            move_backward(start, first, last);
            iterator new_start = start + dis;
            qykDestroy(start, new_start);
            for (map_pointer cur = start.node; cur < new_start.node; ++cur)
					buff_allocator::deallocate(*cur, buffer_size());
			start = new_start;
        }
        else {			
            copy(last, finish, first);		
            iterator new_finish = finish - dis;		
            qykDestroy(new_finish, finish);			
            for (map_pointer cur = new_finish.node + 1; cur <= finish.node; ++cur)
                buff_allocator::deallocate(*cur, buffer_size());
            finish = new_finish;
        }
        return start + dis;
    }

} // end of qyk

#endif
// end of QYK_DEQUE