/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosarray.c
**
**  description:  
**
**  author: zengjiandong
**
**  date:   2006.3.21
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author              date              modification            
**   zengjiandong    2006.3.21       create  
**   zengjiandong    2006.3.23       revise
**************************************************************/
#ifdef  __cplusplus
extern   "C"  {
#endif  /*  __cplusplus */
    
#include "xosarray.h"
#include "xostrace.h"
#include "xoscfg.h"
#include "xosmem.h"
/*-------------------------------------------------------------------------
                           模块内部宏定义
-------------------------------------------------------------------------*/
/*如何对齐*/
#define ALIGN sizeof(XVOID*)

/*计算标志位占用空间的大小*/
#define BIT_VECTOR_SIZE(n)   ( ((XU32)n+7)/8 + (ALIGN - (((XU32)n+7)/8)%ALIGN)%ALIGN)

/*取下一个空闲元素的索引*/
#define NEXT_VACANT(ra, i)  (((vacantNode *)ELEM_DATA(ra, i))->nextVacant)

/*-------------------------------------------------------------------------
                        模块内部数据结构
-------------------------------------------------------------------------*/
typedef struct {
    XS32   nextVacant;                /* 指向下一个空闲的位置*/
} vacantNode;

/*-------------------------------------------------------------------------
                        模块内部全局变量
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                          模块内部函数
-------------------------------------------------------------------------*/
/*********************************************************************
*                  设置指定标志位的值，即是否空闲
*                  value的值为true表示占用,false表示空闲
*********************************************************************/
XSTATIC  XVOID  setBit(XCHAR *ptr, XU32 bit, XBOOL value)
{
    if (value)
    {
        ptr[bit>>3] |=  (0x80 >> (bit & 7));
    }
    else
    {
        ptr[bit>>3] &= ~(0x80 >> (bit & 7));
    }
}


XBOOL XOS_ArrayIsUesd(XOS_HARRAY raH, void* ptr)
{
    XS32 pos = 0;
    pos = XOS_ArrayGetByPtr(raH, ptr);
    if(pos != XERROR) {
        return (((getBit((BIT_VECTOR(raH)), (pos)))!= 0) ? (XTRUE):(XFALSE));
    }
    return XTRUE;
}

/**********************************************************************
*                               判断数组句柄是否有效
**********************************************************************/
#if 0
XBOOL   XOS_ArrayHandleIsValid(  XOS_HARRAY arrayH )
{
    t_XOSARRAY  *ptr=(t_XOSARRAY *)XNULL;
    if( XNULL == arrayH)
    {
        return( XFALSE );
    }
    ptr = (t_XOSARRAY *)arrayH;
    if ( ARRAY_MAGIC_VALUE  == ptr->magicVal )
    {
        return( XTRUE );
    }
    return ( XFALSE );
}
#endif

/***********************************************************************
*                           计算要申请的空间的大小
***********************************************************************/
XSTATIC  XU32  XOS_ArrayGetAllocationSize(XS32 elemSize, XS32 maxNumOfElements)
{
    return (sizeof(t_XOSARRAY) + 
        BIT_VECTOR_SIZE(maxNumOfElements) + 
        (maxNumOfElements * XOS_MAX((XU32)4, (XU32)elemSize)) );
}


/************************************************************************
* XOS_ArrayBuild
* 功能: 数组array 的初始化
* 输入:         
*               buffer                                    - 要初始化的空间
*               elemSize                                  - 数组元素的大小
*               maxNumOfElements                          - 数组元素的数量
*               name                                      - 数组名字
* 输出: 无
* 返回: 无
************************************************************************/
XSTATIC  XVOID  XOS_ArrayBuild(
                                XCHAR*          buffer,
                                XS32            elemSize,
                                XS32            maxNumOfElements,
                                XCONST XCHAR*   name)
{
    t_XOSARRAY *ra;
    XU32 len;
    
    if ( XNULL == buffer)
    {
        return ;
    }
    
    /* 将空间清0 */
    XOS_MemSet(buffer, 0, XOS_ArrayGetAllocationSize(elemSize, maxNumOfElements));
    
    /* 设置管理头结构的信息 */
    ra = (t_XOSARRAY *)buffer;
    
    ra->maxNumOfElements = maxNumOfElements;
    ra->arrayLocation    = (XCHAR*)ra + BIT_VECTOR_SIZE(maxNumOfElements) + sizeof(t_XOSARRAY);
    ra->curNumOfElements = 0;
    ra->sizeofElement    = elemSize;
    ra->maxUsage         = 0;
    ra->magicVal         = ARRAY_MAGIC_VALUE;
    
    if (XNULL != name)
    {
        len = (XU32)XOS_StrLen(name);
        XOS_StrNcpy(ra->name, name, XOS_MIN(len, ARRAY_NAME_SIZE - 1));
    }
    else
    {
        XOS_MemSet(ra->name, 0, ARRAY_NAME_SIZE);
    }
    
    XOS_ArrayInitClear((XOS_HARRAY)ra);   /* 初始化数组为闲链 */
    
    /*return ra;*/
}


/***********************************************************************
*                           设置数组指定位置元素的值
***********************************************************************/
XSTATIC  XVOID XOS_ArrayElemSet(  XOS_HARRAY raH,  XS32  location, XCONST  XVOID* src)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayElemSet , param invalid!\n");
        return ;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    if (src != XNULL)
    {
        XOS_MemCpy(ELEM_DATA(raH, location), (XVOID *)src, ((t_XOSARRAY *)raH)->sizeofElement);
    }
    else
    {
        XOS_MemSet(ELEM_DATA(raH, location), 0, (XU32)((t_XOSARRAY *)raH)->sizeofElement);
    }
}


