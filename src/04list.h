#ifndef QYK_LIST
#define QYK_LIST

#include <iostream>
#include "01qyk_allocator.h"
//#include "src/02alogbase.h"
#include "02construct.h"
#include "02iterator.h"
#include "02type_traits.h"
//#include "src/02uninitialized.h"
#include <cstddef>

namespace qyk
{
    namespace detail
    {
        // list的节点
        template <class T>
        struct list_node
        {
            T data;
            using nodePointer = list_node<T> *;
            nodePointer next;
            nodePointer prev;
        };

        // list的迭代器,管理一个指向list_node的指针
        template <class T>
        class list_iterator
        {
        public:
            using nodePointer = list_node<T> *;
            using iterator = list_iterator<T>;
            using const_iterator = const list_iterator<T>;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using const_pointer = const T *;
            using const_reeference = const T &;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using iterator_category = bidirectional_iterator_tag;

            nodePointer node; // 指向list_node的指针

            // 构造函数
            list_iterator() : node(nullptr) {}
            // 传入一个节点
            list_iterator(const nodePointer &n) : node(n) {}
            // 拷贝构造
            list_iterator(const iterator &other) : node(other.node) {}

            // 解指针操作
            reference operator*() const
            {
                return (*node).data;
            }
            // 重载->
            pointer operator->() const
            {
                return &(operator*());
            }
            // 重载==
            bool operator==(const iterator &other) const
            {
                return node == other.node;
            }
            // 重载!=
            bool operator!=(const iterator &other) const
            {
                return node != other.node;
            }
            // 重载前置++
            iterator &operator++()
            {
                node = node->next;
                return *this;
            }
            // 重载后置++
            iterator operator++(int)
            {
                auto temp = *this;
                ++*this;
                return temp;
            }
            // 重载前置--
            iterator &operator--()
            {
                node = node->prev;
                return *this;
            }
            // 重载后置--
            iterator operator--(int)
            {
                auto temp = *this;
                --*this;
                return temp;
            }
        }; // end of list_iterator

    } // end of detail



    template <class T, class Alloc = alloc>
    class list
    {
    public:
        using nodePointer = typename detail::list_iterator<T>::nodePointer;
        using iterator = typename detail::list_iterator<T>::iterator;
        using const_iterator = typename detail::list_iterator<T>::const_iterator;
        using value_type = typename detail::list_iterator<T>::value_type;
        using pointer = typename detail::list_iterator<T>::pointer;
        using reference = typename detail::list_iterator<T>::reference;
        using const_pointer = typename detail::list_iterator<T>::const_pointer;
        using const_reeference = typename detail::list_iterator<T>::const_reeference;
        using size_type = typename detail::list_iterator<T>::size_type;

    private:
        using list_node = typename detail::list_node<T>;
        // list包含一个指向list_node的指针，这个指针指向链表最后一个元素的下一个位置，它的next是链表的起点
        size_type size_;
        nodePointer node;
        using node_allocator = allocator<list_node, Alloc>;

        // 获取一个list_node的空间
        nodePointer get_node()
        {
            return node_allocator::allocate();
        }

        // 创建一个list_node，根据传入的const T为list_node赋值
        nodePointer construct_node(const T &value)
        {
            nodePointer p = get_node();
            construct(&p->data, value);
            return p;
        }

        // 析构目标list_node的数据，并回收list_node
        void destroy_node(nodePointer p)
        {
            destroy(&(p->data));
            free_node(p);
        }

        // 回收list_node
        void free_node(nodePointer p)
        {
            node_allocator::deallocate(p);
        }

        // 创建一个空的list
        void empty_list()
        {
            node = get_node();
            node->next = node;
            node->prev = node;
            size_ = 0;
        }

    public:
        // 构造函数，默认构造
        list() { empty_list(); }

        // 构造一个含有n个value的list
        list(size_type n, const T &value)
        {
            empty_list();
            auto cue = node;
            for (auto i = n; i > 0; i--)
            {
                cue->next = construct_node(value);
                (cue->next)->prev = cue;
                cue = cue->next;
            }
            cue->next = node;
            node->prev = cue;
            size_ = n;
        }

        // 通过迭代器构造
        template <class InputIterator, typename = typename my_enable_if<is_iterator<InputIterator>::value>::type>
        list(InputIterator first, InputIterator last)
        {
            empty_list();
            insert(begin(), first, last);
        }

        // 拷贝构造
        list(const list<T> &other)
        {
            empty_list();
            insert(begin(),other.begin(), other.end());
        }

