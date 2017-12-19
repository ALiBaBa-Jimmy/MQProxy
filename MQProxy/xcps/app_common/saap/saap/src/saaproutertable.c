/*----------------------------------------------------------------------
saapRouterTable.h - 全局宏与类型定义

版权所有 2004 -2006 信威公司深研所BSC项目组.

author: 张海

修改历史记录
ALl OK
--------------------
添加版权说明.
----------------------------------------------------------------------*/

#ifdef  __cplusplus
    extern  "C"{
#endif
#include "../../hashlst/inc/saap_def.h"
#include "../inc/saaproutertable.h"
#include "../../hashlst/inc/syshash.h"

//由于根据UID,电话号码路由表可能重复,如同一号码既可能路由到HLR,也可能路由到SMC

SAAP_TELL_INDEX_DSTDEVICEID_CCB  *gSaapTelToDstIDCcTbl;
SAAP_TELL_INDEX_DSTDEVICEID_CCB  stSaapTelToDstIDCcTbl[SAAP_ROUTE_TBL_MAX][SAAP_MAX_SAAPCCB_NUM];
HASH_TABLE                       gSAAPTelToDstIDHashTbl[SAAP_ROUTE_TBL_MAX];// tell to dstid HASH表

// UID HASH,DATA TBL
SAAP_HLR_INDEX_DSTDEVICEID_CCB   *gSaapHlrToDstIDCcTbl;
SAAP_HLR_INDEX_DSTDEVICEID_CCB   stSaapHlrToDstIDCcTbl[SAAP_ROUTE_TBL_MAX][SAAP_MAX_SAAPCCB_NUM];
HASH_TABLE                       gSAAPHlrToDstIDHashTbl[SAAP_ROUTE_TBL_MAX];// hlr to dstid HASH表

//EID HASH ,DATA TBL
SAAP_PID_INDEX_DSTDEVICEID_CCB   *gSaapPidToDstIDCcTbl;
HASH_TABLE                        gSAAPPidToDstIDHashTbl;   // eid to dstid HASH表

SAAP_GLOBAL_STRUCT                g_ulSAAPGlobal ;   // 用于标识自已的信令点码

/********************************** 
函数名称	: SAAP_MallocGlobalMemory
作者		: 
设计日期	: 2008年9月24日 CodeReview
功能描述	: SAAP 模块内存与HASH 初始化函数
返回值		: XS32 
************************************/
XS32 SAAP_MallocGlobalMemory()
{
    XS32 i = 0;
    //== 生成tell号段 to DstID 的桶表与HASH表 ==//
    for(i = 0;i< SAAP_ROUTE_TBL_MAX;i++)
    {
        //初始化
        XOS_MemSet(&stSaapTelToDstIDCcTbl[i], BLANK_UCHAR, sizeof(SAAP_TELL_INDEX_DSTDEVICEID_CCB) * SAAP_MAX_SAAPCCB_NUM);
    }
    for(i = 0;i<SAAP_ROUTE_TBL_MAX;i++)
    {
        if( XSUCC != SYS_HashTblCreat(SAAP_HASH_TABLE_SIZE, SAAP_HASH_KEY_MASK,
            SAAP_GetTellToDstIDHashIndexFromHashKey, &gSAAPTelToDstIDHashTbl[i]))
        {
            return XERROR;
        }
    }

    //== 生成HLR号段 to DstID 的桶表与HASH表 ==//
    for(i=0;i<SAAP_ROUTE_TBL_MAX;i++)
    {
        //初始化
        XOS_MemSet(&stSaapHlrToDstIDCcTbl[i], BLANK_UCHAR, sizeof(SAAP_HLR_INDEX_DSTDEVICEID_CCB) * SAAP_MAX_SAAPCCB_NUM);
    }
    for(i=0;i<SAAP_ROUTE_TBL_MAX;i++)
    {
        if( XSUCC != SYS_HashTblCreat(SAAP_HASH_TABLE_SIZE, SAAP_HASH_KEY_MASK,
                SAAP_GetHlrToDstIDHashIndexFromHashKey, &gSAAPHlrToDstIDHashTbl[i]) )
        {
            return XERROR;
        }
    }

    //== 生成PID to DstID 的桶表与HASH表 ==//
    gSaapPidToDstIDCcTbl = (SAAP_PID_INDEX_DSTDEVICEID_CCB *)XOS_MemMalloc( FID_SAAP,
            sizeof(SAAP_PID_INDEX_DSTDEVICEID_CCB) * SAAP_MAX_SAAPCCB_NUM );
    if( XNULL == gSaapPidToDstIDCcTbl )
    {
         return XERROR;
    }
    //初始化
    XOS_MemSet(gSaapPidToDstIDCcTbl, BLANK_UCHAR, sizeof(SAAP_PID_INDEX_DSTDEVICEID_CCB) * SAAP_MAX_SAAPCCB_NUM);

    if( XSUCC != SYS_HashTblCreat(SAAP_HASH_TABLE_SIZE, SAAP_HASH_KEY_MASK,
            SAAP_GetPidToDstIDHashIndexFromHashKey, &gSAAPPidToDstIDHashTbl) )
    {
        return XERROR;
    }

    return XSUCC;
}


/********************************** 
函数名称	: SAAP_GetHlrToDstIDHashIndexFromHashKey
作者		: ..
设计日期	: 2008年9月25日
功能描述	: hash 回调定位算法函数
参数		: XU32 ulHashKey
参数		: XU32 *pulHashCntnIndex
返回值		: XS32 
************************************/
XS32 SAAP_GetHlrToDstIDHashIndexFromHashKey(XU32 ulHashKey, XU32 *pulHashCntnIndex)
{
    if( XNULL == pulHashCntnIndex )
    {
        return XERROR;
    }

    *pulHashCntnIndex = ulHashKey % SAAP_HASH_KEY_MASK;

    return XSUCC;
}

/********************************** 
函数名称	: SAAP_GetTellToDstIDHashIndexFromHashKey
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU32 ulHashKey
参数		: XU32 *pulHashCntnIndex
返回值		: XS32 
************************************/
XS32 SAAP_GetTellToDstIDHashIndexFromHashKey(XU32 ulHashKey, XU32 *pulHashCntnIndex)
{
    if( XNULL == pulHashCntnIndex )
    {
        return XERROR;
    }

    *pulHashCntnIndex = ulHashKey % SAAP_HASH_KEY_MASK;

    return XSUCC;
}

/********************************** 
函数名称	: SAAP_GetPidToDstIDHashIndexFromHashKey
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU32 ulHashKey
参数		: XU32 *pulHashCntnIndex
返回值		: XS32 
************************************/
XS32 SAAP_GetPidToDstIDHashIndexFromHashKey(XU32 ulHashKey, XU32 *pulHashCntnIndex)
{
    if( XNULL == pulHashCntnIndex )
    {
        return XERROR;
    }

    *pulHashCntnIndex = ulHashKey % SAAP_HASH_KEY_MASK;

    return XSUCC;
}


/********************************** 
函数名称	: SAAP_InsertTelToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 数据插入函数
参数		: XU32 ulHashKey
参数		: XU32 ulHashValue
参数		: XU32 ulRouteTblId
返回值		: XU16 
************************************/
XU16 SAAP_InsertTelToDstIDTable(XU32 ulHashKey, XU32 ulHashValue,XU32 ulRouteTblId)
{

    XU16 usCcCbNo = SAAP_ERROR_NUM;
    XU32  ulTemp   = BLANK_ULONG;

    if(SAAP_ROUTE_TBL_MAX <= ulRouteTblId)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"tabid(ulRouteTblId = %d) err.",ulRouteTblId);
        return SAAP_ERROR_NUM;
    }

    if( XSUCC != SYS_HashNodeInsert( &gSAAPTelToDstIDHashTbl[ulRouteTblId], ulHashKey, ulHashValue, &ulTemp) )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_InsertTelToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( SAAP_MAX_SAAPCCB_NUM <= ulTemp )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_InsertTelToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    usCcCbNo = (XU16)ulTemp;

    SAAP_InitTellToDstIDCb(usCcCbNo);

    return usCcCbNo;

}


