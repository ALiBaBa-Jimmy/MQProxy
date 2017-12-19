/****************************************************************************
文件名  :cpp_tcn_vector.h

文件描述:

作者    :yuxiao

创建日期:2006/2/21

修改记录:
*************************************************************************/
#ifndef __CPP_TCN_VECTOR_H_
#define __CPP_TCN_VECTOR_H_

#include "cpp_prealloc.h"

#ifdef XOS_WIN32
//暂时过滤4996告警，add by liaod
#pragma warning(disable:4996)
#endif

namespace tcn
{
template <class InputIterator, class OutputIterator>
inline OutputIterator assign(InputIterator first, InputIterator last,OutputIterator result)
{
    for ( ; first != last; ++result, ++first)
        *result = *first;
    return result;
}

//由于vector本身就支持内存预分配的思想,所以就使用一般的配置
template <class T, class Alloc = CAllocator>
class vector
{
public:
    typedef T                 value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type*       iterator;
    typedef const value_type* const_iterator;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    typedef size_t            size_type;
    typedef ptrdiff_t         difference_type;

protected:
    enum {eNodeSize = sizeof(T)};

protected:
    Alloc       data_allocator;//内存配置器对象
    iterator    start;
    iterator    finish;
    iterator    end_of_storage;//存储区的结束地址的下一字节的地址
    XVOID insert_aux(iterator position, const T& x);
    XVOID init()
    {
        start           = 0;
        finish          = 0;
        end_of_storage  = 0;
    }
    //释放内存
    XVOID deallocate()
    {
        data_allocator.deallocate(start);
        init();
    }
    XVOID fill_initialize(size_type n, const T& value)
    {
        start = allocate_and_fill(n, value);
        finish = start + n;
        end_of_storage = finish;
    }

public:
    iterator begin()
    {
        return start;
    }
    const_iterator begin() const
    {
        return start;
    }
    iterator end()
    {
        return finish;
    }
    const_iterator end() const
    {
        return finish;
    }

    size_type size() const
    {
        return size_type(end() - begin());
    }
    size_type max_size() const
    {
        return size_type(-1) / sizeof(T);
    }
    size_type capacity() const
    {
        return size_type(end_of_storage - begin());
    }
    bool empty() const
    {
        return begin() == end();
    }
    reference operator[](size_type n)
    {
        return *(begin() + n);
    }
    const_reference operator[](size_type n) const
    {
        return *(begin() + n);
    }

public:
    //构造函数的实现,由于本VECTOR用法是支持内存预分配，所以去掉一些其他的
    //构造函数的接口
    vector()
        :data_allocator(eNodeSize)
    {
        init();
    }
    vector(size_type n, const T& value)
        :data_allocator(eNodeSize)
    {
        init();
        if(n > 0)
        {
            fill_initialize(n, value);
        }
    }
    //这里只用指针
    vector(pointer first, pointer last)
        :data_allocator(eNodeSize)
    {
        init();
        start  = allocate_and_copy((last-first), first, last);
        finish = start + (last-first);
        end_of_storage = finish;
    }

    vector(const vector<T, Alloc>& x)
        :data_allocator(x.data_allocator)
    {
        init();
        start = allocate_and_copy(x.end() - x.begin(), x.begin(), x.end());
        finish = start + (x.end() - x.begin());
        end_of_storage = finish;
    }
    vector<T, Alloc>& operator=(const vector<T, Alloc>& x);
    ~vector()
    {
        DestroyT(start,finish);
        deallocate();
    }

