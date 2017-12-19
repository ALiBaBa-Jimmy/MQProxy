
#include "nfs_test.h"
#include "xosos.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 NfsTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 NfsTestInit( XVOID*, XVOID*);
    XS8 NfsTestClose(XVOID *Para1, XVOID *Para2);
    XS8 NfsTestMsgPro( XVOID* pMsgP, XVOID* sb);
    
    
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherNfsTestFid ={
    { "FID_NFS_TEST",  XNULL, FID_NFS_TEST,},
    { NfsTestInit, NULL, NfsTestClose,},
    { NfsTestMsgPro, XNULL,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 NfsTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherNfsTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_NFS_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_NFS_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "NfsTest", "Nfs Test", "");
    XOS_RegistCommand(promptID, NfsTestCmd, "TraceLog", "Write log to nfs server", "num message");
    
    return XSUCC;
}

XS8 NfsTestInit( XVOID* argv1, XVOID* argv2)
{
    MMInfo("NfsTestClose begin");

    MMInfo("NfsTestClose end");
    return XSUCC;
}


XS8 NfsTestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_NFS_TEST, PL_INFO), "NfsTestClose ...");
    return 0;
}

XS8 NfsTestMsgPro( XVOID* pMsgP, XVOID* sb)
{
    t_XOSCOMMHEAD* pCpsHead = XNULL;
    
    return 0;
}

void NfsTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    int num = 1;
    int i = 0;
    
    if(siArgc >= 3) {
        num = atoi(ppArgv[1]);
        
        for(i = 0; i < num; i++) {
            XOS_Trace(MD(FID_NFS_TEST,PL_ERR), "NfsTestCmd,msg[%d]:%s", i, ppArgv[2]);
            XOS_Sleep(100);
        }
        
    }
}
