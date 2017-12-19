/****************************************************************************
*版权     : Xinwei Telecom Technology Inc
*文件名   : oam_main.cpp
*文件描述 : OAM 模块对其他模块提供的接口的实现
*作者     : xiaohuiming
*创建日期 : 2014-09-01
*修改记录 :
****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
//#pragma warning(disable:4996)
#include "oam_main.h"
#include "oam_file.h"
#include "oam_link.h"

XSTATIC OAM_SESSION_HEAD_T g_AgtCfgHead[OAM_SESSION_MAX_NUM];

/*表与模块关系结构*/
OAM_TID_MID_T g_TidMidArray[MAX_TB_NUM];
XU32 g_TidMidCnt = 0;
OAM_TID_MID_T g_AsyncTidMidArray[MAX_TB_NUM];
XU32 g_AsyncTidMidCnt = 0;
FM_ALARM_SAVE_T g_AlarmMsgSave[FM_TYPE_NUM];/*0:故障告警缓存 1:事件告警缓存*/
NE_MDU_T g_NeProcInfo;


XU32 g_AgtSrvIpAddr = 0; /*agent的IP地址*/
XU32 g_TsFbIpAddr = 0;   /*TS进程需要的前插板的IP地址*/

XU32 g_SlotId[HA_IP_NUM] = {0};/*主备槽位号*/


XS8 g_ProcessType[VALUE_MAX_LEN] = {0};/*进程类型*/

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
*函数名:OAMEntry
*功能  :XOS入口函数
*输入  :
*           HANDLE hdir
*           XS32 argc
*           XCHAR** argv
*输出  :
*返回  :
*           XS8
*说明  
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
*函数名:OAM_Init
*功能  :向XOS提供的模块初始化函数
*输入  :
*           XVOID *Para1
*           XVOID *Para2
*输出  :
*返回  :
*           XS8
*说明  :
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

    /*初始化OAM关注进程管理表信息*/
    g_TidMidArray[0].uiModuleId = FID_OAM;
    g_TidMidArray[0].uiTableId  = TID_PROCESS_MANAGE;
    g_TidMidCnt++;

    /*获取配置文件路径及进程类型*/
    OAM_XmlFilePathGet();

    /*通过xml文件获取Agent Ip 地址*/
    if (XSUCC != OAM_XmlTblInfoGet(g_xmlFilePath, AGENT_IP_TABLE_ID, FID_OAM, OAM_CFG_SYNC))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "OAM_XmlTblInfoGet agent IP[%s] failed.\r\n", g_xmlFilePath);
    }

    /*通过xml文件获取进程信息*/
    if (XSUCC != OAM_XmlTblInfoGet(g_xmlFilePath, TID_MODULE_MANAGE, FID_OAM, OAM_CFG_SYNC))
    {
        XOS_PRINT(MD(FID_OAM, PL_LOG), "OAM_XmlTblInfoGet TID_MODULE_MANAGE failed.\r\n");
    }
    
    /*填写agent IP地址*/
    oamCfg.peerIp = g_AgtSrvIpAddr;
    XOS_PRINT(MD(FID_OAM, PL_DBG), "oam get [%s] agent IP[0x%x].\r\n",g_xmlFilePath,g_AgtSrvIpAddr);
    
    /*通过平台接口获取网元号、进程ID、框号和槽位号*/
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

    /*命令行注册*/
    OAM_CliRegister();
        
    return XSUCC;
}


/****************************************************************************
*函数名: OAM_Notice
*功能  : NOTICE的消息处理函数
*输入  :
*           XVOID *pMsg
*           XVOID *Para
*输出  :
*返回  :
*           XS8
*说明  :
****************************************************************************/
XS8  OAM_Notice(XVOID* pMsg, XVOID* Para)
{
    return XSUCC;
}


/****************************************************************************
*函数名: OMA_MsgProc
*功能  : 向XOS提供的消息处理函数
*输入  :
*           XVOID *msg
*           XVOID *Para
*输出  :
*返回  :
*           XS8
*说明  :
****************************************************************************/     
XS8  OAM_MsgProc(XVOID* pMsg, XVOID* Para )
{
    XS32 siRet = XSUCC;
    t_XOSCOMMHEAD *pRecvMsg = XNULL;
    TS_IP_INFO_T stTsData = {0};
    
    PTR_NULL_CHECK(pMsg, XERROR);
    
    pRecvMsg = (t_XOSCOMMHEAD *)pMsg;

    /*模块注册消息*/
    if (APP_REGISTER_MSG == pRecvMsg->msgID)
    {
        return OAM_AppRegMsgProc(pRecvMsg, pRecvMsg->message, pRecvMsg->datasrc.FID);
    }

    /*TS获取前插板IP消息*/
    if (OAM_MSG_IP_REQ == pRecvMsg->msgID)
    {
        stTsData.uiIpAddr = g_TsFbIpAddr;
        stTsData.uiMslotId = g_SlotId[0];
        stTsData.uiSslotId = g_SlotId[1];
        XOS_PRINT(MD(FID_OAM, PL_LOG), "send to ts info{ip=0x%x,Mslotid=%d,sslotid=%d}!\r\n",
            stTsData.uiIpAddr,stTsData.uiMslotId,stTsData.uiSslotId);
        return OAM_OperMsgSend(pRecvMsg->datasrc.FID,OAM_MSG_IP_RSP,sizeof(TS_IP_INFO_T),&stTsData);
        
    }

    /*网元性能测量数据定时上报消息*/
    if (OAM_PM_NOTIFY_MSG == pRecvMsg->msgID)
    {
        return OAM_PmReportMsgProc(pRecvMsg->message);
    }

    if (AGENT_DEBUG_RSP_MSG == pRecvMsg->msgID)
    {
        return OAM_SoftDebugRspSend((OAM_SOFTDEBUG_RSP_T*)pRecvMsg->message);
    }
    
    /*BM状态上报及响应消息*/
    if (FID_BM == pRecvMsg->datasrc.FID)
    {
        siRet = OAM_BmMsgProc(pRecvMsg);
    }
    else if (FID_NTL == pRecvMsg->datasrc.FID)/*agent配置和操作消息*/
    {
        siRet = OAM_NtlMsgProc(pRecvMsg);
    }
    else if (FID_OAM == pRecvMsg->datasrc.FID)
    {
        siRet = OAM_SelfMsgProc(pRecvMsg->message);/*处理自身消息*/
    }
    else /*业务配置响应消息*/ 
    {
        siRet = OAM_AppRspMsgProc(pRecvMsg);   
    }
    
    return (XS8)siRet;
}


