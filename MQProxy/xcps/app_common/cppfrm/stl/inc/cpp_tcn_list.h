/************************************************************************************
文件名  :cpp_tcn_list.h

文件描述:链表定义

作者    :zzt

创建日期:2006/4/27

修改记录:
************************************************************************************/
#ifndef __CPP_TCN_LIST_H_
#define __CPP_TCN_LIST_H_

#include "cpp_prealloc.h"



namespace tcn
{
    template <class T> struct __list_node
    {
        typedef XVOID* void_pointer;
        void_pointer next;
        void_pointer prev;
        T data;
    };

    //注意，怎么实现允许将非常量类型的迭代器赋值给常量类型的迭代器
    //反之却不可以
    template<class T, class Ref, class Ptr>
    struct __list_iterator
    {
        typedef __list_iterator<T, T&, T*>             iterator;
        typedef __list_iterator<T, const T&, const T*> const_iterator;
        typedef __list_iterator<T, Ref, Ptr>           self;

        typedef std::bidirectional_iterator_tag iterator_category;
        typedef T   value_type;
        typedef Ptr pointer;
        typedef Ref reference;
        typedef __list_node<T>* link_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        link_type node;

        __list_iterator(link_type x) : node(x)
        {
        }
        __list_iterator()
        {
        }
        __list_iterator(const iterator& x)
            :node(x.node)
        {
        }
        bool operator==(const self& x) const
        {
            return node == x.node;
        }
        bool operator!=(const self& x) const
        {
            return node != x.node;
        }
        reference operator*() const
        {
            return (*node).data;
        }
        pointer operator->() const
        {
            return &(operator*());
        }
        self& operator++()
        {
            node = (link_type)((*node).next);
            return *this;
        }
        self operator++(XS32)
        {
            self tmp = *this;
            ++*this;
            return tmp;
        }
        self& operator--()
        {
            node = (link_type)((*node).prev);
            return *this;
        }
        self operator--(XS32)
        {
            self tmp = *this;
            --*this;
            return tmp;
        }
    };

