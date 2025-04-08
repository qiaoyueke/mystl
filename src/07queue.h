#ifndef QYK_QUEUE
#define QYK_QUEUE

#include "04list.h"

namespace qyk
{

    template <class T, class cor = list<T>>
    class queue
    {
    public:
        using value_type = typename cor::value_type;
        using size_type = typename cor::size_type;
        using reference = typename cor::reference;
        using const_reference = typename cor::const_reference;

    private:
        cor core;

    public:
        queue():core(){}
        bool empty() const { return core.empty(); }
        size_type size() const { return core.size(); }
        reference front() { return core.front(); }
        const_reference front() const { return core.front(); }
        reference back() { return core.back(); }
        const_reference back() const { return core.back(); }
        void push(const value_type &value) { core.push_front(value); }
        void pop() { core.pop_back(); }
    };

}

#endif
