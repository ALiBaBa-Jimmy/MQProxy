/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xoshash.c
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
**   author                  date                modification            
**   zengjiandong        2006.3.8              create  
**   zengjiandong        2006.4.18             revise
**************************************************************/
#ifdef  __cplusplus
extern   "C"  {
#endif  /*  __cplusplus */


#include "xoshash.h"
#include "xoscfg.h"
#include "xostrace.h"
#include "xosmem.h"

/*-------------------------------------------------------------------------
              模块内部宏定义
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              模块内部数据结构
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              模块内部全局变量
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              模块内部函数
-------------------------------------------------------------------------*/
#if 0
/************************************************************************
* XOS_HashHandleIsValid
* 功能: 判断hash句柄是否有效
* 输入: hHash                           - hash句柄
* 输出: 无
* 返回: 有效返回XTRUE，无效返回XFALSE
************************************************************************/
XBOOL    XOS_HashHandleIsValid(    XOS_HHASH hHash  )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;

    if ( XNULL == hHash)
    {
        return( XFALSE );
    }

    if ( HASH_MAGIC_VALUE  == hash->magicVal )
    {
        return( XTRUE );
    }
    return ( XFALSE );
}
#endif


/************************************************************************
* XOS_HashHandleIsValid
* 功能: 判断n是否为素数，由IntFirstPrime调用
* 输入: n       - 待判断的整数
* 输出: 无
* 返回: 是素数返回XTRUE，非素数返回XFALSE
************************************************************************/
XSTATIC  XBOOL  IsPrime(  XS32 n  )
{
    XS32  i;
    for (i = 2; i < n/i/*sqrt(n)*/+1; i++)
    {
        if (!(n%i)) 
        {
            return XFALSE;
        }
    }

    return XTRUE;
}


/************************************************************************
* IntFirstPrime
* 功能: 求从n 起的第一个素数
* 输入: n              - 起始的整数
* 输出: 无
* 返回: 求得的素数
************************************************************************/
XSTATIC  XS32  IntFirstPrime(  XS32  n  )
{
    XS32  i = n;

    if (0 == (i & 1))
    {
        i++;
    }
    while (!IsPrime(i))
    {
        i += 2;
    }    
    return i;
}


/************************************************************************
* HashFind
* 功能: 通过关键字查找元素
* 输入:
* hHash                   - hash句柄
* key                       - 关键字
* 输出: 无
* 返回: 成功返回元素，失败则返回XNULL
************************************************************************/
XSTATIC  XVOID*  HashFind(   XOS_HHASH hHash,  XVOID* key   )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XU32 keyValue;
    XOS_HashListElement* hashElem;

    /*输入的句柄或者关键字为空，直接返回空*/
    if (XNULL == hHash ||XNULL == key)
    {
        return XNULL;
    }

    /* 计算hash桶号 */
    keyValue = (XU32)(hash->hash(key, (XS32)(hash->userKeySize), (XS32)(hash->numKeys))) % (hash->numKeys);

    /* 判断桶中是否存在元素 */
    if (hash->keys[keyValue] == XNULL)
    {
        return XNULL;
    }

    /* 不为空，开始在桶中查找 */
    hashElem = (XOS_HashListElement *)hash->keys[keyValue];

    /* 顺序查找是否有key 值相等的元素，由则返回 */
    while (XNULL != hashElem)
    {
        if (hash->compare(key, (XCHAR*)hashElem + sizeof(XOS_HashListElement), hash->userKeySize))
        {
            /* Found! */
            return hashElem;
        }
        hashElem = hashElem->next;
    }

    /* 没有找到，返回XNULL */
    return XNULL;
}


/************************************************************************
* HashFindNext
* 功能: 从桶的当前位置查找有相同key 值的元素
* 输入:
* hHash                  - hash句柄
* key                    - 关键字
* location               - 在hash 桶中当前位置
* 输出: 无
* 返回: 成功返回元素，失败则返回XNULL
************************************************************************/
XSTATIC  XVOID*   HashFindNext(
                               XOS_HHASH      hHash,
                               XVOID*         key,
                               XVOID*         location)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* hashElem;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    if (XNULL == location || XNULL == key)
    {
        return XNULL;
    }
    hashElem = (XOS_HashListElement *)location;

    /*从当前位置往下查找key 值相等的元素*/
    hashElem = hashElem->next;
    while (XNULL != hashElem)
    {
        if (hash->compare(key, (XCHAR*)hashElem + sizeof(XOS_HashListElement), hash->userKeySize))
        {
            /* Found! */
            return hashElem;
        }
        hashElem = hashElem->next;
    }
    return XNULL;
}


