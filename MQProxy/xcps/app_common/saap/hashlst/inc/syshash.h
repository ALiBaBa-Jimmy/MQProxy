/*-----------------------------------------------------------
    sysHash.h -  Hash表头文件

    版权所有 2004 -2006 信威公司深研所BSC项目组.

    修改历史记录
    --------------------
    20.00.01,       08-4-2004,      李培冀      创建.
-----------------------------------------------------------*/
#ifndef _SYSHASH_H_
#define _SYSHASH_H_

#ifdef  __cplusplus
extern  "C"
{
#endif

#include "saap_def.h"

//------------------------------函数指针类型定义------------------------------------
//获取哈希桶的函数指针类型
typedef XS32 (* HASH_CNTN_GET_FUNC)(XU32 ulKey, XU32 *pulBin);

//------------------------------结构定义------------------------------------

//双向链表
typedef struct _SYS_LISTENT_t
{
    struct _SYS_LISTENT_t   *prev;  //前向指针
    struct _SYS_LISTENT_t   *next;  //后向指针
}SYS_LISTENT_t;

//哈希节点
typedef struct tagHashNode
{
    SYS_LISTENT_t     stLe;
    XU32              ulIndex;
    XU32              ulHashKey;
    XU32              ulHashValue;
}HASH_NODE_XW;

//哈希表管理结构
typedef struct  tagHashTbl
{
    SYS_LISTENT_t       stIdle;
    HASH_NODE_XW        *pstNode;
    XU32                ulNodeSize;
    SYS_LISTENT_t       *pstBin;
    XU32                ulBinSize;
    HASH_CNTN_GET_FUNC  fnFunc;
}HASH_TABLE;

//扩展哈希节点
typedef struct tagExtendHashNode
{
    SYS_LISTENT_t     stLe;
    XU32              ulIndex;
    XU32              ulHashKey;
    XU32              ulHashValue;
}EXTEND_HASH_NODE;

//扩展哈希表管理结构
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

//定时器句柄类型
typedef struct tagTimerNode *   HTIMER;

//定时器双向链表节点结构
typedef struct tagTimerNode
{
    SYS_LISTENT_t          stLe;           //双向链表头
    HTIMER                 *pTimer;        //定时器句柄
    XU8                    ucFid;          //功能块ID
    XU32                   ulLength;       //定时器长度
    XU32                   ulName;         //定时器名字
    XU32                   ulPara;         //定时器超时回传参数
    XU8                    ucMode;         //启动定时器方式: 循环或非循环
    XU32                   ulWalkTimes;    //节点被遍历的次数
#ifdef SYS_WD_TIMER
#define WDOG_ID int
    WDOG_ID                  wdId;           //看门狗ID
#endif
}TIMER_NODE;

//------------------------------函数声明------------------------------------
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


