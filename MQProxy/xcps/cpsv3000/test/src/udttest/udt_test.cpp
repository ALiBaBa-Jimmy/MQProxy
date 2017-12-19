
#include "udt_test.h"


#ifdef __cplusplus
extern "C" {
#endif

    XS32 UdtTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 UdtTestInit( XVOID*, XVOID*);
    XS8 UdtTestClose(XVOID *Para1, XVOID *Para2);
    XS8 UdtTestMsgPro( XVOID* pMsgP, XVOID* sb);
    
    void AddIpc(XS8* localip, XS32 localport, XS8* remoteip, XS32 remoteport);
    void DelIpc(XS32 localipc, XS32 remoteipc);
    void SendUdtMsg(XS8* msg);
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherUdtTestFid ={
    { "FID_UDT_TEST",  XNULL, FID_UDT_TEST,},
    { UdtTestInit, NULL, UdtTestClose,},
    { UdtTestMsgPro, XNULL,}, eXOSMode, NULL
};

static unsigned int _ipcLocalIndex = -1;
static unsigned int _ipcRemoteIndex = -1;

// ----------------------------- MyTest -----------------------------


XS32 UdtTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherUdtTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_UDT_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_UDT_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "UdtTest", "Udt Test", "");
    XOS_RegistCommand(promptID, UdtTestCmd, "AddIpc", "Add Udt ipc", "localip, localport remoteip remoteport");
    XOS_RegistCommand(promptID, UdtTestCmd, "DelIpc", "Del Udt ipc", "localipc remoteipc");
    XOS_RegistCommand(promptID, UdtTestCmd, "SendMsg", "Send Message to remote peer", " message");
    
    return XSUCC;
}

XS8 UdtTestInit( XVOID* argv1, XVOID* argv2)
{
    MMInfo("UdtTestInit begin");

    udt_ini();
    
    MMInfo("UdtTestInit end");
    return XSUCC;
}


XS8 UdtTestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "UdtTestClose ...");

    udt_end();
    
    return 0;
}

XS8 UdtTestMsgPro( XVOID* pMsgP, XVOID* sb)
{
    t_XOSCOMMHEAD* pCpsHead = XNULL;

    pCpsHead = (t_XOSCOMMHEAD*)pMsgP;

}

void UdtTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    if(!XOS_StrCmp(ppArgv[0], "AddIpc")) {
        if(siArgc < 5) {
            XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "AddIpc missing paramters!!");
            return;
        }
        AddIpc(ppArgv[1], atoi(ppArgv[2]), ppArgv[3], atoi(ppArgv[4]));
    } else if(!XOS_StrCmp(ppArgv[0], "DelIpc")) {
        if(siArgc < 3) {
            XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "DelIpc missing paramters!!");
            return;
        }
        DelIpc(atoi(ppArgv[1]), atoi(ppArgv[2]));
    } else if(!XOS_StrCmp(ppArgv[0], "SendMsg")) {
        SendUdtMsg(ppArgv[1]);
    }
}

void AddIpc(XS8* localip, XS32 localport, XS8* remoteip, XS32 remoteport)
{
    if(XOS_ipcAddSrc(&_ipcLocalIndex, inet_addr(localip), localport) == -1) {
        XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "Add source ipaddr fail!");
    } else {
        XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "Add source ipaddr success, IpcIndex:%d", _ipcLocalIndex);
    }
    if(XOS_ipcAddDst(_ipcLocalIndex, inet_addr(remoteip), remoteport, 0, &_ipcRemoteIndex) == -1) {
        XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "Add remote ipaddr fail!");
    } else {
        XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "Add remote ipaddr success, IpcIndex:%d", _ipcRemoteIndex);
    }
}

void DelIpc(XS32 localipc, XS32 remoteipc)
{
    if(XOS_ipcDelIpc(localipc, remoteipc) == -1) {
        XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "Del Ipc index fail, localindex:%d, remoteindex:%d", localipc, remoteipc);
    }
}
    
void SendUdtMsg(XS8* msg)
{
    if(_ipcLocalIndex == -1 && _ipcRemoteIndex == -1) {
        XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "Not set local address and remote address!");
        return;
    }
    if(XOS_ipcSendMsg(_ipcLocalIndex, _ipcRemoteIndex, (XU8*)msg, strlen(msg)) == -1) {
        XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "Send message to remote peer fail!");
    } else {
        XOS_CpsTrace(MD(FID_UDT_TEST, PL_INFO), "Send message to remote peer ok!");
    }
}