/*-------------------------------------------------------------------------
                               模块接口函数
-------------------------------------------------------------------------*/

/************************************************************************
* XOS_ArrayConstruct
* 功能: 创建一个数组 对象
* 输入: 
*       elemSize                         - 元素的大小(字节为单位)
*       maxNumOfElements                 - 最大元素数
*       name                             - 数组名字
* 输出: 无
* 返回: 成功返回数组对象句柄
*       失败返回XNULL
************************************************************************/
XPUBLIC  XOS_HARRAY XOS_ArrayConstruct(  XS32 elemSize,  XS32  maxNumOfElements,  XCONST XCHAR*   name )
{
    t_XOSARRAY*   ra;
    XS32          size;
    XVOID*        ptr = XNULL;
    
    if (elemSize <= 0 || maxNumOfElements <= 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_ArrayConstruct()->Invalid param!\n");
        return XNULL;
    }
    
    /* 确保元素的大小至少为4 字节,并且按4 字节对齐 */
    if (elemSize < 4)
    {
        size = 4;
    }
    else
    {
        size = elemSize;
    }
    size = RV_ALIGN(size);
    
    /* 申请所需要的存储空间 */
    if ((XS32)XOS_ArrayGetAllocationSize(size, maxNumOfElements) < 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_ArrayConstruct()->XOS_ArrayGetAllocationSize is too big!\n");
        return XNULL;    
    }
    if (XNULL ==  (ptr = (XVOID *)XOS_MemMalloc((XU32)FID_ROOT, XOS_ArrayGetAllocationSize(size, maxNumOfElements))))
    {
        return XNULL;
    }
    
    /* 初始化管理结构体和数组 */
    XOS_ArrayBuild((XCHAR *)ptr, size, maxNumOfElements, name);
    
    ra = (t_XOSARRAY*)ptr;
    return (XOS_HARRAY)ra;         /* 返回数组句柄*/
}


/************************************************************************
* XOS_ArrayConstruct
* 功能: 创建一个数组 对象(内存初始化之前可用)
* 输入: 
*       elemSize                         - 元素的大小(字节为单位)
*       maxNumOfElements                 - 最大元素数
*       name                             - 数组名字
* 输出: 无
* 返回: 成功返回数组对象句柄
*               失败返回XNULL
************************************************************************/
XPUBLIC  XOS_HARRAY XOS_ArrayMemCst(  XS32 elemSize,  XS32  maxNumOfElements,  XCONST XCHAR*   name  )
{
    t_XOSARRAY*   ra;
    XS32          size;
    XVOID*        ptr = XNULL;
    
    if (elemSize <= 0 || maxNumOfElements <= 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_ArrayConstruct()->Invalid param!\n");
        return XNULL;
    }
    
    /* 确保元素的大小至少为4 字节,并且按4 字节对齐 */
    if (elemSize < 4)
    {
        size = 4;
    }
    else
    {
        size = elemSize;
    }
    size = RV_ALIGN(size);
    
    /* 申请所需要的存储空间 */
    if ((XS32)XOS_ArrayGetAllocationSize(size, maxNumOfElements) < 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_ArrayConstruct()->XOS_ArrayGetAllocationSize is too big!\n");
        return XNULL;
    }
    if (XNULL ==  (ptr = (XVOID *)XOS_Malloc(XOS_ArrayGetAllocationSize(size, maxNumOfElements))))
    {
        return XNULL;
    }
    
    /* 初始化管理结构体和数组 */
    XOS_ArrayBuild((XCHAR *)ptr, size, maxNumOfElements, name);
    
    ra = (t_XOSARRAY*)ptr;
    return (XOS_HARRAY)ra;         /* 返回数组句柄*/
}


