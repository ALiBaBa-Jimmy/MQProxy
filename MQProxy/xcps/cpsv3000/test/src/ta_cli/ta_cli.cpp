
#include "ta_cli.h"
#include "trace_agent.h"

#ifdef XOS_TRACE_AGENT

#ifdef __cplusplus
extern "C" {
#endif

    XS32 TaCliTest(HANDLE hdir, XS32 argc, XCHAR** argv);

    XS8 TaCliInit( XVOID*, XVOID*);
    XS8 TaCliMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 TaCliTimerProc(t_BACKPARA* para);

    void TaCliCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
    void TaCliShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
    XS32 TaCliSend(XS8* data, XS32 len,XU32 fid,XU16 msgID);

t_XOSFIDLIST g_TaCliFid =
{
    { "FID_TA_CLI",  NULL, FID_TA_CLI,},
    { TaCliInit, NULL, NULL,},
    { TaCliMsgProc, TaCliTimerProc,},eXOSMode, NULL
};

// ----------------------------- MyTest -----------------------------
XS32 TaCliTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_TaCliFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_Ta_CLI", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_TA_CLI;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "test", "test modules", "");

    promptID = XOS_RegistCmdPrompt(promptID, "tacli", "tacli", "");

    XOS_RegistCommand(promptID, TaCliCmd, "tacli", "start Ta client", "");

    XOS_RegistCommand(promptID, TaCliShow, "tashow", "show local info", "tashow");

    return XSUCC;
}

XS8 TaCliInit( XVOID*, XVOID*)
{
    MMInfo("TaCliInit begin");


    MMInfo("TaCliInit end");
    return XSUCC;
}

XS8 TaCliMsgProc( XVOID* inMsg, XVOID*)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    XS8 ret = XERROR;
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_TA_CLI,PL_ERR),"TaCliMsgProc()->input param error!");
        return XERROR;
    }
    
    XOS_Trace(MD(FID_TA_CLI,PL_ERR),"recv a msg!");

    t_TaGetTelReq *telReq = NULL;
    t_TaGetImsiReq *imsiReq = NULL;
    t_TaGetTelRsp telRsp;
    t_TaGetImsiRsp imsiRsp;
    XU8 testNum[] = "1234512345";

    switch (pMsg->msgID) 
    {
        case MME_IMSI_REQ:
            XOS_Trace(MD(FID_TA_CLI, PL_ERR), "MME_IMSI_REQ");
            imsiReq = (t_TaGetImsiReq *)pMsg->message;
            XOS_MemCpy(&(imsiRsp.head), &(imsiReq->head),sizeof(t_TaMsgHead));
            imsiRsp.head.usCmd = XOS_HtoNs(TA_CMD_GET_IMSI_RSP);

            XOS_StrNcpy(imsiRsp.data.ucMd5,imsiReq->data.ucMd5, MAX_MD5_LEN);
            imsiRsp.data.usImsiLen = XOS_StrLen(testNum);
            XOS_StrNcpy(imsiRsp.data.ucImsi,testNum, XOS_StrLen(testNum));
            
            imsiRsp.tail.usEndFlag = imsiReq->tail.usEndFlag;
            
            ret = TaCliSend((XS8 *)&imsiRsp, sizeof(t_TaGetImsiRsp), FID_TA,MME_IMSI_RSP);
            if(XERROR == ret)
            {
                XOS_Trace(MD(FID_TA_CLI, PL_ERR), "MME_IMSI_REQ send msg to ta fail");
            }

            break;
            
        case MME_TEL_REQ:
            XOS_Trace(MD(FID_TA_CLI, PL_ERR), "MME_TEL_REQ");
            telReq = (t_TaGetTelReq *)pMsg->message;
            XOS_MemCpy(&(telRsp.head), &(telReq->head),sizeof(t_TaMsgHead));
            telRsp.head.usCmd = XOS_HtoNs(TA_CMD_GET_TEL_RSP);

            XOS_StrNcpy(telRsp.data.ucMd5,telReq->data.ucMd5, MAX_MD5_LEN);
            telRsp.data.usTelLen = XOS_StrLen(testNum);
            XOS_StrNcpy(telRsp.data.ucTel,testNum, XOS_StrLen(testNum));
            
            telRsp.tail.usEndFlag = telReq->tail.usEndFlag;
            
            ret = TaCliSend((XS8 *)&telRsp, sizeof(t_TaGetTelRsp), FID_TA,MME_TEL_RSP);
            if(XERROR == ret)
            {
                XOS_Trace(MD(FID_TA_CLI, PL_ERR), "MME_IMSI_REQ send msg to ta fail");
            }
            break;
            
        default:
            XOS_CpsTrace(MD(FID_TA_CLI, PL_INFO), "get other msgid=%d", pMsg->msgID);
            break;
        }

    return 0;
}

XS8 TaCliTimerProc(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_TA_CLI, PL_INFO), "TaCliTimerOut -> %d", para->para1);

    XS32 nIndex = 0;
    if(NULL != para)
    {
    }

    return 0;
}

void TaCliCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    char cmd[64];
    XS32 i = 0;
    XU32 nSendLen = 0;
    if(siArgc < 2) {
        return;
    }

    if(!XOS_StrCmp(cmd, "Taclient")) 
    {

    } 


}

void TaCliShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{

    return ;
}

XS32 TaCliSend(XS8* data, XS32 len,XU32 fid,XU16 msgID)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_TA_CLI, len);
    if(pMsg)
    {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_TA_CLI;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = fid;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = msgID;
        XOS_MemCpy(pMsg->message, data, len);

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_TA_CLI, pMsg);
            XOS_Trace(FILI, FID_TA_CLI, PL_ERR, "ERROR: TaCliSend send data failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

