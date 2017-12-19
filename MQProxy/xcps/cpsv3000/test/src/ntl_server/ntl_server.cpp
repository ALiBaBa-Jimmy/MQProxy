#include "../fid_def.h"
#include "ntl_server.h"
#include "xosenc.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 NtlServerTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 NtlServerInit( XVOID*, XVOID*);
    XS8 NtlServerClose(XVOID *Para1, XVOID *Para2);
    XS8 NtlServerMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 NtlServerMsgOut(t_BACKPARA* para);

    void NtlServerCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
    void TcpSerShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

    void TcpSvrStart();

    static void TcpSvrSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 count);
    
    static XS32 NtlLinkInit(t_LINKINIT & linkInit);
    static XS32 NtlLinkStart(t_LINKSTART& linkStart);
    static XS32 NtlSendData(XS8* data, XS32 len);
    static XS32 NtlLinkStop(int index);
    XS32 TcpLinkRelease(XS32 index);
    
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherNtlServerFid ={
    { "FID_NTL_SERVER",  XNULL, FID_NTL_SERVER,},
    { NtlServerInit, NULL, NtlServerClose,},
    { NtlServerMsgProc, NtlServerMsgOut,}, eXOSMode, NULL
};


typedef struct NtlServerObj {
    XS8    _used;
    HAPPUSER   _app;

    t_LINKINITACK _IniAck;
    t_STARTACK   _StartAck;
    t_CONNIND _Conn[10];
    int con_num;
}NtlServerObj;

typedef struct tcpMsg{
    XCHAR ip[64];
    XU16 port;
    XU16 index;
};
tcpMsg g_tcpMsg = {0};

#ifndef MaxObj
#define MaxObj 1024
#endif
XCHAR g_sendBuf[2048];

static XS32 g_appNum = 0x01;
static XS32 g_index = 0;
static XCHAR g_localIp[24];
static XS32 g_localPort = 20000;

static NtlServerObj g_serArray[MaxObj];
PTIMER g_timer = XNULL;

static XS32 LOG_GetLocalIP(XCHAR* ipaddress)
{
    XU32 ipaddr = 0;
    XCHAR ipstr[XOS_INET_ADDR_LEN];
    struct in_addr inaddr;
    if(XOS_GetPhysicIP(&ipaddr) == XSUCC) {
        inaddr.s_addr = htonl(ipaddr);
        XOS_Inet_ntoa(inaddr,ipstr);
        XOS_StrCpy(ipaddress, ipstr);
        XOS_CpsTrace(MD(FID_NTL_SERVER, PL_INFO), "LOG_GetLocalIP :%s.",ipstr);
        return XSUCC;
    }
    return XERROR;
}


// ----------------------------- MyTest -----------------------------


XS32 NtlServerTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherNtlServerFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_NTL_SERVER", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_NTL_SERVER;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "ntlserver", "ntlserver", "");
    XOS_RegistCommand(promptID, NtlServerCmd, "tcpserver", "Auto start tcp servers", "localip startport servernumber");
    XOS_RegistCommand(promptID, NtlServerCmd, "tcprelease", "release a tcp server", "link index");
    XOS_RegistCommand(promptID, NtlServerCmd, "tcpclose", "close a tcp server", "linkindex");
    XOS_RegistCommand(promptID, NtlServerCmd, "tcpsend", "send data to client", "ip port index len");
    XOS_RegistCommand(promptID, TcpSerShow, "tcpshow", "show local network info", "tcpshow");
    
    return XSUCC;
}

XS8 NtlServerInit( XVOID*, XVOID*)
{
    MMInfo("NtlServerInit begin");
    
    if (XSUCC != XOS_TimerReg(FID_NTL_SERVER, 500, 5, 0))
    {
        XOS_Trace(MD(FID_NTL_SERVER, PL_ERR), "NtlServerInit()-> register timer error!\n");
        return XERROR;
    }

    //LOG_GetLocalIP(g_localIp);
    g_localPort = 20000;
    
    for(int i = 0; i < MaxObj; i++) {
        g_serArray[i]._used = 0;
    }    
    
    MMInfo("NtlServerInit end");
    return XSUCC;
}

XS8 NtlServerClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_NTL_SERVER, PL_INFO), "NtlServerClose ...");

    return 0;
}

