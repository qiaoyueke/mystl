/*使用链表式的hash_table，用一个vector存放每个链表的起点，元素的key值通过hash函数计算后映射到vector的某个序号n
将这个元素以链表节点hash_node的形式链接在vector[n]存放的链表后面；
预先设置一个数组，里面存放一系列升放置的质数，后一个质数大致为前一个质数的两倍大小
hash表的大小从这个数组里面取，保持存放的总元素个数小于vector.size；
*/

#ifndef QYK_HASH_TABLE
#define QYK_HASH_TABLE

#include "03vector.h"
#include "02alogbase.h"

namespace qyk
{
    namespace detail
    {
        // hash_table内链表节点的声明
        template <class Value>
        class hash_table_node
        {
        public:
            using Node = hash_table_node<Value>;
            Value value;
            Node *next;
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
        template <class Key, class Value, class KeyOfValue, class KeyEqual, class HashFuc, class Alloc = alloc>
        class hash_table_iterator
        {
        public:
            using Node = typename hash_table_node<Value>::Node;
            using HashTable = hash_table<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;
            using value_type = Value;
            using pointer = Value *;
            using const_pointer = const Value *;
            using reference = Value &;
            using const_reference = const Value &;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using iterator = hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;
            using const_iterator = hash_table_iterator<const Key, const Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;
            using iterator_category = forward_iterator_tag;
            using self = hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;

            Node *pnode;
            HashTable *pht;

        public:
            // 自增， 前置++ ,哈希表的迭代器没有后退操作
            self &operator++()
            {
                if (pnode->next != nullptr)
                {
                    pnode = pnode->next;
                }
                else
                {
                    size_t bn = pht->get_bucket_number(*this);
                    while (++bn < (pht->buckets).size() && (pht->buckets)[bn] == nullptr)
                        if (bn == (pht->buckets).size())
                        {
                            pnode = self(nullptr, pht);
                        }
                        else
                        {
                            pnode = (pht->buckets)[bn];
                        }
                }
                return pnode;
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

            bool operator==(const iterator &other) const { return (pnode == other.pnode); }
            bool operator!=(const iterator &other) const { return (pnode != other.pnode); }
        }; //end of iterator

        //hash_table 的正式定义
        template <class Key, class Value, class KeyOfValue, class KeyEqual, class HashFuc, class Alloc = alloc>
        class hash_table{
            public:
            using self = hash_table<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;
            using value_type = Value;
            using pointer = Value *;
            using const_pointer = const Value *;
            using reference = Value &;
            using const_reference = const Value &;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using iterator = hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;
            using const_iterator = hash_table_iterator<const Key, const Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;
            
            friend class hash_table_iterator<Key, Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;
            friend class hash_table_iterator<const Key, const Value, KeyOfValue, KeyEqual, HashFuc, Alloc>;

            private:
            using Node = typename hash_table_node<Value>::Node;

            vector<hash_table_node<Value>, Alloc> buckets;
            size_type num;
            public:
        };// end of hash_table

    } // end of detail
} // end of qyk

#endif