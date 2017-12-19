/************************************************************************************
�ļ���  :cpp_tcn_index_list.h

�ļ�����:����������

����    :zzt

��������:2005/10/28

�޸ļ�¼:

************************************************************************************/
#ifndef _CPP_TCN_INDEX_LIST_H_
#define _CPP_TCN_INDEX_LIST_H_

#include "cpp_prealloc.h"

namespace tcn
{

/*********************************************************************
���� : template <class T>struct ListNode
ְ�� : ��������Ľڵ�����
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
#define LIST_INVALID_INDEX -1

template <class T>struct ListNode
{
    typedef XS32  pointer;
    pointer next;
    pointer prev;
    T data;
};

template<class T,class Alloc = CAllocator >  class  CIndexList;


/*********************************************************************
���� : template<class T, class Ref, class Ptr>  ListIterator
ְ�� : ��������ĵ�����
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
template<class T, class Ref, class Ptr>
struct ListIterator
{
    typedef ListIterator<T, T&, T*>                       iterator;
    typedef ListIterator<T, const T&, const T*>           const_iterator;
    typedef ListIterator<T, Ref, Ptr>                     self;
    typedef std::bidirectional_iterator_tag               iterator_category;
    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef typename ListNode<T>::pointer link_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    link_type             cur;
    CIndexList<T>        *pvct;
    //�ÿ��������������const_iterator ���͵Ķ�������iterator���͵Ķ��󿽱�
    ListIterator(const iterator& x)
    {
        cur  = x.cur;
        pvct = x.pvct;
    }
    bool operator==(const self& x) const
    {
        return (cur == x.cur)&&(pvct == x.pvct);
    }
    bool operator!=(const self& x) const
    {
        return !operator==(x);
    }
    Ref  operator*() const;
    Ptr  operator->() const
    {
        return &(operator*());
    }
    self&  operator++();
    self  operator++(XS32)
    {
        self tmp = *this;
        operator++();
        return tmp;
    }
    self&   operator--();
    self  operator--(XS32)
    {
        self  tmp = *this;
        operator--();
        return tmp;
    }
    link_type GetIndex()const
    {
        return cur;
    }
    ListIterator(CIndexList<T> * t,link_type x)
    {
        cur  = x;
        pvct = t;
    }
    ListIterator()
    {
        pvct = XNULL;
        cur  = LIST_INVALID_INDEX;
    }
};

/*********************************************************************
���� : template<class T>   CIndexList
ְ�� : 1.�ṩ��STL LIST һ�µĽӿ�(���汾��ֻ�ṩ��һ����
           STL LIST�Ľӿ�),���ڴ��뻥��

       2.�ṩ���ٵĲ����ɾ������.�����ڴ���Ƭ.��STL LIST
           ��һ������,�ڲ���ڵ�ʱ���ܵ����ڴ����·���,
           ��һ���VECTOR����,��Ҳ�в�ͬ,VECTOR���·����ڴ��
           ��ʹ��ǰ����ĵ�����ʧЧ,��CIndexList�����·����ڴ�
           ����ǰ����ĵ�������Ȼ��Ч

       3.��ǰ�ݲ��ṩ�����͸�ֵ����

       4.VC STL��SGI STL��ϵ������,���Ա�������ʹ��STL��������
          ֱ�ӵ���opeator new ��operator delete�����ڴ�
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
template<class T,class Alloc>
class  CIndexList
{
    friend struct ListIterator<T, T&, T*>;
    friend struct ListIterator<T, const T&, const T*>;
public:
    typedef T                   value_type;
    typedef value_type*         pointer;
    typedef const value_type*   const_pointer;
    typedef value_type&         reference;
    typedef const value_type&   const_reference;
    typedef ptrdiff_t           difference_type;
    typedef ListIterator<T, T&, T*>             iterator;
    typedef ListIterator<T, const T&, const T*> const_iterator;
    typedef ListNode<T>         node_type ;//�ڵ�����
    typedef typename node_type::pointer link_type;//������
    typedef link_type           size_type;
protected:
    enum {eNodeSize = sizeof(node_type)};
public:
    //Ԥ������Դ��cursize��Ԫ�ص��ڴ�
    explicit   CIndexList(size_type  CurSize=0)
        :m_alloc(eNodeSize)
    {
        array_init(CurSize);
        list_init();
    }
    ~CIndexList()
    {
        destory_node();
        array_free();

    }
    //�������캯��.added by zzt 2006-02-24
    CIndexList(const CIndexList& other)
    {
        array_init(other.size());
        list_init();
        for(const_iterator it = other.begin(); it != other.end(); ++it)
        {
            push_back(*it);
        }
    }

    iterator begin()
    {
        return iterator(this,m_bh);
    }
    const_iterator begin() const
    {
        return const_iterator(get_noconst_this(),m_bh);
    }
    iterator end()
    {
        return iterator(this,LIST_INVALID_INDEX);
    }
    const_iterator end() const
    {
        return  const_iterator(get_noconst_this(),LIST_INVALID_INDEX);
    }
    bool empty() const
    {
        // return begin() == end();
        return size() ==0;
    }
    size_type size() const
    {
        return m_size;
    }
    //��position֮ǰ����ڵ�
    iterator insert(iterator position,const T& x)
    {
        //�����½ڵ�,������½ڵ������
        link_type tmp = create_node(x);
        set_next(tmp, position.cur);
        //������Ϊ��
        if(empty())
        {
            set_prev(tmp,m_bh);
            m_bh = tmp;
            m_bt  = tmp;
            ++m_size;
            return iterator(this,tmp);
        }

        //��END�ڵ�֮ǰ����,���ǽ�����뵽β�ڵ�֮��
        if(position.cur == LIST_INVALID_INDEX)
        {
            set_prev(tmp,m_bt);
            set_next(m_bt,tmp);
            m_bt = tmp;
            ++m_size;
            return iterator(this,tmp);
        }
        //ͷ�ڵ�֮ǰ����
        if(position.cur== m_bh)
        {
            set_prev(tmp, LIST_INVALID_INDEX);
            set_prev(m_bh,tmp);
            m_bh = tmp;
            ++m_size;
            return iterator(this,tmp);
        }
        //���м�ڵ�ǰ����
        set_prev(tmp,get_prev(position.cur));
        set_prev(position.cur,tmp);
        set_next(get_prev(tmp),tmp);
        ++m_size;
        return iterator(this,tmp);
    }
    //ɾ���ڵ�,Ȼ�󷵻ؽڵ����һ��λ��
    iterator erase(iterator position)
    {
        //ɾ������end��
        if(position.cur == LIST_INVALID_INDEX)
        {
            return position;
        }
        link_type next  = get_next(position.cur);
        link_type prev  = get_prev(position.cur);
        //��ɾ������β�ڵ�
        if(next ==LIST_INVALID_INDEX)
        {
            m_bt = prev;
        }
        //��ɾ������ͷ�ڵ�
        if(prev ==LIST_INVALID_INDEX )
        {
            m_bh = next;
        }
        if(prev != LIST_INVALID_INDEX )
        {
            set_next(prev,next);
        }
        if(next !=LIST_INVALID_INDEX)
        {
            set_prev(next,prev);
        }
        //�ͷ���Դ
        DestroyT(&*position);
        //ɾ���������,Ԫ�ظ����ݼ�
        --m_size;
        //���ڵ�����������
        set_next(position.cur,m_fi);
        m_fi = position.cur;
        //������һ���ڵ�ĵ�����
        return iterator(this,next);
    }
    XVOID clear()
    {
        //�ͷ����нڵ�
        destory_node();
        list_init();
    }
    //������ͷ��ڴ�
    XVOID clear_and_free()
    {
        clear();
        array_free();
    }

    XVOID push_front(const T& x)
    {
        insert(begin(), x);
    }
    XVOID push_back(const T& x)
    {
        insert(end(), x);
    }
    XVOID pop_front()
    {
        erase(begin());
    }
    XVOID pop_back()
    {
        iterator tmp = end();
        erase(--tmp);
    }

    reference front()
    {
        return *begin();
    }
    const_reference front()const
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

private:
    size_type capacity()const
    {
        return array_capacity();
    }
    XVOID destory_node()
    {
        for(iterator iter = begin(); iter != end(); ++iter)
        {
            DestroyT(&*iter);
        }
    }
    CIndexList * get_noconst_this() const
    {
        return const_cast<CIndexList * const>(this);
    }

    XVOID set_next(link_type i,link_type next)
    {
        get_node(i)->next = next;
    }
    XVOID set_prev(link_type i,link_type prev)
    {
        get_node(i)->prev = prev;
    }
    link_type get_next(link_type i)const
    {
        return get_node(i)->next;
    }
    link_type get_prev(link_type i)const
    {
        return get_node(i)->prev;
    }
    XVOID list_init()
    {

        m_bh   = LIST_INVALID_INDEX;
        m_bt   = LIST_INVALID_INDEX;
        m_size = 0;
        m_fi   = LIST_INVALID_INDEX;
    }

    XVOID   array_extend_memory()
    {
        //���������������������Ԫ�ص���Ŀ����������
        if(array_capacity() >   size())
        {
            return ;
        }
        const size_type new_capacity = ( size() != 0?  2* size():1);//�����㷨��SGI STL vectorһ��
        node_type* pNew = array_malloc(new_capacity);
        //�������е�Ԫ�ص�˳�����ο�������λ��
        for(size_type i = m_bh; i  != LIST_INVALID_INDEX ; i = get_node(i)->next)
        {
            ConstructT(pNew+i,*get_node(i));
        }

        destory_node(); //����m_buffer�д�ŵ�T����������
        array_free();
        //���³�ʼ������ָ���Ա
        m_pStart   = pNew;
        m_pEnd     = m_pStart+new_capacity;
    }

    //�õ�һ���õ�������һ���ڵ�,�νڵ㻹δ��ӵ�������
    link_type create_node(const T& value)
    {
        link_type tmp =LIST_INVALID_INDEX ;
        if(m_fi == LIST_INVALID_INDEX)
        {
            array_extend_memory();
            tmp = size();//�����size������ĳ���,���������ε���create_node����Ľڵ�
        }
        else
        {
            tmp = m_fi ;
            m_fi = get_next(m_fi);
        }
        ConstructT( &get_node(tmp)->data,value);
        return tmp;
    }
    //����N���ڵ���ڴ�ռ�
    node_type* array_malloc(size_type  num)
    {
        return num == 0 ?  XNULL:(node_type*) m_alloc.allocate(num);
    }
    XVOID array_free()
    {
        if(m_pStart != XNULL)
        {
            m_alloc.deallocate(m_pStart);
            m_pStart = XNULL;
        }
        m_pEnd = XNULL;
    }

    XVOID array_init(size_type  num)
    {
        m_pStart   =  array_malloc(num)   ;
        m_pEnd     = m_pStart+num;
    }
    node_type * get_node(link_type i)const
    {
        CPP_ASSERT_RV((i<array_capacity()), XNULL);
        return m_pStart+i;
    }
    size_type  array_capacity()const
    {
        return m_pEnd - m_pStart;
    }
    //�ݲ��ṩ��ֵ����
    CIndexList& operator = (const CIndexList& other);
private:
    Alloc        m_alloc;
private:
    //�����ڴ���
    node_type*   m_pStart;//��ʼ��ַ
    node_type*   m_pEnd;//������ַ����һ��ַ�ڴ�����������һ������[m_pStart,m_pEnd)
    //�������ڴ���ϳ���
    link_type    m_fi;//����ָ��,
    link_type    m_bh;//æͷָ��ָ��
    link_type    m_bt;//æβָ��
    size_type    m_size;//Ԫ�ظ���
};


template<class T, class Ref, class Ptr>
typename ListIterator<T,Ref,Ptr>::self& ListIterator<T,Ref,Ptr>::operator++()
{
    if( cur != LIST_INVALID_INDEX)
    {
        cur = pvct->get_next(cur);
    }
    return *this;
}

template<class T, class Ref, class Ptr>
typename ListIterator<T,Ref,Ptr>::self& ListIterator<T,Ref,Ptr>::operator--()
{
    cur = ((cur ==LIST_INVALID_INDEX) ? pvct->m_bt:pvct->get_prev(cur));
    return *this;
}

template<class T, class Ref, class Ptr>  Ref  ListIterator<T,Ref,Ptr>::operator*() const
{
    return  pvct->get_node(cur)->data;
}

}


#endif