XS8 NtlServerMsgProc( XVOID* inMsg, XVOID*)
{
    t_LINKSTART linkStart;
    int index = 0;
    
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_NTL_SERVER,PL_ERR),"NtlServerMsgProc()->input param error!");
        return XERROR;
    }

    switch (pMsg->msgID) {
       case eInitAck:
           {
               t_LINKINITACK * pInitAck = (t_LINKINITACK*)pMsg->message;
               printf("NtlServerTest LinkInitAck result=%d,apphandle:%d\n", pInitAck->lnitAckResult,pInitAck->appHandle);
               
               if(pInitAck->lnitAckResult == eSUCC) {
                   for(int i = 0; i < MaxObj; i++) {
                       if(g_serArray[i]._app == pInitAck->appHandle) {
                           XOS_MemCpy(&(g_serArray[i]._IniAck), pInitAck, sizeof(t_LINKINITACK));
                           index = i;
                           break;
                       }
                   }
                   
                   linkStart.linkHandle = g_serArray[index]._IniAck.linkHandle;
                   XOS_StrtoIp(g_localIp,&(linkStart.linkStart.tcpServerStart.myAddr.ip));
//                linkStart.linkStart.tcpServerStart.myAddr.ip = inet_network(g_localIp);
                linkStart.linkStart.tcpServerStart.myAddr.port = g_localPort++;
                linkStart.linkStart.tcpServerStart.allownClients = 100;
                linkStart.linkStart.tcpServerStart.authenFunc = NULL;
                XOS_CpsTrace(MD(FID_NTL_SERVER, PL_INFO), "start tcp server IP :%x,port:%d.",
                    linkStart.linkStart.tcpServerStart.myAddr.ip,
                    linkStart.linkStart.tcpServerStart.myAddr.port);
                if(NtlLinkStart(linkStart) != XSUCC) {
                    printf("send NtlServerTest::NtlLinkStart fail!, port=%d\n", g_localPort - 1);
                   }
               }
           }
           break;
       case eStartAck:
           {
               t_STARTACK * pStartAck = (t_STARTACK*)pMsg->message;
               if(pStartAck->linkStartResult == eSUCC) {
                   for(int i = 0; i < MaxObj; i++) {
                       if(g_serArray[i]._app == pStartAck->appHandle) {
                           XOS_MemCpy(&(g_serArray[i]._StartAck), pStartAck, sizeof(t_STARTACK));
                           printf("Link started successfully, index=%d\n", i);
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
            for(int i = 0; i < MaxObj; i++) 
            {
                if(g_serArray[i]._app == pConn->appHandle) 
                {
                    XOS_MemCpy(&(g_serArray[i]._Conn[g_serArray[i].con_num]), pConn, sizeof(t_CONNIND));
                    XOS_IptoStr(pConn->peerAddr.ip, ipval);
                    g_serArray[i].con_num++;
                    printf("Accept a client connection,serv:%d,cliNum:%d,ip=%s,port=%d\n",
                        i,g_serArray[i].con_num, ipval, pConn->peerAddr.port);
///////////////////////////////
/*                    t_DATAREQ dataReq;
                    char msg[] = "msg to client";
                    
                    while(1)
                    {
                        XS8* pMsg = (XS8*)XOS_MemMalloc(FID_NTL_SERVER, strlen(msg));

                        dataReq.dstAddr.ip = pConn->peerAddr.ip;
                        dataReq.dstAddr.port = pConn->peerAddr.port;
                        dataReq.linkHandle = g_serArray[i]._IniAck.linkHandle;
                        dataReq.msgLenth = strlen(msg);
                        dataReq.pData = pMsg;
                        XOS_MemCpy(dataReq.pData, msg, strlen(msg));
                        
                        t_XOSCOMMHEAD *pSendMsg = XNULL;
                        XS8 ret = XERROR;
                        
                        pSendMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_SERVER, sizeof(t_DATAREQ) + strlen(msg));
                        if(pSendMsg) {
                            pSendMsg->datasrc.PID = XOS_GetLocalPID();
                            pSendMsg->datasrc.FID = FID_NTL_SERVER;
                            pSendMsg->datadest.PID = XOS_GetLocalPID();
                            pSendMsg->datadest.FID = FID_NTL;
                            pSendMsg->prio = eFastMsgPrio;
                            pSendMsg->msgID = eSendData;
                            XOS_MemCpy(pSendMsg->message, &dataReq, sizeof(t_DATAREQ) + strlen(msg));

                            ret = XOS_MsgSend(pSendMsg);
                            if(ret != XSUCC)
                            {
                                XOS_MsgMemFree(FID_NTL_SERVER, pSendMsg);
                                printf("ntl servers send data failed.");
                                return XERROR;
                            }
                        }
                        //XOS_Sleep(500);
*/
                        //////////////////////////
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
                       printf("Recv Msg,ip=%s,port=%d, msg=%s\n\n",
                           ipval,
                           pData->peerAddr.port,
                           pData->pData);
                       XOS_MemFree(FID_NTL_SERVER,pData->pData);
                   }
               }
           }
           break;
        case eStopInd:
            {
                t_LINKCLOSEIND * pConn = (t_LINKCLOSEIND*)pMsg->message;

                for(int i = 0; i < MaxObj; i++) 
                {
                    if(g_serArray[i]._app == pConn->appHandle) 
                    {
                        g_serArray[i].con_num = 0;
                    }
                }
            }
            break;
       default:
           printf("get other msgid=%d\n", pMsg->msgID);
           break;
       }

    return 0;
}

XS8 NtlServerMsgOut(t_BACKPARA* para)
{
    XS32 nIndex = 0;
    if(NULL != para)
    {
        nIndex = para->para1;
        if(nIndex == 1)
        {
            TcpSvrSend(g_tcpMsg.ip, g_tcpMsg.port,g_sendBuf,1);
        }
    }

    return 0;
}

void TcpStartSendTimer(void)
{
    t_PARA timerpara;
    t_BACKPARA backpara = {0};

    timerpara.fid = FID_NTL_SERVER;
    timerpara.len = 500;
    timerpara.mode = TIMER_TYPE_LOOP;
    timerpara.pre  = TIMER_PRE_HIGH;
    backpara.para1 = 1;
    if (XSUCC != XOS_TimerStart(&g_timer, &timerpara, &backpara))
    {
        XOS_Trace(MD(FID_NTL_SERVER, PL_EXP), "ntl server start test timer error!");
    }
}

void NtlServerCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    char cmd[64];
    int serNum = 0;
    int i = 0;
    int sendLen = 0;
    if(siArgc < 2)
    {
        return;
    }

    XOS_StrNcpy(cmd, ppArgv[0], sizeof(cmd));
    if(!XOS_StrCmp(cmd, "tcpserver")) 
    {
        strcpy(g_localIp, ppArgv[1]);
        g_localPort = atoi(ppArgv[2]);
        serNum = atoi(ppArgv[3]);
        for(int i =0;i< serNum;i++)
        {
            TcpSvrStart();
            XOS_Sleep(50);
        }
    }
    else if(!XOS_StrCmp(cmd, "tcpsend"))
    {
        if(siArgc >= 2) 
        {
            sendLen = atoi(ppArgv[4]);
            for(i = 0 ;i < sendLen; i++)
            {
                g_sendBuf[i] = 'a';
            }
            g_sendBuf[i] = '\0';
            memcpy(g_tcpMsg.ip, ppArgv[1], strlen(ppArgv[1]));
            g_tcpMsg.port = atoi(ppArgv[2]);
            g_tcpMsg.index = atoi(ppArgv[3]);

            TcpStartSendTimer();
        }
    }
    else if(!XOS_StrCmp(cmd, "tcprelease"))
    {
        if(siArgc >= 2) 
        {
            TcpLinkRelease(atoi(ppArgv[1]));
        }
    }
    else if(!XOS_StrCmp(cmd, "tcpclose")) 
    {
        if(siArgc >= 2) 
        {
            NtlLinkStop(atoi(ppArgv[1]));
        }
    }

}

void TcpSerShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS32 index = -1;
    XS32 i =0 ;
    XS32 j = 0;

    /*tcp client cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "tcp server statistic list \r\n----------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-6s%-12s\r\n",
        "index",
        "serveraddr",
        "listenport",
        "type"
        );

    for(i=0; i<MaxObj; i++)
    {
        if(g_serArray[i]._used)
        {
            XOS_CliExtPrintf(pCliEnv,
                "----------------------------\r\n");
            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-6d   tcp server\r\n",
                i,g_serArray[i]._StartAck.localAddr.ip,g_serArray[i]._StartAck.localAddr.port
                );
            for(j = 0;j<g_serArray[i].con_num;j++)
            {
                XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-6d    tcp client\r\n",
                j,g_serArray[i]._Conn[j].peerAddr.ip,g_serArray[i]._Conn[j].peerAddr.port
                );
            }
        }
    }

    /*end of list */
    XOS_CliExtPrintf(pCliEnv,
        "----------------------------\r\n");
    return ;
}