/************************************************************************
* XOS_ArrayDestruct
* 功能 : 释放数组，释放占用的内存
* 输入 : arrayH                      - 要释放的数组对象句柄
* 输出 : 无
* 返回 : 无
************************************************************************/
XPUBLIC  XVOID XOS_ArrayDestruct( XOS_HARRAY arrayH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayDestruct , param invalid!\n");
        return ;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    ra->magicVal = 0;/*先破坏句柄再释放*/
    if (XERROR == XOS_MemFree((XU32)FID_ROOT, (t_XOSARRAY*)arrayH))        /* 释放数组空间*/
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayDestruct , mem free fail!\n");
        return ;
    }
}


/************************************************************************
* XOS_ArrayMemDst
* 功能 : 释放数组，释放占用的内存(内存初始化之前用)
* 输入 : arrayH                      - 要释放的数组对象句柄
* 输出 : 无
* 返回 : 无
************************************************************************/
XPUBLIC  XVOID XOS_ArrayMemDst( XOS_HARRAY arrayH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayDestruct , param invalid!\n");
        return ;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    XOS_Free(ra);        /* 释放数组空间*/
}


/************************************************************************
* XOS_ArrayInitClear
* 功能: 数组创建时调用，初始化一个空的元素表
* 输入: raH                        - 数组对象句柄
* 输出: 无
* 返回: 无
************************************************************************/
XPUBLIC  XVOID  XOS_ArrayInitClear(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    XS32 i;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayInitClear , param invalid!\n");
        return ;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    ra->curNumOfElements = 0;
    ra->firstNodeLocation = ra->lastNodeLocation = -1;
    ra->maxUsage = 0;
    
    /*  将数组初始化为闲链 */
    if (ra->maxNumOfElements > 0)
    {
        ra->firstVacantElement = 0;
    }
    else
    {
        ra->firstVacantElement = -1;
    }
    ra->lastVacantElement = ra->maxNumOfElements-1;
    for (i=0; i<ra->maxNumOfElements; i++)
    {
        ((vacantNode *)ELEM_DATA(ra, i))->nextVacant = i+1;
    }
    
    ((vacantNode *)ELEM_DATA(ra, ra->maxNumOfElements-1))->nextVacant = XERROR;
    
    /* 标志位全部清0，即数组的元素均空闲 */
    XOS_MemSet((XVOID*)BIT_VECTOR(ra), 0, BIT_VECTOR_SIZE(ra->maxNumOfElements)); /* make all free */
}


/************************************************************************
* XOS_ArraySetCompareFunc
* 功能: 设置比较函数给XOS_ArrayFind()使用
* 输入: 
*                arrayH             - 数组对象句柄
*                func               - 比较函数
* 输出: 无
* 返回: 成功返回XSUCC,失败返回XERROR
************************************************************************/
XPUBLIC  XS32 XOS_ArraySetCompareFunc( XOS_HARRAY raH,  XOS_ArrayCompare func)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
#ifdef XOS_ARRAY_DEBUG    
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArraySetCompareFunc , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    ra->compare = func;         /*  设置比较函数*/
    return XSUCC;
}


