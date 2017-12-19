/************************************************************************************
�ļ���  :cpp_new.h

�ļ�����:�ڴ�����ļ�

����    :zzt

��������:2006/03/29

�޸ļ�¼:

************************************************************************************/
#ifndef __CPP_NEW_H_
#define __CPP_NEW_H_


#if 0

#include <new>
#ifdef WIN32
#include <memory>
#endif

#include "xosshell.h"


#ifdef WIN32
#pragma warning( disable : 4290 )
#endif


//ģ��ID����
const XU32 CPPFRM_MODULE_ID = FID_USERMIN;


//�ڴ��ض�λ
#define CPP_MALLOC(modId, size)     XOS_MemMalloc(modId, size)
#define CPP_FREE(modId, p)          XOS_MemFree(modId, p)


/*********************************************************************
�������� :PreprcSize
�������� :�����û����ܷ���0�ֽڴ�С�ģ�������Ҫ��size�Ĵ�С1

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID PreprcSize(size_t& size)
{
    if(0 == size)
    {
        ++size;
    }
}

/*********************************************************************
�������� : ��׼C++��NEW������Ϊ
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID* StandardNewOperator(XU32 modId, size_t size)throw(std::bad_alloc)
{
    PreprcSize(size);

    XVOID *p = XNULL;
    while(!(p = CPP_MALLOC(modId, size)))
    {
#ifndef XOS_WIN32
        std::new_handler exe_new_handler = std::set_new_handler(0);
        (XVOID)std::set_new_handler(exe_new_handler);  //�����ֵ��дд��ȥ
#else
        #if (_MSC_VER < 1310)
        new_handler exe_new_handler = set_new_handler(0);
        (XVOID)set_new_handler(exe_new_handler);       //�����ֵ��дд��ȥ
        #else
        std::new_handler exe_new_handler = std::set_new_handler(0);
        (XVOID)std::set_new_handler(exe_new_handler);  //�����ֵ��дд��ȥ
        #endif
#endif
        if(exe_new_handler)
        {
            (*exe_new_handler)();
        }
        else
        {
            #ifndef MAKE_SXC_LINUX
            throw std::bad_alloc();
            #endif
        }
    }
    return p;
}

/*********************************************************************
�������� :
�������� : ��׼C++��DELETE������Ϊ

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID StandardDelOperator(XU32 modId, XVOID*p)
{
    if(XNULL != p)
    {
        CPP_FREE(modId, p);
    }
}



/*********************************************************************
�������� :
�������� : VC6.0��Ҫ�ö���.��VC6.0�϶������´���:
          //new �� delete�ĵ��԰汾����
          XVOID* operator new[](size_t ,const XS8 *,XS32 ) throw(std::bad_alloc)
          XVOID  operator delete[](XVOID *,const XS8 *,XS32 );
          //���������뽫����
          XS32 *p = new(__FILE__,__LINE__) XS32[10];
          delete[] p;//error C2660: 'delete[]' : function does not take 1 parameters
�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/

//���ڵ�������ͷ�ļ���ʹ�õ�NEW, ���������޷��滻, ����������Ҫ��������

XVOID* operator new (size_t size) throw(std::bad_alloc);
XVOID* operator new[](size_t size) throw(std::bad_alloc);

XVOID* operator new(size_t size, const std::nothrow_t& nothrow) throw();
XVOID* operator new[](size_t size, const std::nothrow_t& nothrow) throw();

XVOID* operator new (size_t size, const XU32 modId) throw(std::bad_alloc);
XVOID* operator new[](size_t size, const XU32 modId) throw(std::bad_alloc);

XVOID* operator new(size_t size, const XU32 modId,const std::nothrow_t& nothrow) throw();
XVOID* operator new[](size_t size, const XU32 modId,const std::nothrow_t& nothrow) throw();

//vc6��������������׳��쳣ʱ��δ����delete�ͷ��ڴ�
XVOID operator delete(XVOID *ptr) throw();
XVOID operator delete[](XVOID* ptr) throw();

XVOID operator delete(XVOID* ptr, const std::nothrow_t& nothrow) throw();
XVOID operator delete[](XVOID* ptr, const std::nothrow_t& nothrow) throw();

XVOID operator delete(XVOID *ptr, const XU32 modId) throw();
XVOID operator delete[](XVOID *ptr, const XU32 modId) throw();


XVOID operator delete(XVOID* ptr, const XU32 modId,const std::nothrow_t& nothrow) throw();
XVOID operator delete[](XVOID* ptr, const XU32 modId,const std::nothrow_t& nothrow) throw();


//�ض����
#define new        CPP_NEW
#define CPP_NEW    new

#endif


#endif //ifndef __FRM_NEW_H_



