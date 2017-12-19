/*----------------------------------------------------------------------
    created:    2004/09/29
    created:    29:9:2004   9:27
    filename:   E:\work\VcCallsim\src\Callsim\cssOam.c
    file path:  E:\work\VcCallsim\src\Callsim
    file base:  cssOam
    file ext:   c
    author:

    purpose:
----------------------------------------------------------------------*/
#ifdef  __cplusplus
    extern  "C"{
#endif

#include "../../hashlst/inc/saap_def.h"
#include "../inc/saappublichead.h"
#include "../inc/saapmain.h"
#include "../inc/saapoamproc.h"
#include "../inc/saaproutertable.h"
#include "../../hashlst/inc/syshash.h"
#include "oam_interface.h"

extern unsigned short app_register(unsigned int moduleid, unsigned int *ptableid, int count);
extern unsigned char  get_resource(unsigned int tbid, unsigned int size, unsigned int row_count);
XU8 SaapOamCallBack(XU32  uiTableId, XU16 usMsgId, XU32 uiSequence,XU8 ucPackEnd, tb_record *ptRow);

// �ⲿʵ������
extern SAAP_TELL_INDEX_DSTDEVICEID_CCB  *gSaapTelToDstIDCcTbl;
extern HASH_TABLE                        gSAAPTelToDstIDHashTbl[SAAP_ROUTE_TBL_MAX]; //gSAAPTelToDstIDHashTbl;   // tell to dstid HASH��
extern SAAP_HLR_INDEX_DSTDEVICEID_CCB   *gSaapHlrToDstIDCcTbl;
extern HASH_TABLE                        gSAAPHlrToDstIDHashTbl[SAAP_ROUTE_TBL_MAX]; //gSAAPHlrToDstIDHashTbl;   // hlr to dstid HASH��
extern SAAP_PID_INDEX_DSTDEVICEID_CCB   *gSaapPidToDstIDCcTbl;
extern HASH_TABLE                        gSAAPPidToDstIDHashTbl;   // eid to dstid HASH��
extern SAAP_TELL_INDEX_DSTDEVICEID_CCB  stSaapTelToDstIDCcTbl[SAAP_ROUTE_TBL_MAX][SAAP_MAX_SAAPCCB_NUM]; //*gSaapTelToDstIDCcTbl;   ;
extern SAAP_HLR_INDEX_DSTDEVICEID_CCB   stSaapHlrToDstIDCcTbl[SAAP_ROUTE_TBL_MAX][SAAP_MAX_SAAPCCB_NUM]; //*gSaapHlrToDstIDCcTbl;   ;

extern SAAP_GLOBAL_STRUCT                g_ulSAAPGlobal;  // ���ڱ�ʶ���ѵ��������

extern SAAP_SPA_CLI_DATA_T gstSaapSpaCliData;
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
/// �Ŷα����豸ID����ز�������  //////////////////////////////////
////////////////////////////////////////////////////////////////////

/********************************** 
��������    : saap setGTTable
����        : 
�������    : 2008��9��24�� CodeReview
��������    : ��������������SAAP·�ɱ�
����IN      : XU8 tag ָʾ��HLR��������豸EID���tell�Ŷ�
����        : XU32 gt �ǺŶ�or �豸��
����IN      : XU16 DstID ΪĿ���������������ֵ
����IN      : XU32 ulRouteTblId ֻ��ȡ����ֵSAAP_ROUTE_TBL_HLR,SAAP_ROUTE_TBL_SMC,��ʾ���õ�����,SMC��HLR
����ֵ      : XU32 
************************************/
XU32 saap_setGTTable(XU8 tag, XU32 gt, XU16 DstID,XU32 ulRouteTblId)
{
    XU16  usNo=0;
    XU32  ulTemp=0;

    if(SAAP_ROUTE_TBL_MAX <= ulRouteTblId)
    {
        XOS_Trace( MD(FID_SAAP, PL_ERR), "saap setGTTable ulRouteTblId %d error.",ulRouteTblId);
        return XERROR;
    }
    gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[ulRouteTblId][0];
    gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[ulRouteTblId][0];

    switch( tag )
    {
        case  SAAP_OP_TEL_TABLE_TAG :  
            {
                // ����DN��
                usNo = SAAP_SearchTellToDstIDTable( gt ,ulRouteTblId);
                //������������Ѵ��ڣ����޸�
                if( ERR_CB_NUM != usNo )
                {
                    SYS_HashNodeDel(&gSAAPTelToDstIDHashTbl[ulRouteTblId], gSaapTelToDstIDCcTbl[usNo].tellNo, &ulTemp);
                    SAAP_InitTellToDstIDCb(usNo);
                    XOS_Trace( MD(FID_SAAP, PL_DBG), "The tellNO: %d(0x%x) is covered !", gt, gt );
                }

                usNo = SAAP_InsertTelToDstIDTable( gt, BLANK_ULONG,ulRouteTblId);
                if( ERR_CB_NUM == usNo )
                {
                    XOS_Trace( MD(FID_SAAP, PL_ERR), "NO CCB Full ERR: usNo=%x SAAP_InsertTelToDstIDTable()", gt );
                    return XERROR;
                }

                //��DN ROUTER��������
                gSaapTelToDstIDCcTbl[usNo].tellNo = gt ;
                gSaapTelToDstIDCcTbl[usNo].DstID = DstID ;

            }
            break;

        case  SAAP_OP_HLR_TABLE_TAG :  // ����UID��
            {
                usNo = SAAP_SearchHlrToDstIDTable( gt ,ulRouteTblId);
                //������������Ѵ��ڣ����޸�
                if( ERR_CB_NUM != usNo )
                {
                    SYS_HashNodeDel(&gSAAPHlrToDstIDHashTbl[ulRouteTblId], gSaapHlrToDstIDCcTbl[usNo].hlrNo, &ulTemp);
                    SAAP_InitHlrToDstIDCb(usNo);
                    XOS_Trace( MD(FID_SAAP, PL_DBG), "The hlrNO: %d(0x%x) is covered !", gt, gt );
                }

                usNo = SAAP_InsertHlrToDstIDTable( gt,  BLANK_ULONG,ulRouteTblId);
                if( ERR_CB_NUM == usNo )
                {
                    XOS_Trace( MD(FID_SAAP, PL_ERR), "NO CCB Full ERR: usNo=%x SAAP_InsertHlrToDstIDTable()", gt );
                    return XERROR;
                }

                //��UID ROUTER��������
                gSaapHlrToDstIDCcTbl[usNo].hlrNo = (XU16)gt ;
                gSaapHlrToDstIDCcTbl[usNo].DstID = DstID ;

            }
            break;

        case  SAAP_OP_EID_TABLE_TAG :  // ����EID��
            {
                usNo = SAAP_SearchPidToDstIDTable( gt );
                //������������Ѵ��ڣ����޸�
                if( ERR_CB_NUM != usNo )
                {
                    SYS_HashNodeDel(&gSAAPPidToDstIDHashTbl, gSaapPidToDstIDCcTbl[usNo].pidNo, &ulTemp);
                    SAAP_InitPidToDstIDCb(usNo);
                    XOS_Trace( MD(FID_SAAP, PL_DBG), "The pidNO: %d(0x%x) is covered !", gt, gt );
                }

                usNo = SAAP_InsertPidToDstIDTable( gt,  BLANK_ULONG);
                if( ERR_CB_NUM == usNo )
                {
                    XOS_Trace( MD(FID_SAAP, PL_ERR), " NO CCB Full ERR: usNo=%x SAAP_InsertPidToDstIDTable()", gt );
                    return XERROR;
                }

                //��PID ROUTER��������
                gSaapPidToDstIDCcTbl[usNo].pidNo = gt ;
                gSaapPidToDstIDCcTbl[usNo].DstID = DstID ;

            }
            break;
        default:
            XOS_Trace( MD(FID_SAAP, PL_WARN), "saap setGTTable(),unsupport tag %d",tag);
            break;
    }

    return XSUCC ;

}

/********************************** 
��������    : saap _delGTTable
����        : 
�������    : 2008��9��24�� CodeReview
��������    : ɾ��һ��GT��
����        : XU8 tag
����        : XU32 gt
����        : XU32 routeTabId
����ֵ      : XU32 
************************************/
XU32 saap_delGTTable(XU8 tag, XU32 gt,XU32 routeTabId )
{
    XU16 usNo;

    XU32 ulTemp,ulHashKey ;

    switch(tag)
    {
    case  SAAP_OP_TEL_TABLE_TAG :  // ����DN��
        {
            usNo = SAAP_SearchTellToDstIDTable( gt ,routeTabId);

            //ɾ��ʧ��
            if( ERR_CB_NUM == usNo )
            {
                XOS_Trace( MD(FID_SAAP, PL_ERR), "saap _delGTTable delete hash tbl tel node failed." );
                return XERROR;
            }
            gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[routeTabId][0];

            //��UID ROUTER��������
            ulHashKey = gSaapTelToDstIDCcTbl[usNo].tellNo;

            SAAP_InitTellToDstIDCb(usNo);

            if( XSUCC != SYS_HashNodeDel(&gSAAPTelToDstIDHashTbl[routeTabId],ulHashKey,&ulTemp))
            {
                XOS_Trace(MD(FID_SAAP,PL_ERR),"saap _delGTTable delete hash tbl tel node failed.");
                return XERROR;
            }
            if( ulTemp != usNo )
            {
                return XERROR;
            }

            usNo = (XU16)ulTemp;
        }

        break;

    case  SAAP_OP_HLR_TABLE_TAG :  // ����UID��
        {
            usNo = SAAP_SearchHlrToDstIDTable( gt ,routeTabId);

            //ɾ��ʧ��
            if( ERR_CB_NUM == usNo )
            {
                XOS_Trace(MD(FID_SAAP,PL_ERR),"saap _delGTTable delete hash tbl HLR node failed.");
                return XERROR;
            }

            //��UID ROUTER��������
            gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[routeTabId][0];
            ulHashKey = gSaapHlrToDstIDCcTbl[usNo].hlrNo;

            SAAP_InitHlrToDstIDCb(usNo);

            if( XSUCC != SYS_HashNodeDel(&gSAAPHlrToDstIDHashTbl[routeTabId], ulHashKey, &ulTemp) )
            {
                XOS_Trace(MD(FID_SAAP,PL_ERR),"saap _delGTTable delete hash tbl HLR node failed.");
                return XERROR;
            }
            if( ulTemp != usNo )
            {
                return XERROR;
            }

            usNo = (XU16)ulTemp;
        }

        break;

    case  SAAP_OP_EID_TABLE_TAG :  // ����EID��
        {
            usNo = SAAP_SearchPidToDstIDTable( gt );

            //ɾ��ʧ��
            if( ERR_CB_NUM == usNo )
            {
                XOS_Trace(MD(FID_SAAP,PL_ERR),"saap _delGTTable delete hash tbl PID node failed.");
                return XERROR;
            }

            //��UID ROUTER��������
            ulHashKey = gSaapPidToDstIDCcTbl[usNo].pidNo;

            SAAP_InitPidToDstIDCb(usNo);

            if( XSUCC != SYS_HashNodeDel(&gSAAPPidToDstIDHashTbl, ulHashKey, &ulTemp) )
            {
                XOS_Trace(MD(FID_SAAP,PL_ERR),"saap _delGTTable delete hash tbl PID node failed.");
                return XERROR;
            }
            if( ulTemp != usNo )
            {
                return XERROR;
            }

            usNo = (XU16)ulTemp;
        }
        break;
    default:
        XOS_Trace(MD(FID_SAAP,PL_INFO),"saap _delGTTable unsupport tag type.");
        return XERROR;
    }
    return XSUCC ;
}

/********************************** 
��������    : saap qryGTTable
����        : 
�������    : 2008��9��24�� CodeReview
��������    : ��ѯһ��GT������
����        : XU8 tag
����        : XU32 gt
����        : XU32 ulRouteTabid
����        : XU16 *pDstID
����ֵ      : XU32 
************************************/
XU32 saap_qryGTTable(XU8 tag, XU32 gt,XU32 ulRouteTabid, XU16 *pDstID  )
{
    XU16 usNo = 0;
    XU32 i =0;
    switch( tag )
    {
    case  SAAP_OP_TEL_TABLE_TAG :  // ����Ŷα�
        {
            usNo = SAAP_SearchTellToDstIDTable( gt,ulRouteTabid );

            for ( i = 0 ; i < 8 ; i++ )//8888 why macro
            {
                // �Ŷαȶ�
                usNo = SAAP_SearchTellToDstIDTable(gt,ulRouteTabid);
                if ( SAAP_ERROR_NUM != usNo)
                {
                    break;
                }
                else
                {
                    gt = gt / 10 ;//8888
                }
            }

            //����ʧ��
            if( ERR_CB_NUM == usNo )
            {
                XOS_Trace( MD(FID_SAAP, PL_ERR), "saap qryGTTable tel failed!" );
                return XERROR;
            }
            gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[ulRouteTabid][0];
            //UID ROUTER��������
            *pDstID = gSaapTelToDstIDCcTbl[usNo].DstID ;
        }
        break;

    case  SAAP_OP_HLR_TABLE_TAG :  // ����HLR��
        {
            usNo = SAAP_SearchHlrToDstIDTable( gt ,ulRouteTabid);

            //����ʧ��
            if( ERR_CB_NUM == usNo )
            {
                XOS_Trace( MD(FID_SAAP, PL_ERR), "saap qryGTTable HLR failed!" );
                return XERROR;
            }
            gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[ulRouteTabid][0];
            //UID ROUTER��������
            *pDstID = gSaapHlrToDstIDCcTbl[usNo].DstID ;
        }
        break;

    case  SAAP_OP_EID_TABLE_TAG :  // ����EID��
        {
            usNo = SAAP_SearchPidToDstIDTable( gt );
            //����ʧ��
            if( ERR_CB_NUM == usNo )
            {
                XOS_Trace( MD(FID_SAAP, PL_ERR), "saap qryGTTable PID failed!" );
                return XERROR;
            }

            *pDstID = gSaapPidToDstIDCcTbl[usNo].DstID ;

        }
        break;
    default:
        break;
    }

    return XSUCC ;

}

///////////////////////////////////////////////////////////////
// ����������Ϣ��������
///////////////////////////////////////////////////////////////

/********************************** 
��������    : saap_setLocalEID
����        : 
�������    : 2008��9��24�� CodeReview
��������    : �����豸������
����        :  XU32 eid  �豸��
����        : XU32 nodeType �ڵ�����
����ֵ      : XU32 
************************************/
XU32 saap_setLocalEID( XU32 eid,XU32 nodeType )
{

    XOS_Trace(MD(FID_SAAP,PL_MIN),"enter saap_setLocalEID.");
    if (BLANK_ULONG == eid)
    {
        g_ulSAAPGlobal.eidNoCfg.flag = 0 ;
    }
    else
    {
        g_ulSAAPGlobal.eidNoCfg.flag = 1 ;
    }
    if(SAAP_NODE_TYPE_MAX <= nodeType)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"saap nodeType(%d) err.",nodeType);
        return XERROR;
    }
    g_ulSAAPGlobal.eidNoCfg.eidNo = eid ;
    g_ulSAAPGlobal.eidNoCfg.nodeType = nodeType;

    XOS_Trace(MD(FID_SAAP,PL_MIN),"out saap_setLocalEID.");
    return XSUCC ;
}

