#ifndef QYK_PRIORITY_QUEUE
#define QYK_PRIORITY_QUEUE

#include "03vector.h"
#include "02alogbase.h"

namespace qyk
{
    template <class T, class cor = vector<T>, class Compare = less<typename cor::value_type>>
    class priority_queue
    {
    public:
        using value_type = typename cor::value_type;
        using reference = typename cor::reference;
        using const_reference = typename cor::const_reference;
        using size_type = typename cor::size_type;
        using difference_type = typename cor::difference_type;
        using iterator = typename cor::iterator;

        // private:
        cor core;
        Compare compare;

        // make_heap,将传入的容器改造为符合传入的比较规则的堆
        void make_heap();
        // heap_down(size_t n),将core（n）这个节点进行下溯操作
        void heap_down(size_t parent);
        // heap_down(size_t n),将core（n）这个节点进行上溯操作
        void heap_up(size_t parent);

    public:
        // 默认构造
        priority_queue() : core() { std::cout << "morengouzao"; }
        // 传入容器
        priority_queue(const cor &core) : core(core)
        {
            make_heap();
        }
        // 迭代器构造
        priority_queue(iterator first, iterator last) : core(first, last)
        {
            make_heap();
        }

        bool empty() const { return core.empty(); }
        size_type size() const { return core.size(); }
        const_reference top() const { return core.front(); }

        // 插入元素,将插入的元素放到最后，然后对这个节点上溯
        void push(const T &value)
        {
            core.push_back(value);
            heap_up(core.size() - 1);
        }

        // 抛出元素,将最后一个元素的元素放到堆顶，然后瑞堆顶下溯
        void pop()
        {
            if (size() < 2)
            { // 为空或者只有一个元素，不需要维护heap
                if (size() == 0)
                {
                    return;
                }
                core.pop_back();
                return;
            }
            core[0] = core[core.size() - 1];
            core.pop_back();
            heap_down(0);
        }
    };

    // make_heap的实现，heap可以看作一颗树，其中每个节点与其父节点都满足Compare为真
    // 从最后一个非叶子节点开始，到根节点位为止，进行下溯操作，可以完成堆的构建
    template <class T, class cor, class Compare>
    void priority_queue<T, cor, Compare>::make_heap()
    {
        size_type len = core.size();
        if (len < 2) // 只有一个元素或者没有元素，不需要操作
            return;
        difference_type parentNode = len / 2 - 1; // 找到最后一个非叶子节点
        while (parentNode >= 0)                   // 遍历所有非叶子节点
        {
            heap_down(parentNode); // 依次进行下溯
            --parentNode;
        }
    }

    // heap_down
    template <class T, class cor, class Compare>
    void priority_queue<T, cor, Compare>::heap_down(size_t parent)
    {
        value_type value = core[parent]; // 需要下溯节点的值
        size_t child = 2 * (parent + 1); // 该节点可能存在的右子节点编号
        while (child < core.size())      // 当右子节点存在时
        {
            if (!compare(core[child - 1], core[child]))
            { // 找到两个子节点中权重更高的
                child--;
            }
            if (!compare(value, core[child]))
            { // 如果进行下溯的节点权重更高，则停止下溯
                break;
            }
            // 否则将权重更高的子节点转移上来，更新节点继续下溯
            core[parent] = core[child];
            parent = child;
            child = 2 * (parent + 1);
        }
        if (child == core.size()) // 只有左子节点
        {
            if (compare(value, core[child - 1]))
            {
                core[parent] = core[child - 1];
                parent = child - 1;
            }
        }
        core[parent] = value; // 下溯完成，找到了初始节点下溯后的位置，将值转移过来
    }

    // heap_up
    template <class T, class cor, class Compare>
    void priority_queue<T, cor, Compare>::heap_up(size_t child)
    {
        value_type value = core[child];
        size_t parent = child ? (child - 1) / 2 : 0;
        while (child != 0)
        {
            if (compare(value, core[parent]))
            {
                break;
            }
            core[child] = core[parent];
            child = parent;
            parent = (child - 1) / 2 ;
        }
        core[child] = value;
    }
}

#endif