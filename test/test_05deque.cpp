#include "../src/05deque.h"
#include <iostream>
#include "../src/03vector.h"

using namespace qyk;

int main()
{


    detail::deque_iterator<int, int &, int *, 5> iterator;
    qyk_vector<int> b1(5, 1);
    qyk_vector<int> b2(5, 2);
    qyk_vector<int> b3(5, 3);
    qyk_vector<int *> map{b1.begin(), b2.begin(), b3.begin()};
    iterator.set_node(map.begin());
    iterator.cur = b1.begin();
    auto finish = b3.end() - 1;
    for (auto i = iterator; i.cur != finish; ++i)
    {
        std::cout << *i << std::endl;
    }
    std::cout << iterator[6] << std::endl;

    deque<int> que1;

    deque<int> que2(5, 5);

    deque<int> que3(6);

    deque<int> que4(b1.begin(), b1.end());

    for (auto i = que4.begin(); i != que4.end(); ++i)
    {
        std::cout << *i << "    ";
    }
    std::cout << "        que4" << std::endl;

    deque<int> que5(que4);

    for (auto i = que5.begin(); i != que5.end(); ++i)
    {
        std::cout << *i << "    ";
    }
    std::cout << "       que5: que5(que4)" << std::endl;

    deque<int> que6(std::move(que4));

    for (auto i = que6.begin(); i != que6.end(); ++i)
    {
        std::cout << *i << "    ";
    }
    std::cout << "       que6: que6(que4)" << std::endl;

    for (auto i = que4.begin(); i != que4.end(); ++i)
    {
        std::cout << *i << "    ";
    }
    std::cout << "        que4:que6(que4)" << std::endl;

    std::cout << "que6.front(): " << que6.front() << "    que6.back(): " << que6.back() << std::endl;
    que6.pop_front();
    que6.push_front(2);
    que6.pop_back();
    que6.push_back(2);
    que6.push_back(2);

    for (auto i = que6.begin(); i != que6.end(); ++i)
    {
        std::cout << *i << "    ";
    }
    std::cout << "       que6: push_front/back" << std::endl;

    deque<int, 5> que7;
    for (int i = 0; i < 1000; i++)
    {
        que7.emplace_front(i);
    }
    for (int i = 0; i < 9; i++)
    {
        que7.emplace_back(i);
    }

    que7.insert(que7.end()-10,10);

    que7.insert(que7.begin()+3,10,10);

    que7.insert(que7.end()-3,10,10);

    que7.insert(que7.end()-3,que6.begin(),que6.end());

    for (auto i = que7.begin(); i != que7.end(); ++i)
    {
        std::cout << *i << "    ";
    }
    std::cout << "       que7: push_front/back" << std::endl;

    return 0;
}

