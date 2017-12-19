/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: 
**
**  description:
**
**  author: 
**
**  date:   2006.3.30
**
**************************************************************
**                          history
**
***************************************************************
**   author          date                   modification
**   guolili         2006.3.30              create
**   lxn             2006.8.3               modify
**   lxn             2006.8.17              modify  ��Ҫ���win32 ��linux�µĲ�ͬ�汾�Լ�д��¼�ļ�
**   lxn             2006.9.6               modify  ��ɽ�trace��Ϊ�����Ĺ��ܿ���ƣ�����ע�Ṧ�ܿ飬������Ϣ��������Ϣ����
                                                    ����log����д��־�ļ��ȡ��Լ���ͬϵͳ�汾�ļ����ԸĽ�
**   lixn            2006.11.20             ����������
**   zgq             2007.11.10             ������Ӽ���������
**   zgq             2009.09.04             ���ܸ���,�����ڴ���Ϣ,�ڴ�trace����
**************************************************************/

#ifdef  __cplusplus
extern  "C"
{
#endif

#include "xostrace.h"
#include "xoslog.h"

#include "xosmodule.h"
#include "xosencap.h"
#include "xosport.h"
#include "xoshash.h"

////////////////////////////////////VxWorks/////////////////////////////////////////
#ifdef   XOS_VXWORKS
XEXTERN t_XOSTT gMsec; /*����ʱ�ӵ�������*/
#endif
////////////////////////////////////VxWorks/////////////////////////////////////////

/*Traceģ��ȫ�ֱ�������*/
XEXTERN e_PRINTLEVEL g_lPrintToLogLevel;
XEXTERN char gszPrefixInfo[CLI_PRE_FIX_LEN];
XSTATIC t_XOSMUTEXID g_no_fid_trace_mutext; 


XSTATIC XCHAR*   g_PrintLevelName[PL_MAX]   = {"PL_MIN" ,"PL_DBG", "PL_INFO","PL_WARN","PL_ERR","PL_EXP","PL_LOG"};
XSTATIC XCHAR*   g_PrintDevName[MAX_DEV]    = {"COSOLE","LOGFILE","COSOLE_AND_FILE","TELNET","SERVER"};

/*���й��ܿ�رձ�־,[SWITCH_OFF]��ʾ�ر�*/
XSTATIC XU32     g_ulTraceAllFidCloseFlg    = SWITCH_ON;

/*trace���ܿ黹û������*/
XSTATIC XU16     g_ulTraceIsSetup           = SWITCH_OFF;

//add by gcg
//�����Ƿ񽫴�ӡ��Ϣ��������ذ忪��
XEXTERN XU32     g_ulPrinToMcbFlg;
XEXTERN t_FIDCB * MOD_getFidCb(XU32 fid);
//!add
XVOID TRC_CMDSetPrinToMcbFlg(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XS32 Trace_regCliCmd(int cmdMode);

XS32 XOS_TraceCheckLevel(t_FIDCB *pFidCb, e_PRINTLEVEL level);
XS32 XOS_TraceCheckSessionLevel(t_FIDCB *pFidCb, e_PRINTLEVEL level,e_PRINTLEVEL minLevel);


/*�ڲ�����˵��*/
/********************************�ڲ�����*************************************/
/*-----------------------------------------------
��������TR_sendmsg
��  �ܣ�trace�ڶ���Ϣ���ݽ�����䲢����
��  ��: fid   -Ҫ���msg���FID���ܿ��
        level -��Ϣ����
        str   -��Ϣ��message��
��  ��:
��  �أ��ɹ�-XSUCC;ʧ��-XERROR
------------------------------------------------*/
XVOID TRC_sendmsg(XU32 fid,e_PRINTLEVEL level,XCHAR *str)
{
    t_XOSCOMMHEAD* sendmsg = XNULL;/*���͵�traceģ�����Ϣ*/
    XU32           msglen  = 0;    /*��¼��Ϣ����*/
    
    msglen  = (XU32)XOS_StrLen( str )+1;
    sendmsg = XOS_MsgMemMalloc( FID_TRACE, msglen );
    
    if ( XNULLP == sendmsg )
    {
        XOS_CliInforPrintf("XOS_Trace()->malloc msg failed  !\r\n");
        return;
    }
    
    /*��д��Ϣ����*/
    sendmsg->datasrc.FID    = fid;
    sendmsg->datasrc.FsmId  = 0;
    sendmsg->datasrc.PID    = XOS_GetLocalPID();
    sendmsg->datadest.FID   = (XU32)FID_TRACE;
    sendmsg->datadest.FsmId = 0;
    sendmsg->datadest.PID   = XOS_GetLocalPID();
    sendmsg->msgID          = 0;
    sendmsg->msgID          = (XU16)level;
    sendmsg->subID          = 0;
    sendmsg->prio           = eNormalMsgPrio;
    sendmsg->length         = msglen;
    
    XOS_MemCpy(sendmsg->message, str, msglen);
    
    /*������Ϣ*/
    if ( XSUCC != XOS_MsgSend( sendmsg ) )
    {
        XOS_CliInforPrintf("TRC_sendmsg,FID %d msg level [%d] failed!\r\n",
            fid,level);
        XOS_MsgMemFree(fid,sendmsg );
        return;
    }
}

/*-----------------------------------------------
��������formateloctime
��  �ܣ�ȡϵͳʱ�䲢����Ϊ��-��-��- ʱ��ĸ�ʽ
��  ��:datet--��¼ʱ�䴮��ָ��
��  ��:
��  �أ��ɹ�-ʵ�ʸ�ʽ������;ʧ��-XERROR
------------------------------------------------*/
XS32 TRC_formateloctime(XCHAR *datet)
{
    time_t        lt;
    t_trc_systime tmptime;
    
    if ( XNULLP == datet )
    {
        return XERROR;
    }
    
    time( &lt );
#ifdef XOS_VXWORKS
    lt = lt + gMsec;
#endif
    
    XOS_MemCpy(&tmptime, localtime(&lt), sizeof(t_trc_systime));
    return XOS_Sprintf(datet,
        MAX_FILE_NAME_LEN,
        "%04d-%02d-%02d %02d:%02d:%02d",
        tmptime.dt_year+1900,
        tmptime.dt_mon+1,
        tmptime.dt_mday,
        tmptime.dt_hour,
        tmptime.dt_min,
        tmptime.dt_sec);    
}

/************************************************************************
������:  
��  ��:  ��Ҫ��ģ�����ģ���ṩ�ĸı�fid��trace��Ϣ�ṹ���и������ƿ��صĻص�����
��  ��:
��  ��:
��  ��: ǰһ��fid ������Ϣ���param
˵  ��:
************************************************************************/
XVOID *TRC_cbsetfidlevel(t_FIDTRACEINFO *pFidTraceInfo, XVOID *param,XVOID *param2)
{
    pFidTraceInfo->traceLevel =*(e_PRINTLEVEL*)param;

    /*���õ�ǰtelnet�ն˵�trace����*/
    pFidTraceInfo->sessionLevel[*(XS32*)param2] = *(e_PRINTLEVEL*)param;
    return param;
}

////////////////////////////////////Trace Cli/////////////////////////////////////////
/*---------------------------------------------------------------------
��������  TRC_settotask
��  �ܣ�  ������Ӧfid���ܿ飬����ӡ���ն˵���Ϣ���͸�traceģ����Ϣ���б�־��
          traceģ�齫������Ϣͳһ�����ӡ��Ϣ��
��  �룺  fid -���ܿ�ID
��  ��:
����ֵ:
˵  ����
------------------------------------------------------------------------*/
XVOID TRC_settotask (CLI_ENV *pCliEnv,XU32 fid,XBOOL openflag)
{
    t_FIDTRACEINFO* fidtrace = XNULL;
    if ( !XOS_isValidFid(fid) )
    {
        XOS_CliExtPrintf(pCliEnv,"input para fid %d is wrong\r\n",fid);
        return;
    }

    fidtrace = MOD_getFidTraceInfo( fid );
    if ( XNULL == fidtrace )
    {
        XOS_CliExtPrintf(pCliEnv,"get fid %d traceinfo return null!\r\n",fid);
        return;
    }
    
    fidtrace->isPrintInTraceTsk = openflag;
    if(XTRUE == openflag)
    {
        XOS_CliExtPrintf(pCliEnv,"set fid %d trace msg output to trace queue\r\n",fid);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"set fid %d trace msg output to terminal(telnet|console)\r\n",fid);
    }
    return;
    
}
/*---------------------------------------------------------------------
��������  
��  �ܣ�  ��[���й��ܿ�Ĵ�ӡ����]����
��  �룺  pCliEnv     - ���룬�������ն˺�
��  ����  ��ӡ���ַ�����
����ֵ:   XSUCC  -  �ɹ�          XERROR -  ʧ��
˵  ����
------------------------------------------------------------------------*/
XS32 TRC_settraceopenall (CLI_ENV *pCliEnv,XBOOL openflag)
{
    
    g_ulTraceAllFidCloseFlg = openflag;
    if(XTRUE == openflag)
    {
        XOS_CliExtPrintf(pCliEnv,"SWITCH_ON all module trace\r\n");
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"SWITCH_OFF all module trace\r\n");
    }
    return  XSUCC;
}

/*---------------------------------------------------------------------
��������  XOS_TraceSetFidLevel
��  �ܣ�  ��[ĳ���ܿ�Ĵ�ӡ����]����
��  �룺  ulFid           - ���룬����ID
          ePrintLevel     - ���룬��ӡ����
��  ����  
����ֵ:   XSUCC  -  �ɹ�          XERROR -  ʧ��
˵  ����
------------------------------------------------------------------------*/
XS32 XOS_TraceSetFidLevel (XU32  ulFid, e_PRINTLEVEL ulLevel)
{
    t_FIDTRACEINFO* tmp  = XNULL; /*fid��trace��Ϣ�Ľṹ��*/
    XS32 i =0;
    
    if ( !XOS_isValidFid(ulFid) )
    {
        printf("input para module FID %d is wrong\r\n",ulFid);
        return XERROR;
    }
    
    if ( ulLevel >= PL_MAX||ulLevel < PL_MIN )
    {
        printf("input module FID %d print level %d is wrong\r\n",ulFid,ulLevel);
        return XERROR;
    }
    
    /*MOD_setFidTraceLevel(ulFid, ulLevel );*/
    tmp = MOD_getFidTraceInfo(ulFid);
    XOS_LogSetFidLevel(ulFid,ulLevel);
    
    if ( XNULL != tmp )
    {
        tmp->traceLevel = ulLevel;
        /*��������telnet�ն˵�trace����*/
        for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
        {
            tmp->sessionLevel[i] = ulLevel;
        }
        
        printf("set module FID [%d] print level to %s\r\n",
                ulFid,
                g_PrintLevelName[ulLevel]);
    }
    else
    {
        printf("get FID %d traceinfo return null!\r\n",ulFid);
    }
    
    return  XSUCC;
    
}

