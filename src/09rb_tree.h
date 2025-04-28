#ifndef QYK_RB_TREE
#define QYK_RB_TREE

#include <cstddef>
#include <utility>
#include "02iterator.h"
#include "01qyk_allocator.h"
#include "02alogbase.h"
#include "02construct.h"
#include "07queue.h"

namespace qyk
{
    namespace detail
    {
        using rb_tree_node_color = bool;                    // 红黑树节点颜色
        constexpr rb_tree_node_color rb_tree_red = true;    // 红色节点
        constexpr rb_tree_node_color rb_tree_black = false; // 黑色节点

        template <class T>
        class rb_tree_node
        {
        public:
            using nodePointer = rb_tree_node<T> *;
            rb_tree_node_color color;
            nodePointer parent;
            nodePointer left;
            nodePointer right;
            T value; // 存放元素
        };

        template <class T, class ref = T &, class ptr = T *>
        class rb_tree_iterator
        {
        public:
            using nodePointer = typename rb_tree_node<T>::nodePointer;
            using value_type = T;
            using pointer = T *;
            using const_pointer = const T *;
            using reference = T &;
            using const_reference = const T &;
            using size_type = size_t;
            using difference_type = ptrdiff_t;
            using iterator = rb_tree_iterator<T, T &, T *>;
            using const_iterator = rb_tree_iterator<T, const T &, const T *>;
            using iterator_category = bidirectional_iterator_tag;
            using self = rb_tree_iterator<T, ref, ptr>;

            nodePointer node; // 指向一个红黑树节点

            rb_tree_iterator(nodePointer x) : node(x) {}
            rb_tree_iterator(const iterator &x) : node(x.node) {}

            reference operator*() { return node->value; }
            pointer operator->() { return &(operator*()); }

            // 前置++
            self &operator++()
            {
                if (node->right == node)
                {
                    return *this;
                }
                if (node->right != nullptr)
                { // 如果有右子树，右子树的最左节点为下一个
                    node = node->right;
                    while (node->left != nullptr)
                    {
                        node = node->left;
                    }
                }
                else
                { // 没有右子树，则找到父节点
                    auto parent = node->parent;
                    while (parent->right == node)
                    { // 当前node是parent的右子节点，则node的权重大于parent，继续往上寻找
                        node = parent;
                        parent = node->parent;
                    }
                    // 结束循环后，如果node是parent的左子节点，则返回parent
                    // 但是node不是parent的右子节点，node有可能同时也不是parent的左子节点
                    // 当此时node为根节点时，parent为head节点，说明调用++迭代器原本指向的是最大权重的节点，即end()-1;
                    // 此时应返回head，同样是返回parent，然而如果此时rb_tree只有一个元素root，此时root->parent->right==root
                    // 上面的循环会多走一步，即node=head，parent=root，此时返回node
                    if (node->right != parent)
                    { // 不是只有一个root节点
                        node = parent;
                    }
                }
                return *this;
            }

            // 后置++
            self operator++(int)
            {
                auto temp = *this;
                ++*this;
                return temp;
            }

            // 前置--
            self &operator--()
            {
                if (node->left == node)
                    return *this;
                if (node->parent->parent == node && node->color == rb_tree_red)
                { // node为head,--end()返回最大元素,head->right
                    node = node->right;
                    return *this;
                }
                else if (node->left != nullptr)
                { // 存在左子树，返回左子树中最大的
                    node = node->left;
                    while (node->right != nullptr)
                    {
                        node = node->right;
                    }
                }
                else
                { // 没有左子树，往上寻找父节点，直到当前节点为父节点的右子树，返回这个父节点
                    auto parent = node->parent;
                    while (parent->left == node)
                    {
                        node = parent;
                        parent = node->parent;
                    }
                    if (parent->parent != node) // 排除只有一个根节点时对begin（）--的情况
                        node = parent;
                }
                return *this;
            }

            // 后置--
            self operator--(int)
            {
                auto temp = *this;
                --*this;
                return temp;
            }

            bool operator==(const iterator &other) const { return (node == other.node); }
            bool operator==(const const_iterator &other) const { return (node == other.node); }
            bool operator!=(const iterator &other) const { return (node != other.node); }
            bool operator!=(const const_iterator &other) const { return (node != other.node); }
        };

