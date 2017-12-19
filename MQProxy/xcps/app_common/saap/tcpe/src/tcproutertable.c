/*----------------------------------------------------------------------
    created:
    created:
    filename:
    file path:
    file base:
    file ext:
    author:

    purpose:    ���ļ���Ҫ�洢Ŀ���������뵽IP��·�ŵı��˱��OAM������ʽ
----------------------------------------------------------------------*/
#ifdef  __cplusplus
    extern  "C"{
#endif

#include "xosshell.h"
#include "saap_def.h"
#include "../inc/tcproutertable.h"
#include "../inc/tcpoamproc.h"
#include "syshash.h"

TCP_DSTID_INDEX_IP_CCB    *gTCPDstIDToIPTbl;
HASH_TABLE                 gTCPDstIDToIPHashTbl;

/**********************************
��������    : tcp_SetDstIDtoLintTable
����        :
�������    : 2008��10��9��
��������    : �˺��������������������������һ�������������Ϣ
����        :  XU16  DstId
����        : XU16 LintNo
����ֵ      : XU32
************************************/
XU32 tcp_SetDstIDtoLintTable( XU16  DstId , XU16 LintNo )
{
    // ������һ�����������HASH���������������
    XU16 usNo;
    //�Ϸ��Լ��
    if(( TCP_LINT_MAX <= LintNo )||(LintNo<=0))
    {
        XOS_Trace(MD(FID_TCPE, PL_ERR), "LinkIndex error!");
        return XERROR;
    }

    usNo = TCP_SearchDstIDToIpTable( DstId );
    //������������Ѵ��ڣ����޸�
    if( TCP_ERROR_NUM != usNo )
    {
        TCP_deleteDstIDtoIPTable( DstId );
        XOS_Trace( MD(FID_TCPE, PL_DBG), "The DstID: %d(0x%x) is covered !", DstId, DstId );
    }

    //��dstid��������
    usNo = TCP_InsertDstIDToIpTable( DstId ,  BLANK_USHORT);
    if( TCP_ERROR_NUM == usNo )
    {
        XOS_Trace( MD(FID_TCPE, PL_ERR), "NO ID CCB Full ERR: usNo=%x tcp_SetDstIDAndIpRouterTable()", DstId );
        return XERROR;
    }

    gTCPDstIDToIPTbl[usNo].DstDeviceID  = DstId ;
    gTCPDstIDToIPTbl[usNo].usLintIndex  = LintNo ;

    return XSUCC ;
}

/**********************************
��������    : TCP_deleteDstIDtoIPTable
����        :
�������    : 2008��10��9��
��������    : �˺����������������������ɾ��һ�������������Ϣ
����        : XU16 DstId
����ֵ      : XU32
************************************/
XU32 TCP_deleteDstIDtoIPTable(XU16 DstId )
{

    XU16 usNo;
    XU32 ulTemp,ulHashKey ;

    // ���DSTID���ɾ��
    usNo = TCP_SearchDstIDToIpTable( DstId );
    if( TCP_ERROR_NUM == usNo )
    {
        XOS_Trace( MD(FID_TCPE, PL_ERR), "delete dstId 0x%x from DstID2IpTable failed",DstId);
        return XERROR;
    }

    //ȡ��ֵ
    ulHashKey = gTCPDstIDToIPTbl[usNo].DstDeviceID;
    TCP_InitDstIDToIpCcb(usNo);
    if( XSUCC != SYS_HashNodeDel(&gTCPDstIDToIPHashTbl, ulHashKey, &ulTemp) )
    {
        return TCP_ERROR_NUM;
    }
    if( ulTemp != usNo )
    {
        return TCP_ERROR_NUM;
    }
    usNo = (XU16)ulTemp;

    return XSUCC ;
}

/**********************************
��������    : TCP_init_DstID_table
����        :
�������    : 2008��10��9��
��������    : Ŀ����������ʼ������
����        : void
����ֵ      : XS32
************************************/
XS32 TCP_init_DstID_table(void)
{

    //== ���� DstID to ipLint ��Ͱ����HASH�� ==//
    gTCPDstIDToIPTbl = (TCP_DSTID_INDEX_IP_CCB *)XOS_MemMalloc( FID_TCPE,
            sizeof(TCP_DSTID_INDEX_IP_CCB) * TCP_MAX_IPROUTER_NUM );
    if( XNULL == gTCPDstIDToIPTbl )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe init proc,init DstID table failed.");
        return XERROR;
    }

    //��ʼ��
    XOS_MemSet(gTCPDstIDToIPTbl, BLANK_UCHAR, sizeof(TCP_DSTID_INDEX_IP_CCB) * TCP_MAX_IPROUTER_NUM);

    if( XSUCC != SYS_HashTblCreat( TCP_DSTID_TO_LINT_HTABLE_SIZE,  TCP_IDSTID_TO_LINT_HKEY_MASK,
            TCP_GetDstIDToIPHashIndexFromHashKey, &gTCPDstIDToIPHashTbl) )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe init proc,init DstID table failed.");
        return XERROR;
    }

    return XSUCC;
}

