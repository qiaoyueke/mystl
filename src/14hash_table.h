/*使用链表式的hash_table，用一个vector存放每个链表的起点，元素的key值通过hash函数计算后映射到vector的某个序号n
将这个元素以链表节点hash_node的形式链接在vector[n]存放的链表后面；
预先设置一个数组，里面存放一系列升放置的质数，后一个质数大致为前一个质数的两倍大小
hash表的大小从这个数组里面取，保持存放的总元素个数小于vector.size；
*/

#ifndef QYK_HASH_TABLE
#define QYK_HASH_TABLE

#include "03vector.h"
#include "02alogbase.h"
#include <utility>
#include "02construct.h"
#include "02iterator.h"
#include <cstddef>
#include "01qyk_allocator.h"
#include <type_traits>

namespace qyk
{
    namespace detail
    {
        // hash_table内链表节点的声明
        template <class Value>
        class hash_table_node
        {
        public:
            using nodePointer = hash_table_node<Value> *;
            Value value;
            nodePointer next;
        };

        // 用于维护hashtanle桶子个数的质数数组（两相邻数之间大致成两倍关系）
        constexpr int __stl_num_primes = 28;
        constexpr unsigned long __stl_prime_list[__stl_num_primes] = {
            53, 97, 193, 389, 769,
            1543, 3079, 6151, 12289, 24593,
            49157, 98317, 196613, 393241, 786433,
            1572869, 3145739, 6291469, 12582917, 25165843,
            50331653, 100663319, 201326611, 402653189, 805306457,
            1610612741, 3221225473ul, 4294967291ul};

        // 以下函数找出以上数组中不小于n的最小的质数
        inline unsigned long __stl_next_prime(unsigned long n)
        {
            const unsigned long *first = __stl_prime_list;
            const unsigned long *last = __stl_prime_list + __stl_num_primes;
            const unsigned long *pos = lower_bound(first, last, n);
            return pos == last ? *(last - 1) : *pos;
        }

        // hash_table的结构声明
        template <class Key, class Value, class KeyOfValue, class KeyEqual, class HashFuc, class Alloc = alloc>
        class hash_table;

        // hash_table的迭代器
        template <class Key, class Value, class KeyOfValue, class KeyEqual, class HashFuc, bool IsConst, class Alloc = alloc>
        class hash_table_iterator
        {
        public:
            using nodePointer = typename std::conditional<IsConst,const hash_table_node<Value>*, hash_table_node<Value>*>::type;
            using HashTable = hash_table<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;
            using HashTablePointer = typename std::conditional<IsConst, const HashTable *, HashTable *>::type;
            using value_type = typename std::conditional<IsConst, const Value, Value>::type;
            using pointer = value_type *;
            using reference = value_type &;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using iterator = hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, false, Alloc>;
            using const_iterator = hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, true, Alloc>;
            using iterator_category = forward_iterator_tag;
            using self = hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, IsConst, Alloc>;

            nodePointer pnode;
            HashTablePointer pht;

        public:
            hash_table_iterator() : pnode(nullptr), pht(nullptr) {}
            hash_table_iterator(nodePointer node, HashTablePointer ht) : pnode(node), pht(ht) {}
            hash_table_iterator(const self &other) : pnode(other.pnode), pht(other.pht) {}
            template <bool OtherIsConst, typename = std::enable_if_t<IsConst && !OtherIsConst>>
            hash_table_iterator(const hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, OtherIsConst, Alloc> &other)
                : pnode(other.pnode), pht(other.pht) {}

            // 自增， 前置++ ,哈希表的迭代器没有后退操作
            self &operator++()
            {
                if (pnode->next == nullptr)
                {
                    size_t bn = pht->bucket_num(**this);
                    while (++bn < (pht->buckets).size())
                    {
                        if ((pht->buckets)[bn] != nullptr)
                        {
                            pnode = (pht->buckets)[bn];
                            return *this;
                        }
                    }
                }
                pnode = pnode->next;
                return *this;
            }
            // 自增，后置++
            self operator++(int)
            {
                self temp = *this;
                ++(*this);
                return temp;
            }

            // 解迭代器
            reference operator*() const { return pnode->value; }
            pointer operator->() const { return &(operator*()); }

            bool operator==(const self &other) const { return (pnode == other.pnode); }
            bool operator!=(const self &other) const { return (pnode != other.pnode); }
        }; // end of iterator

        // hash_table 的正式定义
        template <class Key, class Value, class KeyOfValue, class KeyEqual, class HashFuc, class Alloc>
        class hash_table
        {
        public:
            using key_type = Key;
            using value_type = Value;
            using pointer = Value *;
            using const_pointer = const Value *;
            using reference = Value &;
            using const_reference = const Value &;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using iterator = hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, false, Alloc>;
            using const_iterator = hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, true, Alloc>;

            friend class hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, true, Alloc>;

            friend class hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, false, Alloc>;

