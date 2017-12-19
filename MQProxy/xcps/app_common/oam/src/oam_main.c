/****************************************************************************
*��Ȩ     : Xinwei Telecom Technology Inc
*�ļ���   : oam_main.cpp
*�ļ����� : OAM ģ�������ģ���ṩ�Ľӿڵ�ʵ��
*����     : xiaohuiming
*�������� : 2014-09-01
*�޸ļ�¼ :
****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
//#pragma warning(disable:4996)
#include "oam_main.h"
#include "oam_file.h"
#include "oam_link.h"

XSTATIC OAM_SESSION_HEAD_T g_AgtCfgHead[OAM_SESSION_MAX_NUM];

/*����ģ���ϵ�ṹ*/
OAM_TID_MID_T g_TidMidArray[MAX_TB_NUM];
XU32 g_TidMidCnt = 0;
OAM_TID_MID_T g_AsyncTidMidArray[MAX_TB_NUM];
XU32 g_AsyncTidMidCnt = 0;
FM_ALARM_SAVE_T g_AlarmMsgSave[FM_TYPE_NUM];/*0:���ϸ澯���� 1:�¼��澯����*/
NE_MDU_T g_NeProcInfo;


XU32 g_AgtSrvIpAddr = 0; /*agent��IP��ַ*/
XU32 g_TsFbIpAddr = 0;   /*TS������Ҫ��ǰ����IP��ַ*/

XU32 g_SlotId[HA_IP_NUM] = {0};/*������λ��*/


XS8 g_ProcessType[VALUE_MAX_LEN] = {0};/*��������*/

extern XS8 g_xmlFilePath[FILE_PATH_MAX_LEN];
extern OAM_LINK_CB_T g_OamLinkCB;

XSTATIC t_XOSFIDLIST agent_oam_stack =
{
    { "FID_OAM",  XNULL, FID_OAM,},
    { OAM_Init, OAM_Notice, NULL,},
    { OAM_MsgProc, OAM_TimeOut,}, 
    eXOSMode,
    NULL
};


/****************************************************************************
*������:OAMEntry
*����  :XOS��ں���
*����  :
*           HANDLE hdir
*           XS32 argc
*           XCHAR** argv
*���  :
*����  :
*           XS8
*˵��  
****************************************************************************/
XS32 OAMEntry(HANDLE hdir, XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST login_list;
    XOS_MemSet(&login_list, 0, sizeof(t_XOSLOGINLIST));

    XOS_Sprintf(login_list.taskname, MAX_TID_NAME_LEN, "task_oam");
    login_list.stack     = &agent_oam_stack;
    login_list.TID       = FID_OAM;
    login_list.prio      = TSK_PRIO_NORMAL;
    login_list.quenum    = MAX_MSGS_IN_QUE;
    
    return XOS_MMStartFid(&login_list, 0, NULL);
}

/****************************************************************************
*������:OAM_Init
*����  :��XOS�ṩ��ģ���ʼ������
*����  :
*           XVOID *Para1
*           XVOID *Para2
*���  :
*����  :
*           XS8
*˵��  :
****************************************************************************/
XS8  OAM_Init(XVOID* Para1, XVOID* Para2)
{
    OAM_LINK_CFG_T oamCfg = { 0 };

    XOS_PRINT(MD(FID_OAM, PL_DBG), "Enter OAM_Init.\r\n");
    
    XOS_MemSet(g_AgtCfgHead, 0, OAM_SESSION_MAX_NUM * sizeof(OAM_SESSION_HEAD_T));
    XOS_MemSet(g_TidMidArray, 0, sizeof(g_TidMidArray));
    XOS_MemSet(g_AsyncTidMidArray, 0, sizeof(g_AsyncTidMidArray));
    XOS_MemSet(g_AlarmMsgSave, 0, sizeof(g_AlarmMsgSave));
    XOS_MemSet(&g_NeProcInfo, 0, sizeof(g_NeProcInfo));

    /*��ʼ��OAM��ע���̹������Ϣ*/
    g_TidMidArray[0].uiModuleId = FID_OAM;
    g_TidMidArray[0].uiTableId  = TID_PROCESS_MANAGE;
    g_TidMidCnt++;

    /*��ȡ�����ļ�·������������*/
    OAM_XmlFilePathGet();

    /*ͨ��xml�ļ���ȡAgent Ip ��ַ*/
    if (XSUCC != OAM_XmlTblInfoGet(g_xmlFilePath, AGENT_IP_TABLE_ID, FID_OAM, OAM_CFG_SYNC))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "OAM_XmlTblInfoGet agent IP[%s] failed.\r\n", g_xmlFilePath);
    }

    /*ͨ��xml�ļ���ȡ������Ϣ*/
    if (XSUCC != OAM_XmlTblInfoGet(g_xmlFilePath, TID_MODULE_MANAGE, FID_OAM, OAM_CFG_SYNC))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "OAM_XmlTblInfoGet TID_MODULE_MANAGE failed.\r\n");
    }
    
    /*��дagent IP��ַ*/
    oamCfg.peerIp = g_AgtSrvIpAddr;
    XOS_PRINT(MD(FID_OAM, PL_DBG), "oam get [%s] agent IP[0x%x].\r\n",g_xmlFilePath,g_AgtSrvIpAddr);
    
    /*ͨ��ƽ̨�ӿڻ�ȡ��Ԫ�š�����ID����źͲ�λ��*/
    if(0 == XOS_StrNcmp(g_ProcessType, "bm", XOS_StrLen("bm")))
    {
        oamCfg.siPID = OAM_BM_PID;
    }
    else if(0 == XOS_StrNcmp(g_ProcessType, "ts", XOS_StrLen("ts")))
    {
        oamCfg.siPID = OAM_TS_PID;
    }
    else
    {
    //#ifdef XOS_LINUX
	#if 0
        if(XERROR == (oamCfg.siNeID = XOS_GetNeId()))
        {
            XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetNeId[%d] failed.\r\n",oamCfg.siNeID);
            return XERROR;
        }

        if(XERROR == (oamCfg.siPID = XOS_GetLogicPid()))
        {
            XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetLogicPid[%d] failed.\r\n",oamCfg.siPID);
            return XERROR;
        }
    #endif
    }

	//#ifdef XOS_LINUX
#if 0
    if(XERROR == (oamCfg.siFrameId = XOS_GetShelfNum()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetShelfID[%d] failed.\r\n",oamCfg.siFrameId);
        return XERROR;
    }

    if(XERROR == (oamCfg.siSlotId  = XOS_GetSlotNum()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetSlotNum[%d] failed.\r\n",oamCfg.siSlotId);
        return XERROR;
    }
#endif

#ifdef WIN32/*test*/
    oamCfg.siNeID    = 1;
    oamCfg.siPID     = 1;
    oamCfg.siFrameId = 1;
    oamCfg.siSlotId  = 1;
#endif
    OAM_LinkCBInit(oamCfg);    
    OAM_LinkInit();

    /*������ע��*/
    OAM_CliRegister();
        
    return XSUCC;
}


/****************************************************************************
*������: OAM_Notice
*����  : NOTICE����Ϣ������
*����  :
*           XVOID *pMsg
*           XVOID *Para
*���  :
*����  :
*           XS8
*˵��  :
****************************************************************************/
XS8  OAM_Notice(XVOID* pMsg, XVOID* Para)
{
    return XSUCC;
}


/****************************************************************************
*������: OMA_MsgProc
*����  : ��XOS�ṩ����Ϣ������
*����  :
*           XVOID *msg
*           XVOID *Para
*���  :
*����  :
*           XS8
*˵��  :
****************************************************************************/     
XS8  OAM_MsgProc(XVOID* pMsg, XVOID* Para )
{
    XS32 siRet = XSUCC;
    t_XOSCOMMHEAD *pRecvMsg = XNULL;
    TS_IP_INFO_T stTsData = {0};
    
    PTR_NULL_CHECK(pMsg, XERROR);
    
    pRecvMsg = (t_XOSCOMMHEAD *)pMsg;

    /*ģ��ע����Ϣ*/
    if (APP_REGISTER_MSG == pRecvMsg->msgID)
    {
        return OAM_AppRegMsgProc(pRecvMsg, pRecvMsg->message, pRecvMsg->datasrc.FID);
    }

    /*TS��ȡǰ���IP��Ϣ*/
    if (OAM_MSG_IP_REQ == pRecvMsg->msgID)
    {
        stTsData.uiIpAddr = g_TsFbIpAddr;
        stTsData.uiMslotId = g_SlotId[0];
        stTsData.uiSslotId = g_SlotId[1];
        XOS_PRINT(MD(FID_OAM, PL_LOG), "send to ts info{ip=0x%x,Mslotid=%d,sslotid=%d}!\r\n",
            stTsData.uiIpAddr,stTsData.uiMslotId,stTsData.uiSslotId);
        return OAM_OperMsgSend(pRecvMsg->datasrc.FID,OAM_MSG_IP_RSP,sizeof(TS_IP_INFO_T),&stTsData);
        
    }

    /*��Ԫ���ܲ������ݶ�ʱ�ϱ���Ϣ*/
    if (OAM_PM_NOTIFY_MSG == pRecvMsg->msgID)
    {
        return OAM_PmReportMsgProc(pRecvMsg->message);
    }

    if (AGENT_DEBUG_RSP_MSG == pRecvMsg->msgID)
    {
        return OAM_SoftDebugRspSend((OAM_SOFTDEBUG_RSP_T*)pRecvMsg->message);
    }
    
    /*BM״̬�ϱ�����Ӧ��Ϣ*/
    if (FID_BM == pRecvMsg->datasrc.FID)
    {
        siRet = OAM_BmMsgProc(pRecvMsg);
    }
    else if (FID_NTL == pRecvMsg->datasrc.FID)/*agent���úͲ�����Ϣ*/
    {
        siRet = OAM_NtlMsgProc(pRecvMsg);
    }
    else if (FID_OAM == pRecvMsg->datasrc.FID)
    {
        siRet = OAM_SelfMsgProc(pRecvMsg->message);/*����������Ϣ*/
    }
    else /*ҵ��������Ӧ��Ϣ*/ 
    {
        siRet = OAM_AppRspMsgProc(pRecvMsg);   
    }
    
    return (XS8)siRet;
}


/****************************************************************************
*������: OAM_BmMsgProc
*����  : ������BM��ص���Ϣ
*����  :
*           t_XOSCOMMHEAD *pMsg ���յ�BM��Ϣ
*���  :
*����  :
*           XS8
*˵��  :
****************************************************************************/     
XS8 OAM_BmMsgProc(t_XOSCOMMHEAD *pMsg )
{
    t_XOSCOMMHEAD *pRecvMsg = XNULL;
    XS32 siRet = XSUCC;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_BmMsgProc function!\r\n");

    pRecvMsg = pMsg;

    switch(pRecvMsg->msgID)
    { 
        case MSG_REP_BOARD_HARDINFO: /*�����ϱ�����Ӳ����Ϣ*/
        {
            if(XSUCC != (siRet = OAM_AgtReqSend(pRecvMsg, pRecvMsg->msgID, FID_BM, 0)))
            {
                XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_AgtReqSend %d msg\r\n",pRecvMsg->msgID);
            }
            else
            {
                if(XSUCC != OAM_MsgIdSend(FID_BM, pRecvMsg->msgID))
                {
                    XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_MsgIdSend failed!\r\n");
                    return XERROR;
                }
            }
        }
        break;
        
        case MSG_REQ_PROC_OPER: /*BM�������������б���Ϣ*/
        case MSG_REP_PROCINFO:  /*�ϱ���Ԫ������Ϣ*/
        case MSG_REP_NETIFINFO: /*�ϱ�������Ϣ��Ϣ*/
        case MSG_REP_BOARDSTAT: /*�ϱ�����״̬*/
        case MSG_REP_PROCSTAT:  /*�ϱ�����״̬*/
        case MSG_REP_NETIFSTAT: /*�ϱ�����link״̬*/
        case MSG_REP_DISKINFO:  /*�ϱ�������Ϣ*/
        case MSG_REP_BOARDINFO: /*�ϱ�������Ϣ*/
        case MSG_QUE_BOARD_HARDINFO:/*��ѯ����Ӳ����Ϣ*/
        {
            siRet = OAM_AgtReqSend(pRecvMsg, pRecvMsg->msgID, FID_BM, 0);
        }
        break;

        case MSG_GET_AGENTIP:       /*��ȡAGENT IP��Ϣ*/
        {
            siRet = OAM_OperMsgSend(FID_BM, pRecvMsg->msgID, sizeof(XU32), &g_AgtSrvIpAddr);
        }    
        break;
        
        default:
            XOS_PRINT(MD(FID_OAM,PL_WARN),"unknown msg\r\n");
            break;
    }

    return siRet;
}


/****************************************************************************
*������: OAM_NtlMsgProc
*����  : ������NTL��ص���Ϣ
*����  :
*           t_XOSCOMMHEAD *pMsg ���յ�BM��Ϣ
*���  :
*����  :
*           XS8
*˵��  :
****************************************************************************/     
XS8 OAM_NtlMsgProc(t_XOSCOMMHEAD *pMsg )
{
    t_XOSCOMMHEAD *pRecvMsg = XNULL;
    XS32 siRet = XSUCC;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_NtlMsgProc function!\r\n");

    pRecvMsg = pMsg;

    switch(pRecvMsg->msgID)
    {
        case eSctpInitAck:/*��·��ʼ����Ӧ����*/
            siRet = OAM_LinkInitAckProc(pRecvMsg);
            break;

        case eSctpStartAck:/*��·������Ӧ����*/
            siRet = OAM_LinkStartAckProc(pRecvMsg);
            break;

        case eSctpDataInd:/*��·������Ϣ����*/
            siRet = OAM_LinkDataIndProc(pRecvMsg);
            break;

        case eSctpStopInd:/*��·�Ͽ�ָʾ*/
            siRet = OAM_LinkStopIndProc(pRecvMsg);
            break;

        case eSctpErrorSend:/*���ݷ��ʹ���*/
            XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM eErrorSend: reason=%d\r\n", 
                                                ((t_SENDSCTPERROR*)pRecvMsg->message)->errorReson);
            break;
            
        default:
            XOS_PRINT(MD(FID_OAM,PL_WARN),"unknown msg\r\n");
        break;
    }

    return siRet;
}