/********************************** 
��������    : saap_SetSaapSstNoID
����        : 
�������    : 2008��9��24�� CodeReview
��������    : 
����        : XU32 flag  ָʾ�� DN ���� HLRNO
����        : XU32 index ָʾ����
����        : XU32 dataFalg ָʾ �����Ƿ���Ч
����        : XU32 hlrNo
����        : XU32 dn
����ֵ      : XU32 
************************************/
XU32 saap_SetSaapSstNoID(  XU32 flag , XU32 index , XU32 dataFalg , XU32 hlrNo , XU32 dn )
{
    if( flag == SAAP_OAM_CF_HLRNO_FLAG )
    {
        if (  SAAP_LOCAL_PREFIX_NUM <= index)
        {
            XOS_Trace(MD(FID_SAAP, PL_ERR), "saap_SetSaapSstNoID for HLR para index extend limit");
            return XERROR;
        }
        g_ulSAAPGlobal.hlrNoCfg[index].flag  =(XU8)dataFalg ;
        g_ulSAAPGlobal.hlrNoCfg[index].hlrNo = (XU16)hlrNo ;
    }
    else if( flag == SAAP_OAM_CF_DN_FLAG )
    {
        g_ulSAAPGlobal.tellNoCfg[index].flag    = (XU8)dataFalg ;
        g_ulSAAPGlobal.tellNoCfg[index].tellNo =  dn ;
    }

    return XSUCC ;
}