/************************************************************************
* HashDoAll
* 功能: hash 遍历函数
* 输入:
* hHash                 - hash 句柄
* func                  - 遍历函数(  必须先定义)
* param                 - 比较的参数
* 输出: 无
* 返回: 成功返回XSUCC，失败则返回XERROR
************************************************************************/
XSTATIC  XS32  HashDoAll(
                         XOS_HHASH           hHash,
                         XOS_HashFunc        func,
                         XVOID*              param)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XS32 cur;
    XCHAR  *ptr = XNULL;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == func)
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    cur = -1;

    /*在忙闲链中顺序遍历元素*/
    while (0 <= (cur = XOS_ArrayGetNextPos(hash->elements, cur)))
    {
        ptr = (XCHAR*)XOS_ArrayGetElemByPos(hash->elements, cur);
        if ( XNULL != ptr )
        {
            param = func(hHash, ((XCHAR*)ptr + sizeof(XOS_HashListElement) + hash->alignedKeySize) , param);
        }
    }
    return XSUCC;
}


/************************************************************************
* 默认的hash函数
* 输入 :
* param                - 元素
* paramSize            - 元素的大小(  单位字节)
* hashSize             - hash表的大小
* 返回 : hash 结果
************************************************************************/
XSTATIC  XU32 XOS_Hashstr(  XVOID*  param,  XS32    paramSize,  XS32    hashSize)
{
    XS32 hash = 0;
    XU32 ulSeed = 31; //31、131、1313、13131
    XCHAR* ptr = (XCHAR *)param;

    if (0 >= paramSize)
    {
        /* string is null terminated */
        while (*ptr)
        {
            //hash = hash << 1 ^ *ptr++;
			hash = hash*ulSeed + *ptr++; /*更新哈希算法*/
        }
    }
    else
    {
        while (0 < paramSize--  && *ptr)
        {
            //hash = hash << 1 ^ *ptr++;
            hash = hash*ulSeed + *ptr++; /*更新哈希算法*/
        }
    }

    return (hash % hashSize);
}


/************************************************************************
* 默认的比较函数. Checks byte-by-byte，完全匹配比较.
* 输入  :
* key1, key2            - 用于比较的两个Key值
* keySize               - key值的大小
* 返回 : 相等返回XTURE,否则返回XFALSE
************************************************************************/
XSTATIC  XBOOL  XOS_HashDefaultCompare( XVOID *key1,  XVOID* key2,  XU32 keySize)
{
    return (XBOOL)(XOS_MemCmp(key1, key2, keySize) == 0);
}