void TcpSvrStart()
{
    t_LINKINIT linkInit;
    
    int index = 0;
    int i = 0;
    
    linkInit.linkType = eTCPServer;
    linkInit.ctrlFlag = eNullLinkCtrl;
    linkInit.appHandle = (HAPPUSER)g_appNum++;

    for(i = 0; i < MaxObj; i++) {
        if(!g_serArray[i]._used) {
            g_serArray[i]._app = linkInit.appHandle;
            g_serArray[i]._used = 1;
            index = i;
            break;
        }
    }
    
    if(i == MaxObj) {
        printf("NtlServer Link number is enough, max=%d\n", MaxObj);
        return;
    }
    
    NtlLinkInit(linkInit);
}

void TcpSvrSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 count)
{
    t_DATAREQ dataReq;
    int j;
    for(int i = 0; i < count; i++) 
    {
        for(j=0;j<500;j++)
        {
            XS8* pMsg = (XS8*)XOS_MemMalloc(FID_NTL_SERVER, strlen(msg));

            XOS_StrtoIp(peerip,&(dataReq.dstAddr.ip));
            dataReq.dstAddr.port = peerport + j;
            dataReq.linkHandle = g_serArray[g_tcpMsg.index + j]._IniAck.linkHandle;
            dataReq.msgLenth = strlen(msg);
            dataReq.pData = pMsg;
            XOS_MemCpy(dataReq.pData, msg, strlen(msg));

            NtlSendData((XCHAR *)&dataReq, sizeof(t_DATAREQ));
        }
    }    
}

