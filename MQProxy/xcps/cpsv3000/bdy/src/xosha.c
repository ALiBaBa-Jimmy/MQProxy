/*************************************************************
 *
 *   Xinwei Telecom Technology co., ltd. ShenZhen R&D center
 *
 *
 *
 *   filename:ha.c
 *
 *   description:
 *
 *   author:
 *
 *   date:   2008.01.22
 *
 **************************************************************
 *                          history
 *
 **************************************************************
 *    author         date          modification
 *    zengguanqun    2008.01.22    create
 **************************************************************/
#ifdef  __cplusplus
extern  "C"
{
#endif

/*-------------------------------------------------------------
                  包含头文件
--------------------------------------------------------------*/
#include "xosha.h"
#include "xostrace.h"
#include "clishell.h"
#include "xoscfg.h"

#ifdef VXWORKS
#include "vxWorks.h"
#include <taskLib.h>
static t_ha_register_para  ha_callArray[MAX_HA_CALLBACK];
XBOOL g_bHaIsInit=XFALSE;
XBOOL g_bHaEnable=XTRUE;
XS32  g_timezone = 480;
static XS32  g_tspregisterdelaytime=1;

int xos_syslog(void *para1);
extern void syslog_cbinit_entry(int(*FuncEntry1)(void *));

extern void memshow(void);
extern int  msgqshow(void);

//定义信号量
SEM_ID HARevTaskSemId;

//HA任务处理函数
int drv_hac();

// 驱动提供的函数
//返回0成功,其他不成功
extern int setBoardFromMasterToSlave();

//驱动提供的单板重启接口
extern void reset();

//status==0,表明good;status==1,表明bad
extern void setBoardGoodOrBad(int status);

//驱动提供的供平台调用HA注册处理函数
extern void ha_up_inform_entry(int(*FuncEntry1)(int,void *));

extern int  competeMain(int);
extern int setLogicIp(char* ip,char* mask);
extern int getLogicIp(XU32* ip,XU32* mask);
extern int DrvGetIp( XU32 *ip,XU32 *mask,XU32 *gateway);

/**********************************
函数名称    :  HAInit
作者        : ZengGuanQun
设计日期    : 2007年12月18日
功能描述    : 驱动提供的HA初始化接口函数
返回值      : extern void HAInit()
************************************/
extern void  HAInit();

/*设置时间*/
extern void Set_Rtc_Time2(int second,int minute,int hour,int mday,int month,int year,int Weekday);
extern void Get_Rtc_Time2(int *second,int *minute,int *hour,int *mday,int *month,int *year,int *Weekday);
extern XU8  getBoardLocation( void );

/* 点亮RUN灯*/
extern void sysRunLedOn(void);
/* 熄灭RUN灯*/
extern void sysRunLedOff(void);
/* 点亮ALM灯*/
extern void sysAlarmLedOn(void);
/* 熄灭ALM灯*/
extern void sysAlarmLedOff(void);
/*指示接口板0在位*/
extern void IU0OnlineLedOn();
/*指示接口板0不在位*/
extern void IU0OnlineLedOff();
/*指示接口板1在位*/
extern void IU1OnlineLedOn();
/*指示接口板1不在位*/
extern void IU1OnlineLedOff();
/* 判断是否是主板,0:主板;-1:备板*/

/*bit 0表示接口板0，bit1表示接口板1
1表示在位，0表示不在位*/
extern unsigned char IU0AndIU1Online();
/*返回TSP板上的OCT扣板是否在位
参数index:
              1表示1号槽位
              2表示2号槽位
              其他非法，注意这里不是从0计数
返回值:   1表示在位
                     0表示不在位
                     -1表示输入参数错误*/
extern int oct93xxBoardExist(int index);
extern int checkBoardMain(void);

XS32 ha_cliCommandInit(void);
int chassis_tsp_register(void);

#endif

t_xosha_info g_sysha_context= {eHaStateNull,0,0,"NULL","2007-12-01 10:01:01","Ha test default"};

t_chassisinfo g_chassis_board[BOARD_TYPE_MAX]=
{
 {1,0,BOARD_TYPE_CSP,0,3,0,0,0,0,0,0,0,"cps","cps board not registered","0",1,0},
 {1,0,BOARD_TYPE_TSP,0,3,0,0,0,0,0,0,0,"tsp","tsp board not registered","0",1,0},
 {1,0,BOARD_TYPE_OCI,0,3,0,0,0,0,0,0,0,"oci0","oci0 board not registered","0",1,0},
 {1,0,BOARD_TYPE_EPI,0,3,0,0,0,0,0,0,0,"epi0","epi0 board not registered","0",1,0},
 {1,0,BOARD_TYPE_NSP,0,3,0,0,0,0,0,0,0,"nsp","nsp board not registered","0",1,0},
 {1,0,BOARD_TYPE_OCI,0,3,0,0,0,0,0,0,0,"oci1","oci1 board not registered","0",1,0},
 {1,0,BOARD_TYPE_EPI,0,3,0,0,0,0,0,0,0,"epi1","epi1 board not registered","0",1,0},
 {1,0,BOARD_TYPE_OCT,0,3,0,0,0,0,0,0,0,"oct","oct board not registered","0",1,0},
 {1,0,BOARD_TYPE_AMP600,0,3,0,0,0,0,0,0,0,"amp600","amp600 0 board not registered","0",1,0},
 {1,0,BOARD_TYPE_AMP600,0,3,0,0,0,0,0,0,0,"amp600","amp600 1 board not registered","0",1,0}
};
/**********************************
函数名称    : ha_getboardslot
作者        : Jeff.Zeng
设计日期    : 2008年1月22日
功能描述    : 得到当前所在的板位号
返回值      : XU32
************************************/
XU32 ha_getboardslot()
{
  XU8 BoardSlot=99;
#ifdef VXWORKS
  BoardSlot=getBoardLocation();
#endif
  //XOS_Trace(MD(FID_COMM, PL_INFO),"Get current board location is %d",BoardSlot);
  //printf("call getBoardLocation return value %d.\r\n",BoardSlot);
  return BoardSlot;
}
/**********************************
函数名称    : chassis_board_check
作者        : Jeff.Zeng
设计日期    : 2008年1月22日
功能描述    : 检查配置下发是否正确
返回值      : XU32
************************************/
int chassis_board_check(t_chassisinfo board_info)
{
    XS32 iSlotNo=0;
    XU32 iBoardType=board_info.boardType;
    int  iPosRet=-1;
    iSlotNo=ha_getboardslot();
    if(iSlotNo <0 || iSlotNo > BOARD_VTA_MAXNUM)
    {
      return iPosRet;
    }
    if(iBoardType < BOARD_TYPE_MAX && iBoardType >=0)
    {
        switch(iBoardType)
        {
            case BOARD_TYPE_CSP:
                iPosRet=0;
                break;
                case BOARD_TYPE_TSP:
                if( iSlotNo== board_info.SlotNo)
                {
                    iPosRet = 1;
                }
                break;
            case BOARD_TYPE_OCI:
                if(board_info.SubId== 0)
                {
                 iPosRet = 2;
                }else if(board_info.SubId == 1)
                {
                 iPosRet = 5;
                }
                break;
            case BOARD_TYPE_EPI:
                if(board_info.SubId== 0)
                {
                 iPosRet = 3;
                }else if(board_info.SubId == 1)
                {
                 iPosRet = 6;
                }
                break;
            case BOARD_TYPE_NSP:
                iPosRet = 4;
                break;
            case BOARD_TYPE_OCT:
                iPosRet = 7;
                break;
            case BOARD_TYPE_AMP600:
                if(board_info.SubId== 0)
                {
                   iPosRet = 8;
                }else if(board_info.SubId == 1)
                {
                   iPosRet = 9;
                }
                break;

            default:
                break;
        }
    }
    if(iPosRet <0)
    {
       XOS_Trace(MD(FID_COMM, PL_INFO), "chassis_board_check failed,slot %d;type=%d,subid=%d.",
       board_info.SlotNo,board_info.boardType,board_info.SubId);
    }
    return iPosRet;

}

#ifdef VXWORKS
/**********************************
函数名称    : ha_state_get
作者        : ZengGuanQun
设计日期    : 2007年12月18日
功能描述    : XCPS提供给驱动HA管理模块的回调函数,BSP发生切换时,快速通知该接口
参数        : int cmd;消息类型:用户消息和驱动消息
参数        : void * para 传入接口参数
返回值      : int
************************************/
int ha_state_get(int cmd, void * para)
{
    t_xosha_info * req=NULL;
    int result=-1;
    req = (t_xosha_info *)para;
    if(!req)
    {
      return -1;
    }
    memcpy(&g_sysha_context,req,sizeof(t_xosha_info));
    switch(cmd)
    {
        case eHaCmdUser:
             //do xos/cnms operation
             //logMsg("hello,ha_state_get eHaCmdUser call coming\n",0,0,0,0,0,0);
             break;

        case eHaCmdDriver:
             //do driver operation
             //logMsg("hello,ha_state_get eHaCmdDriver call coming\n",0,0,0,0,0,0);
             break;
        default:
             //logMsg("hello,ha_state_get unknown switch msg coming\n",0,0,0,0,0,0);
             break;
    }
    semGive(HARevTaskSemId);
    return result;
}
int ha_getswitchtime(char *datet)
{
    time_t tt = time(NULL);
    struct tm *mm = localtime(&tt);
    if ( XNULLP== datet )
    {
        return XERROR;
    }
    sprintf(datet, "%04d-%02d-%02d %02d:%02d:%02d",
    mm->tm_year+1900,
    mm->tm_mon+1,
    mm->tm_mday,
    mm->tm_hour,
    mm->tm_min,
    mm->tm_sec);
    return XSUCC;
}

int xos_haopen()
{
   g_bHaEnable = XTRUE;
   return 0;
}

int xos_haclose()
{
   g_bHaEnable = XFALSE;
   return 0;
}

/**********************************
函数名称    : drv_hac
作者        : ZengGuanQun
设计日期    : 2007年12月18日
功能描述    : HA任务处理函数
返回值      : int
************************************/
int drv_hac()
{
//    if(g_sysha_context.hastate == eHaStateActive )
//    {
//      XOS_Sleep(1000);
//    }
    while(1)
    {
        if(semTake (HARevTaskSemId, WAIT_FOREVER) == 0)
        {
           //printf("cps ha processor get sem,processing...!\r\n");
           switch(g_sysha_context.hastate)
           {
             case eHaStateNull:
                  //printf("board status null\r\n");
                  continue;
                  break;
             case eHaStateActive:
                  //printf("master board\r\n");
                  break;
             case eHaStateStandby:
                  //printf("slave board\r\n");
                  break;
             case eHaStateSingle: /*上层发送的事件*/
                  break;
             default:
                  continue;
                  break;
           }
           if(g_sysha_context.hastate ==eHaStateNull )
           {
              //printf("cps ha processor get sem,ha switch finished!\r\n");
              continue;
           }
           //modify switch time
           ha_getswitchtime(g_sysha_context.ocurtime);
           if(g_bHaEnable)
           {
              ha_call_callbacks(g_sysha_context.hastate,(void *)&g_sysha_context);
           }
           //printf("cps ha processor get sem,ha switch finished!\r\n");
        }
    }
}

typedef XS32 (*HAReadyCallback)(XVOID);
HAReadyCallback g_haReadyCallback = NULL;

/**********************************
函数名称    : xos_hainit
作者        : ZengGuanQun
设计日期    : 2007年12月18日
功能描述    : 平台提供的HA初始化函数,执行驱动的一系列操作起动HA管理
返回值      : int
************************************/
int xos_hainit()
{
    if(g_bHaIsInit)
    {
        printf("ha service has been initialized already!\r\n");
        return XSUCC;
    }
    //memset(ha_callArray, 0x0, sizeof(ha_callArray));
    /*create XCPS HA semphore*/
    HARevTaskSemId = semBCreate(SEM_Q_FIFO,SEM_EMPTY);

    /*create XCPS HA process task*/
    taskSpawn("Tsk_hac",75,0,204800,(FUNCPTR)drv_hac,0,0,0,0,0,0,0,0,0,0);

    //competeMain(0);

    /*register XCPS HA process func*/
    ha_up_inform_entry(ha_state_get);
    syslog_cbinit_entry(xos_syslog);

    /*call driver HA init interface*/
    HAInit();

    g_bHaIsInit = XTRUE;
    ha_cliCommandInit();

    if (NULL != g_haReadyCallback) /*上层应用在ha中的回调*/
    {
        g_haReadyCallback();
    }

    printf("xos_hainit initialized ok.\r\n");
    return XSUCC;
}

/////////Ha callback/////////
/**********************************
函数名称    : ha_call_callbacks
作者        : ZengGuanQun
设计日期    : 2007年12月18日
功能描述    :
参数        : int cmd
参数        : void * hainfo_arg
返回值      : int
************************************/
int ha_call_callbacks(int cmd, void * hainfo_arg)
{
    int i=0;
    t_ha_register_para *order_scp;
    int count = 0;
    /*
     * for each registered callback
     */
    for(i=0;i<MAX_HA_CALLBACK;i++)
    {
       order_scp =&ha_callArray[i];
       if(order_scp->module_id >0)
       {
           //printf("ha callback:calling module_id %d callback.\r\n",order_scp->module_id);
          (*(order_scp->p_notify_cbf))(cmd,hainfo_arg,order_scp->app_arg);
           count++;
       }
    }
    //printf("ha callback:total %d callbacks called.\r\n",count);
    return XSUCC;
}

/*2008-01-22
增加与机框管理相关的接口,
包括物理硬件特性和状态信息
*/

/**********************************
函数名称    : ha_setsystime
作者        : Jeff.Zeng
设计日期    : 2008年1月22日
功能描述    :
参数        : XCHAR* strTimeIn 18个字符的时间填充缓冲,时间格式yyyymmdd-hhmmss-wd
返回值      : XS32
************************************/
XS32 ha_setsystime(XCHAR* strTimeIn)
{
    struct tm InTm = {0,0,0,0,0,0,0,0,0};
    char *TimeHead;
    char strBak[20]={0};
    char Year[5];
    char Month[3];
    char Day[3];
    char s_hour[3];
    char s_minute[3];
    char s_second[3];
    char s_wday[3];
    int  ilen=0;
    if(!strTimeIn)
    {
       printf("usage:ha_setsystime yyyymmdd-hhmmss-wd\r\n");
       return XERROR;
    }
    ilen= strlen(strTimeIn);
    if(ilen != 18)
    {
       printf("ERROR:input timestr is wrong,usage: ha_setsystime \"yyyymmdd-hhmmss-wd\"\r\n");
       return XERROR;
    }
    strTimeIn[18]=0x0;
    //备份输入日期
    strcpy(strBak,strTimeIn);

    //分离出输入日期的年份
    TimeHead = strBak;
    strncpy(Year,TimeHead,4);
    Year[4] = 0;
    InTm.tm_year = atoi(Year);

    //分离出输入日期的月份
    TimeHead = &strBak[4];
    strncpy(Month,TimeHead,2);
    Month[2] = 0;
    InTm.tm_mon = atoi(Month);

    //分离出输入日期的日期
    TimeHead = &strBak[6];
    strncpy(Day,TimeHead,2);
    Day[2] = 0;
    InTm.tm_mday = atoi(Day);

    //分离出输入日期的时
    TimeHead = &strBak[9];
    strncpy(s_hour,TimeHead,2);
    s_hour[3] = 0;
    InTm.tm_hour= atoi(s_hour);

    //分离出输入日期的分
    TimeHead = &strBak[11];
    strncpy(s_minute,TimeHead,2);
    s_minute[2] = 0;
    InTm.tm_min= atoi(s_minute);

    //分离出输入日期的秒
    TimeHead = &strBak[13];
    strncpy(s_second,TimeHead,2);
    s_second[2] = 0;
    InTm.tm_sec= atoi(s_second);

    //分离出输入日期的wday
    TimeHead = &strBak[16];
    strncpy(s_wday,TimeHead,2);
    s_wday[2] = 0;
    InTm.tm_wday= atoi(s_wday);

    Set_Rtc_Time2(InTm.tm_sec,InTm.tm_min,InTm.tm_hour,InTm.tm_mday,InTm.tm_mon,InTm.tm_year,InTm.tm_wday);
    XOS_Trace(MD(FID_COMM, PL_INFO),"HA set systime ok!");

    return XSUCC;
}

/**********************************
函数名称    : ha_getsystime
作者        : Jeff.Zeng
设计日期    : 2008年1月22日
功能描述    : 获取系统时间
返回值      : XCHAR* 18个字符时间格式yyyymmdd-hhmmss-wd
************************************/
XCHAR* ha_getsystime()
{
    static XCHAR strTimeOut[19]={0};
    struct tm outTm = {0,0,0,0,0,0,0,0,0};
    if(!strTimeOut)
    {
       return "NULL";
    }
    memset(strTimeOut,0x0,sizeof(strTimeOut));
    Get_Rtc_Time2(&outTm.tm_sec,&outTm.tm_min,&outTm.tm_hour,&outTm.tm_mday,&outTm.tm_mon,&outTm.tm_year,&outTm.tm_wday);
    sprintf(strTimeOut, "%04d%02d%02d-%02d%02d%02d-%02d",
    outTm.tm_year,
    outTm.tm_mon,
    outTm.tm_mday,
    outTm.tm_hour,
    outTm.tm_min,
    outTm.tm_sec,
    outTm.tm_wday);
    //printf("HA get systime is %s.\r\n",strTimeOut);
    //XOS_Trace(MD(FID_COMM, PL_INFO),"HA get systime %s!",strTimeOut);

    return strTimeOut;
}
int ha_oam_settime(int parain)
{
    XCHAR strssTimeOut[20]={0};
    t_XOSTT board_now=0;
    t_XOSTT now = 0;
    t_XOSTD oam_imte;
    XS32 timezone_len=(g_timezone*60);
    parain+=timezone_len;
    now = parain;
    XOS_Time(&board_now);
    if((board_now - now ) <100)
    {
      return 0;
    }
    if((now - board_now ) <100)
    {
      return 0;
    }
    XOS_LocalTime(&now, &oam_imte);
    sprintf(strssTimeOut, "%04d%02d%02d-%02d%02d%02d-%02d",
                       oam_imte.dt_year+1900, oam_imte.dt_mon+1,
                       oam_imte.dt_mday, oam_imte.dt_hour,
                       oam_imte.dt_min, oam_imte.dt_sec,oam_imte.dt_wday);
    strssTimeOut[18]=0x0;
    ha_setsystime(strssTimeOut);
    memset(strssTimeOut,0x0,sizeof(strssTimeOut));
    sprintf(strssTimeOut, "%04d%02d%02d-%02d%02d%02d-%02d%02d%04d",
                       oam_imte.dt_year+1900, oam_imte.dt_mon+1,
                       oam_imte.dt_mday, oam_imte.dt_hour,
                       oam_imte.dt_min, oam_imte.dt_sec,
                       oam_imte.dt_isdst,oam_imte.dt_wday,oam_imte.dt_yday);

    printf("time %d , %s.\r\n",parain,strssTimeOut);

    return 0;  
}

XCHAR* ha_oam_gettime()
{
    static XCHAR oamTimeOut[19]={0};
    t_XOSTT board_now=0;
    t_XOSTD oam_imte;
    XOS_Time(&board_now);
    XOS_LocalTime(&board_now, &oam_imte);
    sprintf(oamTimeOut,"%04d-%02d-%02d %02d:%02d:%02d",
                       oam_imte.dt_year+1900,oam_imte.dt_mon+1,
                       oam_imte.dt_mday,oam_imte.dt_hour,
                       oam_imte.dt_min,oam_imte.dt_sec);
    return oamTimeOut;  
}
/**********************************
函数名称    : ha_ledrun
作者        : Jeff.Zeng
设计日期    : 2008年1月22日
功能描述    :
参数        : XS32 LedType 点灯类型1:运行灯;2:告警灯
参数        : XBOOL bActive 1:点灯;0:熄灯
返回值      : XS32
************************************/
XS32 ha_ledrun(XS32 LedType,XBOOL bActive)
{
    switch(LedType)
    {
       case XRUN_LED:
            {
                if(bActive)
                {
                    sysRunLedOn();
                }else
                {
                    sysRunLedOff();
                }
            }
            break;
       case XALARM_LED:
            {
                if(bActive)
                {
                    sysAlarmLedOn();
                }else
                {
                    sysAlarmLedOff();
                }
            }
            break;
       case XEPI_LED:
            {
                if(bActive)
                {
                    IU0OnlineLedOn();
                }else
                {
                    IU0OnlineLedOff();
                }
            }
            break;
       case XOCT_LED:
            {
                if(bActive)
                {
                    IU1OnlineLedOn();
                }else
                {
                    IU1OnlineLedOff();
                }
            }
            break;
       default:
            //XOS_Trace(MD(FID_COMM,PL_INFO),"HA unknow led request.");
            break;
    }
    if(++g_tspregisterdelaytime % 60 == 0 )
    {
         chassis_tsp_register();
    }
    return XSUCC;
}

/**********************************
函数名称    : ha_testled
作者        : Jeff.Zeng
设计日期    : 2008年1月31日
功能描述    : 测试函数
参数        : XU32 millisecond
返回值      : XS32
************************************/
XS32 ha_testled(XU32 millisecond )
{
   int i=0;
   if(millisecond <100 || millisecond >10000)
   {
     millisecond =200;
   }

   while(i<60)
   {
      ha_ledrun(XRUN_LED,XTRUE);
      ha_ledrun(XALARM_LED,XFALSE);
      XOS_Sleep(millisecond);
      ha_ledrun(XRUN_LED,XFALSE);
      ha_ledrun(XALARM_LED,XTRUE);
      XOS_Sleep(millisecond);
      i++;
   }
   i=0;
   while(i<60)
   {
      ha_ledrun(XEPI_LED,XTRUE);
      ha_ledrun(XOCT_LED,XFALSE);
      XOS_Sleep(millisecond);
      ha_ledrun(XEPI_LED,XFALSE);
      ha_ledrun(XOCT_LED,XTRUE);
      XOS_Sleep(millisecond);
      i++;
   }
   ha_ledrun(XRUN_LED,XTRUE);
   ha_ledrun(XALARM_LED,XFALSE);
   ha_ledrun(XEPI_LED,XFALSE);
   ha_ledrun(XOCT_LED,XFALSE);
   return XSUCC;
}

/**********************************
函数名称    : ha_checkBoardMain
作者        : Jeff.Zeng
设计日期    : 2008年1月31日
功能描述    : 返回当前主备状态
返回值      : XS32
主板:value = 0 = 0x0
备板:value = -1 = 0xffffffff
************************************/
XS32 ha_checkBoardMain()
{
  return checkBoardMain();
}
/**********************************
函数名称    : ha_getBoardEPI
作者        : Jeff.Zeng
设计日期    : 2008年2月15日
功能描述    : 返回当前主板的接口板(E1)状态
返回值      : XS32
value = 0 = 都不在位
value = 1 = 0板在位
value = 2 = 1板在位
value = 3 = 两板都在位
************************************/
XS32 ha_getBoardEPI()
{
    XS32 slot=0;
    XU8 sta;
    sta = IU0AndIU1Online();
    if((sta & 0x01) == 0)
    {
      //0板不在位
    }else
    {
      //0板在位
      slot = 1;
    }
    if(((sta & 0x02) >> 1) == 0)
    {
      //1板不在位
    }else
    {
      //1板在位
      slot += 2;
    }
    return slot;
}
/**********************************
函数名称    : ha_getBoardOCT
作者        : Jeff.Zeng
设计日期    : 2008年2月15日
功能描述    : 返回当前主板的扣板(OCT)状态
返回值      : XS32
value = 0 = 都不在位
value = 1 = 0板在位
value = 2 = 1板在位
value = 3 = 两板都在位
************************************/
XS32 ha_getBoardOCT()
{
    XS32 slot=0;
    if(!oct93xxBoardExist(1))
    {
      //0板不在位
    }else
    {
      //0板在位
       slot = 1;
   }
    if(!oct93xxBoardExist(2))
    {
      //1板不在位
    }else
    {
      //1板在位
      slot += 2;
    }
    return slot;
}
XS32 getBoardIp( XU32 *ip,XU32 *mask,XU32 *gateway)
{
    return DrvGetIp(ip,mask,gateway);
}

//////////HA CLI Interface////////
/**********************************
函数名称    : ha_cliResetBoard
作者        : Jeff.Zeng
设计日期    : 2008年1月31日
功能描述    : 复位单板
参数        : CLI_ENV* pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID
************************************/
XVOID ha_cliResetBoard(CLI_ENV* pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    if(siArgc==2)
    {
        if(0 != XOS_StrCmp(ppArgv[1], "5G"))
        {
            XOS_CliExtPrintf(pCliEnv,"input parameter is wrong!\r\n");
            return;
        }
    }else
    {
        return;
    }
    reset();
    return;
}

/**********************************
函数名称    : ha_cliShow
作者        : Jeff.Zeng
设计日期    : 2008年1月31日
功能描述    : 显示单板状态信息
             需要显示的信息包括如下
             1.主备板
             2.板业务运行状态
             3.当前Slot号
             4.对板连接状态
             5.对板的HA状态
             6.A,B网口及串口连接状态
参数        :  CLI_ENV* pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID
************************************/
XVOID ha_cliShow( CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv )
{
   XOS_CliExtPrintf(pCliEnv,  "--------------Ha system state summary--------------\r\n");
   XOS_CliExtPrintf(pCliEnv,  "Board slot num: %d\r\n",ha_getboardslot());
   switch(g_sysha_context.hastate)
   {
      case eHaStateNull:
           XOS_CliExtPrintf(pCliEnv,"HA state: board state is null\r\n");
           break;
      case eHaStateActive:
           XOS_CliExtPrintf(pCliEnv,"HA state: master board\r\n");
           break;
      case eHaStateStandby:
           XOS_CliExtPrintf(pCliEnv,"HA state: standby board\r\n");
           break;
      default:
           XOS_CliExtPrintf(pCliEnv,"HA state: board state is unknown\r\n");
           break;
   }
   XOS_CliExtPrintf(pCliEnv,  "Board current systime: %s\r\n",ha_getsystime());
   XOS_CliExtPrintf(pCliEnv,  "--------------Ha system state summary--------------\r\n");
   return;
}
/**********************************
函数名称    : ha_cliHaAppRegShow
作者        : Jeff.Zeng
设计日期    : 2008年1月31日
功能描述    : 显示注册HA服务的模块号
参数        :  CLI_ENV* pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID
************************************/
XVOID ha_cliHaAppRegShow( CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv )
{
    int i=0;
    t_ha_register_para *order_scp;
    /*
     * for each registered callback
     */
    XOS_CliExtPrintf(pCliEnv,  "--------------Ha registered moudule summary--------\r\n");
    for(i=0;i<MAX_HA_CALLBACK;i++)
    {
       order_scp =&ha_callArray[i];
       if(order_scp->module_id >0)
       {
           XOS_CliExtPrintf(pCliEnv,"Module ID=%d\r\n",order_scp->module_id);
       }
    }
    XOS_CliExtPrintf(pCliEnv,  "--------------Ha registered moudule summary--------\r\n");
   return;
}
/**********************************
函数名称    : ha_cliSetSystemTime
作者        : Jeff.Zeng
设计日期    : 2008年1月31日
功能描述    : 设置单板时间
参数        : CLI_ENV* pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID
************************************/
XVOID ha_cliSetSystemTime(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv )
{
    XCHAR strTimeIn[64]={0};
    if(siArgc != 2)
    {
        XOS_CliExtPrintf(pCliEnv,"wrong parameter!\r\n usage:setsystime 20080131-090019-05\r\n");
        return;
    }
    if(XOS_StrLen(ppArgv[1]) != 18)
    {
        XOS_CliExtPrintf(pCliEnv,"wrong parameter!\r\n usage:setsystime 20080131-090019-05\r\n");
        return;
    }
    XOS_MemCpy(strTimeIn,ppArgv[1],18);
    strTimeIn[18]=0x0;
    ha_setsystime(strTimeIn);
    XOS_CliExtPrintf(pCliEnv,"set systime %s\r\n",strTimeIn);
    return;
}
/**********************************
函数名称    : ha_chasssisShow
作者        : Jeff.Zeng
设计日期    : 2008年1月31日
功能描述    : 显示机框注册信息
参数        : CLI_ENV* pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID
************************************/
XVOID ha_clichasssisShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv )
{
    int i=0;
    t_chassisinfo board_info;
    XOS_CliExtPrintf(pCliEnv,  "--------------chassis board registered moudule summary--------\r\n");
    for(i=0;i<BOARD_TYPE_MAX;i++)
    {
        if( g_chassis_board[i].registerState == 1)
        {
            board_info = g_chassis_board[i];
            XOS_CliExtPrintf(pCliEnv,"ChassisId = %d       \r\n",board_info.ChassisId);
            XOS_CliExtPrintf(pCliEnv,"SlotNo = %d          \r\n",board_info.SlotNo);
            switch(board_info.boardType)
            {
               case  BOARD_TYPE_CSP:
                     XOS_CliExtPrintf(pCliEnv,"boardType =csp board\r\n");
                     break;
               case  BOARD_TYPE_TSP:
                     XOS_CliExtPrintf(pCliEnv,"boardType =tsp board\r\n");
                     break;
               case  BOARD_TYPE_OCI:
                     XOS_CliExtPrintf(pCliEnv,"boardType =oci board\r\n");
                     break;
               case  BOARD_TYPE_EPI:
                     XOS_CliExtPrintf(pCliEnv,"boardType =epi board\r\n");
                     break;
               case  BOARD_TYPE_NSP:
                     XOS_CliExtPrintf(pCliEnv,"boardType =nsp board\r\n");
                     break;
               case  BOARD_TYPE_OCT:
                     XOS_CliExtPrintf(pCliEnv,"boardType =oct board\r\n");
                     break;
               case  BOARD_TYPE_AMP600:
                     XOS_CliExtPrintf(pCliEnv,"boardType =amp600 board\r\n");
                     break;
               default:
                     XOS_CliExtPrintf(pCliEnv,"boardType = %d       \r\n",board_info.boardType);
                     break;
            }
            //XOS_CliExtPrintf(pCliEnv,"boardType = %d       ",board_info.boardType);
            XOS_CliExtPrintf(pCliEnv,"SubId = %d           \r\n",board_info.SubId);
            switch(board_info.HAMode)
            {
               case  1:
                     XOS_CliExtPrintf(pCliEnv,"HAMode = one_and_one\r\n");
                     break;
               case  2:
                     XOS_CliExtPrintf(pCliEnv,"HAMode = n_and_m\r\n");
                     break;
               case  3:
                     XOS_CliExtPrintf(pCliEnv,"HAMode = onlyone\r\n");
                     break;
                default:
                     XOS_CliExtPrintf(pCliEnv,"HAMode = %d          \r\n",board_info.HAMode);
                     break;
            }
            switch(board_info.HAState)
            {
               case  0:
                     XOS_CliExtPrintf(pCliEnv,"HAState = Initial\r\n");
                     break;
               case  1:
                     XOS_CliExtPrintf(pCliEnv,"HAState = master board\r\n");
                     break;
               case  2:
                     XOS_CliExtPrintf(pCliEnv,"HAState = slave board\r\n");
                     break;
               case  3:
                     XOS_CliExtPrintf(pCliEnv,"HAState = reserved\r\n");
                     break;
                default:
                     XOS_CliExtPrintf(pCliEnv,"HAState = %d         \r\n",board_info.HAState);
                     break;
            }
            if(board_info.RunState ==  0)
            {
               XOS_CliExtPrintf(pCliEnv,"RunState = board service good       \r\n");
            }else if(board_info.RunState ==  1)
            {
               XOS_CliExtPrintf(pCliEnv,"RunState = board service bad        \r\n");
            }else
            {
               XOS_CliExtPrintf(pCliEnv,"RunState = %d        \r\n",board_info.RunState);
            }
            XOS_CliExtPrintf(pCliEnv,"PhyIp = 0x%x         \r\n",board_info.PhyIp);
            XOS_CliExtPrintf(pCliEnv,"RoutingGateway =0x%x \r\n",board_info.RoutingGateway);
            XOS_CliExtPrintf(pCliEnv,"PhyNetmask = 0x%x    \r\n",board_info.PhyNetmask);
            XOS_CliExtPrintf(pCliEnv,"LogicIp = 0x%x       \r\n",board_info.LogicIp);
            XOS_CliExtPrintf(pCliEnv,"LogicIPmask = 0x%x   \r\n",board_info.LogicIPmask);
            XOS_CliExtPrintf(pCliEnv,"Version = %s    \r\n",board_info.Version);
            XOS_CliExtPrintf(pCliEnv,"Desc= %s        \r\n",board_info.Desc);
            if(board_info.boardType == BOARD_TYPE_TSP)
            {
               XOS_CliExtPrintf(pCliEnv,"board time= %s        \r\n",ha_oam_gettime());
            }
            XOS_CliExtPrintf(pCliEnv,"\r\n\r\n");
        }
    }
    XOS_CliExtPrintf(pCliEnv,  "--------------chassis board registered moudule summary--------\r\n");
   return;
}
XVOID ha_cliHaSwitch(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XBOOL bOpen=XFALSE;
    if(siArgc==2)
    {
        if(XOS_StrCmp(ppArgv[1], "on")==0)
        {
            bOpen=XTRUE;
        }else if(XOS_StrCmp(ppArgv[1], "off")==0)
        {
            bOpen=XFALSE;
        }else
        {
            XOS_CliExtPrintf(pCliEnv,"wrong parameter,usage:haswitch <on|off>\r\n");
            return;
        }
    }else
    {
        XOS_CliExtPrintf(pCliEnv,"wrong parameter,usage:haswitch <on|off>\r\n");
        return;
    }
    if(bOpen)
    {
        XOS_CliExtPrintf(pCliEnv,"haswitch on\r\n");
        xos_haopen();
    }else
    {
        XOS_CliExtPrintf(pCliEnv,"haswitch off\r\n");
        xos_haclose();
    }
    return;
}
/**********************************
函数名称    : ha_command_init
作者        : Jeff.Zeng
设计日期    : 2008年1月31日
功能描述    : 注册HA的CLI服务
返回值      : XS32
************************************/
XS32 ha_cliCommandInit(void)
{
    XS32 ret=0,ret1=0;
    XS32 reg_result;
    ret = XOS_RegistCmdPrompt( SYSTEM_MODE, "plat", "plat", "no parameter" );
    if(ret<0)
    {
        return XERROR;
    }
    ret1 = XOS_RegistCmdPrompt(ret,"ha","-d- HA","no parameter");
    if ( 0 >= ret1)
    {
        printf("failed to register plat/ha command prompt.\r\n");
        return XERROR;
    }
    reg_result =XOS_RegistCommand(ret1,ha_cliResetBoard,"resetboard","resetboard","resetboard");
    if(reg_result < 0)
    {
        return XERROR;
    }
    reg_result =XOS_RegistCommand(ret1,ha_cliShow,"showha","showha","showha");
    if(reg_result < 0)
    {
        return XERROR;
    }
    reg_result =XOS_RegistCommand(ret1,ha_cliHaAppRegShow,"hareg","hareg","hareg");
    if(reg_result < 0)
    {
        return XERROR;
    }
    reg_result =XOS_RegistCommand(ret1,ha_cliSetSystemTime,"setsystime","setsystime","setsystime");
    if(reg_result < 0)
    {
        return XERROR;
    }

    reg_result =XOS_RegistCommand(ret1,ha_clichasssisShow,"chassisshow","chassisshow","chassisshow");
    if(reg_result < 0)
    {
        return XERROR;
    }
    reg_result =XOS_RegistCommand(ret1,ha_cliHaSwitch,"haswitch","haswitch","haswitch");
    if(reg_result < 0)
    {
        return XERROR;
    }

    return XSUCC;
}
//////////////////
#endif /* VXWORKS */