/****************************************************************************
*函数名: OAM_BmMsgProc
*功能  : 处理与BM相关的消息
*输入  :
*           t_XOSCOMMHEAD *pMsg 接收的BM消息
*输出  :
*返回  :
*           XS8
*说明  :
****************************************************************************/     
XS8 OAM_BmMsgProc(t_XOSCOMMHEAD *pMsg )
{
    t_XOSCOMMHEAD *pRecvMsg = XNULL;
    XS32 siRet = XSUCC;
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_BmMsgProc function!\r\n");

    pRecvMsg = pMsg;

    switch(pRecvMsg->msgID)
    { 
        case MSG_REP_BOARD_HARDINFO: /*单次上报单板硬件信息*/
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
        
        case MSG_REQ_PROC_OPER: /*BM请求启动进程列表信息*/
        case MSG_REP_PROCINFO:  /*上报网元进程信息*/
        case MSG_REP_NETIFINFO: /*上报网口信息消息*/
        case MSG_REP_BOARDSTAT: /*上报单板状态*/
        case MSG_REP_PROCSTAT:  /*上报进程状态*/
        case MSG_REP_NETIFSTAT: /*上报网口link状态*/
        case MSG_REP_DISKINFO:  /*上报磁盘信息*/
        case MSG_REP_BOARDINFO: /*上报单板信息*/
        case MSG_QUE_BOARD_HARDINFO:/*查询单板硬件信息*/
        {
            siRet = OAM_AgtReqSend(pRecvMsg, pRecvMsg->msgID, FID_BM, 0);
        }
        break;

        case MSG_GET_AGENTIP:       /*获取AGENT IP消息*/
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
*函数名: OAM_NtlMsgProc
*功能  : 处理与NTL相关的消息
*输入  :
*           t_XOSCOMMHEAD *pMsg 接收的BM消息
*输出  :
*返回  :
*           XS8
*说明  :
****************************************************************************/     
XS8 OAM_NtlMsgProc(t_XOSCOMMHEAD *pMsg )
{
    t_XOSCOMMHEAD *pRecvMsg = XNULL;
    XS32 siRet = XSUCC;
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_NtlMsgProc function!\r\n");

    pRecvMsg = pMsg;

    switch(pRecvMsg->msgID)
    {
        case eSctpInitAck:/*链路初始化响应处理*/
            siRet = OAM_LinkInitAckProc(pRecvMsg);
            break;

        case eSctpStartAck:/*链路启动响应处理*/
            siRet = OAM_LinkStartAckProc(pRecvMsg);
            break;

        case eSctpDataInd:/*链路数据消息处理*/
            siRet = OAM_LinkDataIndProc(pRecvMsg);
            break;

        case eSctpStopInd:/*链路断开指示*/
            siRet = OAM_LinkStopIndProc(pRecvMsg);
            break;

        case eSctpErrorSend:/*数据发送错误*/
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
*函数名: OAM_TimeOut
*功能  : 向XOS提供的定时器超时处理函数
*输入  :
*           t_BACKPARA  *pstPara
*输出  :
*返回  :
*           XS8
*说明  :
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
            /*链路定时器超时处理*/
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
*函数名:OAM_AppRegister
*功能  : 应用模块注册表信息的回调函数
*输入  :
*            XU32 uiModuleId  模块ID
*            XU32 *pTableId   表ID列表
*            XU32 uiTableNum  表个数
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_AppRegister(XU32 uiModuleId, const XU32 *pTableId, XU32 uiTableNum)
{    
    XU32 uiType     = 0;
    XU32 uiOffset   = 0;
    XU32 uiTotalLen = 0;
    XS8* pBuf           = XNULL;
    t_XOSCOMMHEAD *pMsg = XNULL;

    unsigned int uiLen = uiTableNum;
        
    /*指针入参检查*/
    PTR_NULL_CHECK(pTableId, XERROR);

    /*count左移两位表示count的值增大为原来的4倍*/
    uiTableNum = uiTableNum << OAM_OFFSET_LEN;
    if(uiTableNum == 0)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "\table count is less than zero!\r\n");
        return XERROR;
    }

    /*TLV格式中类型、长度和值所占内存之和的大小*/
    uiTotalLen = uiTableNum + OAM_OFFSET_LEN * sizeof(unsigned int);
    
    if(XNULL == (pMsg = XOS_MsgMemMalloc(uiModuleId, uiTotalLen)))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR),"XOS_MsgMemMalloc failed!\r\n");
        return XERROR;
    }

    /*填写消息类型*/
    SET_XOS_MSG_HEAD(pMsg, APP_REGISTER_MSG, uiModuleId, FID_OAM);
    
    /*按TLV格式拷贝表ID*/
    //if(XNULL != pTableId)
    {
        pBuf = (XS8*)pMsg->message;
        
        /*指定拷贝类型*/
        uiType = OAM_REG_TALBEID;
        memcpy((void*)&pBuf[uiOffset], (void*)&uiType, sizeof(XU32));
        uiOffset += sizeof(XU32);
        
        /*指定拷贝长度*/
        memcpy((void*)&pBuf[uiOffset], (void*)&uiLen, sizeof(XU32));
        uiOffset += sizeof(XU32);
        
        /*指定拷贝的值*/
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
*函数名:OAM_AppRegMsgProc
*功能  : 应用模块注册消息处理
*输入  :
*            XVOID *pRecvMsg  收到的注册消息
*            XVOID *pMsg      消息内容
*            XU32 uiModuleId  模块ID
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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

    /*指针入参检查*/
    PTR_NULL_CHECK(pRecvMsg, XERROR);
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AppRegMsgProc function!\r\n");
    
    pTempVal = (XS8*)pMsg;

    /*取出注册类型*/
    XOS_MemCpy((XS8*)&uiRegType, (XVOID*)&pTempVal[uiOffset], sizeof(XU32));
    uiOffset += sizeof(XU32);

    /*取出注册表数*/
    XOS_MemCpy((XS8*)&uiTableNum, (XVOID*)&pTempVal[uiOffset], sizeof(XU32));
    uiOffset += sizeof(XU32);
        
    for (uiLoop = 0; uiLoop < uiTableNum; uiLoop++)
    {
        uiTableId = 0;
        XOS_MemCpy((XS8*)&uiTableId, (XVOID*)&pTempVal[uiOffset], sizeof(XU32));
        uiOffset += sizeof(XU32);

        /*加入存储*/
        if(XERROR==OAM_TableIdAdd(uiTableId,uiModuleId))
        {
            XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_TableIdAdd[tid=%d][mid=%d] failed.\r\n",
                uiTableId,uiModuleId);
            return XERROR;
        }

        /*先获取链路状态*/
        if(XSUCC != OAM_LinkGet(&stOamLink))
        {
            /*加入存储*/
            if(XERROR == OAM_AsyncTableIdAdd(uiTableId,uiModuleId))
            {
                XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_AsyncTableIdAdd[tid=%d][mid=%d] failed.\r\n",
                uiTableId,uiModuleId);
                return XERROR;
            }
        
            /*从本地xml文件查找表ID对应数据记录并发送
            if (XSUCC != OAM_XmlTblInfoGet(g_xmlFilePath, uiTableId, uiModuleId))
            {
                XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_XmlTblInfoGet Module[%d]Table[%d] failed\r\n",
                                                                           uiModuleId, uiTableId);
                return XERROR;
            }*/
        }
        else
        {
            /*发送配置请求到agent获取表ID对应的数据记录*/
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
*函数名:OAM_SelfMsgProc
*功能  : 处理模块格式化操作进程信息
*输入  :
*            XVOID*pMsg
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_SelfMsgProc(XVOID*pMsg)
{
    AGT_OAM_CFG_REQ_T *pAgtOamData = XNULL;
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_SelfMsgProc function!\r\n");

    pAgtOamData = (AGT_OAM_CFG_REQ_T *)pMsg;

    OAM_NeProcInfoProc(pAgtOamData->pData, OAM_CFG_FMT, pAgtOamData->uiMsgLen);

    return XSUCC;
}


/****************************************************************************
*函数名:OAM_AgtCfgMsgProc
*功能  : agent配置消息处理
*输入  :
*            XVOID*pMsg    配置消息头和消息体
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
    XS8* pRecvData = XNULL;/*配置数据地址*/
    AGT_CFG_REQ_T* pRecvMsg = XNULL;
    AGT_OAM_CFG_REQ_T stAgtOamData = {0};
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtCfgMsgProc function!\r\n");
    
    pRecvMsg = (AGT_CFG_REQ_T *)pMsg;
    
    /*计算配置数据起始地址*/
    pRecvData = (XS8*)pMsg;
    pRecvData += (sizeof(AGT_CFG_REQ_T) - sizeof(XS8*));

    uiDataLen = XOS_NtoHl(pRecvMsg->stAgtOamData.uiMsgLen);

    /*获取未使用的消息头控制块索引并记录消息头*/
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

    /*转本地序*/
    stAgtOamData.uiIndex     = XOS_NtoHl(pRecvMsg->stAgtOamData.uiIndex);
    stAgtOamData.uiSessionId = XOS_NtoHl(pRecvMsg->stAgtOamData.uiSessionId);
    stAgtOamData.usNeId      = XOS_NtoHs(pRecvMsg->stAgtOamData.usNeId);
    stAgtOamData.usPid       = XOS_NtoHs(pRecvMsg->stAgtOamData.usPid);
    stAgtOamData.usModuleId  = XOS_NtoHs(pRecvMsg->stAgtOamData.usModuleId);
    stAgtOamData.uiOperType  = XOS_NtoHl(pRecvMsg->stAgtOamData.uiOperType);
    stAgtOamData.uiTableId   = XOS_NtoHl(pRecvMsg->stAgtOamData.uiTableId);
    stAgtOamData.uiRecNum    = XOS_NtoHl(pRecvMsg->stAgtOamData.uiRecNum);
    stAgtOamData.uiMsgLen    = XOS_NtoHl(pRecvMsg->stAgtOamData.uiMsgLen);
    
    /*拷贝配置数据*/
    XOS_MemCpy(stAgtOamData.pData, pRecvData, uiDataLen);
    
    if (OAM_CFG_ADD == stAgtOamData.uiOperType 
        || OAM_CFG_MOD == stAgtOamData.uiOperType
        || OAM_CFG_DEL == stAgtOamData.uiOperType)
    {
        /*给agent回响应*/
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
            //return XERROR;告警
        }
    }
    
    /*配置数据格式化*/
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
    
    /*往业务模块发配置消息*/
    if(XSUCC == OAM_TableIdFind(stAgtOamData.uiTableId, uiMid, &uiCount))
    {
        for(uiLoop=0; uiLoop < uiCount; uiLoop++)
        {
            if (FID_OAM == uiMid[uiLoop])/*覆盖保存进程信息*/
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
*函数名:OAM_AgtDataMsgProc
*功能  : agent数据消息处理(同步响应及导航树信息)
*输入  :
*            XVOID*pMsg    配置消息头和消息体
*            XU32 uiMsgId  消息ID
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtDataMsgProc[msgid=0x%x] function!\r\n",uiMsgId);

    pRecvMsg = (AGT_CFG_REQ_T *)pMsg;

    pucData = (XS8*)pMsg;
    pucData += (sizeof(AGT_CFG_REQ_T) - sizeof(XS8*));
    
    /*数据转换字节序*/
    uiDataLen = sizeof(AGT_OAM_CFG_REQ_T)- sizeof(XS8*);
    XOS_MemCpy(&stAgtOamData, &pRecvMsg->stAgtOamData, uiDataLen);
    OAM_AgtOamDataNtoH(&stAgtOamData);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtDataMsgProc[mid=%d] function!\r\n",
                                                                        stAgtOamData.usModuleId);
    /*计算消息长度*/
    uiMsgLen = sizeof(AGT_OAM_CFG_REQ_T) + stAgtOamData.uiMsgLen;
    if (XNULL == (pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
        
    SET_XOS_MSG_HEAD(pSendMsg, uiMsgId, FID_OAM, stAgtOamData.usModuleId);

    pAgtOamData = (AGT_OAM_CFG_REQ_T*)pSendMsg->message;
    XOS_MemCpy(pAgtOamData, &stAgtOamData, uiDataLen);

    /*计算pData值并拷贝pData*/
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
*函数名:OAM_AgtRspMsgProc
*功能  : agent响应消息处理
*输入  :
*            XVOID*pMsg    响应消息头和消息体
*输出  :
*返回  :
*            XSUCC       成功
*            其他值      失败
*说明  :
****************************************************************************/
XS32 OAM_AgtRspMsgProc(XVOID*pMsg)
{
    AGT_OAM_CFG_REQ_T* pCfgData     = XNULL;
    AGT_CFG_RSP_T*     pRecvMsg     = XNULL;
    AGT_OAM_CFG_HEAD_T stAgtCfgHead = { 0 };
    XU32               uiMsgLen     = 0;

    /*指针入参检查*/
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
    /*转本地序*/
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
        /*发送配置消息到业务模块*/
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
*函数名:OAM_AppRspMsgProc
*功能  : 处理模块同步响应消息
*输入  :
*            XVOID*pMsg
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
    
    /*指针入参检查*/
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
        
        /*前4个byte是msgType*/
        *(XU32*)pRspData = XOS_HtoNl(AGENT_CFG_MSG);
        pRspMsg = (AGT_CFG_RSP_T*)(&pRspData[sizeof(XU32)]);

        /*获取对应的消息头并添加消息头*/
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

        /*修改msgType为响应消息*/
        pRspMsg->stAgtCfgHead.uiMsgType = XOS_HtoNl(OAM_MSGTYPE_RESPONSE);

        /*添加消息体*/
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

        /*填充dataind数据*/
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
        /*不需要释放内存,暂时不处理业务响应*/
    }
    
    return XSUCC;
}


/****************************************************************************
*函数名:OAM_AgtFmtMsgProc
*功能  : 处理agent的格式化网元数据消息
*输入  :
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_AgtFmtMsgProc()
{
    XU32 uiLoop = 0;
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtFmtMsgProc function!\r\n");

    for(uiLoop=0; uiLoop < g_TidMidCnt && uiLoop < MAX_TB_NUM; uiLoop++)
    {
        if(g_TidMidArray[uiLoop].uiTableId >0 && g_TidMidArray[uiLoop].uiModuleId > 0)
        {
            /*从本地xml文件查找表ID对应数据记录并发送*/
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
*函数名:OAM_AgtSftDbgMsgProc
*功能  : 处理agent的软调消息
*输入  :
*            XVOID*pMsg
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
*函数名:OAM_PmReportMsgProc
*功能  : agent配置消息处理
*输入  :
*            XVOID*pMsg    配置消息头和消息体
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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

    /*获取oam链路*/
    if(XSUCC != OAM_LinkGet(&stOamLink))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "get oam link failed\r\n");
        return XERROR;
    }
        
    /*申请消息内容内存*/
    uiMsgLen = sizeof(XU32)+ sizeof(OAM_PM_REPORT_T);
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    /*前4个byte是msgType*/
    *(XU32*)pReqData = XOS_HtoNl(OAM_PM_NOTIFY_MSG);

    XOS_MemCpy(pReqData, &stPmData, sizeof(OAM_PM_REPORT_T));
    
    /*申请消息内存*/
    if(XNULL == (pSendMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_SCTPDATAREQ))))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }

    /*填写NTL数据发送链路*/
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
*函数名:OAM_CfgMsgSend
*功能  : 配置数据发送到模块接口
*输入  :
*            XU32 uiModuleId  模块ID
*            XU32 uiMsgId     消息类型
*            XVOID*pBuffer    配置数据地址
*            XU32 uiDataLen   pdata数据长度
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_CfgMsgSend(XU32 uiModuleId, XU32 uiMsgId, XVOID*pBuffer, XU32 uiDataLen)
{
    t_XOSCOMMHEAD* pSendMsg = XNULL;
    AGT_OAM_CFG_REQ_T *pTmpMsg = XNULL; 
    AGT_OAM_CFG_REQ_T *pAgtOamData = XNULL;
    XU32 uiMsgLen = 0;
    XU32 uiLoop = 0;
    XS8 *pTempData = XNULL;
    
    /*指针入参检查*/
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
    
    /*填写消息类型*/
    SET_XOS_MSG_HEAD(pSendMsg, (XU16)uiMsgId, FID_OAM, uiModuleId);

    /*拷贝除了pData之外的配置数据*/
    pSendMsg->length = uiMsgLen;
    pAgtOamData = (AGT_OAM_CFG_REQ_T*)pSendMsg->message;
    XOS_MemCpy(pAgtOamData, pTmpMsg, sizeof(AGT_OAM_CFG_REQ_T) - sizeof(XS8*));

    /*计算pData值并拷贝pData*/
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
*函数名:OAM_AgtOamDataNtoH
*功能  :AGT_OAM_CFG_REQ_T结构转本地序
*输入  :
*            AGT_OAM_CFG_REQ_T* pAgtOamData
*输出  :
*返回  :
*说明  :
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
*函数名:OAM_AgtRspSend
*功能  : 给agent回配置响应消息
*输入  :
*            XVOID*pMsg 配置消息头和消息体
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);
    XOS_PRINT(MD(FID_OAM,PL_ERR),"Enter OAM_AgtRspSend function!\r\n");
    pRecvMsg = (AGT_CFG_REQ_T*)pMsg;

    XOS_MemCpy(&stAgtCfgHead, &pRecvMsg->stAgtCfgHead, sizeof(AGT_OAM_CFG_HEAD_T));
    //stAgtCfgHead.uiSessionId = XOS_NtoHl(stAgtCfgHead.uiSessionId);

    XOS_MemCpy(&stAgtOamData, &pRecvMsg->stAgtOamData, sizeof(AGT_OAM_CFG_REQ_T));

    /*转字节序*/
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

        /*前4个byte是msgType*/
        *(XU32*)pRspData = XOS_HtoNl(AGENT_CFG_MSG);

        pRspMsg = (AGT_CFG_RSP_T*)(&pRspData[sizeof(XU32)]);

        /*获取对应的消息头并添加消息头*/
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

        /*修改msgType为响应消息*/
        pRspMsg->stAgtCfgHead.uiMsgType = XOS_HtoNl(OAM_MSGTYPE_RESPONSE);

        /*添加消息体*/
        pRspMsg->stAgtOamData.uiIndex     = XOS_HtoNl(stAgtOamData.uiIndex);
        pRspMsg->stAgtOamData.uiSessionId = XOS_HtoNl(stAgtOamData.uiSessionId);
        pRspMsg->stAgtOamData.usNeId      = XOS_HtoNs(stAgtOamData.usNeId);
        pRspMsg->stAgtOamData.usPid       = XOS_HtoNs(stAgtOamData.usPid);
        pRspMsg->stAgtOamData.usModuleId  = XOS_HtoNs(stAgtOamData.usModuleId);
        pRspMsg->stAgtOamData.uiOperType  = XOS_HtoNl(stAgtOamData.uiOperType);
        pRspMsg->stAgtOamData.uiTableId   = XOS_HtoNl(stAgtOamData.uiTableId);
        pRspMsg->stAgtOamData.uiRecNum    = XOS_HtoNl(stAgtOamData.uiRecNum);
        pRspMsg->stAgtOamData.uiRetCode   = XOS_HtoNl(XSUCC);

        /*填写消息类型*/
        SET_XOS_MSG_HEAD(pSendMsg, eSendData, FID_OAM, FID_NTL);

        /*获取oam链路*/
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
*函数名:OAM_OperMsgSend
*功能  : 操作数据发送到模块接口
*输入  :
*            XU32 uiModuleId  模块ID
*            XU32 uiMsgLen    消息长度
*            XVOID*pBuffer    配置数据地址
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_OperMsgSend(XU32 uiModuleId, XU32 uiMsgId, XU32 uiMsgLen, XVOID*pBuffer)
{
    t_XOSCOMMHEAD* pSendMsg = XNULL;

    /*指针入参检查*/
    PTR_NULL_CHECK(pBuffer, XERROR);

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_OperMsgSend[mid=%d] function!\r\n", uiModuleId);
    
    pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, uiMsgLen);
    if (NULL == pSendMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
    
    /*填写消息类型*/
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
*函数名:OAM_TransMsgSend
*功能  : OAM透传agent和BM消息发送接口
*输入  :
*            XVOID* pMsg    透传消息
*            XU32 uiMsgId   消息ID
*            XU32 uiDstFid  目的FID
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_TransMsgSend(XVOID* pMsg, XU32 uiMsgId, XU32 uiDstFid)
{
    XU32 uiLoop = 0;
    XU32 uiMsgLen = 0;
    XU32 uiDataLen = 0;
    t_XOSCOMMHEAD* pSendMsg  = XNULL;
    AGT_CFG_REQ_T *pRecvData = XNULL;
    XS8 *pucBuf = NULL;
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_OperMsgSend[mid=%d,msgid=%d] function!\r\n", uiDstFid,
        uiMsgId);

    pRecvData = (AGT_CFG_REQ_T *)pMsg;
    pucBuf = (XS8*)pMsg;

    uiDataLen = XOS_NtoHl(pRecvData->stAgtOamData.uiMsgLen);
    /*计算消息长度*/
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
*函数名:OAM_AgtReqSend
*功能  : OAM发送BM请求上报消息到agent
*输入  :
*            XVOID* pMsg    BM请求上报消息
*            XU32 uiMsgType
*            XU32 uiModuleId 模块ID
*            XU32 uiTableId  表ID
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_AgtReqSend(XVOID* pMsg, XU32 uiMsgType, XU32 uiModuleId, XU32 uiTableId)
{
    t_XOSCOMMHEAD* pRecvMsg = XNULL;
    XS8* pReqData = XNULL;
    AGT_CFG_REQ_T stReqMsg = { {0} };
    XU32 uiMsgLen = 0;

    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtReqSend[type=%d] function!\r\n", uiMsgType);
    
    pRecvMsg = (t_XOSCOMMHEAD *)pMsg;

    uiMsgLen = sizeof(XU32) + sizeof(AGT_CFG_REQ_T) + pRecvMsg->length;

    /*申请消息内容内存*/
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    /*前4个byte是msgType*/
    if(MSG_REQ_PROC_OPER == uiMsgType || MSG_QUE_BOARD_HARDINFO == uiMsgType)
    {
        *(XU32*)pReqData = XOS_HtoNl(uiMsgType);
    }
    else
    {
        *(XU32*)pReqData = XOS_HtoNl(OAM_MSGTYPE_NOTIFY);
    }
    
    /*添加消息头*/
    stReqMsg.stAgtCfgHead.uiMsgId   = XOS_HtoNl(pRecvMsg->msgID);
    stReqMsg.stAgtCfgHead.uiMsgType = XOS_HtoNl(uiMsgType);

    /*添加消息体, 只有上报消息和sync消息才有*/
    if(MSG_REQ_PROC_OPER != uiMsgType)
    {    
        stReqMsg.stAgtOamData.usModuleId = XOS_HtoNs(uiModuleId);
        stReqMsg.stAgtOamData.uiOperType = XOS_HtoNl(uiMsgType);
        stReqMsg.stAgtOamData.uiMsgLen   = XOS_HtoNl(pRecvMsg->length);
        
        XOS_MemCpy(pReqData + sizeof(XU32), &stReqMsg, sizeof(AGT_CFG_REQ_T));
        
        /*复制配置请求消息待发送*/
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

    /*发送消息*/
    if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    return XSUCC;
}


/****************************************************************************
*函数名:OAM_AgtSyncReqSend
*功能  : OAM发SYNC消息到agent
*输入  :
*            XVOID* pMsg    BM请求上报消息
*            XU32 uiMsgType
*            XU32 uiModuleId 模块ID
*            XU32 uiTableId  表ID
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_AgtSyncReqSend(XU32 uiMsgType, XU32 uiModuleId, XU32 uiTableId)
{
    XS8* pReqData = XNULL;
    AGT_CFG_REQ_T stReqMsg = { {0} };
    XU32 uiMsgLen = 0;
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtSyncReqSend function!\r\n");

    uiMsgLen = sizeof(XU32) + sizeof(AGT_CFG_REQ_T);
    
    /*申请消息内容内存*/
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    *(XU32*)pReqData = XOS_HtoNl(uiMsgType);
    
    /*添加消息头*/
    stReqMsg.stAgtCfgHead.uiMsgType = XOS_HtoNl(uiMsgType);

    stReqMsg.stAgtOamData.usModuleId = XOS_HtoNs(uiModuleId);
    stReqMsg.stAgtOamData.uiOperType = XOS_HtoNl(OAM_CFG_SYNC);
    
    /*通过平台接口获取网元ID*/
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

    /*发送消息*/
    if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
    {
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    return XSUCC;
}


/****************************************************************************
*函数名:OAM_HeadIdxGetBySid
*功能  :根据SessionId查找对应的head控制块索引
*输入  :
*            XU32 uiSessionId
*输出  :
*            XU32* puiIndex
*返回  :
*            XSUCC       成功
*            XERROR      失败
*说明  :
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
*函数名:OAM_HeadIndexGet
*功能  :查找未使用的head控制块索引
*输入  :
*输出  :
*            XU32* puiIndex
*返回  :
*            XSUCC       成功
*            XERROR      失败
*说明  :
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
*函数名:OAM_TsNavMsgProc
*功能  : TS导航树消息处理
*输入  :
*            XVOID*pMsg    配置消息头和消息体
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_TsNavMsgProc(XVOID*pMsg)
{
    XU32 uiLoop  = 0;
    XS8* pRecvData = XNULL;/*配置数据地址*/
    AGT_CFG_REQ_T* pRecvMsg = XNULL;
    t_XOSCOMMHEAD* pSendMsg  = XNULL;
    AGT_OAM_CFG_REQ_T stAgtOamData = { 0 };
    TS_CFG_REQ_T* pReq = XNULL;
    TS_NAV_INFO* pucData = XNULL;
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_TsNavMsgProc function!\r\n");
    
    pRecvMsg = (AGT_CFG_REQ_T *)pMsg;
    pRecvData = (XS8*)pMsg;
    pRecvData += sizeof(AGT_CFG_REQ_T);
    
    /*转字节序*/
    XOS_MemCpy(&stAgtOamData, &pRecvMsg->stAgtOamData, sizeof(AGT_OAM_CFG_REQ_T) - sizeof(XS8*));
    stAgtOamData.uiRecNum = XOS_NtoHl(stAgtOamData.uiRecNum);
    stAgtOamData.uiMsgLen = XOS_NtoHl(stAgtOamData.uiMsgLen);

    /*打印数据码流*/
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
    
    /*往TS模块发配置消息*/
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

    /*拷贝数据*/
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
*函数名:OAM_MsgIdSend
*功能  : 通知消息发送
*输入  :
*            XU32 uiModuleId  模块ID
*            XU32 uiMsgId     消息ID
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_MsgIdSend(XU32 uiModuleId, XU32 uiMsgId)
{
    t_XOSCOMMHEAD* pSendMsg = XNULL;

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_TsNavMsgProc[mid=%d,msgid=%d] function!\r\n",
        uiModuleId, uiMsgId);
    
    /*指针入参检查*/    
    pSendMsg =(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_XOSCOMMHEAD));
    if (NULL == pSendMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }
        
    /*填写消息类型*/
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
*函数名:OAM_TableIdFind
*功能  : 通过表ID查找模块列表
*输入  :
*            XU32 uiTableId  表ID
*输出  :
*            XU32* pModList
*            XU32* pCount
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
*函数名:OAM_TableIdAdd
*功能  : 增加表ID与模块对应关系记录
*输入  :
*            XU32 uiTableId  表ID
*            XU32 uiModuleId 模块
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
函数名:OAM_GetComPatcCurTime
功能     :获取"yyyymmddHHMMSS"格式的当前时间
输入     :
                XCHAR* dst,
                XU32 dstLen
输出     :
                XCHAR* dst,
返回     :
          XVOID
说明     :
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
*函数名:OAM_AlarmLocInfoGet
*功能  : 告警位置信息获取
*输入  :
*输出  :
*            XVOID*pMsg    位置信息
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
*函数名:OAM_AlarmSend
*功能  : 告警上报
*输入  :
*            XVOID*pMsg    网元告警参数信息
*            XU32 uiAlarmClass 告警类别 0:故障告警 1:事件告警
*            XU32 uiAlarmFlag  告警标志 0:告警恢复 1:告警上报
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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
    
    /*获取当前告警产生时间*/
    OAM_GetComPatcCurTime(ucCurTime, sizeof(ucCurTime));
    XOS_StrToLongNum(ucCurTime, &stFmAlarm.ulAlarmTime);

    stFmAlarm.uiAlarmClass = uiAlarmClass;
    stFmAlarm.ucAlarmFlag  = uiAlarmFlag;
    
    /*获取告警位置信息*/
    if(XSUCC != OAM_AlarmLocInfoGet(&stFmAlarm))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_AlarmLocInfoGet failed\r\n");
        return XERROR;
    }

    /*拷贝告警定位信息*/
    XOS_MemCpy(&stFmAlarm.stFmPara, pFmParaInfo, sizeof(FM_PARA_T));
        
    /*申请消息内容内存*/
    uiMsgLen = sizeof(XU32)+ sizeof(FM_ALARM_T);
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    /*前4个byte是msgType*/
    *(XU32*)pReqData = XOS_HtoNl(OAM_FM_NOTIFY_MSG);

    XOS_MemCpy(pReqData + sizeof(XU32), &stFmAlarm, sizeof(FM_ALARM_T));
    
    /*发送消息*/
    if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
    {
        /*告警缓存*/
        OAM_AlarmMsgSave(&stFmAlarm);
        PTR_MEM_FREE(FID_OAM, pReqData);
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
        return XERROR;
    }

    
    return XSUCC;
            
}