/************************************************************************
* XOS_ArrayAdd
* 功能: 分配一个元素到数组,元素被分配到数组的位置
由返回值表明
* 输入:  
*        arrayH             - 数组对象句柄
*        elem               - 要分配的元素
*                             如果该值为XNULL,则元素的值为零
* 输出: 无
* 返回: 成功返回元素在数组中的下标值(非负值)
*               失败返回XERROR
************************************************************************/
XPUBLIC  XS32   XOS_ArrayAdd( XOS_HARRAY arrayH, XOS_ArrayElement elem)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    XS32  vLocation;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayAdd , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /* 先判断数组中是否有空闲的空间 */
    if ( (vLocation = ra->firstVacantElement) == XERROR)
    {
        XOS_PRINT(MD(FID_ROOT, PL_WARN), "XOS_ArrayAdd (%s): Array full (%d elements)\n", ra->name, ra->maxNumOfElements);
        return XERROR;
    }
    
    /* 有剩余空间，则在闲链中取得空间，置标志位 */
    ra->firstVacantElement = ((vacantNode *)ELEM_DATA(ra, vLocation))->nextVacant;
    if (ra->firstVacantElement == XERROR)
    {
        ra->lastVacantElement = XERROR;
    }
    setBit(BIT_VECTOR(ra), (XU32)vLocation, XTRUE); /* make it occupied */
    
    /* 在空闲位置上设置元素的值 */
    XOS_ArrayElemSet(arrayH, vLocation, elem);
    
    /* 设置数组的统计数据 */
    ra->curNumOfElements++;
    if (ra->curNumOfElements > ra->maxUsage)
    {
        ra->maxUsage = ra->curNumOfElements;
    }
    
    /* 返回增加的元素的位置 */
    return vLocation;
}


/************************************************************************
* XOS_ArrayAddExt
* 功能: 从数组中申请一个元素的位置
* 输入 : arrayH            - 数组对象的句柄
* 输出 : pOutElem          - 申请到的元素的指针，如果等于XNULL则没有申请到.
* 返回 : 成功返回非负值的位置，失败则返回XERROR
************************************************************************/
XPUBLIC  XS32   XOS_ArrayAddExt( XOS_HARRAY arrayH,  XOS_ArrayElement *pOutElem)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    XS32 vLocation;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayAddExt , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /*  先判断数组里是否有空闲空间 */
    if ( (vLocation = ra->firstVacantElement) == XERROR)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_WARN), "XOS_ArrayAddExt (%s): Array full (%d elements)\n", ra->name, ra->maxNumOfElements);
        if (pOutElem != XNULL)
        {
            *pOutElem = XNULL;
        }
        return XERROR;
    }
    
    /* 从闲链中取得元素空间，并且设置标志位 */
    ra->firstVacantElement = ((vacantNode *)ELEM_DATA(ra, vLocation))->nextVacant;
    if (ra->firstVacantElement == XERROR)
    {
        ra->lastVacantElement = XERROR;
    }
    setBit(BIT_VECTOR(ra), vLocation, XTRUE); /* make it occupied */
    /* Set statistical information */
    ra->curNumOfElements++;
    
    if (ra->curNumOfElements > ra->maxUsage)
    {
        ra->maxUsage = ra->curNumOfElements;
    }
    
    /*  输出申请到的元素的地址*/
    if (pOutElem != XNULL)
    {
        *pOutElem = (XOS_ArrayElement)ELEM_DATA(arrayH, vLocation);
    }
    
    return vLocation;     /* Return the location */
}


/************************************************************************
* XOS_ArrayGetElemByPos
* 功能: 获取指定位置上的元素
* 输入 : arrayH             - 数组对象句柄
* 输出 : location           - 元素在数组中的位置
* 返回 : 返回要获取的元素,location小于零返回XNULL
************************************************************************/
XPUBLIC  XOS_ArrayElement XOS_ArrayGetElemByPos( XOS_HARRAY  arrayH,  XS32  location)
{
    t_XOSARRAY* ra = (t_XOSARRAY *)arrayH;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetElemByPos , param invalid!\n");
        return XNULL;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /*  该位置是否有效*/
    if ((location < 0) || (unsigned)location >= (unsigned)(ra->maxNumOfElements))
    {
        return XNULL;
    }
    
    /*  该位置是否有元素占用*/
    if (XOS_ArrayElemIsVacant(arrayH, location) == XTRUE)
    {
        return XNULL;
    }
    
    return (XOS_ArrayElement)ELEM_DATA(arrayH, location);/* 返回元素*/
}


