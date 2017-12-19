/****************************************************************************
文件名  :cpp_tcn_hashtable.h

文件描述:

作者    :yuxiao

创建日期:2006/3/29

修改记录:
         ZZT 20060531 去掉不必要的功能，添加按迭代器删除的的接口
*************************************************************************/
#ifndef __CPP_TCN_HASHTABLE_H_
#define __CPP_TCN_HASHTABLE_H_

#include "cpp_prealloc.h"


using namespace std;

//namespace tcn
//{
template <class Value> struct __hashtable_node
{
    __hashtable_node* next;
    Value val;
};

//hash表类的前置声明
template <class Value, class Key, class HashFcn,
         class ExtractKey, class EqualKey, class Alloc> class hashtable;


//hash表的非常量迭代器前置声明
template <class Value, class Key, class HashFcn,
         class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator;

//hash表的常量迭代器前置声明
template <class Value, class Key, class HashFcn,
         class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_const_iterator;

/****************************************************************/
//迭代器
template <class Value, class Key, class HashFcn,
         class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator
{
    typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
    _hashtable;
    typedef __hashtable_iterator<Value, Key, HashFcn,
            ExtractKey, EqualKey, Alloc>
            iterator;
    typedef __hashtable_const_iterator<Value, Key, HashFcn,
            ExtractKey, EqualKey, Alloc>
            const_iterator;
    typedef __hashtable_node<Value> node;

    typedef forward_iterator_tag iterator_category;
    typedef Value value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef Value& reference;
    typedef Value* pointer;

    __hashtable_iterator(node* n, _hashtable* tab)
        : cur(n), ht(tab)
    {
    }
    __hashtable_iterator()
        :cur(XNULL),ht(XNULL)
    {
    }
    reference operator*() const
    {
        return cur->val;
    }
    pointer operator->() const
    {
        return &(operator*());
    }

    iterator& operator++();
    iterator operator++(XS32);
    bool operator==(const iterator& it) const
    {
        return cur == it.cur;
    }
    bool operator!=(const iterator& it) const
    {
        return cur != it.cur;
    }
//private:
    node* cur;//当前节点指针
    _hashtable* ht;//当前HASH表对象的指针
};

/**********************************************************/
//常量迭代器
template <class Value, class Key, class HashFcn,
         class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_const_iterator
{
    typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
    _hashtable;
    typedef __hashtable_iterator<Value, Key, HashFcn,
            ExtractKey, EqualKey, Alloc>
            iterator;
    typedef __hashtable_const_iterator<Value, Key, HashFcn,
            ExtractKey, EqualKey, Alloc>
            const_iterator;
    typedef __hashtable_node<Value> node;

    typedef forward_iterator_tag iterator_category;
    typedef Value value_type;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    typedef const Value& reference;
    typedef const Value* pointer;
    __hashtable_const_iterator(const node* n, const _hashtable* tab)
        : cur(n), ht(tab)
    {
    }
    __hashtable_const_iterator()
        :cur(XNULL),ht(XNULL)
    {
    }
    __hashtable_const_iterator(const iterator& it)
        :cur(it.cur), ht(it.ht)
    {
    }
    reference operator*() const
    {
        return cur->val;
    }
    pointer operator->()const
    {
        return &(operator*());
    }

    const_iterator& operator++();
    const_iterator operator++(XS32);
    bool operator==(const const_iterator& it) const
    {
        return cur == it.cur;
    }
    bool operator!=(const const_iterator& it) const
    {
        return cur != it.cur;
    }

    const node* cur;
    const _hashtable* ht;
};


size_t next_power_two(size_t i);

/***************************************************************/
//哈希表定义
//CBANAllocator     --是普通配置器
//CPreBANAllocator  --是内存预分配的配置器
//hash表的桶的大小是不能动态扩充变大的，否则，将和内存预分配的思想
//冲突
template <class Value, class Key, class HashFcn,class ExtractKey, class EqualKey, class Alloc = CBANAllocator >
class hashtable
{
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef HashFcn hasher;
    typedef EqualKey key_equal;
    typedef ExtractKey E_K;
    typedef size_t            size_type;
    typedef ptrdiff_t         difference_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;

    hasher hash_funct()const
    {
        return hash;
    }
    key_equal key_eq()const
    {
        return equals;
    }
private:
    typedef __hashtable_node<Value> node;
    enum {eNodeSize = sizeof(node)};
    Alloc  node_allocator;
    hasher hash;
    key_equal equals;
    ExtractKey get_key;
    typedef node*  BucketType;
    BucketType*  buckets;//桶
    size_t buckets_size;//桶的长度,桶的长度为2的n次方，这样计算hash表的