//////////////////////////////////////////////////////////////////////////
// SAAP�����к���
//////////////////////////////////////////////////////////////////////////

// ��ѯһ��Ŀ��GT
XVOID SaapGetDstDpByGt(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU8  tag;
    XU32  gt ;
    XU16 dstID;
    XU32 ulEpType = 0;

    if(4 != siArgc)
    {
        XOS_CliExtPrintf(pCliEnv, "para num err.\r\n");
        return;
    }

    tag = (XU8)XOS_ATOL(ppArgv[1]);
    ulEpType = XOS_ATOI(ppArgv[2]);
    XOS_StrToNum(ppArgv[3],&gt);

    if(SAAP_ROUTE_TBL_MAX <= ulEpType)
    {
        XOS_CliExtPrintf(pCliEnv,"epType should <%d \r\n",SAAP_ROUTE_TBL_MAX);
        return;
    }

    if(XSUCC == saap_qryGTTable(tag, gt ,ulEpType, &dstID ))
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nGt(0x%x(%d)-eptype(%d)->DestDpId=%-8d ", gt,gt,ulEpType,dstID );
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nnot found dstDp byGt(0x%x(%d)-eptype(%d).",gt,gt,ulEpType);
    }

    return;
}



/*

// ��Ϊ���Ը���ģʽ
XVOID SetSaapEncapMessageMonitorModel(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU8  model ;
    if( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv, "\r\n���������������");
        return;
    }

    model  = (XU8)XOS_ATOL(ppArgv[1]);

//  g_ucSaapMonitorMondel = model  ;

    XOS_CliExtPrintf(pCliEnv,  "\r\n���óɹ�"  );

    return;
}
*/



/********************************** 
��������	: SaapTblRegToOAM
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 uiModId
����ֵ		: void 
************************************/
void SaapTblRegToOAM(XU32 uiModId)
{
    XU32 aryTabList[] = 
    {SAAP_LOCAL_EID_TABLE,
    SAAP_LOCAL_UID_PREFIX_TABLE,
    SAAP_LOCAL_TELL_PREFIX_TABLE,
    SAAP_EID_ROUTE_TABLE,
    SAAP_UID_ROUTE_TABLE,
    SAAP_TELLNO_REOUTE_TABLE
    };
    
    t_XOSMUTEXID *ptLock = XNULL;
    // �����豸��
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_EID_TABLE,(unsigned short)APP_SYNC_MSG,  SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_EID_TABLE,(unsigned short)SA_UPDATE_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_EID_TABLE, SA_INSERT_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_EID_TABLE, SA_DELETE_MSG, SaapOamCallBack, ptLock);

    //(void)register_tb_proc(FID_SAAP, SAAP_LOCAL_EID_TABLE, SA_GET_MSG, SaapOamCallBack, ptLock);
    (void)get_resource(SAAP_LOCAL_EID_TABLE, sizeof(T_SAAP_LOCAL_EID_TBL), 1); // LOCOL dp record num is 1
    //����UIDǰ׺
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_UID_PREFIX_TABLE, APP_SYNC_MSG,  SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_UID_PREFIX_TABLE, SA_UPDATE_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_UID_PREFIX_TABLE, SA_INSERT_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_UID_PREFIX_TABLE, SA_DELETE_MSG, SaapOamCallBack, ptLock);
    //(void)register_tb_proc(FID_SAAP, SAAP_LOCAL_UID_PREFIX_TABLE, SA_GET_MSG, SaapOamCallBack, ptLock);
    (void)get_resource(SAAP_LOCAL_UID_PREFIX_TABLE, sizeof(T_SAAP_LOCAL_UID_TBL), SAAP_LOCAL_PREFIX_NUM);
    // ���ǵ绰����ǰ׺
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_TELL_PREFIX_TABLE , APP_SYNC_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_TELL_PREFIX_TABLE , SA_UPDATE_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_TELL_PREFIX_TABLE, SA_INSERT_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_LOCAL_TELL_PREFIX_TABLE, SA_DELETE_MSG, SaapOamCallBack, ptLock);
    //(void)register_tb_proc(FID_SAAP, SAAP_LOCAL_TELL_PREFIX_TABLE, SA_GET_MSG, SaapOamCallBack, ptLock);
    (void)get_resource(SAAP_LOCAL_TELL_PREFIX_TABLE, sizeof(T_SAAP_LOCAL_TELL_TBL), SAAP_LOCAL_PREFIX_NUM);
    //�豸��·�ɱ�
    (void)register_tb_proc(FID_SAAP, SAAP_EID_ROUTE_TABLE, APP_SYNC_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_EID_ROUTE_TABLE, SA_UPDATE_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_EID_ROUTE_TABLE, SA_INSERT_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_EID_ROUTE_TABLE, SA_DELETE_MSG, SaapOamCallBack, ptLock);
    //(void)register_tb_proc(FID_SAAP, SAAP_EID_ROUTE_TABLE, SA_GET_MSG, SaapOamCallBack, ptLock);
    (void)get_resource(SAAP_EID_ROUTE_TABLE, sizeof(T_SAAP_EID_ROUTE_TBL), SAAP_MAX_SAAPCCB_NUM);
    //UIDǰ׺·�ɱ�
    (void)register_tb_proc(FID_SAAP, SAAP_UID_ROUTE_TABLE, APP_SYNC_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_UID_ROUTE_TABLE, SA_UPDATE_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_UID_ROUTE_TABLE, SA_INSERT_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_UID_ROUTE_TABLE, SA_DELETE_MSG, SaapOamCallBack, ptLock);
    //(void)register_tb_proc(FID_SAAP, SAAP_UID_ROUTE_TABLE, SA_GET_MSG, SaapOamCallBack, ptLock);
    (void)get_resource(SAAP_UID_ROUTE_TABLE, sizeof(T_SAAP_UID_ROUTE_TBL), SAAP_MAX_SAAPCCB_NUM);
    //�绰����ǰ׺·��
    (void)register_tb_proc(FID_SAAP, SAAP_TELLNO_REOUTE_TABLE, APP_SYNC_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_TELLNO_REOUTE_TABLE, SA_UPDATE_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_TELLNO_REOUTE_TABLE, SA_INSERT_MSG, SaapOamCallBack, ptLock);
    (void)register_tb_proc(FID_SAAP, SAAP_TELLNO_REOUTE_TABLE, SA_DELETE_MSG, SaapOamCallBack, ptLock);
    //(void)register_tb_proc(FID_SAAP, SAAP_TELLNO_REOUTE_TABLE, SA_GET_MSG, SaapOamCallBack, ptLock);

    get_resource(SAAP_TELLNO_REOUTE_TABLE, sizeof(T_SAAP_TELLNO_ROUTE_TBL), SAAP_MAX_SAAPCCB_NUM);

    app_register(uiModId, aryTabList, sizeof(aryTabList)/sizeof(XU32));

    return;
}

