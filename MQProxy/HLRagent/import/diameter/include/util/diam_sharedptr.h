#ifndef __UTIL_SHARED_PTR_H__
#define __UTIL_SHARED_PTR_H__

#include <util/diam_exception.h>
#include <iostream>
#include <util/diam_ace.h>

using namespace std;

namespace diameter
{

template<class T> struct SharedPtrTraits
{
    typedef T & reference;
};

class SharedCount
{
public:
    SharedCount() : _count (0)
    {
    }
public:
    bool decRef()
    {
        mutex.acquire();
        UTIL_ASSERT(_count > 0);
        --_count;
        if( _count == 0 )
        {
            mutex.release();
            return true;
        }
        mutex.release();
        return false;

    }

    void incRef()
    {
        mutex.acquire();
        UTIL_ASSERT(_count >= 0);
        _count++;
        mutex.release();
    }

    void * operator new(std::size_t )
    {
        return std::allocator<SharedCount>().allocate( 1, static_cast<SharedCount *>(0) );
    }

    void operator delete( void * p )
    {
        std::allocator<SharedCount>().deallocate(static_cast<SharedCount *>(p), 1 );
    }

private:
    //用于线程安全
    ACE_Recursive_Thread_Mutex mutex;
    INT32 _count;
};

} // namespace detail

class NullHandleException : public Exception
{
public:
    NullHandleException(const string & file, const  INT32 & line) :
        Exception(file,line)
    {
    }

    string getPrototypeName() const
    {
        return "util::NullHandleException";
    }

    string getMessage() const
    {
        return "pointer is null.";
    }
};

template<class T>
class SharedPtr
{
private:

    typedef SharedPtr<T> this_type;

public:

    typedef T element_type;
    typedef T value_type;
    typedef T * pointer;
    typedef typename diameter::SharedPtrTraits<T>::reference reference;

    SharedPtr() : px(0),use_count_(0)
    {
    }

    template<class Y>
    explicit SharedPtr(Y * p): px(static_cast<T *>(p))
    {
        use_count_ = new diameter::SharedCount;
        use_count_->incRef();
    }

    explicit SharedPtr(T * p): px(p)
    {
        use_count_ = new diameter::SharedCount;
        use_count_->incRef();
    }

    template<class Y>
    SharedPtr(SharedPtr<Y> const & r): px(static_cast<T *>(r.px)),
        use_count_(r.use_count_)
    {
        if(use_count_ != 0)
        {
            use_count_->incRef();
        }
    }

    SharedPtr(SharedPtr const & r): px(r.px),use_count_(r.use_count_)
    {
        if(use_count_ != 0)
        {
            use_count_->incRef();
        }
    }
    template<class Y>
    SharedPtr & operator = (SharedPtr<Y> const & r)
    {
        if(r.px != this->px)
        {
            checkDelete();
            px = static_cast<T *>(r.px);
            use_count_ = r.use_count_;
            if(use_count_ != 0)
            {
                use_count_->incRef();
            }
        }
        else
        {
            //如果引用的对象相同，也要同步引用计数器
            use_count_ = r.use_count_;
        }
        return *this;
    }

    SharedPtr & operator = (SharedPtr const & r)
    {
        if(r.px != this->px)
        {
            checkDelete();
            px = r.px;
            use_count_ = r.use_count_;
            if(use_count_ != 0)
            {
                use_count_->incRef();
            }
        }
        else
        {
            //如果引用的对象相同，也要同步引用计数器
            use_count_ = r.use_count_;
        }
        return *this;
    }


    reference operator* () const
    {
        if(px == NULL)
        {
            throw NullHandleException(__FILE__,__LINE__);
        }
        return *px;
    }

    T * operator-> () const
    {
        if(px == NULL)
        {
            throw NullHandleException(__FILE__,__LINE__);
        }
        return px;
    }

    T * get() const
    {
        return px;
    }

    operator bool () const
    {
        return px != 0;
    }

    bool operator! () const // never throws
    {
        return px == 0;
    }

    void reset(T* ptr = 0)
    {
        //不是相同的对象
        if (ptr != this->px)
        {
            //解除原有引用
            checkDelete();
            //重新构建引用
            px = ptr;
            use_count_ = new diameter::SharedCount;
            use_count_->incRef();
        }
        //相同对象保存不变
    }

    T* release()
    {
        T * tmp = this->px;
        checkDelete();
        this->px = NULL;
        this->use_count_ = NULL;
        return tmp;
    }

    ~SharedPtr()
    {
        //printf("***********Destructor Object %p**********",px);
        checkDelete();
    }

private:
    void checkDelete()
    {
        if(use_count_ != 0)
        {
            if(use_count_->decRef())
            {
                delete use_count_;
                use_count_ = 0;
                delete px;
                px = 0 ;
            }
        }
    }

#ifndef _WIN32
private:
    template<class Y> friend class SharedPtr;
#else
public:
#endif

    T * px;
    diameter::SharedCount * use_count_;
};  // SharedPtr


template<typename T, typename U>
inline bool operator==(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l == *r;
    }
    else
    {
        return !l && !r;
    }
}

template<typename T, typename U>
inline bool operator!=(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l != *r;
    }
    else
    {
        return l || r;
    }
}

template<typename T, typename U>
inline bool operator<(const SharedPtr<T>& lhs, const SharedPtr<U>& rhs)
{
    T* l = lhs.get();
    U* r = rhs.get();
    if(l && r)
    {
        return *l < *r;
    }
    else
    {
        return !l && r;
    }
}

#endif  //__UTIL_SHARED_PTR_H__