/*-------------------------------------------------------------------------
                 模块接口函数
-------------------------------------------------------------------------*/
/************************************************************************
* XOS_HashConstruct
* 功能: 创建一个hash对象, 使用默认的hash函数和比较函数
* 输入  :
*        numOfKeys           - hash表的大小.应远大于表中的元素数
*        numOfElems          - hash表中元素的数量
*        hashFunc               - hash函数
*        compareFunc         - keys值比较函数
*        keySize                 - keys的大小
*        elemSize               - 元素的大小
* 输出 : 无
* 返回 : 成功返回hash表句柄，失败返回XNULL
************************************************************************/
XPUBLIC  XOS_HHASH    XOS_HashConstruct(
                                       XU32             numOfKeys,
                                       XU32             numOfElems,
                                       XU32             keySize,
                                       XU32             elemSize ,
                                       XCONST XCHAR*     name)
{
    t_XOSHASH* phash;
    XS32      actualKeySize;
    actualKeySize = IntFirstPrime((XS32)numOfKeys); /* 求hash 桶的数量*/
    if ((XS32)numOfKeys <= 0 || (XS32)keySize <= 0 || (XS32)elemSize <= 0 || (XS32)numOfElems <= 0 || actualKeySize < 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error!\n");
        return XNULL;
    }
    /* 创建hash 桶空间*/
    if ((XS32)(actualKeySize * sizeof(XOS_ArrayElement)) < 0 || (XS32)(sizeof(t_XOSHASH) + actualKeySize * sizeof(XOS_ArrayElement)) < 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error, memory  keySize*numOfKeys  is too big!\n");
        return XNULL;
    }
    phash = (t_XOSHASH *)XOS_MemMalloc( (XU32)FID_ROOT, 
        (XS32)(sizeof(t_XOSHASH) + actualKeySize * sizeof(XOS_ArrayElement)));
    if( XNULL == phash )
    {
        return  XNULL;
    }

    /*HASH 桶指针初始化*/
    XOS_MemSet(((XU8*)phash + sizeof(t_XOSHASH)), 0, actualKeySize * sizeof(XOS_ArrayElement));

    /*hash 管理头结构的初始化*/
    phash->hash = XOS_Hashstr;
    phash->compare = XOS_HashDefaultCompare;
    phash->numKeys = (XU32)actualKeySize;
    phash->userKeySize = keySize;
    phash->userElemSize = elemSize;
    phash->alignedKeySize = (XU32)RV_ALIGN(keySize);
    phash->numElements = numOfElems;
    phash->curSize = 0;
    phash->keys = (XOS_ArrayElement*)((XU8*)phash + sizeof(t_XOSHASH));
    phash->magicVal = HASH_MAGIC_VALUE;

    /*创建忙闲链*/
    if ((XS32)(elemSize + phash->alignedKeySize + sizeof(XOS_HashListElement)) < 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error, elemSize+alignedKeySize+sizeof(XOS_HashListElement) is too big!\n");
        XOS_MemFree( (XU32)FID_ROOT, phash);
        return XNULL;
    }
    phash->elements =
        XOS_ArrayConstruct((XS32)(elemSize + phash->alignedKeySize + sizeof(XOS_HashListElement)), 
        (XS32)numOfElems, name);
    if (XNULL == phash->elements)
    {
        XOS_MemFree( (XU32)FID_ROOT, phash);
        return XNULL;
    }

    return (XOS_HHASH)phash;
}


/************************************************************************
* XOS_HashMemCst
* 功能: 创建一个hash对象,(给没有内存初始化的用)
* 输入  :
*                numOfKeys           - hash表的大小.应远大于表中的元素数
*                numOfElems          - hash表中元素的数量
*                hashFunc               - hash函数
*                compareFunc         - keys值比较函数
*                keySize                 - keys的大小
*                elemSize               - 元素的大小
* 输出 : 无
* 返回 : 成功返回hash对象句柄，失败返回XNULL
************************************************************************/
XPUBLIC  XOS_HHASH    XOS_HashMemCst(
                                    XU32             numOfKeys,
                                    XU32             numOfElems,
                                    XU32             keySize,
                                    XU32             elemSize ,
                                    XCONST XCHAR*     name)
{
    t_XOSHASH* phash;
    XS32      actualKeySize;

    actualKeySize = IntFirstPrime((XS32)numOfKeys); /* 求hash 桶的数量*/
    if ((XS32)numOfKeys <= 0 || (XS32)keySize <= 0 || (XS32)elemSize <= 0 || (XS32)numOfElems <= 0 || actualKeySize < 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error!\n");
        return XNULL;
    }
    /* 创建hash 桶空间*/
    if ((XS32)(actualKeySize * sizeof(XOS_ArrayElement)) < 0 || (XS32)(sizeof(t_XOSHASH) + actualKeySize * sizeof(XOS_ArrayElement)) < 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error, memory  keySize*numOfKeys  is too big!\n");
        return XNULL;
    }
    phash = (t_XOSHASH *)XOS_Malloc((XS32)(sizeof(t_XOSHASH) + actualKeySize * sizeof(XOS_ArrayElement)));
    if( XNULL == phash )
    {
        return  XNULL;
    }

    /*HASH 桶指针初始化*/
    XOS_MemSet(((XU8*)phash + sizeof(t_XOSHASH)), 0, actualKeySize * sizeof(XOS_ArrayElement));

    /* hash 管理头结构的初始化*/
    phash->hash = XOS_Hashstr;
    phash->compare = XOS_HashDefaultCompare;
    phash->numKeys = (XU32)actualKeySize;
    phash->userKeySize = keySize;
    phash->userElemSize = elemSize;
    phash->alignedKeySize = (XU32)RV_ALIGN(keySize);
    phash->numElements = numOfElems;
    phash->curSize = 0;
    phash->keys = (XOS_ArrayElement*)((XU8*)phash + sizeof(t_XOSHASH));
    phash->magicVal = HASH_MAGIC_VALUE;

    /* 创建忙闲链*/
    if ((XS32)(elemSize + phash->alignedKeySize + sizeof(XOS_HashListElement)) < 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error, elemSize+alignedKeySize+sizeof(XOS_HashListElement) is too big!\n");
        XOS_Free( phash);
        return XNULL;    
    }
    phash->elements =
        XOS_ArrayMemCst((XS32)(elemSize + phash->alignedKeySize + sizeof(XOS_HashListElement)), 
        (XS32)numOfElems, name);
    if (XNULL == phash->elements)
    {
        XOS_Free( phash);
        return XNULL;
    }

    return (XOS_HHASH)phash;
}


