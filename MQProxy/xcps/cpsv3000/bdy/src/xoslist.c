/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xoslist.h
**
**  description:  
**
**  author: wangzongyou
**
**  date:   2006.6.26
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author              date              modification            
**  
**************************************************************/
#ifdef  __cplusplus
extern   "C"  {
#endif  /*  __cplusplus */

#include "xosarray.h"
#include "xoslist.h"
#include "xostrace.h"
/*-------------------------------------------------------------------------
               模块内部宏定义
-------------------------------------------------------------------------*/
#define INVALID_LIST_LOCATION                                  (-1)


/*-------------------------------------------------------------------------
               模块内部数据结构
-------------------------------------------------------------------------*/
typedef struct 
{
    XS32 prev;
    XS32 next;
    XCHAR *data;
} t_LISTELEM;


/*-------------------------------------------------------------------------
              模块内部全局变量
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              模块内部函数
-------------------------------------------------------------------------*/
/************************************************************************
* XOS_listAddNode
* 功能: 添加节点
* 输入:
*       hlist       - 链表句柄
*       elem        - 元素
* 输出: 无
* 返回: 成功返回数组中位置信息
*       失败返回XERROR
************************************************************************/
XSTATIC XS32 XOS_listAddNode(XOS_HLIST hlist, nodeType elem)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    XS32 loc;
    t_LISTELEM *elemList;

    if ( (loc = XOS_ArrayAddExt(raH, (XVOID **)&elemList)) < 0)
    {
        return XERROR;
    }

    if (elem)
    {
        XOS_MemCpy((XVOID *)&(elemList->data), elem, XOS_ArrayGetElemSize(raH)-sizeof(t_LISTELEM)+sizeof(XCHAR*));
    }
    else
    {
        XOS_MemSet((XVOID *)&(elemList->data), 0, XOS_ArrayGetElemSize(raH)-sizeof(t_LISTELEM)+sizeof(XCHAR*));
    }

    elemList->prev = elemList->next = XERROR;
    return loc;
}


/*-------------------------------------------------------------------------
                      模块接口函数
-------------------------------------------------------------------------*/
/************************************************************************
* XOS_listConstruct
* 功能: 创建一个链表 对象
* 输入:
*              elemSize                         - 元素的大小(字节为单位)
*              maxNumOfElements                 - 最大元素数
*              name                             - 数组名字
* 输出: 无
* 返回: 成功返回链表对象句柄
*               失败返回XNULL
************************************************************************/
XOS_HLIST XOS_listConstruct(
                           XS32          elemSize,
                           XS32          maxNumOfElements,
                           const XCHAR*  name)
{
    XOS_HARRAY raH;

    if (elemSize <= 0 || maxNumOfElements <= 0)
    {
        return XNULL;
    }
    raH = XOS_ArrayConstruct((XS32)(elemSize + sizeof(t_LISTELEM) - sizeof(XCHAR*)),
                             maxNumOfElements,
                             name);

    if (raH == XNULL)
    {
        return XNULL;
    }

    XOS_ArraySetFirstNode(raH, INVALID_LIST_LOCATION);
    XOS_ArraySetLastNode(raH, INVALID_LIST_LOCATION);
    return (XOS_HLIST)raH;
}


