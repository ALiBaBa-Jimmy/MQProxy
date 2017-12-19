
#include "xosmd5_test.h"


#ifdef __cplusplus
extern "C" {
#endif

    XS32 Md5Test(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 Md5TestInit( XVOID*, XVOID*);
    XS8 Md5TestClose(XVOID *Para1, XVOID *Para2);
    XS8 Md5TestMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 Md5TestTimeOut(t_BACKPARA* para);

    XS32 Md5TestCheckSum(XS8* msg);
    XS32 Md5TestFileSum(XS8 * msg);
    static const char* PrintSummary(XS8* sum, XS32 len);
    
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherMd5TestFid ={
    { "FID_MD5_TEST",  XNULL, FID_MD5_TEST,},
    { Md5TestInit, NULL, Md5TestClose,},
    { Md5TestMsgProc, Md5TestTimeOut,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 Md5Test(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherMd5TestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_MD5_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_MD5_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "Md5Test", "Md5Test", "");
    XOS_RegistCommand(promptID, Md5TestCmd, "MsgSum", "get message summary of md5", "message");
    XOS_RegistCommand(promptID, Md5TestCmd, "FileSum", "get file summary of md5", "file name");
    
    return XSUCC;
}

XS8 Md5TestInit( XVOID* argv1, XVOID* argv2)
{
    MMInfo("Md5TestInit begin");


    MMInfo("Md5TestInit end");
    return XSUCC;
}

XS8 Md5TestMsgProc( XVOID* inMsg, XVOID* argv2)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    t_MD5HEADER * pMd5Header = XNULL;
    t_MD5CHECKSUMACK * pMd5CheckSum = XNULL;
    t_MD5CHECKFILEACK * pMd5FileSum = XNULL;
    
    XOS_Trace(MD(FID_MD5_TEST, PL_INFO), "TEST - RECV");

    pMsg = (t_XOSCOMMHEAD*)inMsg;
    if(!pMsg) {
        return XERROR;
    }
    if(pMsg->length == 0 || pMsg->message == XNULL) {
        XOS_CpsTrace(MD(FID_MD5_TEST, PL_INFO), "t_XOSCOMMHEAD error, length or message invalid");
        return XERROR;
    }
    pMd5Header = (t_MD5HEADER*)pMsg->message;
    
    switch(pMsg->msgID) {
    case BeginMd5Ack:
        break;
    case UpdateAck:
        break;
    case GetAck:
        break;
    case CheckSumAck:
        pMd5CheckSum = (t_MD5CHECKSUMACK*)pMd5Header->message;
        XOS_CpsTrace(MD(FID_MD5_TEST, PL_INFO), "Md5 Check summary,len=%d,msg=%s",
            pMd5CheckSum->macLen,
            PrintSummary(pMd5CheckSum->macMsg,
            pMd5CheckSum->macLen));
        break;
    case CheckFileAck:
        pMd5FileSum = (t_MD5CHECKFILEACK*)pMd5Header->message;
        XOS_CpsTrace(MD(FID_MD5_TEST, PL_INFO), "Md5 Check summary,len=%d,msg=%s",
            pMd5FileSum->msgLen,
            PrintSummary(pMd5FileSum->msg,
            pMd5FileSum->msgLen));
        break;
    default:
        break;
    }
    return 0;
}

XS8 Md5TestTimeOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_MD5_TEST, PL_INFO), "Md5TestTimeOut -> %d", para->para1);

    return 0;
}

XS8 Md5TestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_MD5_TEST, PL_INFO), "Md5TestClose ...");
    return 0;
}

void Md5TestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    if(siArgc < 2) {
        return;
    }
    if(XOS_StrCmp(ppArgv[0], "MsgSum") == 0) {
        Md5TestCheckSum(ppArgv[1]);
    } else if(XOS_StrCmp(ppArgv[0], "FileSum") == 0) {
        Md5TestFileSum(ppArgv[1]);
    }
}