/*---------------------------------------------------------------------
��������  
��  �ܣ�  ��[ĳ���ܿ�Ĵ�ӡ����]����
��  �룺  ulFid           - ���룬����ID
          ePrintLevel     - ���룬��ӡ����
��  ���� 
����ֵ:   XSUCC  -  �ɹ�          XERROR -  ʧ��
˵  ����
------------------------------------------------------------------------*/
XS32 TRC_setfidlevel (CLI_ENV *pCliEnv,XU32  ulFid, e_PRINTLEVEL ulLevel)
{
    t_FIDTRACEINFO* tmp  = XNULL; /*fid��trace��Ϣ�Ľṹ��*/
    
    if ( !XOS_isValidFid(ulFid) )
    {
        XOS_CliExtPrintf(pCliEnv,"input para module FID %d is wrong\r\n",ulFid);
        return XERROR;
    }
    
    if ( ulLevel >= PL_MAX||ulLevel < PL_MIN )
    {
        XOS_CliExtPrintf(pCliEnv,"input module FID %d print level %d is wrong\r\n",ulFid,ulLevel);
        return XERROR;
    }
    
    /*MOD_setFidTraceLevel(ulFid, ulLevel );*/
    tmp = MOD_getFidTraceInfo(ulFid);
    XOS_LogSetFidLevel(ulFid,ulLevel);

    if ( XNULL != tmp )
    {
        tmp->traceLevel = ulLevel;
        /*���õ�ǰtelnet�ն˵�trace����*/
        tmp->sessionLevel[MMISC_GetFsmId(pCliEnv)] = ulLevel;
        
        XOS_CliExtPrintf(pCliEnv,
            "set module FID [%d] print level to %s\r\n",
            ulFid,
            g_PrintLevelName[ulLevel]);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"get FID %d traceinfo return null!\r\n",ulFid);
    }
    
    return  XSUCC;
    
}
/***********************************************************************
������: 
��  ��: ��������fid���ܿ��tracelevel��ӡ����
��  ��: ulLevel --��ӡ����
        pCliEnv --�������ն˺�
��  ��:
����ֵ: �ɹ�����XSUCC�� ʧ�ܷ���XERROR
*************************************************************************/
XS32 TRC_setalllevel(CLI_ENV *pCliEnv,e_PRINTLEVEL ulLevel)
{
    XU32 level = (XU32) ulLevel;
    XS32 session = 0;
    
    if ( ulLevel < PL_MIN
        || ulLevel >= PL_MAX )
    {
        XOS_CliExtPrintf(pCliEnv,
            "input print level %d is wrong\r\n",
            ulLevel);
        return XERROR;
    }

    session = MMISC_GetFsmId(pCliEnv);
    /*����xosmodule.h���ṩ�Ļص������������е�fid�������Ϣ*/
    if ( XSUCC != MOD_setAllTraceInfo(TRC_cbsetfidlevel,&level,&session) )
    {
        XOS_CliExtPrintf(pCliEnv,"set print tracelevel failed!\r\n");
        return XERROR;
    }
    
    XOS_CliExtPrintf(pCliEnv,"all FID tracelevel have set to %s\r\n",g_PrintLevelName[ulLevel]);
    
    return XSUCC;
    
}
/*---------------------------------------------------------------------
��������  XOS_TraceCheckLevel
��  �ܣ�  ���trace��log��ӡ����
��  �룺
��  ����
����ֵ:
˵  ����
------------------------------------------------------------------------*/
XS32 XOS_TraceCheckLevel(t_FIDCB *pFidCb, e_PRINTLEVEL elevel)
{
    t_FIDTRACEINFO* traceInfo = NULL;

    if(NULL == pFidCb) 
    {
        return XSUCC;
    }

    traceInfo = &(pFidCb->traceInfo);

    if(NULL == traceInfo) 
    {
        if(elevel < pFidCb->logLevel)
        {
            return XERROR;
        }
    }   
    else 
    {
        if (elevel >= traceInfo->traceLevel ) 
        {
            return XSUCC;
        } 
        else 
        {
            if (elevel >= pFidCb->logLevel)
            {
                return XSUCC;
            }
        }
        return XERROR;
    }
    
    return XSUCC;
}
/*---------------------------------------------------------------------
��������  XOS_TraceCheckSessionLevel
��  �ܣ�  ���trace��log��ӡ����
��  �룺  pFidCb:ģ����ƿ�
        elevel:��־��ӡ����
        minLevel:��־���˼���
��  ����
����ֵ:XSUCC �ɹ���XERROR ʧ��
˵  ��������telnet�ն˴�ӡtraceʱ����
------------------------------------------------------------------------*/
XS32 XOS_TraceCheckSessionLevel(t_FIDCB *pFidCb, e_PRINTLEVEL elevel,e_PRINTLEVEL minLevel)
{
    t_FIDTRACEINFO* traceInfo = NULL;

    if(NULL == pFidCb) 
    {
        return XSUCC;
    }

    traceInfo = &(pFidCb->traceInfo);

    if(NULL == traceInfo) 
    {
        if(elevel < pFidCb->logLevel)
        {
            return XERROR;
        }
    }   
    else 
    {
        if (elevel >= minLevel ) 
        {
            return XSUCC;
        } 
        else 
        {
            if (elevel >= pFidCb->logLevel)
            {
                return XSUCC;
            }
        }
        return XERROR;
    }
    
    return XSUCC;
}
/***********************************************************************
������: 
��  ��: ���ö�Ӧfid���ܿ������Ϣʱ�Ƿ��ӡʱ��
��  ��: ulFid   -���ܿ�ID
        ulCtrl  -���ƿ���0,1
        pCliEnv -�������ն˺�
��  ��:
����ֵ: �ɹ�����XSUCC�� ʧ�ܷ���XERROR
*************************************************************************/
XS32 TRC_setfidtime(CLI_ENV *pCliEnv,XU32 ulFid,XBOOL ulCtrl)
{
    t_FIDTRACEINFO* fidtrace = XNULL;
    
    if ( !XOS_isValidFid(ulFid) )
    {
        XOS_CliExtPrintf(pCliEnv,"fid %d parameter wrong\n",ulFid);
        return XERROR;
    }
    
    /*�ж�devid�Ƿ�Ϸ�*/
    if ( XTRUE != ulCtrl && XFALSE != ulCtrl )
    {
        XOS_CliExtPrintf(pCliEnv,"switch para %d wrong\r\n",ulCtrl);
        return XERROR;
    }
    fidtrace = MOD_getFidTraceInfo(ulFid);
    if ( XNULL == fidtrace )
    {
        XOS_CliExtPrintf(pCliEnv,"set FID %d traceinfo null!\r\n",ulFid);
        return XERROR;
    }
    
    fidtrace->isNeedTime = ulCtrl;
    
    if ( SWITCH_ON == ulCtrl )
    {
        XOS_CliExtPrintf(pCliEnv,"fid %d time switch is open\r\n",ulFid);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"fid %d time switch is close\r\n",ulFid);
    }
    return XSUCC;
}
/*---------------------------------------------------------------------
��������  
��  �ܣ�  �򿪶�Ӧfid���ܿ�[��ӡ�ļ����ʹ��������Ĵ�ӡ����]����
��  �룺  pCliEnv   -�������ն˺�
          ulCtrl    -�򿪻�ر�
          fid       -���ܿ�ID
��  ����  ��ӡ���ַ�����
����ֵ:   XSUCC  -  �ɹ�          XERROR -  ʧ��
˵  ����
------------------------------------------------------------------------*/
XS32 TRC_setfidline (CLI_ENV *pCliEnv,XU32 fid,XBOOL ulCtrl)
{
    
    t_FIDTRACEINFO* tmpfidtrace = XNULL;
    
    tmpfidtrace = MOD_getFidTraceInfo(fid);
    
    if ( XNULL == tmpfidtrace )
    {
        XOS_CliExtPrintf(pCliEnv,"get FID %d traceinfo return null!\r\n",fid);
        return XERROR;
    }
    
    tmpfidtrace->isNeedLine     = ulCtrl;
    tmpfidtrace->isNeedFileName = ulCtrl;
    
    if ( SWITCH_ON == ulCtrl )
    {
        XOS_CliExtPrintf(pCliEnv,"SWITCH_ON FID %d for filename and line trace\r\n",fid);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"SWITCH_OFF FID %d for filename and line trace\r\n",fid);
    }
    
    return XSUCC;
    
}

/***********************************************************************
������: 
��  ��: ���ö�Ӧfid���ܿ��ӡ��Ϣ����豸��(�ն˻��Ǽ�¼�ļ�)����
��  ��: pCliEnv -�������ն˺�
        devid   -�豸��¼��0,1,2
        fid     -���ܿ�ID

��  ��:
����ֵ: �ɹ�����XSUCC�� ʧ�ܷ���XERROR
*************************************************************************/
XS32 TRC_setoutdev (CLI_ENV *pCliEnv,XU32 fid,e_TRACEPRINTDES devid)
{
    t_FIDTRACEINFO* fidtrace   = XNULL;
    
    if ( !XOS_isValidFid(fid) )
    {
        XOS_CliExtPrintf(pCliEnv,"input para module FID %d is wrong\r\n",fid);
        return XERROR;
    }
    
    /*�ж�devid�Ƿ�Ϸ�*/
    if ( devid > TOSERVER )
    {
        XOS_CliExtPrintf(pCliEnv,"input para devid %d is wrong\r\n",devid);
        return XERROR;
    }
    
    fidtrace = MOD_getFidTraceInfo(fid);
    
    if ( XNULL == fidtrace )
    {
        XOS_CliExtPrintf(pCliEnv,"get FID %d traceinfo return null!\r\n",fid);
        return XERROR;
    }
    
    fidtrace->traceOutputMode = devid;
    
    XOS_CliExtPrintf(pCliEnv,"set FID %d print output deviceID to %s\r\n",
        fid,g_PrintDevName[devid]);
    
    return XSUCC;

}

XEXTERN XVOID  TRC_CMDShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);
/*---------------------------------------------------------------------
������:  
��  ��:  ���ö�ӦFID��trace�����Ϣ�Ƿ��ӡʱ������
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��:
˵  ��:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetfidtime(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*����ΪFID�Ϳ��ƿ���*/
    XU32      ulFid;
    XU32      lflag;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }
    
    ulFid = atoi(ppArgv[1]);
    lflag = atoi(ppArgv[2]);
    
    if ( 1 != lflag && 0 != lflag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter %d is wrong!\r\n",lflag);
        return;
    }
    
    /*��������fid�����Ϣ��ӡʱ��ĺ���*/
    TRC_setfidtime(pCliEnv,ulFid,(XBOOL)lflag);
    
    return;
}

