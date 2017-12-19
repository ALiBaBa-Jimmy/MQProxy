/***************************************************************
 **
 **  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
 **  
 **  Core Network Department  platform team  
 **
 **  filename: xostrie.h
 **
 **  description:  
 **
 **  author: zengjiandong
 **
 **  date:   2006.4.6
 **
 ***************************************************************
 **                          history                     
 **  
 ***************************************************************
 **   author                date              modification            
 **   zengjiandong        2006.4.6              create  
 **************************************************************/
#ifndef _xostrie_H_
#define _xostrie_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                                包含头文件
-------------------------------------------------------------------------*/
#include "xostype.h"

/*-------------------------------------------------------------------------
                               宏定义
-------------------------------------------------------------------------*/
XOS_DECLARE_HANDLE(XOS_HTRIE);             /*  trie 对象句柄  */


/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/

typedef   struct xnode  t_TRIENODE;

/*  16P 节点中的一个元素 */
typedef struct
{
    XU32                 ip;                     /*  网络号*/
    XU32                 maskLen;          /*  掩码长度*/
    XVOID*             pRt;                   /*  路由信息*/
    t_TRIENODE*     pNext;               /*  指向下一级16P节点 */
}t_TRIEELEM;


/* Trie 树中的一个节点*/
struct xnode
{
    XU32                 usage;             /*节点已经有的元素的个数*/
    t_TRIENODE*     pParent;          /* 指向上级的指针*/
    t_TRIEELEM        node[16];         /*  16P 节点  */	
};


/************************************************************************
 * 函数名：pPrintElem
 * 功能 : 打印元素信息的函数
 * 输入 : pElem                        - 元素的地址
 * 输出 : 无
 * 返回 : 无
 ************************************************************************/
typedef  XVOID (*pPrintElem)( t_TRIEELEM* pElem);


/************************************************************************
 * 函数名：pAddElem
 * 功能 : 用户在树中增加一个节点的函数
 * 输入 : 无
 * 输出 : 无
 * 返回 : 成功返回16P 节点的地址,失败返回XNULL
 ************************************************************************/
typedef  XVOID* (*pAddNode)(XVOID);


/************************************************************************
 * 函数名：pDelElem
 * 功能 : 用户在树中删除一个16P 节点的函数
 * 输入 : pNode                       - 16P 节点的地址
 * 输出 : 无
 * 返回 : 成功返回trie树句柄,失败返回XNULL
 ************************************************************************/
typedef XVOID  (*pDelNode)(XVOID* pNode);


/************************************************************************
 * 函数名：pDownLoad
 * 功能 : 用户在trie树的一个16P 节点改变后执行的操作
 * 输入 : pNode                              - 被修改过的16P 节点的地址
 * 输出 : 无
 * 返回 : 无
 ************************************************************************/
typedef XVOID (*pDownLoad)( XVOID* pNode );


/************************************************************************
 * 函数名：pRootDownLoad
 * 功能 : 用户在trie树的一个根节点改变后执行的操作
 * 输入 : pElem                      - 被修改过的根节点一个元素的地址
 * 输出 : 无
 * 返回 : 无
 ************************************************************************/
typedef XVOID (*pRootDownLoad)( XVOID* pElem );


/*  key 结构体*/
typedef struct
{
    XU32 ip;                                         /*网络号*/
    XU32 masklen;                              /*掩码长度*/
}t_TRIEKEY;

/* Trie树控制结构体*/
typedef struct
{
    pPrintElem     printElemFunc;   /* 打印元素信息的函数，遍历时使用*/
    pAddNode      addNodeFunc;     /* 通知用户要增加一个16P 节点的函数*/
    pDelNode       delNodeFunc;      /* 通知用户要删除一个节点的函数*/
    pDownLoad    downloadFunc;    /* 在节点作修改后通知用户修改的节点*/
    pRootDownLoad rootDownloadFunc;/*在根节点作修改后通知用户*/

    XU32              numOfRt;             /* 增加的Rt 的数量*/
    XU32              numOfNode;        /* 当前16P 节点的数量*/
    XU32              maxNumOfNode; /* 16P 节点的最大可用数量*/
    XVOID* defMask;                       /*默认网关*/
    t_TRIEELEM*   pLongMask;         /* 指向长掩码树的根节点*/
    t_TRIEELEM*   pShortMask;        /* 指向短掩码树的根节点*/
}t_XOSTRIE;

/*-------------------------------------------------------------------------
                                           API 声明
-------------------------------------------------------------------------*/

/************************************************************************
 * 函数名：XOS_TrieHandleIsValid
 * 功能 : 判断创建的trie对象是否有效
 * 输入 : HTrie             - Trie树句柄
 * 输出 : 无
 * 返回 : 成功返回XOS_HTRIE,失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieHandleIsValid( XOS_HTRIE  HTrie);



/************************************************************************
 * 函数名：XOS_TrieElemAdd
 * 功能: 增加一个新的元素到trie树中
 * 输入: HTrie                       - Trie树句柄
 *               pKey                       - 关键字
 *               pElement                 - 要插入的trie树中的元素
 * 输出 : 无
 * 返回 : 添加成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieElemAdd(  XOS_HTRIE     HTrie,
                                                               XVOID*          pKey,
                                                               XVOID*          pElement);


/************************************************************************
 * 函数名：XOS_TrieElemDel
 * 功能: 在trie树中删除一个元素
 * 输入: HTrie                                      - Trie树句柄
 *               pKey                                      - 关键字
 *               pElement                                - 要删除的trie树中的元素
 * 输出 : 无
 * 返回 : 删除成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieElemDel(  XOS_HTRIE        HTrie, 
                                                             XVOID*              pKey,
                                                             XVOID*              pElement);


/************************************************************************
 * 函数名：XOS_TrieSearch
 * 功能: 在Trie树中查找一个元素
 * 输入: HTrie                                      - Trie树句柄
 *              pKey                                      - 关键字
 * 输出: pRt                                        - 查找到的Rt信息
                 失败或成功查找但Rt为NULL时输出为XNULL,
                 成功且Rt不为NULL则输出Rt的地址
 * 返回: 查找成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieSearch( XOS_HTRIE HTrie, XVOID*  pKey, XVOID** pRt);


/************************************************************************
 * 函数名：XOS_TrieALUSearch
 * 功能 : 在Trie树中查找一个元素，根据ip从高位到低位尽可能多地匹配
 * 输入 : HTrie                                     - Trie树句柄
 *               ip                                          - 网络号
 * 输出: pRt                                        - 查找到的Rt信息
                 失败或成功查找但Rt为NULL时输出为XNULL,
                 成功且Rt不为NULL则输出Rt的地址
 * 返回 : 查找成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieALUSearch( XOS_HTRIE HTrie, XU32  ip, XVOID** pRt);


/************************************************************************
 * 函数名：XOS_TrieTravel
 * 功能: 在Trie树中遍历
 * 输入: HTrie                                     - Trie树句柄
 * 输出: 无
 * 返回: 遍历成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieTravel( XOS_HTRIE  HTrie );


/************************************************************************
 * 函数名：XOS_TrieSetDefaultMask
 * 功能: 设置Trie树的默认网关(当查找无匹配时返回)
 * 输入: HTrie                                     - Trie树句柄
 * 输出: 无
 * 返回: 遍历成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieSetDefaultMask( XOS_HTRIE  HTrie, XVOID* pMask);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* xostrie.h */

