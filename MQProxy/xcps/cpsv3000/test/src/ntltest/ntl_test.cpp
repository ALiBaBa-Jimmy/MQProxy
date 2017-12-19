
#include "ntl_test.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 NtlTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 NtlTestInit( XVOID*, XVOID*);
    XS8 NtlTestClose(XVOID *Para1, XVOID *Para2);
    XS8 NtlTestMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 NtlTestMsgOut(t_BACKPARA* para);

    void NtlTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

    void StartTcpSvr(XS8* ip, XS32 port);
    void StartTcpCli(XS8* ip, XS32 port, XS8* peerip, XS32 peerport);
    void TcpSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 count);

    XS32 TcpSvrStart();
    
    XS32 NtlLinkInit(t_LINKINIT & linkInit);
    XS32 NtlLinkStart(t_LINKSTART& linkStart);
    XS32 NtlSendData(XS8* data, XS32 len);
    
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherNtlTestFid ={
    { "FID_NTL_TEST",  XNULL, FID_NTL_TEST,},
    { NtlTestInit, NULL, NtlTestClose,},
    { NtlTestMsgProc, NtlTestMsgOut,}, eXOSMode, NULL
};


typedef struct NtlTestObj {
    XS8    _used;
    t_XOSSEMID _Semid;
    HAPPUSER   _app;

    t_LINKINITACK _IniAck;
    t_STARTACK   _StartAck;
    t_CONNIND _Conn;
}NtlTestObj;

XS32 g_appNum = 0x01;
XS32 g_index = 0;
#define MaxObj 100
NtlTestObj g_objArray[MaxObj];


// ----------------------------- MyTest -----------------------------


XS32 NtlTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherNtlTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_NTL_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_NTL_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "NtlTest", "NtlTest", "");
    XOS_RegistCommand(promptID, NtlTestCmd, "TcpServer", "start tcp server", "ip port");
    XOS_RegistCommand(promptID, NtlTestCmd, "TcpClient", "start tcp client", "localip localport peerip peerport");
    XOS_RegistCommand(promptID, NtlTestCmd, "TcpSend", "send message to peer", "peerip peerport message count");
    
    return XSUCC;
}

XS8 NtlTestInit( XVOID*, XVOID*)
{
    MMInfo("NtlTestInit begin");

    for(int i = 0; i < MaxObj; i++) {
        XOS_SemCreate(&(g_objArray[i]._Semid), 0);
        g_objArray[i]._used = 0;
    }    
    
    MMInfo("NtlTestInit end");
    return XSUCC;
}

XS8 NtlTestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "NtlTestClose ...");

    for(int i = 0; i < MaxObj; i++) {
        XOS_SemDelete(&(g_objArray[i]._Semid));
    }
    
    return 0;
}