/*������ע��ʹ�ú���*/
/*---------------------------------------------------------------------
������:  TRC_CMDSettotask
��  ��:  ���ö�ӦFIDģ��������ն˵���Ϣ���Ƿ�Ҫ������Ϣ��traceģ������
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��:
˵  ��:
------------------------------------------------------------------------*/
XVOID TRC_CMDSettotask(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*�����ֱ�ΪFID�Ϳ��ƿ���*/
    XU32      ulFid;
    XU32      lflag;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        return;
    }
    
    ulFid = atoi(ppArgv[1]);
    lflag = atoi(ppArgv[2]);
    
    if (1 != lflag && 0 != lflag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter %d is wrong\r\n",lflag);
        return;
    }
    
    /*��������fid��Ӧlflag����*/
    if ( 1 == lflag )
    {
        TRC_settotask(pCliEnv,ulFid,XTRUE);
    }
    
    if ( 0 == lflag )
    {
        TRC_settotask(pCliEnv,ulFid,XFALSE);
    }
    
    return;
    
}
/*---------------------------------------------------------------------
������:  
��  ��:  ���ô�/�ر�����ģ��Ĵ�ӡ��������
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��:
˵  ��:
------------------------------------------------------------------------*/
XVOID TRC_CMDSettraceopenall(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*����Ϊ���ƿ���*/
    XU32      lflag;
    
    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        if(g_ulTraceAllFidCloseFlg)
        {
            XOS_CliExtPrintf(pCliEnv,"current all module trace switch is open\r\n");
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,"current all module trace switch is close\r\n");
        }
        return;
    }
    
    lflag = atoi(ppArgv[1]);
    
    if ( 1 != lflag && 0 != lflag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter switch %d is wrong\r\n",lflag);
        return;
    }
    
    /*��������allfidprintflag����*/
    if ( 1 == lflag )
    {
        TRC_settraceopenall(pCliEnv,XTRUE);
    }
    
    if ( 0 == lflag )
    {
        TRC_settraceopenall(pCliEnv,XFALSE);
    }
    
    return;
}
/*---------------------------------------------------------------------
������:  
��  ��:  ��һ�����ܿ�Ĵ�ӡ��������
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��:
˵  ��:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetfidlevel(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*����ΪFID���������*/
    
    XU32  ulFid;
    e_PRINTLEVEL ePrintLevel;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"setfidlevel <fid> <level>  --set fid printf level.\r\n");

        return;
    }
    
    ulFid       = atoi(ppArgv[1]);
    ePrintLevel = (e_PRINTLEVEL) atoi(ppArgv[2]);
    
    TRC_setfidlevel(pCliEnv, ulFid, ePrintLevel);
    
    return;
}


/*---------------------------------------------------------------------
������:  
��  ��:  ��������ģ���trace�����Ϣ��������
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��:
˵  ��:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetalllevel(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*����Ϊ�������*/
    XU32      level;
    
    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"setalllevel  <level>  --set all fid printf level.\r\n");

        return;
    }
    
    level = atoi(ppArgv[1]);
    
    /*������������fid�������ĺ���*/
    TRC_setalllevel(pCliEnv,(e_PRINTLEVEL)level);
    return;
}

/*---------------------------------------------------------------------
������:  
��  ��:  ���ö�ӦFID��trace�����Ϣ�Ƿ��ļ������к�����
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��:
˵  ��:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetfidline(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    /*����ΪFID�Ϳ��ƿ���*/
    
    XU32      ulFid;
    XU32      lflag;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }
    
    ulFid = atoi(ppArgv[1]);
    lflag = atoi(ppArgv[2]);
    
    if ( 1 != lflag && 0 != lflag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter %d is wrong\r\n",lflag);
        return;
    }
    
    /*����������Ӧfid��ӡ�ļ������кŵĺ���*/
    if ( 1 == lflag )
    {
        TRC_setfidline(pCliEnv,ulFid,(XBOOL)SWITCH_ON);
    }
    
    if ( 0 == lflag )
    {
        TRC_setfidline(pCliEnv,ulFid,(XBOOL)SWITCH_OFF);
    }
    
    return;
    
}
/*---------------------------------------------------------------------
������:  
��  ��:  ���ô�ӡ��Ϣ���Ŀ���豸
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��:
˵  ��:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetoutdev(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32            lfid;
    e_TRACEPRINTDES devtype;
    
    if ( 3 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }
    
    lfid    = atoi(ppArgv[1]);
    devtype = (e_TRACEPRINTDES)atoi(ppArgv[2]);
    
    TRC_setoutdev(pCliEnv,lfid,devtype);
    
    return;
    
}
////////////////////////////////////Trace Cli/////////////////////////////////////////

/*---------------------------------------------------------------------
������:  XOS_PrintInit
��  ��:  ��ӡģ���ʼ��������ʼ��traceģ�飬ͬʱ���ó�ʼ��logģ��(��Ϊ��ģ��)
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��: XSUCC, ��������ʧ�ܷ���XERROR
˵  ��:
------------------------------------------------------------------------*/
XS8 Trace_Init(XVOID * t, XVOID *V)
{    
    /*init mutex*/
    if ( XSUCC != XOS_MutexCreate( &g_no_fid_trace_mutext) )
    {
        XOS_CliInforPrintf("XOS_MutexCreate g_no_fid_mutext failed!");
        return XERROR;
    }
    
    Trace_regCliCmd(SYSTEM_MODE);
    return(XSUCC);
}

XS32 Trace_regCliCmd(int cmdMode)
{    
    XS32 ret;
    XS32 backvalue;

    ret = XOS_RegistCmdPrompt( cmdMode, "plat", "plat", "no parameter" );
    if ( XERROR >= ret )
    {
        XOS_CliInforPrintf("trace init failed,return %d\r\n",ret);
        return XERROR;
    }
    
    if ( XERROR >=  ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetfidlevel,
        "setfidlevel", "set fid trace level",
        "para:fid 0/1 \r\n\tPL_MIN=0��\r\n\tPL_DBG=1��\r\n\tPL_INFO=2��\r\n\tPL_WARN=3��\r\n\tPL_ERR=4��\r\n\tPL_EXP=5��\r\n\tPL_LOG=6. \r\nexample:setfidlevel 1 2") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setfidtrclev failed,return %d\r\n",backvalue);
    }
    
    if( XERROR >=  ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetalllevel,
        "setalllevel", "set all fid trace level",
        "para:\r\n\tPL_MIN=0��\r\n\tPL_DBG=1��\r\n\tPL_INFO=2��\r\n\tPL_WARN=3��\r\n\tPL_ERR=4�� \r\n\tPL_EXP=5��\r\n\tPL_LOG=6.\r\nexample:setalltrclev 5\r\n" ) ))
    {
        XOS_CliInforPrintf("trace init,reg cmd setallfidtrslev failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >=  ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetfidtime,
        "setptime","set fid trace time flag",
        "para:fid 0/1\r\n\t0-close;\r\n\t1-open.\r\nexample: to close fid 1 time switch; usage:setptime 1 0") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setptime failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >=   ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetfidline,
        "setpfileline","set fid trace filename and line flag",
        "para:fid 0/1\r\n\t0-close;\r\n\t1-open.\r\nexample:setpfileline 1 0") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setpfileline failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >=   ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetoutdev,
        "setoutdev",
        "set fid trace output device",
        "para:fid dev \r\ndev:\r\n\t0-only console;\r\n\t1-only to logfile;\r\n\t2-both console and logfile\r\nexample:setoutdev 1 0") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setoutdev failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >= ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSettotask,
        "settotask", "set fid trace output mode",
        "output to terminal or task queue\r\npara:fid 0/1 \r\nexample:settotask 201 1") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd settotask failed,return %d\r\n",backvalue);
    }
    
    if ( XERROR >=   ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSettraceopenall,
        "settraceflag", "set all fid trace flag(close/open)",
        "para:0/1\r\n\t0-close,1-open.\r\nexample:setprintflag 0") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd setprintflag failed,return %d\r\n",backvalue);
    }

//add by gcg
    if ( XERROR >= ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDSetPrinToMcbFlg,
        "setprint2mcb", "set print info to main control bord",
        "set print info to main control bord\r\npara:flag 0/1 \r\nexample:setprint2mcb 1") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd settotask failed,return %d\r\n",backvalue);
    }
//!add
    
    if ( XERROR >=   ( backvalue = XOS_RegistCommand(ret,
        TRC_CMDShow,
        "showalltrcinfo",
        "display all fid trace information",
        "no parameter") ) )
    {
        XOS_CliInforPrintf("trace init,reg cmd showallfidtrcinfo failed,return %d\r\n",backvalue);
    }

    /*����ȫ�ֱ���,trace������������Լ�trace���ܿ�������־*/
    /*���й��ܿ�رձ�־,[SWITCH_OFF]��ʾ�ر�*/
    g_ulTraceAllFidCloseFlg = SWITCH_ON;
    
    g_ulTraceIsSetup  = SWITCH_ON;
    
    return(XSUCC);
    
}

//begin added for cpu efficiency zenghaixin 20100802
/*---------------------------------------------------------------------
��������  XOS_JudgeMsgFormat
��  �ܣ�  �жϸ�ʽ���ַ���������ȷ��
��  �룺  FileName      - �ļ���(��__FILE__)
          out_filename  - ���ڱ����ɾ���·��FileName�����õ����ļ���
          prt_time      - ���ڱ����¼�����ʱ��
          ulFid         - ���ܿ�ID
          fid_name      - ���ڱ����߳�����
          format_len    - ��Ϣ����������
��  ����  out_filename  - ��¼�ɾ���·��FileName�����õ����ļ���
          prt_time      - ��¼�¼�����ʱ��
          fid_name      - �߳�����

���أ�    1 - ������ȷ  0 - ���ȷǷ�
˵  ����  �ú�����XOS_Trace()��������Ҫʱ���ã��Լ�����Ч���ڴ濽�������CPUЧ��
------------------------------------------------------------------------*/
XS32 XOS_JudgeMsgFormat( const XCHAR* FileName,       /*�ļ���(��__FILE__)*/
                        XCHAR    *out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
                        XCHAR    *prt_time,        /*��¼�¼�����ʱ��*/
                        XU32 ulFid,            /*���ܿ�ID*/
                        XCHAR    *fid_name,        /*�߳�����*/  
                        XS32    format_len        /*��Ϣ����������*/  )
{
    XS32   msg_curlen = 0;   /*����Ϣ���ĳ��ȼ�¼*/
    XCHAR* fidName = NULL;
    
    /*�õ��ļ����ƴ�*/
    if ( XSUCC != Trace_abFileName(FileName,out_filename,MAX_FILE_NAME_LEN) )
    {
        return 0;
    }
    msg_curlen = format_len + (XU32)XOS_StrLen(out_filename);
    
    /*����ϵͳʱ�䣬�õ���Ӧ��ʱ���ַ���*/
    if ( XSUCC >= TRC_formateloctime(prt_time) )
    {
        prt_time[0] = '\0';
    }
    msg_curlen = msg_curlen + (XU32)XOS_StrLen(prt_time);
    
    /*�õ�ģ������*/
    fidName = XOS_getFidName(ulFid);
    if ( XNULLP != fidName)
    {
        XOS_Sprintf(fid_name,MAX_FILE_NAME_LEN,"%s", fidName);
    }
    else
    {
        XOS_Sprintf(fid_name,MAX_FILE_NAME_LEN,"Unknown_FID[%d]",ulFid);
    }
    msg_curlen = msg_curlen + (XU32)XOS_StrLen(fid_name);
    
    if(msg_curlen > MAX_TRACE_INFO_LEN-16)
    {
        return 0;
    }
    
    return 1;
}

