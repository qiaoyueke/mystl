#ifndef QYK_MULTIMAP
#define QYK_MULTIMAP
#include "09rb_tree.h"

namespace qyk
{
    template <class Key, class T, class Compare = less<Key>, class Alloc = alloc>
    class multimap
    {
    private:
        using Value = std::pair<Key,T>;
        using cor = detail::rb_tree<Key, Value, select1st<Key,T>, Compare, Alloc>;
        cor core;

    public:
        using key_type = Key;
        using value_type = Value;
        using iterator = typename cor::iterator;
        using const_iterator = typename cor::const_iterator;
        using self = multimap<T, Compare, Alloc>;
        using pointer = typename cor::pointer;
        using reference = typename cor::reference;
        using const_pointer = typename cor::const_pointer;
        using const_reference = typename cor::const_reference;
        using size_type = typename cor::size_type;
        using difference_type = typename cor::difference_type;

        multimap() : core(Compare()) {}
        explicit multimap(Compare comp) : core(comp) {}
        template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
        multimap(Iterator first, Iterator last) : core(Compare())
        {
            core.insert_equal(first, last);
        }
        multimap(const self &other) : core(other.core) {}

        self &operator=(const self &other)
        {
            core = other.core;
            return *this;
        }

        iterator begin() { return core.begin(); }
        iterator end() { return core.end(); }
        bool empty() { return core.empty(); }
        size_type size() { return core.size(); }
        void swap(self &other) { return core.swap(other.core); }

        std::pair<iterator, bool> insert(const value_type &x)
        {
            std::pair<iterator, bool> p = core.insert_equal(x);
            return std::pair<iterator, bool>(p.first, p.second);
        }

        template <class InputIterator>
        void insert(InputIterator first, InputIterator last)
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

        size_type count(const key_type &x) const { return core.count(x); }
        iterator find(const key_type &x)  { return core.find(x); }
        iterator lower_bound(const key_type &x)  { return core.lower_bound(x); }
        iterator upper_bound(const key_type &x)  { return core.upper_bound(x); }

        const_iterator find(const key_type &x) const { return core.find(x); }
        const_iterator lower_bound(const key_type &x) const { return core.lower_bound(x); }
        const_iterator upper_bound(const key_type &x) const { return core.upper_bound(x); }

        T& operator[](const key_type& key){
            return (*((core.insert_equal(value_type(key,T()))).first)).second;
            //core.insert_equal(value_type(key,T())) 插入一个value：(key,T())，返回一个pair<iterator,bool>
            //返回值的first是迭代器，指向插入的值，或者冲突的值，取second就是T&
        }
    };
}

#endif