    size_type num_elements;//hash表中元素个数

public:
    typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
    iterator;
    typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey,Alloc>
    const_iterator;

    friend struct
            __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>;
    friend struct
            __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>;

public:
    //buckSize--桶的大小
    hashtable(size_type buckSize,const HashFcn& hf = HashFcn(),const EqualKey& eql = EqualKey())
        :node_allocator(eNodeSize),hash(hf), equals(eql), get_key(ExtractKey())
    {
        initialize_buckets(buckSize);
    }
    //使用预分配的配置器的构造函数接口
    //buckSize--桶的大小；PreNodeNum--预分配节点数目
    hashtable(size_type buckSize,size_t PreNodeNum,const HashFcn& hf = HashFcn(),const EqualKey& eql = EqualKey())
        :node_allocator(PreNodeNum,eNodeSize),hash(hf), equals(eql), get_key(ExtractKey())
    {
        initialize_buckets(buckSize);
    }

    //拷贝和赋值,注意，配置器的拷贝比较特殊
    hashtable(const hashtable& ht)
        :hash(ht.hash), equals(ht.equals), get_key(ht.get_key),node_allocator(ht.node_allocator)
    {
        copy_from(ht);
    }

    XVOID reserve(size_type n)
    {
        // 清除以前的节点
        clear();
        //将桶的内存释放掉
        node_allocator.ByteDealloc(buckets);
        buckets = XNULL;

        // 重新分配空间
        node_allocator.reserve(n);

        // 初始化
        initialize_buckets(n);
    }

    hashtable& operator= (const hashtable& ht)
    {
        if(&ht != this)
        {
            clear();
            hash = ht.hash;
            equals = ht.equals;
            get_key = ht.get_key;

            //配置器的赋值,配置器的赋值一定要在copy_from之前.
            //也就是配置器要在HASH节点构着之前初始化好
            node_allocator = ht.node_allocator;
            copy_from(ht);
        }
        return *this;
    }

    ~hashtable()
    {
        clear();
        //将桶的内存释放掉
        node_allocator.ByteDealloc(buckets);
        buckets = XNULL;

    }

    size_type size()const
    {
        return num_elements;
    }
    size_type max_size() const
    {
        // return size_type(-1);
        return node_allocator.max_size();
    }
    bool empty() const
    {
        return size() == 0;
    }
    XVOID swap(hashtable& ht)
    {
        std::swap(hash, ht.hash);
        std::swap(equals, ht.equals);
        std::swap(get_key, ht.get_key);
        //交换配置器的
        node_allocator.swap(ht.node_allocator);
        //交换桶的信息
        std::swap(buckets,ht.buckets);
        std::swap(buckets_size,ht.buckets_size);
        //交换HASH节点的个数
        std::swap(num_elements, ht.num_elements);
    }
    //迭代器相关函数
    //-----------------------------------------------------------------------

    iterator begin()
    {
        for (size_type n = 0; n < bucket_count(); ++n)
        {
            if (buckets[n])
            {
                return iterator(buckets[n], this);
            }
        }

        return end();
    }

    iterator end()
    {
        return iterator(0, this);
    }

    const_iterator begin() const
    {
        for (size_type n = 0; n < bucket_count(); ++n)
        {
            if (buckets[n])
            {
                return const_iterator(buckets[n], this);
            }
        }
        return end();
    }

    const_iterator end()const
    {
        return const_iterator(0, this);
    }

public:
    //表中桶的数目
    size_type bucket_count() const
    {
        return buckets_size;
    }
    //某处桶中的元素数目
    size_type elems_in_bucket(size_type bucket) const
    {
        size_type result = 0;
        for (node* cur = buckets[bucket]; cur; cur = cur->next)
        {
            result += 1;
        }
        return result;
    }
    //留下该接口，目的如下:
    //1.兼容。本次修改是在hashtable已被大量应用的前提下进行的
    //2.扩充。
    pair<iterator, bool> insert_unique(const value_type& obj)
    {
        //插入唯一不重整对象结构的对象值
        return insert_unique_noresize(obj);
    }
    //留下该接口，目的如下:
    //1.兼容。本次修改是在hashtable已被大量应用的前提下进行的
    //2.扩充。
    iterator insert_equal(const value_type& obj)
    {
        return insert_equal_noresize(obj);
    }
    pair<iterator, bool> insert_unique_noresize(const value_type& obj);
    iterator insert_equal_noresize(const value_type& obj);