/*---------------------------------------------------------------------
��������  XOS_FillMsgFilter
��  �ܣ�  ��������Ϣ����ʽ
��  �룺  msg_filter    - ���ڱ��������Ϣ����ʽ 
          eLevel        - ��ӡ����
          fidtrace      - ��¼��fid�õ�����Ӧ��trace��Ϣ���ƿ��ؽṹ��
          prt_time      - ��¼�¼�����ʱ��
          fid_name      - �߳�����
          out_filename    - ��¼����·���ļ��� 
          ulLineNum     - �к�
          msg_format    - ��¼��Ϣ���������ݣ���cformat,...�õ�

��  ����  msg_filter  -  ������Ϣ����ʽ
˵  ����  �ú�����XOS_Trace()��������Ҫʱ���ã��Լ�����Ч���ڴ濽�������CPUЧ��
------------------------------------------------------------------------*/
XVOID XOS_FillMsgFilter( XCHAR *msg_filter,                     /*������Ϣ��*/
                        const int               size,           /*������Ϣ����С*/
                        const e_PRINTLEVEL      eLevel,         /*��ӡ����*/
                        const t_FIDTRACEINFO*   fidtrace,       /*��¼��fid�õ�����Ӧ��trace��Ϣ���ƿ��ؽṹ��*/
                        const XCHAR*            prt_time,       /*��¼�¼�����ʱ��*/
                        const XCHAR*            fid_name,       /*�߳�����*/  
                        const XCHAR*            out_filename,   /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
                        const XU32              ulLineNum,      /*�к�*/
                        const XCHAR*            msg_format      /*��¼��Ϣ���������ݣ���cformat,...�õ�*/ )
{
    XU32 msg_curlen = 0;   /*����Ϣ���ĳ��ȼ�¼*/
    XU32 msg_seglen = 0;    /*��¼ÿ�δ������ӳ���*/
    
    /*�������õ�cli����,��ʽ������ַ���*/
    /*����{ʱ�䣺�ļ��� ���к�}  FID������Ϣ����*/
    XOS_MemSet(msg_filter,0x0,size);
    msg_curlen = XOS_Sprintf(msg_filter,size,
        "%s:{",g_PrintLevelName[eLevel]);
    if ( XTRUE == fidtrace->isNeedTime )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            size - msg_curlen,
            "%s,",prt_time);
        msg_curlen  += msg_seglen;
    }
    
    if ( XTRUE ==  fidtrace->isNeedFileName )
    {
        /*��ӡ�ļ���*/
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            size - msg_curlen,
            "%s,",out_filename);
        msg_curlen  +=msg_seglen;
    }
    
    if ( XTRUE == fidtrace->isNeedLine )
    {
        /*��ӡ�к�*/
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            size - msg_curlen,
            "%d",ulLineNum);
        msg_curlen  +=msg_seglen;
    }
    
    msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
        size - msg_curlen,
        "}%s: %s\r\n",fid_name,msg_format);
    msg_curlen  +=msg_seglen;
    
    /*��֤���Ľ���*/
    msg_filter[size -1] = '\0';
}
//end added for cpu efficiency zenghaixin 20100802

#if 0
extern int XOS_ShouldTrace(const XCHAR* file_name, XU32 line, XU32 fid, e_PRINTLEVEL level);
#endif

/*---------------------------------------------------------------------
��������  XOS_Trace
��  �ܣ�  ��Ϣ��ӡ����
��  �룺  FileName       - ���룬�ļ���(��__FILE__)
          ulLineNum      - ���룬�к�   (��__LINE__)
          ulFid          - ���룬���ܿ�ID
          ulLevel        - ���룬��ӡ����
          ucFormat       - ���룬��ӡ��ʽ���ַ���
          ...            - ���룬��ӡ����

��  ����  XSUCC  -  �ɹ�          XERROR -  ʧ��
˵  �����ú���Ϊ�û������Ϣ�ӿ�
        ���g_ulTraceAllToTraceTsk��־�رգ���������ն˵���Ϣֱ�Ӵ�ӡ
        ���g_ulTraceAllToTraceTsk����������Ϣ���͸�trace����ģ��
        ���ڴ�ӡ������Ŀ���豸����Ϣ�����͵�trace����ģ����д���
------------------------------------------------------------------------*/
#if 0
XVOID XOS_Trace( const XCHAR* FileName, XU32 ulLineNum, XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, ... )
{
    //modified for cpu efficiency zenghaixin 20100802
    //XU32   msg_curlen                     = 0;     /*����Ϣ���ĳ��ȼ�¼*/
    XS32   format_len                       = 0;     /*��Ϣ����������*/
    XS32   callFlag                         = 0;     
    //end modified for cpu efficiency zenghaixin 20100802
    //XU32   msg_seglen                        = 0;     /*��¼ÿ�δ������ӳ���*/
    XCHAR  msg_filter[MAX_TRACE_INFO_LEN ]   = {0};   /*��¼������Ϣ����ʽ*/
    XCHAR  msg_format[MAX_TRACE_INFO_LEN ]   = {0};   /*��¼��Ϣ���������ݣ���cformat,...�õ�*/
    XCHAR  out_filename[MAX_FILE_NAME_LEN]   = {0};   /*��¼�ɾ���·��FileName�����õ����ļ���*/
    XCHAR  prt_time[MAX_FILE_NAME_LEN]       = {0};   /*��¼�¼�����ʱ��*/
    XCHAR  fid_name[MAX_FILE_NAME_LEN]       = {0};   /*�߳�����*/
    //XU32   test; 
    
    //#ifdef XOS_NEED_CHK
    va_list ap;
    //#endif
    t_FIDTRACEINFO* fidtrace            = XNULL; /*��¼��fid�õ�����Ӧ��trace��Ϣ���ƿ��ؽṹ��*/
    if ( SWITCH_OFF == g_ulTraceIsSetup || XNULL == cFormat)
    {
        return ;
    }

#ifdef XOS_NEED_CHK
    
    //��ʽ������ĸ�����
    /*�õ���Ϣ������*/
    /*���𲻶�*/
    if ( eLevel >= PL_MAX|| eLevel < PL_MIN )
    {
        return;
    }
 
    if ( XNULL == cFormat )
    {
        return;
    }
#if 0 
        va_start( ap, cFormat );
        //msg_curlen=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);//modified for cpu efficiency zenghaixin 20100802
        va_end( ap );
        //if(msg_curlen<=0)
        if(format_len<=0)//modified for cpu efficiency zenghaixin 20100802
        {
            return;
        }
    }
    
//#if 0 //deleteded for cpu efficiency zenghaixin 20100802
    /*�õ��ļ����ƴ�*/
    if ( XSUCC !=  Trace_abFileName(FileName,out_filename,MAX_FILE_NAME_LEN) )
    {
        return;
    }
    msg_curlen=msg_curlen+XOS_StrLen(out_filename);
    
    /*����ϵͳʱ�䣬�õ���Ӧ��ʱ���ַ���*/
    if ( XERROR == TRC_formateloctime(prt_time) )
    {
        prt_time[0] = '\0';
    }
    msg_curlen=msg_curlen+XOS_StrLen(prt_time);
    
    /*�õ�ģ������*/
    if ( XNULLP != XOS_getFidName(ulFid) )
    {
        XOS_Sprintf(fid_name,MAX_FILE_NAME_LEN,"%s",XOS_getFidName(ulFid));
    }else
    {
        XOS_Sprintf(fid_name,MAX_FILE_NAME_LEN,"Unknown_FID[%d]",ulFid);
    }
    msg_curlen=msg_curlen+XOS_StrLen(fid_name);
    
    if(msg_curlen > MAX_TRACE_INFO_LEN-16)
    {
        return;
    }
#endif //end deleteded for cpu efficiency zenghaixin 20100802   
#else
    return;
#endif
    ///////////////
    /*�õ���fid��trace����*/
    fidtrace = MOD_getFidTraceInfo(ulFid);
    if ( XNULL == fidtrace )
    {
        va_start( ap, cFormat );
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        va_end( ap );
        if(format_len<=0)
        {
            return;
        }
        //δע��ģ���Trace��Ϣ�������ӡ�����������־����
        if ( XOS_UNREG_FID_TRACE_LEV <= eLevel )
        {
            //added for cpu efficiency zenghaixin 20100802
            //�жϸ�ʽ���ַ���������ȷ��
            if( 0 == XOS_JudgeMsgFormat( FileName,      /*�ļ���(��__FILE__)*/
                out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
                prt_time,        /*��¼�¼�����ʱ��*/
                ulFid,            /*���ܿ�ID*/
                fid_name,        /*�߳�����*/  
                format_len        /*��Ϣ����������*/ ) )
            {
                return;
            }
            callFlag = 1;
            //end added for cpu efficiency zenghaixin 20100802
            XOS_PrinToMcb("%s",msg_format);
            XOS_CliInforPrintf("%s",msg_format);
        }
        
        if ( eLevel >= g_lPrintToLogLevel )
        {
            //added for cpu efficiency zenghaixin 20100802
            if(0 == callFlag)
            {
                if( 0 == XOS_JudgeMsgFormat( FileName,      /*�ļ���(��__FILE__)*/
                    out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
                    prt_time,        /*��¼�¼�����ʱ��*/
                    ulFid,            /*���ܿ�ID*/
                    fid_name,        /*�߳�����*/  
                    format_len        /*��Ϣ����������*/ ) )
                {
                    return;
                }
            }
            //end added for cpu efficiency zenghaixin 20100802
            //������־����ı������
            Log_Write(ulFid,eLevel,msg_format);
        }
        return;
    }
    
