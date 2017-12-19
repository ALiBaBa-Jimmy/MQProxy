/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xostrace.h
**
**  description:
**
**  author: guolili
**
**  date:   2006.3.9
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   guolili         2006.3.9             create
**   lxn	            2006.7.27           modify
**************************************************************/
#ifndef _XOS_TRACE_H_
#define _XOS_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include <string.h>

#include "xosos.h"
#include "xostype.h"
#include "clishell.h"

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/

/*-------------------------------宏定义------------------------------------*/
#define MAX_XU32INT                 0xffffffff /*定义xu32类型的最大值*/
#define SWITCH_ON                   1 /*控制开关:开*/
#define SWITCH_OFF                  0 /*控制开关:关*/

#define MSGID_PRINTO_MCB      1

#ifndef MAX_FILE_NAME_LEN
#define MAX_FILE_NAME_LEN      128/*打印的最长文件名2007/11/08 adjust from 50 to 64*/
#endif
#define MAX_MSGHEAD_LEN         100 /*要打印的消息头格式串最大长度*/
#define MAX_TRACE_INFO_LEN      4160/*整个trace输出信息串最长长度,2007/11/08 adjust from 512 to 1024*/
                                     /*2008-08 adjust from 1024 t0 512*/
                                    /*2011-07 adjust from 512 to 800 for ipc xos_trace string len*/

/*定义信息输出级别宏控制*/
/*FID不合法时，信息输出的过滤级别*/
#define XOS_INVALID_FID_TRACE_LEV    PL_ERR

/*FID没有注册时的信息输出过滤级别*/
#define XOS_UNREG_FID_TRACE_LEV      PL_ERR

/*FID正常时，默认模块的信息输出过滤级别*/
#define XOS_FID_INIT_TRACE_LEV       PL_ERR

#define XOS_PRINT                    XOS_Trace /*两个打印版本之间的兼容*/
#define XOS_PrintInit                Trace_Init /*模块初始化时两种版本的兼容*/

/*打印时输入的格式: XOS_PRINT(MD(ulFid, ulLevel), format...);中的宏定义*/
#ifdef XOS_LINUX
#define MD( ulFid, ulLevel) (XCHAR*)(__FILE__), (XU32)(__LINE__), (XCHAR*)(__FUNCTION__), (XU32 )(ulFid), (e_PRINTLEVEL)(ulLevel)
#else
#define MD( ulFid, ulLevel) (XCHAR*)(__FILE__), (XU32)(__LINE__), NULL, (XU32 )(ulFid), (e_PRINTLEVEL)(ulLevel)
#endif

#ifdef XOS_LINUX
#define FILI (XCHAR*)(__FILE__), (XU32)(__LINE__), (XCHAR*)(__FUNCTION__)
#else
#define FILI (XCHAR*)(__FILE__), (XU32)(__LINE__), NULL
#endif
/*-------------------------------------------------------------------------

                结构和枚举声明
-------------------------------------------------------------------------*/



typedef enum    /*信息输出目的文件枚举结构*/
{
   TOCOSOLE = 0,     /*只打印到终端*/
   TOLOGFILE = 1,    /*只打印到文件*/
   TOFILEANDCOS = 2, /*打印到终端和文件*/
   TOTELNET = 3,     /*打印到telnet，扩展*/
   TOSERVER = 4,     /*打印到远端服务器，扩展*/
   MAX_DEV
}e_TRACEPRINTDES;

/*fid 相关的trace信息控制开关结构体*/
typedef struct _TRACE_FID
{
 XBOOL isNeedFileName;     /*是否需要打印文件名，XTRUE表示需要打印，XFALSE表示不需要打印*/
 XBOOL isNeedLine;         /*是否需要打印行号，XTRUE表示需要打印，XFALSE表示不需要打印*/
 XBOOL isNeedTime;         /* 是否需要打印时间， XTRUE表示需要打印，XFALSE表示不需要打印*/
 XBOOL isPrintInTraceTsk;  /*是否在trace的任务空间中打印，XTRUE表示在。XFALSE表示在用户自己的任务中打印*/
 e_PRINTLEVEL traceLevel;  /*模块的打印级别*/
 e_PRINTLEVEL sessionLevel[kCLI_MAX_CLI_TASK];  /*模块的telnet终端打印级别*/
 e_TRACEPRINTDES traceOutputMode;
 t_XOSTT prevTime;                    /*上一次访问时间*/
 t_XOSMUTEXID xosFidTraceMutex;       /*临界区*/
}t_FIDTRACEINFO;

