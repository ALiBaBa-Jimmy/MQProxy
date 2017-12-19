/************************************************************************************
文件名  :cpp_new.cpp

文件描述:
         重载C++标准库中的new操作符的实现。将内存分配定位到平台托管的内存上
         并添加一些调试的就接口。C++标准文档与new 表达式相关的章节有如下
         5.3.4
         a. 3.7.3
         b. 17.4.3.4 Any such function definition replaces the default version provided in the library (17.4.3.4)
         c. 18.4.1   Some global allocation and deallocation functions are replaceable (18.4.1)

作者    :zzt

创建日期:2006/03/28

修改记录:
         2006.03.09  zzt     新版本平台发布消息头文件。废弃老平台
************************************************************************************/
#include "cpp_new.h"

#if 0

#undef new

/*lint -e1550*/
/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new(size_t size) throw(std::bad_alloc)
{
    return StandardNewOperator(CPPFRM_MODULE_ID, size);
}



/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new[](size_t size) throw(std::bad_alloc)
{
    return operator new (size);
}

/*********************************************************************
函数名称 :
功能描述 :  不抛出异常的new 操作符

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new(size_t size, const std::nothrow_t& nothrow) throw()
{
    (XVOID)PreprcSize(size); //若size == 0则将size加1

    return CPP_MALLOC(CPPFRM_MODULE_ID, size);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new[](size_t size, const std::nothrow_t& nothrow) throw()
{
    return operator new(size, nothrow);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new(size_t size, const XU32 modId) throw(std::bad_alloc)
{
    return StandardNewOperator(modId, size);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new[](size_t size, const XU32 modId) throw(std::bad_alloc)
{
    return operator new (size, modId);
}

/*********************************************************************
函数名称 :
功能描述 :  不抛出异常的new 操作符

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new(size_t size, const XU32 modId,const std::nothrow_t& nothrow) throw()
{
    PreprcSize(size); //若size == 0则将size加1

    return CPP_MALLOC(modId, size);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* operator new[](size_t size, const XU32 modId,const std::nothrow_t& nothrow) throw()
{
    return operator new(size,modId,nothrow);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete(XVOID* ptr) throw()
{
    if (XNULL != ptr)
    {
        (XVOID)CPP_FREE(CPPFRM_MODULE_ID, ptr); //这里要改为调用平台的接口
    }
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete[](XVOID* ptr) throw()
{
    operator delete(ptr);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete(XVOID* ptr, const std::nothrow_t& nothrow) throw()
{
    if (XNULL != ptr)
    {
        operator delete(ptr);
    }
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete[](XVOID* ptr, const std::nothrow_t& nothrow) throw()
{
    operator delete(ptr);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete(XVOID *ptr, const XU32 modId) throw()
{
    if (XNULL != ptr)
    {
        (XVOID)CPP_FREE(modId, ptr); //这里要改为调用平台的接口
    }

}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete[](XVOID *ptr, const XU32 modId) throw()
{
    operator delete(ptr, modId);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete(XVOID* ptr, const XU32 modId,const std::nothrow_t& nothrow) throw()
{
    if (XNULL != ptr)
    {
        operator delete(ptr, modId);
    }
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID operator delete[](XVOID* ptr, const XU32 modId,const std::nothrow_t& nothrow) throw()
{
    operator delete(ptr, modId);
}

#endif