/************************************************************************
* XOS_HashDestruct
* 功能: 删除hash对象，释放内存
* 输入: hHash                     - HASH 对象句柄
* 输出: 无
* 返回: 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashDestruct(   XOS_HHASH  hHash)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    XOS_ArrayDestruct(hash->elements);
    XOS_MemFree( (XU32)FID_ROOT, hash);

    return XSUCC;
}


/************************************************************************
* XOS_HashMemDst
* 功能: 删除hash对象，释放内存 (内存初始化之前可以用)
* 输入: hHash                     - HASH 对象句柄
* 输出: 无
* 返回: 成功返回XSUCC，失败返回XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashMemDst(   XOS_HHASH  hHash)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    XOS_ArrayMemDst(hash->elements);
    XOS_Free( hash);

    return XSUCC;
}


/************************************************************************
* 函数名：XOS_HashClear
* 功能 : 清空hash表到刚创建时的那个状态
* 输入 : hHash                   - HASH对象句柄
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashClear( XOS_HHASH  hHash )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HARRAY ra = XNULL;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    /*HASH 桶指针初始化*/
    XOS_MemSet(((XU8*)hash + sizeof(t_XOSHASH)), 0, 
        (hash->numKeys) * sizeof(XOS_ArrayElement));

    hash->hash = XOS_Hashstr;
    hash->compare = XOS_HashDefaultCompare;
    hash->curSize = 0;
    hash->keys = (XOS_ArrayElement*)((XU8*)hash + sizeof(t_XOSHASH));
    if (XNULL == (ra = hash->elements))
    {
        XOS_MemFree( (XU32)FID_ROOT, hash);
        return XERROR;
    }

    XOS_ArrayInitClear((XOS_HARRAY)ra);   /* 初始化闲链 */
    return XSUCC;
}


/************************************************************************
* XOS_HashElemAdd
* 功能: 增加一个新的元素到hash表
*              如果设置allowSameKey参数为FALSE
               则先判断hash表中是否已经存在一个元素key值
               和新元素的key值相等，若有则新元素不被添加
* 输入:
*              hHash                          - HASH 对象句柄
*              pKey                           - key值
*              pElement                       - 添加的元素
*              allowSameKey                   - 是否允许有相同的key
* 输出: 无
* 返回: 成功返回指向元素在hash表中的位置
*       失败返回XNULL
* 说明: 要求用户输入的key和element都不能为空才添加
************************************************************************/
#if 0
XPUBLIC  XVOID*  XOS_HashElemAdd(
                                XOS_HHASH  hHash,
                                XVOID*        pKey,
                                XVOID*        pElement,
                                XBOOL         allowSameKey)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* newElem;
    XS32 keyValue;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == pKey || XNULL == pElement)
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /* 查找到有相同key 值的元素则返回元素，而不添加新元素*/
    if (!allowSameKey)
    {
        newElem = (XOS_HashListElement*)HashFind(hHash, pKey);
        if (XNULL != newElem)
        {
            return newElem;
        }
    }

    /* 在忙闲链中查找是否有空闲元素空间*/
    if (0 > XOS_ArrayAddExt(hash->elements, (XOS_ArrayElement*)&newElem))
    {
        /* Not found... */
        return XNULL;
    }
    /* 用hash 函数求散列桶*/
    keyValue = hash->hash(pKey, (XS32)(hash->userKeySize), 
        (XS32)(hash->numKeys)) % ((XU32)hash->numKeys);
    newElem->prev = XNULL;
    newElem->entryNum = keyValue;
    newElem->next = (XOS_HashListElement *)hash->keys[keyValue];
    if (XNULL != newElem->next)
    {
        newElem->next->prev = newElem;
    }

    /* 添加元素*/
    XOS_MemCpy((XCHAR*)newElem + sizeof(XOS_HashListElement), pKey, hash->userKeySize);
    XOS_MemCpy((XCHAR*)newElem + sizeof(XOS_HashListElement) + hash->alignedKeySize, 
        pElement, hash->userElemSize);
    hash->keys[keyValue] = (XOS_ArrayElement)newElem;
    hash->curSize++;
    return newElem;
}
#endif


