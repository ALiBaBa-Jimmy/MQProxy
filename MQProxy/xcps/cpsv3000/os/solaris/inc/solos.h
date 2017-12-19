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
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>/**/

/* According to POSIX 1003.1-2001 */
//#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>


#include "errno.h"

/*-------------------------------------------------------------------------
                  宏定义
-------------------------------------------------------------------------*/
typedef pthread_mutex_t  t_XOSMUTEXID;
typedef pthread_t        t_XOSTASKID;
typedef long             t_XOSMSGQID;
/*typedef long             t_XOSSEMID;*/
typedef sem_t             t_XOSSEMID;
typedef long             t_XOSSEMINI;

#define OS_GetChar()     getchar()

typedef void* (*os_taskfunc)(void *);       /*任务函数指针类型*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*os.h*/
