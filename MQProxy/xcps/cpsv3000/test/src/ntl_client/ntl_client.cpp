#include "../fid_def.h"
#include "ntl_client.h"
#include "xosenc.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 NtlClientTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 NtlClientInit( XVOID*, XVOID*);
    XS8 NtlClientClose(XVOID *Para1, XVOID *Para2);
    XS8 NtlClientMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 NtlClientMsgOut(t_BACKPARA* para);

    void NtlClientCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

    void TcpCliStart();
    void TcpCliShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

    static XS32 NtlLinkInit(t_LINKINIT & linkInit);
    static XS32 NtlLinkStart(t_LINKSTART& linkStart);
    static XS32 NtlSendData(t_IPADDR addr, HLINKHANDLE linkHandle, XS8* data, XS32 len);
    static XS32 NtlLinkStop(int index);
    static XS32 NtlLinkRelease(int index);
    
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherNtlClientFid ={
    { "FID_NTL_CLIENTTEST",  XNULL, FID_NTL_CLIENT},
    { NtlClientInit, NULL, NtlClientClose,},
    { NtlClientMsgProc, NtlClientMsgOut,}, eXOSMode, NULL
};


typedef struct NtlClientObj {
    XS8    _used;
    HAPPUSER   _app;

    t_LINKINITACK _IniAck;
    t_STARTACK   _StartAck;
    t_CONNIND _Conn;
}NtlClientObj;

#ifndef MaxObj
#define MaxObj 1024
#endif

static XS32 g_appNum = 0x01;
static XS32 g_index = 0;
static XCHAR g_localIp[24];
static XCHAR g_PeerIp[24];
static XS32 g_localPort = 40000;
static XS32 g_peerPort = 20000;

static NtlClientObj g_cliArray[MaxObj];
XS32 recv_num = 1;

// ----------------------------- MyTest -----------------------------


XS32 NtlClientTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherNtlClientFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_NTL_CLIENT", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_NTL_CLIENT;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "ntlclient", "ntlclient", "");
    XOS_RegistCommand(promptID, NtlClientCmd, "tcpclient", "Auto start tcp clients", "localip, localport,peerip startport clientnumber");
    XOS_RegistCommand(promptID, NtlClientCmd, "tcprelease", "release a tcp clent", "link index");
    XOS_RegistCommand(promptID, NtlClientCmd, "tcpclose", "close a tcp clent", "link index");
    XOS_RegistCommand(promptID, TcpCliShow, "tcpshow", "show local network info", "tcpshow");
    return XSUCC;
}


XS8 NtlClientInit( XVOID*, XVOID*)
{
    MMInfo("NtlClientInit begin");

    for(int i = 0; i < MaxObj; i++) {
        g_cliArray[i]._used = 0;
    }    
    
    MMInfo("NtlClientInit end");
    return XSUCC;
}

XS8 NtlClientClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_NTL_CLIENT, PL_INFO), "NtlClientClose ...");

    return 0;
}

