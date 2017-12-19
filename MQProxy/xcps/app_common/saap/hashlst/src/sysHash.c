/*-----------------------------------------------------------
    sysHash.c -  Hash表c文件

    版权所有 2004 -2006 信威公司深研所BSC项目组.

    修改历史记录
    --------------------
    20.00.01,       08-4-2004,      李培冀      创建.
-----------------------------------------------------------*/
#ifdef  __cplusplus
extern  "C"
{
#endif

/*
*文件包含
*/

#include "saap_def.h"
#include "syshash.h"

/*
*宏定义
*/

#define HASH_NODE HASH_NODE_XW

/*
*外部函数声明
*/

/*
*本地函数定义
*/

/*---------------------------------------------------------------------
SYS_HashTblCreat    - 哈希表创建
参数说明：
	ulBinNum        - 输入，哈希表的哈希桶大小
	ulNodeNum       - 输入，哈希表的哈希节点数目
	fnFunc          - 输入，获取哈希桶索引的函数指针
	pstHashTbl      - 输出，返回的哈希表地址

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashTblCreat(XU32 ulBinNum, XU32 ulNodeNum, HASH_CNTN_GET_FUNC fnFunc, HASH_TABLE *pstHashTbl)
{
    XU32  i;

    //合法性检查
    if ((0 == ulBinNum) || (0 == ulNodeNum) || (XNULL == fnFunc) || (XNULL == pstHashTbl))
    {
        SYS_RETURN(XERROR);
    }

    //创建哈希节点表
    pstHashTbl->pstNode = (HASH_NODE_XW*)XOS_MemMalloc(FID_SAAP, sizeof(HASH_NODE_XW) * ulNodeNum);
    if (XNULL == pstHashTbl->pstNode)
    {
        SYS_RETURN(XERROR);
    }
    //创建哈希容器(哈希桶)表
    pstHashTbl->pstBin = (SYS_LISTENT_t*)XOS_MemMalloc(FID_SAAP, sizeof(SYS_LISTENT_t) * ulBinNum);
    if (XNULL == pstHashTbl->pstBin)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstNode);
        SYS_RETURN(XERROR);
    }

    //记录哈希桶大小和哈希表的节点大小和获取哈希桶的函数指针
    pstHashTbl->ulBinSize   = ulBinNum;
    pstHashTbl->ulNodeSize  = ulNodeNum;
    pstHashTbl->fnFunc      = fnFunc;

    //初始化哈希桶
    for (i = 0; i < ulBinNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstBin[i]);
    }
    //初始化哈希表的空闲节点链表
    SYS_ListIni(&pstHashTbl->stIdle);
    for (i = 0; i < ulNodeNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstNode[i].stLe);
        pstHashTbl->pstNode[i].ulIndex = i;
        SYS_ListAdd(pstHashTbl->stIdle.prev, &pstHashTbl->pstNode[i].stLe);
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_HashTblDel      - 哈希表注销

参数说明：
    pstHashTbl      - 输入，哈希表指针

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashTblDel(HASH_TABLE *pstHashTbl)
{
    //合法性检查
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }

    if(XNULL != pstHashTbl->pstNode)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstNode);
        pstHashTbl->pstNode = XNULL;
    }

    if(XNULL != pstHashTbl->pstBin)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstBin);
        pstHashTbl->pstBin = XNULL;
    }

    SYS_ListIni(&pstHashTbl->stIdle);
    pstHashTbl->ulNodeSize = 0;
    pstHashTbl->ulBinSize = 0;

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_HashTblClear      - 哈希表清空

参数说明：
    pstHashTbl      - 输入，哈希表指针

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashTblClear(HASH_TABLE *pstHashTbl)
{
    XU32  ulBinNum, ulNodeNum, i;

    //合法性检查
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }

    ulBinNum = pstHashTbl->ulBinSize;
    ulNodeNum = pstHashTbl->ulNodeSize;

    //初始化哈希桶
    for(i = 0; i < ulBinNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstBin[i]);
    }

    //初始化哈希表的空闲节点链表
    SYS_ListIni(&pstHashTbl->stIdle);
    for (i = 0; i < ulNodeNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstNode[i].stLe);
        SYS_ListAdd(pstHashTbl->stIdle.prev, &pstHashTbl->pstNode[i].stLe);
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_HashNodeInsert  - 往哈希表插入一个节点

参数说明：
    pstHashTbl      - 输入，哈希表指针
    ulHashKey       - 输入，关键字
    ulHashValue     - 输入，查找时返回的值
    pulIndex        - 输出，新节点在静态哈希表中的位置(索引)

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashNodeInsert(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulHashValue, XU32 *pulIndex)
{
    XU32      ulBinIndex, ulHashValue1, ulIndex1;
    HASH_NODE_XW   *pstNode;

    //合法性检查
    if(XNULL == pstHashTbl || XNULL == pulIndex)
    {
        SYS_RETURN(XERROR);
    }

    if (pstHashTbl->stIdle.next == &pstHashTbl->stIdle)//没有空余资源
    {
        SYS_RETURN(XERROR);
    }

    //找出哈希桶的索引号
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //判断哈希桶索引号的合法性
    if (ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    ulHashValue1 = ulHashValue;
    ulIndex1 = BLANK_ULONG;
    if (XSUCC == SYS_HashNodeSearch(pstHashTbl, ulHashKey, &ulHashValue1, &ulIndex1))//如果表中有相同项
    {
        SYS_RETURN(XERROR);
    }//9999

    //插入到桶中
    pstNode = (HASH_NODE_XW*)pstHashTbl->stIdle.next;
    SYS_ListDel(&pstNode->stLe);
    SYS_ListAdd(pstHashTbl->pstBin[ulBinIndex].prev, &pstNode->stLe);
    pstNode->ulHashKey = ulHashKey;
    pstNode->ulHashValue = ulHashValue;

    //填写新节点在静态哈希表中的位置(索引)
    *pulIndex = pstNode->ulIndex; //9999

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_HashNodeDel  - 从哈希表中删除一个节点

参数说明：
    pstHashTbl      - 输入，哈希表指针
    ulHashKey       - 输入，关键字
    pulIndex        - 输出，被删除的哈希节点的索引号

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashNodeDel(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulIndex)
{
    XU32      ulBinIndex;
    HASH_NODE_XW   *pstNode;

    //合法性检查
    if((XNULL == pstHashTbl) || (XNULL == pulIndex))
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //找出哈希桶的索引号
    if(XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //判断哈希桶索引号的合法性
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    *pulIndex = BLANK_ULONG;

    pstNode = (HASH_NODE_XW*)pstHashTbl->pstBin[ulBinIndex].next;
    while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
    {
        if (pstNode->ulHashKey == ulHashKey)//找到
        {
            SYS_ListDel(&pstNode->stLe);
            SYS_ListAdd(pstHashTbl->stIdle.prev, &pstNode->stLe);
            *pulIndex = pstNode->ulIndex;
            SYS_RETURN(XSUCC);
        }
        pstNode = (HASH_NODE*)pstNode->stLe.next;
    }
    SYS_RETURN(XERROR);
}

/*---------------------------------------------------------------------
SYS_HashNodeSearch  - 从哈希表中查找一个节点

参数说明：
    pstHashTbl      - 输入，哈希表指针
    ulHashKey       - 输入，哈希关键字
    pulHashValue    - 输出，哈希值
    pulIndex        - 输出，节点在静态哈希表中的位置(索引)

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashNodeSearch(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulHashValue, XU32 *pulIndex)
{
    XU32      ulBinIndex;
    HASH_NODE   *pstNode;

    //合法性检查
    if (XNULL == pstHashTbl || XNULL == pulHashValue || XNULL == pulIndex)
    {
        SYS_RETURN(XERROR);
    }
    if (XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //找出哈希桶的索引号
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //判断哈希桶索引号的合法性
    if (ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    pstNode = (HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
    while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
    {
        if (pstNode->ulHashKey == ulHashKey)//找到
        {
            *pulHashValue = pstNode->ulHashValue;
            *pulIndex = pstNode->ulIndex;
            SYS_RETURN(XSUCC);
        }
        pstNode = (HASH_NODE*)pstNode->stLe.next;
    }

    //注: 由于查找哈希节点不到是正常的，因此不使用[SYS_RETURN]
    return(XERROR);
}

/*---------------------------------------------------------------------
SYS_HashTblWalk  - 遍历哈希表的所有节点

参数说明：
    pstHashTbl      - 输入，哈希表指针
    fnVisit         - 输入，根据以关键字为参数的处理函数指针(参数形式为: (XS32 ulHashValue, XS32 ulIndex))
    ulHashKey       - fnVisit的参数类型，关键字
    ulHashValue     - fnVisit的参数类型，哈希值
    ulIndex         - fnVisit的参数类型，哈希节点在哈希表中的索引号

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashTblWalk(HASH_TABLE *pstHashTbl, XVOID (*fnVisit)(XU32 ulHashKey, XU32 ulHashValue, XU32 ulIndex))
{
    XU32      ulBinIndex;
    HASH_NODE   *pstNode;

    //合法性检查
    if (XNULL == pstHashTbl || XNULL == fnVisit)
    {
        SYS_RETURN(XERROR);
    }

    for (ulBinIndex = 0; ulBinIndex < pstHashTbl->ulBinSize; ulBinIndex++)
    {
        pstNode = (HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
        while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
        {
            fnVisit(pstNode->ulHashKey, pstNode->ulHashValue, pstNode->ulIndex);
            pstNode = (HASH_NODE*)pstNode->stLe.next;
        }
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashTblCreat    - 扩展哈希表创建

参数说明：
    ulBinNum     - 输入，扩展哈希表大小
    ulNodeNum     - 输入，扩展哈希表节点数目
    ulMaxSameKeyNode        - 输入，扩展哈希表允许相同关键字的最大节点数
    fnFunc - 输入，获取扩展哈希表的哈希桶的函数指针
    pstHashTbl              - 输出，返回的扩展哈希表地址

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashTblCreat(XU32 ulBinNum, XU32 ulNodeNum, XU32 ulMaxSameKeyNode, HASH_CNTN_GET_FUNC fnFunc, EXTEND_HASH_TABLE *pstHashTbl)
{
    XU32  i;

    //合法性检查
    if ((0 == ulBinNum) || (0 == ulNodeNum) || (0 == ulMaxSameKeyNode) || (XNULL == fnFunc) || (XNULL == pstHashTbl))
    {
        SYS_RETURN(XERROR);
    }

    //创建哈希节点表
    pstHashTbl->pstNode = (EXTEND_HASH_NODE *)XOS_MemMalloc(FID_SAAP, sizeof(EXTEND_HASH_NODE) * ulNodeNum);
    if (XNULL == pstHashTbl->pstNode)
    {
        SYS_RETURN(XERROR);
    }

    //创建哈希容器(哈希桶)表
    pstHashTbl->pstBin = (SYS_LISTENT_t *)XOS_MemMalloc(FID_SAAP, sizeof(SYS_LISTENT_t) * ulBinNum);
    if(XNULL == pstHashTbl->pstBin)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstNode);
        SYS_RETURN(XERROR);
    }

    //记录哈希表大小和哈希桶大小,获取哈希桶的函数指针,允许相同关键字的最大节点数
    pstHashTbl->ulBinSize             = ulBinNum;
    pstHashTbl->ulNodeSize            = ulNodeNum;
    pstHashTbl->fnFunc                = fnFunc;
    pstHashTbl->ulMaxSameKeyNode      = ulMaxSameKeyNode;

    //初始化哈希桶
    for(i = 0; i < ulBinNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstBin[i]);
    }

    //初始化哈希表的空闲节点链表
    SYS_ListIni(&pstHashTbl->stIdle);
    for (i = 0; i < ulNodeNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstNode[i].stLe);
        pstHashTbl->pstNode[i].ulIndex = i;
        SYS_ListAdd(pstHashTbl->stIdle.prev, &pstHashTbl->pstNode[i].stLe);
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashTblDel      - 扩展哈希表注销

参数说明：
    pstHashTbl      - 输入，哈希表指针

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashTblDel(EXTEND_HASH_TABLE *pstHashTbl)
{
    //合法性检查
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }

    //释放哈希节点链表
    if(XNULL != pstHashTbl->pstNode)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstNode);
        pstHashTbl->pstNode = XNULL;
    }

    //释放哈希桶链表
    if(XNULL != pstHashTbl->pstBin)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstBin);
        pstHashTbl->pstBin = XNULL;
    }

    SYS_ListIni(&pstHashTbl->stIdle);
    pstHashTbl->ulNodeSize = 0;
    pstHashTbl->ulBinSize = 0;

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashTblClear      - 扩展哈希表清空

参数说明：
    pstHashTbl      - 输入，扩展哈希表指针

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashTblClear(EXTEND_HASH_TABLE *pstHashTbl)
{
    XU32 ulBinNum, ulNodeNum, i;

    //合法性检查
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }

    ulBinNum = pstHashTbl->ulBinSize;
    ulNodeNum = pstHashTbl->ulNodeSize;

    //初始化哈希桶
    for(i = 0; i < ulBinNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstBin[i]);
    }

    //初始化哈希表的空闲节点链表
    SYS_ListIni(&pstHashTbl->stIdle);
    for (i = 0; i < ulNodeNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstNode[i].stLe);
        SYS_ListAdd(pstHashTbl->stIdle.prev, &pstHashTbl->pstNode[i].stLe);
    }

    SYS_RETURN(XSUCC);
}
/*---------------------------------------------------------------------
SYS_ExtendHashNodeSearch  - 从扩展哈希表中查找一个多个节点

参数说明：
    pstHashTbl      - 输入，哈希表指针
    ulHashKey       - 输入，关键字
    ulValueNum          - 输入，插入的节点数
    pulHashValues       - 输入，查找时返回的值数组

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashNodeSearch1(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulValueNum, XU32 pulHashValues[])
{
    XU32              ulBinIndex, i;
    EXTEND_HASH_NODE    *pstNode;

    //合法性检查
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //找出哈希桶的索引号
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //判断哈希桶索引号合法性
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    for (i = 0; i < ulValueNum; i++)
    {
        pstNode = (EXTEND_HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
        while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
        {
            if ((pstNode->ulHashKey == ulHashKey) && (pstNode->ulHashValue == pulHashValues[i]))
            {
                SYS_RETURN (XSUCC);
            }
            pstNode = (EXTEND_HASH_NODE*)pstNode->stLe.next;
        }
    }
    SYS_RETURN(XERROR);
}

/*---------------------------------------------------------------------
SYS_ExtendHashNodeInsert  - 往扩展哈希表插入一个或多个节点

参数说明：
    pstHashTbl    - 输入，扩展哈希表指针
    ulHashKey           - 输入，关键字
    ulValueNum          - 输入，插入的节点数
    pulHashValues       - 输入，查找时返回的值数组
    pulIndexs           - 输出，新节点在静态扩展哈希表中的位置(索引)数组

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashNodeInsert(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulValueNum, XU32 pulHashValues[], XU32 pulIndexs[])
{
    XU32      ulBinIndex, i;
    EXTEND_HASH_NODE   *pstNode;

    //合法性检查
    if(XNULL == pstHashTbl || XNULL == pulHashValues || XNULL == pulIndexs)
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //如果插入的节点数大于[最大相同关键字节点数],则失败
    if (ulValueNum > pstHashTbl->ulMaxSameKeyNode)
    {
        SYS_RETURN(XERROR);
    }
    //如果插入的值的个数为 0,则直接返回成功
    if (0 == ulValueNum)
    {
        SYS_RETURN(XSUCC);
    }

    //查找是否有相同用户
    if (XSUCC == SYS_ExtendHashNodeSearch1(pstHashTbl, ulHashKey, ulValueNum, pulHashValues))
    {
        SYS_RETURN(XERROR);
    }

    //查看有没有足够的空闲节点
    pstNode = (EXTEND_HASH_NODE*)pstHashTbl->stIdle.next;
    for (i = 0; i < ulValueNum; i++)
    {
        if (&pstNode->stLe == &pstHashTbl->stIdle)//没有足够的空闲节点
        {
            SYS_RETURN(XERROR);
        }
        pstNode = (EXTEND_HASH_NODE*)pstNode->stLe.next;
    }

    //找出哈希桶的索引号
    if(XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //判断哈希桶索引号合法性
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    for (i = 0; i < ulValueNum; i++)
    {
        pstNode = (EXTEND_HASH_NODE*)pstHashTbl->stIdle.next;
        SYS_ListDel(&pstNode->stLe);
        SYS_ListAdd(pstHashTbl->pstBin[ulBinIndex].prev, &pstNode->stLe);
        pstNode->ulHashKey = ulHashKey;
        pstNode->ulHashValue = pulHashValues[i];
        pulIndexs[i] = pstNode->ulIndex;
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashNodeDel  - 从扩展哈希表中删除指定关键字的部分节点或者所有节点

参数说明：
    pstHashTbl        - 输入，扩展哈希表指针
    ulHashKey               - 输入，需要删除的节点的关键字
    ulValueNum              - 输入，输入的值的个数，等于 BLANK_ULONG(0xFFFFFFFF)时表示要删除关键字相同的所有节点
    pulHashValues           - 输入，输入的各节点的值(当ulValueNum==BLANK_ULONG时，该字段不起作用)
    pulDelNum               - 输出，被删除的节点个数
    pulIndexs               - 输出，被删除的节点的相应索引号

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashNodeDel(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulValueNum, XU32 pulHashValues[], XU32 *pulDelNum, XU32 pulIndexs[])
{
    XU32      ulBinIndex, i;
    EXTEND_HASH_NODE   *pstNode;

    //合法性检查
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }
    if ((BLANK_ULONG != ulValueNum) && (ulValueNum > pstHashTbl->ulMaxSameKeyNode))
    {
        SYS_RETURN(XERROR);
    }

    //找出哈希桶的索引号
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //判断哈希桶索引号合法性
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    pulDelNum[0] = 0;
    for (i = 0; i < ulValueNum; i++)
    {
        pstNode = (EXTEND_HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
        while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
        {
            if ((pstNode->ulHashKey == ulHashKey) && (pstNode->ulHashValue == pulHashValues[i]))
            {
                pulIndexs[i] = pstNode->ulIndex;
                pulDelNum[0]++;
                SYS_ListDel(&pstNode->stLe);
                SYS_ListAdd(pstHashTbl->stIdle.prev, &pstNode->stLe);
                break;
            }
            pstNode = (EXTEND_HASH_NODE*)pstNode->stLe.next;
        }
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashNodeSearch  - 从扩展哈希表中查找一个节点

参数说明：
    pstHashTbl      - 输入，哈希表指针
    ulHashKey       - 输入，关键字
    pulValueNum     - 输出，查找到的哈希节点个数
    ulHashValues    - 输出，哈希值存放数组
    ulIndexs        - 输出，节点在静态哈希表中的位置(索引)

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashNodeSearch(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulValueNum, XU32 ulHashValues[], XU32 ulIndexs[])
{
    XU32      ulBinIndex;
    EXTEND_HASH_NODE   *pstNode;

    //合法性检查
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //找出哈希桶的索引号
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //判断哈希桶索引号合法性
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    *pulValueNum = 0;
    pstNode = (EXTEND_HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
    while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
    {
        if (pstNode->ulHashKey == ulHashKey)
        {
            if (*pulValueNum >= pstHashTbl->ulMaxSameKeyNode)
            {
                SYS_RETURN (XSUCC);
            }
            ulHashValues[*pulValueNum] = pstNode->ulHashValue;
            ulIndexs[*pulValueNum] = pstNode->ulIndex;
            pulValueNum[0]++;
        }
        pstNode = (EXTEND_HASH_NODE*)pstNode->stLe.next;
    }
    if (0 != pulValueNum[0])
    {
        SYS_RETURN(XSUCC);
    }
    else
    {
        SYS_RETURN(XERROR);
    }
}

XS32 SYS_HashNodeRebuild(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulHashValue, XU32 ulHashNodeIndex)
{
    SYS_RETURN(XSUCC);
}

XS32 SYS_ListIni(SYS_LISTENT_t *ent)
{
    ent->next = ent->prev = ent;
    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ListAdd    - 把一节点添加到双向链表中

参数说明：
    pstTblHeader      - 输入，双向链表头指针
    pstNewNode        - 输入，双向链新节点

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ListAdd(SYS_LISTENT_t *old, SYS_LISTENT_t *add)
{
    add->next = old->next;
    add->prev = old;
    old->next = add;
    (add->next)->prev = add;
    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ListDel    - 从双向链表中删除一个节点

参数说明：
    psDelNode      - 输入，需要删除的节点的指针

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
XS32 SYS_ListDel(SYS_LISTENT_t *del)
{
    if(XNULL == del)
    {
        SYS_RETURN(XERROR);
    }

    (del->prev)->next = del->next;
    (del->next)->prev = del->prev;
    del->next = del->prev = del;
    SYS_RETURN(XSUCC);
}

#ifdef  __cplusplus
}
#endif


