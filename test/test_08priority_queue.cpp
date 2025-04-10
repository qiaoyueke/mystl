#include "../src/08priority_queue.h"


using namespace qyk;
int main()
{
    vector<int> a{0,1,2,3,4,5,6,7,8,9};
    priority_queue<int> que(a);
    for (int i = 0; i < 10; i++)
    {
        std::cout<<que.top()<<"    "<<que.size()<<std::endl ;
        que.pop();
    }

    std::cout<<que.empty()<<std::endl;
    std::cout<<que.size()<<std::endl;

    vector<int> b(1);
    b.pop_back();


    std::cout<<b.empty()<<std::endl;
    std::cout<<b.size()<<std::endl;

    for (int i = -10; i < 0; i++)
    {
        que.push(i);
        for(auto j=que.core.begin();j!=que.core.end();j++){
            std::cout<<*j<<"    ";
        }std::cout<<std::endl;
        
    }

    return 0;
}