    /*双先链表，不支持逆向迭代器*/
    template <class T, class Alloc = CAllocator> class list
    {
    protected:
        typedef XVOID*           void_pointer;
        typedef __list_node<T>  list_node;
        enum {eNodeSize = sizeof(list_node)};
    public:
        typedef T                   value_type;
        typedef value_type*         pointer;
        typedef const value_type*   const_pointer;
        typedef value_type&         reference;
        typedef const value_type&   const_reference;
        typedef list_node*          link_type;
        typedef size_t              size_type;
        typedef ptrdiff_t           difference_type;

    public:
        typedef __list_iterator<T, T&, T*>             iterator;
        typedef __list_iterator<T, const T&, const T*> const_iterator;
    protected:
        link_type get_node()
        {
            return static_cast<link_type>(node_alloctor.allocate());
        }
        XVOID put_node(link_type p)
        {
            node_alloctor.deallocate(p);
        }
        link_type create_node(const T& x)
        {
            link_type p = get_node();
            ConstructT(&p->data, x);
            return p;
        }
        XVOID destroy_node(link_type p)
        {
            DestroyT(&p->data);
            put_node(p);
        }
        //链表是否为空，或者只有一个元素
        bool empty_or_one()const
        {
            if (node->next == node || link_type(node->next)->next == node)
            {
                return true;
            }
            return false;
        }

    protected:
        XVOID empty_initialize()
        {
            node = get_node();
            node->next = node;
            node->prev = node;
        }

        XVOID fill_initialize(size_type n, const T& value)
        {
            empty_initialize();
            for(size_type i = 0; i < n; ++i)
            {
                insert(begin(),value);
            }


        }
        template <class InputIterator>
        XVOID range_initialize(InputIterator first, InputIterator last)
        {
            empty_initialize();
            for ( ; first != last; ++first)
            {
                insert(end(), *first);
            }

        }
    protected:
        Alloc     node_alloctor;
        link_type node;//头节点
        CAllocRecord m_rec; // 个数计数器
    public:
        iterator begin()
        {
            return (link_type)((*node).next);
        }
        const_iterator begin() const
        {
            return (link_type)((*node).next);
        }
        iterator end()
        {
            return node;
        }
        const_iterator end() const
        {
            return node;
        }

        bool empty() const
        {
            return node->next == node;
        }
        size_type size() const
        {
            return m_rec.GetAlloced();
        }
        size_type max_size() const
        {
            return node_alloctor.max_size() - 1;
        }
        reference front()
        {
            return *begin();
        }
        const_reference front() const
        {
            return *begin();
        }
        reference back()
        {
            return *(--end());
        }
        const_reference back() const
        {
            return *(--end());
        }
        XVOID swap(list<T, Alloc>& x)
        {
            std::swap(node, x.node);
            node_alloctor.swap(x.node_alloctor);
        }

        iterator insert(iterator position, const T& x)
        {
            link_type tmp = create_node(x);
            tmp->next = position.node;
            tmp->prev = position.node->prev;
            (link_type(position.node->prev))->next = tmp;
            position.node->prev = tmp;
            m_rec.IncrementCount();
            return tmp;
        }

        XVOID push_front(const T& x)
        {
            insert(begin(), x);
        }
        XVOID push_back(const T& x)
        {
            insert(end(), x);
        }

        //删除一个节点，若删除的是END点，不能让程序行为未定义
        iterator erase(iterator position )
        {
            //删除END点，直接返回将迭代器返回END点
            if(position.node == node)
            {
                return end();
            }

            link_type next_node = link_type(position.node->next);
            link_type prev_node = link_type(position.node->prev);
            prev_node->next = next_node;
            next_node->prev = prev_node;
            destroy_node(position.node);
            m_rec.DecrementCount();
            return iterator(next_node);
        }
        iterator erase(iterator first, iterator last)
        {
            while (first != last)
            {
                erase(first++);
            }
            return last;
        }
        XVOID clear();
        XVOID pop_front()
        {
            erase(begin());
        }
        XVOID pop_back()
        {
            iterator tmp = end();
            erase(--tmp);
        }
    public://构造函数接口

        //支持内存预分配版本的LIST,tMaxCnt预计在LIST中存放的元素的个数
        list(size_type tMaxCnt)
            :node_alloctor(tMaxCnt+1,eNodeSize), m_rec(0xFFFFFFFF) //+1是因为需要一个头节点
        {
            empty_initialize();
        }

        //使用非预分配的接口
        list()
            :node_alloctor(eNodeSize), m_rec(0xFFFFFFFF)
        {
            empty_initialize();
        }

        //在一定范围内插入元素，使用内存预分配
        template <class InputIterator>
        list(InputIterator first, InputIterator last,size_type tMaxCnt)
            :node_alloctor(tMaxCnt+1,eNodeSize), m_rec(0xFFFFFFFF)
        {
            empty_initialize();
            for ( ; first != last; ++first)
            {
                insert(end(), *first);
            }
        }
        //在一定范围内插入元素，不使用内存预分配
        template <class InputIterator>
        list(InputIterator first, InputIterator last)
            :node_alloctor(eNodeSize), m_rec(0xFFFFFFFF)
        {
            //由于VC对在成员模板中调用另一个成员模板的用法支持不好，所以这里造成一定的重复代码
            empty_initialize();
            for ( ; first != last; ++first)
            {
                insert(end(), *first);
            }

        }
        list(const list<T, Alloc>& x)
            :node_alloctor(x.node_alloctor), m_rec(0xFFFFFFFF)
        {
            range_initialize(x.begin(), x.end());
        }

        ~list()
        {
            clear();
            put_node(node);
        }

        // 用于内存预分配,用户可以在构造函数中不提供预分配大小,
        // 接着使用reserve重新保留大小,使用方法和vector类似
        // add by liusj
        XVOID reserve(size_type n)
        {
            // 清除以前的节点
            clear();
            put_node(node);

            // 重新分配空间
            node_alloctor.reserve(n + 1);

            // 初始化
            empty_initialize();
        }

        list<T, Alloc>& operator=(const list<T, Alloc>& x);

    protected:
        //将[first,last)范围中的元素移到position之前。STL的迭代器一般使前闭后开区间
        XVOID transfer(iterator position, iterator first, iterator last)
        {
            if (position != last)
            {
                (*(link_type((*last.node).prev))).next = position.node;
                (*(link_type((*first.node).prev))).next = last.node;
                (*(link_type((*position.node).prev))).next = first.node;
                link_type tmp = link_type((*position.node).prev);
                (*position.node).prev = (*last.node).prev;
                (*last.node).prev = (*first.node).prev;
                (*first.node).prev = tmp;
            }
        }

    public:
        //若链表内存预分配，则调用slplice操作是非法的
        //若链表内存预分配，则调用merge操作是非法的
        XVOID remove(const T& value);
        XVOID unique();
        XVOID reverse();
        XVOID sort();

    #if defined (XOS_LINUX) ||defined (XOS_WIN32)
        friend bool operator==(const list& x, const list& y);
    #else
        friend bool operator==<T, Alloc>(const list<T,Alloc>& x, const list<T,Alloc>& y);
    #endif
    };