/****************************************************************************
*函数名:OAM_AgtReqSend
*功能  : OAM发送BM请求上报消息到agent
*输入  :
*            XVOID* pMsg    发送给agent的消息
*            XU32 uiMsgLen  消息长度
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_AgtMsgSend(XVOID* pMsg, XU32 uiMsgLen)
{
    t_XOSCOMMHEAD* pSendMsg = XNULL;
    t_SCTPDATAREQ* pNtlHead = XNULL;
    XS8* pReqData = XNULL;
    OAM_LINK_T stOamLink = { {0} };
    
    /*指针入参检查*/
    PTR_NULL_CHECK(pMsg, XERROR);
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_AgtMsgSend function!\r\n");
    
    pReqData = (XS8 *)pMsg;

    /*获取oam链路*/
    if(XSUCC != OAM_LinkGet(&stOamLink))
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "get oam link failed\r\n");
        return XERROR;
    }
    
    /*申请消息内存*/
    pSendMsg=(t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_SCTPDATAREQ));
    if(XNULL == pSendMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgMemMalloc failed\r\n");
        return XERROR;
    }

    /*填写NTL数据发送链路*/
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
*函数名:OAM_AlarmRepeatSend
*功能  : 缓存告警上报
*输入  :
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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

            /*申请消息内容内存*/
            uiMsgLen = sizeof(XU32)+ sizeof(FM_ALARM_T);
            if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
            {
                XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
                return XERROR;
            }

            XOS_MemSet(pReqData, 0, uiMsgLen);

            /*前4个byte是msgType*/
            *(XU32*)pReqData = XOS_HtoNl(OAM_FM_NOTIFY_MSG);

            XOS_MemCpy(pReqData, &stFmAlarmInfo, sizeof(FM_ALARM_T));
            
            /*发送消息*/
            if(XSUCC != OAM_AgtMsgSend(pReqData, uiMsgLen))
            {
                PTR_MEM_FREE(FID_OAM, pReqData);
                XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed!\r\n");
                return XERROR;
            }

            /*继续发下一条*/
            g_AlarmMsgSave[uiTypeIndex].usHead++;
            g_AlarmMsgSave[uiTypeIndex].usHead %= FM_MSG_SAVE_NUM;
            uiLoop++;
        }
    }
    
    return XSUCC;
}


