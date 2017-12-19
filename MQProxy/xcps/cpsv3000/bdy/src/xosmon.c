
/***************************************************************
**
**  Xinwei Telecom Technology co.,ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosmon.c
**
**  description:  system monitor
**
**  author: wangjing
**
**  date:   2011.6.29
**
***************************************************************
**                          history
**
***************************************************************
**   author            date              modification
**   wangjing     2011.6.29            create
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "xosshell.h"
#include "../inc/xosmon.h"

#ifdef XOS_LINUX

int g_MonGetCpuVal = 4; // seconds 
PTIMER mon_timer= XNULL;
float g_MonCpuRate = 0;
XU32 g_CpuTotal = 0;
XU32 g_CpuIdle = 0;

t_XOSFIDLIST XOS_MON =
{
    {
        "FID_MON",
        XNULL,
        FID_MON,
    },
    {
        MON_Init,
        XNULLP, 
        XNULL,
    },
    {
        MON_msgProc,
        MON_timerProc,
    },
    eXOSMode,
    XNULLP
};

XS32  XOS_FIDMON(HANDLE hDir,XS32 argc, XS8** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;

    XOS_MemSet( &stLoginList, 0x00, sizeof(t_XOSLOGINLIST) );

    stLoginList.stack     = &XOS_MON;
    XOS_StrNcpy(stLoginList.taskname , "tsk_mon", MAX_TID_NAME_LEN);
    stLoginList.TID       = FID_MON;
    stLoginList.prio      = TSK_PRIO_HIGHER;    
    stLoginList.quenum    = MAX_MSGS_IN_QUE;

    ret = XOS_MMStartFid(&stLoginList,XNULLP, XNULLP);

    return ret;
} 

/************************************************************************
 函数名:MON_Init
 功能: Init MON 
 输入:
 输出:
 返回:
 说明: 注册到模块管理中
************************************************************************/
XS8 MON_Init(XVOID*p1,XVOID*p2)
{
    t_PARA timerPara;
    t_BACKPARA backPara;

    if(XSUCC != XOS_TimerReg(FID_MON, 1000 ,1, 0))
    {
        return XERROR;
    }
    
    timerPara.fid  = FID_MON;
    timerPara.len  = g_MonGetCpuVal* 1000; // 4* 1000ms 间隔时长*/
    timerPara.mode = TIMER_TYPE_LOOP;
    timerPara.pre  = TIMER_PRE_LOW;
    
    backPara.para1 = 0;
    
    if(XSUCC !=XOS_TimerStart(&mon_timer,&timerPara,&backPara))
    {
        XOS_Trace(MD(FID_MON,PL_ERR),"XOS_TimerStart failed.");
        return XERROR;
    }
    
    return XSUCC;
}

XS8 MON_msgProc(XVOID* pMsgP,XVOID*sb )
{
    return XSUCC;
}

XS8 MON_timerProc( t_BACKPARA* pParam)
{
    float rate = 0;
    FILE *fp = NULL;
    char *tmp = NULL;
    XU8 buf[1024] = {0};
    XU8 strcpu[5] = {0};
    XU32 para1=0,para2=0,para3=0,para4=0,para5=0,para6=0,para7=0,para8=0;
    /*cpu  3136190 19790 348604 34385836 335202 6711 55775 2424
           cpuidlerate = idle_num/other value's sum*/

    fp = XOS_Fopen("/proc/stat","r");

    XOS_Fread(buf,1,1024,fp);
    tmp=strstr((char*)buf,"cpu");
    if(NULL == tmp)
    {
        printf("srch cpu failed.]r\n");
        return XERROR;
    }
    else
    {
        //printf("cpu=%s\r\n",tmp);
    }
    sscanf(tmp,"%s%d %d %d %d %d %d %d %d",strcpu,&para1,&para2,&para3,&para4,&para5,&para6,&para7,&para8);
    XOS_Fclose(fp);
    rate = (float)(para4-g_CpuIdle)/(float)(para1+para2+para3+para4+para5+para6+para7+para8-g_CpuTotal);
    g_CpuIdle = para4;
    g_CpuTotal = para1+para2+para3+para4+para5+para6+para7+para8;

    g_MonCpuRate = (float)100-rate*(float)100;

    //printf("rate = %f\r\n",g_MonCpuRate);

    return XSUCC;
}

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