    //查找
    reference find_or_insert(const value_type& obj);

    //通过键值查找记录
    iterator find(const key_type& key)
    {
        size_type n = bkt_num_key(key);
        node* first;
        for( first = buckets[n]; first && !equals(get_key(first->val),key); first = first->next)
        {

        }
        return iterator(first, this);
    }

    //通过键值查找记录
    const_iterator find(const key_type& key) const
    {
        size_type n = bkt_num_key(key);
        //比对每个元素的 键值，成功即退出
        node* first;
        for (first = buckets[n]; first && !equals(get_key(first->val), key); first = first->next);
        return const_iterator(first, this);
    }
    size_type erase(const key_type& key);
    XVOID erase(const iterator& it);

    XVOID clear();

private:
    XVOID erase_by_pointer(node* & refpNode)
    {
        if(refpNode == XNULL)
        {
            return;
        }
        node* pCur = refpNode;
        refpNode   = refpNode->next;//重新成链
        delete_node(pCur);//删掉该节点
        --num_elements;//将节点个数减1
    }

    node* add_after(node*& refCur,const value_type& obj)
    {
        node* tmp  = new_node(obj);
        tmp->next  = refCur;
        refCur     = tmp;
        ++num_elements;
        return tmp;
    }

    XVOID initialize_buckets(size_type n)
    {
        num_elements = 0;
        //将桶地大小改为2的n次方
        buckets_size  = next_power_two(n);
        //初始化
        buckets = reinterpret_cast<BucketType*>(node_allocator.ByteAlloc(sizeof(BucketType)*buckets_size));
        for(XU32 i = 0; i < buckets_size; ++i)
        {
            buckets[i] = XNULL;
        }
    }
    //通过键值计算桶的位置(一个索引值)
    size_type bkt_num_key(const key_type& key) const
    {
        //因为buckets_size的大小是2的n次方，所以mod运算可以简单以按位与
        //进行，这样，效率较高
        return hash(key)&(buckets_size-1);
    }

    size_type bkt_num(const value_type& obj) const
    {
        return bkt_num_key(get_key(obj));
    }
    node* new_node(const value_type& obj)
    {
        //分配一个节点
        node* n =  static_cast<node*>(node_allocator.allocate());
        n->next = 0;
        ConstructT(&n->val, obj);
        return n;
    }
    XVOID delete_node(node* p)
    {
        DestroyT(&p->val);
        node_allocator.deallocate(p);
    }
    XVOID copy_from(const hashtable& ht);
};


//插入唯一不重构的关键码
template <class V, class K, class HF, class Ex, class Eq, class A>
pair<typename hashtable<V, K, HF, Ex, Eq, A>::iterator, bool>
hashtable<V, K, HF, Ex, Eq, A>::insert_unique_noresize(const value_type& obj)
{
    const size_type n = bkt_num(obj);
    node* first = buckets[n];
    for(node* cur = first; cur; cur = cur->next)
    {
        if(equals(get_key(cur->val), get_key(obj)))
        {
            //找到一个键值一样的,返回插入失败
            return pair<iterator, bool>(iterator(cur, this), false);
        }
    }
    return pair<iterator, bool>(iterator(add_after(buckets[n],obj), this), true);
}

template <class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::iterator
hashtable<V, K, HF, Ex, Eq, A>::insert_equal_noresize(const value_type& obj)
{
    const size_type n = bkt_num(obj);
    node* first = buckets[n];
    for(node* cur = first; cur; cur = cur->next)
    {
        if(equals(get_key(cur->val), get_key(obj)))
        {
            return iterator(add_after(cur->next,obj), this);
        }
    }
    return iterator(add_after(buckets[n],obj), this);
}

template <class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::reference
hashtable<V, K, HF, Ex, Eq, A>::find_or_insert(const value_type& obj)
{
    size_type n = bkt_num(obj);
    node* first = buckets[n];
    for (node* cur = first; cur; cur = cur->next)
    {
        if (equals(get_key(cur->val), get_key(obj)))
        {
            return cur->val;
        }
    }
    return add_after(buckets[n],obj)->val;
}

//删除所有含该关键码值的记录
template <class V, class K, class HF, class Ex, class Eq, class A>
typename hashtable<V, K, HF, Ex, Eq, A>::size_type
hashtable<V, K, HF, Ex, Eq, A>::erase(const key_type& key)
{
    const size_type n = bkt_num_key(key);
    node* first = buckets[n];
    size_type erased = 0;
    if(first)
    {
        node* cur = first;
        node* next = cur->next;
        while(next)
        {
            if(equals(get_key(next->val), key))
            {
                cur->next = next->next;
                delete_node(next);
                next = cur->next;
                --num_elements;
                ++erased;
            }
            else
            {
                cur = next;
                next = cur->next;
            }
        }

        if(equals(get_key(first->val), key))
        {
            buckets[n] = first->next;
            delete_node(first);
            ++erased;
            --num_elements;
        }
    }
    return erased;
}

