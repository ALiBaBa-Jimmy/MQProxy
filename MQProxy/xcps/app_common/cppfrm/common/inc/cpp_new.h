/************************************************************************************
文件名  :cpp_new.h

文件描述:内存管理文件

作者    :zzt

创建日期:2006/03/29

修改记录:

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


//模块ID定义
const XU32 CPPFRM_MODULE_ID = FID_USERMIN;


//内存重定位
#define CPP_MALLOC(modId, size)     XOS_MemMalloc(modId, size)
#define CPP_FREE(modId, p)          XOS_MemFree(modId, p)


/*********************************************************************
函数名称 :PreprcSize
功能描述 :由于用户可能分配0字节大小的，所以需要将size的大小1

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID PreprcSize(size_t& size)
{
    if(0 == size)
    {
        ++size;
    }
}

/*********************************************************************
函数名称 : 标准C++的NEW操作行为
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID* StandardNewOperator(XU32 modId, size_t size)throw(std::bad_alloc)
{
    PreprcSize(size);

    XVOID *p = XNULL;
    while(!(p = CPP_MALLOC(modId, size)))
    {
#ifndef XOS_WIN32
        std::new_handler exe_new_handler = std::set_new_handler(0);
        (XVOID)std::set_new_handler(exe_new_handler);  //将句柄值重写写回去
#else
        #if (_MSC_VER < 1310)
        new_handler exe_new_handler = set_new_handler(0);
        (XVOID)set_new_handler(exe_new_handler);       //将句柄值重写写回去
        #else
        std::new_handler exe_new_handler = std::set_new_handler(0);
        (XVOID)std::set_new_handler(exe_new_handler);  //将句柄值重写写回去
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
函数名称 :
功能描述 : 标准C++的DELETE操作行为

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
inline XVOID StandardDelOperator(XU32 modId, XVOID*p)
{
    if(XNULL != p)
    {
        CPP_FREE(modId, p);
    }
}



/*********************************************************************
函数名称 :
功能描述 : VC6.0需要该定义.在VC6.0上对于如下代码:
          //new 和 delete的调试版本声明
          XVOID* operator new[](size_t ,const XS8 *,XS32 ) throw(std::bad_alloc)
          XVOID  operator delete[](XVOID *,const XS8 *,XS32 );
          //如下语句编译将出错
          XS32 *p = new(__FILE__,__LINE__) XS32[10];
          delete[] p;//error C2660: 'delete[]' : function does not take 1 parameters
参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/

//对于第三方库头文件中使用的NEW, 可能我们无法替换, 所以我们需要如下申明

XVOID* operator new (size_t size) throw(std::bad_alloc);
XVOID* operator new[](size_t size) throw(std::bad_alloc);

XVOID* operator new(size_t size, const std::nothrow_t& nothrow) throw();
XVOID* operator new[](size_t size, const std::nothrow_t& nothrow) throw();

XVOID* operator new (size_t size, const XU32 modId) throw(std::bad_alloc);
XVOID* operator new[](size_t size, const XU32 modId) throw(std::bad_alloc);

XVOID* operator new(size_t size, const XU32 modId,const std::nothrow_t& nothrow) throw();
XVOID* operator new[](size_t size, const XU32 modId,const std::nothrow_t& nothrow) throw();

//vc6在类的析构函数抛出异常时并未调用delete释放内存
XVOID operator delete(XVOID *ptr) throw();
XVOID operator delete[](XVOID* ptr) throw();

XVOID operator delete(XVOID* ptr, const std::nothrow_t& nothrow) throw();
XVOID operator delete[](XVOID* ptr, const std::nothrow_t& nothrow) throw();

XVOID operator delete(XVOID *ptr, const XU32 modId) throw();
XVOID operator delete[](XVOID *ptr, const XU32 modId) throw();


XVOID operator delete(XVOID* ptr, const XU32 modId,const std::nothrow_t& nothrow) throw();
XVOID operator delete[](XVOID* ptr, const XU32 modId,const std::nothrow_t& nothrow) throw();


//重定义宏
#define new        CPP_NEW
#define CPP_NEW    new

#endif


#endif //ifndef __FRM_NEW_H_