/********************************** 
��������	: SaapOamCallBack
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32  uiTableId
����		: XU16 usMsgId
����		: XU32 uiSequence
����		: XU8 ucPackEnd
����		: tb_record *ptRow
����ֵ		: XU8 
************************************/
XU8 SaapOamCallBack(XU32  uiTableId, XU16 usMsgId, XU32 uiSequence,XU8 ucPackEnd, tb_record *ptRow)
{
    XS32  result;
    T_SAAP_LOCAL_EID_TBL * pSaapLocalEidTbl=XNULL;
    T_SAAP_LOCAL_PREFIX_TBL * pSaapLocalPrefixTabl=XNULL;
    T_SAAP_PREFIX_ROUTE_TBL *  pSaapUidRouteTbl=XNULL;

    if(XNULL == ptRow)
    {
        return OAM_CALL_FAIL;
    }

    while(XNULL != ptRow)
    {
        switch(uiTableId)
        {
            case SAAP_LOCAL_EID_TABLE:
                {
                    pSaapLocalEidTbl = (T_SAAP_LOCAL_EID_TBL*)ptRow->panytbrow;
                    if(XNULL != pSaapLocalEidTbl)
                    {
                        result = SaapLocalEidTblOpr(usMsgId, pSaapLocalEidTbl, ptRow->head.mask);
                    }
                    break;
                }
            case SAAP_LOCAL_UID_PREFIX_TABLE: // no break
            case SAAP_LOCAL_TELL_PREFIX_TABLE:
                {
                    pSaapLocalPrefixTabl = (T_SAAP_LOCAL_PREFIX_TBL *)ptRow->panytbrow;
                    if(XNULL != pSaapLocalPrefixTabl)
                    {
                        result = SaapLocalUidTellTblOpr(uiTableId,usMsgId, pSaapLocalPrefixTabl, ptRow->head.mask);
                    }
                    else
                    {
                        XOS_Trace(MD(FID_SAAP,PL_ERR)," pSaapLocalPrefixTabl ==XNULL.");
                    }
                    break;
                }
            case SAAP_EID_ROUTE_TABLE: //no break;
            case SAAP_UID_ROUTE_TABLE: // no break;
            case SAAP_TELLNO_REOUTE_TABLE:
                {
                    pSaapUidRouteTbl = (T_SAAP_PREFIX_ROUTE_TBL *)ptRow->panytbrow;
                    if(XNULL != pSaapUidRouteTbl)
                    {
                        result = SaapUidTellNoRouteTblOpr(uiTableId,usMsgId, pSaapUidRouteTbl, ptRow->head.mask);
                    }
                    else
                    {
                        XOS_Trace(MD(FID_SAAP,PL_ERR)," pSaapRouteTbl  ==XNULL.");
                    }
                    break;
                }
            default:
                {
                    XOS_Trace(MD(FID_SAAP,PL_ERR), "record is not saap config\r\n");
                    ptRow = (tb_record *)ptRow->next;
                    continue;
                    break;
                }
            }
        //�������
        if(OAM_CALL_SUCC != result)
        {
            XOS_Trace(MD(FID_SAAP,PL_ERR), "tab(%d) opt failed\r\n",uiTableId);
            if(APP_SYNC_MSG == usMsgId)
            {
                ptRow->head.err_no = OAM_RESPONSE_SYNC_ERROR;
            }
            if(SA_INSERT_MSG == usMsgId)
            {
                ptRow->head.err_no = OAM_RESPONSE_INSERT_ERROR;
            }
            if(SA_UPDATE_MSG == usMsgId)
            {
                ptRow->head.err_no = OAM_RESPONSE_UPDATE_ERROR;
            }
            if(SA_DELETE_MSG == usMsgId)
            {
                ptRow->head.err_no = OAM_RESPONSE_DELETE_ERROR;
            }
            if(SA_GET_MSG == usMsgId)
            {
                ptRow->head.err_no = OAM_RESPONSE_GET_ERROR;
            }
            }
        else
        {
            ptRow->head.err_no = OAM_RESPONSE_OK;
        }

        ptRow = ptRow->next;
    }
    return XSUCC;
}

