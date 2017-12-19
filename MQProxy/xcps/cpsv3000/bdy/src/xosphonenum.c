/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosphonenum.c
**
**  description: 号码分析树的源文件
**
**  author: wentao
**
**  date:   2006.11.13
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "xosphonenum.h"
#include "xoscfg.h"
#include "xosmem.h"
#include "xosarray.h"

/*-------------------------------------------------------------------------
                模块内部宏定义
-------------------------------------------------------------------------*/

#define CHAR_NUM        12   /*号码中字符的个数*/
#define MAX_INDEX_NUM    8192 /*索引表的大小，索引的最大个数，这里是8K*/



/*判断字符ch是否是合法的号码字符*/
#define CHARACTER_ISVALID(ch) \
    ((('#'==(ch)) || ('*'==(ch)) || (((ch)>='0')&&((ch)<='9'))) ? XTRUE : XFALSE)

/*判断字符ch是否是合法的号码字符*/
#define PHONENUM_ISVALID(pCh, info, i, actualLen) \
{\
    pCh = info;\
    for (i = 0; i<actualLen; i++)\
    {\
        if (XFALSE == CHARACTER_ISVALID(*pCh))\
        {\
            return XERROR;\
        }\
        pCh++;\
    }\
}

/*释放内存*/
#define XX_FREE(pt)\
{\
    XOS_MemFree(FID_NUMTREE, (pt));\
    (pt) = XNULL;\
}

/*获取指针的偏移量*/
#define POINTER_OFFSET(ch) \
    (('*' == (ch)) ? (CHAR_NUM-2) : (('#' == (ch)) ? (CHAR_NUM-1) : ((ch) - '0')))

/*-------------------------------------------------------------------------
                模块内部数据结构
-------------------------------------------------------------------------*/
/*分析树的数据结构*/
typedef struct Tree_Node t_PHONENUM;
struct Tree_Node
{
    XU8 info;
    t_CHARNET *property; /*填入你想要的值*/
    t_PHONENUM* pChild[CHAR_NUM];   /*指向子节点的指针*/
};

/*索引表的数据结构*/
typedef struct
{
    XU32    index;     /*索引值*/
    t_PHONENUM  *address;   /*结点地址*/
    t_CHARNET *property; /*属性地址*/
}t_INDEX;

/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/
/*定义的全局树*/
XSTATIC  t_PHONENUM *sys_pRoot = XNULL;

/*索引表，在创建树是创建*/
XSTATIC XOS_HARRAY HIndexTable = XNULL;

/*在按位分析号码时使用*/
XSTATIC XS8 convenient[CHAR_NUM+1] = "1234567890*#";

/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/
/************************************************************
  函数功能：创建并初始化一个号码分析树中的节点
  参数    ：pTree 指向节点的指针
			ch    节点中存放的字符
  返回值  ：成功返回    节点地址
            失败返回：	XNULL
************************************************************/
XSTATIC t_PHONENUM* XOS_nodeConstruct(XU8 ch)
{
    t_PHONENUM* pTree;
    XS32 i;

    if (XNULL == (pTree = (t_PHONENUM*)XOS_MemMalloc(FID_NUMTREE,sizeof(t_PHONENUM))))/*创建结点空间*/
    {
        return XNULL;
    }

    /* 初始化结点 */
    pTree->info = ch;
    pTree->property = XNULL;
    for (i=0; i<CHAR_NUM; i++)
    {
        pTree->pChild[i] = XNULL;
    }

    return pTree;
}

/************************************************************
  函数功能：判断节点是否为空
  参数    ： pNode       当前节点
  返回值  ：为空返回 XTRUE；否则返回 XFALSE
************************************************************/
XSTATIC XBOOL XOS_nodeIsNULL(t_PHONENUM* pNode)
{
    XS32 i;
    
    if (XNULL == pNode || XNULL != pNode->property)
    {
        return XFALSE;
    }

    for (i=0; i<CHAR_NUM; i++)
    {
        if (XNULL != pNode->pChild[i])
            return XFALSE;
    }

    return XTRUE;
}

