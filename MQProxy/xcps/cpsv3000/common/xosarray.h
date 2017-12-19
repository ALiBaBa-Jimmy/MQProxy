/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosarray.h
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
**   author          date              modification            
**   zengjiandong    2006.3.21       create  
**   zengjiandong    2006.3.23       revise
**************************************************************/
#ifndef _xosarray_H_
#define _xosarray_H_
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
#define  ARRAY_MAGIC_VALUE        0x0aaaff77
#define  ARRAY_NAME_SIZE     32/*数组名的大小*/
/*#define  XOS_ARRAY_ERROR     ( -1 )*/

/* 数组下标为i 的元素的地址 */
#define ELEM_DATA(ra, i) \
    ((XCHAR *) (((t_XOSARRAY*)(ra))->arrayLocation + (i)*(((t_XOSARRAY*)(ra))->sizeofElement)))

/*  数组句柄  */
XOS_DECLARE_HANDLE(XOS_HARRAY);

/*四个字节对齐*/
#ifdef _ALPHA_
#define RV_ALIGN(numBytes) (((numBytes + 7) / 8) * 8)
#else
#define RV_ALIGN(numBytes) (((numBytes + 3) / 4) * 4)
#endif

#define BIT_VECTOR(ra)      ((XCHAR *)(ra) + sizeof(t_XOSARRAY) )


/* 取得指定标志位的值，即是否空闲*/
#define getBit(ptr, bit) \
    ((XU32)(((XCHAR *)(ptr))[((XU32)(bit)) >> 3] & (0x80 >> ((XU32)(bit) & 7))))


/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/

/* 数组 元素类型 */
typedef XVOID  *XOS_ArrayElement;

/*比较函数指针*/
typedef XBOOL (*XOS_ArrayCompare)(XOS_ArrayElement element1, XVOID *param);

typedef struct {
    XCHAR            name[ARRAY_NAME_SIZE];   /* 数组的名字*/
    XS32             magicVal;                      /* 用于判断数组是否有效*/
    XCHAR*           arrayLocation;              /* 数组的起始地址*/
    XS32             firstVacantElement;     /* 第一个空闲的位置*/
    XS32             lastVacantElement;      /* 最后一个空闲的位置*/
    XS32             maxNumOfElements;   /* 数组元素的最大数量*/
    XS32             curNumOfElements;     /* 数组元素的当前数量*/
    XS32             sizeofElement;             /* 元素的大小*/
    XS32             maxUsage;                   /* 数组至今的最大使用量*/
    XOS_ArrayCompare compare;       /* 比较函数*/
    
    XS32             firstNodeLocation;       
    XS32             lastNodeLocation;
} t_XOSARRAY;

/*-------------------------------------------------------------------------
API 声明
-------------------------------------------------------------------------*/


/****************************************************************
* XOS_ArrayConstruct
* 功能: 创建一个数组 对象
* 输入: elemSize                        - 元素的大小(字节为单位)
*       maxNumOfElements                - 最大元素数
*       name                            -数组名字
* 输出: 无
* 返回: 成功返回数组对象句柄
*               失败返回XNULL
***************************************************************/
XPUBLIC  XOS_HARRAY XOS_ArrayConstruct(
                                       XS32 elemSize, 
                                       XS32  maxNumOfElements,
                                       XCONST XCHAR*   name  );


/************************************************************************
* XOS_ArrayConstruct
* 功能: 创建一个数组 对象(内存初始化之前可用)
* 输入: elemSize                        - 元素的大小(字节为单位)
*       maxNumOfElements                - 最大元素数
        name                            -数组名字
* 输出: 无
* 返回: 成功返回数组对象句柄
*               失败返回XNULL
************************************************************************/
XPUBLIC  XOS_HARRAY XOS_ArrayMemCst(
                                    XS32 elemSize,  
                                    XS32  maxNumOfElements,  
                                    XCONST XCHAR*   name  );

/****************************************************************
* XOS_ArrayDestruct
* 功能  : 释放数组，释放占用的内存
* 输入  : arrayH               - 要释放的数组对象句柄
* 输出 : 无
* 返回 : 无
***************************************************************/
XPUBLIC  XVOID XOS_ArrayDestruct( XOS_HARRAY arrayH );

/************************************************************************
* XOS_ArrayMemDst
* 功能 : 释放数组，释放占用的内存(内存初始化之前用)
* 输入 : arrayH                      - 要释放的数组对象句柄
* 输出 : 无
* 返回 : 无
************************************************************************/
XPUBLIC  XVOID XOS_ArrayMemDst( XOS_HARRAY arrayH);