/********************************** 
��������	: SaapLocalEidTblOpr
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU16 usMsgId
����		: T_SAAP_LOCAL_EID_TBL * pSaabLocalEidTbl
����		: char* mask
����ֵ		: XS32 
************************************/
XS32 SaapLocalEidTblOpr(XU16 usMsgId, T_SAAP_LOCAL_EID_TBL * pSaabLocalEidTbl, char* mask)
{
    XOS_Trace(MD(FID_SAAP,PL_MIN), "enter process SaapLocalEidTblOpr");
    if(XNULL == pSaabLocalEidTbl)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR)," pSaabLocalInfoTbl is Null");
        return XERROR;
    }

    switch(usMsgId)
    {
        case APP_SYNC_MSG:
        {
            XOS_Trace(MD(FID_SAAP,PL_DBG),"pSaabLocalEidTbl->localEid= %d",pSaabLocalEidTbl->localEid);
            return saap_setLocalEID(pSaabLocalEidTbl->localEid,pSaabLocalEidTbl->nodeType);
        }
        //break;
        case SA_UPDATE_MSG:
        {
            g_ulSAAPGlobal.eidNoCfg.flag =1;
            if(OAM_MASK_BIT_ZERO == (mask[0] & OAM_MASK_BIT_ZERO))//MASK=00000010��һ��
            {
                g_ulSAAPGlobal.eidNoCfg.eidNo = pSaabLocalEidTbl->localEid ;
                //return XSUCC;
            }
            if(OAM_MASK_BIT_ONE == (mask[0] & OAM_MASK_BIT_ONE))//MASK=00000010�ڶ���
            {
                if(SAAP_NODE_TYPE_MAX <= pSaabLocalEidTbl->nodeType)
                {
                    XOS_Trace(MD(FID_SAAP,PL_ERR)," saap nodetype(%d) err.",pSaabLocalEidTbl->nodeType);
                    return XERROR;
                }
                g_ulSAAPGlobal.eidNoCfg.nodeType = pSaabLocalEidTbl->nodeType;
                return XSUCC;
            }
            return XSUCC;
        }

        case SA_INSERT_MSG:
        {
            return saap_setLocalEID(pSaabLocalEidTbl->localEid,pSaabLocalEidTbl->nodeType);
        }

        case SA_DELETE_MSG:
        {
            return saap_setLocalEID(BLANK_ULONG,pSaabLocalEidTbl->nodeType);
        }
        default:
        {
            break;
        }
    }

    return OAM_CALL_SUCC;
}

