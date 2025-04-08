// stack并不是单独的容器，而是一个适配器，它的功能通过内部的底层容器实现

#ifndef QYK_STACK
#define QYK_STACK

#include "04list.h" //使用lsit作为底层容器

namespace qyk
{
    template <class T, class cor = list<T>>
    class stack
    {
    public:
        using value_type = typename cor::value_type;
        using size_type = typename cor::size_type;
        using reference = typename cor::reference;
        using const_reference = typename cor::const_reference;

    private:
        list<T> core; // 底层容器

    public:
        stack() : core() {}
        bool empty() const { return core.empty(); }
        size_type size() const { return core.size(); }
        reference top() { return core.back(); }
        const_reference top() const { return core.back(); }
        void push(const value_type &value) { core.push_back(value); }
        void pop() { core.pop_back(); }
    };

}

#endif