/****************************************************************************
*������: OAM_TimeOut
*����  : ��XOS�ṩ�Ķ�ʱ����ʱ������
*����  :
*           t_BACKPARA  *pstPara
*���  :
*����  :
*           XS8
*˵��  :
****************************************************************************/     
XS8  OAM_TimeOut(t_BACKPARA* para)
{
    XS8 siRet = XSUCC;

    if(XNULL == para)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_TimeOut: para is null");
        return XERROR;
    }

    switch(para->para1)
    {
    case TM_TYPE_NULL:
    case TM_TYPE_LINKINIT:
    case TM_TYPE_LINKSTART:
    case TM_TYPE_REG:
    case TM_TYPE_HS:
    case TM_TYPE_HS_ACK:
    case TM_TYPE_SYNC:
        {
            /*��·��ʱ����ʱ����*/
            siRet = OAM_LinkTimeOutProc(para);
        }
        break;
        
    default:
        XOS_PRINT(MD(FID_OAM, PL_WARN), "unknown timer para: %d", para->para1);
        break;
    }
    
    return siRet;
}


/****************************************************************************
*������:OAM_AppRegister
*����  : Ӧ��ģ��ע�����Ϣ�Ļص�����
*����  :
*            XU32 uiModuleId  ģ��ID
*            XU32 *pTableId   ��ID�б�
*            XU32 uiTableNum  �����
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AppRegister(XU32 uiModuleId, const XU32 *pTableId, XU32 uiTableNum)
{    
    XU32 uiType     = 0;
    XU32 uiOffset   = 0;
    XU32 uiTotalLen = 0;
    XS8* pBuf           = XNULL;
    t_XOSCOMMHEAD *pMsg = XNULL;

    unsigned int uiLen = uiTableNum;
        
    /*ָ����μ��*/
    PTR_NULL_CHECK(pTableId, XERROR);

    /*count������λ��ʾcount��ֵ����Ϊԭ����4��*/
    uiTableNum = uiTableNum << OAM_OFFSET_LEN;
    if(uiTableNum == 0)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "\table count is less than zero!\r\n");
        return XERROR;
    }

    /*TLV��ʽ�����͡����Ⱥ�ֵ��ռ�ڴ�֮�͵Ĵ�С*/
    uiTotalLen = uiTableNum + OAM_OFFSET_LEN * sizeof(unsigned int);
    
    if(XNULL == (pMsg = XOS_MsgMemMalloc(uiModuleId, uiTotalLen)))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR),"XOS_MsgMemMalloc failed!\r\n");
        return XERROR;
    }

    /*��д��Ϣ����*/
    SET_XOS_MSG_HEAD(pMsg, APP_REGISTER_MSG, uiModuleId, FID_OAM);
    
    /*��TLV��ʽ������ID*/
    //if(XNULL != pTableId)
    {
        pBuf = (XS8*)pMsg->message;
        
        /*ָ����������*/
        uiType = OAM_REG_TALBEID;
        memcpy((void*)&pBuf[uiOffset], (void*)&uiType, sizeof(XU32));
        uiOffset += sizeof(XU32);
        
        /*ָ����������*/
        memcpy((void*)&pBuf[uiOffset], (void*)&uiLen, sizeof(XU32));
        uiOffset += sizeof(XU32);
        
        /*ָ��������ֵ*/
        memcpy((void*)&pBuf[uiOffset], pTableId, uiTableNum);
    }

    if (XSUCC != XOS_MsgSend(pMsg))
    {
        PTR_MEM_FREE(FID_OAM, pMsg);
        XOS_PRINT(MD(FID_OAM, PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    return XSUCC;
}


/****************************************************************************
*������:OAM_AppRegMsgProc
*����  : Ӧ��ģ��ע����Ϣ����
*����  :
*            XVOID *pRecvMsg  �յ���ע����Ϣ
*            XVOID *pMsg      ��Ϣ����
*            XU32 uiModuleId  ģ��ID
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS8 OAM_AppRegMsgProc(XVOID* pRecvMsg, XVOID *pMsg, XU32 uiModuleId)
{
    XU32 uiLoop     = 0;
    XU32 uiRegType  = 0;   
    XU32 uiOffset   = 0;
    XU32 uiTableId  = 0;
    XU32 uiTableNum = 0;
    XS8 *pTempVal   = XNULL;
    OAM_LINK_T stOamLink = { {0} };

    /*ָ����μ��*/
    PTR_NULL_CHECK(pRecvMsg, XERROR);
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AppRegMsgProc function!\r\n");
    
    pTempVal = (XS8*)pMsg;

    /*ȡ��ע������*/
    XOS_MemCpy((XS8*)&uiRegType, (XVOID*)&pTempVal[uiOffset], sizeof(XU32));
    uiOffset += sizeof(XU32);

    /*ȡ��ע�����*/
    XOS_MemCpy((XS8*)&uiTableNum, (XVOID*)&pTempVal[uiOffset], sizeof(XU32));
    uiOffset += sizeof(XU32);
        
    for (uiLoop = 0; uiLoop < uiTableNum; uiLoop++)
    {
        uiTableId = 0;
        XOS_MemCpy((XS8*)&uiTableId, (XVOID*)&pTempVal[uiOffset], sizeof(XU32));
        uiOffset += sizeof(XU32);

        /*����洢*/
        if(XERROR==OAM_TableIdAdd(uiTableId,uiModuleId))
        {
            XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_TableIdAdd[tid=%d][mid=%d] failed.\r\n",
                uiTableId,uiModuleId);
            return XERROR;
        }

        /*�Ȼ�ȡ��·״̬*/
        if(XSUCC != OAM_LinkGet(&stOamLink))
        {
            /*����洢*/
            if(XERROR == OAM_AsyncTableIdAdd(uiTableId,uiModuleId))
            {
                XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_AsyncTableIdAdd[tid=%d][mid=%d] failed.\r\n",
                uiTableId,uiModuleId);
                return XERROR;
            }
        
            /*�ӱ���xml�ļ����ұ�ID��Ӧ���ݼ�¼������
            if (XSUCC != OAM_XmlTblInfoGet(g_xmlFilePath, uiTableId, uiModuleId))
            {
                XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_XmlTblInfoGet Module[%d]Table[%d] failed\r\n",
                                                                           uiModuleId, uiTableId);
                return XERROR;
            }*/
        }
        else
        {
            /*������������agent��ȡ��ID��Ӧ�����ݼ�¼*/
            if(XSUCC != OAM_AgtSyncReqSend(OAM_MSGTYPE_SYNC, uiModuleId, uiTableId))
            {
                XOS_PRINT(MD(FID_OAM, PL_ERR), 
                    "OAM_AppRegMsgProc: OAM_AgtSyncReqSend(OAM_MSGTYPE_SYNC) failed\r\n");
                return XERROR;
            }

            XOS_PRINT(MD(FID_OAM, PL_LOG), 
                    "OAM_AppRegMsgProc: OAM Send(OAM_MSGTYPE_SYNC)[mid=%d,tid=%d] succ\r\n",
                    uiModuleId, uiTableId);
            
            XOS_Sleep(100);
        }
    }

    return XSUCC;
}