/****************************************************************************
*函数名:OAM_AlarmMsgSave
*功能  : 告警缓存
*输入  :
*            XVOID*pMsg    网元告警参数信息
*            XU32 uiAlarmClass 告警类别 0:故障告警 1:事件告警
*            XU32 uiAlarmFlag  告警标志 0:告警恢复 1:告警上报
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
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

    if (FM_CLASS_ALARM == pFmAlarm->uiAlarmClass)/*故障告警缓存*/
    {
        uiIndex = 0;
    }
    else /*事件告警缓存*/
    {
        uiIndex = 1;
    }

    /*消息缓存到对应消息数组里*/
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
*函数名:OAM_NeProcInfoProc
*功能  :处理网元进程信息
*输入  :
*            XVOID* pMsg 数据信息
*            XU32 uiType 操作类型-OAM_CFG_E
*            XU32 uiMsgLen 消息长度
*输出  :
*返回  :
*            XSUCC       成功
*            XERROR      失败
*说明  :
****************************************************************************/ 
XVOID OAM_NeProcInfoProc(XVOID* pMsg, XU32 uiType, XU32 uiMsgLen)
{
    XU32 uiIndex = 0;
    XU32 uiRecNum = 0;
    MODULE_MANAGE_T *pModuleInfo = XNULL;
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_NeProcInfoProc function!\r\n");
    
    /*指针入参检查*/
    if (XNULL == pMsg)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"msg ptr is null!!\r\n");
        return;
    }

    /*记录长度判断*/
    if (sizeof(MODULE_MANAGE_T) != uiMsgLen)
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"input msglen[%d] diff struct len[%d]!!\r\n", 
                                                              uiMsgLen, sizeof(MODULE_MANAGE_T));
        return;
    }
    
    pModuleInfo = (MODULE_MANAGE_T *)pMsg;

    switch (uiType)
    {
        case OAM_CFG_ADD:/*直接增加记录*/
        {
            XOS_MemCpy(&g_NeProcInfo.stMduInfo[g_NeProcInfo.uiProcNum], pModuleInfo, uiMsgLen);
            g_NeProcInfo.uiProcNum++;
        }
        break;
            
        case OAM_CFG_MOD:/*通过内存比较找到并修改*/
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
            
        case OAM_CFG_DEL:/*删除根据网元ID和进程ID匹配清除内存记录*/
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
            
        case OAM_CFG_SYNC:/*同步或者格式化操作直接全覆盖记录信息*/
        {
            XOS_MemSet(&g_NeProcInfo, 0, sizeof(g_NeProcInfo));
            XOS_MemCpy(&g_NeProcInfo.stMduInfo[g_NeProcInfo.uiProcNum], pModuleInfo, uiMsgLen);
            g_NeProcInfo.uiProcNum++;
        }
        break;
        
        case OAM_CFG_FMT:/*格式化操作*/
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
*函数名:OAM_HaIpInfoGet
*功能  :获取HA的ip信息
*输入  :
*输出  :
*            XU16 usSlotId  槽位号
*返回  :
*            XSUCC       成功
*            XERROR      失败
*说明  :
****************************************************************************/ 
XVOID OAM_HaInfoGet(OAM_HAINFO_T *pHaInfo)
{
    XU16 usPort = 0;
    XU32 uiLoop = 0;
    XU32 uiIdx  = 0;
    XCHAR ucIpStr[MAX_IPSTR_LEN+1] = {0};

    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_HaInfoGet function!\r\n");

    /*根据规则计算端口*/
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
    
    /*框号从1开始, ip从0开始:uiFrameId - 1*/
    if (1 == g_NeProcInfo.uiProcNum) /*主备部署或者单机*/
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
*函数名:OAM_SoftDebugRspSend
*功能  : 软调响应
*输入  :
*            OAM_SOFTDEBUG_RSP_T* pMsg*pMsg    网元响应信息
*输出  :
*返回  :
*            0           成功
*            其他值      失败
*说明  :
****************************************************************************/ 
XS32 OAM_SoftDebugRspSend(OAM_SOFTDEBUG_RSP_T* pMsg)
{
    XU32 uiMsgLen = 0;
    XS8* pReqData = XNULL;
    OAM_AGT_SFTDBG_RSP_T stSoftDebug;    
    
    XOS_PRINT(MD(FID_OAM,PL_DBG),"Enter OAM_SoftDebugRspSend function!\r\n");
    
    PTR_NULL_CHECK(pMsg, XERROR);

    XOS_MemSet(&stSoftDebug, 0, sizeof(OAM_AGT_SFTDBG_RSP_T));
    
    /*获取位置信息*/
    stSoftDebug.stLocInfo.usRackId  = 1; /*目前只有一个机架*/
    stSoftDebug.stLocInfo.usFrameId = (XU16)g_OamLinkCB.siFrameId;
    stSoftDebug.stLocInfo.usSlotId  = (XU16)g_OamLinkCB.siSlotId;
    stSoftDebug.stLocInfo.usNeId    = (XU16)g_OamLinkCB.usNeId;
    stSoftDebug.stLocInfo.usProcId  = (XU16)g_OamLinkCB.usPid;

    /*拷贝响应信息*/
    XOS_MemCpy(&stSoftDebug.stRspInfo, pMsg, sizeof(OAM_SOFTDEBUG_RSP_T));
        
    /*申请消息内容内存*/
    uiMsgLen = sizeof(XU32)+ sizeof(OAM_AGT_SFTDBG_RSP_T);
    if(XNULL == (pReqData = (XS8*)XOS_MemMalloc(FID_OAM, uiMsgLen)))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MemMalloc failed\r\n");
        return XERROR;
    }

    XOS_MemSet(pReqData, 0, uiMsgLen);

    /*前4个byte是msgType*/
    *(XU32*)pReqData = XOS_HtoNl(AGENT_DEBUG_RSP_MSG);

    XOS_MemCpy(pReqData, &stSoftDebug, sizeof(OAM_AGT_SFTDBG_RSP_T));
    
    /*发送消息*/
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