/************************************************************************
 * checkIndexElem
 * 功能: 在删除节点时保持索引表的同步
 * 输入  : arrayH     - 数组对象句柄
 * 输出 : pNode      - 要删除的结点的地址
 * 返回 : 成功返回XSUCC，否则返回XERROR(-1)
 *************************************************************************/
XSTATIC XS32  XOS_checkIndexElem(t_PHONENUM *pNode)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)HIndexTable;
    t_INDEX *pIndex = XNULL;
    XS32 i, count;

    if ( XTRUE  != XOS_ArrayHandleIsValid(HIndexTable) || XNULL == pNode)
    {
        return XERROR;  /*SYS_ARRAY_ERROR*/
    }

    count = 0;
    for (i=0; i<ra->maxNumOfElements; i++)
    {
        if (!XOS_ArrayElemIsVacant(HIndexTable, i)) 
        {
            count++;
            pIndex = (t_INDEX*)XOS_ArrayGetElemByPos(HIndexTable, i);
            if (XNULL != pIndex && pIndex->address == pNode)
            {
                pIndex->address = XNULL;
            }
        }
        if (count == ra->curNumOfElements)
        {
            break;
        }
    }
    return XSUCC;
}

/************************************************************
  函数功能：递归删除树中的所有节点和属性
  参数    ： pNode       号码分析树中的节点
  返回值  ：成功返回 SUCC；否则返回 XERROR
************************************************************/
XSTATIC XS32 XOS_nodeDestruct (t_PHONENUM* pNode)
{
    XS32 i;
    
    if (XNULL == pNode)/*参数检验*/
    {
        return XSUCC;
    }

    for (i=0; i<CHAR_NUM; i++)
    {
        XOS_nodeDestruct (pNode->pChild[i]);
        pNode->pChild[i] = XNULL;
    }

    /*若属性不为空，则释放属性空间，删除属性*/
    if (XNULL != pNode->property)
    {
        XX_FREE(pNode->property);
    }

    /*释放节点空间，删除节点*/
    XX_FREE(pNode);

    return XSUCC;
}

/*-------------------------------------------------------------------------
                模块接口函数
-------------------------------------------------------------------------*/
/************************************************************
  函数功能：添加号码（树不存在就先创建树）
  参数    ：info	-	电话号码字符串(以'\0'结束的字符串，长度为0~ MAXPHONELEN)
            	    电话号码，举例："13800138000"
	        len	-	电话号码的长度，如"13800138000"的长度为 11
                    len 取值范围：0 ~ MAXPHONELEN
	        property	-	号码信息（必须为初始化了的结构指针，不应为XNULL）

  返回值  ：成功返回    XSUCC
            失败返回：	1. 一般失败XERROR(-1)(例如：参数有误,内存分配失败)
                        2.号码已经存在NUM_EXIST(2)

************************************************************/
XEXTERN XS32 XOS_PhoneAddInfo(XU8 *info, XU32 len, t_CHARNET *property)
{
    XS32 i, actualLen;/* 实际添加的号码长度 */
    t_PHONENUM *pNode = XNULL, *pTemp = XNULL;
    XU8 *pCh = XNULL;

    /* 参数验证 */
    if ((XNULL == info)||(0 == len)||(XNULL == property)||(MAXPHONELEN < (actualLen = (XU32)XOS_MIN(XOS_StrLen(info), len))))
    {
        return XERROR;
    }

    PHONENUM_ISVALID(pCh, info, i, actualLen);/* 判断号码是否正确 */

    if (XNULL == sys_pRoot)/* 号码树还未创建 */
    {
        if (XNULL == (sys_pRoot = XOS_nodeConstruct('T')))/* 创建并初始化一个根节点 */
        {
            return XERROR;
        }
        if (XNULL == (HIndexTable = XOS_ArrayConstruct( sizeof(t_INDEX), MAX_INDEX_NUM, "Index_Table")))/*创建索引表*/
        {
            XX_FREE(sys_pRoot);
            return XERROR;
        }
    }

    if( 0 >= actualLen)
    {
        return XERROR;  /**/
    }
    
    pNode = sys_pRoot;
    pCh = info;
    for (i = 0; i<actualLen; i++)
    {
        if (XNULL == (pTemp = pNode->pChild[POINTER_OFFSET(*pCh)]))/* 寻找下一层节点 */
        {
            if (XNULL == (pTemp = XOS_nodeConstruct(*pCh)))/*下层节点为NULL则创建新节点*/
            {
                return XERROR;
            }
            pNode->pChild[POINTER_OFFSET(*pCh)] = pTemp;
        }
        pNode = pTemp;
        pCh++;
    }

    /*若没有元素则赋值，否则返回NUM_EXIST*/
    if (XNULL != pNode->property)
    {
        return NUM_EXIST;
    }

    if(XNULL == (pNode->property = (t_CHARNET *)XOS_MemMalloc(FID_NUMTREE,sizeof(t_CHARNET))))
    {
        return XERROR;
    }
    XOS_MemCpy(pNode->property, property, sizeof(t_CHARNET));

    return XSUCC;
}