/****************************************************************************
*������:OAM_SelfMsgProc
*����  : ����ģ���ʽ������������Ϣ
*����  :
*            XVOID*pMsg
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_SelfMsgProc(XVOID*pMsg)
{
    AGT_OAM_CFG_REQ_T *pAgtOamData = XNULL;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_SelfMsgProc function!\r\n");

    pAgtOamData = (AGT_OAM_CFG_REQ_T *)pMsg;

    OAM_NeProcInfoProc(pAgtOamData->pData, OAM_CFG_FMT, pAgtOamData->uiMsgLen);

    return XSUCC;
}


/****************************************************************************
*������:OAM_AgtCfgMsgProc
*����  : agent������Ϣ����
*����  :
*            XVOID*pMsg    ������Ϣͷ����Ϣ��
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AgtCfgMsgProc(XVOID*pMsg)
{
    XU32 uiHeadIndex = 0;
    XU32 RetValLen = 0;
    XU32 uiLoop    = 0;
    XU32 uiDataLen = 0;
    XS8 ucRetData[BUFF_MAX_LEN] = {0};
    XU32 uiMid[MAX_TB_NUM] = {0};
    XU32 uiCount =0;
    XS8 *pTempData = XNULL;
    XS8* pRecvData = XNULL;/*�������ݵ�ַ*/
    AGT_CFG_REQ_T* pRecvMsg = XNULL;
    AGT_OAM_CFG_REQ_T stAgtOamData = {0};
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtCfgMsgProc function!\r\n");
    
    pRecvMsg = (AGT_CFG_REQ_T *)pMsg;
    
    /*��������������ʼ��ַ*/
    pRecvData = (XS8*)pMsg;
    pRecvData += (sizeof(AGT_CFG_REQ_T) - sizeof(XS8*));

    uiDataLen = XOS_NtoHl(pRecvMsg->stAgtOamData.uiMsgLen);

    /*��ȡδʹ�õ���Ϣͷ���ƿ���������¼��Ϣͷ*/
    if(XSUCC != OAM_HeadIndexGet(&uiHeadIndex))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_AgtCfgHead List is full\r\n");
        return XERROR;
    }
    
    XOS_MemCpy(&g_AgtCfgHead[uiHeadIndex].stAgtCfgHead, &pRecvMsg->stAgtCfgHead, 
                                                            sizeof(AGT_OAM_CFG_HEAD_T));
    g_AgtCfgHead[uiHeadIndex].ucUsed = OAM_SESSION_HEAD_USED;

    if (XNULL == (stAgtOamData.pData = (XS8*)XOS_MemMalloc(FID_OAM,uiDataLen)))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "XOS_MemMalloc failed.\r\n");
        return XERROR;
    }
    XOS_MemSet(stAgtOamData.pData, 0, uiDataLen);

    /*ת������*/
    stAgtOamData.uiIndex     = XOS_NtoHl(pRecvMsg->stAgtOamData.uiIndex);
    stAgtOamData.uiSessionId = XOS_NtoHl(pRecvMsg->stAgtOamData.uiSessionId);
    stAgtOamData.usNeId      = XOS_NtoHs(pRecvMsg->stAgtOamData.usNeId);
    stAgtOamData.usPid       = XOS_NtoHs(pRecvMsg->stAgtOamData.usPid);
    stAgtOamData.usModuleId  = XOS_NtoHs(pRecvMsg->stAgtOamData.usModuleId);
    stAgtOamData.uiOperType  = XOS_NtoHl(pRecvMsg->stAgtOamData.uiOperType);
    stAgtOamData.uiTableId   = XOS_NtoHl(pRecvMsg->stAgtOamData.uiTableId);
    stAgtOamData.uiRecNum    = XOS_NtoHl(pRecvMsg->stAgtOamData.uiRecNum);
    stAgtOamData.uiMsgLen    = XOS_NtoHl(pRecvMsg->stAgtOamData.uiMsgLen);
    
    /*������������*/
    XOS_MemCpy(stAgtOamData.pData, pRecvData, uiDataLen);
    
    if (OAM_CFG_ADD == stAgtOamData.uiOperType 
        || OAM_CFG_MOD == stAgtOamData.uiOperType
        || OAM_CFG_DEL == stAgtOamData.uiOperType)
    {
        /*��agent����Ӧ*/
        if (XSUCC != OAM_AgtRspSend(pRecvMsg))
        {
            PTR_MEM_FREE(FID_OAM, stAgtOamData.pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_AgtRspSend failed\r\n");
            return XERROR;
        }

        if (XSUCC != OAM_XmlCfgDataWrite(&stAgtOamData,stAgtOamData.uiOperType))
        {
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_XmlCfgDataWrite[TYPE:%D] failed\r\n",
                                          stAgtOamData.uiOperType);
            //PTR_MEM_FREE(FID_OAM, stAgtOamData.pData);
            //return XERROR;�澯
        }
    }
    
    /*�������ݸ�ʽ��*/
    OAM_XmlCfgDataFmt(stAgtOamData.pData, stAgtOamData.uiMsgLen, ucRetData, &RetValLen);
    stAgtOamData.uiMsgLen = RetValLen;
    PTR_MEM_FREE(FID_OAM, stAgtOamData.pData);

    stAgtOamData.pData = (XS8*)ucRetData;

    pTempData = (XS8*)ucRetData;
    XOS_PRINT(MD(FID_OAM,PL_DBG),"OAM_SendToAPP MSG DATA![idx=%d,sessid=%d,neid=%d,pid=%d,mid=%d,\
        type=%d,tid=%d,recnum=%d,len=%d]\r\n",
        stAgtOamData.uiIndex,
        stAgtOamData.uiSessionId,
        stAgtOamData.usNeId,
        stAgtOamData.usPid,
        stAgtOamData.usModuleId,
        stAgtOamData.uiOperType,
        stAgtOamData.uiTableId,
        stAgtOamData.uiRecNum,
        stAgtOamData.uiMsgLen);

    for (uiLoop = 0; uiLoop < RetValLen; uiLoop++)
    {
        XOS_PRINT(MD(FID_OAM,PL_DBG),"%02x ",(*pTempData)&0xFF);
        pTempData++;
    }

    XOS_PRINT(MD(FID_OAM,PL_DBG),"\r\nOAM_TableIdFind[TID=%d] mid!\r\n",stAgtOamData.uiTableId);
    
    /*��ҵ��ģ�鷢������Ϣ*/
    if(XSUCC == OAM_TableIdFind(stAgtOamData.uiTableId, uiMid, &uiCount))
    {
        for(uiLoop=0; uiLoop < uiCount; uiLoop++)
        {
            if (FID_OAM == uiMid[uiLoop])/*���Ǳ��������Ϣ*/
            {
                OAM_NeProcInfoProc(stAgtOamData.pData, stAgtOamData.uiOperType, 
                                                                            stAgtOamData.uiMsgLen);
            }
            else
            {
                if (XSUCC != OAM_CfgMsgSend(uiMid[uiLoop], AGENT_CFG_MSG, 
                                      (XVOID*)&stAgtOamData, stAgtOamData.uiMsgLen))
                {
                    XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_CfgMsgSend failed\r\n");
                    return XERROR;
                }
            }
        }
    }
        
    return XSUCC;
}

/****************************************************************************
*������:OAM_AgtDataMsgProc
*����  : agent������Ϣ����(ͬ����Ӧ����������Ϣ)
*����  :
*            XVOID*pMsg    ������Ϣͷ����Ϣ��
*            XU32 uiMsgId  ��ϢID
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AgtDataMsgProc(XVOID*pMsg, XU32 uiMsgId)
{
    XS8* pucData = XNULL;
    XU32 uiLoop = 0;
    XU32 uiMsgLen = 0;
    XU32 uiDataLen = 0;
    t_XOSCOMMHEAD* pSendMsg  = XNULL;
    AGT_CFG_REQ_T* pRecvMsg = XNULL;
    AGT_OAM_CFG_REQ_T stAgtOamData = {0};
    AGT_OAM_CFG_REQ_T *pAgtOamData = XNULL;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtDataMsgProc[msgid=0x%x] function!\r\n",uiMsgId);

    pRecvMsg = (AGT_CFG_REQ_T *)pMsg;

    pucData = (XS8*)pMsg;
    pucData += (sizeof(AGT_CFG_REQ_T) - sizeof(XS8*));
    
    /*����ת���ֽ���*/
    uiDataLen = sizeof(AGT_OAM_CFG_REQ_T)- sizeof(XS8*);
    XOS_MemCpy(&stAgtOamData, &pRecvMsg->stAgtOamData, uiDataLen);
    OAM_AgtOamDataNtoH(&stAgtOamData);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtDataMsgProc[mid=%d] function!\r\n",
                                                                        stAgtOamData.usModuleId);
    /*������Ϣ����*/
    uiMsgLen = sizeof(AGT_OAM_CFG_REQ_T) + stAgtOamData.uiMsgLen;
    if (XNULL == (pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
        
    SET_XOS_MSG_HEAD(pSendMsg, uiMsgId, FID_OAM, stAgtOamData.usModuleId);

    pAgtOamData = (AGT_OAM_CFG_REQ_T*)pSendMsg->message;
    XOS_MemCpy(pAgtOamData, &stAgtOamData, uiDataLen);

    /*����pDataֵ������pData*/
    pAgtOamData->pData = (XS8*)(pAgtOamData + 1);
    XOS_MemCpy(pAgtOamData->pData, pucData, stAgtOamData.uiMsgLen);

    XOS_PRINT(MD(FID_OAM,PL_LOG),"OAM_SendSyncToAPP MSG DATA![idx=%d,sessid=%d,neid=%d,pid=%d,mid=%d,\
        type=%d,tid=%d,recnum=%d,len=%d]\r\n",
        pAgtOamData->uiIndex,
        pAgtOamData->uiSessionId,
        pAgtOamData->usNeId,
        pAgtOamData->usPid,
        pAgtOamData->usModuleId,
        pAgtOamData->uiOperType,
        pAgtOamData->uiTableId,
        pAgtOamData->uiRecNum,
        pAgtOamData->uiMsgLen);
    
    for (uiLoop = 0; uiLoop < uiMsgLen; uiLoop++)
    {
        XOS_PRINT(MD(FID_OAM,PL_LOG),"%02x ",pAgtOamData->pData[uiLoop]);
    }
    
    if(XSUCC != XOS_MsgSend(pSendMsg))
    {
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }
    
    XOS_PRINT(MD(FID_OAM,PL_LOG),"oam send sync msg to moudle[%d] success!\r\n",pAgtOamData->usModuleId);
    return XSUCC;
}


/****************************************************************************
*������:OAM_AgtRspMsgProc
*����  : agent��Ӧ��Ϣ����
*����  :
*            XVOID*pMsg    ��Ӧ��Ϣͷ����Ϣ��
*���  :
*����  :
*            XSUCC       �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/
XS32 OAM_AgtRspMsgProc(XVOID*pMsg)
{
    AGT_OAM_CFG_REQ_T* pCfgData     = XNULL;
    AGT_CFG_RSP_T*     pRecvMsg     = XNULL;
    AGT_OAM_CFG_HEAD_T stAgtCfgHead = { 0 };
    XU32               uiMsgLen     = 0;

    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtRspMsgProc function!\r\n");
    
    pRecvMsg = (AGT_CFG_RSP_T *)pMsg;
    XOS_MemCpy(&stAgtCfgHead, &pRecvMsg->stAgtCfgHead, sizeof(AGT_OAM_CFG_HEAD_T));
    stAgtCfgHead.uiMsgType = XOS_NtoHl(stAgtCfgHead.uiMsgType);

    uiMsgLen = XOS_NtoHl(pRecvMsg->stAgtOamData.uiMsgLen) + sizeof(AGT_OAM_CFG_REQ_T) - sizeof(XS8*);
    
    if(XNULL == (pCfgData = (AGT_OAM_CFG_REQ_T*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "XOS_MemMalloc failed\r\n");
        return XERROR;
    }
    /*ת������*/
    pCfgData->uiIndex     = XOS_NtoHl(pRecvMsg->stAgtOamData.uiIndex);
    pCfgData->uiSessionId = XOS_NtoHs(pRecvMsg->stAgtOamData.uiSessionId);
    pCfgData->usNeId      = XOS_NtoHs(pRecvMsg->stAgtOamData.usNeId);
    pCfgData->usPid       = XOS_NtoHs(pRecvMsg->stAgtOamData.usPid);
    pCfgData->usModuleId  = XOS_NtoHs(pRecvMsg->stAgtOamData.usModuleId);
    pCfgData->uiOperType  = XOS_NtoHl(pRecvMsg->stAgtOamData.uiOperType);
    pCfgData->uiTableId   = XOS_NtoHl(pRecvMsg->stAgtOamData.uiTableId);
    pCfgData->uiRecNum    = XOS_NtoHl(pRecvMsg->stAgtOamData.uiRecNum);
    pCfgData->uiMsgLen    = XOS_NtoHl(pRecvMsg->stAgtOamData.uiMsgLen);
    XOS_MemCpy(pCfgData->pData, pRecvMsg->stAgtOamData.pRetData, pCfgData->uiMsgLen);

    switch(pCfgData->uiOperType)
    {
    case OAM_MSGTYPE_SYNC:
        /*����������Ϣ��ҵ��ģ��*/
        if (XSUCC != OAM_CfgMsgSend(pCfgData->usModuleId, AGENT_CFG_MSG, 
                                                            (XVOID*)pCfgData, pCfgData->uiMsgLen))
        {
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_CfgMsgSend[%d] failed\r\n", pCfgData->usModuleId);
            PTR_MEM_FREE(FID_OAM,pCfgData);
            return XERROR;
        }
        break;

    default:
        XOS_PRINT(MD(FID_OAM, PL_WARN), "Unknown RspMsg:uiOperType[%d]\r\n", pCfgData->uiOperType);
        break;
    }

    PTR_MEM_FREE(FID_OAM,pCfgData);

    return XSUCC;
}


