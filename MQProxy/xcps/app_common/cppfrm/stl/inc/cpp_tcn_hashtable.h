/****************************************************************************
�ļ���  :cpp_tcn_hashtable.h

�ļ�����:

����    :yuxiao

��������:2006/3/29

�޸ļ�¼:
         ZZT 20060531 ȥ������Ҫ�Ĺ��ܣ���Ӱ�������ɾ���ĵĽӿ�
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

//hash�����ǰ������
template <class Value, class Key, class HashFcn,
         class ExtractKey, class EqualKey, class Alloc> class hashtable;


//hash��ķǳ���������ǰ������
template <class Value, class Key, class HashFcn,
         class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator;

//hash��ĳ���������ǰ������
template <class Value, class Key, class HashFcn,
         class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_const_iterator;

/****************************************************************/
//������
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
    node* cur;//��ǰ�ڵ�ָ��
    _hashtable* ht;//��ǰHASH������ָ��
};

/**********************************************************/
//����������
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
//��ϣ����
//CBANAllocator     --����ͨ������
//CPreBANAllocator  --���ڴ�Ԥ�����������
//hash���Ͱ�Ĵ�С�ǲ��ܶ�̬������ģ����򣬽����ڴ�Ԥ�����˼��
//��ͻ
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
    BucketType*  buckets;//Ͱ
    size_t buckets_size;//Ͱ�ĳ���,Ͱ�ĳ���Ϊ2��n�η�����������hash���

    size_type num_elements;//hash����Ԫ�ظ���

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
    //buckSize--Ͱ�Ĵ�С
    hashtable(size_type buckSize,const HashFcn& hf = HashFcn(),const EqualKey& eql = EqualKey())
        :node_allocator(eNodeSize),hash(hf), equals(eql), get_key(ExtractKey())
    {
        initialize_buckets(buckSize);
    }
    //ʹ��Ԥ������������Ĺ��캯���ӿ�
    //buckSize--Ͱ�Ĵ�С��PreNodeNum--Ԥ����ڵ���Ŀ
    hashtable(size_type buckSize,size_t PreNodeNum,const HashFcn& hf = HashFcn(),const EqualKey& eql = EqualKey())
        :node_allocator(PreNodeNum,eNodeSize),hash(hf), equals(eql), get_key(ExtractKey())
    {
        initialize_buckets(buckSize);
    }

    //�����͸�ֵ,ע�⣬�������Ŀ����Ƚ�����
    hashtable(const hashtable& ht)
        :hash(ht.hash), equals(ht.equals), get_key(ht.get_key),node_allocator(ht.node_allocator)
    {
        copy_from(ht);
    }

    XVOID reserve(size_type n)
    {
        // �����ǰ�Ľڵ�
        clear();
        //��Ͱ���ڴ��ͷŵ�
        node_allocator.ByteDealloc(buckets);
        buckets = XNULL;

        // ���·���ռ�
        node_allocator.reserve(n);

        // ��ʼ��
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

            //�������ĸ�ֵ,�������ĸ�ֵһ��Ҫ��copy_from֮ǰ.
            //Ҳ����������Ҫ��HASH�ڵ㹹��֮ǰ��ʼ����
            node_allocator = ht.node_allocator;
            copy_from(ht);
        }
        return *this;
    }

    ~hashtable()
    {
        clear();
        //��Ͱ���ڴ��ͷŵ�
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
        //������������
        node_allocator.swap(ht.node_allocator);
        //����Ͱ����Ϣ
        std::swap(buckets,ht.buckets);
        std::swap(buckets_size,ht.buckets_size);
        //����HASH�ڵ�ĸ���
        std::swap(num_elements, ht.num_elements);
    }
    //��������غ���
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
    //����Ͱ����Ŀ
    size_type bucket_count() const
    {
        return buckets_size;
    }
    //ĳ��Ͱ�е�Ԫ����Ŀ
    size_type elems_in_bucket(size_type bucket) const
    {
        size_type result = 0;
        for (node* cur = buckets[bucket]; cur; cur = cur->next)
        {
            result += 1;
        }
        return result;
    }
    //���¸ýӿڣ�Ŀ������:
    //1.���ݡ������޸�����hashtable�ѱ�����Ӧ�õ�ǰ���½��е�
    //2.���䡣
    pair<iterator, bool> insert_unique(const value_type& obj)
    {
        //����Ψһ����������ṹ�Ķ���ֵ
        return insert_unique_noresize(obj);
    }
    //���¸ýӿڣ�Ŀ������:
    //1.���ݡ������޸�����hashtable�ѱ�����Ӧ�õ�ǰ���½��е�
    //2.���䡣
    iterator insert_equal(const value_type& obj)
    {
        return insert_equal_noresize(obj);
    }
    pair<iterator, bool> insert_unique_noresize(const value_type& obj);
    iterator insert_equal_noresize(const value_type& obj);

    //����
    reference find_or_insert(const value_type& obj);

    //ͨ����ֵ���Ҽ�¼
    iterator find(const key_type& key)
    {
        size_type n = bkt_num_key(key);
        node* first;
        for( first = buckets[n]; first && !equals(get_key(first->val),key); first = first->next)
        {

        }
        return iterator(first, this);
    }

    //ͨ����ֵ���Ҽ�¼
    const_iterator find(const key_type& key) const
    {
        size_type n = bkt_num_key(key);
        //�ȶ�ÿ��Ԫ�ص� ��ֵ���ɹ����˳�
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
        refpNode   = refpNode->next;//���³���
        delete_node(pCur);//ɾ���ýڵ�
        --num_elements;//���ڵ������1
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
        //��Ͱ�ش�С��Ϊ2��n�η�
        buckets_size  = next_power_two(n);
        //��ʼ��
        buckets = reinterpret_cast<BucketType*>(node_allocator.ByteAlloc(sizeof(BucketType)*buckets_size));
        for(XU32 i = 0; i < buckets_size; ++i)
        {
            buckets[i] = XNULL;
        }
    }
    //ͨ����ֵ����Ͱ��λ��(һ������ֵ)
    size_type bkt_num_key(const key_type& key) const
    {
        //��Ϊbuckets_size�Ĵ�С��2��n�η�������mod������Լ��԰�λ��
        //���У�������Ч�ʽϸ�
        return hash(key)&(buckets_size-1);
    }

    size_type bkt_num(const value_type& obj) const
    {
        return bkt_num_key(get_key(obj));
    }
    node* new_node(const value_type& obj)
    {
        //����һ���ڵ�
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


//����Ψһ���ع��Ĺؼ���
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
            //�ҵ�һ����ֵһ����,���ز���ʧ��
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

//ɾ�����к��ùؼ���ֵ�ļ�¼
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

//��������ɾ��.���ܰ�����������ɾ����
template <class V, class K, class HF, class Ex, class Eq, class A>
XVOID hashtable<V,K,HF,Ex,Eq,A>::erase(const iterator& it)
{

    //ɾ������end��
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
    //Ѱ�ұ�ɾ���Ľڵ����һ���ڵ�
    while( (pPrev != XNULL) &&(pPrev->next !=it.cur))
    {
        pPrev = pPrev->next;
    }

    //�ҵ��˸ýڵ�
    if(pPrev != XNULL)
    {
        erase_by_pointer(pPrev->next);
    }
}

//������м�¼
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

//����hash�����Ķ���
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