/*-记录系统事件格式*/
typedef struct _TIME
{
    XU32    dt_sec;         /* seconds */
    XU32    dt_min;         /* minutes */
    XU32    dt_hour;        /* hours */
    XU32    dt_mday;        /* day of the month */
    XU32    dt_mon;         /* month */
    XU32    dt_year;        /* year */
    XU32    dt_wday;        /* day of the week */
    XU32    dt_yday;        /* day in the year */
    XU32    dt_isdst;       /* daylight saving time */
}t_trc_systime;

/*------------------------结构定义------------------------------------*/


/*---------------------------------------------------------------------
函数名：  XOS_CpsTrace
功能：   针对平台内部的trace打印消息，负责打印到终端并且记录到日志文件中
输入：

        ulFid           - 输入，功能块ID
        ulLevel         - 输入，打印级别
        ucFormat        - 输入，打印格式化字符串
        ...             - 输入，打印参数

输出：
返回值:  XSUCC  -	成功		  XERROR -	失败
说明：
------------------------------------------------------------------------*/
XPUBLIC XVOID XOS_CpsTrace( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, ... );

/*---------------------------------------------------------------------
函数名：  XOS_CpsPrintf
功  能：  针对平台内部的trace打印消息，负责打印到终端并且记录到日志文件中
输  入：  ulFid            - 输入，功能块ID
          ulLevel          - 输入，打印级别
          etraceOutputMode - 输入，输出到终端或文件 
          ucFormat         - 输入，打印格式化字符串
          ap               - 输入，变参的栈起始地址

输  出：
返回值:  XSUCC  -   成功； XERROR - 失败
说  明： 如果输入的长度大于MAX_TRACE_INFO_LEN，则会失败，且不进行提示
------------------------------------------------------------------------*/
XVOID XOS_CpsPrintf(XU32 ulFid, e_PRINTLEVEL eLevel, e_TRACEPRINTDES etraceOutputMode, const XCHAR *cFormat, va_list ap);

XVOID XOS_TraceTa( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat,va_list ap );
XVOID XOS_TraceInfo( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat,va_list ap ,XCHAR *buf);



#define XOS_Trace XOS_CpsTrace
#define XOS_Printf XOS_CpsPrintf

/*---------------------------------------------------------------------
函数名：  XOS_Trace
功能：    新版信息打印函数
输入：
         FileName       - 输入，文件名(宏__FILE__)
        ulLineNum       - 输入，行号   (宏__LINE__)
        ulFid           - 输入，功能块ID
        ulLevel         - 输入，打印级别
        ucFormat        - 输入，打印格式化字符串
        ...             - 输入，打印参数

输出：
返回值:  XSUCC  -	成功		  XERROR -	失败
说明：
------------------------------------------------------------------------*/
#if 0
XPUBLIC XVOID XOS_Trace( const XCHAR* FileName, XU32 ulLineNum, XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, ... );
#endif

/******************************************************************
函数名: Trace_Init
功能:系统启动时对打印模块的初始化操作

*******************************************************************/
XEXTERN XS8  Trace_Init(XVOID * t, XVOID *V);