/************************************************************
  函数功能：删除电话号码
  参数    ：info	-	电话号码字符串(以'\0'结束的字符串，长度为0~ MAXPHONELEN)
            	        电话号码，举例："13800138000"
	        len	-	电话号码的长度，如"13800138000"的长度为 11
            	        len 取值范围：0 ~ MAXPHONELEN

  返回值  ：成功返回	XSUCC
		    失败返回：	1. 一般失败XERROR(-1)
		                3.号码不存在NUM_NON(3)

************************************************************/
XEXTERN XS32 XOS_PhoneDelInfo(XU8 *info, XU32 len)
{
    XU8 *pCh = XNULL;
    XS32 i, actualLen;
    t_PHONENUM  *pNode = XNULL, *pTemp = XNULL;
    t_PHONENUM* pNodesList[MAXPHONELEN+1];/*用于保存整条删除路线上的节点地址*/

    if (XNULL == sys_pRoot)/* 号码树还未创建 */
    {
        return NUM_NON;
    }

    /* 参数验证 */
    if ((XNULL == info) || (0 == len)||(MAXPHONELEN < (actualLen = (XU32)XOS_MIN(XOS_StrLen(info), len))))
    {
        return XERROR;
    }

    PHONENUM_ISVALID(pCh, info, i, actualLen);/* 判断号码是否正确 */

    if( 0 >= actualLen)
    {
        return XERROR;  /**/
    }
    
    for (i=0; i<=MAXPHONELEN; i++)/*初始化指针为NULL*/
    {
        pNodesList[i] = XNULL;
    }
    pNodesList[0] = sys_pRoot;

    pNode = sys_pRoot;
    pCh = info;
    for (i = 1; i<=actualLen; i++)
    {
        if (XNULL == (pTemp = pNode->pChild[POINTER_OFFSET(*pCh)]))/* 寻找下一层节点 */
        {
            return NUM_NON;
        }
        pNode = pTemp;
        pNodesList[i] = pTemp;
        pCh++;
    }

    if (XNULL == pNode->property)/*判断指定位置的属性值是否为空*/
    {
        return NUM_NON;
    }

    XX_FREE(pNode->property);

    for (i = actualLen; i>0; i--)/*向上回溯，将可以删除的节点删除*/
    {
        if (XNULL != pNodesList[i])
        {
            if (XFALSE == XOS_nodeIsNULL(pNodesList[i]))
            {
                return XSUCC;
            }
            if(XERROR == XOS_checkIndexElem(pNodesList[i])) /* */
            {
                return XERROR;
            }
            pNodesList[i-1]->pChild[POINTER_OFFSET(pNodesList[i]->info)] = XNULL;
            XX_FREE(pNodesList[i]);
        }
    }

    return XSUCC;
}

