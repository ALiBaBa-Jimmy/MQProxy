
#include "signal_test.h"


#ifdef __cplusplus
extern "C" {
#endif

    XS32 SignalTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 SignalTestInit( XVOID*, XVOID*);
    XS8 SignalTestClose(XVOID *Para1, XVOID *Para2);

#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherSignalTestFid ={
    { "FID_SIGNAL_TEST",  XNULL, FID_SIGNAL,},
    { SignalTestInit, NULL, SignalTestClose,},
    { NULL, NULL,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 SignalTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherSignalTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_SIGNAL_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_SIGNAL;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "SignalTest", "SignalTest", "");
    XOS_RegistCommand(promptID, SignalTestCmd, "Signal", "raise a signal", "signum");
    
    return XSUCC;
}

XS8 SignalTestInit( XVOID* argv1, XVOID* argv2)
{
    MMInfo("SignalTestInit begin");

    
    MMInfo("SignalTestInit end");
    return XSUCC;
}

XS8 SignalTestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_SIGNAL, PL_INFO), "SignalTestClose ...");
    return 0;
}

void SignalTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    if(siArgc < 2) {
        return;
    }
#ifdef XOS_LINUX
    raise(atoi(ppArgv[1]));
#endif
}
