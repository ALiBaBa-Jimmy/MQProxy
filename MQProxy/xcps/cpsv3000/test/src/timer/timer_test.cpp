
#include "timer_test.h"

#define MAX_TIMER_COUNT 1000

int g_begin_timer = 0;

#define MAX_TIMER_LEN 900        //每次建立定时器的最大数量
#define LOW_TIMER_NUM 800        //一个建立低精度定时器的数量
#define HIGH_TIMER_NUM 100    //一次建立高精度定时器的数量

#define LOW_BASE_TIME        1000        //低精度定时器测试最小超时时间  ms
#define LOW_INTERVAL_TIME        1000        //低精度定时器测试超时间隔时间  ms
#define HIGH_BASE_TIME        1000        //高精度定时器测试最小超时时间  ms
#define HIGH_INTERVAL_TIME        1000        //高精度定时器测试超时间隔时间  ms

typedef struct t_timer_unit
{
    PTIMER _timeout[MAX_TIMER_LEN];
    int _ifset;
    int _timer_run_count;
    int _timer_count;
    XU32 _time_len;
    e_TIMERTYPE _timer_mode; 
    struct timeval _tm_begin;
    struct timeval _tm_end;
}t_timer_unit;

XSTATIC t_timer_unit *g_low_timer = NULL;
XSTATIC t_timer_unit *g_high_timer = NULL;

#ifdef __cplusplus
extern "C" {
#endif

    XS32 TimerTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    
    XS8 TimerInit( XVOID*, XVOID*);
    XS8 TimerClose(XVOID *Para1, XVOID *Para2);
    XS8 TimerMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 TimeMsgOut(t_BACKPARA* para);

    void AddTimer(int ishigh, int timelen, int timecount);
    void AutoAddTimer(int ishigh,int timer_len, int timer_count,int index);

    XSTATIC XVOID TimerTest_Task(XVOID* taskNo);

#ifdef __cplusplus
}
#endif

t_XOSFIDLIST g_dispatcherTimerTestFid ={
    { "FID_TIMER_TEST",  XNULL, FID_TIMER_TEST,},
    { TimerInit, NULL, TimerClose,},
    { TimerMsgProc, TimeMsgOut,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 TimerTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherTimerTestFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_TIMER_TEST", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_TIMER_TEST;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGSNUM_IN_QUE;
    stLoginList.stacksize = 1024*100;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "TimerTest", "TimerTest", "");
    XOS_RegistCommand(promptID, AddHighTimer, "AddHighTimer", "add high timer ", "timeout count");
    XOS_RegistCommand(promptID, AddLowTimer, "AddLowTimer", "add low timer ", "timeout count");
    XOS_RegistCommand(promptID, AutoTestLowTimer, "AutoTestLowTimer", "auto test low timer ", "maxcount");
    XOS_RegistCommand(promptID, AutoTestHighTimer, "AutoTestHighTimer", "auto test high timer ", "maxcount");
    XOS_RegistCommand(promptID, RunTimer, "RunTimer", "run timer which has been added", "");
    XOS_RegistCommand(promptID, StopTimer, "StopTimer", "Stop all timer", "");
    XOS_RegistCommand(promptID, GetLocalTimeT, "GetLocalTime", "get local time with pre", "pre");
    XOS_RegistCommand(promptID, GetTickTime, "GetTickTime", "get tick time with pre", "pre");
    XOS_RegistCommand(promptID, TestCreateTime, "CreateTimer", "create some low timer", "timelen");
    XOS_RegistCommand(promptID, TestBeginTime, "BeginTimer", "begin times", "");
    
    return XSUCC;
}

XS8 TimerInit( XVOID*, XVOID*)
{
    MMInfo("MyTestInit begin");
    size_t size_u = sizeof(t_timer_unit);

    g_low_timer = (struct t_timer_unit *)XOS_Malloc(size_u*MAX_TIMER_LEN);
    g_high_timer = (struct t_timer_unit *)XOS_Malloc(size_u*MAX_TIMER_LEN);

    XOS_MemSet(g_low_timer, 0, size_u * MAX_TIMER_LEN);
    XOS_MemSet(g_high_timer, 0, size_u * MAX_TIMER_LEN);

    
    XOS_TimerReg(FID_TIMER_TEST, 10, 10, 0); /*驱动间隔为1秒*/

    MMInfo("MyTestInit end");
    
    return XSUCC;
}

