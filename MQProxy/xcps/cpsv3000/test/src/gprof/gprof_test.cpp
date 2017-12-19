
#include "gprof_test.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 GprofTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 GprofTestInit( XVOID*, XVOID*);
    XS8 GprofTestClose(XVOID *Para1, XVOID *Para2);
    XS8 GprofTestMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 GprofTestMsgOut(t_BACKPARA* para);
    
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherGprofTestFid ={
    { "FID_GPROF_TEST",  XNULL, FID_GPROF_TEST,},
    { GprofTestInit, NULL, GprofTestClose,},
    { GprofTestMsgProc, GprofTestMsgOut,}, eXOSMode, NULL
};

// ----------------------------- MyTest -----------------------------

void NormalExit(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    exit(0);
}


XS32 GprofTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherGprofTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_GPROF_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_GPROF_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList,XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "GprofTest", "GprofTest", "");
    XOS_RegistCommand(promptID, NormalExit, "NormalExit", "exit application normally", "");
    return XSUCC;
}

XS8 GprofTestInit( XVOID*, XVOID*)
{
    MMInfo("GprofTestInit begin");

    
    MMInfo("GprofTestInit end");
    return XSUCC;
}

XS8 GprofTestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_GPROF_TEST, PL_INFO), "GprofTestClose ...");
    
    return 0;
}

XS8 GprofTestMsgProc( XVOID* inMsg, XVOID*)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_GPROF_TEST,PL_ERR),"GprofTestMsgProc()->input param error!");
        return XERROR;
    }
       
    return 0;
}

XS8 GprofTestMsgOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_GPROF_TEST, PL_INFO), "GprofTestMsgProc -> %d", para->para1);

    return 0;
}
