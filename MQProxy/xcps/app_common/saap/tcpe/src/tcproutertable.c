/*----------------------------------------------------------------------
    created:
    created:
    filename:
    file path:
    file base:
    file ext:
    author:

    purpose:    本文件主要存储目的信令点编码到IP链路号的表及此表的OAM操作函式
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
函数名称    : tcp_SetDstIDtoLintTable
作者        :
设计日期    : 2008年10月9日
功能描述    : 此函数用于向信令点索引表中增加一个信令点配置信息
参数        :  XU16  DstId
参数        : XU16 LintNo
返回值      : XU32
************************************/
XU32 tcp_SetDstIDtoLintTable( XU16  DstId , XU16 LintNo )
{
    // 本函数一次完成向两个HASH表里面的数据增加
    XU16 usNo;
    //合法性检查
    if(( TCP_LINT_MAX <= LintNo )||(LintNo<=0))
    {
        XOS_Trace(MD(FID_TCPE, PL_ERR), "LinkIndex error!");
        return XERROR;
    }

    usNo = TCP_SearchDstIDToIpTable( DstId );
    //如果索引表中已存在，就修改
    if( TCP_ERROR_NUM != usNo )
    {
        TCP_deleteDstIDtoIPTable( DstId );
        XOS_Trace( MD(FID_TCPE, PL_DBG), "The DstID: %d(0x%x) is covered !", DstId, DstId );
    }

    //置dstid表中数据
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
函数名称    : TCP_deleteDstIDtoIPTable
作者        :
设计日期    : 2008年10月9日
功能描述    : 此函数用于在信令点索引表中删除一个信令点配置信息
参数        : XU16 DstId
返回值      : XU32
************************************/
XU32 TCP_deleteDstIDtoIPTable(XU16 DstId )
{

    XU16 usNo;
    XU32 ulTemp,ulHashKey ;

    // 完成DSTID表的删除
    usNo = TCP_SearchDstIDToIpTable( DstId );
    if( TCP_ERROR_NUM == usNo )
    {
        XOS_Trace( MD(FID_TCPE, PL_ERR), "delete dstId 0x%x from DstID2IpTable failed",DstId);
        return XERROR;
    }

    //取键值
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
函数名称    : TCP_init_DstID_table
作者        :
设计日期    : 2008年10月9日
功能描述    : 目的信令点与初始化函数
参数        : void
返回值      : XS32
************************************/
XS32 TCP_init_DstID_table(void)
{

    //== 生成 DstID to ipLint 的桶表与HASH表 ==//
    gTCPDstIDToIPTbl = (TCP_DSTID_INDEX_IP_CCB *)XOS_MemMalloc( FID_TCPE,
            sizeof(TCP_DSTID_INDEX_IP_CCB) * TCP_MAX_IPROUTER_NUM );
    if( XNULL == gTCPDstIDToIPTbl )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe init proc,init DstID table failed.");
        return XERROR;
    }

    //初始化
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
函数名称    : TCP_GetDstIDToIPHashIndexFromHashKey
作者        :
设计日期    : 2008年10月9日
功能描述    : hash了函数,得到hash索引
参数        : XU32 ulHashKey
参数        : XU32 *pulHashCntnIndex
返回值      : XS32
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
函数名称    : TCP_InsertDstIDToIpTable
作者        : Jeff.Zeng
设计日期    : 2008年10月9日
功能描述    :
参数        : XU16 ulHashKey
参数        : XU16 ulHashValue
返回值      : XU16
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
函数名称    : TCP_SearchDstIDToIpTable
作者        :
设计日期    : 2008年10月9日
功能描述    :
参数        : XU16 ulHashKey
返回值      : XU16
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
函数名称    : TCP_InitDstIDToIpCcb
作者        :
设计日期    : 2008年10月9日
功能描述    : 初始化索引表
参数        : XU16 usCcbNo
返回值      : void
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