/************************************************************************
* XOS_HashEbscSagAdd
* 功能: 增加一个新的元素到hash表
* 输入:
*              hHash                          - HASH 对象句柄
*              pKey                           - key值
*              pElement                       - 添加的元素
*              allowSameKey                   - 是否允许有相同的key                 
* 输出: pIndex                        - hash元素存储的索引号
* 返回: 成功返回指向元素在hash表中的位置
*       失败返回XNULL
* 说明: 此接口暂时只给ebsc和sag使用
************************************************************************/
XPUBLIC  XVOID*  XOS_HashEbscSagAdd(
                                    XOS_HHASH     hHash,
                                    XVOID*        pKey,
                                    XVOID*        pElement,
                                    XBOOL         allowSameKey,
                                    XS32*         pIndex )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* newElem;
    XS32 keyValue;
    XS32 index = -1;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == pKey || XNULL == pElement)
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /* 查找到有相同key 值的元素则返回元素，而不添加新元素*/
    if (!allowSameKey)
    {
        newElem = (XOS_HashListElement*)HashFind(hHash, pKey);
        if (XNULL != newElem)
        {
            return newElem;
        }
    }

    /* 在忙闲链中查找是否有空闲元素空间*/
    if (0 >(index=XOS_ArrayAddExt(hash->elements, (XOS_ArrayElement*)&newElem)))
    {
        /* Not found... */
        return XNULL;
    }
    /* 用hash 函数求散列桶*/
    keyValue = hash->hash(pKey, (XS32)(hash->userKeySize), 
        (XS32)(hash->numKeys)) % ((XU32)hash->numKeys);
    newElem->prev = XNULL;
    newElem->entryNum = keyValue;
    newElem->next = (XOS_HashListElement *)hash->keys[keyValue];
    if (XNULL != newElem->next)
    {
        newElem->next->prev = newElem;
    }

    /* 添加元素*/
    XOS_MemCpy((XCHAR*)newElem + sizeof(XOS_HashListElement), pKey, hash->userKeySize);
    XOS_MemCpy((XCHAR*)newElem + sizeof(XOS_HashListElement) + hash->alignedKeySize, 
        pElement, hash->userElemSize);
    hash->keys[keyValue] = (XOS_ArrayElement)newElem;
    hash->curSize++;
    if(pIndex != XNULLP)
    {
        *pIndex = index;
    }
    return newElem;
}


/************************************************************************
* 函数名：XOS_HashElemDel
* 功能: 从hash表中删除一个指定位置上的元素
* 输入:
*       hHash          - HASH对象句柄
*       pLocation      - hash元素的位置指针,在add调用时返回
* 输出 : 无
* 返回 : 成功返回 XSUCC, 失败返回 XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashElemDel( XOS_HHASH   hHash,  XVOID* pLocation )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* userElem;
    XS32 loc;

    if (XNULL == hash || XNULL == pLocation)
    {
        return XERROR;
    }

    userElem = (XOS_HashListElement *)pLocation;
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, userElem)))
    {
        return XERROR;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XERROR;
    }

    /* 将元素从hash 桶中删除*/
    if (XNULL == userElem->prev)
    {
        /* First element - update keys table */
        hash->keys[userElem->entryNum] = userElem->next;
    }
    else
    {
        userElem->prev->next = userElem->next;
    }

    if (XNULL != userElem->next)
    {
        userElem->next->prev = userElem->prev;
    }
    hash->curSize = hash->curSize - 1;

    /* 将空间加入到闲链中*/
    return XOS_ArrayDeleteByPos(hash->elements, XOS_ArrayGetByPtr(hash->elements, userElem));
}