/************************************************************************
* XOS_listDestruct
* 功能: 释放一个链表 对象
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 
************************************************************************/
XVOID XOS_listDestruct(XOS_HLIST hlist)
{
    XOS_ArrayDestruct((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listClear
* 功能:清空链表到初始化状态
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 
************************************************************************/
XVOID XOS_listClear(XOS_HLIST hlist)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;

    XOS_ArrayInitClear(raH);
    XOS_ArraySetFirstNode(raH, INVALID_LIST_LOCATION);
    XOS_ArraySetLastNode(raH, INVALID_LIST_LOCATION);
}


/************************************************************************
* XOS_listSetCompareFunc
* 功能: 设置比较函数
* 输入:
*       hlist                        - 链表句柄
*       func                         - 比较函数指针
* 输出: 无
* 返回: 成功返回XSUCC,失败返回XERROR
************************************************************************/
XS32 XOS_listSetCompareFunc( XOS_HLIST hlist,  nodeCmpFunc func)
{
    return XOS_ArraySetCompareFunc((XOS_HARRAY)hlist, func);
}


/************************************************************************
* XOS_listElemIsVacant
* 功能: 判断节点是否为空
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回:检查结果为空返回XTURE, 不为空返回XFALSE
*      失败返回XERROR 
************************************************************************/
XS32 XOS_listElemIsVacant(XOS_HLIST hlist, XS32 location)                
{
    if (XNULL == hlist)
    {
        return XERROR;
    }
    return (XS32)XOS_ArrayElemIsVacant((XOS_HARRAY)hlist, location);
}


/************************************************************************
* XOS_listCurSize
* 功能: 当前链表中元素的个数
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 当前元素的个数,错误返回XERROR
************************************************************************/
XS32 XOS_listCurSize(XOS_HLIST hlist)
{
    return XOS_ArrayGetCurElemNum((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listMaxSize
* 功能:获取链表能存储元素的最大个数
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 最大可以存储元素的个数,错误返回XERROR
************************************************************************/
XS32     XOS_listMaxSize    (XOS_HLIST hlist)
{
    return XOS_ArrayGetMaxElemNum((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listHead
* 功能: 获取链表头部元素的索引
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 获取头节点的索引,错误返回XERROR
************************************************************************/
XS32 XOS_listHead(XOS_HLIST hlist)
{
    return XOS_ArrayGetFirstNode((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listTail
* 功能: 获取链表尾部元素的索引
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 获取尾节点的索引,错误返回XERROR
************************************************************************/
XS32 XOS_listTail(XOS_HLIST hlist)
{
    return XOS_ArrayGetLastNode((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listNext
* 功能: 获取下一个节点的索引
* 输入:
*       hlist                        - 链表句柄
*       location                     - 当前元素的索引
* 输出: 无
* 返回: 下一个节点的索引,错误返回XERROR
************************************************************************/
XS32 XOS_listNext(XOS_HLIST hlist, XS32 location)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    t_LISTELEM *elem;

    if ( !(elem = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, location)))
    {
        return XERROR;
    }
    else
    {
        return elem->next;
    }
}


/************************************************************************
* XOS_listPrev
* 功能: 获取前一个节点的索引
* 输入:
*       hlist                        - 链表句柄
*       location                     - 当前元素的索引
* 输出: 无
* 返回: 前一个节点的索引,错误返回XERROR
************************************************************************/
XS32 XOS_listPrev(XOS_HLIST hlist, XS32 location)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    t_LISTELEM *elem;

    if (!(elem = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, location)))
    {
        return XERROR;
    }
    else
    {
        return elem->prev;
    }
}


/************************************************************************
* XOS_listMaxUsage
* 功能: 获取最大使用过元素,便于统计
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 最大使用过的数量
************************************************************************/
XS32 XOS_listMaxUsage(XOS_HLIST hlist)
{
    return XOS_ArrayGetMaxUsageElemNum((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listMaxUsage
* 功能: 通过索引获取元素
* 输入:
*       hlist                        - 链表句柄
*       location                     - 元素在链表中的索引
* 输出: 无
* 返回: 节点元素的在链表中的首地址
************************************************************************/
nodeType XOS_listGetElem(XOS_HLIST hlist, XS32 location)
{
    t_LISTELEM *elem;

    if (!(elem = (t_LISTELEM *)XOS_ArrayGetElemByPos((XOS_HARRAY)hlist, location)))
    {
        return NULL;
    }
    else
    {
        return &(elem->data);
    }
}


/************************************************************************
* XOS_listGetByPtr
* 功能: 通过元素指针获取元素索引
* 输入:
*       hlist                        - 链表句柄
*       ptr                          - 元素在链表中的首地址
* 输出: 无
* 返回: 索引值,错误返回XERROR
************************************************************************/
XS32 XOS_listGetByPtr(XOS_HLIST hlist, XVOID *ptr)
{
    return XOS_ArrayGetByPtr((XOS_HARRAY)hlist, (XCHAR *)ptr - 2*sizeof(XS32));
}


/************************************************************************
* XOS_listCompare
* 功能: 直接通过比较函数查找匹配的元素
* 输入:
*       hlist                     - 链表句柄
*       location                  - 查找节点的起始索引
*       param                     - 比较函数参数
*       compare                   - 比较函数
* 输出: 无
* 返回: 匹配元素的索引值,错误返回XERROR
************************************************************************/
XS32 XOS_listCompare(XOS_HLIST hlist, XS32 location, XVOID *param, nodeCmpFunc compare)
{
    nodeType *elem;
    XS32 cur;

    for (cur=location; cur != INVALID_LIST_LOCATION; cur=XOS_listNext(hlist, cur)) 
    {
        if (!(elem = (nodeType*)XOS_listGetElem(hlist, cur)))
        {
            return XERROR; /* invalid element */
        }
        if ( (compare && compare(elem, param)) || elem == param)
        {
            return cur;
        }
    }

    return XERROR;
}


/************************************************************************
* XOS_listFind
* 功能:找出匹配的元素
* 输入:
*       hlist                        - 链表句柄
*       location                     - 查找节点的起始索引
*       param                        - 比较函数参数
* 输出: 无 
* 返回: 匹配元素的索引值,错误返回XERROR
************************************************************************/
XS32 XOS_listFind(XOS_HLIST hlist, XS32 location, XVOID *param)
{
    return XOS_listCompare(hlist, location, param, (nodeCmpFunc)XOS_ArrayGetCompareFunc((XOS_HARRAY)hlist));
}


/************************************************************************
* XOS_listAdd
* 功能: 向节点索引为location后加一个节点
* 输入:
*       hlist                   - 链表句柄
*       location                - 节点索引
*       elem                    - 节点的元素(添加节点前先用一个临时变量
*                                 存储要添加的元素，添加节点时在接口传入
*                                 变量地址，元素类型与创建时相同)
* 输出: 无
* 返回: 添加元素在链中的索引,错误返回XERROR
************************************************************************/
XS32 XOS_listAdd(XOS_HLIST hlist, XS32 location, nodeType elem)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    XS32 Apos, Bpos, Cpos=INVALID_LIST_LOCATION;
    t_LISTELEM *A, *B, *C;

    if ( (Bpos = XOS_listAddNode(hlist, elem)) < 0)
    {
        return XERROR;
    }
    if (Bpos == location)
    {
        location = XERROR;/*将节点插入到链表头*/
    }
    (location<0)?(Apos=INVALID_LIST_LOCATION):(Apos=location);

    B = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Bpos);
    A = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Apos);
    (A)?(Cpos=A->next):(Cpos=XOS_ArrayGetFirstNode(raH), Apos=INVALID_LIST_LOCATION);
    C = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Cpos);

    B->prev = Apos;
    B->next = Cpos;
    if (C)
    {
        C->prev = Bpos;
    }
    else
    {
        XOS_ArraySetLastNode(raH, Bpos);
    }
    if (A)
    {
        A->next = Bpos;
    }
    else
    {
        XOS_ArraySetFirstNode(raH, Bpos);
    }

    return Bpos;
}

/************************************************************************
* XOS_listAddHead
* 功能:向链表头部加一个节点
* 输入:
*       hlist       - 链表句柄
*       elem        - 节点的元素(添加节点前先用一个临时变量
*                              存储要添加的元素，添加节点时在接口传入
*                              变量地址，元素类型与创建时相同)
* 输出: 无
* 返回: 添加元素在链中的索引值,错误返回XERROR
************************************************************************/
XS32 XOS_listAddHead(XOS_HLIST hlist, nodeType elem)
{
    return XOS_listAdd(hlist, INVALID_LIST_LOCATION, elem);
}


/************************************************************************
* XOS_listAddTail
* 功能: 向尾部加一个节点
* 输入:
*       hlist       - 链表句柄
*       elem        - 节点的元素(添加节点前先用一个临时变量
*                              存储要添加的元素，添加节点时在接口传入
*                              变量地址，元素类型与创建时相同)
* 输出: 无
* 返回: 添加元素在链中的索引值,错误返回XERROR
************************************************************************/
XS32 XOS_listAddTail(XOS_HLIST hlist, nodeType elem)
{
    return XOS_listAdd(hlist, XOS_listTail(hlist), elem);
}


/************************************************************************
* XOS_listDelete
* 功能:删除指定位置的节点
* 输入:
*       hlist                        - 链表句柄
*       location                     - 要删除的节点的索引
* 输出: 无
* 返回: 成功XTRUE, 失败XFALSE
************************************************************************/
XBOOL XOS_listDelete(XOS_HLIST hlist, XS32 location)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    XS32 Apos, Cpos;
    t_LISTELEM *A, *B, *C;

    if (XNULLP == hlist)
    {
        return XFALSE;
    }
    if (XOS_ArrayElemIsVacant(raH, location) == XTRUE)
    {
        return XFALSE;
    }
    B = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, location);
    if (!B)
    {
        return XFALSE;
    }

    Apos = B->prev;
    Cpos = B->next;
    A = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Apos);
    C = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Cpos);

    if (C)
    {
        C->prev = Apos;
    }
    else
    {
        XOS_ArraySetLastNode(raH, Apos);
    }
    if (A)
    {
        A->next = Cpos;
    }
    else
    {
        XOS_ArraySetFirstNode(raH, Cpos);
    }

    XOS_ArrayDeleteByPos(raH, location);
    return XTRUE;
}


/************************************************************************
* XOS_listDeleteHead
* 功能: 删除头节点
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 成功XTRUE, 失败XFALSE
************************************************************************/
XBOOL XOS_listDeleteHead(XOS_HLIST hlist)
{
    return XOS_listDelete(hlist, XOS_listHead(hlist));
}


/************************************************************************
* XOS_listDeleteTail
* 功能:删除尾部的节点
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 成功XTRUE, 失败XFALSE
************************************************************************/
XBOOL XOS_listDeleteTail(XOS_HLIST hlist)
{
    return XOS_listDelete(hlist, XOS_listTail(hlist));
}


/************************************************************************
* XOS_listDeleteAll
* 功能: 删除所有的节点
* 输入: hlist                        - 链表句柄
* 输出: 无
* 返回: 
************************************************************************/
XBOOL XOS_listDeleteAll(XOS_HLIST hlist)
{
    while (XOS_listDeleteHead(hlist));
    return XTRUE;
}


#ifdef  __cplusplus
}
#endif  /*  __cplusplus */



