
#include "mem_test.h"


#ifdef __cplusplus
extern "C" {
#endif

    XS32 MemTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 MemTestInit( XVOID*, XVOID*);
    XS8 MemTestClose(XVOID *Para1, XVOID *Para2);
    XS8 MemTestMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 MemTestTimeOut(t_BACKPARA* para);

    void HandleMemCmd(XCONST XS8* cmd, XCONST XS8* arg1 , XCONST XS8* arg2);
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherMemTestFid ={
    { "FID_MEM_TEST",  XNULL, FID_MEM_TEST,},
    { MemTestInit, NULL, MemTestClose,},
    { MemTestMsgProc, MemTestTimeOut,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 MemTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherMemTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_MEM_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_MEM_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "MemTest", "MemTest", "");
    XOS_RegistCommand(promptID, MemTestCmd, "Malloc", "malloc a memory block", "fid size");
    XOS_RegistCommand(promptID, MemTestCmd, "Free", "free a memory block", "fid address");
    
    return XSUCC;
}

XS8 MemTestInit( XVOID* argv1, XVOID* argv2)
{
    MMInfo("MemTestInit begin");

//    HandleMemCmd("Free", "0", "1230000000");
    
    MMInfo("MemTestInit end");
    return XSUCC;
}

XS8 MemTestMsgProc( XVOID* inMsg, XVOID* argv2)
{
    XOS_CpsTrace(MD(FID_MEM_TEST, PL_INFO), "MemTestMsgProc begin");
    XOS_CpsTrace(MD(FID_MEM_TEST, PL_INFO), "MemTestMsgProc end");
    return 0;
}

XS8 MemTestTimeOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_MEM_TEST, PL_INFO), "MemTestTimeOut");

    return 0;
}

XS8 MemTestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_MEM_TEST, PL_INFO), "MemTestClose ...");
    return 0;
}

void MemTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    if(siArgc < 3) {
        return;
    }
    HandleMemCmd(ppArgv[0], ppArgv[1], ppArgv[2]);
}

void HandleMemCmd(XCONST XS8* cmd, XCONST XS8* arg1 , XCONST XS8* arg2)
{
    XS8* memBuf = 0;

    MMInfo("HandleMemCmd begin");
    
    if(XOS_StrCmp(cmd, "Malloc") == 0) {
        memBuf = XOS_MemMalloc(atoi(arg1), atoi(arg2));
        if(!memBuf) {
            XOS_CpsTrace(MD(FID_MEM_TEST, PL_INFO), "malloc memory fail!,size=%d", atoi(arg2));
        }else {
            XOS_CpsTrace(MD(FID_MEM_TEST, PL_INFO), "malloc memory ok!,size=%u,memory:%d", atoi(arg2), (XU32)memBuf);
        }
    } else if(XOS_StrCmp(cmd, "Free") == 0) {
        if(XOS_MemFree(atoi(arg1), atoi(arg2)) == XERROR) {
            MMInfo("free memory fail");
            XOS_CpsTrace(MD(FID_MEM_TEST, PL_INFO), "free memory fail!, fid=%d, memory:%u", atoi(arg1), atoi(arg2));
        } else {
            XOS_CpsTrace(MD(FID_MEM_TEST, PL_INFO), "free memory ok!, fid=%d, memory:%u", atoi(arg1), atoi(arg2));
        }
    }
}