#include "../src/07queue.h"

using namespace qyk;
int main()
{
    queue<int> que;
    for (int i = 0; i < 10; i++)
    {
        que.push(i);
    }
    for (int i = 0; i < 10; i++)
    {
        std::cout << que.front() <<"    "<<que.back()<< std::endl;
        que.pop();
    }
    return 0;
}