        // 移动构造
        list(list<T> &&other) noexcept : node(other.node), size_(other.size_)
        {
            other.node = get_node();
            other.node->next = other.node;
            other.node->prev = other.node;
            other.size_ = 0;
        }

        // 析构函数
        ~list()
        {
            clear();
            free_node(node);
        }

        // begin()
        iterator begin() { return (*node).next; }
        const_iterator begin() const { return (*node).next; }

        // end()
        iterator end() { return node; }
        const_iterator end() const { return node; }

        // size()
        size_type size() const { return size_; }

        // clear(),清空所有元素
        void clear()
        {
            auto cue = node->next;
            while (cue != node)
            {
                auto temp = cue;
                cue = cue->next;
                destroy_node(temp);
            }
            node->next = node;
            node->prev = node;
            size_ = 0;
        }

        reference front() const { return *begin(); }
        reference back() const { return (node->prev)->data; }
        void push_front(const T &value) { insert(begin(), value); }
        void push_back(const T &value) { insert(end(), value); }
        void pop_front()
        {
            if (size_)
            {
                auto temp = node->next;
                node->next = temp->next;
                temp->next->prev = node;
                destroy_node(temp);
                --size_;
            }
        }
        void pop_back()
        {
            if (size_)
            {
                auto temp = node->prev;
                node->prev = temp->prev;
                temp->prev->next = node;
                destroy_node(temp);
                --size_;
            }
        }

        // 在pod处插入一个value
        void insert(iterator pod, const T &value) { insert(pod, 1, value); }

        // 在pod处插入n个value
        void insert(iterator pod, size_type n, const T &value)
        {
            list<T> temp(n, value);
            splice(pod, temp);
        }

        // 将一个链表合并到pod处
        void splice(iterator pod, list<T> &other)
        {
            pod.node->prev->next = other.begin().node;
            (other.begin().node)->prev = pod.node->prev;
            
            pod.node->prev = other.end().node->prev;
            other.end().node->prev->next = pod.node;
            
            size_ += other.size();
            other.node->next = other.node;
            other.node->prev = other.node;
            other.size_ = 0;
        }

        // 在pod处插入first到last之间的内容（复制）
        template <class InputIterator, typename = typename my_enable_if<is_iterator<InputIterator>::value>::type>
        void insert(iterator pod, InputIterator first, InputIterator last)
        {
            size_type n = distance(first, last);
            size_ += n;
            nodePointer cue = pod.node->prev;
            for (; n > 0; n--)
            {
                cue->next = construct_node(*first);
                cue->next->prev = node;
                cue = cue->next;
            }
            cue->next = pod.node;
            pod.node->prev = cue;
        }

        // erase 移除单个元素
        iterator erase(iterator pod)
        {
            pod.node->prev->next = pod.node->next;
            pod.node->next->prev = pod.node->prev;
            pod++;
            destroy_node(pod.node->prev);
            size_--;
            return pod;
        }
        // erase 移除范围元素
        iterator erase(iterator first, iterator last)
        {
            first.node->prev->next = last.node;
            last.node->prev = first.node->prev;
            while (first != last)
            {
                first++;
                destroy_node(first.node->prev);
                size_--;
            }
            return first;
        }

        // remove移除所有等于value的节点
        void remove(const T &value)
        {
            iterator first = begin();
            iterator last = end();
            while (first != last)
            {
                if (*first == value)
                {
                    first = erase(first);
                    continue;
                }
                ++first;
            }
        }

        // unique 移除相邻的重复元素
        void unique()
        {
            if (size_ == 0)
                return;
            iterator first = begin();
            iterator last = --end();
            while (first != last)
            {
                iterator tmp = first++;
                if (*first == *tmp)
                {
                    erase(tmp);
                }
            }
        }

        // reverse 反转链表
        void reverse()
        {
            if (size_ < 1)
                return;
            nodePointer cue = node;
            nodePointer temp=nullptr;
            for (int i = size_+1; i != 0; --i)
            {
                temp = cue->next;
                cue->next = cue->prev;
                cue->prev = temp;
                cue = cue->next;
            }
    
        }

        void _sort(iterator first, iterator last){
            value_type pod=*first;
            iterator low=first;
            iterator high=last;
            while(low!=high){
                while(low!=high&&*high>=pod) --high;
                low.node->data=*high;
                while(low!=high&&*low<=pod) ++low;
                high.node->data=*low;
            }
            low.node->data=pod;
            if(low!=first){
                _sort(first,--low);
                ++low;
            }
            if(low!=last){
                _sort(++low,last);
                --low;
            }
        }

        //排序
        void sort(){
            if(size_<=1) return;
            _sort(begin(),--end());
        }

    }; // end of list

}

#endif