/********************************** 
函数名称	: SAAP_InsertHlrToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU32 ulHashKey
参数		: XU32 ulHashValue
参数		: XU32 ulRouteTblId
返回值		: XU16 
************************************/
XU16 SAAP_InsertHlrToDstIDTable(XU32 ulHashKey, XU32 ulHashValue,XU32 ulRouteTblId)
{

    XU16 usCcCbNo = SAAP_ERROR_NUM;
    XU32  ulTemp   = BLANK_ULONG;

    if(SAAP_ROUTE_TBL_MAX <= ulRouteTblId)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"tabid(ulRouteTblId =%d) err.",ulRouteTblId);
        return SAAP_ERROR_NUM;
    }

    if( XSUCC != SYS_HashNodeInsert( &gSAAPHlrToDstIDHashTbl[ulRouteTblId], ulHashKey, ulHashValue, &ulTemp) )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_InsertHlrToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( SAAP_MAX_SAAPCCB_NUM <= ulTemp )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_InsertHlrToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    usCcCbNo = (XU16)ulTemp;

    SAAP_InitHlrToDstIDCb(usCcCbNo);

    return usCcCbNo;

}

/********************************** 
函数名称	: SAAP_InsertPidToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU32 ulHashKey
参数		: XU32 ulHashValue
返回值		: XU16 
************************************/
XU16 SAAP_InsertPidToDstIDTable(XU32 ulHashKey, XU32 ulHashValue)
{

    XU16 usCcCbNo = SAAP_ERROR_NUM;
    XU32  ulTemp   = BLANK_ULONG;

    if( XSUCC != SYS_HashNodeInsert( &gSAAPPidToDstIDHashTbl, ulHashKey, ulHashValue, &ulTemp) )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_InsertPidToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( SAAP_MAX_SAAPCCB_NUM <= ulTemp )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_InsertPidToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    usCcCbNo = (XU16)ulTemp;

    SAAP_InitPidToDstIDCb(usCcCbNo);

    return usCcCbNo;

}