        // 红黑树正式定义
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc = alloc>
        class rb_tree
        {
        public:
            using iterator = typename rb_tree_iterator<Value>::iterator;
            using const_iterator = typename rb_tree_iterator<Value, const Value &, const Value *>::const_iterator;
            using value_type = typename rb_tree_iterator<Value>::value_type;
            using key_type = Key;
            using reference = typename rb_tree_iterator<Value>::reference;
            using const_reference = typename rb_tree_iterator<Value, const Value &, const Value *>::const_reference;
            using pointer = typename rb_tree_iterator<Value>::pointer;
            using const_pointer = typename rb_tree_iterator<Value, const Value &, const Value *>::const_pointer;
            using size_type = typename rb_tree_iterator<Value>::size_type;
            using difference_type = typename rb_tree_iterator<Value>::difference_type;
            using nodePointer = typename rb_tree_iterator<Value>::nodePointer;

        protected:
            using color_type = rb_tree_node_color;
            using tree_node = rb_tree_node<value_type>;
            using rb_tree_node_allocator = allocator<tree_node, Alloc>;
            using self = rb_tree<Key, Value, KeyOfValue, Compare, Alloc>;

            nodePointer head;    // 指向头节点，头节点的parent为存放数据部分的根节点，left指向最小元素节点，right指向最大元素节点
            size_type nodeCount; // 存放元素的数量
            Compare comp;        // 节点的key值的比较规则

            // 获取节点颜色
            color_type get_color(nodePointer n)
            {
                if (n == nullptr)
                    return rb_tree_black;
                return n->color;
            }

            // 获取节点的parent
            nodePointer &get_parent(nodePointer n) { return n->parent; }

            // 获取节点的left
            nodePointer &get_left(nodePointer n) { return n->left; }

            // 获取节点的right
            nodePointer &get_right(nodePointer n) { return n->right; }

            // 申请一个head节点，初始状态下他的parent和left，right指向自身
            void get_head()
            {
                head = rb_tree_node_allocator::allocate();
                head->color = rb_tree_red; // head节点标为红色
                head->parent = nullptr;
                head->left = head;
                head->right = head;
            }

            // 申请一个node，存放value
            nodePointer creat_node(const value_type &value)
            {
                nodePointer node = rb_tree_node_allocator::allocate();
                construct(&node->value, value);
                return node;
            }

            // 回收一个node
            void free_node(nodePointer node)
            {
                qykDestroy(&node->value);
                rb_tree_node_allocator::deallocate(node);
            }

            // 获取一个节点下的最小值的节点
            nodePointer get_min(nodePointer node)
            {
                while (node->left != nullptr)
                {
                    node = node->left;
                }
                return node;
            }

            // 获取一个节点下的最大值的节点
            nodePointer get_max(nodePointer node)
            {
                while (node->right != nullptr)
                {
                    node = node->right;
                }
                return node;
            }

            // 复制一整颗树
            nodePointer _copy(nodePointer root)
            {
                if (root == nullptr)
                    return nullptr;
                nodePointer cur = creat_node(root->value);
                cur->left = _copy(root->left);
                cur->right = _copy(root->left);
                if (cur->left != nullptr)
                    cur->left->parent = cur;
                if (cur->right != nullptr)
                    cur->right->parent = cur;
                return cur;
            }

            // 插入一个元素
            iterator _insert(nodePointer pod, const value_type &value);

            // 插入一个节点后重新平衡
            void _rb_tree_rebanlance(nodePointer newNode);

            // 左旋
            void rotate_left(nodePointer x);

            // 右旋
            void rotate_right(nodePointer x);

            // 删除节点的内部实现
            void _erase(nodePointer dNode);

            // 消除双黑节点
            void _erase_double_black(nodePointer dNode);

        public:
            rb_tree(const Compare compare = Compare()) : nodeCount(0), comp(compare) { get_head(); }

            rb_tree(const self &other)
            {
                get_head();
                *this = other;
            }

            ~rb_tree()
            {
                clear();
                free_node(head);
            }

            // 拷贝赋值
            self &operator=(const self &other);