    XVOID reserve(size_type n)
    {
        if (capacity() < n)
        {
            const size_type old_size = size();
            iterator tmp = allocate_and_copy(n, start, finish);
            DestroyT(start, finish);
            deallocate();
            start = tmp;
            finish = tmp + old_size;
            end_of_storage = start + n;
        }
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
        return *(end() - 1);
    }
    const_reference back() const
    {
        return *(end() - 1);
    }
    XVOID push_back(const T& x)
    {
        if (finish != end_of_storage)
        {
            ConstructT(finish, x);
            ++finish;
        }
        else
        {
            insert_aux(end(), x);
        }
    }
    XVOID swap(vector<T, Alloc>& x)
    {
        std::swap(start, x.start);
        std::swap(finish, x.finish);
        std::swap(end_of_storage, x.end_of_storage);
        data_allocator.swap(x.data_allocator);
    }
    //插入操作，在position之前插入X
    iterator insert(iterator position, const T& x)
    {
        size_type n = position - begin();
        //若还有剩余的内存空间,并且是在 END点前插入
        if (finish != end_of_storage && position == end())
        {
            ConstructT(finish, x);
            ++finish;
        }
        else
        {
            insert_aux(position, x);
        }
        return begin() + n;
    }
    //插入操作，插入一段元素
    XVOID insert(iterator position, const_iterator first, const_iterator last);
    XVOID pop_back()
    {
        --finish;
        DestroyT(finish);
    }
    //删除操作，若删除的是END点，则不应该让程序行为不可确定
    iterator erase(iterator position)
    {
        if(position + 1 != end())
        {
            std::copy(position + 1, finish, position);
        }
        --finish;
        DestroyT(finish);
        return position;
    }
    iterator erase(iterator first, iterator last)
    {
        iterator i = std::copy(last, finish, first);
        DestroyT(i, finish);
        finish = finish - (last - first);
        return first;
    }
    XVOID clear()
    {
        erase(begin(), end());
    }

protected:
    //已完成改写
    iterator allocate_and_fill(size_type n, const T& x)
    {
        //分配N个元素的内存的大小
        iterator result = static_cast<iterator>(data_allocator.allocate(n));
        //这里用比较简单但效率较低的实现
        for(size_type i = 0; i < n; ++i)
        {
            ConstructT(result+i,x);
        }
        return result;
    }
    //已完成改写
    template <class ForwardIterator>
    iterator allocate_and_copy(size_type n,ForwardIterator first, ForwardIterator last)
    {
        if (n > 0)
        {
            iterator result = static_cast<iterator>(data_allocator.allocate(n));
            ConstructT(result,first,last);
            return result;
        }
        return end();
    }

};

template <class T, class Alloc>
vector<T, Alloc>& vector<T, Alloc>::operator=(const vector<T, Alloc>& x)
{
    if(&x != this)
    {
        if (x.size() > capacity())
        {
            iterator tmp = allocate_and_copy(x.end() - x.begin(),x.begin(), x.end());
            DestroyT(start, finish);
            deallocate();
            start = tmp;
            end_of_storage = start + (x.end() - x.begin());
        }
        else if (size() >= x.size())
        {
            iterator i = std::copy(x.begin(), x.end(), begin());
            DestroyT(i, finish);
        }
        else
        {
            std::copy(x.begin(), x.begin() + size(), start);
            std::uninitialized_copy(x.begin() + size(), x.end(), finish);
        }
        finish = start + x.size();
        data_allocator = x.data_allocator;
    }
    return *this;
}

//在position处插入一个元素
template <class T, class Alloc>
XVOID vector<T, Alloc>::insert_aux(iterator position, const T& x)
{
    if(finish != end_of_storage)
    {
        //在尾部构建一个元素，便于以后的赋值操作
        ConstructT(finish, *(finish - 1));
        ++finish;
        //copy_backward是STL标准规定的必须有的函数，内部实现用赋值
        std::copy_backward(position, finish - 2, finish - 1);
        T x_copy = x;//这里将要调用一次拷贝构造
        *position = x_copy;//这里将要调用一次赋值操作

    }
    else
    {
        const size_type old_size = size();
        const size_type len = old_size != 0 ? 2 * old_size : 1;
        iterator new_start = static_cast<iterator>(data_allocator.allocate(len));
        iterator new_finish = new_start;
        new_finish = std::uninitialized_copy(start, position, new_start);
        ConstructT(new_finish, x);
        ++new_finish;
        new_finish = std::uninitialized_copy(position, finish, new_finish);
        DestroyT(begin(), end());
        deallocate();
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}

template <class T, class Alloc>
XVOID vector<T, Alloc>::insert(iterator position, const_iterator first, const_iterator last)
{
    //若参数范围不合法，则直接返回
    if(first >= last)
    {
        return ;
    }
    //计算元素个数
    size_type n = last-first;

    if(size_type(end_of_storage - finish) >= n)
    {
        //剩余的内存空间足够大,不需要扩容
        //将被移动的元素个数
        const size_type elems_after = finish - position;
        iterator old_finish = finish;
        if (elems_after > n)
        {
            //将要被移动的元素个数大于将要被插入的元素的个数
            std::uninitialized_copy(finish - n, finish, finish);
            finish += n;
            std::copy_backward(position, old_finish - n, old_finish);
            assign(first, last, position);

        }
        else
        {
            std::uninitialized_copy(first + elems_after, last, finish);
            finish += n - elems_after;
            std::uninitialized_copy(position, old_finish, finish);
            finish += elems_after;
            assign(first, first + elems_after, position);
        }
    }
    else
    {
        //剩余空间不够大，需要扩容
        const size_type old_size = size();
        const size_type len = old_size + (old_size > n? old_size:n);
        iterator new_start = static_cast<iterator>(data_allocator.allocate(len));
        iterator new_finish = new_start;
        new_finish = std::uninitialized_copy(start, position, new_start);
        new_finish = std::uninitialized_copy(first, last, new_finish);
        new_finish = std::uninitialized_copy(position, finish, new_finish);
        DestroyT(start, finish);
        deallocate();
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
    }
}

}//namespace tcn

#endif