/********************************** 
��������	: SaapLocalUidTellTblOpr
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 tabId
����		: XU16 usMsgId
����		: T_SAAP_LOCAL_PREFIX_TBL* pSaapTbl
����		: char* mask
����ֵ		: XS32 
************************************/
XS32 SaapLocalUidTellTblOpr(XU32 tabId,XU16 usMsgId, T_SAAP_LOCAL_PREFIX_TBL* pSaapTbl, char* mask)
{
    XU32 ulPreFixValue  = 0;
    XU32 index = 0;
    XU32 optFlag = 0;

    XOS_Trace(MD(FID_SAAP,PL_MIN),"enter SaapLocalUidTellTblOpr.");
    index = pSaapTbl->index;
    switch(usMsgId)
    {
        case APP_SYNC_MSG:
        {
            ulPreFixValue = pSaapTbl->localPrefix;
            if(SAAP_LOCAL_UID_PREFIX_TABLE == tabId)
            {
                return saap_SetSaapSstNoID( SAAP_OAM_CF_HLRNO_FLAG , index, 1 , ulPreFixValue , 0 );//8888 �����ú�
            }
            else
            {
                return saap_SetSaapSstNoID( SAAP_OAM_CF_DN_FLAG   , index , 1 , 0 , ulPreFixValue );//8888 �����ú�
            }
        }
        break;

        case SA_UPDATE_MSG:
        {
            if(OAM_MASK_BIT_ONE == (mask[0] & OAM_MASK_BIT_ONE))//MASK=00000010�ڶ���
            {
                ulPreFixValue = pSaapTbl->localPrefix;
            }
            else
            {
                return XSUCC;
            }

            if(SAAP_LOCAL_UID_PREFIX_TABLE == tabId)
            {
                return saap_SetSaapSstNoID( SAAP_OAM_CF_HLRNO_FLAG , index, 1 , ulPreFixValue , 0 );//8888 �����ú�
            }
            else
            {
                return saap_SetSaapSstNoID( SAAP_OAM_CF_DN_FLAG , index , 1 , 0 , ulPreFixValue );//8888 �����ú�
            }
        }
        break;

        case SA_INSERT_MSG:
        {
            ulPreFixValue = pSaapTbl->localPrefix;
            if(SAAP_LOCAL_UID_PREFIX_TABLE == tabId)
            {
                return saap_SetSaapSstNoID( SAAP_OAM_CF_HLRNO_FLAG , index, 1 , ulPreFixValue , 0 );//8888 �����ú�
            }
            else
            {
                return saap_SetSaapSstNoID( SAAP_OAM_CF_DN_FLAG    , index , 1 , 0 , ulPreFixValue );//8888 �����ú�
            }
        }

        case SA_DELETE_MSG:
        {
            if(SAAP_LOCAL_UID_PREFIX_TABLE == tabId)
            {
                optFlag = SAAP_OAM_CF_HLRNO_FLAG;
            }
            else
            {
                optFlag = SAAP_OAM_CF_DN_FLAG;
            }
            return saap_SetSaapSstNoID( optFlag , index, 0 , 0 , 0 ) ;//8888 �����ú�
        }
        default:
        {
            break;
        }
    }
    return XSUCC;
}

