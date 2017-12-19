/************************************************************************************
文件名  :cpp_tcn_index_list.h

文件描述:索引链表定义

作者    :zzt

创建日期:2005/10/28

修改记录:

************************************************************************************/
#ifndef _CPP_TCN_INDEX_LIST_H_
#define _CPP_TCN_INDEX_LIST_H_

#include "cpp_prealloc.h"

namespace tcn
{

/*********************************************************************
名称 : template <class T>struct ListNode
职责 : 索引链表的节点类型
协作 :
历史 :
       修改者   日期          描述
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
名称 : template<class T, class Ref, class Ptr>  ListIterator
职责 : 索引链表的迭代器
协作 :
历史 :
       修改者   日期          描述
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
    //该拷贝构造必须满足const_iterator 类型的对象不能向iterator类型的对象拷贝
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
名称 : template<class T>   CIndexList
职责 : 1.提供和STL LIST 一致的接口(本版本中只提供了一部分
           STL LIST的接口),便于代码互换

       2.提供快速的插入和删除操作.避免内存碎片.与STL LIST
           不一样的是,在插入节点时可能导致内存重新分配,
           这一点和VECTOR相似,但也有不同,VECTOR重新分配内存后
           会使以前定义的迭代器失效,但CIndexList在重新分配内存
           后以前定义的迭代器依然有效

       3.当前暂不提供拷贝和赋值操作

       4.VC STL和SGI STL体系不兼容,所以本容器不使用STL的配置器
          直接调用opeator new 和operator delete管理内存
协作 :
历史 :
       修改者   日期          描述
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
    typedef ListNode<T>         node_type ;//节点类型
    typedef typename node_type::pointer link_type;//链类型
    typedef link_type           size_type;
protected:
    enum {eNodeSize = sizeof(node_type)};
public:
    //预分配可以存放cursize个元素的内存
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
    //拷贝构造函数.added by zzt 2006-02-24
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
    //在position之前插入节点
    iterator insert(iterator position,const T& x)
    {
        //构造新节点,并获得新节点的索引
        link_type tmp = create_node(x);
        set_next(tmp, position.cur);
        //若链表为空
        if(empty())
        {
            set_prev(tmp,m_bh);
            m_bh = tmp;
            m_bt  = tmp;
            ++m_size;
            return iterator(this,tmp);
        }

        //在END节点之前插入,就是将其插入到尾节点之后
        if(position.cur == LIST_INVALID_INDEX)
        {
            set_prev(tmp,m_bt);
            set_next(m_bt,tmp);
            m_bt = tmp;
            ++m_size;
            return iterator(this,tmp);
        }
        //头节点之前插入
        if(position.cur== m_bh)
        {
            set_prev(tmp, LIST_INVALID_INDEX);
            set_prev(m_bh,tmp);
            m_bh = tmp;
            ++m_size;
            return iterator(this,tmp);
        }
        //在中间节点前插入
        set_prev(tmp,get_prev(position.cur));
        set_prev(position.cur,tmp);
        set_next(get_prev(tmp),tmp);
        ++m_size;
        return iterator(this,tmp);
    }
    //删除节点,然后返回节点的下一个位置
    iterator erase(iterator position)
    {
        //删除的是end点
        if(position.cur == LIST_INVALID_INDEX)
        {
            return position;
        }
        link_type next  = get_next(position.cur);
        link_type prev  = get_prev(position.cur);
        //若删除的是尾节点
        if(next ==LIST_INVALID_INDEX)
        {
            m_bt = prev;
        }
        //若删除的是头节点
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
        //释放资源
        DestroyT(&*position);
        //删除操作完成,元素个数递减
        --m_size;
        //将节点放入空闲链表
        set_next(position.cur,m_fi);
        m_fi = position.cur;
        //返回下一个节点的迭代器
        return iterator(this,next);
    }
    XVOID clear()
    {
        //释放所有节点
        destory_node();
        list_init();
    }
    //清除并释放内存
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
        //若数组的容量大于链表中元素的数目则不扩充容量
        if(array_capacity() >   size())
        {
            return ;
        }
        const size_type new_capacity = ( size() != 0?  2* size():1);//扩容算法与SGI STL vector一样
        node_type* pNew = array_malloc(new_capacity);
        //按链表中的元素的顺序依次拷贝到新位置
        for(size_type i = m_bh; i  != LIST_INVALID_INDEX ; i = get_node(i)->next)
        {
            ConstructT(pNew+i,*get_node(i));
        }

        destory_node(); //调用m_buffer中存放的T的析构函数
        array_free();
        //重新初始化数组指针成员
        m_pStart   = pNew;
        m_pEnd     = m_pStart+new_capacity;
    }

    //得到一个得到并构造一个节点,次节点还未添加到链表中
    link_type create_node(const T& value)
    {
        link_type tmp =LIST_INVALID_INDEX ;
        if(m_fi == LIST_INVALID_INDEX)
        {
            array_extend_memory();
            tmp = size();//这里的size是链表的长度,不包括本次调用create_node创造的节点
        }
        else
        {
            tmp = m_fi ;
            m_fi = get_next(m_fi);
        }
        ConstructT( &get_node(tmp)->data,value);
        return tmp;
    }
    //分配N个节点的内存空间
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
    //暂不提供赋值操作
    CIndexList& operator = (const CIndexList& other);
private:
    Alloc        m_alloc;
private:
    //连续内存区
    node_type*   m_pStart;//起始地址
    node_type*   m_pEnd;//结束地址的下一地址内存区间是这样一个区间[m_pStart,m_pEnd)
    //在连续内存块上成链
    link_type    m_fi;//空闲指针,
    link_type    m_bh;//忙头指针指针
    link_type    m_bt;//忙尾指针
    size_type    m_size;//元素个数
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

