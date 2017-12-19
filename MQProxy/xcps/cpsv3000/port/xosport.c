/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosport.c
**
**  description:  作模块管理的读配置文件启动时，这个文件
                          将消失。
**
**  author: wangzongyou
**
**  date:   2006.7.24
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
#include "xosshell.h"

/*-------------------------------------------------------------------------
                 模块内部结构和枚举定义
-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/
/* t_XOSLOGINLIST *LoginList;*/
/*change by wulei 2006.8.18 
  此变量应用在注册时会放在不同的文件中，因此放入.h文件较好
  将此变量声明放入xosport.h中
*/


/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块接口函数
-------------------------------------------------------------------------*/
#ifdef XOS_NEED_MAIN


#ifndef XOS_MDLMGT
#if 1/* 从B10不提供非模块启动方式 如业务需要，则自己进行封装 */
/************************************************************************
函数名:XOS_InfoReg
功能: 业务模块的注册
输入:
输出:
返回:
说明: 暂时为了接口不变,写成由.c文件注册. 做模块管理
          时,将取消该.c 文件,所有的模块通过读配置文件启动
************************************************************************/
XPUBLIC XS8 XOS_InfoReg(t_XOSLOGINLIST *LoginList)
{
    /*在此添加其他的业务的应用*/

    /*注意下标最多有FID_MAX-FID_USERMIN 个, 不能越界*/
    return XSUCC;
}
#endif


#if defined (XOS_WIN32) || defined (XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VXWORKS)
/************************************************************************
函数名: main
功能:
输入:
输出:
返回:
说明:
************************************************************************/
int main( int argc, char **argv )
{
    XOS_DaemonInit( argc, argv );
    /*  在此调用平台入口函数，启动平台 */
    if(XSUCC != XOS_Root())
    {
        return XERROR;
    }

    CLI_locConsoleEntry();

    return 0;
}
#endif

#endif /* XOS_MDLMGT */ 

/* 模块管理启动方式 */
#ifdef XOS_MDLMGT
/************************************************************************
函数名:    main        
功能：          
参数：          
输出：          
返回：          
说明：          
************************************************************************/

#if defined (XOS_WIN32) || defined (XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VTA)
int main( int argc,char *argv[] )
{
    g_main_argc = argc;
    g_main_argv = argv;
    XOS_DaemonInit( argc, argv );

#ifndef XOS_EW_START    
    if (XSUCC != XosMMinitSymbTbl(argv[0]))
#else
    if (XSUCC != XosMMinitSymbTbl( XOS_CliGetSysName() ))
#endif
    {
        MMErr("Init Symbol Table Failed");
        return -1;
    }

    XosModuleManagerInit(argv[1], argv[2]);
    /*MM_taskSuspendCurrent();*/

    CLI_locConsoleEntry();

    return 0;
}
#endif /* #if defined (XOS_WIN32) || defined (XOS_LINUX) || defined(XOS_SOLARIS) */


#ifdef XOS_VXWORKS
#ifdef XOS_NVTA
XVOID CPSRoot(int argc,char **argv)
{
    if (XSUCC != XosMMinitSymbTbl(argv[0]))
    {
        MMErr("Init Symbol Table Failed");
        return;
    }

    XosModuleManagerInit(argv[1], argv[2]);
    MM_taskSuspendCurrent();
    return;
}
#endif /*XOS_NVTA*/
#endif/* XOS_VXWORKS */
#endif/* XOS_MDLMGT */
#endif /*XOS_NEED_MAIN*/


#ifdef __cplusplus
}
#endif /* __cplusplus */


