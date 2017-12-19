/*----------------------------------------------------------------------
saapRouterTable.h - ȫ�ֺ������Ͷ���

��Ȩ���� 2004 -2006 ������˾������BSC��Ŀ��.

author: �ź�

�޸���ʷ��¼
ALl OK
--------------------
��Ӱ�Ȩ˵��.
----------------------------------------------------------------------*/

#ifdef  __cplusplus
    extern  "C"{
#endif
#include "../../hashlst/inc/saap_def.h"
#include "../inc/saaproutertable.h"
#include "../../hashlst/inc/syshash.h"

//���ڸ���UID,�绰����·�ɱ�����ظ�,��ͬһ����ȿ���·�ɵ�HLR,Ҳ����·�ɵ�SMC

SAAP_TELL_INDEX_DSTDEVICEID_CCB  *gSaapTelToDstIDCcTbl;
SAAP_TELL_INDEX_DSTDEVICEID_CCB  stSaapTelToDstIDCcTbl[SAAP_ROUTE_TBL_MAX][SAAP_MAX_SAAPCCB_NUM];
HASH_TABLE                       gSAAPTelToDstIDHashTbl[SAAP_ROUTE_TBL_MAX];// tell to dstid HASH��

// UID HASH,DATA TBL
SAAP_HLR_INDEX_DSTDEVICEID_CCB   *gSaapHlrToDstIDCcTbl;
SAAP_HLR_INDEX_DSTDEVICEID_CCB   stSaapHlrToDstIDCcTbl[SAAP_ROUTE_TBL_MAX][SAAP_MAX_SAAPCCB_NUM];
HASH_TABLE                       gSAAPHlrToDstIDHashTbl[SAAP_ROUTE_TBL_MAX];// hlr to dstid HASH��

//EID HASH ,DATA TBL
SAAP_PID_INDEX_DSTDEVICEID_CCB   *gSaapPidToDstIDCcTbl;
HASH_TABLE                        gSAAPPidToDstIDHashTbl;   // eid to dstid HASH��

SAAP_GLOBAL_STRUCT                g_ulSAAPGlobal ;   // ���ڱ�ʶ���ѵ��������

/********************************** 
��������	: SAAP_MallocGlobalMemory
����		: 
�������	: 2008��9��24�� CodeReview
��������	: SAAP ģ���ڴ���HASH ��ʼ������
����ֵ		: XS32 
************************************/
XS32 SAAP_MallocGlobalMemory()
{
    XS32 i = 0;
    //== ����tell�Ŷ� to DstID ��Ͱ����HASH�� ==//
    for(i = 0;i< SAAP_ROUTE_TBL_MAX;i++)
    {
        //��ʼ��
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

    //== ����HLR�Ŷ� to DstID ��Ͱ����HASH�� ==//
    for(i=0;i<SAAP_ROUTE_TBL_MAX;i++)
    {
        //��ʼ��
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

    //== ����PID to DstID ��Ͱ����HASH�� ==//
    gSaapPidToDstIDCcTbl = (SAAP_PID_INDEX_DSTDEVICEID_CCB *)XOS_MemMalloc( FID_SAAP,
            sizeof(SAAP_PID_INDEX_DSTDEVICEID_CCB) * SAAP_MAX_SAAPCCB_NUM );
    if( XNULL == gSaapPidToDstIDCcTbl )
    {
         return XERROR;
    }
    //��ʼ��
    XOS_MemSet(gSaapPidToDstIDCcTbl, BLANK_UCHAR, sizeof(SAAP_PID_INDEX_DSTDEVICEID_CCB) * SAAP_MAX_SAAPCCB_NUM);

    if( XSUCC != SYS_HashTblCreat(SAAP_HASH_TABLE_SIZE, SAAP_HASH_KEY_MASK,
            SAAP_GetPidToDstIDHashIndexFromHashKey, &gSAAPPidToDstIDHashTbl) )
    {
        return XERROR;
    }

    return XSUCC;
}


/********************************** 
��������	: SAAP_GetHlrToDstIDHashIndexFromHashKey
����		: ..
�������	: 2008��9��25��
��������	: hash �ص���λ�㷨����
����		: XU32 ulHashKey
����		: XU32 *pulHashCntnIndex
����ֵ		: XS32 
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
��������	: SAAP_GetTellToDstIDHashIndexFromHashKey
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 ulHashKey
����		: XU32 *pulHashCntnIndex
����ֵ		: XS32 
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
��������	: SAAP_GetPidToDstIDHashIndexFromHashKey
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 ulHashKey
����		: XU32 *pulHashCntnIndex
����ֵ		: XS32 
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
��������	: SAAP_InsertTelToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: ���ݲ��뺯��
����		: XU32 ulHashKey
����		: XU32 ulHashValue
����		: XU32 ulRouteTblId
����ֵ		: XU16 
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
��������	: SAAP_InsertHlrToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 ulHashKey
����		: XU32 ulHashValue
����		: XU32 ulRouteTblId
����ֵ		: XU16 
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
��������	: SAAP_InsertPidToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 ulHashKey
����		: XU32 ulHashValue
����ֵ		: XU16 
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

// hash ���Һ���
/********************************** 
��������	: SAAP_SearchTellToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: hash ���Һ���
����		: XU32 ulHashKey
����		: XU32 ulRouteTblId
����ֵ		: XU16 
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
��������	: SAAP_SearchHlrToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 ulHashKey
����		: XU32 ulRouteTblId
����ֵ		: XU16 
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
��������	: SAAP_SearchPidToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 ulHashKey
����ֵ		: XU16 
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
��������	: SAAP_DelTellToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: hash ɾ����ʽ
����		: XU16 usCcCbNo
����ֵ		: XU16 
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
��������	: SAAP_DelHlrToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU16 usCcCbNo
����ֵ		: XU16 
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
��������	: SAAP_DelPidToDstIDTable
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU16 usCcCbNo
����ֵ		: XU16 
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
��������	: SAAP_InitTellToDstIDCb
����		: ..
�������	: 2008��9��25��
��������	: hash ��ʼ��
����		: XU16 usCcNo
����ֵ		: void 
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
��������	: SAAP_InitHlrToDstIDCb
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU16 usCcNo
����ֵ		: void 
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
��������	: SAAP_InitPidToDstIDCb
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU16 usCcNo
����ֵ		: void 
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