/************************************************************************
* 函数名：XOS_HashDelByElem
* 功能: 从hash表中删除一个指定位置上的元素
* 输入:
*       hHash          - HASH对象句柄
*       pElem          - hash元素的真实位置指针
* 输出 : 无
* 返回 : 成功返回 XSUCC, 失败返回 XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashDelByElem( XOS_HHASH   hHash,  XVOID* pElem )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* userElem;
    XS32 loc;

    if (XNULL == hash || XNULL == pElem)
    {
        return XERROR;
    }
    userElem = (XOS_HashListElement *)((char*)pElem - hash->alignedKeySize - sizeof(XOS_HashListElement));
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, userElem)))
    {
        return XERROR;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XERROR;
    }

    /* 将元素从hash 桶中删除*/
    if (XNULL == userElem->prev)
    {
        /* First element - update keys table */
        hash->keys[userElem->entryNum] = userElem->next;
    }
    else
    {
        userElem->prev->next = userElem->next;
    }

    if (XNULL != userElem->next)
    {
        userElem->next->prev = userElem->prev;
    }
    hash->curSize = hash->curSize - 1;

    /* 将空间加入到闲链中*/
    return XOS_ArrayDeleteByPos(hash->elements, XOS_ArrayGetByPtr(hash->elements, userElem));
}


/************************************************************************
* XOS_HashElemFind
* 功能: 通过key值查找元素的位置
* 输入:
*       hHash                        - HASH对象句柄
*       key                          - key值
* 输出: 无
* 返回: 成功返回指向第一个符合key值的元素的指针
*       查找失败返回XNULL指针
************************************************************************/
#if 0
XPUBLIC  XVOID*    XOS_HashElemFind(XOS_HHASH  hHash, XVOID* pKey)
{
    XVOID *ptr = XNULL;

#ifdef XOS_HASH_DEBUG     
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /* 通过key 查找元素*/
    ptr = HashFind( (XOS_HHASH)hHash, pKey);
    if ( XNULL != ptr )
    {
        /*  返回元素*/
        return ( XOS_HashGetElem( (XOS_HHASH)hHash, ptr ) );
    }
    return XNULL;    /* 没有找到则返回XNULL*/
}
#endif


/************************************************************************
* XOS_HashEbscSagFind
* 功能: 通过key值查找元素的位置
* 输入:
*       hHash                      - HASH对象句柄
*       key                        - key值
* 输出: pIndex                     - hash  元素的索引号
* 返回: 成功返回指向第一个符合key值的元素的指针
*       查找失败返回XNULL指针
************************************************************************/
XPUBLIC  XVOID*    XOS_HashEbscSagFind(XOS_HHASH  hHash, XVOID* pKey, XS32* pIndex )
{
    XVOID *ptr = XNULL;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /* 通过key 查找元素*/
    ptr = HashFind( (XOS_HHASH)hHash, pKey);
    if ( XNULL != ptr )
    {
        /*  返回元素*/
        if(pIndex != XNULLP)
        {
            *pIndex = XOS_ArrayGetByPtr(((t_XOSHASH*)hHash)->elements, ptr);
        }
        return ( XOS_HashGetElem( (XOS_HHASH)hHash, ptr ) );
    }
    return XNULL;    /* 没有找到则返回XNULL*/
}


/************************************************************************
* XOS_HashElemFindNext
* 功能: 查找与上次调用XOS_HashElemFind()有相同的key值
        的下一个元素
* 输入 :
*        hHash             - HASH对象句柄
*        pKey              - key值
*        pLocation         - 上次用HashFindNext() 或HashFind()查找到的元素
* 输出 : 无
* 返回 : 成功返回找到的元素的指针，否则XNULL
************************************************************************/
XPUBLIC  XVOID*    XOS_HashElemFindNext(
                                       XOS_HHASH   hHash,
                                       XVOID*         pKey,
                                       XVOID*         pLocation )
{
    XVOID *ptr1 = XNULL, *ptr2 = XNULL;
    t_XOSHASH*  pHash = (t_XOSHASH *)XNULL;

    if ( ( !XOS_HashHandleIsValid(hHash) ) 
        || (XNULL == pKey) || (XNULL == pLocation) )
    {
        return( XNULL );
    }
    pHash = (t_XOSHASH *)hHash;
    ptr1 = ((XCHAR *)pLocation - (pHash->alignedKeySize)) - sizeof(XOS_HashListElement) ;

    /* 从当前位置往下查找相同key 值的元素*/
    ptr2 = HashFindNext( (XOS_HHASH)pHash, pKey, ptr1  );
    if ( XNULL != ptr2 )
    {
        return ( XOS_HashGetElem( (XOS_HHASH)pHash, ptr2 ) );
    }
    return XNULL;
}


