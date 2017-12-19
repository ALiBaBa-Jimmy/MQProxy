/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xoshash.h
**
**  description:  
**
**  author: zengjiandong
**
**  date:   2006.3.8
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author                  date                 modification            
**   zengjiandong        2006.3.8              create  
**************************************************************/
#ifndef _xoshash_H_
#define _xoshash_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "xosarray.h"

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
/*#define XOS_HASH_ERROR     (-1)*/
#define HASH_MAGIC_VALUE    0x0aaaff77
XOS_DECLARE_HANDLE(XOS_HHASH);             /*  hash对象句柄  */


/*-------------------------------------------------------------------------
                              结构和枚举声明
-------------------------------------------------------------------------*/
/************************************************************************
* XOS_HashListElement struct
* 这个结构体包括了用户的元素，同时提供一个向前
* 和向后的指针，给具有同一个Key值的元素构成
* 一个链表
* 
* next           - 指向具有相同key值的下一个元素
* prev           - 指向具有相同key值的上一个元素
* entryNum    - key值在hash表里的入口点
************************************************************************/
typedef struct hashListElement_tag XOS_HashListElement;
struct hashListElement_tag
{
    XOS_HashListElement*    next;
    XOS_HashListElement*    prev;
    XS32                    entryNum;
};


/*-------------------------------------------------------------------------
API 声明
-------------------------------------------------------------------------*/

/************************************************************************
* 用于比较hash元素中的某个内容是否相同
* hHash-->hash句柄
* ElemPtr-->元素的指针
* 比较相同返回XSUCC,否则XERROR
************************************************************************/
typedef   XU32  (*HashElemCmp)(  XOS_HHASH   hHash,
                                 XVOID*          ElemPtr );


/************************************************************************
* Hash函数的定义
* 输入：param                 - 元素
*                  paramSize           - 元素的大小(单位字节)
*                  hashSize              -  hash表的大小
* 返回 : hash 结果
************************************************************************/
typedef XU32 (*HashFunc) (
                          XVOID*   param, 
                          XS32     paramSize, 
                          XS32     hashSize);


/************************************************************************
* key 比较函数的定义
* 输入  : key1, key2              - 用于比较的两个Key值
*                keySize                   - key值的大小
* 返回 : 相等返回XTURE,否则返回XFALSE
************************************************************************/
typedef XBOOL (*HashKeyCmp) (
                             XVOID* key1, 
                             XVOID* key2, 
                             XU32 keySize );


/************************************************************************
* XOS_HASHFunc
* 功能: 定义对每个hash元素通用的函数
* 输入: hHash   - 对象句柄
*       elem    - 元素
*       param   - 参数
* 输出:
* 返回: 指向一个参数，给下次调用XOS_HASHFunc使用
************************************************************************/
typedef XVOID* (*XOS_HashFunc)(XOS_HHASH hHash, XVOID* elem, XVOID *param);


/************************************************************************
* t_XOSHASH structure
* 保存hash对象的相关信息
* hash                   - 要使用的hash函数
* compare             - 比较函数
* numKeys            - hash表的大小(根据key的数量定义)
* userKeySize        - key的数量
* userElemSize       - 元素的大小
* alignedKeySize    - key值的大小(字节)
* numElements       - 元素的数量
* curSize                - 当前元素的数量
* keys                    - 数组的指针，这个数组保存指向元素的指针
*                    
* elements              - 数组，用于存放元素
************************************************************************/
typedef struct
{
    HashFunc                    hash;
    XU32                        magicVal;
    HashKeyCmp                  compare;
    XU32                        numKeys;
    XU32                        userKeySize;
    XU32                        userElemSize;
    XU32                        alignedKeySize;
    XU32                        numElements;
    XU32                        curSize;
    XOS_ArrayElement*           keys;
    XOS_HARRAY                  elements;
} t_XOSHASH;

/************************************************************************
* 函数名：XOS_HashConstruct
* 功能 : 创建一个hash对象,使用默认的hash函数和比较函数
* 输入 : numOfKeys           - hash表的大小,应远大于表中的元素数
*        numOfElems          - hash表中元素的数量
*        keySize             - keys的大小
*        elemSize            - 元素的大小
* 输出 : 无
* 返回 : 成功返回hash表句柄,失败返回XNULL
* 说明 : hash函数和hash比较函数采用默认的函数，
*               需要配置自己的函数时，调用XOS_HashSetHashFunc()
*               和XOS_HashSetCompareFunc()配置
************************************************************************/
XPUBLIC  XOS_HHASH    XOS_HashConstruct( XU32             numOfKeys,
                                         XU32             numOfElems,
                                         XU32             keySize,
                                         XU32             elemSize,
                                         XCONST XCHAR*    name);