/****************************************************************************
*������:OAM_AppRspMsgProc
*����  : ����ģ��ͬ����Ӧ��Ϣ
*����  :
*            XVOID*pMsg
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AppRspMsgProc(XVOID*pMsg)
{
    XU32 uiLoop      = 0;
    XU32 uiHeadIndex = 0;
    XU32 uiSessionId = 0;
    t_XOSCOMMHEAD* pRecvMsg = XNULL;
    AGT_OAM_CFG_RSP_T *pAppRspMsg = XNULL;
    OAM_LINK_T stOamLink = { {0} };
    t_XOSCOMMHEAD* pSendMsg = XNULL;
    XS8* pRspData = XNULL;
    t_SCTPDATAREQ* pNtlHead  = XNULL;
    AGT_CFG_RSP_T* pRspMsg = XNULL;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AppRspMsgProc function!\r\n");
    
    pRecvMsg   = (t_XOSCOMMHEAD *)pMsg;
    pAppRspMsg = (AGT_OAM_CFG_RSP_T *)pRecvMsg->message;

    if (OAM_CFG_GET == pAppRspMsg->uiOperType)
    {
        if(XSUCC != OAM_LinkGet(&stOamLink))
        {
            XOS_PRINT(MD(FID_OAM, PL_ERR), "get oam link failed\r\n");
            return XERROR;
        }

        if(XNULL == (pSendMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM,sizeof(t_SCTPDATAREQ))))
        {
            XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
            return XERROR;
        }
        
        SET_XOS_MSG_HEAD(pSendMsg, eSendData, FID_OAM, FID_NTL);

        pRspData = (XS8*)XOS_MemMalloc(FID_OAM, sizeof(XU32) + sizeof(AGT_CFG_RSP_T) + pAppRspMsg->uiMsgLen);
        if(XNULL == pRspData)
        {
            XOS_MsgMemFree(FID_OAM, pSendMsg);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
            return XERROR;
        }

        XOS_MemSet(pRspData, 0, sizeof(XU32) + sizeof(AGT_CFG_RSP_T) + pAppRspMsg->uiMsgLen);
        
        /*ǰ4��byte��msgType*/
        *(XU32*)pRspData = XOS_HtoNl(AGENT_CFG_MSG);
        pRspMsg = (AGT_CFG_RSP_T*)(&pRspData[sizeof(XU32)]);

        /*��ȡ��Ӧ����Ϣͷ�������Ϣͷ*/
        uiSessionId = XOS_HtoNl(pAppRspMsg->uiSessionId);
        if(XSUCC != (OAM_HeadIdxGetBySid(uiSessionId, &uiHeadIndex)))
        {
            PTR_MEM_FREE(FID_OAM, pRspData);
            XOS_MsgMemFree(FID_OAM, pSendMsg);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"can't find msg head\r\n");
            return XERROR;
        }
        XOS_MemCpy(&pRspMsg->stAgtCfgHead, &g_AgtCfgHead[uiHeadIndex].stAgtCfgHead, 
                                                                    sizeof(AGT_OAM_CFG_HEAD_T));
        g_AgtCfgHead[uiHeadIndex].ucUsed = OAM_SESSION_HEAD_UNUSED;

        /*�޸�msgTypeΪ��Ӧ��Ϣ*/
        pRspMsg->stAgtCfgHead.uiMsgType = XOS_HtoNl(OAM_MSGTYPE_RESPONSE);

        /*�����Ϣ��*/
        pRspMsg->stAgtOamData.uiIndex     = XOS_HtoNl(pAppRspMsg->uiIndex);
        pRspMsg->stAgtOamData.uiSessionId = XOS_HtoNl(pAppRspMsg->uiSessionId);
        pRspMsg->stAgtOamData.usNeId      = XOS_HtoNs(pAppRspMsg->usNeId);
        pRspMsg->stAgtOamData.usPid       = XOS_HtoNs(pAppRspMsg->usPid);
        pRspMsg->stAgtOamData.usModuleId  = XOS_HtoNs(pAppRspMsg->usModuleId);
        pRspMsg->stAgtOamData.uiOperType  = XOS_HtoNl(pAppRspMsg->uiOperType);
        pRspMsg->stAgtOamData.uiTableId   = XOS_HtoNl(pAppRspMsg->uiTableId);
        pRspMsg->stAgtOamData.uiRecNum    = XOS_HtoNl(pAppRspMsg->uiRecNum);
        pRspMsg->stAgtOamData.uiMsgLen    = XOS_HtoNl(pAppRspMsg->uiMsgLen);
        pRspMsg->stAgtOamData.uiRetCode   = XOS_HtoNl(pAppRspMsg->uiRetCode);

        if(pAppRspMsg->uiMsgLen > 0)
        {
            pRspMsg->stAgtOamData.pRetData = pRspData + sizeof(XU32) + sizeof(AGT_CFG_RSP_T);
            XOS_MemCpy(pRspMsg->stAgtOamData.pRetData, pAppRspMsg->pRetData, pAppRspMsg->uiMsgLen);

            for (uiLoop = 0; uiLoop < pAppRspMsg->uiMsgLen; uiLoop++)
            {
                XOS_PRINT(MD(FID_OAM,PL_DBG),"%02x ", pRspMsg->stAgtOamData.pRetData[uiLoop]&0xFF);
            }
        }

        /*���dataind����*/
        pNtlHead  = (t_SCTPDATAREQ*)pSendMsg->message;
        XOS_MemSet(pNtlHead, 0, sizeof(t_SCTPDATAREQ));
        pNtlHead->linkHandle = stOamLink.hLinkHandle;
        pNtlHead->dstAddr.ip = 0;
        pNtlHead->dstAddr.port = 0;
        pNtlHead->msgLenth = sizeof(XU32) + sizeof(AGT_CFG_RSP_T) + pAppRspMsg->uiMsgLen;
        pNtlHead->pData = (XCHAR*)pRspData;
        
        if(XSUCC != XOS_MsgSend(pSendMsg))
        {
            PTR_MEM_FREE(FID_OAM, pRspData);
            XOS_MsgMemFree(FID_OAM, pSendMsg);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
            return XERROR;
        }
    }
    else
    {
        /*����Ҫ�ͷ��ڴ�,��ʱ������ҵ����Ӧ*/
    }
    
    return XSUCC;
}


/****************************************************************************
*������:OAM_AgtFmtMsgProc
*����  : ����agent�ĸ�ʽ����Ԫ������Ϣ
*����  :
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AgtFmtMsgProc()
{
    XU32 uiLoop = 0;
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtFmtMsgProc function!\r\n");

    for(uiLoop=0; uiLoop < g_TidMidCnt && uiLoop < MAX_TB_NUM; uiLoop++)
    {
        if(g_TidMidArray[uiLoop].uiTableId >0 && g_TidMidArray[uiLoop].uiModuleId > 0)
        {
            /*�ӱ���xml�ļ����ұ�ID��Ӧ���ݼ�¼������*/
            if (XSUCC != OAM_XmlTblInfoGet(g_xmlFilePath, g_TidMidArray[uiLoop].uiTableId, 
                                                    g_TidMidArray[uiLoop].uiModuleId, OAM_CFG_FMT))
            {
                XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_XmlTblInfoGet Module[%d]Table[%d] failed\r\n",
                                g_TidMidArray[uiLoop].uiModuleId, g_TidMidArray[uiLoop].uiTableId);
                return XERROR;
            }
        }
    }

    return XSUCC;
            
}


/****************************************************************************
*������:OAM_AgtSftDbgMsgProc
*����  : ����agent�������Ϣ
*����  :
*            XVOID*pMsg
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AgtSftDbgMsgProc(XVOID*pMsg)
{
    OAM_SOFTDEBUG_REQ_T *pSftDbgInfo = XNULL;
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtSftDbgMsgProc function!\r\n");
    
    PTR_NULL_CHECK(pMsg, XERROR);
    
    pSftDbgInfo = (OAM_SOFTDEBUG_REQ_T *)pMsg;

    if (XSUCC != OAM_OperMsgSend(FID_CLI, AGENT_DEBUG_REG_MSG, sizeof(OAM_SOFTDEBUG_REQ_T),
                                                                                    &pSftDbgInfo))
    {
        XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtSftDbgMsgProc function!\r\n");
        return XERROR;
    }
    
    return XSUCC;
}

/****************************************************************************
*������:OAM_PmReportMsgProc
*����  : agent������Ϣ����
*����  :
*            XVOID*pMsg    ������Ϣͷ����Ϣ��
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_PmReportMsgProc(XVOID*pMsg)
{
    XS32 siNeId = 0;
    XS32 siPid  = 0;
    XU32 uiMsgLen = 0;
    XS8* pReqData = XNULL;
    t_SCTPDATAREQ* pNtlHead = XNULL;
    t_XOSCOMMHEAD* pSendMsg = XNULL;
    PM_REPORT_T *pPmReportInfo = XNULL;
    OAM_LINK_T stOamLink = { {0} };
    OAM_PM_REPORT_T stPmData = {0};
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_PmReportMsgProc function!\r\n");
    
    PTR_NULL_CHECK(pMsg, XERROR);
    
    pPmReportInfo = (PM_REPORT_T*)pMsg;
    XOS_MemSet(&stPmData, 0, sizeof(OAM_PM_REPORT_T));

	//#ifdef XOS_LINUX
	#if 0
        if(XERROR == (siNeId = XOS_GetNeId()))
        {
            XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetNeId[%d] failed.\r\n", siNeId);
            return XERROR;
        }
        else
        {
            stPmData.usNeId = (XU16)siNeId;
        }
        
        if(XERROR == (siPid = XOS_GetLogicPid()))
        {
            XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetLogicPid[%d] failed.\r\n", siPid);
            return XERROR;
        }
        else
        {
            stPmData.usPid = (XU16)siPid;
        }
    #endif

    XOS_MemCpy(&stPmData.stPmReport, pPmReportInfo, sizeof(PM_REPORT_T));

    /*��ȡoam��·*/
    if(XSUCC != OAM_LinkGet(&stOamLink))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "get oam link failed\r\n");
        return XERROR;
    }
        
    /*������Ϣ�����ڴ�*/
    uiMsgLen = sizeof(XU32)+ sizeof(OAM_PM_REPORT_T);
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    /*ǰ4��byte��msgType*/
    *(XU32*)pReqData = XOS_HtoNl(OAM_PM_NOTIFY_MSG);

    XOS_MemCpy(pReqData, &stPmData, sizeof(OAM_PM_REPORT_T));
    
    /*������Ϣ�ڴ�*/
    if(XNULL == (pSendMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_SCTPDATAREQ))))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }

    /*��дNTL���ݷ�����·*/
    SET_XOS_MSG_HEAD(pSendMsg, eSendData, FID_OAM, FID_NTL);
    pNtlHead  = (t_SCTPDATAREQ*)pSendMsg->message;
    XOS_MemSet(pNtlHead, 0, sizeof(t_SCTPDATAREQ));
    pNtlHead->linkHandle = stOamLink.hLinkHandle;
    pNtlHead->dstAddr.ip = 0;
    pNtlHead->dstAddr.port = 0;
    pNtlHead->msgLenth = uiMsgLen;
    pNtlHead->pData = (XCHAR*)pReqData;

    if(XSUCC != XOS_MsgSend(pSendMsg))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    return XSUCC;
            
}


