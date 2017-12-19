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
                  ����ͷ�ļ�
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
                �궨��
-------------------------------------------------------------------------*/

#define MAX_MEM_STACK_DEPTH  20  /*���庯������ջ��������*/
#define MAX_STACK_FUN_NAME 64
/*-------------------------------------------------------------------------
                �ṹ��ö������
-------------------------------------------------------------------------*/


/************************************************************************
 ������ : MEM_Initlize
 ����   : �ڴ��ʼ��
 ����   : 
 ���   : none
 ����   : 
 ˵��   :
************************************************************************/
XS32 MEM_Initlize(XVOID );


/************************************************************************
������: XOS_MemMalloc
���ܣ�  ����һ���ڴ��
���룺  fid           - ���ܿ�id
        nbytes        - Ҫ�����ڴ���ֽ���
�����  N/A
���أ�  XVOID *       -������ڴ�ָ��
˵����  ����ֵ���û�ǿ��ת��
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
������: XOS_MemCheck
���ܣ�  �ж��ڴ���Ƿ�����bucket���ڴ��,���ͷ��ڴ�
���룺  fid           - ���ܿ�id
                ptr           - �ڴ���׵�ַ
�����  N/A
����:   XSUCC  -    ��
        XERROR -    ��
˵���� 
************************************************************************/
XS32 XOS_MemCheck(XU32 fid, XVOID *ptr);


/************************************************************************
������: XOS_MemFree
���ܣ�  �ͷ�һ���ڴ��
���룺  fid           - ���ܿ�id
        ptr           - Ҫ�ͷŵ��ڴ��׵�ַ
�����  N/A
����:   XSUCC  -    �ɹ�
        XERROR -    ʧ��
˵���� 
************************************************************************/
XS32 XOS_MemFree(XU32 fid, XVOID *ptr);


/************************************************************************
������: XOS_MEMCtrl(����trilliumЭ��ջ��SSI�ڴ�ʹ��)
���ܣ� �����ڴ���ʹ��״̬
���룺  nbytes          - bucket�ڴ��Ĵ�С
       
�����  N/A
����:   ʹ�ñ���    
˵���� 
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