            size_type size() const { return nodeCount; }

            bool empty() const { return 0 == nodeCount; }

            iterator begin() { return iterator(head->left); }
            const_iterator begin() const { return iterator(head->left); }
            iterator end() { return iterator(head); }
            const_iterator end() const { return const_iterator(head); }

            // 交换两个rb_tree
            void swap(self &other)
            {
                auto temp = head;
                auto tcount = nodeCount;
                nodeCount = other.nodeCount;
                head = other.head;
                other.head = temp;
                other.nodeCount = tcount;
            }

            // 寻找键值为k的节点迭代器
            const_iterator find(const key_type k) const;
            iterator find(const key_type k)
            {
                const_iterator temp = static_cast<const self &>(*this).find(k);
                nodePointer p = static_cast<nodePointer>(temp.node);
                return iterator(p);
            }

            // 统计键值k的个数
            size_type count(const key_type &k) const
            {
                const_iterator first = find(k);
                size_type kCount = 0;
                if (first != end())
                {
                    kCount++;
                    while (++first != end() && !comp(KeyOfValue()(*first), k) && !comp(k, KeyOfValue()(*first)))
                    {
                        kCount++;
                    }
                }
                return kCount;
            }

            // 删除pod上的点
            void erase(iterator pod);
            // 删除迭代器之间的点
            void erase(iterator first, iterator last)
            {
                while (first != last)
                {
                    erase(first++);
                }
            }
            // 删除键为k的节点
            void erase(const key_type &k)
            {
                iterator first = find(k);
                size_type kcount = count(k);
                while (kcount != 0)
                {
                    erase(first++);
                    kcount--;
                }
            }

            // 回收除head外所有节点
            void clear()
            {
                queue<nodePointer> que;
                if (head->parent != nullptr)
                {
                    que.push(head->parent);
                    while (!que.empty())
                    {
                        auto temp = que.front();
                        if (temp->left != nullptr)
                        {
                            que.push(temp->left);
                        }
                        if (temp->right != nullptr)
                        {
                            que.push(temp->right);
                        }
                        que.pop();
                        free_node(temp);
                    }
                }
                nodeCount = 0;
                head->parent = nullptr;
                head->left = head;
                head->right = head;
            }

            // 插入元素，如果键值重复，则插入失败
            std::pair<iterator, bool> insert_unique(const value_type &value);
            template <class InputIterator>
            void insert_unique(InputIterator first, InputIterator last)
            {
                while (first != last)
                {
                    insert_unique(*first);
                    first++;
                }
            }

            // 插入元素，允许键值重复
            std::pair<iterator, bool> insert_equal(const value_type &value);
            template <class InputIterator>
            void insert_equal(InputIterator first, InputIterator last)
            {
                while (first != last)
                {
                    insert_equal(*first);
                    first++;
                }
            }

            iterator lower_bound(const key_type x)
            {
                return _lower_bound(begin(), end(), x, comp,
                                    KeyOfValue(), forward_iterator_tag(), distance_type(begin()));
            }
            const_iterator lower_bound(const key_type x) const
            {
                return _lower_bound(begin(), end(), x, comp,
                                    KeyOfValue(), forward_iterator_tag(), distance_type(begin()));
            }
            iterator upper_bound(const key_type x)
            {
                return _upper_bound(begin(), end(), x, comp,
                                    KeyOfValue(), forward_iterator_tag(), distance_type(begin()));
            }
            const_iterator upper_bound(const key_type x) const
            {
                return _upper_bound(begin(), end(), x, comp,
                                    KeyOfValue(), forward_iterator_tag(), distance_type(begin()));
            }

        }; // end of rb_tree class

        // 拷贝赋值
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc> &
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::operator=(const self &other)
        {
            clear();
            comp = other.comp;
            head->parent = _copy(other.head->parent);
            if (other.head->parent != nullptr)
            {
                nodeCount = other.nodeCount;
                head->left = get_min(head->parent);
                head->right = get_max(head->parent);
            }
            return *this;
        }