/****************************************************************************
*������:OAM_CfgMsgSend
*����  : �������ݷ��͵�ģ��ӿ�
*����  :
*            XU32 uiModuleId  ģ��ID
*            XU32 uiMsgId     ��Ϣ����
*            XVOID*pBuffer    �������ݵ�ַ
*            XU32 uiDataLen   pdata���ݳ���
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_CfgMsgSend(XU32 uiModuleId, XU32 uiMsgId, XVOID*pBuffer, XU32 uiDataLen)
{
    t_XOSCOMMHEAD* pSendMsg = XNULL;
    AGT_OAM_CFG_REQ_T *pTmpMsg = XNULL; 
    AGT_OAM_CFG_REQ_T *pAgtOamData = XNULL;
    XU32 uiMsgLen = 0;
    XU32 uiLoop = 0;
    XS8 *pTempData = XNULL;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pBuffer, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_CfgMsgSend[mid=%d] function!\r\n", uiModuleId);
    
    pTmpMsg = (AGT_OAM_CFG_REQ_T *)pBuffer;

    uiMsgLen = sizeof(AGT_OAM_CFG_REQ_T) + uiDataLen;
    pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, uiMsgLen);
    if (NULL == pSendMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
    
    /*��д��Ϣ����*/
    SET_XOS_MSG_HEAD(pSendMsg, (XU16)uiMsgId, FID_OAM, uiModuleId);

    /*��������pData֮�����������*/
    pSendMsg->length = uiMsgLen;
    pAgtOamData = (AGT_OAM_CFG_REQ_T*)pSendMsg->message;
    XOS_MemCpy(pAgtOamData, pTmpMsg, sizeof(AGT_OAM_CFG_REQ_T) - sizeof(XS8*));

    /*����pDataֵ������pData*/
    pAgtOamData->pData = (XS8*)(pAgtOamData + 1);
    if (uiDataLen > 0)
    {
        XOS_MemCpy(pAgtOamData->pData, pTmpMsg->pData, uiDataLen);
    }

    if (OAM_CFG_FMT == pAgtOamData->uiOperType)
    {
        pTempData = pAgtOamData->pData;
        XOS_PRINT(MD(FID_OAM,PL_DBG),"!!!!!!!oam fmt msg[tableid=%d][recnum=%d][mid=%d]!!!!!!\r\n", 
            pAgtOamData->uiTableId, pAgtOamData->uiRecNum, pAgtOamData->usModuleId);
        for (uiLoop = 0; uiLoop < uiDataLen; uiLoop++)
        {
            XOS_PRINT(MD(FID_OAM,PL_DBG),"%02x ",(*pTempData)&0xFF);
            pTempData++;
        }
    }
    
    if(XSUCC != XOS_MsgSend(pSendMsg))
    {
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        return XERROR;
    }
    
    return XSUCC;
}

/****************************************************************************
*������:OAM_AgtOamDataNtoH
*����  :AGT_OAM_CFG_REQ_T�ṹת������
*����  :
*            AGT_OAM_CFG_REQ_T* pAgtOamData
*���  :
*����  :
*˵��  :
****************************************************************************/ 
XVOID OAM_AgtOamDataNtoH(AGT_OAM_CFG_REQ_T* pAgtOamData)
{
    if(XNULL != pAgtOamData)
    {
        pAgtOamData->uiIndex     = XOS_NtoHl(pAgtOamData->uiIndex);
        pAgtOamData->uiSessionId = XOS_NtoHl(pAgtOamData->uiSessionId);
        pAgtOamData->usNeId      = XOS_NtoHs(pAgtOamData->usNeId);
        pAgtOamData->usPid       = XOS_NtoHs(pAgtOamData->usPid);       
        pAgtOamData->usModuleId  = XOS_NtoHs(pAgtOamData->usModuleId);
        pAgtOamData->uiOperType  = XOS_NtoHl(pAgtOamData->uiOperType);
        pAgtOamData->uiTableId   = XOS_NtoHl(pAgtOamData->uiTableId);
        pAgtOamData->uiRecNum    = XOS_NtoHl(pAgtOamData->uiRecNum);
        pAgtOamData->uiMsgLen    = XOS_NtoHl(pAgtOamData->uiMsgLen);
    }
}

/****************************************************************************
*������:OAM_AgtRspSend
*����  : ��agent��������Ӧ��Ϣ
*����  :
*            XVOID*pMsg ������Ϣͷ����Ϣ��
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AgtRspSend(XVOID*pMsg)
{
    XU32 uiHeadIndex = 0;
    t_SCTPDATAREQ* pNtlHead = XNULL;
    XS8*           pRspData = XNULL;
    AGT_CFG_RSP_T* pRspMsg  = XNULL;
    t_XOSCOMMHEAD* pSendMsg = XNULL;
    AGT_CFG_REQ_T* pRecvMsg = XNULL;
    OAM_LINK_T stOamLink    = { {0} };
    AGT_OAM_CFG_HEAD_T stAgtCfgHead = { 0 };
    AGT_OAM_CFG_REQ_T  stAgtOamData = { 0 };
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);
    XOS_PRINT(MD(FID_OAM,PL_ERR),"Enter OAM_AgtRspSend function!\r\n");
    pRecvMsg = (AGT_CFG_REQ_T*)pMsg;

    XOS_MemCpy(&stAgtCfgHead, &pRecvMsg->stAgtCfgHead, sizeof(AGT_OAM_CFG_HEAD_T));
    //stAgtCfgHead.uiSessionId = XOS_NtoHl(stAgtCfgHead.uiSessionId);

    XOS_MemCpy(&stAgtOamData, &pRecvMsg->stAgtOamData, sizeof(AGT_OAM_CFG_REQ_T));

    /*ת�ֽ���*/
    OAM_AgtOamDataNtoH(&stAgtOamData);

    if(OAM_CFG_ADD == stAgtOamData.uiOperType 
        || OAM_CFG_MOD == stAgtOamData.uiOperType
        || OAM_CFG_DEL == stAgtOamData.uiOperType)
    {
        pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_SCTPDATAREQ));
        if (NULL == pSendMsg)
        {
            XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
            return XERROR;
        }

        pRspData = (XS8*)XOS_MemMalloc(FID_OAM, sizeof(XU32) + sizeof(AGT_CFG_RSP_T));
        if(XNULL == pRspData)
        {
            XOS_MsgMemFree(FID_OAM, pSendMsg);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
            return XERROR;
        }
        XOS_MemSet(pRspData, 0, sizeof(XU32) + sizeof(AGT_CFG_RSP_T));

        /*ǰ4��byte��msgType*/
        *(XU32*)pRspData = XOS_HtoNl(AGENT_CFG_MSG);

        pRspMsg = (AGT_CFG_RSP_T*)(&pRspData[sizeof(XU32)]);

        /*��ȡ��Ӧ����Ϣͷ�������Ϣͷ*/
        if(XSUCC != (OAM_HeadIdxGetBySid(stAgtCfgHead.uiSessionId, &uiHeadIndex)))
        {
            PTR_MEM_FREE(FID_OAM, pRspData);
            XOS_MsgMemFree(FID_OAM, pSendMsg);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"can't find msg head\r\n");
            return XERROR;
        }
        XOS_MemCpy(&pRspMsg->stAgtCfgHead, &g_AgtCfgHead[uiHeadIndex].stAgtCfgHead, 
                                                            sizeof(AGT_OAM_CFG_HEAD_T));
        g_AgtCfgHead[uiHeadIndex].ucUsed = OAM_SESSION_HEAD_UNUSED;

        /*�޸�msgTypeΪ��Ӧ��Ϣ*/
        pRspMsg->stAgtCfgHead.uiMsgType = XOS_HtoNl(OAM_MSGTYPE_RESPONSE);

        /*�����Ϣ��*/
        pRspMsg->stAgtOamData.uiIndex     = XOS_HtoNl(stAgtOamData.uiIndex);
        pRspMsg->stAgtOamData.uiSessionId = XOS_HtoNl(stAgtOamData.uiSessionId);
        pRspMsg->stAgtOamData.usNeId      = XOS_HtoNs(stAgtOamData.usNeId);
        pRspMsg->stAgtOamData.usPid       = XOS_HtoNs(stAgtOamData.usPid);
        pRspMsg->stAgtOamData.usModuleId  = XOS_HtoNs(stAgtOamData.usModuleId);
        pRspMsg->stAgtOamData.uiOperType  = XOS_HtoNl(stAgtOamData.uiOperType);
        pRspMsg->stAgtOamData.uiTableId   = XOS_HtoNl(stAgtOamData.uiTableId);
        pRspMsg->stAgtOamData.uiRecNum    = XOS_HtoNl(stAgtOamData.uiRecNum);
        pRspMsg->stAgtOamData.uiRetCode   = XOS_HtoNl(XSUCC);

        /*��д��Ϣ����*/
        SET_XOS_MSG_HEAD(pSendMsg, eSendData, FID_OAM, FID_NTL);

        /*��ȡoam��·*/
        if(XSUCC != OAM_LinkGet(&stOamLink))
        {
            PTR_MEM_FREE(FID_OAM, pRspData);
            XOS_MsgMemFree(FID_OAM, pSendMsg);
            XOS_PRINT(MD(FID_OAM, PL_ERR), "get oam link failed\r\n");
            return XERROR;
        }

        pNtlHead  = (t_SCTPDATAREQ*)pSendMsg->message;
        XOS_MemSet(pNtlHead, 0, sizeof(t_SCTPDATAREQ));
        pNtlHead->linkHandle = stOamLink.hLinkHandle;
        pNtlHead->dstAddr.ip = 0;
        pNtlHead->dstAddr.port = 0;
        pNtlHead->msgLenth = sizeof(XU32) + sizeof(AGT_CFG_RSP_T);
        pNtlHead->pData = (XCHAR*)pRspData;

        if(XSUCC != XOS_MsgSend(pSendMsg))
        {
            PTR_MEM_FREE(FID_OAM, pRspData);
            XOS_MsgMemFree(FID_OAM, pSendMsg);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
            return XERROR;
        }
    }    
    
    return XSUCC;
}


/****************************************************************************
*������:OAM_OperMsgSend
*����  : �������ݷ��͵�ģ��ӿ�
*����  :
*            XU32 uiModuleId  ģ��ID
*            XU32 uiMsgLen    ��Ϣ����
*            XVOID*pBuffer    �������ݵ�ַ
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_OperMsgSend(XU32 uiModuleId, XU32 uiMsgId, XU32 uiMsgLen, XVOID*pBuffer)
{
    t_XOSCOMMHEAD* pSendMsg = XNULL;

    /*ָ����μ��*/
    PTR_NULL_CHECK(pBuffer, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_OperMsgSend[mid=%d] function!\r\n", uiModuleId);
    
    pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, uiMsgLen);
    if (NULL == pSendMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
    
    /*��д��Ϣ����*/
    SET_XOS_MSG_HEAD(pSendMsg, (XU16)uiMsgId, FID_OAM, uiModuleId);
    XOS_MemCpy(pSendMsg->message,pBuffer,uiMsgLen);
    pSendMsg->length = uiMsgLen;
    
    if(XSUCC != XOS_MsgSend(pSendMsg))
    {
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        return XERROR;
    }
    
    return XSUCC;
}


