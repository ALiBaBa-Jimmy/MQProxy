/************************************************************************************
�ļ���  :cpp_tcn_list.h

�ļ�����:������

����    :zzt

��������:2006/4/27

�޸ļ�¼:
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

    //ע�⣬��ôʵ�������ǳ������͵ĵ�������ֵ���������͵ĵ�����
    //��֮ȴ������
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

    /*˫��������֧�����������*/
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
        //�����Ƿ�Ϊ�գ�����ֻ��һ��Ԫ��
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
        link_type node;//ͷ�ڵ�
        CAllocRecord m_rec; // ����������
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

        //ɾ��һ���ڵ㣬��ɾ������END�㣬�����ó�����Ϊδ����
        iterator erase(iterator position )
        {
            //ɾ��END�㣬ֱ�ӷ��ؽ�����������END��
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
    public://���캯���ӿ�

        //֧���ڴ�Ԥ����汾��LIST,tMaxCntԤ����LIST�д�ŵ�Ԫ�صĸ���
        list(size_type tMaxCnt)
            :node_alloctor(tMaxCnt+1,eNodeSize), m_rec(0xFFFFFFFF) //+1����Ϊ��Ҫһ��ͷ�ڵ�
        {
            empty_initialize();
        }

        //ʹ�÷�Ԥ����Ľӿ�
        list()
            :node_alloctor(eNodeSize), m_rec(0xFFFFFFFF)
        {
            empty_initialize();
        }

        //��һ����Χ�ڲ���Ԫ�أ�ʹ���ڴ�Ԥ����
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
        //��һ����Χ�ڲ���Ԫ�أ���ʹ���ڴ�Ԥ����
        template <class InputIterator>
        list(InputIterator first, InputIterator last)
            :node_alloctor(eNodeSize), m_rec(0xFFFFFFFF)
        {
            //����VC���ڳ�Աģ���е�����һ����Աģ����÷�֧�ֲ��ã������������һ�����ظ�����
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

        // �����ڴ�Ԥ����,�û������ڹ��캯���в��ṩԤ�����С,
        // ����ʹ��reserve���±�����С,ʹ�÷�����vector����
        // add by liusj
        XVOID reserve(size_type n)
        {
            // �����ǰ�Ľڵ�
            clear();
            put_node(node);

            // ���·���ռ�
            node_alloctor.reserve(n + 1);

            // ��ʼ��
            empty_initialize();
        }

        list<T, Alloc>& operator=(const list<T, Alloc>& x);

    protected:
        //��[first,last)��Χ�е�Ԫ���Ƶ�position֮ǰ��STL�ĵ�����һ��ʹǰ�պ�����
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
        //�������ڴ�Ԥ���䣬�����slplice�����ǷǷ���
        //�������ڴ�Ԥ���䣬�����merge�����ǷǷ���
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

        // �����Ż�
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
    //�ú�����Ҫ��д
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
    //ɾ�������е�����ֵ����value�Ľڵ�
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
    //ʹ�����������ڵ�Ԫ��û����ͬ��
    template <class T, class Alloc> XVOID list<T, Alloc>::unique()
    {

        //��ǰ����Ϊ�գ���������ֻ��һ���ڵ�Ͳ��øò���
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
    //ʹ�ò�������.
    template <class T, class Alloc> XVOID list<T, Alloc>::sort()
    {
        //����Ϊ�ջ���������ֻ��һ��Ԫ��
        if(empty_or_one())
        {
            return ;
        }

        //it_pst ָ��ǰ��Ҫ������Ľڵ�.��ʵ�����Ѿ�������һ��Ԫ�ص����
        iterator it_pst = begin();
        ++it_pst;
        //ʹ�ò�������
        while(end() != it_pst)
        {
            //it_order_nodeָ��ǰ������Ľڵ�
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