/************************************************************************
* XOS_ArrayDeleteByPos
* 功能: 删除指定位置上的元素
* 输入 : arrayH                       - 数组对象句柄
* 输出 : location                     - 要删除的元素的位置
* 返回 : 成功返回XSUCC，失败返回XERROR
************************************************************************/
XPUBLIC  XS32   XOS_ArrayDeleteByPos( XOS_HARRAY arrayH,  XS32 location)
{
    t_XOSARRAY *pArrayH = XNULL;
    pArrayH = (t_XOSARRAY *)arrayH;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayDelByPos , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /* 位置是否有效,是否存储了元素*/
    if (location < 0 || location >= pArrayH->maxNumOfElements)
    {
        return XERROR;
    }
    if (XOS_ArrayElemIsVacant(arrayH, location) == XTRUE)
    {
        return XERROR;
    }
    
    /* 将它加入到闲链中 */
    pArrayH->curNumOfElements--;
    
    /*DEBUG下,释放的数组元素放在闲链头;非DEBUG下放到闲链尾*/
#ifdef XOS_ARRAYMEM_DEBUG
    ((vacantNode *)ELEM_DATA(arrayH, location))->nextVacant = pArrayH->firstVacantElement;
    pArrayH->firstVacantElement = location;
    if (pArrayH->lastVacantElement == XERROR)
    {
        pArrayH->lastVacantElement = location;
    }
#else
    ((vacantNode *)ELEM_DATA(arrayH, location))->nextVacant = XERROR;
    if (pArrayH->lastVacantElement != XERROR)
    {
        ((vacantNode *)ELEM_DATA(arrayH, pArrayH->lastVacantElement))->nextVacant = location;
    }
    else
    {
        pArrayH->firstVacantElement = location;
    }
    
    pArrayH->lastVacantElement = location;
#endif
    setBit(BIT_VECTOR(arrayH), (XU32)location, XFALSE); /* 标志位置0 */
    
    return XSUCC;
}

#if 0
/************************************************************************
* XOS_ArrayElemIsVacant
* 功能: 检查指定位置上的元素是否为空
* 输入 :  
*         arrayH                   - 数组对象句柄
*         location                 - 要检查的位置
* 输出 : 无
* 返回 : 检查结果为空返回XTURE, 不为空返回XFALSE
*                失败返回XERROR
************************************************************************/
XPUBLIC  XBOOL XOS_ArrayElemIsVacant(  XOS_HARRAY arrayH,  XS32 location)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayElemIsVacant , param invalid!\n");
        return XTRUE;
    }
    if (!ra || location<0 || location>=ra->maxNumOfElements)
    {
        return XTRUE;
    }
    
    /*  通过标志位判断该位置的元素是否为空*/
    return ((getBit((BIT_VECTOR(ra)), (location))) != 0) ? (XFALSE):(XTRUE);
}

#endif

/************************************************************************
* XOS_ArrayGetFirstPos
* 功能: 从数组下标0开始获取有元素的第一个位置
* 输入 : arrayH                - 数组对象句柄
* 输出 : 无
* 返回 : 成功返回第一个元素的位置值，否则返回ERROR
*************************************************************************/
XPUBLIC  XS32  XOS_ArrayGetFirstPos(   XOS_HARRAY arrayH )
{
    XS32   i = 0; 
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetFirstPos , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    for ( i=0; i < ( (t_XOSARRAY*)arrayH)->maxNumOfElements; i++ )
    {
        if ( !XOS_ArrayElemIsVacant(arrayH, i ) )/* 元素是否为空*/
        {
            return ( i );
        }
    }
    return XERROR;
}


/************************************************************************
* XOS_ArrayGetNextPos
* 功能: 获取当前位置开始的下一个有元素的位置
* 输入 : arrayH                - 数组对象句柄
* 输出 : location              - 开始的位置
* 返回 : 成功返回下一个元素的位置，否则返回XERROR
************************************************************************/
XPUBLIC XS32  XOS_ArrayGetNextPos(  XOS_HARRAY  arrayH,  XS32  location)
{
    t_XOSARRAY* ra = (t_XOSARRAY *)arrayH;
    XS32 i;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetNextPos , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    if (location < 0)
    {
        i = 0;
    }
    else
    {
        i = location + 1;
    }
    if (i >= ra->maxNumOfElements)
    {
        return XERROR; /* out of bounds */
    }
    
    while ((i < ra->maxNumOfElements) && XOS_ArrayElemIsVacant(arrayH, i))
    {
        i++;
    }
    
    if (i < ra->maxNumOfElements)
    {
        return i;
    }
    else
    {
        return XERROR;
    }
}