/****************************************************************
* XOS_ArrayInitClear
* 功能: 数组创建时调用，初始化一个空的元素表
* 输入: arrayH                       - 数组对象句柄
* 输出: 无
* 返回: 无
***************************************************************/
XPUBLIC  XVOID  XOS_ArrayInitClear( XOS_HARRAY arrayH );


/****************************************************************
* XOS_ArrayAdd
* 功能: 分配一个元素到数组,元素被分配到数组的位置由返回值表明
* 输入:  arrayH                   - 数组对象句柄
*        elem                     - 要分配的元素
*                                   如果该值为XNULL,则元素的值为零
* 输出: 无
* 返回: 成功返回元素在数组中的下标值(非负值)
*              失败返回XERROR
***************************************************************/
XPUBLIC  XS32   XOS_ArrayAdd( XOS_HARRAY arrayH, XOS_ArrayElement elem );


/****************************************************************
* XOS_ArrayAddExt
* 功能: 从数组中申请一个元素的位置
* 输入 : arrayH      - 数组对象的句柄
* 输出 : pOutElem     - 申请到的元素的指针，如果等于XNULL则没有申请到.
* 返回 : 成功返回非负位置值, 失败返回XERROR 
***************************************************************/
XPUBLIC  XS32  XOS_ArrayAddExt(XOS_HARRAY arrayH, XOS_ArrayElement *pOutElem);


/****************************************************************
* XOS_ArrayGetElemByPos 
* 功能: 获取指定位置上的元素
* 输入 : arrayH                - 数组对象句柄
* 输出 : location              - 元素在数组中的位置
* 返回 : 返回要获取的元素,location小于零返回XNULL
***************************************************************/
XPUBLIC  XOS_ArrayElement XOS_ArrayGetElemByPos(XOS_HARRAY arrayH, XS32  location);


/****************************************************************
*XOS_ArrayDeleteByPos
* 功能: 删除指定位置上的元素
* 输入 : arrayH                  - 数组对象句柄
* 输出 : location                - 要删除的元素的位置
* 返回 : 成功返回XSUCC，失败返回XERROR
***************************************************************/
XPUBLIC  XS32   XOS_ArrayDeleteByPos(XOS_HARRAY arrayH,XS32 location);

#if 0
/****************************************************************
* XOS_ArrayElemIsVacant
* 功能: 检查指定位置上的元素是否为空
* 输入  : arrayH              - 数组对象句柄
*         location            - 要检查的位置
* 输出 : 无
* 返回 : 检查结果为空返回XTURE, 不为空返回XFALSE
*               失败返回XERROR
***************************************************************/
XPUBLIC  XBOOL   XOS_ArrayElemIsVacant( XOS_HARRAY arrayH, XS32 location);
#endif
#define XOS_ArrayElemIsVacant(ra, loc)  (((loc)<0 ||(loc)>=((t_XOSARRAY*)(ra))->maxNumOfElements)? XTRUE: (((getBit((BIT_VECTOR(ra)), (loc)))!= 0) ? (XFALSE):(XTRUE)))
/****************************************************************
* XOS_ArrayGetByPtr
* 功能: 根据元素的值获取元素在数组中的位置
* 输入 : arrayH      - 数组对象句柄
* 输出 : ptr         - 元素的值
* 返回 : 成功返回元素的位置值，否则返回XERROR
***************************************************************/
XPUBLIC  XS32  XOS_ArrayGetByPtr(XOS_HARRAY arrayH, XVOID *ptr);


/****************************************************************
* XOS_ArrayGetNextPos
* 功能: 获取当前位置开始的下一个有元素的位置
* 输入 : arrayH               - 数组对象句柄
* 输出 : location             - 开始的位置
* 返回 : 成功返回下一个元素的位置，否则返回XERROR
***************************************************************/
XPUBLIC  XS32  XOS_ArrayGetNextPos( XOS_HARRAY  arrayH, XS32  location);



/****************************************************************
* XOS_ArraySetCompareFunc
* 功能: 设置比较函数给raFind()使用
* 输入  : arrayH           - 数组对象句柄
*         func             - 比较函数
* 输出 : 无
* 返回 : 成功返回XSUCC,失败返回XERROR
***************************************************************/
XPUBLIC  XS32 XOS_ArraySetCompareFunc( XOS_HARRAY arrayH, XOS_ArrayCompare func);