#if 0 //deleteded for cpu efficiency zenghaixin 20100802
    /*�������õ�cli����,��ʽ������ַ���*/
    /*����{ʱ�䣺�ļ��� ���к�}  FID������Ϣ����*/
    XOS_MemSet(msg_filter,0x0,sizeof(msg_filter));
    msg_curlen=0;
    msg_curlen = XOS_Sprintf(msg_filter,MAX_TRACE_INFO_LEN,
        "%s:{",g_PrintLevelName[eLevel]);
    if ( XTRUE == fidtrace->isNeedTime )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            "%s,",prt_time);
        msg_curlen  += msg_seglen;
    }
    
    if ( XTRUE ==  fidtrace->isNeedFileName )
    {
        /*��ӡ�ļ���*/
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            "%s,",out_filename);
        msg_curlen  +=msg_seglen;
    }
    
    if ( XTRUE == fidtrace->isNeedLine )
    {
        /*��ӡ�к�*/
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            "%d",ulLineNum);
        msg_curlen  +=msg_seglen;
    }
    
    msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
        MAX_TRACE_INFO_LEN - msg_curlen,
        "}%s: %s\r\n",fid_name,msg_format);
    msg_curlen  +=msg_seglen;
    
    /*��֤���Ľ���*/
    msg_filter[MAX_TRACE_INFO_LEN -1] = '\0';
#endif //end deleteded for cpu efficiency zenghaixin 20100802   
    ///////////////
    
    /*������й��ܿ��ӡ���ƿ��ض��رգ����������Ļ�κ���Ϣ*/
    if ( SWITCH_OFF == g_ulTraceAllFidCloseFlg )
    {
        /*��־�ļ����ǵ�д*/
        if ( eLevel >= g_lPrintToLogLevel )
        {
            va_start( ap, cFormat );
            format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
            va_end( ap );
            if(format_len<=0)
            {
                return;
            }
            //added for cpu efficiency zenghaixin 20100802
            //�жϸ�ʽ���ַ���������ȷ��
            if( 0 == XOS_JudgeMsgFormat( FileName,      /*�ļ���(��__FILE__)*/
                out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
                prt_time,        /*��¼�¼�����ʱ��*/
                ulFid,            /*���ܿ�ID*/
                fid_name,        /*�߳�����*/  
                format_len     /*��Ϣ����������*/ ) )
            {
                return;
            }
            //��������Ϣ����ʽ
            XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
                prt_time, fid_name, out_filename, ulLineNum, msg_format);
            //end added for cpu efficiency zenghaixin 20100802
            Log_Write(ulFid,eLevel,msg_filter);
        }
        return;
    }
    
    /*�����ʱ��ƽ̨�������Ϣ����ת��cpstrace,����trace��ɵ�������ϵ*/
    if ( FID_XOSMIN <= ulFid && ulFid < FID_XOSMAX )
    {
#ifdef XOS_NEED_CHK
        //added for cpu efficiency zenghaixin 20100802
        va_start( ap, cFormat );
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        va_end( ap );
        if(format_len<=0)
        {
            return;
        }
        //�жϸ�ʽ���ַ���������ȷ��
        if( 0 == XOS_JudgeMsgFormat( FileName,      /*�ļ���(��__FILE__)*/
            out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
            prt_time,        /*��¼�¼�����ʱ��*/
            ulFid,            /*���ܿ�ID*/
            fid_name,        /*�߳�����*/  
            format_len     /*��Ϣ����������*/ ) )
        {
            return;
        }

        XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
            prt_time, fid_name, out_filename, ulLineNum, msg_format);
        
        //end added for cpu efficiency zenghaixin 20100802
        XOS_PrinToMcb("%s",msg_filter);
        XOS_CpsTrace(FileName, ulLineNum, ulFid, eLevel, msg_format);
        return ;
#endif
    }
    
    /*FID������*/
    if ( XFALSE== XOS_isValidFid(ulFid))
    {
        return;
    }

#if 0
    if (XOS_ShouldTrace(FileName, ulLineNum, ulFid, eLevel) != XSUCC)
    {
        return;
    }
#endif

    if(eLevel <  fidtrace->traceLevel)
    {
        //��������־����С��Cli���ü����Traceֱ���������־�ļ���
        //һ������½��쳣�������logfile
        if(eLevel >= g_lPrintToLogLevel)
        {
            va_start( ap, cFormat );
            format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
            va_end( ap );
            if(format_len<=0)
            {
                return;
            }
            //added for cpu efficiency zenghaixin 20100802
            //�жϸ�ʽ���ַ���������ȷ��
            if( 0 == XOS_JudgeMsgFormat( FileName,      /*�ļ���(��__FILE__)*/
                out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
                prt_time,        /*��¼�¼�����ʱ��*/
                ulFid,            /*���ܿ�ID*/
                fid_name,        /*�߳�����*/  
                format_len     /*��Ϣ����������*/ ) )
            {
                return;
            }
            //��������Ϣ����ʽ
            XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
                prt_time, fid_name, out_filename, ulLineNum, msg_format);
            //end added for cpu efficiency zenghaixin 20100802
            Log_Write(ulFid,eLevel,msg_filter);
        }
        return ;
    }
    /*�������������*/
    /*a.Cli���ü���С����־����
    b.trace����cli���ü���*/
    switch ( fidtrace->traceOutputMode )
    {
    case TOCOSOLE:
        //added for cpu efficiency zenghaixin 20100802
        va_start( ap, cFormat );
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        va_end( ap );
        if(format_len<=0)
        {
            return;
        }
        //�жϸ�ʽ���ַ���������ȷ��
        if( 0 == XOS_JudgeMsgFormat( FileName,      /*�ļ���(��__FILE__)*/
            out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
            prt_time,        /*��¼�¼�����ʱ��*/
            ulFid,            /*���ܿ�ID*/
            fid_name,        /*�߳�����*/  
            format_len     /*��Ϣ����������*/ ) )
        {
            return;
        }
        //��������Ϣ����ʽ
        XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
            prt_time, fid_name, out_filename, ulLineNum, msg_format);
        //end added for cpu efficiency zenghaixin 20100802
        if ( XFALSE == fidtrace->isPrintInTraceTsk )
        {
            XOS_PrinToMcb("%s",msg_filter);
            XOS_CliInforPrintf("%s",msg_filter);
        }
        else
        {
            TRC_sendmsg(ulFid,eLevel,msg_filter);
        }
        break;
        
    case TOLOGFILE:
        XOS_CpsTrace()
        if ( eLevel >= g_lPrintToLogLevel )
        {
            va_start( ap, cFormat );
            format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
            va_end( ap );
            if(format_len<=0)
            {
                return;
            }
            //added for cpu efficiency zenghaixin 20100802
            //�жϸ�ʽ���ַ���������ȷ��
            if( 0 == XOS_JudgeMsgFormat( FileName,      /*�ļ���(��__FILE__)*/
                out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
                prt_time,        /*��¼�¼�����ʱ��*/
                ulFid,            /*���ܿ�ID*/
                fid_name,        /*�߳�����*/  
                format_len     /*��Ϣ����������*/ ) )
            {
                return;
            }
            //��������Ϣ����ʽ
            XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
                prt_time, fid_name, out_filename, ulLineNum, msg_format);
            //end added for cpu efficiency zenghaixin 20100802
            TRC_sendmsg(ulFid,eLevel,msg_filter);
        }
        break;
    case TOFILEANDCOS:
        //added for cpu efficiency zenghaixin 20100802
        va_start( ap, cFormat );
        format_len=XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat,ap);
        va_end( ap );
        if(format_len<=0)
        {
            return;
        }
        //�жϸ�ʽ���ַ���������ȷ��
        if( 0 == XOS_JudgeMsgFormat( FileName,      /*�ļ���(��__FILE__)*/
            out_filename,    /*��¼�ɾ���·��FileName�����õ����ļ���*/ 
            prt_time,        /*��¼�¼�����ʱ��*/
            ulFid,            /*���ܿ�ID*/
            fid_name,        /*�߳�����*/  
            format_len     /*��Ϣ����������*/ ) )
        {
            return;
        }
        //��������Ϣ����ʽ
        XOS_FillMsgFilter( msg_filter, MAX_TRACE_INFO_LEN, eLevel, fidtrace, 
            prt_time, fid_name, out_filename, ulLineNum, msg_format);
        //end added for cpu efficiency zenghaixin 20100802
        if ( XFALSE == fidtrace->isPrintInTraceTsk )
        {
            XOS_PrinToMcb("%s",msg_filter);
            XOS_CliInforPrintf("%s\r\n",msg_filter);
        }
        TRC_sendmsg(ulFid,eLevel,msg_filter);
        break;
    default:
        break;
        
    }
    return;
    /*����Ϣ����С�ڹ��˼���ʱ��������˼������ù��󣬿��ܵ�����־�ļ�Ҳ����д��*/
}
#endif 

static int XOS_TraceGetParam(const char* file_name,
        XU32 *pMsg_curlen,
        const XCHAR* fid_name,
        const XU32 line,
        const e_PRINTLEVEL eLevel,
        char* out_filename,
        char* prt_time)
{
    XU32 uCurlen = 0;
    XU32 uTmpLen = 0;

    if(XNULL == pMsg_curlen ||XNULL == file_name || XNULL == out_filename ||XNULL == prt_time ||XNULL == fid_name)
    {
        return XERROR;
    }

    Trace_abFileName(file_name, out_filename, MAX_FILE_NAME_LEN);
    TRC_formateloctime(prt_time);

    uTmpLen = (XU32)XOS_StrLen(out_filename) + (XU32)XOS_StrLen(prt_time) + (XU32)XOS_StrLen(fid_name);

    uCurlen = *pMsg_curlen + uTmpLen;

    if(uCurlen > (MAX_TRACE_INFO_LEN - 16)) 
    {
        uCurlen = (MAX_TRACE_INFO_LEN - 16);

        *pMsg_curlen = uCurlen - uTmpLen;
    }

    return XSUCC;
}

static void XOS_TraceSetOutput(const e_PRINTLEVEL eLevel, const char* prt_time, const char* out_filename, const XU32 ulLineNum, 
                               const char* fid_name, const char* msg_format,
                               char *msg_output)
{
    if(XNULL == prt_time || XNULL == out_filename || XNULL == fid_name || XNULL == msg_format 
        || XNULL == msg_output || 0 > eLevel ||PL_MAX <= eLevel)
    {
        return ;
    }
    
    XOS_MemSet(msg_output, 0, MAX_TRACE_INFO_LEN);
    XOS_Sprintf(msg_output,MAX_TRACE_INFO_LEN,
        "%s:{%s,%s,%d}%s:%s",
        g_PrintLevelName[eLevel],
        prt_time,out_filename,ulLineNum,
        fid_name,msg_format);
}

