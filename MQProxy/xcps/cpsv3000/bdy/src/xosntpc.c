
/***************************************************************
**
**  Xinwei Telecom Technology co.,ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosntpc.c
**
**  description:  ntp client  implement
**
**  author: chenxiaofeng
**
**  date:   2011.4.6
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   chenxiaofeng    2011.4.6            create
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "xosshell.h"
#include "../inc/xosntpc.h"

#if defined(_MSC_VER) && (_MSC_VER > 1200)
void XOS_NtpcForSym(void) {}
#endif

#ifdef XOS_VXWORKS
#include <time.h>
extern STATUS sntpcTimeGet
    (
    char *            pServerAddr, /* server IP address or hostname */
    u_int             timeout,     /* timeout interval in ticks */
    struct timespec * pCurrTime    /* storage for retrieved time value */
    );

#endif

#define NTPC_TIMER_INTERVAL      (120)          /*��Сͬ�����*/
#define MAX_NTPC_TIMER_INTERVAL  (7200)         /*���ͬ�����*/
#define MAX_NTPC_TIMEOUT         (500)          /*timeout interval in ticks */
#define NTPC_TIMEZONE            (8*60)         /* UTC+8:00*/
#define MAX_NTPC_SERVER_LEN      (128)          /*���ntp��������ַ����*/

PTIMER ntpc_timer = XNULL;
static int sntpcTimerLen = MAX_NTPC_TIMEOUT; /* api timeout 500 tick*/
char gszNtpServerIp[MAX_NTPC_SERVER_LEN] = {0};              /*ntp server */
int ntpcTimerInterval = NTPC_TIMER_INTERVAL; /* seconds */
XS16 gtimezone = NTPC_TIMEZONE;                       /* UTC+8:00*/
extern t_XOSFIDLIST XOS_NTPC;