/************************************************************************
* XOS_HashMemCst
* 功能: 创建一个hash对象,(给没有内存初始化的时候用)
*                 
* 输入  :        numOfKeys           - hash表的大小.应远大于表中的元素数
*                numOfElems          - hash表中元素的数量
*                hashFunc            - hash函数
*                compareFunc         - keys值比较函数
*                keySize             - keys的大小
*                elemSize            - 元素的大小
* 输出 : 无
* 返回 : 成功返回hash对象句柄，失败返回XNULL
************************************************************************/
XPUBLIC  XOS_HHASH    XOS_HashMemCst(
                                     XU32             numOfKeys,
                                     XU32             numOfElems,
                                     XU32             keySize,
                                     XU32             elemSize ,
                                     XCONST XCHAR*    name);


/************************************************************************
* 函数名：XOS_HashDestruct
* 功能 : 删除hash对象，释放内存
* 输入 : hHash                   - HASH 对象句柄
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashDestruct(XOS_HHASH hHash);


/************************************************************************
* XOS_HashMemDst
* 功能: 删除hash对象，释放内存 (内存初始化之前可以用)
* 输入: hHash                     - HASH 对象句柄
* 输出: 无
* 返回: 成功返回XSUCC，失败返回XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashMemDst(   XOS_HHASH  hHash);


/************************************************************************
* 函数名：XOS_HashClear
* 功能 : 清空hash表到刚创建时的那个状态
* 输入 : hHash                         - HASH对象句柄
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashClear(XOS_HHASH hHash);


/************************************************************************
* XOS_HashHandleIsValid
* 功能: 判断hash句柄是否有效
* 输入: hHash                           - hash句柄
* 输出: 无
* 返回: 有效返回XTRUE，无效返回XFALSE
************************************************************************/
#define XOS_HashHandleIsValid(hash)  ((hash)? ((((t_XOSHASH*)(hash))->magicVal == HASH_MAGIC_VALUE)?XTRUE:XFALSE):XFALSE)
#if 0
XBOOL    XOS_HashHandleIsValid(    XOS_HHASH hHash  );
#endif


/************************************************************************
* 函数名：XOS_HashElemDel
* 功能 : 从hash表中删除一个元素
* 输入 : hHash          - HASH对象句柄
*               pLocation      - hash元素的位置指针,在add调用时返回
* 输出 : 无
* 返回 : 成功返回 XSUCC, 失败返回 XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashElemDel(XOS_HHASH   hHash,  XVOID* pLocation);


/************************************************************************
* 函数名：XOS_HashDelByElem
* 功能: 从hash表中删除一个指定位置上的元素
* 输入: hHash          - HASH对象句柄
*              pElem          - hash元素的真实位置指针
* 输出 : 无
* 返回 : 成功返回 XSUCC, 失败返回 XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashDelByElem( XOS_HHASH   hHash,  XVOID* pElem );


/************************************************************************
* 函数名：XOS_HashFindNext
* 功能: 查找与上次调用XOS_HashElemFind()有相同的key值
的下一个元素
* 输入  : hHash             - HASH对象句柄
*                pKey               - key值
*                pLocation         - 上次用HashFindNext() 或HashFind()查找到的元素
* 输出 : 无
* 返回 : 成功返回找到的元素的指针，否则XNULL
************************************************************************/
XPUBLIC  XVOID*    XOS_HashElemFindNext(
                                        XOS_HHASH hHash, 
                                        XVOID * pKey, 
                                        XVOID * pLocation);


/************************************************************************
* 函数名：XOS_HashGetElem
* 功能 : 通过位置信息取得元素
* 输入 : hHash       - HASH对象句柄
*        pLocation   - hash元素的位置指针,在add调用时返回
* 输出 : 无
* 返回 : 成功返回元素的首地址,失败返回XNULL
************************************************************************/
XPUBLIC  XVOID*   XOS_HashGetElem(XOS_HHASH  hHash, XVOID *pLocation );