/************************************************************
  函数功能：删除所有拨号计划
  参数    ：N/A
  返回值  ：成功返回    XSUCC
            失败返回：	XERROR
************************************************************/
XEXTERN XS32 XOS_DelAllPNum( XVOID )
{
    XS32 i, count;
    t_INDEX *pIndex;
    t_XOSARRAY *ra = (t_XOSARRAY *)HIndexTable;
    
    if (XNULL == sys_pRoot)
    {
        return XSUCC;
    }

    for (i=0; i<CHAR_NUM; i++)
    {
        XOS_nodeDestruct (sys_pRoot->pChild[i]);/*递归删除节点函数*/
        sys_pRoot->pChild[i] = XNULL;
    }
/*
    if (XNULL != sys_pRoot->property)
    {
        XX_FREE(sys_pRoot->property);
    }
*/    
    count = 0;
    for (i=0; i<ra->maxNumOfElements; i++)
    {
        if (!XOS_ArrayElemIsVacant(HIndexTable, i)) 
        {
            count++;
            pIndex = (t_INDEX*)XOS_ArrayGetElemByPos(HIndexTable, i);
            if (XNULL != pIndex)
            {
                pIndex->address = XNULL;
            }
        }
        if (count == ra->curNumOfElements)
        {
            break;
        }
    }
    return XSUCC;
}

/************************************************************
  函数功能：搜索号码
  参数    ：info	-	电话号码字符串(以'\0'结束的字符串，长度为0~ MAXPHONELEN)
            	        电话号码，举例："13800138000"
	        len	-	电话号码的长度，如" 13800138000"的长度为 11
            	        len 取值范围：0 ~ MAXPHONELEN
            property	-	返回查找到的号码信息

  返回值  ：成功返回    XSUCC
	        失败返回 ：	XERROR
************************************************************/
XEXTERN XS32 XOS_PhoneSearchInfo(XU8 *info, XU32 len, t_CHARNET *property)
{
    XS32 i, actualLen;/* 实际添加的号码长度 */
    t_PHONENUM* pNode = XNULL, *pTemp = XNULL;
    XU8 *pCh = XNULL;

    /* 参数验证 */
    if ((XNULL == info) || (0 == len) || (XNULL == property) || (XNULL == sys_pRoot)||(MAXPHONELEN < (actualLen = (XU32)XOS_MIN(XOS_StrLen(info), len))))
    {
        return XERROR;
    }

    PHONENUM_ISVALID(pCh, info, i, actualLen);/* 判断号码是否正确 */
/*
    if( 0 >= actualLen)
    {
        return XERROR;  
    }
*/    
    pNode = sys_pRoot;
    pCh = info;
    for (i = 0; i<actualLen; i++)
    {
        if (XNULL == (pTemp = pNode->pChild[POINTER_OFFSET(*pCh)]))/* 寻找下一层节点 */
        {
            return XERROR;
        }
        pNode = pTemp;
        pCh++;
    }

    if (XNULL == pNode->property)/*判断指定位置的属性值是否为空*/
    {
        return XERROR;
    }
    XOS_MemCpy(property, pNode->property, sizeof(t_CHARNET));
    return XSUCC;
}

/************************************************************
  函数功能：分析号码串(整串号码)
  参数    ：info	-	电话号码字符串(以'\0'结束的字符串，长度为0~ MAXPHONELEN)
            	        电话号码，举例："13800138000"
	        len	-	电话号码的长度，如" 13800138000"的长度为 11
            	        len 取值范围：0 ~ MAXPHONELEN
            property	-	返回分析出的号码信息
            
  返回值  ：成功：XSUCC （返回号码属性（查找成功时有效））
            失败：XERROR
************************************************************/
XEXTERN XS32 XOS_AnaStrInfo(XU8 *info, XU32 len, t_CHARNET *property)
{
    XU32 i, actualLen, flag = 0;/* actualLen是实际分析的号码长度 */
    t_PHONENUM* pNode = XNULL, *pTemp = XNULL;
    XU8 *pCh = XNULL;

    /* 参数验证 */
    if ((XNULL == info) || (0 == len) || (XNULL == property) || (XNULL == sys_pRoot)||(MAXPHONELEN < (actualLen = (XU32)XOS_MIN(XOS_StrLen(info), len))))
    {
        return XERROR;
    }

    PHONENUM_ISVALID(pCh, info, i, actualLen);/* 判断号码是否正确 */

    pNode = sys_pRoot;
    pCh = info;
    for (i = 0; i<actualLen; i++)
    {
        if (XNULL == (pTemp = pNode->pChild[POINTER_OFFSET(*pCh)]))/* 寻找下一层节点 */
        {
            return((0 == flag) ? XERROR : XSUCC);
        }
        pNode = pTemp;
        if (XNULL != pNode->property)
        {
            XOS_MemCpy(property, pNode->property, sizeof(t_CHARNET));
            flag = 1;/*置标志为1，说明匹配成功*/
        }
        pCh++;
    }

    return((1 == flag) ? XSUCC : XERROR);
}