        // find
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::const_iterator
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::find(const key_type k) const
        {
            nodePointer cur = head->parent; // 根节点
            nodePointer pre = head;         // 头节点 end（）
            while (cur != nullptr)
            {
                if (comp(KeyOfValue()(cur->value), k)) // k大于当前节点，cur往左走
                {
                    cur = cur->right;
                }
                else // k小于或等于当前节点，cur往左，记录下当前节点如果最后k==pre，则当前的pre为第一个k
                {
                    pre = cur;
                    cur = cur->left;
                }
            }
            const_iterator result = const_iterator(pre); // 根节点或者或者key（pre）> k, 这种情况下没找到k，返回end（）否则返回pre：key（pre）==k,
            return ((result == end() || comp(k, KeyOfValue()(*result))) ? end() : result);
        }

        // 插入元素，如果键值重复，则插入失败
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        std::pair<typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator, bool>
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_unique(const value_type &value)
        {
            nodePointer pre = head;
            nodePointer cur = head->parent;
            while (cur != nullptr)
            {
                if (comp(KeyOfValue()(cur->value), KeyOfValue()(value)))
                {
                    pre = cur;
                    cur = cur->right;
                }
                else if (comp(KeyOfValue()(value), KeyOfValue()(cur->value)))
                {
                    pre = cur;
                    cur = cur->left;
                }
                else
                {                                                           // 只有KeyOfValue(value)==KeyOfValue()(cur->value)才会到这里
                    return std::pair<iterator, bool>(iterator(cur), false); // 插入失败
                }
            }
            return std::pair<iterator, bool>(_insert(pre, value), true);
        }

        // 插入元素,键值可以重复
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        std::pair<typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator, bool>
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::insert_equal(const value_type &value)
        {
            nodePointer pre = head;
            nodePointer cur = head->parent;
            while (cur != nullptr)
            {
                if (comp(KeyOfValue()(cur->value), KeyOfValue()(value)))
                {
                    pre = cur;
                    cur = cur->right;
                }
                else
                {
                    pre = cur;
                    cur = cur->left;
                }
            }
            return std::pair<iterator, bool>(_insert(pre, value), true);
        }

        // 插入元素的内部实现
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        typename rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::iterator
        rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::_insert(nodePointer pod, const value_type &value)
        {
            nodePointer newNode = creat_node(value);
            newNode->parent = pod;
            if (pod == head)
            { // 加入的第一个元素
                nodeCount++;
                pod->parent = newNode;
                pod->left = newNode;
                pod->right = newNode;
                _rb_tree_rebanlance(newNode);
                return newNode;
            }

            if (comp(KeyOfValue()(pod->value), KeyOfValue()(value)))
            {
                // value大于pod，构建右子节点
                pod->right = newNode;
                if (pod == head->right)
                { // 更新最大值
                    head->right = newNode;
                }
            }
            else
            {
                // value小于等于插入节点的值，构建pod的左节点
                pod->left = newNode;
                if (pod == head->left)
                { // 更新最小值
                    head->left = newNode;
                }
            }
            nodeCount++;
            _rb_tree_rebanlance(newNode);
            return newNode;
        }

        //_rb_tree_rebanlance的实现
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        void rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::_rb_tree_rebanlance(nodePointer newNode)
        {
            newNode->color = rb_tree_red; // 新插入的节点为红色

            while (newNode != head->parent && newNode->parent->color == rb_tree_red)
            {
                // 只有当前节点不是根节点且父节点为红时需要调整
                nodePointer father = get_parent(newNode);
                nodePointer grandFather = get_parent(father);
                nodePointer uncle = (grandFather->left == father) ? grandFather->right : grandFather->left;
                if (uncle != nullptr && get_color(uncle) == rb_tree_red)
                {
                    // 叔叔节点为红,将父节点和叔叔节点变黑，爷爷节点变红，再对爷爷节点平衡
                    father->color = rb_tree_black;
                    uncle->color = rb_tree_black;
                    grandFather->color = rb_tree_red;
                    newNode = grandFather;
                }
                else
                {
                    // 叔叔节点为黑
                    if (newNode == father->left)
                    {
                        if (father == grandFather->left)
                        {
                            // ll型
                            grandFather->color = rb_tree_red;
                            father->color = rb_tree_black;
                            rotate_right(grandFather);
                        }
                        else
                        {
                            // rl型
                            rotate_right(father);
                            grandFather->color = rb_tree_red;
                            newNode->color = rb_tree_black;
                            rotate_left(grandFather);
                        }
                    }
                    else
                    {
                        if (father == grandFather->left)
                        {
                            // lr型
                            rotate_left(father);
                            grandFather->color = rb_tree_red;
                            newNode->color = rb_tree_black;
                            rotate_right(grandFather);
                        }
                        else
                        {
                            // rr型
                            grandFather->color = rb_tree_red;
                            father->color = rb_tree_black;
                            rotate_left(grandFather);
                        }
                    }
                }
            } // end of while

            head->parent->color = rb_tree_black; // 根节点为黑
        }

