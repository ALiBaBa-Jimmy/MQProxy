
#include "auto_test.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 AutoTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 AutoTestInit( XVOID*, XVOID*);
    XS8 AutoTestClose(XVOID *Para1, XVOID *Para2);

    XS8 Test_timerProc( t_BACKPARA* pParam);
    
    void AutoTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

    
#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherAutoTestFid ={
    { "FID_AUTO_TEST",  XNULL, FID_AUTO_TEST,},
    { AutoTestInit, NULL, AutoTestClose,},
    { XNULL, Test_timerProc,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 AutoTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherAutoTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_AUTO_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_AUTO_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "AutoTest", "Auto Test", "");
    XOS_RegistCommand(promptID, AutoTestCmd, "AutoTest", "auto run all test modules", "");
    
    return XSUCC;
}

XS8 AutoTestInit( XVOID* argv1, XVOID* argv2)
{
    MMInfo("AutoTestInit begin");

    MMInfo("AutoTestInit end");
    return XSUCC;
}


XS8 AutoTestClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_FTP_TEST, PL_INFO), "FtpTestClose ...");
    return 0;
}


XS8 Test_timerProc( t_BACKPARA* pParam)
{
    
    XOS_Trace(MD(FID_AUTO_TEST,PL_ERR),"timer %d is timeout", pParam->para1);
    return XSUCC;
}


PTIMER  g_tHandle[100];
t_BACKPARA gt_paRatmp ={1, 0, 0, 0};
void AutoTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{

    int a = 10;
    int i = 0;
    if(a<3)
    {
        XOS_CliExtPrintf(pCliEnv,"%s\n", "create timer failed");
    }
    else if(a<4)
    {
        XOS_CliExtPrintf(pCliEnv,"%s\n", "create timer failed");
    }    
    else if(a<5)
    {
        XOS_CliExtPrintf(pCliEnv,"%s\n", "create timer failed");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"%s\n", "create timer failed");
    }

    for(i = 0;i <100; i++)
    {
        gt_paRatmp.para1 = i;
        if(XSUCC != XOS_TimerCreate(FID_AUTO_TEST, &g_tHandle[i], TIMER_TYPE_ONCE,
            TIMER_PRE_LOW, &gt_paRatmp))
        {
            XOS_Trace(MD(FID_AUTO_TEST,PL_ERR),"%s\n", "create timer failed");

            return XERROR;
        }
        else
        {
            XOS_Trace(MD(FID_AUTO_TEST,PL_ERR), "%s %d  %s", "create timer", i, "success");
        }
    }

    XOS_Trace(MD(FID_AUTO_TEST,PL_ERR),"timer is start 5s");

    for(i = 0; i < 100 ;i++)
    {
        if(XSUCC != XOS_TimerBegin(FID_AUTO_TEST, g_tHandle[i], 5 * 1000))
        {
            XOS_Trace(MD(FID_AUTO_TEST,PL_ERR),"%s\n", "begin timer failed");
            return XERROR;
        }
        else
        {
            XOS_Trace(MD(FID_AUTO_TEST,PL_ERR),"%s %d %s", "begin timer ", i, "success");
        }
    }
    
}