XS8 NtlTestMsgProc( XVOID* inMsg, XVOID*)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_NTL_TEST,PL_ERR),"NtlTestMsgProc()->input param error!");
        return XERROR;
    }

    switch (pMsg->msgID) {
       case eInitAck:
           {
               t_LINKINITACK * pInitAck = (t_LINKINITACK*)pMsg->message;
               XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "LinkInitAck result=%d", pInitAck->lnitAckResult);
               if(pInitAck->lnitAckResult == eSUCC) {
                   for(int i = 0; i < MaxObj; i++) {
                       if(g_objArray[i]._app == pInitAck->appHandle) {
                           XOS_MemCpy(&(g_objArray[i]._IniAck), pInitAck, sizeof(t_LINKINITACK));
                           XOS_SemPut(&(g_objArray[i]._Semid));
                           break;
                       }
                   }
               }
           }
           break;
       case eStartAck:
           {
               t_STARTACK * pStartAck = (t_STARTACK*)pMsg->message;
               if(pStartAck->linkStartResult == eSUCC) {
                   for(int i = 0; i < MaxObj; i++) {
                       if(g_objArray[i]._app == pStartAck->appHandle) {
                           XOS_MemCpy(&(g_objArray[i]._StartAck), pStartAck, sizeof(t_STARTACK));
                           g_index = i;
                           XOS_SemPut(&(g_objArray[i]._Semid));
                           XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "Link started successfully, you can send something please.");
                           break;
                       }
                   }
               }
           }
           break;
       case eConnInd:
           {
               char ipval[255] = {""};
               
               t_CONNIND * pConn = (t_CONNIND*)pMsg->message;
               for(int i = 0; i < MaxObj; i++) {
                if(g_objArray[i]._app == pConn->appHandle) {
                    XOS_MemCpy(&(g_objArray[i]._Conn), pConn, sizeof(t_CONNIND));
                    XOS_IptoStr(pConn->peerAddr.ip, ipval);
                    XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO),
                        "Accept a client,ip=%s,port=%d",
                        ipval, pConn->peerAddr.port);
                    break;
                }
            }
           }
           break;
       case eDataInd:
           {
               char ipval[255] = {""};
               t_DATAIND * pData = (t_DATAIND*)pMsg->message;
               if(pData) {
                   XOS_IptoStr(pData->peerAddr.ip, ipval);
                   if(pData->dataLenth > 0 && pData->pData) {
                       XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO),
                           "Recv Msg,ip=%s,port=%d, msg=%s",
                           ipval,
                           pData->peerAddr.port,
                           pData->pData);
                   }
               }
           }
           break;
       default:
           XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "get other msgid=%d", pMsg->msgID);
           break;
       }

       //XOS_MemFree(FID_NTL_TEST, pMsg);
       
    return 0;
}

XS8 NtlTestMsgOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "NtlTestMsgOut -> %d", para->para1);

    return 0;
}



void NtlTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    char cmd[64];

    if(siArgc < 2) {
        return;
    }

    XOS_StrNcpy(cmd, ppArgv[0], sizeof(cmd));
    if(!XOS_StrCmp(cmd, "TcpServer")) {
        if(siArgc >= 3) {
            StartTcpSvr(ppArgv[1], atoi(ppArgv[2]));
        }
    } else if(!XOS_StrCmp(cmd, "TcpClient")) {
        if(siArgc >= 5) {
            StartTcpCli(ppArgv[1], atoi(ppArgv[2]), ppArgv[3], atoi(ppArgv[4]));
        }
    } else if(!XOS_StrCmp(cmd, "TcpSend")) {
        if(siArgc >= 5) {
            TcpSend(ppArgv[1], atoi(ppArgv[2]), ppArgv[3], atoi(ppArgv[4]));
        }
    }
}

void StartTcpSvr(XS8* ip, XS32 port)
{
    t_LINKINIT linkInit;
    t_LINKSTART linkStart;
    int index = 0;
    int i = 0;
    
    linkInit.linkType = eTCPServer;
    linkInit.ctrlFlag = eNullLinkCtrl;
    linkInit.appHandle = (HAPPUSER)g_appNum++;

    for(i = 0; i < MaxObj; i++) {
        if(!g_objArray[i]._used) {
            g_objArray[i]._app = linkInit.appHandle;
            g_objArray[i]._used = 1;
            index = i;
            break;
        }
    }
    if(i == MaxObj) {
        XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "Test Link is enough, max=%d", MaxObj);
        return;
    }
    
    if(NtlLinkInit(linkInit) == XSUCC) {
        if(XOS_SemGetExt(&(g_objArray[index]._Semid), 5) == XSUCC) {
            linkStart.linkHandle = g_objArray[index]._IniAck.linkHandle;
            XOS_StrtoIp(ip, &(linkStart.linkStart.tcpServerStart.myAddr.ip));
            linkStart.linkStart.tcpServerStart.myAddr.port = port;
            linkStart.linkStart.tcpServerStart.allownClients = 100;
            linkStart.linkStart.tcpServerStart.authenFunc = NULL;
            if(NtlLinkStart(linkStart) == XSUCC) {
                XOS_SemGetExt(&(g_objArray[index]._Semid), 5);
            } else {
                XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "NtlLinkStart failed,port=%d", port);
            }
        } else {
            XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "LinkInit wait response timeout");
        }
    } else {
        XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "LinkInit send fail");
    }
}