// hash 查找函数
/********************************** 
函数名称	: SAAP_SearchTellToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: hash 查找函数
参数		: XU32 ulHashKey
参数		: XU32 ulRouteTblId
返回值		: XU16 
************************************/
XU16 SAAP_SearchTellToDstIDTable(XU32 ulHashKey,XU32 ulRouteTblId)
{
    XU32  ulTemp = BLANK_ULONG;
    XU32 ulHashValue;

    if(SAAP_ROUTE_TBL_MAX <= ulRouteTblId)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"tabid(ulRouteTblId) err.",ulRouteTblId);
        return SAAP_ERROR_NUM;
    }

    if( XSUCC != SYS_HashNodeSearch( &gSAAPTelToDstIDHashTbl[ulRouteTblId], ulHashKey, &ulHashValue, &ulTemp) )
    {
        XOS_Trace(MD(FID_SAAP, PL_DBG), "err! -- SAAP_SearchTellToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( SAAP_MAX_SAAPCCB_NUM <= ulTemp )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_SearchTellToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    return (XU16)ulTemp;

}

/********************************** 
函数名称	: SAAP_SearchHlrToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU32 ulHashKey
参数		: XU32 ulRouteTblId
返回值		: XU16 
************************************/
XU16 SAAP_SearchHlrToDstIDTable(XU32 ulHashKey,XU32 ulRouteTblId)
{
    XU32  ulTemp = BLANK_ULONG;
    XU32 ulHashValue;

    if(SAAP_ROUTE_TBL_MAX <= ulRouteTblId)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"tabid(ulRouteTblId=%d) err.",ulRouteTblId);
        return SAAP_ERROR_NUM;
    }

    if( XSUCC != SYS_HashNodeSearch( &gSAAPHlrToDstIDHashTbl[ulRouteTblId], ulHashKey, &ulHashValue, &ulTemp) )
    {
        XOS_Trace(MD(FID_SAAP, PL_DBG), "err! -- SAAP_SearchHlrToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( SAAP_MAX_SAAPCCB_NUM <= ulTemp )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_SearchHlrToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    return (XU16)ulTemp;

}

/********************************** 
函数名称	: SAAP_SearchPidToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU32 ulHashKey
返回值		: XU16 
************************************/
XU16 SAAP_SearchPidToDstIDTable(XU32 ulHashKey)
{
    XU32  ulTemp = BLANK_ULONG;
    XU32 ulHashValue;

    if( XSUCC != SYS_HashNodeSearch( &gSAAPPidToDstIDHashTbl, ulHashKey, &ulHashValue, &ulTemp) )
    {
        XOS_Trace(MD(FID_SAAP, PL_DBG), "err! -- SAAP_SearchPidToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( SAAP_MAX_SAAPCCB_NUM <= ulTemp )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_SearchPidToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    return (XU16)ulTemp;

}

/********************************** 
函数名称	: SAAP_DelTellToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: hash 删除函式
参数		: XU16 usCcCbNo
返回值		: XU16 
************************************/
XU16 SAAP_DelTellToDstIDTable(XU16 usCcCbNo)
{
    XU16 usCcNo = SAAP_ERROR_NUM;
    XU32  ulTemp = BLANK_ULONG;
    XU32  ulHashKey;

    if( SAAP_MAX_SAAPCCB_NUM <= usCcCbNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelTellToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    ulHashKey = gSaapHlrToDstIDCcTbl[usCcCbNo].hlrNo ;

    SAAP_InitHlrToDstIDCb(usCcCbNo);

    if( XSUCC != SYS_HashNodeDel(&gSAAPHlrToDstIDHashTbl[0], ulHashKey, &ulTemp) )//8888
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelTellToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( ulTemp != usCcCbNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelTellToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    usCcNo = (XU16)ulTemp;

    return usCcNo;
}

/********************************** 
函数名称	: SAAP_DelHlrToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU16 usCcCbNo
返回值		: XU16 
************************************/
XU16 SAAP_DelHlrToDstIDTable(XU16 usCcCbNo)
{
    XU16 usCcNo = SAAP_ERROR_NUM;
    XU32  ulTemp = BLANK_ULONG;
    XU32  ulHashKey;

    if( SAAP_MAX_SAAPCCB_NUM <= usCcCbNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelHlrToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    ulHashKey = gSaapTelToDstIDCcTbl[usCcCbNo].tellNo ;

    SAAP_InitHlrToDstIDCb(usCcCbNo);

    if( XSUCC != SYS_HashNodeDel(&gSAAPTelToDstIDHashTbl[0], ulHashKey, &ulTemp) ) // ??? 0=?
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelHlrToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( ulTemp != usCcCbNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelHlrToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    usCcNo = (XU16)ulTemp;

    return usCcNo;
}

/********************************** 
函数名称	: SAAP_DelPidToDstIDTable
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU16 usCcCbNo
返回值		: XU16 
************************************/
XU16 SAAP_DelPidToDstIDTable(XU16 usCcCbNo)
{
    XU16 usCcNo = SAAP_ERROR_NUM;
    XU32  ulTemp = BLANK_ULONG;
    XU32  ulHashKey;

    if( SAAP_MAX_SAAPCCB_NUM <= usCcCbNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelPidToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    ulHashKey = gSaapPidToDstIDCcTbl[usCcCbNo].pidNo;

    SAAP_InitPidToDstIDCb(usCcCbNo);

    if( XSUCC != SYS_HashNodeDel(&gSAAPPidToDstIDHashTbl, ulHashKey, &ulTemp) )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelPidToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    if( ulTemp != usCcCbNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_DelPidToDstIDTable()");
        return SAAP_ERROR_NUM;
    }

    usCcNo = (XU16)ulTemp;

    return usCcNo;
}

/********************************** 
函数名称	: SAAP_InitTellToDstIDCb
作者		: ..
设计日期	: 2008年9月25日
功能描述	: hash 初始化
参数		: XU16 usCcNo
返回值		: void 
************************************/
void SAAP_InitTellToDstIDCb(XU16 usCcNo)
{
    if( SAAP_MAX_SAAPCCB_NUM <= usCcNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "SAAP_InitTellToDstIDCb() failed,input para is error");
        return;
    }

    XOS_MemSet( (XCHAR *)&gSaapTelToDstIDCcTbl[usCcNo], 0, sizeof(SAAP_TELL_INDEX_DSTDEVICEID_CCB) );

    gSaapTelToDstIDCcTbl[usCcNo].tellNo          = BLANK_USHORT;
    gSaapTelToDstIDCcTbl[usCcNo].DstID      = BLANK_USHORT;

    return;
}

/********************************** 
函数名称	: SAAP_InitHlrToDstIDCb
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU16 usCcNo
返回值		: void 
************************************/
void SAAP_InitHlrToDstIDCb(XU16 usCcNo)
{
    if( SAAP_MAX_SAAPCCB_NUM <= usCcNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_InitHlrToDstIDCb()");
        return;
    }

    XOS_MemSet( (XCHAR *)&gSaapHlrToDstIDCcTbl[usCcNo], 0, sizeof(SAAP_HLR_INDEX_DSTDEVICEID_CCB) );

    gSaapHlrToDstIDCcTbl[usCcNo].hlrNo         = BLANK_USHORT;
    gSaapHlrToDstIDCcTbl[usCcNo].DstID    = BLANK_USHORT;

    return;
}

/********************************** 
函数名称	: SAAP_InitPidToDstIDCb
作者		: ..
设计日期	: 2008年9月25日
功能描述	: 
参数		: XU16 usCcNo
返回值		: void 
************************************/
void SAAP_InitPidToDstIDCb(XU16 usCcNo)
{
    if( SAAP_MAX_SAAPCCB_NUM <= usCcNo )
    {
        XOS_Trace(MD(FID_SAAP, PL_ERR), "err! -- SAAP_InitPidToDstIDCb()");
        return;
    }

    XOS_MemSet( (XCHAR *)&gSaapPidToDstIDCcTbl[usCcNo], 0, sizeof(SAAP_PID_INDEX_DSTDEVICEID_CCB) );

    gSaapPidToDstIDCcTbl[usCcNo].pidNo          = BLANK_ULONG;
    gSaapPidToDstIDCcTbl[usCcNo].DstID     = BLANK_USHORT;

    return;
}

#ifdef __cplusplus
    }
#endif