/**********************************
函数名称    : ha_app_register
作者        :
设计日期    : 2007年12月18日
功能描述    : 平台提供给应用注册HA服务的回调函数的API接口
参数        : t_ha_para
参数sub1    : int module_id:应用注册的模块号
参数sub2    : HA_CALLBACK_FUNC * new_callback:应用回调函数
注意事项    : new_callback =>应用注册的回调函数必须是非阻塞式设计或异步方式
            : 最好通过消息驱动,严禁阻塞式设计,
            : 回调函数的实现必须简单<30行,高效<30ms
            : 严禁调用printf,或第三方的API接口,不使用锁,信号量等机制
参数sub3    : void *arg:注册时传入的参数
返回值      : int
************************************/
int ha_app_register(t_ha_register_para *t_ha_para)
{
 #ifdef VXWORKS
    int i=-1;
    int module_id;
    t_ha_register_para *newscp = XNULL;
    if(!t_ha_para)
    {
      XOS_Trace(MD(FID_COMM, PL_ERR),"invalid parameter!");
      return XERROR;
    }
    module_id=t_ha_para->module_id;
    if(module_id <=0)
    {
      XOS_Trace(MD(FID_COMM, PL_ERR),"invalid module %d register ha callback failed!",module_id);
      return XERROR;
    }
    if(!t_ha_para->p_notify_cbf)
    {
      XOS_Trace(MD(FID_COMM, PL_ERR),"fuck,how can register ha callback function name with null.");
      return XERROR;
    }
    for(i=0;i<MAX_HA_CALLBACK;i++)
    {
       newscp=&ha_callArray[i];
       if(newscp->module_id == module_id)
       {
          break;
       }
       if(newscp->module_id == 0)
       {
          break;
       }
    }
    if(i < MAX_HA_CALLBACK)
    {
        newscp=&ha_callArray[i];
        newscp->module_id = module_id;
        ha_callArray[i].module_id=module_id;
        if(t_ha_para->app_arg)
        {
          newscp->app_arg = t_ha_para->app_arg;
        }
        newscp->p_notify_cbf = t_ha_para->p_notify_cbf;
        XOS_Trace(MD(FID_COMM, PL_MIN),"ha callback:module %d registered ok,pos %d at %p\n",module_id,i,newscp);
    }else
    {
       //oam_log_debug1(FILI,"module %d register ha callback failed!",module_id);
       XOS_Trace(MD(FID_COMM, PL_ERR),"module %d register ha callback failed!",module_id);
       return XERROR;
    }

#endif
    return XSUCC;

}