/****************************************************************************
*������:OAM_TransMsgSend
*����  : OAM͸��agent��BM��Ϣ���ͽӿ�
*����  :
*            XVOID* pMsg    ͸����Ϣ
*            XU32 uiMsgId   ��ϢID
*            XU32 uiDstFid  Ŀ��FID
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_TransMsgSend(XVOID* pMsg, XU32 uiMsgId, XU32 uiDstFid)
{
    XU32 uiLoop = 0;
    XU32 uiMsgLen = 0;
    XU32 uiDataLen = 0;
    t_XOSCOMMHEAD* pSendMsg  = XNULL;
    AGT_CFG_REQ_T *pRecvData = XNULL;
    XS8 *pucBuf = NULL;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_OperMsgSend[mid=%d,msgid=%d] function!\r\n", uiDstFid,
        uiMsgId);

    pRecvData = (AGT_CFG_REQ_T *)pMsg;
    pucBuf = (XS8*)pMsg;

    uiDataLen = XOS_NtoHl(pRecvData->stAgtOamData.uiMsgLen);
    /*������Ϣ����*/
    uiMsgLen = sizeof(AGT_OAM_CFG_REQ_T) + uiDataLen;
    
    if (NULL == (pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
        
    SET_XOS_MSG_HEAD(pSendMsg, uiMsgId, FID_OAM, uiDstFid);

    pucBuf += sizeof(AGT_CFG_REQ_T);
    for (uiLoop = 0; uiLoop < uiDataLen; uiLoop++)
    {
        XOS_PRINT(MD(FID_OAM,PL_DBG),"%02x ",pucBuf[uiLoop]);
    }
    XOS_MemCpy(pSendMsg->message, pucBuf, uiDataLen);
    pSendMsg->length = uiDataLen;

    if(XSUCC != XOS_MsgSend(pSendMsg))
    {
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }
                
    return XSUCC;
}


/****************************************************************************
*������:OAM_AgtReqSend
*����  : OAM����BM�����ϱ���Ϣ��agent
*����  :
*            XVOID* pMsg    BM�����ϱ���Ϣ
*            XU32 uiMsgType
*            XU32 uiModuleId ģ��ID
*            XU32 uiTableId  ��ID
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AgtReqSend(XVOID* pMsg, XU32 uiMsgType, XU32 uiModuleId, XU32 uiTableId)
{
    t_XOSCOMMHEAD* pRecvMsg = XNULL;
    XS8* pReqData = XNULL;
    AGT_CFG_REQ_T stReqMsg = { {0} };
    XU32 uiMsgLen = 0;

    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtReqSend[type=%d] function!\r\n", uiMsgType);
    
    pRecvMsg = (t_XOSCOMMHEAD *)pMsg;

    uiMsgLen = sizeof(XU32) + sizeof(AGT_CFG_REQ_T) + pRecvMsg->length;

    /*������Ϣ�����ڴ�*/
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    /*ǰ4��byte��msgType*/
    if(MSG_REQ_PROC_OPER == uiMsgType || MSG_QUE_BOARD_HARDINFO == uiMsgType)
    {
        *(XU32*)pReqData = XOS_HtoNl(uiMsgType);
    }
    else
    {
        *(XU32*)pReqData = XOS_HtoNl(OAM_MSGTYPE_NOTIFY);
    }
    
    /*�����Ϣͷ*/
    stReqMsg.stAgtCfgHead.uiMsgId   = XOS_HtoNl(pRecvMsg->msgID);
    stReqMsg.stAgtCfgHead.uiMsgType = XOS_HtoNl(uiMsgType);

    /*�����Ϣ��, ֻ���ϱ���Ϣ��sync��Ϣ����*/
    if(MSG_REQ_PROC_OPER != uiMsgType)
    {    
        stReqMsg.stAgtOamData.usModuleId = XOS_HtoNs(uiModuleId);
        stReqMsg.stAgtOamData.uiOperType = XOS_HtoNl(uiMsgType);
        stReqMsg.stAgtOamData.uiMsgLen   = XOS_HtoNl(pRecvMsg->length);
        
        XOS_MemCpy(pReqData + sizeof(XU32), &stReqMsg, sizeof(AGT_CFG_REQ_T));
        
        /*��������������Ϣ������*/
        if (pRecvMsg->length > 0)
        {
            XOS_MemCpy(pReqData + sizeof(XU32) + sizeof(AGT_CFG_REQ_T), 
                                                        pRecvMsg->message, pRecvMsg->length);
        }
        else
        {
            XOS_PRINT(MD(FID_OAM,PL_ERR),"bm report message[len=%d] is null!\r\n",pRecvMsg->length);
            return XERROR;
        }
    }
    else
    {
        XOS_MemCpy(pReqData + sizeof(XU32), &stReqMsg.stAgtCfgHead, sizeof(AGT_OAM_CFG_HEAD_T));
    }

    /*������Ϣ*/
    if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    return XSUCC;
}


/****************************************************************************
*������:OAM_AgtSyncReqSend
*����  : OAM��SYNC��Ϣ��agent
*����  :
*            XVOID* pMsg    BM�����ϱ���Ϣ
*            XU32 uiMsgType
*            XU32 uiModuleId ģ��ID
*            XU32 uiTableId  ��ID
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AgtSyncReqSend(XU32 uiMsgType, XU32 uiModuleId, XU32 uiTableId)
{
    XS8* pReqData = XNULL;
    AGT_CFG_REQ_T stReqMsg = { {0} };
    XU32 uiMsgLen = 0;
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtSyncReqSend function!\r\n");

    uiMsgLen = sizeof(XU32) + sizeof(AGT_CFG_REQ_T);
    
    /*������Ϣ�����ڴ�*/
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    *(XU32*)pReqData = XOS_HtoNl(uiMsgType);
    
    /*�����Ϣͷ*/
    stReqMsg.stAgtCfgHead.uiMsgType = XOS_HtoNl(uiMsgType);

    stReqMsg.stAgtOamData.usModuleId = XOS_HtoNs(uiModuleId);
    stReqMsg.stAgtOamData.uiOperType = XOS_HtoNl(OAM_CFG_SYNC);
    
    /*ͨ��ƽ̨�ӿڻ�ȡ��ԪID*/
	//#ifdef XOS_LINUX
#if 0
    if(XERROR == (stReqMsg.stAgtOamData.usNeId = XOS_GetNeId()))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM, PL_ERR), "XOS_GetNeId failed.\r\n");
        return XERROR;
    }

    if(XERROR == (stReqMsg.stAgtOamData.usPid = XOS_GetLogicPid()))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM, PL_ERR), "XOS_GetLogicPid failed.\r\n");
        return XERROR;
    }
#endif
    stReqMsg.stAgtOamData.usNeId = XOS_HtoNs(stReqMsg.stAgtOamData.usNeId);
    stReqMsg.stAgtOamData.usPid = XOS_HtoNs(stReqMsg.stAgtOamData.usPid);
    stReqMsg.stAgtOamData.uiTableId = XOS_HtoNl(uiTableId);

    XOS_MemCpy(pReqData + sizeof(XU32), &stReqMsg, sizeof(AGT_CFG_REQ_T));

    /*������Ϣ*/
    if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    return XSUCC;
}


/****************************************************************************
*������:OAM_HeadIdxGetBySid
*����  :����SessionId���Ҷ�Ӧ��head���ƿ�����
*����  :
*            XU32 uiSessionId
*���  :
*            XU32* puiIndex
*����  :
*            XSUCC       �ɹ�
*            XERROR      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_HeadIdxGetBySid(XU32 uiSessionId, XU32* puiIndex)
{
    XU32 uiIndex = 0;
    for(uiIndex=0; OAM_SESSION_MAX_NUM>uiIndex; uiIndex++)
    {
        if(g_AgtCfgHead[uiIndex].ucUsed == 1 
            && g_AgtCfgHead[uiIndex].stAgtCfgHead.uiSessionId == uiSessionId)
        {
            *puiIndex = uiIndex;
            return XSUCC;
        }
    }

    return XERROR;
}


/****************************************************************************
*������:OAM_HeadIndexGet
*����  :����δʹ�õ�head���ƿ�����
*����  :
*���  :
*            XU32* puiIndex
*����  :
*            XSUCC       �ɹ�
*            XERROR      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_HeadIndexGet(XU32* puiIndex)
{
    XU32 uiIndex = 0;
    for(uiIndex=0; OAM_SESSION_MAX_NUM>uiIndex; uiIndex++)
    {
        if(g_AgtCfgHead[uiIndex].ucUsed == 0)
        {
            *puiIndex = uiIndex;
            return XSUCC;
        }
    }

    return XERROR;
}

/****************************************************************************
*������:OAM_TsNavMsgProc
*����  : TS��������Ϣ����
*����  :
*            XVOID*pMsg    ������Ϣͷ����Ϣ��
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_TsNavMsgProc(XVOID*pMsg)
{
    XU32 uiLoop  = 0;
    XS8* pRecvData = XNULL;/*�������ݵ�ַ*/
    AGT_CFG_REQ_T* pRecvMsg = XNULL;
    t_XOSCOMMHEAD* pSendMsg  = XNULL;
    AGT_OAM_CFG_REQ_T stAgtOamData = { 0 };
    TS_CFG_REQ_T* pReq = XNULL;
    TS_NAV_INFO* pucData = XNULL;
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_TsNavMsgProc function!\r\n");
    
    pRecvMsg = (AGT_CFG_REQ_T *)pMsg;
    pRecvData = (XS8*)pMsg;
    pRecvData += sizeof(AGT_CFG_REQ_T);
    
    /*ת�ֽ���*/
    XOS_MemCpy(&stAgtOamData, &pRecvMsg->stAgtOamData, sizeof(AGT_OAM_CFG_REQ_T) - sizeof(XS8*));
    stAgtOamData.uiRecNum = XOS_NtoHl(stAgtOamData.uiRecNum);
    stAgtOamData.uiMsgLen = XOS_NtoHl(stAgtOamData.uiMsgLen);

    /*��ӡ��������*/
    for(uiLoop=0; uiLoop<stAgtOamData.uiRecNum; uiLoop++)
    {
        pucData = (TS_NAV_INFO*)(pRecvData + (uiLoop*sizeof(TS_NAV_INFO)));
        XOS_PRINT(MD(FID_OAM,PL_LOG),"RecNum=%d------------\r\n", uiLoop+1);
        {
            XOS_PRINT(MD(FID_OAM,PL_LOG),"nt=%d,neid=%d,desclen=%d,desc=%s,pid=%d,ptype=%d,slot=%d\r\n", 
                pucData->usNeType, pucData->usNeId, pucData->usDescLen, pucData->ucDesc, 
                pucData->usLogPid, pucData->usModuleType, pucData->usSlotId);
        }
    }
    
    /*��TSģ�鷢������Ϣ*/
    if (NULL == (pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(TS_CFG_REQ_T))))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
        
    SET_XOS_MSG_HEAD(pSendMsg, OAM_MSG_NAV_INFO, FID_OAM,  FID_TS);

    pReq = (TS_CFG_REQ_T*)pSendMsg->message;
    pReq->uiPidNum = stAgtOamData.uiRecNum;
    pReq->uiLen = stAgtOamData.uiMsgLen;

    if(XNULL == (pReq->pNavData = (TS_NAV_INFO*)XOS_MemMalloc(FID_OAM, pReq->uiLen)))
    {
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc pReq->pNavData[%d] failed!\r\n",pReq->uiLen);
        return XERROR;
    }

    /*��������*/
    XOS_MemCpy(pReq->pNavData, pRecvData, pReq->uiLen);

    if(XSUCC != XOS_MsgSend(pSendMsg))
    {
        PTR_MEM_FREE(FID_OAM, pReq->pNavData);
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }
    
    return XSUCC;
}


/****************************************************************************
*������:OAM_MsgIdSend
*����  : ֪ͨ��Ϣ����
*����  :
*            XU32 uiModuleId  ģ��ID
*            XU32 uiMsgId     ��ϢID
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_MsgIdSend(XU32 uiModuleId, XU32 uiMsgId)
{
    t_XOSCOMMHEAD* pSendMsg = XNULL;

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_TsNavMsgProc[mid=%d,msgid=%d] function!\r\n",
        uiModuleId, uiMsgId);
    
    /*ָ����μ��*/    
    pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_XOSCOMMHEAD));
    if (NULL == pSendMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
        
    /*��д��Ϣ����*/
    SET_XOS_MSG_HEAD(pSendMsg, (XU16)uiMsgId, FID_OAM, uiModuleId);
    
    if(XSUCC != XOS_MsgSend(pSendMsg))
    {
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        return XERROR;
    }
    
    return XSUCC;
}