/************************************************************
  函数功能：按位分析号码，将查到的号码信息填入用户传入的pWord_p并返回相应的index信息；
  参数    ：bcd  -   为号码的1 位（bcd码，f1-f9 ,fa,fb,fc依次对应1－9，'0'，'*','#'）
            index    -     为分析树索引（由平台按照需要定义）
            pWord_p  -   返回的结构体property中的pWord（结构体定义见后面的系统数据结构）
            pLen	-	为返回属性的结构长度(号码全长，包括字冠)

  返回值  ：查找结果（0成功 -1 失败 2 继续搜号）
            号码属性及属性(property结构体中的pWord)的全长度（查找成功时有效）
            分析树索引（继续查找时有效）
            
  注意：关于分析树索引index，
        当分析的是第一位号码时应用需要将它的值初始化为INITANA（即0xffffffff，
        平台会依据此值来判断是否是新来的需要分析的号码）再将其地址传入做参数；
        若是查找到相应的号码就将其信息填入索引表并将索引返回，
        之后用户需要继续分析时就可以将上次返回的索引做参数了。
************************************************************/
XEXTERN XS32 XOS_AnaSchInfo(XU32 bcd, XU8 *pWord_p, XU32 *pLen, XU32 *index)
{
    XU32 i;
    t_PHONENUM  *pNode = XNULL, *pTemp = XNULL;
    t_INDEX *pIndex = XNULL, *pIndexElem = XNULL;
    XU8 ch;

    /* 参数检查 */
    if ((XNULL == pWord_p)||(XNULL == pLen)||(XNULL == index)||(XNULL == sys_pRoot)||(XFALSE == XOS_ArrayHandleIsValid(HIndexTable) ))
    {
        return XERROR;
    }

    /*将BCD码转换成字符*/
    i = bcd & 0xf;
    if (i>0 && i<=CHAR_NUM)
    {
        (ch = convenient[i-1]);
    }
    else
    {
        return XERROR;
    }


    /*由输入的*index确定是刚开始分析第一位号码，还是继续分析号码
      -----对索引值和内容进行判断*/
    if (*index<MAX_INDEX_NUM)
    {
        if (XNULL == (pIndex = (t_INDEX*)XOS_ArrayGetElemByPos(HIndexTable, *index)))
        {
            return XERROR;
        }
        pNode = pIndex->address;    /*** ***/
        if (XNULL == pNode)
        {
            if(XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引项*/
            {
                return XERROR;
            }
            return XERROR;
        }
    }
    else if (INITANA == *index)
    {
        pNode = sys_pRoot;
    }
    else
    {
        return XERROR;
    }


    /*-----------------------
      以结点pNode为入口分析号码*/
    pTemp = pNode->pChild[POINTER_OFFSET(ch)]; /*------ pTemp是当前索引中节点的子节点*/
    

    /*------子节点为NULL*/
    if (XNULL == pTemp)/*子节点为NULL*/
    {
        if (pNode == sys_pRoot)/*刚开始分析第一位号码*/
        {
            return XERROR;
        }
        if (XNULL == pIndex->property)/*不是第一位号码，但索引表保存的property为NULL*/
        {
            if(XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引*/
            {
                return XERROR;
            }    
            return XERROR;
        }

        /*不是第一位号码，且索引表保存的property不为NULL*/
        *pLen = ((t_CHARNET*)(pIndex->property))->len;
        *pWord_p = ((t_CHARNET*)(pIndex->property))->pWord;
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引*/
        {
            return XERROR;
        }    
        return XSUCC;
    }


    /*以下是分析得到的------子节点pTemp不为NULL的情况*/
    if (pNode == sys_pRoot)/*刚开始分析第一位号码*/
    {
    
        if (XNULL != pTemp->property && MEET_NUM == pTemp->property->pWord)
        {
            *pLen = pTemp->property->len;
            *pWord_p = MEET_NUM;
            *index = INITANA;/*查找到接入号，重新查找*/
            return NUM_EXIST;
        }
        
        /*找一个空表项保存这次的信息*/
        if ((*index = XOS_ArrayAddExt(HIndexTable, (XVOID**)&pIndexElem)) >= MAX_INDEX_NUM)
        {
            return XERROR;
        }
        
        if (pIndexElem)
        {
            pIndexElem->index = *index;
            pIndexElem->address = pTemp;
            pIndexElem->property = XNULL;
            if (XNULL != pTemp->property) /*****/
            {
                pIndexElem->property = pTemp->property;
            }
        }
        
        for (i=0; i<CHAR_NUM; i++)
        {
            if (XNULL != pTemp->pChild[POINTER_OFFSET(convenient[i])])/*pTemp有子节点*/
            {
                return NUM_EXIST;
            }
        }
        
        if (XNULL == pTemp->property)/*pTemp的属性为NULL，且无子节点; ###多线程时才可能执行到*/
        {
            if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引*/
            {
                return XERROR;
            }    
            return XERROR;
        }
        
        *pLen = pTemp->property->len;/*属性不为NULL，且无子节点*/
        *pWord_p = pTemp->property->pWord;
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引*/
        {
            return XERROR;
        }    
        return XSUCC;
    }


    /*以下是pTemp不为NULL,且------pNode不是根节点的情况*/
    if (XNULL == pTemp->property)/*不是第一位号码，但子节点的 property为NULL NULL*/
    {/*只修改索引表中的结点地址，不修改index和property*/

        pIndex->address = pTemp;
        
        for (i=0; i<CHAR_NUM; i++)
        {
            if (XNULL != pTemp->pChild[POINTER_OFFSET(convenient[i])])/*pTemp有子节点*/
            {
                return NUM_EXIST;
            }
        }

        /*同上###，只有一个用户时不会发生以下情况，多线程时才可能发生*/
        if (XNULL == pIndex->property)/*索引表保存的属性为NULL，且pTemp无子节点*/
        {
            if(XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引*/
            {
                return XERROR;
            }    
            return XERROR;
        }
        
        *pLen = pIndex->property->len;/*索引表保存的属性不为NULL，且无子节点*/
        *pWord_p = pIndex->property->pWord;
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引*/
        {
            return XERROR;
        }    
        return XSUCC;
    }


/*以下处理pNode不是根节点，pTemp不为NULL且其property不为NULL的情况*/
    pIndex->address = pTemp;
    pIndex->property = pTemp->property;
    if (MEET_NUM == pTemp->property->pWord)/*查找到接入号，重新查找*/
    {
        *pLen = pTemp->property->len;
        *pWord_p = MEET_NUM;
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引*/
        {
            return XERROR;
        }    
        *index = INITANA;
        return NUM_EXIST;
    }

    
    for (i=0; i<CHAR_NUM; i++)
    {
        if (XNULL != pTemp->pChild[POINTER_OFFSET(convenient[i])])/*满足继续接号查找条件*/
        {
            return NUM_EXIST;
        }
    }

    
    *pLen = pTemp->property->len;
    *pWord_p = pTemp->property->pWord;
    if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*释放索引*/
    {
        return XERROR;
    }    
    return XSUCC;
}

/************************************************************
  函数功能：Index的空间释放
  参数    ：indexnum	-	为准备释放的index号，范围：0 ~ MAX_INDEX_NUM
  
  返回值  ：成功返回   XSUCC
	        失败返回   XERROR
***********************************************************/
XEXTERN XS32 XOS_IndexFree(XU32 indexnum)
{
    if((indexnum>=0)&&(indexnum < MAX_INDEX_NUM)&&(XTRUE == XOS_ArrayHandleIsValid(HIndexTable)))
    {
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, indexnum)) /*释放索引*/
        {
            return XERROR;
        }    
        return XSUCC;
    }

    return XERROR;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