/********************************** 
��������	: SaapUidTellNoRouteTblOpr
����		: ..
�������	: 2008��9��25��
��������	: 
����		: XU32 tabId
����		: XU16 usMsgId
����		: T_SAAP_PREFIX_ROUTE_TBL* pSaapTbl
����		: char* mask
����ֵ		: XS32 
************************************/
XS32 SaapUidTellNoRouteTblOpr(XU32 tabId,XU16 usMsgId, T_SAAP_PREFIX_ROUTE_TBL* pSaapTbl, char* mask)
{
    XU32 ulDstEpType = 0;//Ŀ���豸����
    XU32 ulPreFixValue    = 0;//ǰ׺
    XU16 usDstDpId = 0;//Ŀ�������
    XU16 usNo = BLANK_USHORT;
    XU8  ucGtTag = 0;
    T_SAAP_EID_ROUTE_TBL * pSaapEidRouteTbl=XNULL;

    XOS_Trace(MD(FID_SAAP,PL_MIN), "enter process Tcpe_oamLinkTableOpr\r\n");

    if(SAAP_UID_ROUTE_TABLE == tabId)
    {
        ucGtTag = SAAP_OP_HLR_TABLE_TAG;
    }
    else if(SAAP_TELLNO_REOUTE_TABLE == tabId)
    {
        ucGtTag = SAAP_OP_TEL_TABLE_TAG;
    }
    else if(SAAP_EID_ROUTE_TABLE == tabId)
    {
        ucGtTag = SAAP_OP_EID_TABLE_TAG;
        pSaapEidRouteTbl = (T_SAAP_EID_ROUTE_TBL *)pSaapTbl;
    }
    else
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"tabid(%d) err.",tabId);
        return XERROR;
    }

    switch(usMsgId)
    {
        case APP_SYNC_MSG:
        {
            if(SAAP_EID_ROUTE_TABLE == tabId)
            {
                ulDstEpType= 0; //��Ȼ����EID·��ʱû�õ�,��Ҫ�úϷ�ֵ
                ulPreFixValue = pSaapEidRouteTbl->Eid;
                usDstDpId = (XU16)pSaapEidRouteTbl->destDpId;
            }
            else
            {
                ulDstEpType= pSaapTbl->destEpType;
                ulPreFixValue = pSaapTbl->routePreFix;
                usDstDpId = (XU16)pSaapTbl->destDpId;
            }

			if (ulPreFixValue > SAAP_TELLNO_PREFIX_MAX)
			{
				XOS_Trace(MD(FID_TCPE,PL_ERR),"\r\n [SAAP_TELLNO_REOUTE_TABLE]the prefix value[%d]of the telNo error.",ulPreFixValue);
				return XERROR;
			}
			
           	// ulDstEpType��Ч���ں����ڲ���
           	return saap_setGTTable( ucGtTag, ulPreFixValue, (XU16)usDstDpId,ulDstEpType);
        }
        break;

        case SA_UPDATE_MSG:
        {
            //Ԥ����
            if(SAAP_EID_ROUTE_TABLE == tabId)
            {
                ulPreFixValue = pSaapEidRouteTbl->Eid;
                ulDstEpType = SAAP_ROUTE_TBL_MAX;
            }
            else
            {
                ulPreFixValue = pSaapTbl->routePreFix;
                ulDstEpType = pSaapTbl->destEpType;
                if(SAAP_ROUTE_TBL_MAX <=ulDstEpType)
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR),"ulDstEpType error.");
                    return XERROR;
                }
            }

			if (ulPreFixValue > SAAP_TELLNO_PREFIX_MAX)
			{
				XOS_Trace(MD(FID_TCPE,PL_ERR),"\r\n [SAAP_TELLNO_REOUTE_TABLE]the prefix value[%d]of the telNo error.",ulPreFixValue);
				return XERROR;
			}
			
            switch(tabId)
            {
                case SAAP_EID_ROUTE_TABLE:
                    usNo = SAAP_SearchPidToDstIDTable(ulPreFixValue);
                    if(SAAP_MAX_SAAPCCB_NUM <=  usNo)
                    {
                        XOS_Trace(MD(FID_TCPE,PL_ERR),"can not find the record when upd eid route table.");
                        return XERROR;
                    }
                    if(OAM_MASK_BIT_ONE == (mask[0] & OAM_MASK_BIT_ONE))//MASK=00000010�ڶ���
                    {
                        gSaapPidToDstIDCcTbl[usNo].DstID = (XU16)pSaapEidRouteTbl->destDpId;
                    }
                    break;
                case SAAP_UID_ROUTE_TABLE:
                    //���ҵ�Hash
                    usNo = SAAP_SearchHlrToDstIDTable(ulPreFixValue, ulDstEpType);
                    if(SAAP_MAX_SAAPCCB_NUM <=  usNo)
                    {
                        XOS_Trace(MD(FID_TCPE,PL_ERR),"can not find the record when upd uid route table.");
                        return XERROR;
                    }
                    gSaapHlrToDstIDCcTbl = &stSaapHlrToDstIDCcTbl[ulDstEpType][0];
                    if(OAM_MASK_BIT_TWO == (mask[0] & OAM_MASK_BIT_TWO))//MASK=00000010�ڶ���
                    {
                        gSaapHlrToDstIDCcTbl[usNo].DstID = (XU16)pSaapTbl->destDpId;
                    }
                    break;
                case SAAP_TELLNO_REOUTE_TABLE:
                    //���ҵ�Hash
                    usNo =SAAP_SearchTellToDstIDTable( ulPreFixValue, ulDstEpType);
                    if(SAAP_MAX_SAAPCCB_NUM <=  usNo)
                    {
                        XOS_Trace(MD(FID_TCPE,PL_ERR),"can not find the record when upd tellno route table.");
                        return XERROR;
                    }
                    gSaapTelToDstIDCcTbl = &stSaapTelToDstIDCcTbl[ulDstEpType][0];
                    if(OAM_MASK_BIT_TWO == (mask[0] & OAM_MASK_BIT_TWO))//MASK=00000010�ڶ���
                    {
                        gSaapTelToDstIDCcTbl[usNo].DstID = (XU16)pSaapTbl->destDpId;
                    }
                    break;
                default:
                    XOS_Trace(MD(FID_TCPE,PL_DBG),"Saap UidTellNoRouteTblOpr update oper,unsupport table id(%d) err.",tabId);
                    break;
            }

        }
        break;

        case SA_INSERT_MSG:
        {
            if(SAAP_EID_ROUTE_TABLE == tabId)
            {
                ulPreFixValue = pSaapEidRouteTbl->Eid;
                usDstDpId = (XU16)pSaapEidRouteTbl->destDpId;
            }
            else
            {
                ulPreFixValue = pSaapTbl->routePreFix;
                usDstDpId = (XU16)pSaapTbl->destDpId;
                ulDstEpType= pSaapTbl->destEpType;
            }
			
			if (ulPreFixValue > SAAP_TELLNO_PREFIX_MAX)
			{
				XOS_Trace(MD(FID_TCPE,PL_ERR),"\r\n [SAAP_TELLNO_REOUTE_TABLE]the prefix value[%d]of the telNo error.",ulPreFixValue);
				return XERROR;
			}
			
			return saap_setGTTable( ucGtTag, ulPreFixValue, (XU16)usDstDpId,ulDstEpType);
        }
        case SA_DELETE_MSG:
        {
            switch(tabId)
            {
                case SAAP_EID_ROUTE_TABLE:
                    ucGtTag = SAAP_OP_EID_TABLE_TAG;
                    ulPreFixValue = pSaapEidRouteTbl->Eid;
                    ulDstEpType = 0;
                    //usDstDpId = (XU16)pSaapEidRouteTbl->destDpId;
                    break;
                case SAAP_UID_ROUTE_TABLE:
                    ucGtTag = SAAP_OP_HLR_TABLE_TAG;
                    ulPreFixValue = pSaapTbl->routePreFix;
                    //usDstDpId = (XU16)pSaapTbl->destDpId;
                    ulDstEpType= pSaapTbl->destEpType;
                    break;;
                case SAAP_TELLNO_REOUTE_TABLE:
                    ucGtTag = SAAP_OP_TEL_TABLE_TAG;
                    ulPreFixValue = pSaapTbl->routePreFix;
                    //usDstDpId = (XU16)pSaapTbl->destDpId;
                    ulDstEpType= pSaapTbl->destEpType;
                    break;
                default:
                    XOS_Trace(MD(FID_SAAP,PL_ERR),"tabid(%d) err.",tabId);
                    return XERROR;
                    //break;
            }
            return saap_delGTTable(ucGtTag, ulPreFixValue,ulDstEpType);
        }
        default:
        {
            break;
        }
    }
    return XSUCC;
}
/********************************** 
��������	: SAAP_SetMonFilter
����		: ..
�������	: 2008��9��25��
��������	: �¹�������,������Ϣ����
����		: SAAP_MON_FILTER_T *pSaapMonFilter
����ֵ		: XU32 
************************************/
XU32 SAAP_SetMonFilter(SAAP_MON_FILTER_T *pSaapMonFilter)
{
    XU32 ip  =0;
    XU32 port  = 0;
    XU32 idx = 0;
    SAAP_MON_FILTER_T mon_filter;

    if(XNULL == pSaapMonFilter)
    {
        XOS_Trace(MD(FID_SAAP,PL_ERR),"SAAP SetMonFilter input para is XNULL.");
        return XERROR;
    }
    XOS_MemCpy(&mon_filter, pSaapMonFilter, sizeof(SAAP_MON_FILTER_T));

    ip = mon_filter.ulSpaCliIpAddr;
    port = mon_filter.ulSpaCliPort;

    //Ccb call trace block
    idx  = Saap_SpaCliFindCcbByIp(ip, port);
    if(BLANK_ULONG == idx)
    {
        if(FLAG_NO == mon_filter.ulOptType) // cls filter
        {
            XOS_Trace(MD(FID_SAAP,PL_ERR),"find the saap filter ccb by ip=%x,port=%d failed.",ip,port);
            return XERROR;
        }
        else // set filter data
        {
            idx = Saap_SpaCliFindIdleCcb();
            if(BLANK_ULONG == idx)
            {
                XOS_Trace(MD(FID_SAAP,PL_ERR),"saap filter ccb is full.");
                return XERROR;
            }
            Saap_SpaCliInsert(idx, &mon_filter);
        }
    } else
    {
        // find the ccb ,change it 8888
        if(FLAG_NO == mon_filter.ulOptType)
        {
            Saap_SpaCliDelCcb(idx);//ȡ������,ɾ����¼,��̬��Դ��λ
        }
        else
        {
            Saap_SpaClisaveData( idx, &mon_filter);//�����������
        }
        return XSUCC;
    }

    return XSUCC;
}