static int XOS_TraceCheckFid(const t_FIDCB *pFidCb, const XU32 ulFid, const e_PRINTLEVEL eLevel, const char* prt_time, const char* out_filename, const XU32 ulLineNum, 
                             const char* fid_name, const char* msg_format,const e_PRINTLEVEL minLevel)
{
    XCHAR   msg_output[MAX_TRACE_INFO_LEN]   = {0};

    /*ģ��δע��*/
    if ( XNULL == pFidCb )
    {
        XOS_TraceSetOutput(eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, msg_output);
        if ( eLevel >=  XOS_UNREG_FID_TRACE_LEV)
        {
            Log_Write(pFidCb, ulFid, eLevel, msg_output);

        }
        return XERROR;
    }

    if ( SWITCH_OFF == g_ulTraceAllFidCloseFlg || eLevel <  minLevel)
    {
        if (eLevel >= pFidCb->logLevel)
        {
            XOS_TraceSetOutput(eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, msg_output);
            Log_Write(pFidCb, ulFid, eLevel, msg_output);
        }
        return XERROR;
    }

    return XSUCC;
}

static void XOS_TraceSetMsg(const char* fun_name,const e_PRINTLEVEL eLevel, const char* prt_time, const char* out_filename, const XU32 ulLineNum, 
                               const char* fid_name, const char* msg_format, const t_FIDTRACEINFO* fidtrace,
                               char *msg_filter)
{
    XU32    msg_curlen  = 0;
    XU32    msg_seglen = 0;

    if(XNULL ==  fidtrace)
    {
        return;
    }
    
    XOS_MemSet(msg_filter, 0x0, MAX_TRACE_INFO_LEN);
    msg_curlen = XOS_Sprintf(msg_filter,MAX_TRACE_INFO_LEN,
        "%s:{",g_PrintLevelName[eLevel]);
    if ( XTRUE == fidtrace->isNeedTime )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            "%s", prt_time);
        msg_curlen  += msg_seglen;
    }

    if ( XTRUE ==  fidtrace->isNeedFileName )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
            MAX_TRACE_INFO_LEN - msg_curlen,
            ",%s", out_filename);
        msg_curlen  += msg_seglen;
    }
    
    msg_seglen = XOS_Sprintf(msg_filter+msg_curlen,
        MAX_TRACE_INFO_LEN - msg_curlen,
        ",%s", fun_name);
    msg_curlen  += msg_seglen;

    if ( XTRUE == fidtrace->isNeedLine )
    {
        msg_seglen = XOS_Sprintf(msg_filter+msg_curlen, MAX_TRACE_INFO_LEN - msg_curlen,
            ",%d",ulLineNum);
        msg_curlen  += msg_seglen;
    }

    // auto add \r\n
    XOS_Sprintf(msg_filter+msg_curlen, MAX_TRACE_INFO_LEN + 4 - msg_curlen,
        "}%s: %s\r\n", fid_name, msg_format);
}
/*---------------------------------------------------------------------
��������  XOS_CpsTrace
��  �ܣ�  ���ƽ̨�ڲ���trace��ӡ��Ϣ�������ӡ���ն˲��Ҽ�¼����־�ļ���
��  �룺      FileName  -�ļ���
        ulFid           - ���룬���ܿ�ID
          ulLineNum     - �к�
          ulLevel         - ���룬��ӡ����
          ucFormat        - ���룬��ӡ��ʽ���ַ���
          ...             - ���룬��ӡ����

��  ����
����ֵ:  XSUCC  -   �ɹ��� XERROR - ʧ��
˵  ���� �������ĳ��ȴ���MAX_TRACE_INFO_LEN-1�����ʧ�ܣ��Ҳ�������ʾ
------------------------------------------------------------------------*/
XVOID XOS_CpsTrace( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, ... )
{
    XS32    msg_curlen  = 0;
    XCHAR   msg_format[MAX_TRACE_INFO_LEN+1]  = {0}; 
    XCHAR   msg_filter[MAX_TRACE_INFO_LEN+5]  = {0}; /*5 bytes for \r\n*/
    XCHAR   out_filename[MAX_FILE_NAME_LEN] = {0};
    XCHAR   fid_name[MAX_FILE_NAME_LEN]     = {0};
    XCHAR   prt_time[MAX_FILE_NAME_LEN]     = {0};
    t_FIDTRACEINFO* fidtrace = NULL;
    t_FIDCB *pFidCb = NULL;
    va_list ap;
    e_PRINTLEVEL min_level = PL_MAX;    /*����telnet�ն���������͵Ĵ�ӡ�������ڿ���log��ӡ*/
    XS32 i = 0;

    if (SWITCH_OFF == g_ulTraceIsSetup || XNULL == cFormat || eLevel >= PL_MAX || eLevel < PL_MIN)
    {
        return ;
    }
#if 0
    if (XOS_ShouldTrace(FileName, ulLineNum, ulFid, eLevel) != XSUCC)
    {
        return;
    }
#endif
    pFidCb = MOD_getFidCb( ulFid);

    if(XNULLP != pFidCb) 
    {
        fidtrace = &(pFidCb->traceInfo);

        min_level = pFidCb->logLevel;
        
        for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
        {
            if(g_paUserSIDLogined[i].xs32SID > 0 && g_paUserSIDLogined[i].xbSwitch)
            {
                min_level = min_level < pFidCb->traceInfo.sessionLevel[g_paUserSIDLogined[i].xs32SID] ? min_level :pFidCb->traceInfo.sessionLevel[g_paUserSIDLogined[i].xs32SID];
            }
        }
    }
    else
    {
        min_level = XOS_UNREG_FID_TRACE_LEV;
    }

    /*��������־�ļ���С�����õļ��𣬾�ֱ�ӷ���*/
    if(XOS_TraceCheckSessionLevel(pFidCb, eLevel,min_level) == XERROR) 
    {
        return;
    }

    va_start(ap, cFormat);
    msg_curlen = XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat, ap);
    va_end(ap);

    if (msg_curlen <= 0)
    {
        return;
    }

    if(XNULLP != pFidCb) 
    {
        if(NULL == pFidCb->fidName) 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
        } 
        else 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "%s", pFidCb->fidName);
        }
    } 
    else 
    {
        XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
    }

    if (XOS_TraceGetParam(FileName, (XU32 *)&msg_curlen, fid_name, ulLineNum, eLevel, out_filename, prt_time) == XERROR)
    {
        return ;
    }

    msg_format[msg_curlen] = '\0';
    
    if(XOS_TraceCheckFid(pFidCb, ulFid, eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format,min_level) == XERROR)
    {
        return;
    }

    XOS_TraceSetMsg(FunName,eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, fidtrace, msg_filter);

    switch ( fidtrace->traceOutputMode )
    {
    case TOCOSOLE: /*��ӡ���ն�*/
        if (XFALSE == fidtrace->isPrintInTraceTsk)
        {
            XOS_PrintToMcbStr(msg_filter);
        }
        XOS_CliInforPrintfSessionStr(pFidCb->traceInfo.sessionLevel,eLevel,msg_filter);

        break ;
    case TOLOGFILE: /*��ӡ���ļ�*/
        Log_Write(pFidCb, ulFid,eLevel,msg_filter);
        break ;

    case TOFILEANDCOS:/*��ӡ���ն˺��ļ�*/
        if (XFALSE == fidtrace->isPrintInTraceTsk)
        {
            XOS_PrintToMcbStr(msg_filter);
        }
        XOS_CliInforPrintfSessionStr(pFidCb->traceInfo.sessionLevel,eLevel,msg_filter);

        Log_SessionWrite(pFidCb, ulFid,eLevel,min_level,msg_filter);
        break ;
    default:
        break ;
    }
}