//按迭代器删除.不能按常量迭代器删除。
template <class V, class K, class HF, class Ex, class Eq, class A>
XVOID hashtable<V,K,HF,Ex,Eq,A>::erase(const iterator& it)
{

    //删除的是end点
    if(it.cur == XNULL)
    {
        return ;
    }

    const size_type n = bkt_num(it.cur->val);

    if(buckets[n] == it.cur)
    {
        erase_by_pointer(buckets[n]);
        return ;
    }
    node* pPrev = buckets[n];
    //寻找被删除的节点的上一个节点
    while( (pPrev != XNULL) &&(pPrev->next !=it.cur))
    {
        pPrev = pPrev->next;
    }

    //找到了该节点
    if(pPrev != XNULL)
    {
        erase_by_pointer(pPrev->next);
    }
}

//清除所有记录
template <class V, class K, class HF, class Ex, class Eq, class A>
XVOID hashtable<V, K, HF, Ex, Eq, A>::clear()
{
    for(size_type i = 0; i < buckets_size; ++i)
    {
        node* cur = buckets[i];
        while (cur != 0)
        {
            node* next = cur->next;
            delete_node(cur);
            cur = next;
        }
        buckets[i] = 0;
    }
    num_elements = 0;
}

template <class V, class K, class HF, class Ex, class Eq, class A>
XVOID hashtable<V, K, HF, Ex, Eq, A>::copy_from(const hashtable& ht)
{
    initialize_buckets(ht.buckets_size);

    for (size_type i = 0; i < ht.buckets_size; ++i)
    {
        if (const node* cur = ht.buckets[i])
        {
            node* copy = new_node(cur->val);
            buckets[i] = copy;

            for (node* next = cur->next; next; cur = next, next = cur->next)
            {
                copy->next = new_node(next->val);
                copy = copy->next;
            }
        }
    }
    num_elements = ht.num_elements;
}

template <class V, class K, class HF, class Ex, class Eq, class A>
bool operator==(const hashtable<V, K, HF, Ex, Eq, A>& ht1,const hashtable<V, K, HF, Ex, Eq, A>& ht2)
{
    typedef typename hashtable<V, K, HF, Ex, Eq, A>::node node;
    if(ht1.bucket_count() != ht2.bucket_count())
    {
        return false;
    }

    for (XS32 n = 0; n < ht1.bucket_count(); ++n)
    {
        node* cur1 = ht1.buckets[n];
        node* cur2 = ht2.buckets[n];
        for ( ; cur1 && cur2 && cur1->val == cur2->val; cur1 = cur1->next, cur2 = cur2->next)
        {
        }
        if (cur1 || cur2)
        {
            return false;
        }
    }
    return true;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>&
__hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++()
{
    const node* old = cur;
    cur = cur->next;
    if (!cur)
    {
        size_type bucket = ht->bkt_num(old->val);
        while (!cur && ++bucket < ht->bucket_count())
        {
            cur = ht->buckets[bucket];
        }

    }
    return *this;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline __hashtable_iterator<V, K, HF, ExK, EqK, A>
__hashtable_iterator<V, K, HF, ExK, EqK, A>::operator++(XS32)
{
    iterator tmp = *this;
    ++*this;
    return tmp;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>&
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>::operator++()
{
    const node* old = cur;
    cur = cur->next;
    if (!cur)
    {
        size_type bucket = ht->bkt_num(old->val);
        while (!cur && ++bucket < ht->bucket_count())
        {
            cur = ht->buckets[bucket];
        }
    }
    return *this;
}

template <class V, class K, class HF, class ExK, class EqK, class A>
inline __hashtable_const_iterator<V, K, HF, ExK, EqK, A>
__hashtable_const_iterator<V, K, HF, ExK, EqK, A>::operator++(XS32)
{
    const_iterator tmp = *this;
    ++*this;
    return tmp;
}

//常用hash函数的定义
template <class Key> struct hash
{
    template<class T> size_t operator()(const T&  x) const
    {
        return x;
    }
};

template<class T> struct CExtractKeyFormVale
{
    T operator()(const T& value)const
    {
        return value;
    }
};

//}

#endif


