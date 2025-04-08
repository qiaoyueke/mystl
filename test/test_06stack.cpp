#include "../src/06stack.h"

using namespace qyk;
int main()
{
    stack<int> stack;
    for (int i = 0; i < 10; i++)
    {
        stack.push(i);
    }
    for (int i = 0; i < 10; i++)
    {
        std::cout << stack.top() << std::endl;
        stack.pop();
    }
    return 0;
}