void StartTcpCli(XS8* ip, XS32 port, XS8* peerip, XS32 peerport)
{
    t_LINKINIT linkInit;
    t_LINKSTART linkStart;
    int index = 0;
    int i = 0;
    
    linkInit.linkType = eTCPClient;
    linkInit.ctrlFlag = eNullLinkCtrl;
    linkInit.appHandle = (HAPPUSER)g_appNum++;

    for(i = 0; i < MaxObj; i++) {
        if(!g_objArray[i]._used) {
            g_objArray[i]._app = linkInit.appHandle;
            g_objArray[i]._used = 1;
            index = i;
            break;
        }
    }
    if(i == MaxObj) {
        XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "Test Link is enough, max=%d", MaxObj);
        return;
    }
    
    if(NtlLinkInit(linkInit) == XSUCC) {
        if(XOS_SemGetExt(&(g_objArray[index]._Semid), 5) == XSUCC) {
            linkStart.linkHandle = g_objArray[index]._IniAck.linkHandle;
            XOS_StrtoIp(ip,&(linkStart.linkStart.tcpClientStart.myAddr.ip));
            linkStart.linkStart.tcpClientStart.myAddr.port = port;
            XOS_StrtoIp(peerip,&(linkStart.linkStart.tcpClientStart.peerAddr.ip));
            linkStart.linkStart.tcpClientStart.peerAddr.port = peerport;
            linkStart.linkStart.tcpClientStart.recntInteval = 0;
            if(NtlLinkStart(linkStart) == XSUCC) {
                XOS_SemGetExt(&(g_objArray[index]._Semid), 5);
            } else {
                XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "NtlLinkStart failed,port=%d", port);
            }
        } else {
            XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "LinkInit wait response timeout");
        }
    } else {
        XOS_CpsTrace(MD(FID_NTL_TEST, PL_INFO), "LinkInit send fail");
    }
}

void TcpSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 count)
{
    t_DATAREQ *dataReq = NULL;

    for(int i = 0; i < count; i++) {
        XS8* pMsg = (XS8*)XOS_MemMalloc(FID_NTL_TEST, sizeof(t_DATAREQ) + strlen(msg));

        dataReq = (t_DATAREQ*)pMsg;
        dataReq->dstAddr.ip = inet_addr(peerip);
        dataReq->dstAddr.port = peerport;
        dataReq->linkHandle = g_objArray[g_index]._IniAck.linkHandle;
        dataReq->msgLenth = strlen(msg);
        dataReq->pData = pMsg + sizeof(t_DATAREQ);
        XOS_MemCpy(dataReq->pData, msg, strlen(msg));

        NtlSendData(pMsg, sizeof(t_DATAREQ) + strlen(msg));
    }    
}



XS32 NtlLinkInit(t_LINKINIT & linkInit)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_TEST, sizeof(t_LINKINIT));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_TEST;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkInit;
        XOS_MemCpy(pMsg->message, &linkInit, sizeof(t_LINKINIT));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_TEST, pMsg);
            XOS_Trace(FILI, FID_NTL_TEST, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 NtlLinkStart(t_LINKSTART& linkStart)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_TEST, sizeof(t_LINKSTART));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_TEST;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkStart;
        XOS_MemCpy(pMsg->message, &linkStart, sizeof(t_LINKSTART));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_TEST, pMsg);
            XOS_Trace(FILI, FID_NTL_TEST, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 NtlSendData(XS8* data, XS32 len)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_TEST, len);
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_TEST;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eSendData;
        XOS_MemCpy(pMsg->message, data, len);

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_TEST, pMsg);
            XOS_Trace(FILI, FID_NTL_TEST, PL_ERR, "ERROR: NtlSendData send data failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}