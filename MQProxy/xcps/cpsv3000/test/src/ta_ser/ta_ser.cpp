
#include "ta_ser.h"
#include "trace_agent.h"

#ifdef XOS_TRACE_AGENT
#ifdef __cplusplus
extern "C" {
#endif

    XS32 TaSerTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    XS8 TaSerInit( XVOID*, XVOID*);
    XS8 TaSerMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 TaSerTimerProc(t_BACKPARA* para);

    void TaTrace(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

t_XOSFIDLIST g_TaSerFid =
{
    { "FID_TA_SER",  NULL, FID_TA_SER,},
    { TaSerInit, NULL, NULL,},
    { TaSerMsgProc, TaSerTimerProc,}, eXOSMode, NULL
};

typedef enum 
{
    TA_MSG_SEND_TIMER = 1,
}e_TaSendTimer;

typedef enum
{
    e_taTrace = 0,
        e_tmgTrace,
        e_dataTrace,
        e_idTrace,
        e_delTrace,
        e_interface,
        e_logTrace,
        e_xosTrace
}e_traceType;

const XS8 *traceName[8] = 
{
    "XOS_TaTrace",
    "e_tmgTrace",
    "XOS_TaDataTrace",
    "XOS_TaIdTrace",
    "XOS_TaRegDel",
    "XOS_TaInterface",
    "XOS_LogTrace",
    "XOS_MsgSend"
};
XSTATIC PTIMER TaSerTimer;

// ----------------------------- MyTest -----------------------------
XS32 deleteFun(XU32 id)
{
    XOS_Trace(MD(FID_TA_SER, PL_ERR), "deleteFun()-> delete id:%d",id);
    return XSUCC;
}

XS32 TaSerTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_TaSerFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_Ta_SER", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_TA_SER;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "test", "test modules", "");

    promptID = XOS_RegistCmdPrompt(promptID, "taser", "taser", "");

    XOS_RegistCommand(promptID, TaTrace, "trace", "create trace info", "trace userID id");

    return XSUCC;
}

XS8 TaSerInit( XVOID*, XVOID*)
{
    MMInfo("TaSerInit begin");

    if (XSUCC != XOS_TimerReg(FID_TA_SER, 200, 1000, 10))
    {
        XOS_Trace(MD(FID_TA_SER, PL_ERR), "TaSerInit()-> register timer error!\n");
        return XERROR;
    }

    MMInfo("TaSerInit end");
    return XSUCC;
}

XS8 TaSerMsgProc( XVOID* inMsg, XVOID*)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_TA_SER,PL_ERR),"TaSerMsgProc()->input param error!");
        return XERROR;
    }

    switch (pMsg->msgID) 
    {

    default:
        XOS_CpsTrace(MD(FID_TA_SER, PL_INFO), "get other msgid=%d", pMsg->msgID);
        break;
    }

    return 0;
}


XS8 TaSerTimerProc(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_TA_SER, PL_INFO), "TaSerTimerOut -> %d", para->para1);

    if(NULL != para)
    {
    }

    return 0;
}


void TaTrace(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS8 cmd[64] = {0};
    XU8 userID[64] = {0};
    XS8 tmgData[] = "test data for tmg";
    XS8 taData[] = "test data for XOS_TaDataTrace";
    XS8 interfaceData[] = "test data for interface";
    t_InfectId infectId;
    XS8 ret = XERROR;
    XU32 traceID = 0;
    XU32 logID = 0;
    t_TaAddress addr;
    XS32 type = 0;

    if(siArgc < 2)
    {
        XOS_Trace(FILI, FID_TA_SER, PL_ERR, "args error,trace userID id.eg:trace 1234 1");
        return;
    }
    XOS_StrNcpy(cmd, ppArgv[0], sizeof(cmd));
    XOS_StrNcpy(userID, ppArgv[1], sizeof(userID));
    type = atoi(ppArgv[2]);
    XOS_Trace(FILI, FID_TA_SER, PL_ERR, "trace user:%s,type:%s",userID,traceName[type]);

    t_XOSCOMMHEAD *pMsg = XNULL;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_TA_SER, 4);
    if(pMsg)
    {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_TA_SER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_TA_CLI;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eSendData;
        XOS_MemCpy(pMsg->message, "test,%d", 4);
    }

    XOS_MemSet(&infectId, 0, sizeof(t_InfectId));
    infectId.idNum = 1;
    XOS_StrNcpy(infectId.id[0], userID, 32);
    /*感染*/
    traceID = XOS_TaInfect(&infectId);
    logID = XOS_LogInfect(&infectId);

    XOS_Trace(FILI, FID_TA_SER, PL_ERR, "infect traceID:%x,logID:%x",traceID,logID);

    pMsg->traceID = traceID;
    pMsg->logID = logID;

    switch(type)
    {
        case e_taTrace:
        /*控制面消息过滤，实际在平台收消息时统一调用，这里只是测试*/
        XOS_Trace(FILI, FID_TA_SER, PL_ERR, "XOS_TaTrace");
        XOS_TaTrace(FID_TA_SER,e_TA_SEND,pMsg);
        break;

        case e_tmgTrace:
        /*媒体数据跟踪过滤*/
        XOS_Trace(FILI, FID_TA_SER, PL_ERR, "XOS_TmgFilter");
        XOS_TmgFilter(tmgData,XOS_StrLen(tmgData),e_TA_VIDEO,0);
        break;

        case e_dataTrace:
        /*模块接口消息过滤*/
        XOS_Trace(FILI, FID_TA_SER, PL_ERR, "XOS_TaDataTrace");
        XOS_TaDataTrace(FID_TA_SER,FID_TA_CLI,taData,XOS_StrLen(taData),userID,e_TA_TEL,e_TA_SEND);
        break;

        case e_idTrace:
        /*nas消息过滤接口*/
        XOS_Trace(FILI, FID_TA_SER, PL_ERR, "XOS_TaIdTrace");
        XOS_TaIdTrace(pMsg,userID,e_TA_TEL,e_TA_SEND);
        break;

        case e_delTrace:
        /*traceID删除接口注册*/
        XOS_Trace(FILI, FID_TA_SER, PL_ERR, "XOS_TaRegDel");
        XOS_TaRegDel(FID_TA_SER,deleteFun);
        break;
        
        case e_interface:
        /*接口消息过滤*/
        XOS_StrtoIp("169.0.169.4", &(addr.src.ip));
        addr.src.port = 20000;
        XOS_StrtoIp("169.0.169.5", &(addr.dst.ip));
        addr.dst.port = 30000;
        XOS_Trace(FILI, FID_TA_SER, PL_ERR, "XOS_TaInterface");
        XOS_TaInterface(FID_TA_SER,IF_TYPE_CX,2,3,e_TA_SEND,&addr,interfaceData,XOS_StrLen(interfaceData));
        break;

        case e_logTrace:
        /*日志过滤*/
        XOS_Trace(FILI, FID_TA_SER, PL_ERR, "XOS_LogTrace");
        XOS_LogTrace(FILI,FID_TA_SER,PL_LOG,logID,"test log:%d",12341234);
        break;

        case e_xosTrace:
        /*平台收消息时自动过滤*/
        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_TA_SER, pMsg);
            XOS_Trace(FILI, FID_TA_SER, PL_ERR, "ERROR: send data failed.");
        }
            
        default:
            break;
    }

}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