XS8 TimerMsgProc( XVOID* inMsg, XVOID*)
{
    XOS_Trace(MD(FID_TIMER_TEST, PL_INFO), "TEST - RECV");
    return 0;
}

XS8 TimeMsgOut(t_BACKPARA* para)
{
    int index = para->para1;
    int time_out = para->para2;
    struct t_timer_unit* timer_unit = (struct t_timer_unit*)para->para3;
    

    if(timer_unit) {
        if(time_out == (timer_unit[index]._timer_count - 1)) {
            timer_unit[index]._timer_run_count++;
            XOS_GetTimeOfDay((struct timeval*)&timer_unit[index]._tm_end, NULL);
        }
        /*
        timer_unit[index]._time_len += 100;
        if(timer_unit[index]._timer_mode == TIMER_TYPE_ONCE) {
            XOS_TimerBegin(FID_TIMER_TEST, timer_unit[index]._timeout[time_out], timer_unit[index]._time_len);
        }*/
    }

    return 0;
}

XS8 TimerClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "TimerClose ...");
    return 0;
}
//增加高精度定时器
void AddHighTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    int timer_len = 0;
    int timer_count = 0;
    
    if(siArgc >= 3) {
        timer_len = atoi(ppArgv[1]);
        timer_count = atoi(ppArgv[2]);
        AddTimer(1, timer_len, timer_count);
    }
}
//增加低精度定时器
void AddLowTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    int timer_len = 0;
    int timer_count = 0;
    
    if(siArgc >= 3) {
        timer_len = atoi(ppArgv[1]);
        timer_count = atoi(ppArgv[2]);
        AddTimer(0, timer_len, timer_count);
    }
}

void AutoTestLowTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XU32 tmlen = LOW_BASE_TIME;
    XU32 maxcount = 0;
    int i = 0;

    if(siArgc >= 2) {
        maxcount = atoi(ppArgv[1]);
        for(i = 0; i < maxcount; i++, tmlen += LOW_INTERVAL_TIME) {
            AutoAddTimer(0,tmlen, LOW_TIMER_NUM,i);
        }
    }
}

void AutoTestHighTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XU32 tmlen = HIGH_BASE_TIME;
    XU32 maxcount = 0;
    int i = 0;

    if(siArgc >= 2) {
        maxcount = atoi(ppArgv[1]);
        for(i = 0; i < maxcount; i++, tmlen += HIGH_INTERVAL_TIME) {
            AutoAddTimer(1, tmlen, HIGH_TIMER_NUM,i);
        }
    }
}

//接收用户命令，执行低精度定时器
void AddTimer(int ishigh, int timer_len, int timer_count)
{
    struct t_timer_unit* timer_unit = NULL;
    
    if(timer_len >= MAX_TIMER_LEN || timer_count > MAX_TIMER_COUNT) {
        XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "timeout or count too big ");
        return;
    }
    
    if(ishigh) {
        timer_unit = g_high_timer;
    } else {
        timer_unit = g_low_timer;
    }
    timer_unit[timer_len]._timer_run_count = 0;
    timer_unit[timer_len]._timer_count = timer_count;
    timer_unit[timer_len]._ifset = 1;
    timer_unit[timer_len]._timer_mode = TIMER_TYPE_LOOP;

    XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "AddTimer:set timer info ok, timelen=%d,count=%d", timer_len, timer_count);
    
}

//自动批量测试低精度时，添加定时器，设置定时器节点
void AutoAddTimer(int ishigh,int timer_len, int timer_count,int index)
{
    struct t_timer_unit* timer_unit = NULL;
    
    if(index >= MAX_TIMER_LEN || timer_count > MAX_TIMER_COUNT) {
        XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "timeout or count too big ");
        return;
    }
    
    if(ishigh) {
        timer_unit = g_high_timer;
    } else {
        timer_unit = g_low_timer;
    }
    
    timer_unit[index]._timer_run_count = 0;
    timer_unit[index]._timer_count = timer_count;
    timer_unit[index]._ifset = 1;
    timer_unit[index]._timer_mode = TIMER_TYPE_LOOP;

    XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "AddTimer:set timer info ok, index=%d,count=%d", index, timer_count);
    
}

