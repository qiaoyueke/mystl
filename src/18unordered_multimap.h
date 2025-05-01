// 使用hash_table来实现，hash_table已经实现了unordered_multimap的方法

#ifndef QYK_UNORDERED_MULTIMAP
#define QYK_UNORDERED_MULTIMAP

#include "14hash_table.h"
#include "02alogbase.h"

namespace qyk
{
    template <class Key, class T, class HashFunc = std::hash<Key>, class KeyEqual = Equal<Key>, class Alloc = alloc>
    class unordered_multimap
    {
    private:
        using Value = std::pair<Key, T>;
        using cor = detail::hash_table<Key, Value, select1st<Key, T>, KeyEqual, HashFunc, Alloc>;
        cor core;

    public:
        using value_type = Value;
        using key_type = Key;
        using iterator = typename cor::iterator;
        using const_iterator = typename cor::const_iterator;
        using self = unordered_multimap<Key, T, HashFunc, KeyEqual, Alloc>;
        using pointer = typename cor::pointer;
        using reference = typename cor::reference;
        using const_pointer = typename cor::const_pointer;
        using const_reference = typename cor::const_reference;
        using size_type = typename cor::size_type;
        using difference_type = typename cor::difference_type;

        unordered_multimap() : core(50, HashFunc(), KeyEqual()) {}
        explicit unordered_multimap(const size_type n) : core(n, HashFunc(), KeyEqual()) {}
        unordered_multimap(const size_type n, const HashFunc &hf) : core(n, hf, KeyEqual()) {}
        unordered_multimap(const size_type n, const HashFunc &hf, const KeyEqual &eql) : core(n, hf, eql) {}
        template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
        unordered_multimap(Iterator first, Iterator last) : core(50, HashFunc(), KeyEqual())
        {
            core.insert_equal(first, last);
        }
        template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
        unordered_multimap(Iterator first, Iterator last, size_type n)
            : core(n, HashFunc(), KeyEqual())
        {
            core.insert_equal(first, last);
        }
        template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
        unordered_multimap(Iterator first, Iterator last,
                      size_type n, const HashFunc &hf) : core(n, hf, KeyEqual())
        {
            core.insert_equal(first, last);
        }
        template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
        unordered_multimap(Iterator first, Iterator last,
                      size_type n, const HashFunc &hf, const KeyEqual &eql) : core(n, hf, eql)
        {
            core.insert_equal(first, last);
        }

        iterator begin() { return core.begin(); }
        iterator end() { return core.end(); }
        const_iterator begin() const { return core.begin(); }
        const_iterator end() const { return core.end(); }
        bool empty() const { return core.empty(); }
        size_type size() const { return core.size(); }
        void swap(self &other) { return core.swap(other.core); }

        T& operator[](const key_type& key){
            return ((*core.insert_equal(value_type(key, T()))).second);
        }

        iterator insert(const value_type &x)
        {
            return core.insert_equal(x);
        }

        template <class Iterator>
        void insert(Iterator first, Iterator last)
        {
            core.insert_equal(first, last);
        }

        void erase(iterator pos)
        {
            core.erase(pos);
        }

        void erase(const key_type &x)
        {
            core.erase(x);
        }
        void erase(iterator first, iterator last)
        {
            core.erase(first, last);
        }
        void clear() { core.clear(); }

        iterator find(const key_type &x) { return core.find(x); }
        const_iterator find(const key_type &x) const { return core.find(x); }
        size_type count(const key_type &x) const { return core.count(x); }
        void resize(size_type new_sz) { core.resize(new_sz); }
    };

}

#endif