        // 左旋
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        void rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::rotate_left(nodePointer oldNode)
        {
            nodePointer newNode = oldNode->right;

            newNode->parent = oldNode->parent;
            if (oldNode->parent == head)
            {
                newNode->parent = head;
                head->parent = newNode;
            }
            else if (oldNode->parent->left == oldNode)
            {
                oldNode->parent->left = newNode;
            }
            else
            {
                oldNode->parent->right = newNode;
            }

            oldNode->right = newNode->left;
            if (newNode->left != nullptr)
            {
                newNode->left->parent = oldNode;
            }

            oldNode->parent = newNode;
            newNode->left = oldNode;
        }

        // 右旋
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        void rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::rotate_right(nodePointer oldNode)
        {
            nodePointer newNode = oldNode->left;

            newNode->parent = oldNode->parent;
            if (oldNode->parent == head)
            {
                newNode->parent = head;
                head->parent = newNode;
            }
            else if (oldNode->parent->left == oldNode)
            {
                oldNode->parent->left = newNode;
            }
            else
            {
                oldNode->parent->right = newNode;
            }

            oldNode->left = newNode->right;
            if (newNode->right != nullptr)
            {
                newNode->right->parent = oldNode;
            }

            oldNode->parent = newNode;
            newNode->right = oldNode;
        }

        // 删除元素
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        void rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::erase(iterator pod)
        {
            if (empty())
            {
                return;
            }

            // 找到要删除的点直接前驱或者直接后继，这里选直接后继
            nodePointer deleteNode = pod.node;
            if (deleteNode->right != nullptr)
            {
                deleteNode = get_min(deleteNode->right);
            }
            // 将直接后继的值移过来，转换为删除直接后继
            *pod = deleteNode->value;
            if (deleteNode == head->left)
            {
                if (pod.node == head->left)
                {
                    head->left = (++pod).node;
                    --pod;
                }
                else
                {
                    head->left = pod.node;
                }
            }
            if (deleteNode == head->right)
            {
                if (pod.node == head->right)
                {
                    head->right = (--pod).node;
                    ++pod;
                }
                else
                {
                    head->right = pod.node;
                }
            }
            _erase(deleteNode);
            nodeCount--;
        }