//测试TimeCreate 
void TestCreateTime(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    int len = 0;
    t_BACKPARA backpara;
    
    if(siArgc >= 2) {
    
        g_begin_timer = 0;
        
        len = atoi(ppArgv[1]);

        //for(len = 100; len < 5000; len += 100) {
            struct t_timer_unit* timer_unit = NULL;
        
            if(len >= MAX_TIMER_LEN) {
                XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "timeout or count too big ");
                return;
            }
            
            timer_unit = g_low_timer;
            timer_unit[len]._timer_run_count = 0;
            timer_unit[len]._timer_count = 1;
            timer_unit[len]._ifset = 1;
            timer_unit[len]._timer_mode = TIMER_TYPE_ONCE;
            timer_unit[len]._time_len = len;
            XOS_GetTimeOfDay((struct timeval *)&timer_unit[len]._tm_begin, NULL);
            
            backpara.para1 = len;
            backpara.para2 = 0;
            backpara.para3 = (XU32)g_low_timer;
                    
            XOS_TimerCreate(FID_TIMER_TEST, &timer_unit[len]._timeout[0], TIMER_TYPE_ONCE, TIMER_PRE_LOW, &backpara);
            

            XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "TestCreateTime:create timer ok, len=%d, handle=%d", len, timer_unit[len]._timeout[0]);
        //}
    }
}
//测试 TimeBegin方法
void TestBeginTime(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    struct t_timer_unit* timer_unit = NULL;
    t_XOSTASKID taskId;

    for(int i = 0; i <     MAX_TIMER_LEN; i++) {
        if(g_low_timer[i]._ifset == 1) {
            XOS_TimerBegin(FID_TIMER_TEST, g_low_timer[i]._timeout[0], g_low_timer[i]._time_len);
            XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "TestBeginTime:begin timer ok, len=%d, handle=%d", g_low_timer[i]._time_len, g_low_timer[i]._timeout[0]);
        }
    }
    g_begin_timer = 1;
    XOS_TaskCreate("Tsk_TestTimer", TSK_PRIO_NORMAL, 10000, (os_taskfunc)TimerTest_Task, (XVOID *)0, &taskId);
}

XVOID TimerTest_Task(XVOID* taskNo)
{
    while(1) {
        if(g_begin_timer) {
            for(int i = 0; i <     MAX_TIMER_LEN; i++) {
                if(g_low_timer[i]._ifset == 1) {
                    g_low_timer[i]._time_len += 100;
                    XOS_TimerBegin(FID_TIMER_TEST, g_low_timer[i]._timeout[0], g_low_timer[i]._time_len);
                    XOS_Sleep(g_low_timer[i]._time_len);
                }
            }
        }
    }
}

void RunTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    int i,j;
    t_PARA timerpara;
    t_BACKPARA backpara;
    
    for(i = 0; i < MAX_TIMER_LEN; i++) {
        if(g_high_timer[i]._ifset == 1) {
            // 启动定时器
            timerpara.fid  = FID_TIMER_TEST;
            timerpara.len  = HIGH_BASE_TIME + i * HIGH_INTERVAL_TIME;
            timerpara.mode = TIMER_TYPE_LOOP;
            timerpara.pre  = TIMER_PRE_HIGH;

            g_high_timer[i]._timer_run_count = 0;

            XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "RunTimer: start high timer,timelen=%d, count=%d", timerpara.len, g_high_timer[i]._timer_count);
            
               XOS_GetTimeOfDay((struct timeval *)&g_high_timer[i]._tm_begin, NULL);
               
            for (j = 0; j < g_high_timer[i]._timer_count; ++j) {
                backpara.para1 = i;
                backpara.para2 = j;
                backpara.para3 = (XU32)g_high_timer;
                
                XOS_TimerStart(&g_high_timer[i]._timeout[j], &timerpara, &backpara);
                
            }
        }
    }
    
    for(i = 0; i < MAX_TIMER_LEN; i++) {
        if(g_low_timer[i]._ifset == 1) {
            // 启动定时器
            timerpara.fid  = FID_TIMER_TEST;
            timerpara.len  = LOW_BASE_TIME + i * LOW_INTERVAL_TIME;
            timerpara.mode = TIMER_TYPE_ONCE;
            timerpara.pre  = TIMER_PRE_LOW;

            g_low_timer[i]._timer_run_count = 0;

            XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO), "RunTimer: start low timer,timelen=%d, count=%d", timerpara.len, g_low_timer[i]._timer_count);
            