XS8 NtlClientMsgProc( XVOID* inMsg, XVOID*)
{
    t_LINKSTART linkStart;
    int index = 0;
    int i;
    
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_NTL_CLIENT,PL_ERR),"NtlClientMsgProc()->input param error!");
        return XERROR;
    }

    switch (pMsg->msgID) {
       case eInitAck:
           {
               t_LINKINITACK * pInitAck = (t_LINKINITACK*)pMsg->message;
               printf("NtlClientMsgProc LinkInitAck result=%d\n", pInitAck->lnitAckResult);
               
               if(pInitAck->lnitAckResult == eSUCC) 
               {
                   for(int i = 0; i < MaxObj; i++) 
                   {
                       if(g_cliArray[i]._app == pInitAck->appHandle) 
                       {
                           XOS_MemCpy(&(g_cliArray[i]._IniAck), pInitAck, sizeof(t_LINKINITACK));
                           index = i;
                           break;
                       }
                   }
                   
                   printf("begin to linkStart,appHandle:%d, localip=%s, port=%d\n",pInitAck->appHandle, g_localIp, g_localPort);
                   
                   linkStart.linkHandle = g_cliArray[index]._IniAck.linkHandle;
                   XOS_StrtoIp(g_localIp,&(linkStart.linkStart.tcpClientStart.myAddr.ip));
                    linkStart.linkStart.tcpClientStart.myAddr.port = g_localPort++;
                    XOS_StrtoIp(g_PeerIp,&(linkStart.linkStart.tcpClientStart.peerAddr.ip));
                    linkStart.linkStart.tcpClientStart.peerAddr.port = g_peerPort++;
                    
                    linkStart.linkStart.tcpClientStart.recntInteval = 1000;
                    if(NtlLinkStart(linkStart) != XSUCC) 
                   {
                        printf("send NtlClientTest::NtlLinkStart fail!, localport=%d, peerport=%d\n",
                        g_localPort - 1,
                        g_peerPort - 1);
                   }
               }
           }
           break;
       case eStartAck:
           {
               t_STARTACK * pStartAck = (t_STARTACK*)pMsg->message;
               char msg[255] = {0};
               
               if(pStartAck->linkStartResult == eSUCC) 
               {
                   for(int i = 0; i < MaxObj; i++) 
                   {
                       if(g_cliArray[i]._app == pStartAck->appHandle) 
                       {
                           XOS_MemCpy(&(g_cliArray[i]._StartAck), pStartAck, sizeof(t_STARTACK));
                           printf("Link started successfully, index=%d,appHandle:%d\n", i,pStartAck->appHandle);

                           XOS_Sprintf(msg, 255, "this is ntl_client,index=%d,port=%d", i, pStartAck->localAddr.port);
//                           for(i = 0; i < 300; i++) {
//                               NtlSendData(g_cliArray[i]._Conn.peerAddr,
//                                   g_cliArray[i]._IniAck.linkHandle, 
//                                   msg,
//                                XOS_StrLen(msg));
/*
                               if(i % 5 == 0) {
                                   NtlLinkStop(i);
                               }
                               if(i % 10 == 0) {
                                   NtlLinkRelease(i);
                               }*/
 //                       }
                               
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
                if(g_cliArray[i]._app == pConn->appHandle) {
                    XOS_MemCpy(&(g_cliArray[i]._Conn), pConn, sizeof(t_CONNIND));
                    XOS_IptoStr(pConn->peerAddr.ip, ipval);
                    printf("connect to server ip=%s,port=%d", ipval, pConn->peerAddr.port);
                    break;
                }
            }
           }
           break;
       case eDataInd:
           {
               t_DATAIND * pData = (t_DATAIND*)pMsg->message;
               if(pData)
               {
                   if(pData->dataLenth > 0 && pData->pData) 
                   {
                        recv_num++;
                        if(recv_num % 10000 == 0)
                        {
                            printf("recv 10000 msg...\n");
                            recv_num = 1;
                        }
                       /*printf("Recv Msg,ip=0x%x,port=%d, msg=%s\n",
                           pData->peerAddr.ip,
                           pData->peerAddr.port,
                           pData->pData);*/
                       XOS_MemFree(FID_NTL_CLIENT, pData->pData);
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

XS8 NtlClientMsgOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_NTL_CLIENT, PL_INFO), "NtlClientMsgOut -> %d", para->para1);

    return 0;
}


void TcpCliShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS32 index = -1;
    XS32 i =0 ;
    XS32 j = 0;

    /*tcp client cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "tcp client statistic list \r\n----------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-6s%-12s\r\n",
        "index",
        "IP",
        "port",
        "type"
        );

    for(i=0; i<MaxObj; i++)
    {
        if(g_cliArray[i]._used)
        {
            XOS_CliExtPrintf(pCliEnv,
            "%-6d%-12x%-6d   tcp client\r\n",
            i,g_cliArray[i]._StartAck.localAddr.ip,g_cliArray[i]._StartAck.localAddr.port
            );
        }
    }

    /*end of list */
    XOS_CliExtPrintf(pCliEnv,
        "----------------------------\r\n");

    return ;
}
void NtlClientCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    char cmd[64];
    int i =0;
    int cliNum = 0;
    if(siArgc < 2) 
    {
        return;
    }

    XOS_StrNcpy(cmd, ppArgv[0], sizeof(cmd));
    if(!XOS_StrCmp(cmd, "tcpclient")) 
    {
        XOS_MemCpy(g_localIp, ppArgv[1], sizeof(g_localIp));
        g_localPort = atoi(ppArgv[2]);
        XOS_MemCpy(g_PeerIp, ppArgv[3], sizeof(g_PeerIp));
        g_peerPort = atoi(ppArgv[4]);
        cliNum = atoi(ppArgv[5]);
        for(i = 0;i< cliNum;i++)
        {
            TcpCliStart();
            XOS_Sleep(100);
        }
    }
    else if(!XOS_StrCmp(cmd, "tcprelease")) 
    {
        if(siArgc >= 2) 
        {
            NtlLinkRelease(atoi(ppArgv[1]));
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

void TcpCliStart()
{
    t_LINKINIT linkInit;
    int index = 0;
    int i = 0;
    
    linkInit.linkType = eTCPClient;
    linkInit.ctrlFlag = eNullLinkCtrl;
    linkInit.appHandle = (HAPPUSER)g_appNum++;
    for(i = 0; i < MaxObj; i++) {
        if(!g_cliArray[i]._used) {
            g_cliArray[i]._app = linkInit.appHandle;
            g_cliArray[i]._used = 1;
            index = i;
            break;
        }
    }
    NtlLinkInit(linkInit);
}


XS32 NtlLinkInit(t_LINKINIT & linkInit)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_CLIENT, sizeof(t_LINKINIT));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkInit;
        XOS_MemCpy(pMsg->message, &linkInit, sizeof(t_LINKINIT));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_CLIENT, pMsg);
            XOS_Trace(FILI, FID_NTL_CLIENT, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
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
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_CLIENT, sizeof(t_LINKSTART));
    if(pMsg) 
    {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkStart;
        XOS_MemCpy(pMsg->message, &linkStart, sizeof(t_LINKSTART));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_CLIENT, pMsg);
            XOS_Trace(FILI, FID_NTL_CLIENT, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 NtlSendData(t_IPADDR addr, HLINKHANDLE linkHandle, XS8* data, XS32 len)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    t_DATAREQ data_req;
    XS8 ret = XERROR;
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_CLIENT, sizeof(t_DATAREQ));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eSendData;

        memcpy(&data_req.dstAddr, &addr, sizeof(t_IPADDR));
        data_req.dstAddr.port = addr.port;
        data_req.linkHandle = linkHandle;
        data_req.msgLenth = len;
        data_req.pData = (XCHAR*)XOS_MemMalloc(FID_NTL_CLIENT, len);
        XOS_MemCpy(data_req.pData, data, len);
        XOS_MemCpy(pMsg->message, &data_req, sizeof(t_DATAREQ));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_CLIENT, pMsg);
            XOS_MemFree(FID_NTL_CLIENT, data_req.pData);
            
            XOS_Trace(FILI, FID_NTL_CLIENT, PL_ERR, "ERROR: NtlSendData send data failed.");
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

    linkClosed.linkHandle = g_cliArray[index]._IniAck.linkHandle;
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_CLIENT, sizeof(t_LINKCLOSEREQ));
    if(pMsg) 
    {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkStop;
        XOS_MemCpy(pMsg->message, &linkClosed, sizeof(t_LINKCLOSEREQ));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_CLIENT, pMsg);
            XOS_Trace(FILI, FID_NTL_CLIENT, PL_ERR, "ERROR: NtlLinkStop  failed.");
            return XERROR;
        }
        g_cliArray[index]._used = 0;
        return XSUCC;
    }
    return ret;
}

XS32 NtlLinkRelease(int index)
{
    t_LINKRELEASE linkRelease;
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    linkRelease.linkHandle = g_cliArray[index]._IniAck.linkHandle;
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL_CLIENT, sizeof(t_LINKRELEASE));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_NTL_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkRelease;
        XOS_MemCpy(pMsg->message, &linkRelease, sizeof(t_LINKRELEASE));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_NTL_CLIENT, pMsg);
            XOS_Trace(FILI, FID_NTL_CLIENT, PL_ERR, "ERROR: NtlLinkRelease failed.");
            return XERROR;
        }
        g_cliArray[index]._used = 0;
        return XSUCC;
    }
    return ret;
}
