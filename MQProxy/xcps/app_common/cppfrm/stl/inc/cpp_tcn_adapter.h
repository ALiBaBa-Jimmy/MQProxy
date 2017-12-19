/************************************************************************************
文件名  :cpp_tcn_adapter.h

文件描述:

作者    :zzt

创建日期:2006/4/27

修改记录:
************************************************************************************/
#ifndef __CPP_TCN_STL_ADAPTER_H_
#define __CPP_TCN_STL_ADAPTER_H_

#include <algorithm>
#include <functional>
#include "cpp_adapter.h"


#ifdef XOS_WIN32
#pragma warning( disable : 4786 )
#endif

//namespace tcn
//{

/*********************************************************************
函数名称 :
功能描述 : Allocate

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID *Allocate(size_t size)
{
    return  new XS8[size];
}

/*********************************************************************
函数名称 : ConstructT
功能描述 : 在VC6.0的STL库没有定义destory和construct,但SGI STL库中定义了这两
           个成员函数。
参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
//#undef new
template <class T1, class T2> inline XVOID ConstructT(T1* p, const T2& value)
{
    new (p) T1(value);
}
//#define new CPP_NEW

/*********************************************************************
函数名称 :
功能描述 : Dealloctor

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID Dealloctor(XVOID*p)
{
    delete[] static_cast<XS8*>(p);
}

/*********************************************************************
函数名称 : Allocate
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> inline T * AllocateT(size_t num,T*)
{
    return (T*)Allocate((size_t)(num*sizeof(T)));
}

/*********************************************************************
函数名称 : DestroyT
功能描述 : 在VC6.0的STL库没有定义destory和construct,但SGI STL库中定义了这两
           个成员函数。
参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template <class T>inline XVOID DestroyT(T* pointer)
{
    pointer->~T();
}

/*********************************************************************
函数名称 : DestroyT
功能描述 : 在VC6.0的STL库没有定义destory和construct,但SGI STL库中定义了这两
           个成员函数。
参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class ForwardIterator>
inline XVOID DestroyT(ForwardIterator first, ForwardIterator last)
{
    for ( ; first != last; ++first)
    {
        DestroyT(&*first);
    }
}

/*********************************************************************
函数名称 : ConstructT
功能描述 : 在VC6.0的STL库没有定义destory和construct,但SGI STL库中定义了这两
           个成员函数。
参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template <class FI1, class FI2> inline XVOID ConstructT(FI1 result,  FI2 first,FI2 last)
{
    for(; first != last; ++first)
    {
        ConstructT(result++,*first);
    }
}

//}

#endif