//               XOS_GetTimeOfDay(&(g_low_timer[i]._tm_begin), NULL);
               
            for (j = 0; j < g_low_timer[i]._timer_count; ++j) {
                backpara.para1 = i;
                backpara.para2 = j;
                backpara.para3 = (XU32)g_low_timer;

                if(j == g_low_timer[i]._timer_count -1)
                {
                    XOS_GetTimeOfDay(&(g_low_timer[i]._tm_begin), NULL);
                }
                XOS_TimerStart(&g_low_timer[i]._timeout[j], &timerpara, &backpara);
                
            }
        }
    }
}

//停止定时器
void StopTimer(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    int i = 0;
    int j = 0;
    double diff_sec = 0.0;
    double diff_msec = 0.0;
    double real_len = 0.0;
    double miss = 0.0;
    double test_sec;
    XS32 test_index = 0;
    test_index = atoi(ppArgv[1]);

    for(i = 0; i < MAX_TIMER_LEN; i++) {
        if(g_high_timer[i]._ifset == 1 && g_high_timer[i]._timer_count > 0 && g_high_timer[i]._timer_run_count > 0) {

            for(j = 0; j < g_high_timer[i]._timer_count; j++) {
                XOS_TimerStop(FID_TIMER_TEST, g_high_timer[i]._timeout[j]);
            }
            
            diff_sec = g_high_timer[i]._tm_end.tv_sec - g_high_timer[i]._tm_begin.tv_sec;
            diff_msec = (g_high_timer[i]._tm_end.tv_usec - g_high_timer[i]._tm_begin.tv_usec) / 1000;
            real_len = (diff_sec*1000 + diff_msec) / g_high_timer[i]._timer_run_count;
            miss = real_len - HIGH_BASE_TIME - i * HIGH_INTERVAL_TIME;
            test_sec = (double)(HIGH_BASE_TIME + i * HIGH_INTERVAL_TIME)/1000;
            if( i == test_index)
            {
                XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO),
                    "result:high timer:%.2f s,  average time:%.2f ms, miss:%.2f ms",
                    test_sec,
                    real_len,
                    miss);
            }
        }
        g_high_timer[i]._ifset = 0;
    }
    
    for(i = 0; i < MAX_TIMER_LEN; i++) {
        if(g_low_timer[i]._ifset == 1 && g_low_timer[i]._timer_count > 0 ) {

            for(j = 0; j < g_low_timer[i]._timer_count; j++) {
                XOS_TimerStop(FID_TIMER_TEST, g_low_timer[i]._timeout[j]);
            }
            if( i == test_index)
            {
                diff_sec = g_low_timer[i]._tm_end.tv_sec - g_low_timer[i]._tm_begin.tv_sec;
                diff_msec = (g_low_timer[i]._tm_end.tv_usec - g_low_timer[i]._tm_begin.tv_usec) / 1000;
                real_len = (diff_sec*1000 + diff_msec) / g_low_timer[i]._timer_run_count;
                miss = real_len - LOW_BASE_TIME - i * LOW_INTERVAL_TIME;
                test_sec = (double)(LOW_BASE_TIME + i * LOW_INTERVAL_TIME)/1000;
                
                XOS_CpsTrace(MD(FID_TIMER_TEST, PL_INFO),
                    "result:low timer:%.1f s,  average time:%.2f ms, miss:%.2f ms",
                    test_sec,
                    real_len,
                    miss);
            }
        }
        g_low_timer[i]._ifset = 0;
    }
}

void GetLocalTimeT(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XU32 sec = 0;
    XU32 msec = 0;
    XS32 pre = 0;
    
    if(siArgc >= 2) {
        pre = atoi(ppArgv[1]);
        
        XOS_GetLocTimeByPri(&sec, &msec, pre);
        
        printf("Get Local Time, sec=%d, msec=%d, pre=%d\n", sec, msec, pre);
        
    }
}

void GetTickTime(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XU32 sec = 0;
    XU32 msec = 0;
    XS32 pre = 0;
    
    if(siArgc >= 2) {
        pre = atoi(ppArgv[1]);
        
        XOS_TicksToSecByPri(XOS_GetSysTicks(), &sec, &msec, pre);
        
        printf("Get Tick Time, sec=%d, msec=%d, pre=%d\n", sec, msec, pre);
        
    }
}