/************************************************************************
* XOS_ArrayGetByPtr
* 功能: 根据元素的值获取元素在数组中的位置
* 输入 : arrayH                      - 数组对象句柄
* 输出 : ptr                         - 元素的指针
* 返回 : 成功返回元素的位置值，否则返回XERROR
************************************************************************/
XPUBLIC  XS32  XOS_ArrayGetByPtr( XOS_HARRAY arrayH, XVOID *ptr)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    XS32  location;
    XS32  position;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )/* 句柄是否有效*/
    {
    /* 此处调用xos_trace将导致递归，所以取消
    XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetByPtr , param invalid!\n");
        */
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /*  元素的地址是否在范围内*/
    if (((XCHAR *)ptr < (XCHAR *)(ra->arrayLocation)) ||
        ((XCHAR *)ptr >= (XCHAR *)(ra->arrayLocation) + ra->maxNumOfElements*(ra->sizeofElement)))
    {
        return XERROR;
    }
    
    /* 通过地址计算元素的数组下标 */
    location = (XS32)((XCHAR *)ptr - ra->arrayLocation);
    
    /* Make sure the pointer is aligned properly */
    if (location % ra->sizeofElement != 0)
    {   /* alignment */
        return XERROR;
    }
    position = location/(ra->sizeofElement);
    
    return position;
}


/************************************************************************
* XOS_ArrayFind
* 功能: 根据参数查找数组里符合的元素的位置
* 输入 : arrayH                - 数组对象句柄
* 输出 : param                 - 比较函数的参数
* 返回 : 成功返回元素的位置值，否则返回ERROR
*************************************************************************/
XPUBLIC  XS32  XOS_ArrayFind( XOS_HARRAY arrayH,  XVOID *param)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    XS32 i;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayFind , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    for (i=0; i < ra->maxNumOfElements; i++)
    {
        if (!XOS_ArrayElemIsVacant(arrayH, i))
        {
            if (ra->compare)
            {
                if (ra->compare(ELEM_DATA(ra, i), param))   /*用户的比较函数*/
                {
                    return i;
                }
            }
            else if (ELEM_DATA(ra, i) == param)      /* 默认的完全匹配比较 */
            {
                return i;
            }
        }
    }
    return XERROR;
}


/************************************************************************
* XOS_ArrayElemCompare
* 功能: 使用指定比较函数从数组的第一个位置起
查找符合参数的元素
* 输入 : 
*        raH                      - 数组对象
*        compare                  - 比较函数
*        param                    - 比较函数的参数
* 输出 :  无
* 返回 : 成功返回符合参数的节点位置，否则返回XERROR
************************************************************************/
XPUBLIC  XS32   XOS_ArrayElemCompare( XOS_HARRAY raH,   XOS_ArrayCompare compare,  XVOID *param)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    XS32 i;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayElemCompare , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    for (i=0; i < ra->maxNumOfElements; i++)
    {
        if (!XOS_ArrayElemIsVacant(raH, i)) 
        {
            if (compare)
            {
                if(compare(ELEM_DATA(ra, i), param)) /* 用户的比较函数*/
                {
                    return i;
                }
            }
            else if (ELEM_DATA(ra, i) == param)    /*默认的比较*/
            {
                return i;
            }
        }
    }
    return XERROR;
}


/************************************************************************
* xos_ArrayGetName
* 功能: 获取数组名字
* 输入 : arrayH                    - 数组对象句柄
* 输出 : 无
* 返回 : 返回名字字符串指针
************************************************************************/
XPUBLIC  XCONST XCHAR *XOS_ArrayGetName( XOS_HARRAY arrayH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    
#ifdef XOS_ARRAY_DEBUG    
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetName , param invalid!\n");
        return XNULL;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return (XCONST XCHAR *)(ra->name);
}


/************************************************************************
* XOS_ArrayGetCurElemNum
* 功能: 获取数组当前有效的元素的数量
* 输入 : raH                      - 数组的句柄
* 输出 : 无
* 返回 : 成功返回元素的数量，否则XERROR
************************************************************************/
XPUBLIC  XS32  XOS_ArrayGetCurElemNum(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
#ifdef XOS_ARRAY_DEBUG  
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetCurElemNum , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ra->curNumOfElements;
}