/************************************************************************
 ������:XOS_FIDNTPC
 ����: ģ����ں�������ƽ̨ͳһ����
 ����:
 ���:
 ����:
 ˵��:
************************************************************************/
XS32  XOS_FIDNTPC(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;

    if(argc > 0) /* argv[0] is the NtpServerIp*/
    {
        ntpc_setServerIp(argv[0]);
    }

    if(argc > 1)  /* syn interval*/
    {
        ntpcTimerInterval = atoi(argv[1]);
        if(ntpcTimerInterval > MAX_NTPC_TIMER_INTERVAL )
        {
            ntpcTimerInterval= MAX_NTPC_TIMER_INTERVAL;
        }

        if(ntpcTimerInterval <= 0)
        {
            ntpcTimerInterval = NTPC_TIMER_INTERVAL;
        }
    }

    if(argc > 2)  /*timezone*/
    {
        gtimezone = ntpc_getTimeZone(argv[2]);
    }

    XOS_MemSet( &stLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    stLoginList.stack     = &XOS_NTPC;
    XOS_StrNcpy(stLoginList.taskname , "tsk_ntpc", MAX_TID_NAME_LEN);
    stLoginList.TID       = FID_NTPC;
    stLoginList.prio      = TSK_PRIO_HIGHER;    
    stLoginList.quenum    = MAX_MSGS_IN_QUE;

    ret = XOS_MMStartFid(&stLoginList,XNULLP,XNULLP);

    return ret;
} 

/************************************************************************
 ������:NTPC_Init
 ����: Init ntpc 
 ����:
 ���:
 ����:
 ˵��: ע�ᵽģ�������
************************************************************************/
XS8 NTPC_Init(XVOID*p1,XVOID*p2)
{
    t_PARA timerPara;
    t_BACKPARA backPara;
    XS32 promptID = XERROR;
    
    /*ע��������*/
    promptID = XOS_RegistCmdPrompt( SYSTEM_MODE, "plat", "plat", "no parameter" );
    if ( XERROR >= promptID )
    {
        XOS_CliInforPrintf("NTPC_Init->XOS_RegistCmdPrompt() failed,return %d\r\n", promptID);
        return XERROR;
    }

    XOS_RegistCommand(promptID, ntpc_showcfg, "ntpcshowcfg", "display ntpc config", "no parameter");
    
    /*ע��һ���;��ȶ�ʱ��*/
    if(XSUCC != XOS_TimerReg(FID_NTPC, 1000 ,1, 0))  /*�������Ϊ1��,����Ϊ100ms*/
    {
        XOS_Trace(MD(FID_NTPC,PL_ERR), "NTPC_Init->XOS_TimerReg() failed\r\n");
        return XERROR;
    }

    timerPara.fid = FID_NTPC;
    timerPara.len = ntpcTimerInterval* 1000; // 120* 1000;/* ͬ�����*/
    timerPara.mode = TIMER_TYPE_LOOP;
    timerPara.pre  = TIMER_PRE_LOW;
    
    backPara.para1 = 0;

    /*������ʱ��*/
    if(XSUCC !=XOS_TimerStart(&ntpc_timer,&timerPara,&backPara))
    {
       XOS_Trace(MD(FID_NTPC,PL_ERR), "start ntpc timer failed\r\n");
       return XERROR;
    }

    //strcpy(gszNtpServerIp,"169.0.199.110"); // test code
    return XSUCC;
}

/************************************************************************
 ������:NTPC_msgProc
 ����: ģ�����Ϣ���� 
 ����:
 ���:
 ����:
 ˵��: ע�ᵽģ�������
************************************************************************/
XS8 NTPC_msgProc(XVOID* pMsgP,XVOID*sb )
{
    
    return 0;
}

/************************************************************************
 ������:NTPC_timerProc
 ����: ģ��Ķ�ʱ����Ϣ����
 ����:
 ���:
 ����:
 ˵��: ע�ᵽģ�������
************************************************************************/
XS8 NTPC_timerProc( t_BACKPARA* pParam)
{
    int ret =0;

    if(0 == strlen(gszNtpServerIp) )
    {
        return XSUCC;
    }
    
    ret = sntpc_SynTime(gszNtpServerIp);

    if(0 != ret)
    {
#ifdef XOS_VXWORKS
        XOS_Trace(MD(FID_NTPC,PL_ERR),"sntpc_SynTime return %d",ret);
#endif
    }
    else
    {
        XOS_Trace(MD(FID_NTPC,PL_DBG),"sntpc_SynTime OK");
    }
    return 0;
}

/************************************************************************
 ������:ntpc_setTimerLen
 ����: ģ��Ķ�ʱ����Ϣ����
 ����:
 ���:
 ����:
 ˵��: 
************************************************************************/
int ntpc_setTimerLen(int timerLen) 
{
    if(timerLen > 60)
    {
        timerLen = 60;
    }
    sntpcTimerLen = timerLen*100;
    return 0;
}

/************************************************************************
 ������:ntpc_setServerIp
 ����: ����ͬ�����
 ����:
 ���:
 ����:
 ˵��: 
************************************************************************/
char *ntpc_setServerIp(char *szIp)
{
    strncpy(gszNtpServerIp, szIp, sizeof(gszNtpServerIp)-1);
    return gszNtpServerIp;
}

/************************************************************************
 ������:ntpc_showcfg
 ����: ��������ʾntpc��������Ϣ
 ����:
 ���:
 ����:
 ˵��: ע�ᵽģ�������
************************************************************************/
void ntpc_showcfg(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    if(NULL != pCliEnv)
    {
        XOS_CliExtPrintf(pCliEnv,"Network Time Protocol Server = %s\nSynchronization Interval = %d seconds\nTimezone = %d\n",
                     gszNtpServerIp, ntpcTimerInterval, gtimezone);   
    } 
}

/************************************************************************
 ������:ntpcshowcfg
 ����: ��ʾntpc��������Ϣ,������vxworks�µ�ֱ�Ӻ������ã������ڴ��ڵ�shell������ʹ��
 ����:
 ���:
 ����:
 ˵��: 
************************************************************************/
void ntpcshowcfg(void)
{
    printf("Network Time Protocol Server = %s\nSynchronization Interval = %d seconds\nTimezone = %d\n",
                     gszNtpServerIp, ntpcTimerInterval, gtimezone);  
}


/************************************************************************
 ������:sntpc_SynTime
 ����: get time from NTP Server
 ����:
 ���:
 ����:0-succ ,1-timeout ,2 - setTime err,3- not support
 ˵��: ֧��vxworks��linux, ��֧��windonws
************************************************************************/
int sntpc_SynTime(char * szIp)
{
   int ret =0;
   
#ifdef XOS_VXWORKS
   t_XOSTB xostm;
    
    struct timespec st;
    if(OK != sntpcTimeGet(szIp,sntpcTimerLen,&st) )
    {
        ret  = 1;
        return ret;
    }

    
    xostm.time = st.tv_sec; // seconds
    xostm.millitm = st.tv_nsec / 1000000; // trann to ms
    xostm.timezone = gtimezone;
    if(XSUCC != XOS_SetSysTime( &xostm ) )
    {
        ret = 2;
    }
#else  

#ifdef XOS_LINUX
    XS8 cmdstr[255] = {0};

    /*ʱ��ͬ��*/
    snprintf(cmdstr, sizeof(cmdstr)-1, "ntpdate %s", gszNtpServerIp);
    ret = XOS_ExeCmd(cmdstr, NULL, 0);

    if(0 != ret)
    {   
        XOS_Trace(MD(FID_NTPC,PL_ERR),"sntpc_SynTime system ntpdate failed");
    }  
#else

#endif

#endif
    return ret;
}

/************************************************************************
 ������:ntpc_getTimeZone
 ����: ��ȡʱ�����
 ����:
 ���:
 ����:
 ˵��: 
************************************************************************/
int ntpc_getTimeZone(char *szTimeZone)
{
    char sztmp[10] ={0};
    int diffminus  =1;
    int diffpara = 0;
    int hh =0;
    int mm =0;
    
    strncpy(sztmp,szTimeZone+3,1);
    if(0 == strcmp(sztmp,"+") )
    {
        diffpara = 1;
    }
    else if(0 == strcmp(sztmp,"-") )
    {
        diffpara = -1;
    }
    else
    {
        diffpara = 0;
    }

    strncpy(sztmp,szTimeZone+4,2); // get hh
    hh = atoi(sztmp);
    strncpy(sztmp,szTimeZone+7,2); // get mm
    mm = atoi(sztmp);

    diffminus = diffpara * (hh * 60 + mm);
    return diffminus;

}


#ifdef __cplusplus
}
#endif /* __cplusplus */

