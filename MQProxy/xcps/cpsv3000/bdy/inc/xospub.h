/**************************************************************
    filename: xpub.h
    func: 公共头文件集合
    auth: david
    time: 2008-05-11
**************************************************************/
#ifndef _XOSPUB_H_
#define _XOSPUB_H_


#ifdef  __cplusplus
    extern  "C"{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef LINUX
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <curses.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <elf.h>
typedef pthread_mutex_t     _mutexId;
#endif

#ifdef WIN32
#include <conio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
typedef CRITICAL_SECTION    _mutexId;

#pragma comment(lib,"WS2_32")
#endif

#ifdef VXWORKS
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

typedef SEM_ID      _mutexId;

#endif


#ifndef u8
#define u8 unsigned char
#endif

#ifndef s8
#define s8 char
#endif

#ifndef u16
#define u16 unsigned short
#endif

#ifndef s16
#define s16 short
#endif

#ifndef u32
#define u32 unsigned int
#endif

#ifndef s32
#define s32 int
#endif

#ifdef WIN32

#ifndef u64
#define u64 unsigned _int64
#endif

#ifndef s64
#define s64 _int64
#endif

#endif

#ifdef LINUX

#ifndef u64
#define u64 unsigned long long
#endif

#ifndef s64
#define s64 long long
#endif


#endif

#define _success_               0x00
#define _failed_                0x01


#include "xosinet.h"

typedef struct
{
    s32 value;
    const s8  *pstr;
}sys_value2string;


/* 双向链表 */
typedef struct _sys_list_st
{
    struct _sys_list_st *prev;  /* 前向指针 */
    struct _sys_list_st *next;  /* 后向指针 */
}sys_list_st;


typedef struct 
{
    u16     year;           /* 年   */
    u8      month;          /* 月   */
    u8      day;            /* 日   */
    u8      hour;           /* 小时 */
    u8      minute;         /* 分钟 */
    u8      second;         /* 秒   */
    u8      recv;    
    u16     missec;         /* 毫秒 */
    u16     recv1;
}sys_time_st;

/* function declare */
s32 sys_logMsg(s8* file, s8 *buf, s32 len);
s32 logTime(s8* file);

s32 sys_mutexInit(_mutexId *mutex);
s32 sys_mutexLock(_mutexId *mutex);
s32 sys_mutexUnlock(_mutexId *mutex);

s32 sys_createThread(void *pFunc, void *para, u8 *threadName);
s32 sys_getCurTime(sys_time_st *pTime);

s32 sys_listInit(sys_list_st *ent);
s32 sys_listAdd(sys_list_st *old, sys_list_st *add);
s32 sys_listDel(sys_list_st *del);
s32 sys_listEmpty(sys_list_st *head);
void sys_listSpliceInit(sys_list_st *slist, sys_list_st *head);
sys_list_st* sys_listNext(sys_list_st *ent);

s32 sys_slistAdd(sys_list_st *ent, sys_list_st *add, _mutexId *m);
s32 sys_slistDel(sys_list_st *ent, _mutexId *m);
sys_list_st* sys_slistGetNext(sys_list_st *ent, _mutexId *m);

s32 sys_getNetBufSize(t_SOCKFD fd, s32 *size);
u32 sys_getTick();
s32 sys_consoleGoxy(int x, int y);
u32 sys_sleep(u32 tm);

s8* sys_getStringByValue(sys_value2string *st, s32 id);
s32 sys_IsMiddleSeq(u32 beginSeq, u32 endSeq, u32 midSeq);
s32 sys_readSymTable(s8 *name);
s32 sys_printSymTable();

s32 sys_symIsFun(u32 type);
s32 sys_findSymbByName(s8 *name, u32 *pAddress, u32 *type);


#pragma pack()

XPOINT strtopointer(const char *nptr, char **endptr, int base);


#ifdef __cplusplus
    }
#endif

#endif