/************************************************************************
* 函数名：XOS_HashGetElem
* 功能: 通过位置信息取得元素
* 输入:
*       hHash       - HASH对象句柄
*       pLocation   - hash元素的位置指针,在add调用时返回
* 输出 : 无
* 返回 : 成功返回元素的首地址,失败返回XNULL
************************************************************************/
XPUBLIC  XVOID*  XOS_HashGetElem( XOS_HHASH hHash,  XVOID* pLocation)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XS32 loc;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, pLocation)))
    {
        return XNULL;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XNULL;
    }

    if (XNULL == pLocation)
    {
        return XNULL;
    }

    /* 返回元素值*/
    return ((XCHAR*)pLocation + sizeof(XOS_HashListElement) + hash->alignedKeySize);
}


/************************************************************************
* 函数名：XOS_HashSetElem
* 功能: 把位置信息为location的元素设置成pElem
* 输入:
*       hHash           - HASH对象句柄
*       pLocation       - hash元素的位置指针,在add调用时返回
*       pElem           - 要设置元素的首地址
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC XS32 XOS_HashSetElem(XOS_HHASH  hHash, XVOID *pLocation, XVOID* pElem)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XS32 loc;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, pLocation)))
    {
        return XERROR;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XERROR;
    }

    if ( XNULL == pLocation || XNULL == pElem)
    {
        return XERROR;
    }

    /* 设置元素的值*/
    XOS_MemCpy((XCHAR*)pLocation+ 
        sizeof(XOS_HashListElement) + hash->alignedKeySize, pElem, hash->userElemSize);
    return XSUCC;
}


/************************************************************************
* 函数名：XOS_HashGetKeyByElem
* 功能: 取得位置信息为location的元素的关键字
* 输入:
*       hHash          - HASH对象句柄
*       pElem          - hash元素的位置的指针
* 输出: 无
* 返回: 成功返回key的首地址, 失败返回空
************************************************************************/
XPUBLIC  XVOID*  XOS_HashGetKeyByElem(XOS_HHASH hHash, XVOID* pElem )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XS32 loc;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == pElem )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /*判断pElem是否有效*/
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, (XCHAR*)pElem - hash->alignedKeySize - sizeof(XOS_HashListElement))))
    {
        return XNULL;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XNULL;
    }

    return (XVOID*)((XCHAR*)pElem - hash->alignedKeySize);/*返回关键字*/
}


/************************************************************************
* 函数名：XOS_HashWalk
* 功能: 遍历hash表(注意要先写遍历函数)
* 输入:
*       hHash                - hash表句柄
*       fFunc                - 遍历函数
*       pParam               - 遍历函数的参数
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashWalk( XOS_HHASH  hHash, XOS_HashFunc  fFunc, XVOID*   pParam)
{
#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == fFunc )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    return ( HashDoAll( (XOS_HHASH)hHash, (XOS_HashFunc)fFunc, pParam ) );
}


/************************************************************************
* XOS_HashSetHashFunc
* 功能: 设置hash表的hash函数
* 输入:
*       hHash         - HASH对象句柄
*       fFunc         - hash函数
* 输出: 无
* 返回: 成功返回XSUCC,否则XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashSetHashFunc( XOS_HHASH hHash,  HashFunc fFunc )
{
    t_XOSHASH *pHash = (t_XOSHASH *)XNULL;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    pHash = (t_XOSHASH *)hHash;
    if (XNULL != fFunc)
    {
        pHash->hash = fFunc ;                               /* 设置hash 函数*/
        return ( XSUCC );
    }
    else
    {
        return ( XERROR );
    }
}


/************************************************************************
* 函数名：XOS_HashSetKeyCompareFunc
* 功能: 设置hash关键字比较函数
(该函数和hash函数适当的配合,可以实现模糊查询)
* 输入:
*       hHash              - HASH对象句柄
*       fFunc              - hash关键字比较函数
* 输出 : 无
* 返回 : 成功返回XSUCC, 失败返回XERROR
************************************************************************/
XPUBLIC XS32  XOS_HashSetKeyCompareFunc( XOS_HHASH hHash,  HashKeyCmp fFunc )
{
    t_XOSHASH *pHash = XNULL;

    pHash = (t_XOSHASH *)hHash;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    if (XNULL != fFunc)
    {
        pHash->compare = fFunc ;                           /* 设置比较函数*/
        return ( XSUCC );
    }
    else
    {
        return ( XERROR );
    }
}


#ifdef   __cplusplus
}
#endif   /*  __cplusplus  */

