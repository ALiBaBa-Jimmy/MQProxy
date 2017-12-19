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
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
#include <tlhelp32.h>
#include <assert.h>
#include <conio.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/timeb.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <windows.h>
#include <winuser.h>
#include <io.h>


/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/

#define OS_GetChar()     getch()

#define XOSTASKSIZEMAX 8388608
#define XOSTASKSIZEDEFAULT 1048576

#define OS_LOCK_MUTEX 1
#define OS_LOCK_CRITSEC 3

/******************以下为WINDOWS下获取CPU占用率用********************************/
#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3
#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

#define DIV 1024

typedef struct
{
    DWORD   dwUnknown1;
    ULONG   uKeMaximumIncrement;
    ULONG   uPageSize;
    ULONG   uMmNumberOfPhysicalPages;
    ULONG   uMmLowestPhysicalPage;
    ULONG   uMmHighestPhysicalPage;
    ULONG   uAllocationGranularity;
    PVOID   pLowestUserAddress;
    PVOID   pMmHighestUserAddress;
    ULONG   uKeActiveProcessors;
    BYTE    bKeNumberProcessors;
    BYTE    bUnknown2;
    WORD    wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
    LARGE_INTEGER   liIdleTime;
    DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG         uCurrentTimeZoneId;
    DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;

typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

extern PROCNTQSI NtQuerySystemInformation;
/******************以上为WINDOWS下获取CPU占用率用********************************/

#if 1
typedef CRITICAL_SECTION t_XOSMUTEXID;	/*mod by 2006.5.15*/
#endif

typedef HANDLE t_XOSTASKID;

#if 0		/*mod by 06.5.15*/
typedef struct _OSLOCK
{
   union
   {
      CRITICAL_SECTION critical;        /* Critical section */
      HANDLE mutex;         /* Mutex in user space */
   }LockID;                          /* Actual OS lock */

   /*XU32         arg;                  additional argumanet */
   unsigned int          LockType;                /* type of the lock */
}t_XOSMUTEXID;
#endif


typedef void (*os_taskfunc)(void*);   /*任务函数指针类型*/
typedef struct _OSTASKCB
{
    os_taskfunc  f;   /*任务处理函数*/
    void         *p;
}t_OSTASKCB;


typedef struct tagWinSemaphore
{
    HANDLE      semaId;
}WIN_SEMAPHORE;
typedef WIN_SEMAPHORE   t_XOSSEMID;

typedef struct tagWinSemini
{
    long       num;
    long       max;
}WIN_SEMINI;
typedef WIN_SEMINI      t_XOSSEMINI;


#if 0	/*del by 06.5.12*/
typedef enum
{
    BIG_END=0,
    LITTLE_END,
    MAX_END
}e_BIGORLITTLE;
#endif

/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*xos.h*/