        // 删除节点的内部实现
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        void rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::_erase(nodePointer dNode)
        {
            // 要删除根节点
            if (dNode == head->parent)
            {
                // 根节点还有子节点
                if (dNode->left != nullptr)
                {
                    dNode->left->parent = head;
                    head->parent = dNode->left;
                    dNode->left->color = rb_tree_black;
                    free_node(dNode);
                }
                else if (dNode->right != nullptr)
                {
                    dNode->right->parent = head;
                    head->parent = dNode->right;
                    dNode->right->color = rb_tree_black;
                    free_node(dNode);
                }
                else
                {
                    // 根节点没有子节点
                    head->parent = nullptr;
                    free_node(dNode);
                }
                return;
            }

            // 要删除的节点为红色，则这个节点必然没有子节点，直接删除
            if (dNode->color == rb_tree_red)
            {
                if (dNode->parent->left == dNode)
                {
                    dNode->parent->left = nullptr;
                }
                else
                {
                    dNode->parent->right = nullptr;
                }
                free_node(dNode);
                return;
            }

            // 节点为黑色，且至少有一个子节点，则这个子节点必然为红色，
            // 用子节点代替dnode后删除子节点
            if (!(dNode->left == nullptr && dNode->right == nullptr))
            {
                if (dNode->left != nullptr)
                { // dNode有左节点
                    dNode->left->parent = dNode->parent;
                    ((dNode->parent->left == dNode) ? (dNode->parent->left) : (dNode->parent->right)) = dNode->left;
                    dNode->left->color = rb_tree_black;
                    free_node(dNode);
                }
                else
                { // dNode有右节点
                    dNode->right->parent = dNode->parent;
                    ((dNode->parent->left == dNode) ? (dNode->parent->left) : (dNode->parent->right)) = dNode->right;
                    dNode->right->color = rb_tree_black;
                    free_node(dNode);
                }
                return;
            }

            // 节点为黑色，且没有子节点

            // 先判断是否要删除根节点
            if (dNode == head->parent)
            {
                // 删除根节点
                head->parent = nullptr;
                head->left = head;
                head->right = head;
                free_node(dNode);
                return;
            }

            // 需要看dNode的兄弟节点，兄弟节点为红时，将父节点与兄弟节点变色
            // 往dNode旋转，此后将变为dnode的兄弟节点为黑的情况

            auto father = dNode->parent;
            auto brother = ((dNode == father->left) ? (father->right) : (father->left));

            if (father->left == dNode && get_color(brother) == rb_tree_red)
            {
                // dNode为左子节点,且兄弟节点为红色
                father->color = rb_tree_red;
                brother->color = rb_tree_black;
                rotate_left(father);
                father = dNode->parent;
                brother = ((dNode == father->left) ? (father->right) : (father->left));
            }
            else if (father->right == dNode && get_color(brother) == rb_tree_red)
            {
                father->color = rb_tree_red;
                brother->color = rb_tree_black;
                rotate_right(father);
                father = dNode->parent;
                brother = ((dNode == father->left) ? (father->right) : (father->left));
            }

            // 到这里剩下的情况只有dNode为黑，兄弟为黑，此时需要查看兄弟节点的子节点中有没有红色节点
            // 如果没有红色节点，则将兄弟变红，删除dNode，将父节点标为双黑节点，后续消除双黑节点
            // 如果兄弟节点有至少一个红色节点，则根据ll，lr，rl，rr，进行旋转与变色

            // 兄弟节点没有红色子节点
            if (get_color(brother->left) == rb_tree_black && get_color(brother->right) == rb_tree_black)
            {
                brother->color = rb_tree_red;
                // 删除dnode节点
                if (father->left == dNode)
                {
                    father->left = nullptr;
                }
                else
                {
                    father->right = nullptr;
                }
                free_node(dNode);

                // 将father节点标为双黑
                _erase_double_black(father);
                return;
            }

            // 此时只剩下兄弟节点至少有一个红色子节点的情况

            if (father->left == brother) // 兄弟节点为父节点的左子节点
            {
                if (get_color(brother->left) == rb_tree_red) // 兄弟节点的左子树为红色，即ll型,
                {
                    // 调整的目的是使兄弟节点的左子树上的叶子节点到根节点的黑色节点数多出来一个
                    // 然后通过旋转将多出来的这个黑色节点转移到dNode所在子树，以弥补因删除dNode而缺少的一个黑色节点
                    // 具体做法是，保留father与brother的颜色，将他们往下平移，然后将father的颜色改为黑色
                    // 这样brother的红色子节点所在分支少了一个红色节点，多了一个黑色节点，旋转后可以维持红黑树的性质
                    brother->left->color = get_color(brother);
                    brother->color = get_color(father);
                    father->color = rb_tree_black;

                    father->right = nullptr;
                    free_node(dNode);
                    rotate_right(father);
                }
                else // 兄弟节点的右子树为红色，即lr型,
                {
                    // 将father的颜色赋给这个红色节点，然后将father变黑，
                    // 左旋brother，右旋father，相当于红色节点变为黑色后转到了右边，
                    // father节点的颜色经过两次旋转后回到原来的位置，维持了红黑树的性质
                    brother->right->color = get_color(father);
                    father->color = rb_tree_black;

                    father->right = nullptr;
                    free_node(dNode);
                    rotate_left(brother);
                    rotate_right(father);
                }
            }
            else // 兄弟节点为父节点的右子节点
            {
                if (get_color(brother->right) == rb_tree_red) // 兄弟节点的右子树为红色，即rr型,
                {
                    brother->right->color = get_color(brother);
                    brother->color = get_color(father);
                    father->color = rb_tree_black;

                    father->left = nullptr;
                    free_node(dNode);
                    rotate_left(father);
                }
                else // rl型
                {
                    brother->left->color = get_color(father);
                    father->color = rb_tree_black;

                    father->left = nullptr;
                    free_node(dNode);
                    rotate_right(brother);
                    rotate_left(father);
                }
            }
        } // end of _erase