/************************************************************************
* 函数名：XOS_HashSetElem
* 功能 : 把位置信息为location的元素设置成pElem
* 输入 : hHash        - HASH对象句柄
*        pLocation    - hash元素的位置指针,在add调用时返回
*        pElem        - 要设置元素的首地址
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32   XOS_HashSetElem(XOS_HHASH  hHash, XVOID *pLocation, XVOID* pElem);


/************************************************************************
* 函数名：XOS_HashGetKeyByElem
* 功能: 取得位置信息为location的元素的关键字
* 输入: hHash          - HASH对象句柄
*       pElem          - hash元素的真实位置的指针
* 输出: 无
* 返回: 成功返回key的首地址, 失败返回空
************************************************************************/
XPUBLIC  XVOID*  XOS_HashGetKeyByElem(XOS_HHASH hHash, XVOID* pElem );


/************************************************************************
* 函数名：XOS_HashWalk
* 功能 : 遍历hash表(注意要先写遍历函数)
* 输入 : hHash                        - hash表句柄
*        fFunc                        - 遍历函数
*        pParam                       - 遍历函数的参数
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32 XOS_HashWalk(XOS_HHASH hHash, XOS_HashFunc fFunc ,XVOID* pParam);


/************************************************************************
* 函数名：XOS_ hashSetHashFunc
* 功能 : 设置hash函数
* 输入 : hHash                - HASH对象句柄
*        fFunc                - hash函数
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32 XOS_HashSetHashFunc(XOS_HHASH hHash, HashFunc fFunc) ;


/************************************************************************
* 函数名：XOS_HashSetKeyCompareFunc
* 功能 : 设置hash关键字比较函数(该函数和hash函数适当的配合,可以实现模糊查询)
* 输入 : hHash               - HASH对象句柄
*        fFunc               - hash关键字比较函数
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32 XOS_HashSetKeyCompareFunc(XOS_HHASH hHash,HashKeyCmp fFunc);


/************************************************************************
* XOS_HashEbscSagFind
* 功能: 通过key值查找元素的位置
* 输入: hHash                        - HASH对象句柄
*       key                          - key值
* 输出: pIndex                       - hash  元素的索引号
* 返回: 成功返回指向第一个符合key值的元素的指针
*                查找失败返回XNULL指针
************************************************************************/
XPUBLIC  XVOID*    XOS_HashEbscSagFind(XOS_HHASH  hHash, XVOID* pKey, XS32* pIndex );


/************************************************************************
* XOS_HashEbscSagAdd
* 功能: 增加一个新的元素到hash表
*              
* 输入: hHash                          - HASH 对象句柄
*       pKey                           - key值
*       pElement                       - 添加的元素
*       allowSameKey                   - 是否允许有相同的key
* 输出: pIndex                         - hash元素存储的索引号
* 返回: 成功返回指向元素在hash表中的位置
*               失败返回XNULL
* 说明: 此接口暂时只给ebsc和sag使用
************************************************************************/
XPUBLIC  XVOID*  XOS_HashEbscSagAdd(
                                    XOS_HHASH  hHash,
                                    XVOID*        pKey,
                                    XVOID*        pElement,
                                    XBOOL         allowSameKey,
                                    XS32*          pIndex );


/************************************************************************
* 函数名：XOS_HashFind
* 功能 : 通过Key值查找元素的位置
* 输入 : hHash                 - HASH对象句柄
*        pKey                  - key值
* 输出 : 无
* 返回 : 成功返回指向第一个符合key值的元素的指针
*                查找失败返回XNULL指针
************************************************************************/
#define XOS_HashElemFind(hHash, pKey)  XOS_HashEbscSagFind(hHash, pKey, 0)


/************************************************************************
* 函数名：XOS_HashElemAdd
* 功能: 增加一个新的元素到hash表
*              如果设置allowSameKey参数为FALSE
*              则先判断hash表中是否已经存在一个元素key值
*              和新元素的key值相等，若有则新元素不被添加
* 输入: hHash                       - HASH 对象句柄
*       pKey                        - key值
*       pElement                    - 添加的元素
*       allowSameKey                - 是否允许有相同的key值
* 输出: 无
* 返回: 成功返回添加的元素所在的位置信息, 失败返回NULL
* 说明: 要求用户输入的key和element都不能为空才添加
************************************************************************/
#define XOS_HashElemAdd(hHash, pKey, pElement, allowSameKey)  \
XOS_HashEbscSagAdd(hHash, pKey, pElement, allowSameKey, 0)


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xoshash.h*/