XS32 NtlLinkInit(t_LINKINIT & linkInit)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_SERVER, sizeof(t_LINKINIT));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkInit;
        XOS_MemCpy(pMsg->message, &linkInit, sizeof(t_LINKINIT));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_SERVER, pMsg);
            XOS_Trace(FILI, FID_NTL_SERVER, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
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
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_SERVER, sizeof(t_LINKSTART));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkStart;
        XOS_MemCpy(pMsg->message, &linkStart, sizeof(t_LINKSTART));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_SERVER, pMsg);
            XOS_Trace(FILI, FID_NTL_SERVER, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
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
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_SERVER, len);
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eSendData;
        XOS_MemCpy(pMsg->message, data, len);

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_SERVER, pMsg);
            XOS_Trace(FILI, FID_NTL_SERVER, PL_ERR, "ERROR: NtlSendData send data failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 NtlLinkStop(int index)
{
    t_LINKCLOSEREQ linkClosed;
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    linkClosed.linkHandle = g_serArray[index]._IniAck.linkHandle;
    linkClosed.cliAddr.ip = 0;
    linkClosed.cliAddr.port = 0;
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_SERVER, sizeof(t_LINKCLOSEREQ));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkStop;
        XOS_MemCpy(pMsg->message, &linkClosed, sizeof(t_LINKCLOSEREQ));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            XOS_MsgMemFree(FID_NTL_SERVER, pMsg);
            XOS_Trace(FILI, FID_NTL_SERVER, PL_ERR, "ERROR: NtlSendData send data failed.");
            return XERROR;
        }
        g_serArray[index]._used = 0;
        return XSUCC;
    }
    return ret;
}

XS32 TcpLinkRelease(XS32 index)
{
    t_LINKRELEASE linkRelease;
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    linkRelease.linkHandle = g_serArray[index]._IniAck.linkHandle;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_SERVER, sizeof(t_LINKRELEASE));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkRelease;
        XOS_MemCpy(pMsg->message, &linkRelease, sizeof(t_LINKRELEASE));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_SERVER, pMsg);
            XOS_Trace(FILI, FID_NTL_SERVER, PL_ERR, "ERROR: TcpLinkRelease failed.");
            return XERROR;
        }
        g_serArray[index]._used = 0;
        return XSUCC;
    }
    return ret;
}