        // 消除双黑节点
        template <class Key, class Value, class KeyOfValue, class Compare, class Alloc>
        void rb_tree<Key, Value, KeyOfValue, Compare, Alloc>::_erase_double_black(nodePointer dNode)
        {
            while (dNode != head->parent && get_color(dNode) == rb_tree_black)
            {
                auto father = dNode->parent;
                auto brother = ((dNode == father->left) ? (father->right) : (father->left));

                // brother为红色时：转换为brother为黑色
                if (father->left == dNode && get_color(brother) == rb_tree_red)
                {
                    // dNode为左子节点,且兄弟节点为红色
                    father->color = rb_tree_red;
                    brother->color = rb_tree_black;
                    rotate_left(father);
                    father = dNode->parent;
                    brother = ((dNode == father->left) ? (father->right) : (father->left));
                }
                else if (father->right == dNode && get_color(brother) == rb_tree_red)
                {
                    father->color = rb_tree_red;
                    brother->color = rb_tree_black;
                    rotate_right(father);
                    father = dNode->parent;
                    brother = ((dNode == father->left) ? (father->right) : (father->left));
                }

                // brother为黑色：

                // brother没有红色子节点
                if (get_color(brother->left) == rb_tree_black && get_color(brother->right) == rb_tree_black)
                {
                    brother->color = rb_tree_red;

                    // 将father节点标为双黑
                    dNode = father;
                    continue;
                }

                // 此时只剩下兄弟节点至少有一个红色子节点的情况

                if (father->left == brother) // 兄弟节点为父节点的左子节点
                {
                    if (get_color(brother->left) == rb_tree_red) // 兄弟节点的左子树为红色，即ll型,
                    {
                        // 调整的目的是使兄弟节点的左子树上的叶子节点到根节点的黑色节点数多出来一个
                        // 然后通过旋转将多出来的这个黑色节点转移到dNode所在子树，以弥补因删除dNode而缺少的一个黑色节点
                        // 具体做法是，保留father与brother的颜色，将他们往下平移，然后将father的颜色改为黑色
                        // 这样brother的红色子节点所在分支少了一个红色节点，多了一个黑色节点，旋转后可以维持红黑树的性质
                        brother->left->color = get_color(brother);
                        brother->color = get_color(father);
                        father->color = rb_tree_black;

                        rotate_right(father);
                        return;
                    }
                    else // 兄弟节点的右子树为红色，即lr型,
                    {
                        // 将father的颜色赋给这个红色节点，然后将father变黑，
                        // 左旋brother，右旋father，相当于红色节点变为黑色后转到了右边，
                        // father节点的颜色经过两次旋转后回到原来的位置，维持了红黑树的性质
                        brother->right->color = get_color(father);
                        father->color = rb_tree_black;

                        rotate_left(brother);
                        rotate_right(father);
                        return;
                    }
                }
                else // 兄弟节点为父节点的右子节点
                {
                    if (get_color(brother->right) == rb_tree_red) // 兄弟节点的右子树为红色，即rr型,
                    {
                        brother->right->color = get_color(brother);
                        brother->color = get_color(father);
                        father->color = rb_tree_black;

                        rotate_left(father);
                        return;
                    }
                    else // rl型
                    {
                        brother->left->color = get_color(father);
                        father->color = rb_tree_black;

                        rotate_right(brother);
                        rotate_left(father);
                        return;
                    }
                }
            } // end of while

            // 此时dNode为红色或者为根节点
            dNode->color = rb_tree_black;
        } // end of _erase_double_black

    } // end of detail

} // end of qyk

#endif