XS32 Md5TestCheckSum(XS8* msg)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    t_MD5HEADER * pMd5Header = XNULL;
    t_MD5CHECKSUMREQ * pMd5CheckSum = XNULL;
    XS8 ret = XERROR;
    XS32 len = sizeof(t_MD5HEADER) + sizeof(t_MD5CHECKSUMREQ) + strlen(msg);

    XOS_CpsTrace(MD(FID_MD5_TEST, PL_INFO), "Md5TestCheckSum begin");
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_MD5_TEST, len);
    
    if(pMsg) {
        XOS_MemSet(pMsg, 0, len);
        pMd5Header = (t_MD5HEADER*)((XS8*)pMsg + sizeof(t_XOSCOMMHEAD));
        pMd5CheckSum = (t_MD5CHECKSUMREQ*)((XS8*)pMd5Header + sizeof(t_MD5HEADER));

        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_MD5_TEST;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_MD5;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = CheckSumReq;
        pMsg->length = len;
        pMsg->message = (XS8*)pMd5Header;

        pMd5Header->length = sizeof(t_MD5CHECKSUMREQ) + strlen(msg);
        pMd5Header->message = (XS8*)pMd5CheckSum;
        pMd5Header->result = Md5Ok;

        pMd5CheckSum->srcLen = strlen(msg);
        pMd5CheckSum->srcMsg = (XS8*)pMd5CheckSum + sizeof(t_MD5CHECKSUMREQ);

        XOS_MemCpy(pMd5CheckSum->srcMsg, msg, strlen(msg));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_MD5_TEST, pMsg);
            XOS_Trace(FILI, FID_MD5_TEST, PL_ERR, "ERROR: Md5TestCheckSum send data failed.");
            return XERROR;
        }
    }
    return ret;
}

XS32 Md5TestFileSum(XS8 * filename)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    t_MD5HEADER * pMd5Header = XNULL;
    t_MD5CHECKFILEREQ * pMd5FileSum = XNULL;
    
    XS8 ret = XERROR;
    XS32 len = sizeof(t_MD5HEADER) + sizeof(t_MD5CHECKFILEREQ);

    XOS_CpsTrace(MD(FID_MD5_TEST, PL_INFO), "Md5TestFileSum begin");
    
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_MD5_TEST, len);
    
    if(pMsg) {
        XOS_MemSet(pMsg, 0, len);
        pMd5Header = (t_MD5HEADER*)((XS8*)pMsg + sizeof(t_XOSCOMMHEAD));
        pMd5FileSum = (t_MD5CHECKFILEREQ*)((XS8*)pMd5Header + sizeof(t_MD5HEADER));

        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_MD5_TEST;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_MD5;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = CheckFileReq;
        pMsg->length = len;
        pMsg->message = (XS8*)pMd5Header;

        pMd5Header->length = sizeof(t_MD5CHECKFILEREQ);
        pMd5Header->message = (XS8*)pMd5FileSum;
        pMd5Header->result = Md5Ok;

        XOS_MemCpy(pMd5FileSum->fileName, filename, strlen(filename));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_MD5_TEST, pMsg);
            XOS_Trace(FILI, FID_MD5_TEST, PL_ERR, "ERROR: Md5TestFileSum send data failed.");
            return XERROR;
        }
    }
    return ret;
}

const char* PrintSummary(XS8* sum, XS32 len)
{
    static XS8 buf[128] = {""};
    int i = 0,j;
    XS8 c;
    for(i = 0,j = 0; i < len; i++) {
        c = (sum[i] >> 4) & 0x0f;
        if(c >= 0 && c <= 9) {
            buf[j++] = c + '0';
        } else {
            buf[j++] = c - 0xa + 'A';
        }
        c = sum[i] & 0x0f;
        if(c >= 0 && c <= 9) {
            buf[j++] = c + '0';
        } else {
            buf[j++] = c - 0xa + 'A';
        }
    }
    buf[j] = 0;
    return buf;
}