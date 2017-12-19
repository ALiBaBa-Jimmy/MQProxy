/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: os.h
**
**  description:  各个操作系统下不同的宏定义 
**
**  author: wentao
**
**  date:   2006.3.29
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   wentao         2006.3.29              create  
**************************************************************/
#ifndef _OS_H_
#define _OS_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
             包含头文件
-------------------------------------------------------------------------*/
#include "vXworks.h"
#include "taskLib.h"
#include "semLib.h"
#include "msgQLib.h"
#include "ftpLib.h" 

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "inetLib.h"
#include "stat.h"
#include "time.h"
#include "timers.h"
#include "ioLib.h"

#include "sockLib.h"
#include "stdioLib.h"
#include "strLib.h"
#include "hostLib.h"
#include "ioLib.h"

#include "types.h"

#include "socket.h"
#include "dirent.h"
#include "wdLib.h"

#include "intlib.h"

#include "sysLib.h"
#include "rebootLib.h"


/*-------------------------------------------------------------------------
             宏定义
-------------------------------------------------------------------------*/
#define OS_GetChar()     getchar()

#define XOSTASKSIZEMAX 2097152
#define XOSTASKSIZEDEFAULT 40960


typedef SEM_ID      t_XOSMUTEXID;
typedef int         t_XOSTASKID;
typedef SEM_ID      t_XOSSEMID;

typedef void* (*os_taskfunc)(void *);       /*任务函数指针类型*/

//static int XOS_spyInfoPrint(const char *fmt, ...);
/*-------------------------------------------------------------------------
                           全局函数声明
-------------------------------------------------------------------------*/
extern int ruby_vsnprintf(char *str, size_t n, const char *fmt, va_list ap);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*os.h*/