/*---------------------------------------------------------------------
函数名：  XOS_Sprintf
功能：     格式化字符串函数.与sprintf功能相同
                    此函数中，首先对cFormat串进行格式化，并将格式化后的字符串个数控制在
                    500个字符数以内。格式化后如果cFormat经格式化后的字符数大于实际要写入的缓冲区大小
                    则，输出错误信息，否则将格式化后的字符串写入实际缓冲区
输入：
        Buffer               - 输入，存放格式化后的字符串的缓冲区.
        charnum              - 输入，实际要写入缓冲区的字符数个数.
        cFormat              - 输入，格式化的字符串
		...                  - 输入, 可以有任意个跟前面的格式化字符串匹配
输出：
返回值: 成功实际写入的字节数
                  失败返回 XERROR
说明：
------------------------------------------------------------------------*/
XEXTERN XS32 XOS_Sprintf( XCHAR* Buffer,XU32 charnum, const XCHAR *cFormat, ... );

/*---------------------------------------------------------------------
函数名：  XOS_getPLName
功能：    获取打印级别的名称
输入：
输出：
返回值:
说明：
------------------------------------------------------------------------*/
XCHAR* XOS_getPLName(e_PRINTLEVEL printLevel);

/*------------------------------------------------------------------
函数名：  Trace_msgProc
功能：   消息处理函数
输入：
输出：
返回值:
说明：
------------------------------------------------------------------------*/
XPUBLIC XS8 Trace_msgProc(XVOID* pMsP, XVOID*sb );
/******************************************************
函数名：  Trace_abFileName
功能：   从绝对路径中解析文件名函数
输入：  FileName -绝对路径
                   fnstr--最终的文件名
                   Max_FileNameLen--文件名的最大长度
输出：
返回值:   成功返回xsucc;失败返回xerror;
说明：

*******************************************************/
XPUBLIC XS32 Trace_abFileName(XCONST XCHAR *FileName,XCHAR *fnstr,XU32 Max_FileNameLen);

/*--------------------------------------------------------------------
函数名：  XOS_GetFidTraceFlag
功能：    获取FID当前trace信息输出标志位
输入：
        ulFid -功能块ID
      ulFidTraceInfo -负责记录trace的各个标志位结构

输出：
返回值:  成功返回XSUCC；失败返回XERROR
说明：
------------------------------------------------------------------------*/
XPUBLIC XS32 XOS_GetFidTraceFlag(XU32 ulFid, t_FIDTRACEINFO *ulFidTraceInfo);

/*--------------------------------------------------------------------
函数名：  XOS_SetFidTraceFlag
功能：    设置FID对应trace信息输出标志位
输入：
      ulFid -功能块ID
      FilenameFlag - 设置是否打印文件名标志（TRUE-打印；FALSE-不打印）
      LinenumFlag -设置是否打印行号标志（TRUE-打印；FALSE-不打印）
      TimeFlag -设置是否打印时间标志（TRUE-打印；FALSE-不打印）
      TransToTaskFlag -设置是否传输到消息队列标志（TRUE-传；FALSE-不传）
      OutputMode -设置输出目标设备标志（见e_TRACEPRINTDES 结构）
      OutputLevel -设置该FID的信息打印过滤级别，输出信息级别低于该级别时将不输出（见e_PRINTLEVEL结构

输出：
返回值:  成功返回XSUCC；失败返回XERROR
说明：
------------------------------------------------------------------------*/
XPUBLIC XS32 XOS_SetFidTraceFlag(XU32 ulFid,XBOOL FilenameFlag,XBOOL LinenumFlag,XBOOL TimeFlag,
              XBOOL TransToTaskFlag, e_TRACEPRINTDES  OutputMode, e_PRINTLEVEL OutputLevel);
XPUBLIC XS8 XOS_TraceClose();

/************************************************************************
 函数名: XOS_PrinToMcb(  )
 功能:   带格式的输出函数,输出信息可以重定向
 输入:   pFmt 格式话字符串
 输出:   无
 返回:   无
 说明:   无
************************************************************************/
XS32 XOS_PrinToMcb( XCHAR* pFmt, ... );
XS32 XOS_PrintToMcbStr(const XCHAR* buff);

XS32 XOS_TraceSetFidLevel (XU32  ulFid, e_PRINTLEVEL ulLevel);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xostrace.h*/