/**********************************
��������    : TCP_GetDstIDToIPHashIndexFromHashKey
����        :
�������    : 2008��10��9��
��������    : hash�˺���,�õ�hash����
����        : XU32 ulHashKey
����        : XU32 *pulHashCntnIndex
����ֵ      : XS32
************************************/
XS32 TCP_GetDstIDToIPHashIndexFromHashKey(XU32 ulHashKey, XU32 *pulHashCntnIndex)
{
    if( XNULL == pulHashCntnIndex )
    {
        return XERROR;
    }

    *pulHashCntnIndex = ulHashKey % TCP_IDSTID_TO_LINT_HKEY_MASK;

    return XSUCC;
}

/**********************************
��������    : TCP_InsertDstIDToIpTable
����        : Jeff.Zeng
�������    : 2008��10��9��
��������    :
����        : XU16 ulHashKey
����        : XU16 ulHashValue
����ֵ      : XU16
************************************/
XU16 TCP_InsertDstIDToIpTable(XU16 ulHashKey, XU16 ulHashValue)
{

    XU16 usCcCbNo = TCP_ERROR_NUM;
    XU32  ulTemp   = BLANK_ULONG;

    if( (0 ==  ulHashKey) || ( BLANK_ULONG ==  ulHashKey) )
    {
        return TCP_ERROR_NUM;
    }

    if( XSUCC != SYS_HashNodeInsert( &gTCPDstIDToIPHashTbl, ulHashKey, ulHashValue, &ulTemp) )
    {
        return TCP_ERROR_NUM;
    }

    if( TCP_MAX_IPROUTER_NUM <= ulTemp )
    {
        return TCP_ERROR_NUM;
    }

    usCcCbNo = (XU16)ulTemp;

    TCP_InitDstIDToIpCcb(usCcCbNo);

    return usCcCbNo;

}

#if 0
XU16 TCP_SearchDstIDToIpTable(XU16 usDestDpId)
{
    XS32 i =0;
    for(i = 0;i < TCP_LINT_MAX)
    {
        if(LINT_CCB_NULL != g_pstTCP->lintCcb[i].ucFlag
           && g_pstTCP->lintCcb[i].DstDeviceID == usDestDpId)
        {
            return i;
        }
    }
    return TCP_ERROR_NUM;
}

#endif

/**********************************
��������    : TCP_SearchDstIDToIpTable
����        :
�������    : 2008��10��9��
��������    :
����        : XU16 ulHashKey
����ֵ      : XU16
************************************/
XU16 TCP_SearchDstIDToIpTable(XU16 ulHashKey)
{
    XU32  ulTemp = BLANK_ULONG;
    XU32 ulHashValue;

    if( 0 ==  ulHashKey || BLANK_ULONG ==  ulHashKey )
    {
        return TCP_ERROR_NUM;
    }

    if( XSUCC != SYS_HashNodeSearch( &gTCPDstIDToIPHashTbl, ulHashKey, &ulHashValue, &ulTemp) )
    {
        return TCP_ERROR_NUM;
    }

    if( TCP_MAX_IPROUTER_NUM <= ulTemp )
    {
        return TCP_ERROR_NUM;
    }

    return (XU16)ulTemp;

}

/**********************************
��������    : TCP_InitDstIDToIpCcb
����        :
�������    : 2008��10��9��
��������    : ��ʼ��������
����        : XU16 usCcbNo
����ֵ      : void
************************************/
void TCP_InitDstIDToIpCcb(XU16 usCcbNo)
{
    if( TCP_MAX_IPROUTER_NUM <= usCcbNo )
    {
        return;
    }
    XOS_MemSet( (XCHAR *)&gTCPDstIDToIPTbl[usCcbNo], 0, sizeof(TCP_DSTID_INDEX_IP_CCB) );

    gTCPDstIDToIPTbl[usCcbNo].DstDeviceID    = BLANK_USHORT;
    gTCPDstIDToIPTbl[usCcbNo].usLintIndex    = BLANK_USHORT;

    return;
}

#ifdef __cplusplus
    }
#endif


