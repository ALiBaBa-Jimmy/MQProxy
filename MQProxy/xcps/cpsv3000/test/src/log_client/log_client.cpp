
#include "log_client.h"


#ifdef __cplusplus
extern "C" {
#endif

PTIMER timerLogclient = XNULL;

XS32 LogCliTest(HANDLE hdir, XS32 argc, XCHAR** argv);

XS8 LogCliInit( XVOID*, XVOID*);
XS8 LogCliClose(XVOID *Para1, XVOID *Para2);
XS8 LogCliMsgProc(XVOID *Para1, XVOID *Para2);
XS8 LogCliTimerProc(t_BACKPARA* para);

void LogCliCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
    
#ifdef __cplusplus
}
#endif



t_XOSFIDLIST g_dispatcherLogClientFid ={
    { "FID_LOG_CLIENT",  XNULL, FID_LOG_CLIENT,},
    { LogCliInit, NULL, LogCliClose,},
    { LogCliMsgProc, LogCliTimerProc,}, eXOSMode, NULL
};

 XU32  MediaBKDRHash( XVOID* param, XS32 paramSize, XS32 hashSize)
{
    XU32 hashKey = 0;
    XU32 ulSeed = 13131; //31、131、1313、13131
    XCHAR* pChar = (XCHAR*)param;
    if(param == XNULLP || paramSize > 10)
    {
        /*参数错误的情况,都挂在第一个位置上*/
        return 0;
    }

    while(*pChar)
    {
        hashKey = hashKey * ulSeed + (*pChar++);
    }

    hashKey = hashKey & 0X7FFFFFFF;

    hashKey = hashKey % ((XU32)hashSize);

    printf("\n%ld\n", hashKey);
    return hashKey;
}

int fun_a(int a, int b)
 {
    if(a == b)
    {
        return 0;
    }
    else
    {
        return 1;
    }
 }

int fun_b(int a)
{
    int b = 0;
    a = a+2;
    return a;
}

int fun_c()
{
    
    if(fun_a(fun_b(3), 4))
    {
        return 0;
    }
}

XSTATIC  XBOOL  IsPrime(  XS32 n  )
{
    XS32  i;
    for (i = 2; i < n/i/*sqrt(n)*/+1; i++)
    {
        if (!(n%i)) 
        {
            return XFALSE;
        }
    }

    return XTRUE;
}


/************************************************************************
* IntFirstPrime
* 功能: 求从n 起的第一个素数
* 输入: n              - 起始的整数
* 输出: 无
* 返回: 求得的素数
************************************************************************/
XSTATIC  XS32  IntFirstPrime(  XS32  n  )
{
    XS32  i = n;

    if (0 == (i & 1))
    {
        i++;
    }
    while (!IsPrime(i))
    {
        i += 2;
    }    
    return i;
}

XS32 next_power_two(XS32 i)
{
    if(i == 0)
    {
        return 1;
    }
    XS32 bit1count = 0;//对i中bit位为1的位进行计数
    XS32 count = 0;//最高不为0的bit位
    while(i != 0)
    {
        if( (i&1) !=0)
        {
            ++bit1count;
        }
        ++count;
        i >>=1;
    }
    return (bit1count == 1)? (1<<(count-1)):(1<<count);
}

typedef enum
{
    eSrvRunStateBegin = -1,
    eSrvRunStateNormal,
    eSrvRunStateAbnormal = 0
}ESrvRunState;

// ----------------------------- MyTest -----------------------------
XS32 LogCliTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    printf("%d, %d, %d", eSrvRunStateBegin, eSrvRunStateNormal, eSrvRunStateAbnormal);


    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherLogClientFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_LOG_CLIENT", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_LOG_CLIENT;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "test", "test modules", "");

    promptID = XOS_RegistCmdPrompt(promptID, "logclient", "logclient", "");

    XOS_RegistCommand(promptID, LogCliCmd, "logclient", "start log client", "logclient");

    return XSUCC;
}

XS8 LogCliInit( XVOID*, XVOID*)
{
    if(XSUCC != XOS_TimerReg(FID_LOG_CLIENT, 1000 ,5, 0))  /*驱动间隔为1秒,精度为100ms*/
    {
        XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "NTPC_Init->XOS_TimerReg() failed\r\n");
        return XERROR;
    }

    return XSUCC;
}

XS8 LogCliClose(XVOID *Para1, XVOID *Para2)
{
    return 0;
}

XS8 LogCliMsgProc( XVOID* inMsg, XVOID*)
{   

    return 0;
}

XS8 LogCliTimerProc(t_BACKPARA* para)
{
    if(NULL == para)
    {
        return XERROR;
    }
    if(0 == para->para1)
    {
        while(1)
        {
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            #if 0
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "fffffffffffffffffffffffffffffffffffffffff\r\nfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
            XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "ffffffffffffffffffffffffffffffffffffff\r\nffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
#endif
            XOS_Sleep(2);
        }
    }

    return 0;
}


void LogCliCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{

    t_PARA timerPara;
    t_BACKPARA backPara;    

    timerPara.fid = FID_LOG_CLIENT;
    timerPara.len = 1000; 
    timerPara.mode = TIMER_TYPE_ONCE;
    timerPara.pre  = TIMER_PRE_LOW;

    backPara.para1 = 0;

    if(XSUCC !=XOS_TimerStart(&timerLogclient,&timerPara,&backPara))
    {
        XOS_Trace(MD(FID_LOG_CLIENT,PL_INFO), "start timerLogclient timer failed\r\n");
        return ;
    }    
}





