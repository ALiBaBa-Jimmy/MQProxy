/************************************************************************************
�ļ���  :cpp_new.cpp

�ļ�����:
         ����C++��׼���е�new��������ʵ�֡����ڴ���䶨λ��ƽ̨�йܵ��ڴ���
         �����һЩ���Եľͽӿڡ�C++��׼�ĵ���new ���ʽ��ص��½�������
         5.3.4
         a. 3.7.3
         b. 17.4.3.4 Any such function definition replaces the default version provided in the library (17.4.3.4)
         c. 18.4.1   Some global allocation and deallocation functions are replaceable (18.4.1)

����    :zzt

��������:2006/03/28

�޸ļ�¼:
         2006.03.09  zzt     �°汾ƽ̨������Ϣͷ�ļ���������ƽ̨
************************************************************************************/
#include "cpp_new.h"

#if 0

#undef new

/*lint -e1550*/
/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new(size_t size) throw(std::bad_alloc)
{
    return StandardNewOperator(CPPFRM_MODULE_ID, size);
}



/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new[](size_t size) throw(std::bad_alloc)
{
    return operator new (size);
}

/*********************************************************************
�������� :
�������� :  ���׳��쳣��new ������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new(size_t size, const std::nothrow_t& nothrow) throw()
{
    (XVOID)PreprcSize(size); //��size == 0��size��1

    return CPP_MALLOC(CPPFRM_MODULE_ID, size);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new[](size_t size, const std::nothrow_t& nothrow) throw()
{
    return operator new(size, nothrow);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new(size_t size, const XU32 modId) throw(std::bad_alloc)
{
    return StandardNewOperator(modId, size);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new[](size_t size, const XU32 modId) throw(std::bad_alloc)
{
    return operator new (size, modId);
}

/*********************************************************************
�������� :
�������� :  ���׳��쳣��new ������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new(size_t size, const XU32 modId,const std::nothrow_t& nothrow) throw()
{
    PreprcSize(size); //��size == 0��size��1

    return CPP_MALLOC(modId, size);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new[](size_t size, const XU32 modId,const std::nothrow_t& nothrow) throw()
{
    return operator new(size,modId,nothrow);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete(XVOID* ptr) throw()
{
    if (XNULL != ptr)
    {
        (XVOID)CPP_FREE(CPPFRM_MODULE_ID, ptr); //����Ҫ��Ϊ����ƽ̨�Ľӿ�
    }
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete[](XVOID* ptr) throw()
{
    operator delete(ptr);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete(XVOID* ptr, const std::nothrow_t& nothrow) throw()
{
    if (XNULL != ptr)
    {
        operator delete(ptr);
    }
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete[](XVOID* ptr, const std::nothrow_t& nothrow) throw()
{
    operator delete(ptr);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete(XVOID *ptr, const XU32 modId) throw()
{
    if (XNULL != ptr)
    {
        (XVOID)CPP_FREE(modId, ptr); //����Ҫ��Ϊ����ƽ̨�Ľӿ�
    }

}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete[](XVOID *ptr, const XU32 modId) throw()
{
    operator delete(ptr, modId);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete(XVOID* ptr, const XU32 modId,const std::nothrow_t& nothrow) throw()
{
    if (XNULL != ptr)
    {
        operator delete(ptr, modId);
    }
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete[](XVOID* ptr, const XU32 modId,const std::nothrow_t& nothrow) throw()
{
    operator delete(ptr, modId);
}

#endif