/********************************** 
��������	: SAAP_FilterSaap2Tcpe
����		: ..
�������	: 2008��9��25��
��������	: 
����		: COMMON_HEADER_SAAP *pstMsg
����ֵ		: XU32 
************************************/
XU32 SAAP_FilterSaap2Tcpe(COMMON_HEADER_SAAP *pstMsg)
{
    XU32 gtValue = 0;//ȫ����
    SAAP_TO_TCPE_MSG_HEAD_T *pSaap2TpceMsgHead=XNULL;
    XU32 iRet = FLAG_NO;
    XU32 i  = 0;
    XU32 dstCodePlan = 0;

    if( FLAG_NO == gstSaapSpaCliData.ulTraceFlag)
    {
        return FLAG_NO;
    }

    pSaap2TpceMsgHead = (SAAP_TO_TCPE_MSG_HEAD_T *)pstMsg;

    switch(pSaap2TpceMsgHead->ucDstCodePlan)
    {
        case SAAP_G_DN_GT:
            gtValue = saap_BCD_to_GT( SAAP_GST_DN, pSaap2TpceMsgHead->uDstGst.gtDN);
            break;
        case SAAP_G_EID_GT: // no break
        case SAAP_G_UID_GT:
            gtValue = saap_BCD_to_GT( SAAP_GST_UID, pSaap2TpceMsgHead->uDstGst.gtDN);
            break;
        //default:
            //iRet = FLAG_NO;
            //break;
    }
    //�õ���żƻ� ��4λ,ĿǰΪ1,8,9
    dstCodePlan = pSaap2TpceMsgHead->ucDstCodePlan >> 4; 
    //gtValue = SYS_NTOHL(gtValue);
    for(i =0;i<SAAP_MAX_SPA_CLI_NUM;i++)
    {
        if(FLAG_NO == gstSaapSpaCliData.cliData[i].busyflag)
        {
            continue;
        }
        if(dstCodePlan != gstSaapSpaCliData.cliData[i].ucCodePlan && BLANK_UCHAR !=gstSaapSpaCliData.cliData[i].ucCodePlan)
        {
            continue;
        }
        if(gstSaapSpaCliData.cliData[i].gtValue == gtValue || BLANK_ULONG  == gstSaapSpaCliData.cliData[i].gtValue )
        {
            gstSaapSpaCliData.cliData[i].ulSend2MntFlag = FLAG_YES;
            iRet =  FLAG_YES;
            //break;
        }
    }
    return iRet;
}

/********************************** 
��������	: SAAP_FilterSaap2Srv
����		: ..
�������	: 2008��9��25��
��������	: 
����		: COMMON_HEADER_SAAP *pstMsg
����ֵ		: XU32 
************************************/
XU32 SAAP_FilterSaap2Srv(COMMON_HEADER_SAAP *pstMsg)
{
    XU32 id = 0;
    APP_OR_SAAP_COMMON_HEADER * pSaapAppMsgHead = XNULL;
    XU32 i  = 0;
    XU32 iRet = FLAG_NO;
    XU32 dstCodePlan = 0;
    pSaapAppMsgHead = (APP_OR_SAAP_COMMON_HEADER *)pstMsg;

    if( FLAG_NO == gstSaapSpaCliData.ulTraceFlag)
    {
        return FLAG_NO;
    }
    if(SAAP_GST_DN == pSaapAppMsgHead->appSaapHead.uGst.GuestSerialNumTag)
    {
        id = saap_BCD_to_GT(SAAP_GST_DN,pSaapAppMsgHead->appSaapHead.uGst.uGstNID.GuestNetIDTell);
    }
    else
    {
        id = pSaapAppMsgHead->appSaapHead.uGst.uGstNID.GuestNetID;
    }

    dstCodePlan = pSaapAppMsgHead->appSaapHead.uGst.GuestSerialNumTag;
    for(i = 0; i<SAAP_MAX_SPA_CLI_NUM;i++)
    {
        if(FLAG_NO == gstSaapSpaCliData.cliData[i].busyflag)
        {
            continue;
        }
        if(dstCodePlan != gstSaapSpaCliData.cliData[i].ucCodePlan && BLANK_UCHAR !=gstSaapSpaCliData.cliData[i].ucCodePlan)
        {
            continue;
        }
        if( id == gstSaapSpaCliData.cliData[i].gtValue || BLANK_ULONG == gstSaapSpaCliData.cliData[i].gtValue)
        {
            gstSaapSpaCliData.cliData[i].ulSend2MntFlag = FLAG_YES;
            iRet =  FLAG_YES;
            //break;
        }
    }
    return iRet;
}
#ifdef __cplusplus
    }
#endif