/****************************************************************************
*������:OAM_TableIdFind
*����  : ͨ����ID����ģ���б�
*����  :
*            XU32 uiTableId  ��ID
*���  :
*            XU32* pModList
*            XU32* pCount
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/
XS32 OAM_TableIdFind(XU32 uiTableId, XU32* pModList,XU32* pCount)
{
    XU32 uiCnt = 0;
    XU32 uiLoop = 0;

    if(XNULL == pModList || XNULL == pCount)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"null ptr !!\r\n");
        return XERROR;
    }

    for(uiLoop=0; uiLoop<g_TidMidCnt; uiLoop++)
    {
        if(g_TidMidArray[uiLoop].uiTableId == uiTableId)
        {
            pModList[uiCnt++] = g_TidMidArray[uiLoop].uiModuleId;
        }
    }

    *pCount = uiCnt;

    if(0 == uiCnt)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"no mid find !!\r\n");
        return XERROR;
    }

    return XSUCC;
}


/****************************************************************************
*������:OAM_TableIdAdd
*����  : ���ӱ�ID��ģ���Ӧ��ϵ��¼
*����  :
*            XU32 uiTableId  ��ID
*            XU32 uiModuleId ģ��
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/
XS32 OAM_TableIdAdd(XU32 uiTableId, XU32 uiModuleId)
{
    if(MAX_TB_NUM - 1 <= g_TidMidCnt)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"tid is relate PID excess[%d]!!!\r\n",g_TidMidCnt);
        return XERROR;
    }

    g_TidMidArray[g_TidMidCnt].uiTableId  = uiTableId;
    g_TidMidArray[g_TidMidCnt].uiModuleId = uiModuleId;

    g_TidMidCnt++;

    return XSUCC;
}


XS32 OAM_AsyncTableIdAdd(XU32 uiTableId, XU32 uiModuleId)
{
    if(MAX_TB_NUM - 1 <= g_AsyncTidMidCnt)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"tid is relate PID excess[%d]!!!\r\n",g_AsyncTidMidCnt);
        return XERROR;
    }

    g_AsyncTidMidArray[g_AsyncTidMidCnt].uiTableId  = uiTableId;
    g_AsyncTidMidArray[g_AsyncTidMidCnt].uiModuleId = uiModuleId;

    g_AsyncTidMidCnt++;

    return XSUCC;
}


/**********************************************************************************
������:OAM_GetComPatcCurTime
����     :��ȡ"yyyymmddHHMMSS"��ʽ�ĵ�ǰʱ��
����     :
                XCHAR* dst,
                XU32 dstLen
���     :
                XCHAR* dst,
����     :
          XVOID
˵��     :
****************************************************************************************/
XVOID OAM_GetComPatcCurTime(XS8* pDst, XU32 uiDstLen)
{
    t_XOSTD stTime = {0};

    if(XNULL == pDst)
    {
        return;
    }
    
    XOS_GetTmTime(&stTime);

    XOS_Sprintf(pDst, uiDstLen, "%04d%02d%02d%02d%02d%02d",
        stTime.dt_year + 1900, stTime.dt_mon + 1, stTime.dt_mday,
        stTime.dt_hour, stTime.dt_min, stTime.dt_sec);
    return;
}


/****************************************************************************
*������:OAM_AlarmLocInfoGet
*����  : �澯λ����Ϣ��ȡ
*����  :
*���  :
*            XVOID*pMsg    λ����Ϣ
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AlarmLocInfoGet(XVOID*pMsg)
{
    XS32 siNeId = 0;
    XS32 siPid  = 0;
    XS32 siFrameId = 0;
    XS32 siSlotId = 0;
    FM_ALARM_T *pFmAlarm = XNULL;
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AlarmLocInfoGet function!\r\n");
    
    PTR_NULL_CHECK(pMsg, XERROR);

    pFmAlarm = (FM_ALARM_T *)pMsg;
    
    pFmAlarm->stFmLocInfo.usRackId = 1;
	//#ifdef XOS_LINUX
#if 0
    if(XERROR == (siFrameId = XOS_GetShelfNum()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetShelfID[%d] failed.\r\n", siFrameId);
        return XERROR;
    }
    else
    {
        pFmAlarm->stFmLocInfo.usFrameId = (XU16)siFrameId;
    }
    
    if(XERROR == (siSlotId  = XOS_GetSlotNum()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetSlotNum[%d] failed.\r\n", siSlotId);
        return XERROR;
    }
    else
    {
        pFmAlarm->stFmLocInfo.usSlotId = (XU16)siSlotId;
    }
    
    if(XERROR == (siNeId = XOS_GetNeId()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetNeId[%d] failed.\r\n", siNeId);
        return XERROR;
    }
    else
    {
        pFmAlarm->stFmLocInfo.usNeId = (XU16)siNeId;
    }
    
    if(XERROR == (siPid = XOS_GetLogicPid()))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "XOS_GetLogicPid[%d] failed.\r\n", siPid);
        return XERROR;
    }
    else
    {
        pFmAlarm->stFmLocInfo.usProcId = (XU16)siPid;
    }
#endif

    return XSUCC;

}


/****************************************************************************
*������:OAM_AlarmSend
*����  : �澯�ϱ�
*����  :
*            XVOID*pMsg    ��Ԫ�澯������Ϣ
*            XU32 uiAlarmClass �澯��� 0:���ϸ澯 1:�¼��澯
*            XU32 uiAlarmFlag  �澯��־ 0:�澯�ָ� 1:�澯�ϱ�
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AlarmSend(XVOID* pMsg, XU32 uiAlarmClass, XU32 uiAlarmFlag)
{
    XU32 uiMsgLen = 0;
    XS8* pReqData = XNULL;
    XS8 ucCurTime[OAM_SEND_MAX_NUM] = {0};
    FM_ALARM_T stFmAlarm;
    FM_PARA_T *pFmParaInfo = XNULL;
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AlarmSend function!\r\n");
    
    PTR_NULL_CHECK(pMsg, XERROR);
    pFmParaInfo = (FM_PARA_T *)pMsg;

    XOS_MemSet(&stFmAlarm, 0, sizeof(FM_ALARM_T));
    
    /*��ȡ��ǰ�澯����ʱ��*/
    OAM_GetComPatcCurTime(ucCurTime, sizeof(ucCurTime));
    XOS_StrToLongNum(ucCurTime, &stFmAlarm.ulAlarmTime);

    stFmAlarm.uiAlarmClass = uiAlarmClass;
    stFmAlarm.ucAlarmFlag  = uiAlarmFlag;
    
    /*��ȡ�澯λ����Ϣ*/
    if(XSUCC != OAM_AlarmLocInfoGet(&stFmAlarm))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_AlarmLocInfoGet failed\r\n");
        return XERROR;
    }

    /*�����澯��λ��Ϣ*/
    XOS_MemCpy(&stFmAlarm.stFmPara, pFmParaInfo, sizeof(FM_PARA_T));
        
    /*������Ϣ�����ڴ�*/
    uiMsgLen = sizeof(XU32)+ sizeof(FM_ALARM_T);
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    /*ǰ4��byte��msgType*/
    *(XU32*)pReqData = XOS_HtoNl(OAM_FM_NOTIFY_MSG);

    XOS_MemCpy(pReqData + sizeof(XU32), &stFmAlarm, sizeof(FM_ALARM_T));
    
    /*������Ϣ*/
    if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
    {
        /*�澯����*/
        OAM_AlarmMsgSave(&stFmAlarm);
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    
    return XSUCC;
            
}


/****************************************************************************
*������:OAM_AgtReqSend
*����  : OAM����BM�����ϱ���Ϣ��agent
*����  :
*            XVOID* pMsg    ���͸�agent����Ϣ
*            XU32 uiMsgLen  ��Ϣ����
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AgtMsgSend(XVOID* pMsg, XU32 uiMsgLen)
{
    t_XOSCOMMHEAD* pSendMsg = XNULL;
    t_SCTPDATAREQ* pNtlHead = XNULL;
    XS8* pReqData = XNULL;
    OAM_LINK_T stOamLink = { {0} };
    
    /*ָ����μ��*/
    PTR_NULL_CHECK(pMsg, XERROR);
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtMsgSend function!\r\n");
    
    pReqData = (XS8 *)pMsg;

    /*��ȡoam��·*/
    if(XSUCC != OAM_LinkGet(&stOamLink))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "get oam link failed\r\n");
        return XERROR;
    }
    
    /*������Ϣ�ڴ�*/
    pSendMsg=(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_SCTPDATAREQ));
    if(XNULL == pSendMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }

    /*��дNTL���ݷ�����·*/
    SET_XOS_MSG_HEAD(pSendMsg, eSendData, FID_OAM, FID_NTL);
    pNtlHead  = (t_SCTPDATAREQ*)pSendMsg->message;
    XOS_MemSet(pNtlHead, 0, sizeof(t_SCTPDATAREQ));
    pNtlHead->linkHandle = stOamLink.hLinkHandle;
    pNtlHead->dstAddr.ip = 0;
    pNtlHead->dstAddr.port = 0;
    pNtlHead->msgLenth = uiMsgLen;
    pNtlHead->pData = (XCHAR*)pReqData;

    if(XSUCC != XOS_MsgSend(pSendMsg))
    {
        XOS_MsgMemFree(FID_OAM, pSendMsg);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    return XSUCC;
}


/****************************************************************************
*������:OAM_AlarmRepeatSend
*����  : ����澯�ϱ�
*����  :
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_AlarmRepeatSend()
{
    XU32 uiLoop = 0;
    XU32 uiMsgLen = 0;
    XU16 usHeadPos = 0;
    XU32 uiTypeIndex = 0;
    XS8* pReqData = XNULL;
    FM_ALARM_T stFmAlarmInfo = {0};
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AlarmRepeatSend function!\r\n");
    
    XOS_MemSet(&stFmAlarmInfo, 0, sizeof(FM_ALARM_T));

    
    for (uiTypeIndex = 0; uiTypeIndex < FM_TYPE_NUM; uiTypeIndex++)
    {
        uiLoop = 0;
        usHeadPos = g_AlarmMsgSave[uiTypeIndex].usHead;
        
        while((g_AlarmMsgSave[uiTypeIndex].usHead != g_AlarmMsgSave[uiTypeIndex].usTail)
                && (uiLoop < FM_MSG_SAVE_NUM))
        {
            XOS_MemCpy(&stFmAlarmInfo, &g_AlarmMsgSave[uiTypeIndex].stFmAlarm[usHeadPos], 
                                                                                sizeof(FM_ALARM_T));

            /*������Ϣ�����ڴ�*/
            uiMsgLen = sizeof(XU32)+ sizeof(FM_ALARM_T);
            if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
            {
                XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
                return XERROR;
            }

            XOS_MemSet(pReqData, 0, uiMsgLen);

            /*ǰ4��byte��msgType*/
            *(XU32*)pReqData = XOS_HtoNl(OAM_FM_NOTIFY_MSG);

            XOS_MemCpy(pReqData, &stFmAlarmInfo, sizeof(FM_ALARM_T));
            
            /*������Ϣ*/
            if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
            {
                PTR_MEM_FREE(FID_OAM, pReqData);
                XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
                return XERROR;
            }

            /*��������һ��*/
            g_AlarmMsgSave[uiTypeIndex].usHead++;
            g_AlarmMsgSave[uiTypeIndex].usHead %= FM_MSG_SAVE_NUM;
            uiLoop++;
        }
    }
    
    return XSUCC;
}


