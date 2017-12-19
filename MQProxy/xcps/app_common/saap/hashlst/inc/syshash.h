/*-----------------------------------------------------------
    sysHash.h -  Hash��ͷ�ļ�

    ��Ȩ���� 2004 -2006 ������˾������BSC��Ŀ��.

    �޸���ʷ��¼
    --------------------
    20.00.01,       08-4-2004,      ���༽      ����.
-----------------------------------------------------------*/
#ifndef _SYSHASH_H_
#define _SYSHASH_H_

#ifdef  __cplusplus
extern  "C"
{
#endif

#include "saap_def.h"

//------------------------------����ָ�����Ͷ���------------------------------------
//��ȡ��ϣͰ�ĺ���ָ������
typedef XS32 (* HASH_CNTN_GET_FUNC)(XU32 ulKey, XU32 *pulBin);

//------------------------------�ṹ����------------------------------------

//˫������
typedef struct _SYS_LISTENT_t
{
    struct _SYS_LISTENT_t   *prev;  //ǰ��ָ��
    struct _SYS_LISTENT_t   *next;  //����ָ��
}SYS_LISTENT_t;

//��ϣ�ڵ�
typedef struct tagHashNode
{
    SYS_LISTENT_t     stLe;
    XU32              ulIndex;
    XU32              ulHashKey;
    XU32              ulHashValue;
}HASH_NODE_XW;

//��ϣ�����ṹ
typedef struct  tagHashTbl
{
    SYS_LISTENT_t       stIdle;
    HASH_NODE_XW        *pstNode;
    XU32                ulNodeSize;
    SYS_LISTENT_t       *pstBin;
    XU32                ulBinSize;
    HASH_CNTN_GET_FUNC  fnFunc;
}HASH_TABLE;

//��չ��ϣ�ڵ�
typedef struct tagExtendHashNode
{
    SYS_LISTENT_t     stLe;
    XU32              ulIndex;
    XU32              ulHashKey;
    XU32              ulHashValue;
}EXTEND_HASH_NODE;

//��չ��ϣ�����ṹ
typedef struct  tagExtendHashTbl
{
    SYS_LISTENT_t       stIdle;
    EXTEND_HASH_NODE    *pstNode;
    XU32                ulNodeSize;
    SYS_LISTENT_t       *pstBin;
    XU32                ulBinSize;
    XU32                ulMaxSameKeyNode;
    HASH_CNTN_GET_FUNC  fnFunc;
}EXTEND_HASH_TABLE;

//��ʱ���������
typedef struct tagTimerNode *   HTIMER;

//��ʱ��˫������ڵ�ṹ
typedef struct tagTimerNode
{
    SYS_LISTENT_t          stLe;           //˫������ͷ
    HTIMER                 *pTimer;        //��ʱ�����
    XU8                    ucFid;          //���ܿ�ID
    XU32                   ulLength;       //��ʱ������
    XU32                   ulName;         //��ʱ������
    XU32                   ulPara;         //��ʱ����ʱ�ش�����
    XU8                    ucMode;         //������ʱ����ʽ: ѭ�����ѭ��
    XU32                   ulWalkTimes;    //�ڵ㱻�����Ĵ���
#ifdef SYS_WD_TIMER
#define WDOG_ID int
    WDOG_ID                  wdId;           //���Ź�ID
#endif
}TIMER_NODE;

//------------------------------��������------------------------------------
XS32 SYS_HashTblCreat(XU32 ulBinNum, XU32 ulNodeNum, HASH_CNTN_GET_FUNC fnFunc, HASH_TABLE *pstHashTbl);
XS32 SYS_HashTblDel(HASH_TABLE *pstHashTbl);
XS32 SYS_HashTblClear(HASH_TABLE *pstHashTbl);
XS32 SYS_HashNodeInsert(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulHashValue, XU32 *pulIndex);
XS32 SYS_HashNodeDel(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulIndex);
XS32 SYS_HashNodeRebuild(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulHashValue, XU32 ulHashNodeIndex);
XS32 SYS_HashNodeSearch(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulHashValue, XU32 *pulIndex);
XS32 SYS_HashTblWalk(HASH_TABLE *pstHashTbl, XVOID(*fnVisit)(XU32 ulHashKey, XU32 ulHashValue, XU32 ulIndex));

XS32 SYS_ExtendHashTblCreat(XU32 ulBinNum, XU32 ulNodeNum, XU32 ulMaxSameKeyNode, HASH_CNTN_GET_FUNC fnFunc, EXTEND_HASH_TABLE *pstHashTbl);
XS32 SYS_ExtendHashTblDel(EXTEND_HASH_TABLE *pstHashTbl);
XS32 SYS_ExtendHashTblClear(EXTEND_HASH_TABLE *pstHashTbl);
XS32 SYS_ExtendHashNodeInsert(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulValueNum, XU32 pulHashValues[], XU32 pulIndexs[]);
XS32 SYS_ExtendHashNodeDel(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulValueNum, XU32 pulHashValues[], XU32 *pulDelNum, XU32 pulIndexs[]);
XS32 SYS_ExtendHashNodeSearch(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulValueNum, XU32 ulHashValues[], XU32 ulIndexs[]);
XS32 SYS_ListIni(SYS_LISTENT_t *ent);
XS32 SYS_ListAdd(SYS_LISTENT_t *old, SYS_LISTENT_t *add);
XS32 SYS_ListDel(SYS_LISTENT_t *del);
#ifdef  __cplusplus
}
#endif
#endif


