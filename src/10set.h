#ifndef QYK_SET
#define QYK_SET

#include "09rb_tree.h"
#include "02alogbase.h"

namespace qyk
{
    template <class T, class Compare = less<T>, class Alloc = alloc>
    class set
    {
    private:
        using cor = detail::rb_tree<T, T, Self<T>, Compare, Alloc>;
        cor core;

    public:
        using value_type = T;
        using key_type = T;
        using iterator = typename cor::const_iterator;
        using const_iterator = typename cor::const_iterator;
        using self = set<T, Compare, Alloc>;
        using pointer = typename cor::const_pointer;
        using reference = typename cor::const_reference;
        using const_pointer = typename cor::const_pointer;
        using const_reference = typename cor::const_reference;
        using size_type = typename cor::size_type;
        using difference_type = typename cor::difference_type;

        set() : core(Compare()){}
        explicit set(Compare comp) : core(comp) {}
        template <class Iterator, typename = typename my_enable_if<is_iterator<Iterator>::value>::type>
        set(Iterator first, Iterator last) : core(Compare())
        {
            core.insert_unique(first, last);
        }
        set(const self &other) : core(other.core) {}

        self &operator=(const self &other)
        {
            core = other.core;
            return *this;
        }

        iterator begin() { return core.begin(); }
        iterator end() { return core.end(); }
        bool empty(){return core.empty(); } 
        size_type size() { return core.size(); }
        void swap(self &other) { return core.swap(other.core); }

        std::pair<iterator, bool> insert(const value_type &x)
        {
            std::pair<iterator, bool> p = core.insert_unique(x);
            return std::pair<iterator, bool>(p.first, p.second);
        }

        template <class InputIterator>
        void insert(InputIterator first, InputIterator last)
        {
            core.insert_unique(first, last);
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

        iterator find(const key_type &x) const { return core.find(x); }
        size_type count(const key_type &x) const { return core.count(x); }
        iterator lower_bound(const key_type &x) const { return core.lower_bound(x); }
        iterator upper_bound(const key_type &x) const { return core.upper_bound(x); }
    };

}

#endif