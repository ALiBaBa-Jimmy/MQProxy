/************************************************************************************
�ļ���  :cpp_tcn_adapter.h

�ļ�����:

����    :zzt

��������:2006/4/27

�޸ļ�¼:
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
�������� :
�������� : Allocate

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID *Allocate(size_t size)
{
    return  new XS8[size];
}

/*********************************************************************
�������� : ConstructT
�������� : ��VC6.0��STL��û�ж���destory��construct,��SGI STL���ж���������
           ����Ա������
�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
//#undef new
template <class T1, class T2> inline XVOID ConstructT(T1* p, const T2& value)
{
    new (p) T1(value);
}
//#define new CPP_NEW

/*********************************************************************
�������� :
�������� : Dealloctor

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID Dealloctor(XVOID*p)
{
    delete[] static_cast<XS8*>(p);
}

/*********************************************************************
�������� : Allocate
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> inline T * AllocateT(size_t num,T*)
{
    return (T*)Allocate((size_t)(num*sizeof(T)));
}

/*********************************************************************
�������� : DestroyT
�������� : ��VC6.0��STL��û�ж���destory��construct,��SGI STL���ж���������
           ����Ա������
�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template <class T>inline XVOID DestroyT(T* pointer)
{
    pointer->~T();
}

/*********************************************************************
�������� : DestroyT
�������� : ��VC6.0��STL��û�ж���destory��construct,��SGI STL���ж���������
           ����Ա������
�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : ConstructT
�������� : ��VC6.0��STL��û�ж���destory��construct,��SGI STL���ж���������
           ����Ա������
�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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


