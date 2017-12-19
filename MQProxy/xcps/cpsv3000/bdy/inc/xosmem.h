/***************************************************************
 **
 **  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
 **  
 **  Core Network Department  platform team  
 **
 **  filename: xosmem.h
 **
 **  description:  
 **
 **  author: wangzongyou
 **
 **  date:   2006.7.20
 **
 ***************************************************************
 **                          history                     
 **  
 ***************************************************************
 **   author          date              modification            
 **     
 **************************************************************/
#ifndef _XOS_MEM_H_
#define _XOS_MEM_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#ifdef MEM_FID_DEBUG
#ifdef XOS_VXWORKS
#include <taskLib.h>
#include <sysSymTbl.h>
#include <symLib.h>
#endif
#ifdef XOS_WIN32
#include <Imagehlp.h>
#endif
#endif/*_MEM_FID_DEBUG_*/

#include "xostype.h"

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/

#define MAX_MEM_STACK_DEPTH  20  /*定义函数调用栈的最大深度*/
#define MAX_STACK_FUN_NAME 64
/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/


/************************************************************************
 函数名 : MEM_Initlize
 功能   : 内存初始化
 输入   : 
 输出   : none
 返回   : 
 说明   :
************************************************************************/
XS32 MEM_Initlize(XVOID );


/************************************************************************
函数名: XOS_MemMalloc
功能：  分配一个内存块
输入：  fid           - 功能块id
        nbytes        - 要分配内存的字节数
输出：  N/A
返回：  XVOID *       -分配的内存指针
说明：  返回值由用户强制转换
************************************************************************/
XVOID *XOS_MemMalloc1(XU32 fid, 
                         XU32 nbytes, 
                         const XCHAR* fileName, 
                         XU32 lineNo);
//XVOID *XOS_MemMalloc(XU32 fid, XU32 nbytes);

#ifdef MEM_FID_DEBUG
#define XOS_MemMalloc(fid, nbytes)  XOS_MemMalloc1(fid, nbytes, __FILE__, __LINE__) 
#else
#define XOS_MemMalloc(fid, nbytes)  XOS_MemMalloc1(fid, nbytes, 0, 0) 
#endif


/************************************************************************
函数名: XOS_MemCheck
功能：  判断内存块是否属于bucket的内存块,不释放内存
输入：  fid           - 功能块id
                ptr           - 内存块首地址
输出：  N/A
返回:   XSUCC  -    是
        XERROR -    否
说明： 
************************************************************************/
XS32 XOS_MemCheck(XU32 fid, XVOID *ptr);


/************************************************************************
函数名: XOS_MemFree
功能：  释放一个内存块
输入：  fid           - 功能块id
        ptr           - 要释放的内存首地址
输出：  N/A
返回:   XSUCC  -    成功
        XERROR -    失败
说明： 
************************************************************************/
XS32 XOS_MemFree(XU32 fid, XVOID *ptr);


/************************************************************************
函数名: XOS_MEMCtrl(兼容trillium协议栈的SSI内存使用)
功能： 返回内存块的使用状态
输入：  nbytes          - bucket内存块的大小
       
输出：  N/A
返回:   使用比例    
说明： 
************************************************************************/
XU32 XOS_MEMCtrl(XU32 nbytes );

XVOID XosStackDumpToSyslog(const char *pszBuf);
XVOID write_stack_to_syslog(XU32 fid);
XS32 write_to_syslog(const XS8* msg,...);



XVOID meminfo(XVOID);
XU32 XOS_GetMemContent(XCONST XS8 *title,XCONST void *mem,XU32 len,XU8 *buf,XU32 bufSize);
void XOS_MemPrint(XCONST XU32 fid,XCONST XS8 *title,e_PRINTLEVEL level,XCONST void *mem,XU32 len);
#ifdef MEM_FID_DEBUG
XVOID memfid(XU32 fid);
XVOID memblock(XS32 size);
XVOID memallblock(XVOID);
XVOID memstack(XPOINT uPtr);
XS32 MEM_fidModify(XU32 *pFid);
XU32 MEM_QueryFidByThreadId(XU64 dwThreadId, XU32 *upNewFid);
XVOID MEM_AddFidByThreadId(XU64 dwThreadId, XU32 uFid);
XU32  MEM_BKDRHash(XU64 dwThreadId);
#endif
XVOID memhelp();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*cmqueue.h*/