        private:
            using nodePointer = hash_table_node<Value> *;
            using node_allocater = allocator<hash_table_node<Value>, Alloc>;
            using self = hash_table<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;

            vector<nodePointer, Alloc> buckets;
            size_type num;

            HashFuc hashfuc;
            KeyEqual key_equal;
            KeyOfValue get_key;

        protected:
            // 初始化与扩容时用到，寻找下一个预设的大小
            size_type next_size(const size_type n) const { return __stl_next_prime(n); }

            // 返回元素在那个bucket内, 可接受的参数有：键，值, 桶子个数
            size_type bucket_num(const key_type &key) const { return size_type(hashfuc(key) % buckets.size()); }

            template <typename value_type>
            auto bucket_num(const value_type &value) const ->
                typename my_enable_if<!my_is_same<value_type, key_type>::value, size_type>::type
            {
                return bucket_num(get_key(value));
            }

            size_type bucket_num(const value_type &value, size_type bucketsNum)
            {
                return hashfuc(get_key(value)) % bucketsNum; // 寻找给出的值在目标桶子数量时的位置
            }

            // 创建节点
            nodePointer creat_node(const value_type &value)
            {
                nodePointer node = node_allocater::allocate();
                construct(&node->value, value);
                node->next = nullptr;
                return node;
            }

            // 销毁节点
            void free_node(nodePointer ptr)
            {
                qykDestroy(&ptr->value);
                ptr->next = nullptr;
                node_allocater::deallocate(ptr);
            }

            // 插入节点，不允许重复
            std::pair<iterator, bool> insert_unique_aux(const value_type &value);
            // 插入节点，允许重复
            iterator insert_equal_aux(const value_type &value);
            // 删除节点
            void erase_aux(const value_type &value)
            {
                size_type n = bucket_num(value);
                nodePointer cur = buckets[n];

                if (cur == nullptr)
                    return;

                // buckets[n]就是要删除的
                if (key_equal(get_key(value), get_key(cur->value)))
                {
                    buckets[n] = cur->next;
                    free_node(cur);
                    num--;
                    return;
                }

                nodePointer pre = cur;
                cur = cur->next;

                while (cur != nullptr)
                {
                    if (key_equal(get_key(value), get_key(cur->value)))
                    {
                        pre->next = cur->next;
                        free_node(cur);
                        num--;
                        return;
                    }
                    pre = cur;
                    cur = cur->next;
                }
            }

        public:
            hash_table(size_type n, const HashFuc &hf, const KeyEqual &ke)
                : buckets(next_size(n), nullptr), hashfuc(hf), key_equal(ke), get_key(KeyOfValue())
            {
                num = 0;
            }

            iterator begin()
            {
                for (size_type i = 0; i < buckets.size(); i++)
                {
                    if (buckets[i] != nullptr)
                    {
                        return iterator(buckets[i], this);
                    }
                }
                return end();
            }

            const_iterator begin() const
            {
                for (size_type i = 0; i < buckets.size(); i++)
                {
                    if (buckets[i] != nullptr)
                    {
                        return const_iterator(buckets[i], this);
                    }
                }
                return end();
            }

            iterator end() { return iterator(nullptr, this); }
            const_iterator end() const { return const_iterator(nullptr, this); }

            size_type size() const { return num; }
            bool empty() const { return num == 0; }

            // 查找键为key的元素，返回迭代器
            iterator find(key_type &key)
            {
                size_type n = bucket_num(key);
                nodePointer res = buckets[n];

                while (res != nullptr)
                {
                    if (key_equal(key, get_key(res->value)))
                    {
                        return iterator(res, this);
                    }
                    res = res->next;
                }
                return end();
            }

            const_iterator find(key_type &key) const
            {
                size_type n = bucket_num(key);
                nodePointer res = buckets[n];

                while (res != nullptr)
                {
                    if (key_equal(key, get_key(res->value)))
                    {
                        return const_iterator(res, this);
                    }
                    res = res->next;
                }
                return end();
            }

