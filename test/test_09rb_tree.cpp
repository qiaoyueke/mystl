#include "../src/09rb_tree.h"
#include "../src/02alogbase.h"

using namespace qyk::detail;

struct keyogvalue
{
    template <class T>
    T operator()(T &t) { return t; }
};

template <class T>
T keyofvaluee(T &t) { return t; }

int main()
{
    rb_tree<int, int, keyogvalue, less<int>> rbtree;

    auto begin = rbtree.begin();
    begin++;
    ++begin;
    begin--;
    --begin;
    // rb_tree<int,int,keyogvalue,less<int>> rbtree1(rbtree);
    // rbtree1.swap(rbtree);
    std::cout << rbtree.begin().node << std::endl;
    std::cout << rbtree.end().node << std::endl;
    rbtree.insert_equal(0);
    rbtree.insert_equal(1);
    rbtree.insert_equal(2);
    rbtree.insert_equal(0);
    rbtree.insert_equal(-10);
    rbtree.insert_equal(-1);
    rbtree.insert_equal(-100);
    rbtree.insert_equal(100);
    std::cout << rbtree.size() << std::endl;
    std::cout << *(++rbtree.begin()) << std::endl;
    std::cout << rbtree.end().node << std::endl;

    std::cout << std::endl;

    std::cout << *rbtree.lower_bound(0) << std::endl;
    std::cout << *rbtree.upper_bound(0) << std::endl;

    while (rbtree.begin() != rbtree.end())
    {
        rbtree.erase(rbtree.begin());

        for (rb_tree<int, int, keyogvalue, less<int>>::iterator i = rbtree.begin(); i != rbtree.end(); i++)
        {
            std::cout << *i << "    ";
        }
        std::cout << std::endl;
    }

    rbtree.erase(rbtree.begin());
    rbtree.erase(rbtree.begin());

    std::cout << rbtree.count(0) << "    " << std::endl;

    std::cout << rbtree.size() << "    " << std::endl;

    std::cout << *rbtree.find(0) << "    " << std::endl;
    return 0;
}