    template <class T, class Alloc>
    inline bool operator==(const list<T,Alloc>& x, const list<T,Alloc>& y)
    {
        typedef typename list<T,Alloc>::link_type link_type;

        // 性能优化
        if (x.size() != y.size())
        {
            return false;
        }

        link_type e1 = x.node;
        link_type e2 = y.node;
        link_type n1 = (link_type) e1->next;
        link_type n2 = (link_type) e2->next;
        for ( ; n1 != e1 && n2 != e2 ; n1 = (link_type) n1->next, n2 = (link_type) n2->next)
        {
            if (n1->data != n2->data)
            {
                return false;
            }
        }
        return n1 == e1 && n2 == e2;
    }


    template <class T, class Alloc> XVOID list<T, Alloc>::clear()
    {
        link_type cur = (link_type) node->next;
        while (cur != node)
        {
            link_type tmp = cur;
            cur = (link_type) cur->next;
            destroy_node(tmp);
            m_rec.DecrementCount();
        }
        node->next = node;
        node->prev = node;
    }
    //该函数需要改写
    template <class T, class Alloc>
    list<T, Alloc>& list<T, Alloc>::operator=(const list<T, Alloc>& x)
    {
        if(this != &x)
        {
            iterator first1 = begin();
            iterator last1  = end();
            const_iterator first2 = x.begin();
            const_iterator last2  = x.end();
            while (first1 != last1 && first2 != last2)
            {
                *first1++ = *first2++;
            }

            if(first2 == last2)
            {
                erase(first1, last1);
            }
            else
            {
                for(; first2 != last2; ++first2)
                {
                    insert(last1, *first2);
                }
            }
        }
        return *this;
    }
    //删除链表中的所有值等于value的节点
    template <class T, class Alloc> XVOID list<T, Alloc>::remove(const T& value)
    {
        iterator first = begin();
        iterator last = end();
        while (first != last)
        {
            iterator next = first;
            ++next;
            if(*first == value)
            {
                erase(first);
            }
            first = next;
        }
    }
    //使连续两个相邻的元素没有相同的
    template <class T, class Alloc> XVOID list<T, Alloc>::unique()
    {

        //当前链表为空，或者链表只有一个节点就不用该操作
        if(empty_or_one())
        {
            return ;
        }
        iterator first = begin();
        iterator last  = end();
        iterator next  = first;
        while (++next != last)
        {
            if(*first == *next)
            {
                erase(first);
            }
            first = next;
        }
    }

    template <class T, class Alloc> XVOID list<T, Alloc>::reverse()
    {
        if(empty_or_one())
        {
            return;
        }
        iterator first = begin();
        ++first;
        while (first != end())
        {
            iterator old = first;
            ++first;
            transfer(begin(), old, first);
        }
    }
    //使用插入排序.
    template <class T, class Alloc> XVOID list<T, Alloc>::sort()
    {
        //链表为空或者链表中只有一个元素
        if(empty_or_one())
        {
            return ;
        }

        //it_pst 指向当前将要被排序的节点.其实这里已经处理了一个元素的情况
        iterator it_pst = begin();
        ++it_pst;
        //使用插入排序
        while(end() != it_pst)
        {
            //it_order_node指向当前被排序的节点
            iterator it_order_node = it_pst;
            ++it_order_node;
            iterator it_cur = begin();
            while(it_cur != it_pst)
            {
                if(*it_pst < *it_cur)
                {
                    transfer(it_cur,it_pst,it_order_node);
                    break;
                }
                ++it_cur;
            }
            it_pst = it_order_node;
        }
    }

}//namespace std end

#endif