/*机框管理*/
int chassis_board_register(t_chassisinfo board_info)
{
  int iboard_pos;
  iboard_pos = chassis_board_check(board_info);
  if(iboard_pos <0)
  {
    return XERROR;
  }
  /*插入记录*/
  XOS_MemCpy(&g_chassis_board[iboard_pos],&board_info,sizeof(t_chassisinfo));
  g_chassis_board[iboard_pos].registerState = 1;
  return XSUCC;
}
int chassis_board_unregister(t_chassisinfo board_info)
{
  int iboard_pos;
  iboard_pos = chassis_board_check(board_info);
  if(iboard_pos <0)
  {
    return XERROR;
  }
  /*删除记录*/
  XOS_MemSet(&g_chassis_board[iboard_pos],0x0,sizeof(t_chassisinfo));
  g_chassis_board[iboard_pos].registerState = 0;
  return XSUCC;

}

#ifdef VXWORKS
int chassis_tsp_register(void)
{
    t_chassisinfo board_info;
    XOS_MemSet(&board_info,0x0,sizeof(t_chassisinfo));
    board_info.ChassisId= CHASSSID_ID_DEFAULT;
    board_info.SlotNo = ha_getboardslot();
    board_info.boardType= BOARD_TYPE_TSP;
    board_info.SubId =0;
    sprintf(board_info.strtimevalue,"%s",ha_oam_gettime());
    getLogicIp(&board_info.LogicIp,&board_info.LogicIPmask);
    if(g_bHaIsInit)
    {
        //oneAndOne (1),
        //nAndM (2),
        //onlyone (3)
        board_info.HAMode = 1;
    }else
    {
        board_info.HAMode = 3;
    }
    if(!ha_checkBoardMain())
    {
        /*
        Initial    (0),
        Master (1),
        Slave (2),
        Reserved (3)
        */
        /*Master*/
        board_info.HAState = 1;
    }else
    {
        if(g_bHaIsInit)
        {
            board_info.HAState = 2;
        }else
        {
            board_info.HAState = 0;
        }
    }
    /*update board ha status*/
    g_sysha_context.hastate = board_info.HAState;

    getBoardIp(&board_info.PhyIp,&board_info.PhyNetmask,&board_info.RoutingGateway);
    XOS_MemCpy(board_info.Version,XOS_GetVersion(),XOS_StrLen(XOS_GetVersion()));
    sprintf(board_info.Desc,"%s","TSP Registered");
    chassis_board_register(board_info);
    return XSUCC;
}

int xos_syslog(void * para1)
{
   memshow();
   msgqshow();
   return 0;
}

#endif

#ifdef  __cplusplus
}
#endif