/*---------------------------------------------------------------------
��������  XOS_TraceTa
��  �ܣ�  ���ƽ̨�ڲ���trace��ӡ��Ϣ�������ӡ���ն˲��Ҽ�¼����־�ļ���
��  �룺 FileName       - �ļ���
        ulLineNum       - �к�
        ulFid           - ���룬���ܿ�ID
          ulLevel         - ���룬��ӡ����
          ucFormat        - ���룬��ӡ��ʽ���ַ���
          ...             - ���룬��ӡ����
��  ����
����ֵ:  XSUCC  -   �ɹ��� XERROR - ʧ��
˵  ���� �������ĳ��ȴ���MAX_TRACE_INFO_LEN-1�����ʧ�ܣ��Ҳ�������ʾ
------------------------------------------------------------------------*/
XVOID XOS_TraceTa( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, va_list ap )
{
    XS32    msg_curlen  = 0;
    XCHAR   msg_format[MAX_TRACE_INFO_LEN+1]  = {0}; 
    XCHAR   msg_filter[MAX_TRACE_INFO_LEN+5]  = {0}; /*5 bytes for \r\n*/
    XCHAR   out_filename[MAX_FILE_NAME_LEN] = {0};
    XCHAR   fid_name[MAX_FILE_NAME_LEN]     = {0};
    XCHAR   prt_time[MAX_FILE_NAME_LEN]     = {0};
    t_FIDTRACEINFO* fidtrace = NULL;
    t_FIDCB *pFidCb = NULL;
    e_PRINTLEVEL min_level = PL_MAX;    /*����telnet�ն���������͵Ĵ�ӡ�������ڿ���log��ӡ*/
    XS32 i = 0;

    if (SWITCH_OFF == g_ulTraceIsSetup || XNULL == cFormat || eLevel >= PL_MAX || eLevel < PL_MIN)
    {
        return ;
    }
#if 0
    if (XOS_ShouldTrace(FileName, ulLineNum, ulFid, eLevel) != XSUCC)
    {
        return;
    }
#endif
    pFidCb = MOD_getFidCb( ulFid);

    if(XNULLP != pFidCb) 
    {
        fidtrace = &(pFidCb->traceInfo);

        min_level = pFidCb->logLevel;
        
        for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
        {
            if(g_paUserSIDLogined[i].xs32SID > 0 && g_paUserSIDLogined[i].xbSwitch)
            {
                min_level = min_level < pFidCb->traceInfo.sessionLevel[g_paUserSIDLogined[i].xs32SID] ? min_level :pFidCb->traceInfo.sessionLevel[g_paUserSIDLogined[i].xs32SID];
            }
        }
    }
    else
    {
        min_level = XOS_UNREG_FID_TRACE_LEV;
    }

    /*��������־�ļ���С�����õļ��𣬾�ֱ�ӷ���*/
    if(XOS_TraceCheckSessionLevel(pFidCb, eLevel,min_level) == XERROR) 
    {
        return;
    }

    msg_curlen = XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat, ap);

    if (msg_curlen <= 0)
    {
        return;
    }

    if(XNULLP != pFidCb) 
    {
        if(NULL == pFidCb->fidName) 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
        } 
        else 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "%s", pFidCb->fidName);
        }
    } 
    else 
    {
        XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
    }

    if (XOS_TraceGetParam(FileName, (XU32 *)&msg_curlen, fid_name, ulLineNum, eLevel, out_filename, prt_time) == XERROR)
    {
        return ;
    }

    msg_format[msg_curlen] = '\0';
    
    if(XOS_TraceCheckFid(pFidCb, ulFid, eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format,min_level) == XERROR)
    {
        return;
    }

    XOS_TraceSetMsg(FunName,eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, fidtrace, msg_filter);

    switch ( fidtrace->traceOutputMode )
    {
    case TOCOSOLE: /*��ӡ���ն�*/
        if (XFALSE == fidtrace->isPrintInTraceTsk)
        {
            XOS_PrintToMcbStr(msg_filter);
        }
        XOS_CliInforPrintfSessionStr(pFidCb->traceInfo.sessionLevel,eLevel,msg_filter);

        break ;
    case TOLOGFILE: /*��ӡ���ļ�*/
        Log_Write(pFidCb, ulFid,eLevel,msg_filter);
        break ;

    case TOFILEANDCOS:/*��ӡ���ն˺��ļ�*/
        if (XFALSE == fidtrace->isPrintInTraceTsk)
        {
            XOS_PrintToMcbStr(msg_filter);
        }
        XOS_CliInforPrintfSessionStr(pFidCb->traceInfo.sessionLevel,eLevel,msg_filter);

        Log_SessionWrite(pFidCb, ulFid,eLevel,min_level,msg_filter);
        break ;
    default:
        break ;
    }
}
/*---------------------------------------------------------------------
��������  XOS_TraceInfo
��  �ܣ�  log���ٽӿ��ڵ��ã���������log�ַ���
��  �룺  FileName       - �ļ���
            ulLineNum   - �к�
          ulFid           - ���룬���ܿ�ID
          ulLevel         - ���룬��ӡ����
          cFormat       - ��ӡ��ʽ���ַ���
          ap             - ���룬va_list�����ַ�������
          msgLen             - ���룬��ӡ���ַ����ĳ���

��  ����
����ֵ:  XSUCC  -   �ɹ��� XERROR - ʧ��
˵  ���� ������TAģ����־����ʱ���ã�
        �������ĳ��ȴ���MAX_TRACE_INFO_LEN-1�����ʧ�ܣ��Ҳ�������ʾ��
------------------------------------------------------------------------*/
XVOID XOS_TraceInfo( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat,va_list ap ,XCHAR *buf)
{
    XS32    msg_curlen = 0;
    XCHAR   msg_format[MAX_TRACE_INFO_LEN+1]  = {0}; 
    XCHAR   out_filename[MAX_FILE_NAME_LEN] = {0};
    XCHAR   fid_name[MAX_FILE_NAME_LEN]     = {0};
    XCHAR   prt_time[MAX_FILE_NAME_LEN]     = {0};
    t_FIDCB *pFidCb = NULL;
    t_FIDTRACEINFO* fidtrace = NULL;

    if (SWITCH_OFF == g_ulTraceIsSetup || eLevel >= PL_MAX || eLevel < PL_MIN)
    {
        return ;
    }

    pFidCb = MOD_getFidCb( ulFid);
    if(XNULLP != pFidCb) 
    {
        fidtrace = &(pFidCb->traceInfo);
    }
    
    msg_curlen = XOS_VsPrintf(msg_format, sizeof(msg_format)-1, cFormat, ap);

    if (msg_curlen <= 0)
    {
        return;
    }

    if(XNULLP != pFidCb) 
    {
        if(NULL == pFidCb->fidName) 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
        } 
        else 
        {
            XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "%s", pFidCb->fidName);
        }
    }
    else 
    {
        XOS_Sprintf(fid_name, MAX_FILE_NAME_LEN, "Unknown_FID[%d]", ulFid);
    }

    if (XOS_TraceGetParam(FileName, (XU32 *)&msg_curlen, fid_name, ulLineNum, eLevel, out_filename, prt_time) == XERROR)
    {
        return ;
    }

    msg_format[msg_curlen] = '\0';
    
    XOS_TraceSetMsg(FunName,eLevel, prt_time, out_filename, ulLineNum, fid_name, msg_format, fidtrace, buf);

}

/*---------------------------------------------------------------------
��������  XOS_CpsPrintf
��  �ܣ�  ���ƽ̨�ڲ���trace��ӡ��Ϣ�������ӡ���ն˲��Ҽ�¼����־�ļ���
��  �룺  ulFid            - ���룬���ܿ�ID
          ulLevel          - ���룬��ӡ����
          etraceOutputMode - ����, ������ն˻��ļ� 
          ucFormat         - ���룬��ӡ��ʽ���ַ���
          ap               - ����, ��ε�ջ��ʼ��ַ

��  ����
����ֵ:  XSUCC  -   �ɹ��� XERROR - ʧ��
˵  ���� �������ĳ��ȴ���MAX_TRACE_INFO_LEN�����ʧ�ܣ��Ҳ�������ʾ
------------------------------------------------------------------------*/
XVOID XOS_CpsPrintf(XU32 ulFid, e_PRINTLEVEL eLevel, e_TRACEPRINTDES etraceOutputMode, const XCHAR *cFormat, va_list ap)
{
    XCHAR   msg_format[MAX_TRACE_INFO_LEN+1]  = {0}; 
    XS32    msg_curlen  = 0;
    t_FIDCB *pFidCb = NULL;

    pFidCb = MOD_getFidCb( ulFid);

    /*��������־�ļ���С�����õļ��𣬾�ֱ�ӷ���*/
    if(XOS_TraceCheckLevel(pFidCb, eLevel) == XERROR) 
    {
        return;
    }   

#ifdef XOS_WIN32
    msg_curlen =  _vsnprintf(msg_format,sizeof(msg_format)-1, cFormat, ap);
#endif
    //#if ( XOS_LINUX  || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )
    msg_curlen = vsnprintf(msg_format, sizeof(msg_format)-1, cFormat, ap);
#endif
    //#if ( XOS_VXWORKS || XOS_VTA )
#if ( defined(XOS_VXWORKS) || defined(XOS_VTA) )
    msg_curlen = vx_vsnprintf(msg_format,sizeof(msg_format)-1,cFormat,ap);
#endif  


    if (msg_curlen <= 0)
    {
       return;
    }

    switch ( etraceOutputMode )
    {
    case TOCOSOLE: /*��ӡ���ն�*/
        //XOS_CliInforPrintfStr(msg_format);
        break ;
    case TOLOGFILE: /*��ӡ���ļ�*/
        Log_Write(pFidCb, ulFid, eLevel, msg_format);
         break ;
    case TOFILEANDCOS:/*��ӡ���ն˺��ļ�*/
        break ;
    default:
        break ;
    }
}


/*---------------------------------------------------------------------
��������  XOS_getPLName
��  �ܣ�  ��ȡ��ӡ���������
��  �룺
��  ����
����ֵ:
˵  ����
------------------------------------------------------------------------*/
XCHAR* XOS_getPLName(e_PRINTLEVEL printLevel)
{
    if( printLevel >= PL_MAX|| printLevel <PL_MIN )
    {
        return (XCHAR*)XNULLP;
    }
    else
    {
        return g_PrintLevelName[printLevel];
    }
}

/*---------------------------------------------------------------------
��������  XOS_getoutModeName
��  �ܣ�  ��ȡ��ӡĿ���豸����
��  �룺
��  ����
����ֵ:
˵  ����
------------------------------------------------------------------------*/
XCHAR* XOS_getoutModeName(e_TRACEPRINTDES desmode)
{
    if ( desmode > MAX_DEV || desmode < TOCOSOLE )
    {
        return (XCHAR*)XNULLP;
        
    }
    else
    {
        return  g_PrintDevName[desmode];
    }
    
}

/*----------------------------------------------
��ӡ���������лص�����
-------------------------------------------------*/
/*---------------------------------------------------------------------
��������  XOS_Sprintf
��  �ܣ�  ��ʽ���ַ�������.��sprintf������ͬ
��  �룺  Buffer    -��Ÿ�ʽ������ַ����Ļ�����.
          charnum   -�������Ĵ�С.
          cFormat   -��ʽ�����ַ���
          ...       -�������������ǰ��ĸ�ʽ���ַ���ƥ��
��  ����
����ֵ: �ɹ�����д���ʵ���ֽ���ʧ�ܷ���0ֵ
˵����
------------------------------------------------------------------------*/
XS32 XOS_Sprintf( XCHAR *Buffer, XU32 charnum, const XCHAR *cFormat, ... )
{
    XS32   length                      = 0;
    va_list ap;
    
    va_start( ap, cFormat );
    length = XOS_VsPrintf(Buffer,charnum,cFormat,ap);
    va_end(ap);

    return length;
}

/*------------------------------------------------------------------
��������  Trace_msgProc
��  �ܣ�  ��Ϣ������
��  �룺
��  ����
����ֵ:
˵  ����
------------------------------------------------------------------------*/
XS8 Trace_msgProc(XVOID* pMsP, XVOID*sb )
{
    
    t_XOSCOMMHEAD*   msg      = XNULL;
    t_FIDTRACEINFO*  fidtrace = XNULL;
    e_TRACEPRINTDES  outputmode;
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    
    
    if ( XNULL == pMsP )
    {
        return XERROR;
    }
    
    msg  = (t_XOSCOMMHEAD *) pMsP;

//add by gcg
//��ӡҵ�����Ϣ 
    if(FID_TRACE == msg->datasrc.FID
       && MSGID_PRINTO_MCB == msg->msgID
       && msg->datasrc.PID != msg->datadest.PID)
    {
        XOS_CliInforPrintf( "%s\r\n",msg->message );
        return XSUCC;
    }
//!add
    pFidCb = MOD_getFidCb( msg->datasrc.FID);
    
    /*δע��ģ��*/
    if ( XNULL == pFidCb )
    {
        return XERROR;
    }

    fidtrace = &(pFidCb->traceInfo);
    outputmode = fidtrace->traceOutputMode;
    
    /*����traceģ���е���Ϣ���Ŀ���豸���أ���������������Ǹ��豸�ļ���*/
    switch ( outputmode )
    {
    case TOCOSOLE:
        XOS_CliInforPrintf( "%s\r\n",msg->message );
        break;
        
    case TOFILEANDCOS:
        if ( XTRUE == fidtrace->isPrintInTraceTsk )
        {
            /*if cli msg output in thread,ignore*/
            XOS_CliInforPrintf("%s\r\n",msg->message);
        }
        Log_Write(pFidCb, msg->datasrc.FID,(e_PRINTLEVEL)msg->msgID,msg->message);
        break;
        
    case TOLOGFILE:
        Log_Write(pFidCb, msg->datasrc.FID,(e_PRINTLEVEL)msg->msgID,msg->message);
        break;
        
    case TOSERVER:
        /*����չ*/
        break;
        
    case TOTELNET:
        /*����չ*/
        break;
        
    default :
        break;
    }
    return XSUCC;
    
}