/************************************************************************
*                    获取array 的当前空闲元素数量
************************************************************************/
XPUBLIC XS32  XOS_ArrayGetVacantElemNum(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetVacantElemNum , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->maxNumOfElements - ((t_XOSARRAY *)raH)->curNumOfElements;
}


/************************************************************************
*                    获取array 可存放元素的最大数量
************************************************************************/
XPUBLIC XS32  XOS_ArrayGetMaxElemNum(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetMaxElemNum , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->maxNumOfElements;
}


/************************************************************************
*                        获取array 的最大使用量
************************************************************************/
XPUBLIC XS32   XOS_ArrayGetMaxUsageElemNum(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetMaxUsageElemNum , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->maxUsage;
}


/************************************************************************
*                         获取元素的大小
************************************************************************/
XPUBLIC XS32  XOS_ArrayGetElemSize(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetElemSize , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->sizeofElement;
}


/************************************************************************
*                          获取比较函数
************************************************************************/
XPUBLIC XOS_ArrayCompare  XOS_ArrayGetCompareFunc(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetCompareFunc , param invalid!\n");
        return XNULL;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->compare;
}

/*
Desc:wzy add the follow funs to implement the list 
*/

/************************************************************************
*                            获取第一个节点
************************************************************************/
XPUBLIC XS32 XOS_ArrayGetFirstNode(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
#ifdef XOS_ARRAY_DEBUG 
    if (!XOS_ArrayHandleIsValid(raH)) return XERROR;
#endif /*XOS_ARRAY_DEBUG*/
    
    return ra->firstNodeLocation;
}


/************************************************************************
*                           获取最后一个节点
************************************************************************/
XPUBLIC XS32 XOS_ArrayGetLastNode(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
#ifdef XOS_ARRAY_DEBUG
    if (!XOS_ArrayHandleIsValid(raH)) return XERROR;
#endif /*XOS_ARRAY_DEBUG*/
    
    return ra->lastNodeLocation;
}


/************************************************************************
*                            设置第一个节点
************************************************************************/

XPUBLIC XS32 XOS_ArraySetFirstNode(XOS_HARRAY raH, XS32 location)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
#ifdef XOS_ARRAY_DEBUG
    if (!XOS_ArrayHandleIsValid(raH)) return XERROR;
#endif /*XOS_ARRAY_DEBUG*/
    
    ra->firstNodeLocation = location;
    return location;
}


/************************************************************************
*                         设置最后一个节点
************************************************************************/
XPUBLIC XS32 XOS_ArraySetLastNode(XOS_HARRAY raH, XS32 location)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
    if (!XOS_ArrayHandleIsValid(raH)) return XERROR;
    
    ra->lastNodeLocation = location;
    return location;
}


/************************************************************************
*                            获取第一个节点首地址
************************************************************************/
XPUBLIC XCHAR* XOS_ArrayGetHeadPtr(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    XS32 maxElements;
    
#ifdef XOS_ARRAY_DEBUG
    if (!XOS_ArrayHandleIsValid(raH)) return XNULLP;
#endif /*XOS_ARRAY_DEBUG*/
    
    maxElements = ((t_XOSARRAY*)ra)->maxNumOfElements;
    return (XCHAR*)ra + sizeof(t_XOSARRAY) + BIT_VECTOR_SIZE(maxElements);
}


/************************************************************************
*                        获取最后一个节点的首地址
************************************************************************/
XPUBLIC XCHAR* XOS_ArrayGetTailPtr(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    XS32 maxElements;
    XS32 elemSize;
    
#ifdef XOS_ARRAY_DEBUG  
    if (!XOS_ArrayHandleIsValid(raH)) return XNULLP;
#endif /*XOS_ARRAY_DEBUG*/
    
    maxElements = ((t_XOSARRAY*)ra)->maxNumOfElements;
    elemSize = ra->sizeofElement;
    
    return  (XCHAR*)ra + sizeof(t_XOSARRAY) 
            + BIT_VECTOR_SIZE(maxElements) + (maxElements * XOS_MAX((XU32)4, (XU32)elemSize));
    
}


#ifdef  __cplusplus
}
#endif  /*  __cplusplus */