/****************************************************************
* XOS_ArrayFind
* 功能: 根据参数查找数组里符合的元素的位置
* 输入 : arrayH           - 数组对象句柄
* 输出 : param            - 比较函数的参数
* 返回 : 成功返回元素的位置值，否则返回XERROR
***************************************************************/
XPUBLIC  XS32  XOS_ArrayFind(XOS_HARRAY arrayH, XVOID *param);


/****************************************************************
* XOS_ArrayGetFirstPos
* 功能: 从数组下标0开始获取有元素的第一个位置
* 输入 : arrayH         - 数组对象句柄
* 输出 : 无
* 返回 : 成功返回第一个元素的位置值，否则返回XERROR
***************************************************************/
XPUBLIC  XS32  XOS_ArrayGetFirstPos( XOS_HARRAY arrayH );


/****************************************************************
* XOS_ArrayElemCompare
* 功能: 使用指定比较函数从数组的第一个位置起查找符合参数的元素
* 输入  : arrayH             - 数组对象
*         compare            -比较函数
*         param              -比较函数的参数
* 输出 :  无
* 返回 : 成功返回符合参数的节点位置，否则XERROR
***************************************************************/
XPUBLIC  XS32   XOS_ArrayElemCompare(XOS_HARRAY arrayH, XOS_ArrayCompare compare, XVOID *param);


/****************************************************************
* XOS_ArrayGetCompareFunc
* 功能: 获取数组的元素比较函数
* 输入 : arrayH                        - 数组对象句柄
* 输出 : 无
* 返回 : 成功返回比较函数；失败返回XNULL
***************************************************************/
XPUBLIC  XOS_ArrayCompare  XOS_ArrayGetCompareFunc(XOS_HARRAY arrayH);


/****************************************************************
* XOS_ArrayGetCurElemNum
* 功能: 获取数组当前有效的元素的数量
* 输入 : arrayH                      - 数组的句柄
* 输出 :  无
* 返回 : 成功返回元素的数量，否则XERROR 
****************************************************************/
XPUBLIC  XS32  XOS_ArrayGetCurElemNum(XOS_HARRAY arrayH);


/****************************************************************
* XOS_ArrayGetName
* 功能: 获取数组名字
* 输入: arrayH                      - 数组对象句柄
* 输出: 无
* 返回: 返回名字字符串指针
***************************************************************/
XPUBLIC  XCONST XCHAR* XOS_ArrayGetName(XOS_HARRAY arrayH);


/*wzy add the funs to inplement the list */
/************************************************************************
*                               获取array 可存放元素的最大数量
************************************************************************/
XS32  XOS_ArrayGetMaxElemNum(XOS_HARRAY raH);


/************************************************************************
*                                    获取array 的最大使用量
************************************************************************/
XS32   XOS_ArrayGetMaxUsageElemNum(XOS_HARRAY raH);


/************************************************************************
*                                     获取元素的大小
************************************************************************/
XS32  XOS_ArrayGetElemSize(XOS_HARRAY raH);


/************************************************************************
*                                         获取第一个节点
************************************************************************/
XS32 XOS_ArrayGetFirstNode(XOS_HARRAY raH);


/************************************************************************
*                                         获取最后一个节点
************************************************************************/
XS32 XOS_ArrayGetLastNode(XOS_HARRAY raH);


/************************************************************************
*                                         设置第一个节点
************************************************************************/

XS32  XOS_ArraySetFirstNode(XOS_HARRAY raH, XS32 location);


/************************************************************************
*                                        设置最后一个节点
************************************************************************/
XS32 XOS_ArraySetLastNode(XOS_HARRAY raH, XS32 location);


/*一下两个接口给内存管理用*/
/************************************************************************
*                                         获取第一个节点首地址
************************************************************************/

XCHAR* XOS_ArrayGetHeadPtr(XOS_HARRAY raH);

/************************************************************************
*                                       获取最后一个节点的首地址
************************************************************************/
XCHAR* XOS_ArrayGetTailPtr(XOS_HARRAY raH);

XBOOL XOS_ArrayIsUesd(XOS_HARRAY raH, void* ptr);

#define XOS_ArrayHandleIsValid(ra)  ((ra)? ((((t_XOSARRAY*)(ra))->magicVal == ARRAY_MAGIC_VALUE)?XTRUE:XFALSE):XFALSE)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xoshash.h*/

