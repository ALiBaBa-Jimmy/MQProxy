/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosroot.c
**
**  description:  整个平台启动
**
**  author: wangzongyou
**
**  date:   2006.07.20
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
 
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "xosroot.h"
#include "xosmodule.h"
#include "xoscfg.h"
#include "xostrace.h"
#include "clishell.h"
#include "xosmem.h"
#include "xosencap.h"
#include "xosmmgt.h"
#include "xosnetinf.h"

/*-------------------------------------------------------------------------
                 全局定义
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                 模块内部宏定义
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/
XU32 g_startSec = 0;

/*-------------------------------------------------------------------------
                 模块内部结构和枚举定义
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/
#ifdef XOS_MDLMGT
XPUBLIC XS32 XOS_Init(HANDLE hDir,XS32 argc, XS8** argv);
XPUBLIC XS32 XOS_ModNotice(HANDLE hDir,XS32 argc, XS8** argv);
#endif

#ifdef XOS_IPC_MGNT
XEXTERN XU32 XOS_IPCGetLocalSlot();
#endif

#ifdef XOS_LINUX
XEXTERN XVOID XOS_RegisterAllSignal();
#endif

/*-------------------------------------------------------------------------
                模块接口函数
-------------------------------------------------------------------------*/
#ifndef XOS_MDLMGT
/************************************************************************
函数名: XOS_Root
功能：  平台软件入口函数
输入：  XVOID
输出：  N/A
返回：  XSUCC OR XERROR
说明：  进行平台软件的初始化，并根据业务层填写的注册结构进行任务
        的创建，调度等
************************************************************************/
XPUBLIC XS8 XOS_Root(XVOID)
{
    XS32 ret;

	//#ifdef XOS_LINUX
#if 0
    /*如果LINUX增加信号注册*/
    XOS_RegisterAllSignal();
#endif

	//#ifdef XOS_LINUX
#if 0
    /* ipmi环境检测 */
    ret = XOS_CheckIpmiEnv();
    if (XSUCC == ret)
    {
        XOS_SetIpmiFlag(XTRUE);
    }
    else
    {
        XOS_SetIpmiFlag(XFALSE);
        printf("XOS_Root()->ipmi env check failed");
    }
#else
    XOS_SetIpmiFlag(XFALSE);
#endif

    /*内存初始化 */
    ret = MEM_Initlize();
    if(XSUCC != ret)
    {
        /*trace 还没有初始化*/
        printf( "XOS_Root()->mem init failed\n");
        goto rootErrorProc;
    }

    /*模块管理初始化*/
    ret = MOD_init();
    if(XSUCC != ret)
    {
        /*trace 还没有初始化*/
        printf("XOS_Root()->module management init failed\n");
        goto rootErrorProc;
    }
#ifdef XOS_IPC_MGNT
    XOS_SetLocalPID(XOS_IPCGetLocalSlot());
#endif
    /* 启动平台*/
    ret = MOD_startXosFids();
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_Root()->start xos fids  failed");
        goto rootErrorProc;
    }

    /*启动业务模块*/
    ret = MOD_startUserFids();
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_Root()->start user fids  failed");
        goto rootErrorProc;
    }

    /*启动通知*/
    ret = MOD_StartNotice();
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_Root()->start notice  failed");
        goto rootErrorProc;
    }

    return XSUCC;

rootErrorProc:

#ifdef XOS_DEBUG    
    /*调试用, 系统还没有起来，挂起保留现场信息*/        
    XOS_SusPend();
#endif

    /*暂时不做释放资源的处理，系统起不来，肯定没有办法跑的*/
    return XERROR;
}
#endif


#ifdef XOS_MDLMGT
extern XS32  XOS_FIDROOT(HANDLE hDir,XS32 argc, XS8** argv);
/************************************************************************
函数名: XOS_Root
功能：  
输入：  XVOID
输出：  N/A
返回：  XSUCC OR XERROR
说明：  
************************************************************************/
XPUBLIC XS32 XOS_Init(HANDLE hDir,XS32 argc, XS8** argv)
{
    XS32 ret;

	//#ifdef XOS_LINUX
#if 0
    /*如果LINUX增加信号注册*/
    XOS_RegisterAllSignal();
#endif

#if 0
	#ifdef XOS_LINUX

    /* ipmi环境检测 */
    ret = XOS_CheckIpmiEnv();
    if (XSUCC == ret)
    {
        XOS_SetIpmiFlag(XTRUE);
    }
    else
    {
        XOS_SetIpmiFlag(XFALSE);
        printf("XOS_Init()->ipmi env check failed");
    }
#else
    XOS_SetIpmiFlag(XFALSE);
#endif
#endif


    /*内存初始化 */
    ret = MEM_Initlize();
    if(XSUCC != ret)
    {
        /*trace 还没有初始化*/
        printf( "XOS_Init()->MEM_Initlize failed\n");
        return XERROR;
    }

    ret = MOD_init();
    if(XSUCC != ret)
    {
        /*trace 还没有初始化*/
        printf( "XOS_Init()->MOD_init failed\n");
        return XERROR;
    }

    /* 初始化网口设置环境 */
    ret = XOS_NetInfInit();
    if (XSUCC != ret)
    {
        printf("XOS_Init()->net interface module init failed\n");
        return XERROR;
    }

    g_startSec = XOS_TicksToSec(XOS_GetSysTicks());
#ifdef XOS_IPC_MGNT
    XOS_SetLocalPID(XOS_IPCGetLocalSlot());
#endif

    XOS_FIDROOT(hDir, argc, argv);


    return XSUCC;
}


/************************************************************************
函数名: XOS_ModNotice
功能：  
输入：  XVOID
输出：  N/A
返回：  XSUCC OR XERROR
说明：  
************************************************************************/
XPUBLIC XS32 XOS_ModNotice(HANDLE hDir,XS32 argc, XS8** argv)
{
    XS32 ret;

    /*启动通知*/
    ret = MOD_StartNotice();
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_Root()->start notice  failed");
        return XERROR;
    }
    return XSUCC;
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
