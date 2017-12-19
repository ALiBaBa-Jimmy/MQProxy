#include <string>
#include <map>
#include <vector>
#include <signal.h>

#include <xosshell.h>
#include <xosmmgt.h>
#include <xmlparser.h>
#include <xosmodule.h>
#include "xostype.h"

#include "../fid_def.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 MsgSendTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 MsgSendInit( XVOID*, XVOID*);
    XS8 MsgSendClose(XVOID *Para1, XVOID *Para2);
    XS8 MsgSendProc(XVOID *Para1, XVOID *Para2);
    XS8 MsgSendOut(t_BACKPARA* para);

    void MsgSendTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherMsgSendFid ={
    { "FID_MSGSEND_TEST",  XNULL, FID_MSGSEND_TEST,},
    { MsgSendInit, NULL, MsgSendClose,},
    { MsgSendProc, MsgSendOut,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 MsgSendTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherMsgSendFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_MSGSEND_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_MSGSEND_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "SendTest", "SendTest", "");
    XOS_RegistCommand(promptID, MsgSendTestCmd, "SendMsg", "Send test message", "message count");
    
    return XSUCC;
}

XS8 MsgSendInit( XVOID*, XVOID*)
{
    MMInfo("MsgSendInit begin");

    MMInfo("MsgSendInit end");
    return XSUCC;
}

XS8 MsgSendProc( XVOID* inMsg, XVOID*)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    
    XOS_Trace(MD(FID_MSGSEND_TEST, PL_INFO), "MsgSendProc RECV MSG, len=%d", pMsg->length);
    return 0;
}

XS8 MsgSendOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_MSGSEND_TEST, PL_INFO), "MsgSendOut -> %d", para->para1);

    return 0;
}

XS8 MsgSendClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_MSGSEND_TEST, PL_INFO), "MsgSendClose ...");
    return 0;
}

#pragma pack(1)
typedef struct TestMsg
{
    t_XOSCOMMHEAD _header;
    XS8 _msg[255];
}TestMsg;
#pragma pack()

void MsgSendTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    int msglen = 0;
    int count = 1;
    
    if(siArgc > 1) {
        msglen = XOS_StrLen(ppArgv[1]);
    }
    if(siArgc > 2) {
        count = atoi(ppArgv[2]);
    }
    
    for (int i=0; i<count; ++i)
    {
        //XOS_Trace(MD(FID_MY_SEND, PL_INFO), "TEST - Send");
        // 消息会被目的消息队列使用，由目的消息队列释放
        TestMsg* pMsg = (TestMsg*)XOS_MsgMemMalloc(FID_MSGSEND_TEST, sizeof(TestMsg));
        pMsg->_header.datasrc.PID  = XOS_GetLocalPID();
        pMsg->_header.datasrc.FID  = FID_MSGSEND_TEST;
        pMsg->_header.length       = msglen;
        pMsg->_header.msgID        = 0;
        pMsg->_header.prio         = eNormalMsgPrio;
        pMsg->_header.datadest.FID = FID_MSGTRACE_TEST;
        pMsg->_header.datadest.PID = XOS_GetLocalPID();
        pMsg->_header.message = pMsg->_msg;

        if(msglen > 0) {
            XOS_StrNcpy(pMsg->_msg, ppArgv[1], 255);
        }

        if (XOS_MsgSend((t_XOSCOMMHEAD*)pMsg) == XERROR)
        {
            XOS_MsgMemFree(FID_MSGSEND_TEST, (t_XOSCOMMHEAD*)pMsg);
            return;
        }
    }
}