            // 返回键为key的元素的个数
            size_type count(const key_type &key) const
            {
                iterator first = find(key);
                size_type result = 0;
                if (first != end())
                {
                    nodePointer cur = first.pnode;
                    while (cur != nullptr)
                    {
                        if (key_equal(key, get_key(cur->value)))
                        {
                            result++;
                            cur = cur->next;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                return result;
            }

            // 可能的扩容操作
            void resize(const size_type newNum)
            {
                if (newNum > buckets.size())
                {
                    // 新的桶子的个数
                    size_type newSize = next_size(newNum);
                    vector<nodePointer> newBuckets(newSize, nullptr);

                    size_type oldSize = buckets.size();

                    // 遍历原来的每个桶子
                    for (size_type i = 0; i < oldSize; i++)
                    {
                        nodePointer cur = buckets[i];
                        // 对桶子里的每一个节点：
                        while (cur != nullptr)
                        {
                            // 找到这个节点在哪一个新桶子内
                            size_type newBucketNum = bucket_num(cur->value, newSize);

                            // 更新旧桶子
                            buckets[i] = cur->next;

                            // 将这个节点插入到桶子的头部
                            cur->next = newBuckets[newBucketNum];
                            newBuckets[newBucketNum] = cur;

                            // 更新cur
                            cur = buckets[i];
                        }
                    }
                    buckets.swap(newBuckets);
                }
            }

            // 插入元素，不允许重复，返回一个pair<iterator, bool>，
            // 第一个元素是插入元素的迭代器或者发生冲突的元素的迭代器，第二个元素表示是否插入成功
            std::pair<iterator, bool> insert_unique(const value_type &value)
            {
                // 先判断插入后是否需要重构
                resize(num + 1);
                return insert_unique_aux(value);
            }
            template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
            void insert_unique(Iterator first, Iterator last)
            {
                while (first != last)
                {
                    insert_unique(*first);
                    ++first;
                }
                return;
            }

            // 插入元素，允许重复，返回一个iterator
            iterator insert_equal(const value_type &value)
            {
                // 先判断插入后是否需要重构
                resize(num + 1);
                return insert_equal_aux(value);
            }
            template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
            void insert_equal(Iterator first, Iterator last)
            {
                while (first != last)
                {
                    insert_equal(*first);
                    ++first;
                }
                return;
            }

            // 删除元素， 可以接受的参数有：值，迭代器，节点
            void erase(const_iterator it) { return erase_aux(*it); }
            void erase(const value_type &value) { return erase_aux(value); }
            void erase(nodePointer node) { return erase_aux(node->value); }
            template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::tpye>
            void erase(Iterator first, Iterator last)
            {
                while (first != last)
                {
                    Iterator temp = first++;
                    erase_aux(*temp);
                }
                return;
            }

            // clear()，清空哈希表,只是释放了表内存放的元素，没有释放vector的内存
            void clear()
            {
                size_type size = buckets.size();
                for (size_type i = 0; i < size; i++)
                {
                    nodePointer cur = buckets[i];
                    while (cur != nullptr)
                    {
                        nodePointer temp = cur;
                        cur = cur->next;
                        free_node(temp);
                    }
                    buckets[i] = nullptr;
                }
                num = 0;
            }

            // 复制 other 到本hashtable
            void copy_from(const hash_table &other)
            {
                // 先将自己清空，把buckets调整到个other的buckets大小一样，用nullptr填充
                clear();
                size_type bucketSize = other.buckets.size();
                buckets.resize(bucketSize, nullptr);

                // 将other的node一一复制过来
                for (size_type i = 0; i < bucketSize; i++)
                {
                    nodePointer cur = other.buckets[i];
                    if (cur != nullptr)
                    {
                        buckets[i] = creat_node((*cur)->value);
                        nodePointer copy = buckets[i];
                        for (cur = cur->next; cur != nullptr; cur = cur->next)
                        {
                            copy->next = creat_node((*cur)->value);
                            copy = copy->next;
                        }
                    }
                }
                num = other.num;
            }

            // 交换两个哈希表
            void swap(self &other)
            {
                buckets.swap(other.buckets);
                qyk::swap(num, other.num);
            }

        }; // end of hash_table

        // 插入元素，不允许重复，
        template <class Key, class Value, class KeyOfValue, class KeyEqual, class HashFuc, class Alloc>
        std::pair<typename hash_table<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>::iterator, bool>
        hash_table<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>::insert_unique_aux(const value_type &value)
        {
            size_type n = bucket_num(value);
            nodePointer cur = buckets[n];

            while (cur != nullptr)
            {
                // 发现重复的，直接返回
                if (key_equal(get_key(value), get_key(cur->value)))
                {
                    return std::pair<iterator, bool>(iterator(cur, this), false);
                }
                cur = cur->next;
            }
            // 到这里说明没有重复的, 申请新的节点放到桶子里

            nodePointer newNode = creat_node(value);
            newNode->next = buckets[n];
            buckets[n] = newNode;
            num++;

            return std::pair<iterator, bool>(iterator(newNode, this), true);
        }

        // 插入元素，允许重复，
        template <class Key, class Value, class KeyOfValue, class KeyEqual, class HashFuc, class Alloc>
        typename hash_table<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>::iterator
        hash_table<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>::insert_equal_aux(const value_type &value)
        {
            size_type n = bucket_num(value);
            nodePointer cur = buckets[n];
            nodePointer newNode = creat_node(value);
            num++;

            while (cur != nullptr)
            {
                // 发现重复的，将新节点放在这一节点后面，返回
                if (key_equal(get_key(value), get_key(cur->value)))
                {
                    newNode->next = cur->next;
                    cur->next = newNode;
                    return iterator(newNode, this);
                }
                cur = cur->next;
            }

            // 到这里说明没有重复的, 申请新的节点放到桶子里
            newNode->next = buckets[n];
            buckets[n] = newNode;
            return iterator(newNode, this);
        }

    } // end of detail
} // end of qyk

#endif