/******************************************************
��������  Trace_abFileName
��  �ܣ�  �Ӿ���·���н����ļ�������
��  �룺  FileName        -����·��
          fnstr           -���յ��ļ���
          Max_FileNameLen -�ļ�������󳤶�
��  ����
����ֵ:   �ɹ�����xsucc;ʧ�ܷ���xerror;
˵  ����
*******************************************************/
XS32 Trace_abFileName(XCONST XCHAR *FileName,XCHAR *fnstr,XU32 Max_FileNameLen)
{
    XU32   filelen = 0; /*��¼����·��FileName�����filename�ĳ���*/
    XCHAR* pSrc    = XNULLP;
    int nameLen = 0;
    
    if ( XNULL == fnstr ||  XNULL == FileName)
    {
        return XERROR;
    }

    nameLen = (XU32)XOS_StrLen(FileName);
    
    pSrc = (XCHAR*)FileName + nameLen;
    
    /*�����ҵ�����·�����ļ����ϲ��ļ���*/
    for(filelen = 0; '\\' != *pSrc && '/' != *pSrc; pSrc--)
    {
        filelen++;
        if('.' == *pSrc)
        {
            filelen = 0;
        }
        if ( filelen > (XU32)nameLen )
        {
            break;
        }
    }
    
    filelen = XOS_MIN(filelen, Max_FileNameLen);
    
    XOS_StrNcpy(fnstr, pSrc+1, filelen);
    
    return XSUCC;
    
}

/*--------------------------------------------------------------------
��������  XOS_GetFidTraceFlag
��  �ܣ�  ��ȡFID��ǰtrace��Ϣ�����־λ
��  �룺
          ulFid          -���ܿ�ID
          ulFidTraceInfo -�����¼trace�ĸ�����־λ�ṹ
��  ����
����ֵ:  �ɹ�����XSUCC��ʧ�ܷ���XERROR
˵  ����
------------------------------------------------------------------------*/
XS32 XOS_GetFidTraceFlag(XU32 ulFid, t_FIDTRACEINFO *ulFidTraceInfo)
{
    t_FIDTRACEINFO* fidtrace = XNULL;
    
    if ( XNULL == ulFidTraceInfo )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_GetFidTraceFlag()-> input param is null!");
        return XERROR;
    }
    
    if ( !XOS_isValidFid(ulFid) )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_GetFidTraceFlag()-> input ulFid is illegal!");
        return XERROR;
    }
    
    fidtrace = MOD_getFidTraceInfo(ulFid);
    
    if ( XNULL == fidtrace )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_GetFidTraceFlag()-> can not get fid-trace flag!");
        return XERROR;
    }
    
    ulFidTraceInfo->isNeedFileName    = fidtrace->isNeedFileName;
    ulFidTraceInfo->isNeedLine        = fidtrace->isNeedLine;
    ulFidTraceInfo->isNeedTime        = fidtrace->isNeedTime;
    ulFidTraceInfo->isPrintInTraceTsk = fidtrace->isPrintInTraceTsk;
    ulFidTraceInfo->traceLevel        = fidtrace->traceLevel;
    ulFidTraceInfo->traceOutputMode   = fidtrace->traceOutputMode;
    
    return XSUCC;
    
}

/*--------------------------------------------------------------------
��������  XOS_SetFidTraceFlag
��  �ܣ�  ����FID��Ӧtrace��Ϣ�����־λ
��  �룺
          ulFid           -���ܿ�ID
          FilenameFlag    -�����Ƿ��ӡ�ļ�����־��TRUE-��ӡ��FALSE-����ӡ��
          LinenumFlag     -�����Ƿ��ӡ�кű�־��TRUE-��ӡ��FALSE-����ӡ��
          TimeFlag        -�����Ƿ��ӡʱ���־��TRUE-��ӡ��FALSE-����ӡ��
          TransToTaskFlag -�����Ƿ��䵽��Ϣ���б�־��TRUE-����FALSE-������
          OutputMode      -�������Ŀ���豸��־����e_TRACEPRINTDES �ṹ��
          OutputLevel     -���ø�FID����Ϣ��ӡ���˼��������Ϣ������ڸü���ʱ�����������e_PRINTLEVEL�ṹ

��  ����
����ֵ:  �ɹ�����XSUCC��ʧ�ܷ���XERROR
˵  ����
------------------------------------------------------------------------*/
XS32 XOS_SetFidTraceFlag(XU32 ulFid,XBOOL FilenameFlag,XBOOL LinenumFlag,XBOOL TimeFlag,
                         XBOOL TransToTaskFlag, e_TRACEPRINTDES  OutputMode, e_PRINTLEVEL OutputLevel)
{
    t_FIDTRACEINFO* fidtrace = XNULL;
    
    if ( !XOS_isValidFid(ulFid) )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_SetFidTraceFlag()-> input ulFid is illegal!\r\n");
        return XERROR;
    }
    
    if ( ( FilenameFlag !=  XTRUE && FilenameFlag != XFALSE )
        ||( LinenumFlag !=  XTRUE && LinenumFlag != XFALSE )
        ||( TimeFlag !=  XTRUE && TimeFlag != XFALSE )
        ||( TransToTaskFlag !=  XTRUE && TransToTaskFlag != XFALSE )
        ||( OutputMode < TOCOSOLE || OutputMode > TOSERVER )
        ||( OutputLevel < PL_MIN || OutputLevel >= PL_MAX )  )
    {
        XOS_Trace(MD(ulFid,PL_ERR),"XOS_SetFidTraceFlag()-> input TRACE FLAGS are illegal!\r\n");
        return XERROR;
    }
    
    fidtrace = MOD_getFidTraceInfo(ulFid);
    
    if ( XNULL == fidtrace )
    {
        XOS_Trace(MD(ulFid,PL_WARN), "XOS_SetFidTraceFlag()->set fid trace flag error!\r\n");
        return XERROR;
    }
    
    fidtrace->isNeedFileName = FilenameFlag;
    fidtrace->isNeedLine = LinenumFlag;
    fidtrace->isNeedTime = TimeFlag;
    fidtrace->isPrintInTraceTsk = TransToTaskFlag;
    fidtrace->traceLevel = OutputLevel;
    fidtrace ->traceOutputMode = OutputMode;
    
    return XSUCC;
}

XS8 XOS_TraceClose(XU32 switchType)
{
    if(SWITCH_OFF == switchType)
    {
        g_ulTraceIsSetup =SWITCH_OFF;
    }
    if(SWITCH_ON == switchType)
    {
        g_ulTraceIsSetup =SWITCH_ON;
    }
    return XSUCC;
}

#define gcg
/*---------------------------------------------------------------------
������:  
��  ��:  �����Ƿ�ҵ����ӡ��Ϣ��������ذ�
��  ��:  pCliEnv         - ���룬�ն˺�
         siArgc          - ���룬����������Ĳ�������
         ppArgv          - ���룬����������Ĳ����б�
��  ��:
��  ��:
˵  ��:
------------------------------------------------------------------------*/
XVOID TRC_CMDSetPrinToMcbFlg(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32      ulFlag;
    
    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong\r\n");
        return;
    }

    ulFlag = atoi(ppArgv[1]);
    
    if ( 1 != ulFlag && 0 != ulFlag )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter %d is wrong!\r\n",ulFlag);
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,"set print to mcb success(%d)!\r\n",ulFlag);
    g_ulPrinToMcbFlg = ulFlag;
    
    return;
}


XS32 XOS_PrintToMcbStr(const XCHAR* buff)
{
    if( XNULL == buff )
    {
        return XERROR;
    }

    if(g_ulPrinToMcbFlg == 1)
    {
        XCHAR szBuf[MAX_BUFF_LEN] = {0};
        t_XOSCOMMHEAD* sendmsg = XNULL;
        XU32    msglen  = 0;
        XU32    ulRemotePid = 0;

        ulRemotePid = cli_getMcbPid();
        if(0xFFFFFFFF == ulRemotePid)
        {
            return XERROR;
        }

        memset(szBuf, 0, MAX_BUFF_LEN);
        //��ӡ��Ϣǰ����PID��IPǰ׺
        sprintf(szBuf,"%s %s", gszPrefixInfo, buff);

        msglen  = (XU32)XOS_StrLen(szBuf)+1;
        sendmsg = XOS_MsgMemMalloc( FID_TRACE, msglen );

        if ( XNULLP == sendmsg )
        {
            //XOS_PRINT(MD(FID_TRACE,PL_ERR),"XOS_MsgMemMalloc fail.");
            return XERROR;
        }

        /*��д��Ϣ����*/
        sendmsg->datasrc.FID    = FID_TRACE;
        sendmsg->datasrc.FsmId  = 0;
        sendmsg->datasrc.PID    = XOS_GetLocalPID();
        sendmsg->datadest.FID   = (XU32)FID_TRACE;
        sendmsg->datadest.FsmId = 0;
        sendmsg->datadest.PID   = ulRemotePid;
        sendmsg->msgID          = MSGID_PRINTO_MCB;
        sendmsg->subID          = 0;
        sendmsg->prio           = eNormalMsgPrio;
        sendmsg->length         = msglen;

        XOS_MemCpy(sendmsg->message, szBuf, msglen);

        if( XOS_MsgSend(sendmsg) != XSUCC)
        {
            XOS_MsgMemFree(sendmsg->datasrc.FID, sendmsg);
            return XERROR;
        }
    }
    return XSUCC;
}

/************************************************************************
 ������: XOS_PrinToMcb(  )
 ����:   ����ʽ���������,�����Ϣ�����ض���
 ����:   pFmt ��ʽ���ַ���
 ���:   ��
 ����:   ��
 ˵��:   ��
************************************************************************/
XS32 XOS_PrinToMcb( XCHAR* pFmt, ... )
{
    XCHAR   buf[MAX_BUFF_LEN] = {0};
    va_list pvar;

    if ( XNULL == pFmt )
    {
        return XERROR;
    }

    va_start( pvar, pFmt );
    XOS_VsPrintf( buf, MAX_BUFF_LEN, pFmt, pvar );
    va_end( pvar );
    buf[MAX_BUFF_LEN-1] = '\0';
    
    return XOS_PrintToMcbStr(buf);
}

#ifdef  __cplusplus
}
#endif