/****************************************************************************
*������:OAM_AlarmMsgSave
*����  : �澯����
*����  :
*            XVOID*pMsg    ��Ԫ�澯������Ϣ
*            XU32 uiAlarmClass �澯��� 0:���ϸ澯 1:�¼��澯
*            XU32 uiAlarmFlag  �澯��־ 0:�澯�ָ� 1:�澯�ϱ�
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XVOID OAM_AlarmMsgSave(XVOID* pMsg)
{
    XU32 uiIndex = 0;
    FM_ALARM_T *pFmAlarm = XNULL;
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AlarmMsgSave function!\r\n");
    
    if (XNULL == pMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"pMsg IS NULL!\r\n");
        return;
    }
    
    pFmAlarm = (FM_ALARM_T *)pMsg;

    if (FM_CLASS_ALARM == pFmAlarm->uiAlarmClass)/*���ϸ澯����*/
    {
        uiIndex = 0;
    }
    else /*�¼��澯����*/
    {
        uiIndex = 1;
    }

    /*��Ϣ���浽��Ӧ��Ϣ������*/
    XOS_MemSet(&g_AlarmMsgSave[uiIndex].stFmAlarm[g_AlarmMsgSave[uiIndex].usTail%FM_MSG_SAVE_NUM], 
                                                                        0, sizeof(FM_ALARM_T));
    XOS_MemCpy(&g_AlarmMsgSave[uiIndex].stFmAlarm[g_AlarmMsgSave[uiIndex].usTail%FM_MSG_SAVE_NUM], 
                                                            pFmAlarm, sizeof(FM_ALARM_T));
    g_AlarmMsgSave[uiIndex].usTail++;
    g_AlarmMsgSave[uiIndex].usTail %= FM_MSG_SAVE_NUM;
    if (g_AlarmMsgSave[uiIndex].usHead == g_AlarmMsgSave[uiIndex].usTail)
    {
        g_AlarmMsgSave[uiIndex].usHead++;
        g_AlarmMsgSave[uiIndex].usHead %= FM_MSG_SAVE_NUM;
    }

    return;
}


/****************************************************************************
*������:OAM_NeProcInfoProc
*����  :������Ԫ������Ϣ
*����  :
*            XVOID* pMsg ������Ϣ
*            XU32 uiType ��������-OAM_CFG_E
*            XU32 uiMsgLen ��Ϣ����
*���  :
*����  :
*            XSUCC       �ɹ�
*            XERROR      ʧ��
*˵��  :
****************************************************************************/ 
XVOID OAM_NeProcInfoProc(XVOID* pMsg, XU32 uiType, XU32 uiMsgLen)
{
    XU32 uiIndex = 0;
    XU32 uiRecNum = 0;
    MODULE_MANAGE_T *pModuleInfo = XNULL;
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_NeProcInfoProc function!\r\n");
    
    /*ָ����μ��*/
    if (XNULL == pMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"msg ptr is null!!\r\n");
        return;
    }

    /*��¼�����ж�*/
    if (sizeof(MODULE_MANAGE_T) != uiMsgLen)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"input msglen[%d] diff struct len[%d]!!\r\n", 
                                                              uiMsgLen, sizeof(MODULE_MANAGE_T));
        return;
    }
    
    pModuleInfo = (MODULE_MANAGE_T *)pMsg;

    switch (uiType)
    {
        case OAM_CFG_ADD:/*ֱ�����Ӽ�¼*/
        {
            XOS_MemCpy(&g_NeProcInfo.stMduInfo[g_NeProcInfo.uiProcNum], pModuleInfo, uiMsgLen);
            g_NeProcInfo.uiProcNum++;
        }
        break;
            
        case OAM_CFG_MOD:/*ͨ���ڴ�Ƚ��ҵ����޸�*/
        {
            for (uiIndex = 0; uiIndex< g_NeProcInfo.uiProcNum; uiIndex++)
            {
                if (g_NeProcInfo.stMduInfo[uiIndex].usNeId == pModuleInfo->usNeId
                    && g_NeProcInfo.stMduInfo[uiIndex].usProcId == pModuleInfo->usProcId)
                {
                    XOS_MemSet(&g_NeProcInfo.stMduInfo[uiIndex], 0, sizeof(MODULE_MANAGE_T));
                    XOS_MemCpy(&g_NeProcInfo.stMduInfo[uiIndex], pModuleInfo, uiMsgLen);
                }
            }
        }
        break;
            
        case OAM_CFG_DEL:/*ɾ��������ԪID�ͽ���IDƥ������ڴ��¼*/
        {
            for (uiIndex = 0; uiIndex< g_NeProcInfo.uiProcNum; uiIndex++)
            {
                if (g_NeProcInfo.stMduInfo[uiIndex].usNeId == pModuleInfo->usNeId
                    && g_NeProcInfo.stMduInfo[uiIndex].usProcId == pModuleInfo->usProcId)
                {
                    XOS_MemSet(&g_NeProcInfo.stMduInfo[uiIndex], 0, sizeof(MODULE_MANAGE_T));
                    g_NeProcInfo.uiProcNum--;
                }
            }
        }
        break;
            
        case OAM_CFG_SYNC:/*ͬ�����߸�ʽ������ֱ��ȫ���Ǽ�¼��Ϣ*/
        {
            XOS_MemSet(&g_NeProcInfo, 0, sizeof(g_NeProcInfo));
            XOS_MemCpy(&g_NeProcInfo.stMduInfo[g_NeProcInfo.uiProcNum], pModuleInfo, uiMsgLen);
            g_NeProcInfo.uiProcNum++;
        }
        break;
        
        case OAM_CFG_FMT:/*��ʽ������*/
        {
            uiRecNum = uiMsgLen/sizeof(MODULE_MANAGE_T);
            XOS_MemSet(&g_NeProcInfo, 0, sizeof(g_NeProcInfo));
            for (uiIndex = 0; uiIndex < uiRecNum; uiIndex++)
            {
                XOS_MemCpy(&g_NeProcInfo.stMduInfo[uiIndex], pModuleInfo, sizeof(MODULE_MANAGE_T));
                g_NeProcInfo.uiProcNum++;
                pModuleInfo += sizeof(MODULE_MANAGE_T);
            }
        }
        break;
            
        default:
            break;
    }

    return;
}

/****************************************************************************
*������:OAM_HaIpInfoGet
*����  :��ȡHA��ip��Ϣ
*����  :
*���  :
*            XU16 usSlotId  ��λ��
*����  :
*            XSUCC       �ɹ�
*            XERROR      ʧ��
*˵��  :
****************************************************************************/ 
XVOID OAM_HaInfoGet(OAM_HAINFO_T *pHaInfo)
{
    XU16 usPort = 0;
    XU32 uiLoop = 0;
    XU32 uiIdx  = 0;
    XCHAR ucIpStr[MAX_IPSTR_LEN+1] = {0};

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_HaInfoGet function!\r\n");

    /*���ݹ������˿�*/
    if(0 == XOS_StrNcmp(g_ProcessType, "agt", XOS_StrLen("agt")))
    {
        usPort = AGT_PORT;
    }
    else if (!XOS_StrNcmp(g_ProcessType, "bm", XOS_StrLen("bm"))) 
    {
        usPort = OAM_BM_PORT;
    }
    else if(!XOS_StrNcmp(g_ProcessType, "ts", XOS_StrLen("ts")))
    {
        usPort = OAM_TS_PORT;
    }
    else
    {
        usPort = NE_PORT_BASE + (g_OamLinkCB.usNeId << NE_PORT_NEID_OFFSET) + 
                                                        (g_OamLinkCB.usPid << NE_PORT_PID_OFFSET);
    }
    
    /*��Ŵ�1��ʼ, ip��0��ʼ:uiFrameId - 1*/
    if (1 == g_NeProcInfo.uiProcNum) /*����������ߵ���*/
    {
        for(uiIdx = 0; uiIdx < MAX_LINK_NUM; uiIdx++)
        {
            if(g_NeProcInfo.stMduInfo[0].usFrameId > 1)
            {
                XOS_Sprintf(ucIpStr, MAX_IPSTR_LEN, "172.16.%d%d.%d", 
                                                    g_NeProcInfo.stMduInfo[0].usFrameId - 1, 
                                                uiIdx, g_NeProcInfo.stMduInfo[uiLoop].usMSlotId);
            }
            else
            {
                XOS_Sprintf(ucIpStr, MAX_IPSTR_LEN, "172.16.%d.%d", uiIdx, 
                                                        g_NeProcInfo.stMduInfo[0].usMSlotId);
            }
            
            XOS_StrtoIp(ucIpStr, &pHaInfo[uiIdx].stIpAddr[0].ip);
            
            pHaInfo[uiIdx].stIpAddr[0].port = usPort;

            if(g_NeProcInfo.stMduInfo[0].usSSlotId > 0)
            {
                if(g_NeProcInfo.stMduInfo[0].usFrameId > 1)
                {
                    XOS_Sprintf(ucIpStr, MAX_IPSTR_LEN, "172.16.%d%d.%d", 
                        g_NeProcInfo.stMduInfo[0].usFrameId - 1, uiIdx, 
                        g_NeProcInfo.stMduInfo[uiLoop].usSSlotId);
                }
                else
                {
                    XOS_Sprintf(ucIpStr, MAX_IPSTR_LEN, "172.16.%d.%d", uiIdx, 
                                                            g_NeProcInfo.stMduInfo[0].usSSlotId);
                }
            }

            XOS_StrtoIp(ucIpStr, &pHaInfo[uiIdx].stIpAddr[HA_IP_NUM - 1].ip);
            
            pHaInfo[uiIdx].stIpAddr[HA_IP_NUM - 1].port = usPort;
        }

    }
    else
    {
        XOS_PRINT(MD(FID_OAM,PL_WARN),"not HA mode!\r\n");
    }
    
    return;
}


/****************************************************************************
*������:OAM_SoftDebugRspSend
*����  : �����Ӧ
*����  :
*            OAM_SOFTDEBUG_RSP_T* pMsg*pMsg    ��Ԫ��Ӧ��Ϣ
*���  :
*����  :
*            0           �ɹ�
*            ����ֵ      ʧ��
*˵��  :
****************************************************************************/ 
XS32 OAM_SoftDebugRspSend(OAM_SOFTDEBUG_RSP_T* pMsg)
{
    XU32 uiMsgLen = 0;
    XS8* pReqData = XNULL;
    OAM_AGT_SFTDBG_RSP_T stSoftDebug;    
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_SoftDebugRspSend function!\r\n");
    
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_MemSet(&stSoftDebug, 0, sizeof(OAM_AGT_SFTDBG_RSP_T));
    
    /*��ȡλ����Ϣ*/
    stSoftDebug.stLocInfo.usRackId  = 1; /*Ŀǰֻ��һ������*/
    stSoftDebug.stLocInfo.usFrameId = (XU16)g_OamLinkCB.siFrameId;
    stSoftDebug.stLocInfo.usSlotId  = (XU16)g_OamLinkCB.siSlotId;
    stSoftDebug.stLocInfo.usNeId    = (XU16)g_OamLinkCB.usNeId;
    stSoftDebug.stLocInfo.usProcId  = (XU16)g_OamLinkCB.usPid;

    /*������Ӧ��Ϣ*/
    XOS_MemCpy(&stSoftDebug.stRspInfo, pMsg, sizeof(OAM_SOFTDEBUG_RSP_T));
        
    /*������Ϣ�����ڴ�*/
    uiMsgLen = sizeof(XU32)+ sizeof(OAM_AGT_SFTDBG_RSP_T);
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    /*ǰ4��byte��msgType*/
    *(XU32*)pReqData = XOS_HtoNl(AGENT_DEBUG_RSP_MSG);

    XOS_MemCpy(pReqData, &stSoftDebug, sizeof(OAM_AGT_SFTDBG_RSP_T));
    
    /*������Ϣ*/
    if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    return XSUCC;
}


#ifdef __cplusplus
}
#endif
