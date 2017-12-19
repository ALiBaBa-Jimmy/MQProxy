/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosmodule.c
**
**  description:  module management implent
**
**  author: wangzongyou
**
**  date:   2006.7.19
**
***************************************************************
**  history
**
***************************************************************
**   author  date  modification
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
  ����ͷ�ļ�
-------------------------------------------------------------------------*/

#include "xosmodule.h"
#include "xosencap.h"
#include "xoscfg.h"
#include "xosqueue.h"
#include "xosarray.h"
#include "xoshash.h"
#include "xosport.h"
#include "cmtimer.h"
#include "xosmem.h"
#include "xospub.h"
#include "trace_agent.h"
#ifdef XOS_UDT_MODULE
#include "udtlib.h"
#endif
#include "ha_interface.h"

/*-------------------------------------------------------------------------
   ģ���ڲ��궨��
-------------------------------------------------------------------------*/

/*��ʱ�õĺ�*/

/*Ϊ�˱�֤���е�ƽ̨ע��ӿڲ���, ʵ�ִ�1.81 ��2.0 ��ƽ��
   ����, �Ƚ�������Ϣ���еĴ�Сд��, ��ģ�����ʱ��������
   ����Ϣ���д�С����*/

/*-------------------------------------------------------------------------
   ģ���ڲ��ṹ��ö�ٶ���
-------------------------------------------------------------------------*/

/*tid ���ƿ�,�ƶ���xosqueue.h*/


typedef struct
{
    CLI_ENV* pCliEnv;
    XS32 index;
}t_FIDSHOW;

typedef struct
{
    setFidTraceInfo setTraceFunc;
    XVOID * param;                  /*trace level*/
    XVOID * param2;                 /*telnet session ID*/
}t_ALLFIDTRACE;

/*-------------------------------------------------------------------------
               ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
t_MODMNT  g_modMnt;
extern XU32 gMMFidTraceLel;
#ifdef MEM_FID_DEBUG
extern XBOOL bModifyFidFlg;
#endif

XU32 g_XOS_LocalPid = 10; /* change by wulei for mss */
XU32 g_Xos_LogicPid = 0;

/*��ʱ�����ȫ�ֱ���, ���������ļ�����ʱӦ��ɾ��*/
/*XEXTERN t_XOSLOGINLIST * g_xosLogin;*/

/*��ʱ�����ע�ắ��, ���������ļ�����ʱӦ��ɾ��*/
XEXTERN XS32 XOS_cfgXosModule(t_XOSLOGINLIST *);
#ifndef XOS_MDLMGT
XEXTERN XS8 XOS_InfoReg(t_XOSLOGINLIST *);
#endif
XEXTERN  XBOOL MsgPflg[FID_MAX];

#ifdef XOS_MSGMEM_TRACE
XEXTERN XVOID XOS_RecordMsgBuf(XU32 fid, XVOID* pAddr);
XEXTERN XVOID XOS_UpdateMsgBuf(XVOID* pAddr);
XEXTERN XVOID XOS_ReleaseMsgBuf(XU32 fid, XVOID* pAddr);
#endif /* XOS_MSGMEM_TRACE */
XEXTERN  XCHAR* XOS_getPLName(e_PRINTLEVEL printLevel);
XEXTERN   XCHAR *XOS_getoutModeName(e_TRACEPRINTDES desmode);

XEXTERN XS32 write_to_syslog(const XS8* msg,...);

/*
XSTATIC XS32 trace_msgq_size(const XS8* msg,...);

static XS32 fid_stat[2048] = {FID_XOSMIN};
static XCHAR fid_name[2048][32] = {{0}};
static XS32 old_queue_nums = 0;
*/

/*-------------------------------------------------------------------------
                 ģ���ڲ�����
-------------------------------------------------------------------------*/
XS32 initFidTraceInfo(t_FIDTRACEINFO* traceInfo)
{
    XS32 i = 0;
    
    traceInfo->isNeedFileName=XTRUE;
    traceInfo->isNeedLine=XTRUE;
    traceInfo->isNeedTime=XTRUE;
    traceInfo->isPrintInTraceTsk=XFALSE;
    traceInfo->traceLevel=PL_ERR;
    /*��ʼ������telnet�ն˵�trace��ӡ����*/
    for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
    {
        traceInfo->sessionLevel[i] = PL_ERR;
    }
    traceInfo->traceOutputMode=TOFILEANDCOS;
     /*�����ٽ���*/
    if ( XSUCC != XOS_MutexCreate( &traceInfo->xosFidTraceMutex) )
    {
        XOS_Trace(MD(FID_ROOT, PL_EXP), "XOS_MutexCreate xosFidMutex failed!");
        return XERROR;
    }

    return XSUCC;
}


/************************************************************************
������: MOD_getFidCb
���ܣ�  ��ȡ���ܿ�Ŀ��ƿ�
���룺  fid   ����ģ���
�����  N/A
���أ�  ���ܿ��ƿ�ָ��,ʧ�ܷ��ؿ�
˵����
************************************************************************/
t_FIDCB* MOD_getFidCb(XU32 fid)
{
    /*��Ч���ж�*/
    if(!XOS_isValidFid( fid))
    {
        return XNULLP;
    }
    return (t_FIDCB*)XOS_HashElemFind(g_modMnt.fidHash, (XVOID*)&fid);
}


/************************************************************************
������: MOD_getTidCbByFid
���ܣ�  ͨ�����ܿ�Ż�ȡ����Ŀ��ƿ���Ϣ
���룺  fid   ����ģ���
�����  N/A
���أ�  ������ƿ�ָ��,ʧ�ܷ��ؿ�
˵����
************************************************************************/
XPUBLIC t_TIDCB* MOD_getTidCbByFid(XU32 fid)
{
    t_FIDCB * pFidCb = (t_FIDCB*)XNULLP;
    
    pFidCb = MOD_getFidCb( fid);
    if(pFidCb == XNULLP)
    {
        return (t_TIDCB*)XNULLP;
    }

    return  (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
}


/************************************************************************
������: MOD_TidNumCompare
���ܣ� 
���룺  fid   ����ģ���
�����  N/A
���أ�  ������ƿ�ָ��,ʧ�ܷ��ؿ�
˵����
************************************************************************/
XBOOL MOD_TidNumCompare(t_TIDCB *ptidCb , XU32 *ptidNum)
{
   if(ptidCb == XNULLP || ptidNum == XNULL)
   {
       XOS_Trace(MD(FID_ROOT, PL_ERR), " MOD_TidCbCompare: para invalid!");
       return XFALSE;
   }

   if(ptidCb->tidNum == *ptidNum)
   {
       return XTRUE;
   }
   return XFALSE;
}


/************************************************************************
������: MOD_getTidCbByTidNum
���ܣ�  ͨ������Ż�ȡ����Ŀ��ƿ���Ϣ
���룺  tidNum   �����
�����  N/A
���أ�  ������ƿ�ָ��,ʧ�ܷ��ؿ�
˵����
************************************************************************/
XPUBLIC t_TIDCB* MOD_getTidCbByTidNum(XU32 tidNum)
{
    XS32 tidIndex = -1;

    tidIndex = XOS_ArrayElemCompare(g_modMnt.tidArray, (XOS_ArrayCompare)MOD_TidNumCompare, &tidNum);
    return  (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
}


/************************************************************************
������: MOD_isSameTid
����: ������ƿ�ıȽϺ���

  ����:element1  ������ƿ��׵�ַ
  param   - �Ƚϲ���
  ���:
  ����: ��ȷ���XTRUE, ���ȷ���XFALSE
  ˵��:
************************************************************************/
XBOOL MOD_isSameTid(XOS_ArrayElement element1, XVOID *param)
{
    t_TIDCB *pTidCb = (t_TIDCB*)XNULLP;

    pTidCb = (t_TIDCB*)element1;
    if(pTidCb->tidNum == (XU32)(XPOINT)param)
    {
        return XTRUE;
    }
    else
    {
        return XFALSE;
    }
}


/************************************************************************
* ���� :
* param      - Ԫ��
* paramSize  - Ԫ�صĴ�С(  ��λ�ֽ�)
* hashSize   - hash��Ĵ�С
* ���� : hash ���
************************************************************************/
XSTATIC  XU32 MOD_hashFunc(  XVOID*  param,  XS32  paramSize,  XS32  hashSize)
{
    XS32 hash;
    hash = *((XU32*)param);

    return (hash % hashSize);
}


/************************************************************************
* MOD_showFid
* ����: �����ÿ��hashԪ��ͨ�õĺ���
* ����:
* hHash   - ������
* elem    - Ԫ��
* param   - ����
* ���:
* ����: ָ��һ�����������´ε���XOS_HASHFuncʹ��
************************************************************************/
XVOID* MOD_showFid(XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
    t_FIDCB *pFidCb;
    t_FIDSHOW *pFidShow;
    t_TIDCB *pTidCb;

    XCHAR* freeMsgType[] =
    {
        "xosMode",
            "userMode",
            "Max"
    };
    XCHAR *attSwitch[] =
    {
        "off",
        "on"
    };

    pFidCb = (t_FIDCB*)elem;
    pFidShow = (t_FIDSHOW*)param;
    pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);

    XOS_CliExtPrintf( pFidShow->pCliEnv,
        "%-6d%-6d%-18s%-18s%-10s%-10s%-10s\r\n",
        pFidShow->index,
        pFidCb->fidNum,
        pFidCb->fidName,
        (
        pTidCb !=XNULLP)?pTidCb->tskName:"noName",
        attSwitch[pFidCb->attSwitch],
        XOS_getPLName((e_PRINTLEVEL)(pFidCb->traceInfo.traceLevel)),
        freeMsgType[pFidCb->msgFreeType]
        );

    pFidShow->index++;
    return param;
}


/************************************************************************
*  MOD_showFidTraceInfo
* ����: �����ÿ��hashԪ��ͨ�õĺ��������������fid��Ӧ��trace��־
* ����:
* hHash   - ������
* elem  - Ԫ��
* param   - ����
* ���:
* ����: ָ��һ�����������´ε���XOS_HASHFuncʹ��
************************************************************************/
XVOID* MOD_showFidTraceInfo(XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
#if 1 /* wulei del for B07 2006.11.20 */
    t_FIDCB *pFidCb;
    t_FIDSHOW *pFidShow;
    /*  t_TIDCB *pTidCb;*/

    XS32 session = 0;
    e_PRINTLEVEL level = PL_MIN;
    
    XCHAR *attSwitch[] =
    {
        "off",
        "on"
    };

    pFidCb = (t_FIDCB*)elem;
    pFidShow = (t_FIDSHOW*)param;
    if ( pFidCb == XNULL || pFidShow == XNULL )
    {
        return XNULL;
    }
    /*
    pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
    */
    session = MMISC_GetFsmId(pFidShow->pCliEnv);
    level = (e_PRINTLEVEL)pFidCb->traceInfo.sessionLevel[session];
    
    XOS_CliExtPrintf( pFidShow->pCliEnv,
        "%-6d%-18s%-8s%-8s%-8s%-8s%-8s%-18s\r\n",
        pFidCb->fidNum,
        pFidCb->fidName,
        XOS_getPLName(level),
        attSwitch[pFidCb->traceInfo.isNeedFileName],
        attSwitch[pFidCb->traceInfo.isNeedLine],
        attSwitch[pFidCb->traceInfo.isNeedTime],
        attSwitch[pFidCb->traceInfo.isPrintInTraceTsk],
        XOS_getoutModeName((pFidCb->traceInfo.traceOutputMode))
        );

    pFidShow->index++;
    return param;
#endif

    return XNULLP;
}


/************************************************************************
* MOD_setFidTrace
* ����:���ù���ģ���trace ��Ϣ
* ����:
*  hHash   - ������
*  elem  - Ԫ��
*  param   - ����
* ���:
* ����: ָ��һ�����������´ε���XOS_HASHFuncʹ��
************************************************************************/
XVOID* MOD_setFidTrace (XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
    t_FIDCB *pFidCb = XNULLP;
    t_ALLFIDTRACE *pFidTraceInfo = XNULLP;

    pFidCb = (t_FIDCB*)elem;
    pFidTraceInfo = (t_ALLFIDTRACE*)param;
    if(pFidCb == XNULLP || pFidTraceInfo == XNULLP)
    {
        return param;
    }
    pFidTraceInfo->setTraceFunc(&(pFidCb->traceInfo), pFidTraceInfo->param,pFidTraceInfo->param2);
    return param;
}


/************************************************************************
������: MOD_allTaskEntry
����:  ����ģ���������ں���
����:pTid  �����
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: ����Ϣ����������������,ƽ̨�����е�����
      �����һ����װ, �ػ�ʱ��ʱ��Դ������Ϣ,
      ���Ҷ�������Ϣ��һ���ַ�.
************************************************************************/
XSTATIC XVOID MOD_allTaskEntry( XVOID * tidCb)
{
    t_TIDCB* pTidCb   = (t_TIDCB*)XNULLP;
    t_XOSCOMMHEAD *pRQMsg = (t_XOSCOMMHEAD*)XNULLP;
    XS32 ret  = 0;
    /*
    XS32 queue_nums = 0;    
    XS32 listIndex = 0;
    XCHAR fid_trace_msg[255] = {0};*/
    t_FIDCB *pFidCb   = (t_FIDCB*)XNULLP;
    
    pTidCb = (t_TIDCB*)tidCb;
#ifdef XOS_LINUX
    pTidCb->tskpid = (XU32)gettid();  /* �߳� pid */
#endif
    
    while( XTRUE )
    {
        /*���ȴ���Ϣ����������Ϣ*/
        pRQMsg = (t_XOSCOMMHEAD*)XNULLP;
        ret = QUE_MsgQRecv( &(pTidCb->recvMsgQue), &pRQMsg );
        if(  ret != XSUCC  || pRQMsg == XNULLP
            || pRQMsg->message != (XCHAR*)pRQMsg+sizeof(t_XOSCOMMHEAD) )
        {
            /*to do �������ش���*/
            XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_allTaskEntry()->recieve msg failed !");
            /* ��ʱ��������*/
            continue;
        }
        /*
        if(pMsg->datadest.FID == FID_NTL || pMsg->datasrc.FID == FID_NTL)
        {
        XOS_Trace(MD(FID_ROOT, PL_WARN), "QUE_MsgQRecv:NTL detail----------------begin");
        XOS_Trace(MD(FID_ROOT, PL_WARN), "pMsg->datadest.FID=%d",pMsg->datadest.FID);
        XOS_Trace(MD(FID_ROOT, PL_WARN), "pMsg->datasrc.FID=%d",pMsg->datasrc.FID);
        XOS_Trace(MD(FID_ROOT, PL_WARN), "pMsg->length=%d",pMsg->length);
        XOS_Trace(MD(FID_ROOT, PL_WARN), "pMsg->msgID=%d",pMsg->msgID);
        XOS_Trace(MD(FID_ROOT, PL_WARN), "QUE_MsgQRecv:NTL detail----------------end");
        }
        */
        /*��Ϣ�ķַ�, ƽ̨�ᴦ��ʱ����ʱ��Դ������Ϣ*/
        /*��ȡfid ����Ϣ*/
        pFidCb = (t_FIDCB*)XNULLP;
        pFidCb = MOD_getFidCb(pRQMsg->datadest.FID);
        if ( pFidCb == XNULLP )
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_allTaskEntry()->get fid cb failed !");
            /* ��ʱ��������,ֻ�ͷ���Ϣ*/
            XOS_MsgMemFree((XU32)FID_ROOT, pRQMsg);
            continue;
        }

        /*ʱ��Դ��������Ϣ*/
        if ( pRQMsg->datasrc.FID == FID_TIME
            &&(pRQMsg->msgID == eTimeHigClock || pRQMsg->msgID == eTimeLowClock))
        {
#ifndef XOS_NEED_OLDTIMER
            TIM_ClckProc((XVOID*)pRQMsg);
#else
            /*ʱ��Դ��Ϣ�Ĵ�����*/
            if( pRQMsg->msgID == eTimeHigClock )
            {
                TIM_ClckProc(MOD_getTimMntByFid(TIMER_PRE_HIGH,pRQMsg->datadest.FID));
            }
            else
            {
                TIM_ClckProc(MOD_getTimMntByFid(TIMER_PRE_LOW,pRQMsg->datadest.FID));
            }
#endif
            /*���еĶ�ʱ��ʱ��������Ϣ������ƽ̨�ͷ�*/
            XOS_MsgMemFree(pRQMsg->datasrc.FID, pRQMsg);
            continue;
        }

        /* �����NTLģ��ʱ��ӡ����������SYSLOG*/
        /*�������ʱΪ�˸�����Ҫ��ӡ����ʽ�����汾��ע�͵�*/
        /*
        if(pRQMsg->datadest.FID == FID_NTL) {
            
            queue_nums = XOS_listMaxUsage(pTidCb->recvMsgQue.queueList);
            
            if(fid_stat[pRQMsg->datasrc.FID] < 0) {
                fid_stat[pRQMsg->datasrc.FID] = 0;
            }
            fid_stat[pRQMsg->datasrc.FID]++;
            XOS_StrCpy(fid_name[pRQMsg->datasrc.FID], XOS_getFidName(pRQMsg->datasrc.FID));
            
            if((queue_nums - old_queue_nums) >= 200 ) {
                
                old_queue_nums = queue_nums;
                
                trace_msgq_size("Ntl module msgq maxUsage: %d\n", queue_nums);
                
                for(listIndex = 0; listIndex < 2048; listIndex++) {
                    if(fid_stat[listIndex] > 0) {
                        trace_msgq_size("Ntl module msgq detail: srcFid=%d, name=%s, used=%d\n",
                            listIndex,
                            fid_name[listIndex],
                            fid_stat[listIndex]);
                    }
                }
                
            }
        }*/
        
        /*�߾��ȶ�ʱ���ĳ�ʱ��Ϣ*/
        /* todo �߾��ȶ�ʱ����ʱû��ʵ��*/

#ifdef XOS_LINUX
        /* HA���̼߳�� ֱ�ӷ�����Ϣ BEGIN: Added by liujun, 2015/1/15 */
        if (FID_HA == pRQMsg->datasrc.FID && HA_WATCH_MSG_REQ == pRQMsg->msgID)
        {
            HA_XOSHelloProcess(pRQMsg->datadest.FID,pRQMsg->message,pRQMsg->length);
            if (eXOSMode == pFidCb->msgFreeType)
            {
                XOS_MsgMemFree(pRQMsg->datasrc.FID, pRQMsg);
            }
            continue;
        }
#endif
        /* END:   Added by liujun, 2015/1/15 */

        /*ģ������Ϣ*/
        if(pFidCb->MsgFunc != XNULLP)
        {
#ifdef XOS_ModMem_Check
            if(XSUCC != XOS_MemCheck(FID_ROOT,pRQMsg))
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR), "check mem error.srcFid(%d),dstFid(%d),msgid(%d)",
                                                pRQMsg->datasrc.FID,
                                                pRQMsg->datadest.FID,
                                                pRQMsg->msgID);
#ifdef XOS_DEBUG
                /*�����ã�������ڴ������ֱ�ӹ���*/
                XOS_SusPend();
#endif
                continue;
            }
#endif

#ifdef XOS_TRACE_AGENT
            XOS_TaTrace(pRQMsg->datadest.FID,e_TA_RECV,pRQMsg);
#endif
            pFidCb->MsgFunc((XVOID*)pRQMsg, &(pFidCb->fidNum));
        }
        /*�ͷ���Ϣ�ڴ�*/
        if(pFidCb->msgFreeType == eXOSMode)
        {
            XOS_MsgMemFree(pRQMsg->datadest.FID, pRQMsg);
        }
    }
}


/*-------------------------------------------------------------------------
               ģ��ӿں���
-------------------------------------------------------------------------*/
/************************************************************************
������: MOD_init
����:  ��ģ��ĳ�ʼ��
����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 MOD_init(void)
{
    /* û�г�ʼ��ʱ��ʼ��*/
    if(g_modMnt.isInit)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_init()->module managment is initlized !");
        return XSUCC;
    }

    /*����hash��*/
    g_modMnt.fidHash =
        XOS_HashConstruct(MAX_FID_NUMS+1, MAX_FID_NUMS, sizeof(XU32), sizeof(t_FIDCB), "fidHash");
    if(!XOS_HashHandleIsValid(g_modMnt.fidHash))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_init()->construct hash failed  !");
        return XERROR;
    }
    /*����hash ����*/
    XOS_HashSetHashFunc(g_modMnt.fidHash, MOD_hashFunc);

    /*����tid�Ĺ���ṹ*/
    g_modMnt.tidArray =
        XOS_ArrayConstruct(sizeof(t_TIDCB),  MAX_TID_NUMS, "tidArray");
    if(!XOS_ArrayHandleIsValid(g_modMnt.tidArray))
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "MOD_init()->construct tid array failed  !");
        XOS_HashDestruct(g_modMnt.fidHash);
        return XERROR;
    }

    /*��������ıȽϺ���*/
    XOS_ArraySetCompareFunc(g_modMnt.tidArray, MOD_isSameTid);

    g_modMnt.isInit = XTRUE;
    return XSUCC;
}


/*----------------------------------------------------------------------
             ������ģ���ע�����������
------------------------------------------------------------------------*/
#ifndef XOS_MDLMGT
/************************************************************************
������: MOD_readXosFids
����:  ��ȡƽ̨���ò���
����:
headFid   ��ʼ����fid
tailFid  ��������fid
ctrlFlag  ������, xfalse ��ƽ̨��,  xtrue ��ҵ���õ�
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: ����һ����ʱ�ĺ���, ����ģ������ʱ��Ӧ��
    �޸�,Ŀǰͨ����.c�ļ��е�����, ��ģ�����ʱ
    �ĳɶ�xml �ļ������þͿ�����.��ɵĹ���Ӧ��
    ��һ����.
    �˴���һ��bug :
    �����fidע�ᵽͬһ��tid ��ʱ��,tid����Ϣ�Ե�
    һ��fidע�ᵽ��tid ʱΪ��.
    ��Ϊ��ֻ��һ����ʱ����,�����ݲ�Ӱ��ʹ��,
    ֻҪע��ʱע��Ϳ�����.�����ݲ��޸�
************************************************************************/
XS32 MOD_readCfgFids(XU32 headFid, XU32 tailFid, XBOOL ctrlFlag)
{
    XS32 ret;
    XU32 i;
    XU32 fid;
    t_FIDCB fidCb;
    t_TIDCB tidCb;
    XS32 tidIndex;
    XVOID *pLocation;
    t_XOSLOGINLIST *pLog;

    /*ִ��.c �����е����ú���*/
    if(!ctrlFlag)
    {
        pLog = XOS_MemMalloc(FID_ROOT, FID_XOSMAX * sizeof(t_XOSLOGINLIST));
        
        if(XNULLP == pLog )
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), " XOS_cfgXosModule()-> malloc error!");
            return XERROR;
        }
        XOS_MemSet(pLog, XNULL, FID_XOSMAX * sizeof(t_XOSLOGINLIST));

        ret = XOS_cfgXosModule(pLog);
    }
    else
    {
        pLog = XOS_MemMalloc(FID_ROOT, (FID_MAX-FID_USERMIN) * sizeof(t_XOSLOGINLIST));
        if(XNULLP ==  pLog)
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR), " XOS_cfgXosModule()-> malloc error!");
            return XERROR;
        }
        XOS_MemSet(pLog, XNULL, (FID_MAX-FID_USERMIN ) * sizeof(t_XOSLOGINLIST));
        ret = XOS_InfoReg(pLog);
    }

    if( ret != XSUCC)
    {
        return XERROR;
    }

    /*�ȶ����ܿ���ص���Ϣ*/
    for(i = 0; i< tailFid-headFid; i++)
    {
        if(pLog[i].stack == XNULLP)
        {
            continue;
        }
        XOS_MemSet(&fidCb, 0, sizeof(t_FIDCB));

        /*��дfid ���ƿ�Ԫ��*/
        fid = pLog[i].stack->head.FID;
        fidCb.attSwitch = XFALSE;
        XOS_MemCpy(fidCb.fidName, pLog[i].stack->head.FIDName, MAX_FID_NAME_LEN);
        fidCb.fidNum = fid;
        fidCb.initFunc = pLog[i].stack->init.init;
        fidCb.noticeFunc = pLog[i].stack->init.notice;
        fidCb.MsgFunc = pLog[i].stack->handle.message;
        fidCb.timerFunc = pLog[i].stack->handle.timer;
        fidCb.pidIndex  = XOS_GetLocalPID();
        fidCb.msgFreeType = pLog[i].stack->memfreetype;
        fidCb.attSwitch = XFALSE;   /*��ʱ�������е�ģ������ʱ��
                                    ��Ϣת�ӿ���Ϊ�ر�״̬����
                                    �����ļ�����ʱ������ֶο���*/
        fidCb.noticeFlag = XFALSE;
        fidCb.logLevel = PL_ERR;

        initFidTraceInfo((&(fidCb.traceInfo))); /*��ʼ��trace ����Ϣ*/
#ifdef XOS_IPC_MGNT
        if(FID_IPCMGNT == fidCb.fidNum)
        {
            fidCb.traceInfo.traceLevel = PL_WARN;
        }
#endif

        /*ҵ���ģ�鲻����ע�ᵽƽ̨��������*/
        if(ctrlFlag && pLog[i].TID <= FID_USERMIN)
        {
            XOS_Trace(MD(FID_ROOT, PL_WARN),
                "fid[%d] try to reg in plat task [%d] failed!", fid, pLog[i].TID);
            continue;
        }

        /*tid index ��Ҫ���Һ�ȷ��*/
        tidIndex = XOS_ArrayFind(g_modMnt.tidArray, (XVOID*)pLog[i].TID);
        if(tidIndex >= 0 || pLog[i].TID == 0) /*�ҵ�����Ҫ�����fid*/
        {
            fidCb.tidIndex = tidIndex;
        }
        else /*û���ҵ�*/
        {
            XOS_MemSet(&tidCb, 0, sizeof(t_TIDCB));
            tidCb.prio = pLog[i].prio;
            tidCb.stackSize = pLog[i].stacksize;
            tidCb.tidNum = pLog[i].TID;
            XOS_MemCpy(&(tidCb.tskName), pLog[i].taskname, MAX_TID_NAME_LEN);
            /* tidCb.timerNum = pLog[i].timenum; */
            /* tidCb.maxMsgsInQue = MAX_MSGS_IN_QUE;   */   /*��ʱд��*/

            if ( MIN_MSGS_IN_QUE  > pLog[i].quenum  ||  MAX_MSGSNUM_IN_QUE <= pLog[i].quenum)
            {
                tidCb.maxMsgsInQue = MAX_MSGS_IN_QUE;
            }
            else
            {
                tidCb.maxMsgsInQue = pLog[i].quenum;
            }

            /*����tid �Ŀ��ƿ�*/
            tidIndex = XOS_ArrayAdd(g_modMnt.tidArray, (XOS_ArrayElement)&tidCb);
            if(tidIndex < 0)
            {
                return XERROR;
            }
            fidCb.tidIndex = tidIndex;
        }

        /*����һ�����ܿ��ƿ�*/
        pLocation = XNULLP;
        pLocation = XOS_HashElemAdd(g_modMnt.fidHash, (XVOID*)&fid, (XVOID*)&fidCb, XFALSE);
        if(pLocation == XNULLP)
        {
            return XERROR;
        }
    }

    XOS_MemFree(FID_ROOT, pLog);
    return XSUCC;
}


/************************************************************************
������: MOD_startXosFids
����:  ����ƽ̨�����ģ��
����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��:  �˺�������ģ�����ʱӦ���޸�.
       ��Ҫ�޸ĵ�, ģ�������˳��ɿ�.
************************************************************************/
XS32 MOD_startXosFids(void)
{
    XS32 ret, retval;
    XS32 i;
    XS32 tskIndex;
    t_TIDCB *pTidCb;
    t_FIDCB *pFidCb;

    /*���ȶ�ȡƽ̨��ģ������*/
    ret = MOD_readCfgFids((XU32)FID_XOSMIN, (XU32)FID_XOSMAX, XFALSE);
    if(ret != XSUCC)
    {
        return XERROR;
    }

    /*������Ϣ����*/
    for(i=FID_XOSMIN; i<FID_XOSMAX; i++)
    {
        pFidCb = (t_FIDCB*)XNULLP;
        pFidCb = MOD_getFidCb(i);
        if(pFidCb == XNULLP)
        {
            continue;
        }

        tskIndex = XOS_ArrayFind(g_modMnt.tidArray, (XVOID*)i);
        if(tskIndex >= 0)
        {
            pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, tskIndex);
            /*xos ��,��ʱ������������ں����Ƚ�����*/
            if (pTidCb->tidNum== FID_TIME)
            {
                /*��ʱ��������ִ�г�ʼ������*/
                pFidCb->initFunc(XNULLP, XNULLP);
#ifndef XOS_NEED_OLDTIMER                        
                /*������ʱ��������������*/
                ret = XOS_TaskCreate((pTidCb->tskName), pTidCb->prio, pTidCb->stackSize,
                    (os_taskfunc)tm_excuteLowProc, XNULLP, &(pTidCb->tskId));
                if(XSUCC != ret)
                {
                    MMErr("FID_TIME:XOS_MMStartFid()-> can not create task[tsk_dealtimer]!");
                    return XERROR;
                }
#else
                /*�;��ȶ�ʱ��*/
                ret = XOS_TaskCreate((pTidCb->tskName), (e_TSKPRIO)pTidCb->prio, pTidCb->stackSize,
                    (os_taskfunc)Low_TimerTask, XNULLP, &(pTidCb->tskId));
                if(XSUCC != ret)
                {
                    XOS_Trace(MD(FID_TIME,PL_ERR),"MOD_startXosFids()-> TIME is wrong!");
                    return XERROR;
                }
#ifdef XOS_HIGHTIMER
                /*�߾��ȶ�ʱ������*/
                ret = XOS_TaskCreate("tsk_HTIM", (e_TSKPRIO)pTidCb->prio, pTidCb->stackSize,
                    (os_taskfunc)High_TimerTask, XNULLP, &(pTidCb->tskId));
                if(XSUCC != ret)
                {
                    XOS_Trace(MD(FID_TIME,PL_ERR),"MOD_startXosFids()-> HIGHTIME is wrong!");
                    return XERROR;
                }
#endif
#endif
                continue;
            }
            else
            {
                pTidCb->recvMsgQue.tskIndex = pTidCb->tidNum;
                retval = QUE_MsgQCreate(&(pTidCb->recvMsgQue), pTidCb->maxMsgsInQue);
                if(retval != XSUCC)
                {
                    XOS_Trace(MD(FID_ROOT, PL_ERR),
                        "MOD_startXosFids()->create msg que failed ! tid: %d, tskName: %s",
                        pTidCb->tidNum, pTidCb->tskName);
                    return XERROR;
                }
                ret = XOS_TaskCreate((pTidCb->tskName), (e_TSKPRIO)pTidCb->prio, pTidCb->stackSize,
                    (os_taskfunc)MOD_allTaskEntry, (XVOID*)pTidCb, &(pTidCb->tskId));

                /* ע��������ػص� BEGIN: Added by liujun, 2015/1/15 */
                HA_AddToDeadWatch(pFidCb->fidNum,pFidCb->tidIndex);
                
            }
            /*ȷ�����������ɹ�*/
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "MOD_startXosFids()->create task failed ! tid: %d, tskName: %s",
                    pTidCb->tidNum, pTidCb->tskName);
                return XERROR;
            }
        }

        /*ִ�и�ģ��ĳ�ʼ������*/
        if(pFidCb->initFunc == XNULLP)
        {
            continue;
        }
        ret = pFidCb->initFunc(XNULLP, XNULLP);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_ROOT, PL_WARN),
                "MOD_startXosFids()->excute fid[%s] init func failed!",
                pFidCb->fidName);
            return XERROR;
        }
    }
    return XSUCC;
}


/************************************************************************
������: MOD_startUserFids
����:  ����ҵ������ģ��
����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 MOD_startUserFids(void)
{
    XS32 ret;
    XS32 i;
    XS32 tskIndex;
    t_TIDCB *pTidCb;
    t_FIDCB *pFidCb;

    /*��ȡӦ��ģ�����Ϣ*/
    ret = MOD_readCfgFids((XU32)FID_USERMIN, (XU32)FID_MAX, XTRUE);
    if(ret != XSUCC)
    {
        return XERROR;
    }

    /*������Ϣ����*/
    for(i=FID_USERMIN; i<FID_MAX; i++)
    {
        /*������Ϣ���в�������*/
        tskIndex = XOS_ArrayFind(g_modMnt.tidArray, (XVOID*)i);
        if(tskIndex >= 0)
        {
            pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, tskIndex);
            pTidCb->recvMsgQue.tskIndex = pTidCb->tidNum;
            ret = QUE_MsgQCreate(&(pTidCb->recvMsgQue), pTidCb->maxMsgsInQue);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "MOD_startXosFids()->create msg que failed ! tid: %d, tskName: %s",
                    pTidCb->tidNum, pTidCb->tskName);
                return XERROR;
            }

            /*��������*/
            ret = XOS_TaskCreate(pTidCb->tskName, (e_TSKPRIO)pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)MOD_allTaskEntry, (XVOID*)pTidCb, &(pTidCb->tskId));
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "MOD_startXosFids()->create task failed ! tid: %d, tskName: %s",
                    pTidCb->tidNum, pTidCb->tskName);
                return XERROR;
            }
        }

        /*ִ�г�ʼ������*/
        pFidCb = (t_FIDCB*)XNULLP;
        pFidCb = MOD_getFidCb(i);
        if(pFidCb == XNULLP ||pFidCb->initFunc == XNULLP)
        {
            continue;
        }
        ret = pFidCb->initFunc(XNULLP, XNULLP);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_ROOT, PL_WARN),
                "MOD_startXosFids()->excute fid[%s] init func failed!",
                pFidCb->fidName);
            return XERROR;
        }
    }

    return XSUCC;
}
#endif


/************************************************************************
������: MOD_startNotice
����:  ����֪ͨ����
����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: Ϊ�˼���1.81�棬�ṩ��ʱ����
************************************************************************/
XS32 MOD_StartNotice(XVOID)
{
    XS32 ret;
    XS32 i;
    t_FIDCB *pFidCb;

    for(i = FID_XOSMIN; i<FID_MAX; i++)
    {
        pFidCb = (t_FIDCB*)XNULLP;
        pFidCb = MOD_getFidCb(i);
        if(XNULL == pFidCb)
        {
            continue;
        }

        /*������־��ӡ����,����ģ�����noticeģ�����޸�Ĭ����־����*/
        pFidCb->logLevel = PL_ERR;

        if(XNULL == pFidCb->noticeFunc)
        {
            continue;
        }

        if(pFidCb->noticeFlag == XFALSE)
        {
            ret = pFidCb->noticeFunc(XNULLP, XNULLP);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "MOD_StartNotice()->excute fid[%s] init func failed!",
                    pFidCb->fidName);
                return XERROR;
            }
            pFidCb->noticeFlag = XTRUE;
        }
    }
#ifdef MEM_FID_DEBUG
    bModifyFidFlg = XTRUE;/*�ȴ�����ģ���ʼ����ɣ���fid��������*/
#endif
    
    return XSUCC;
}


/*----------------------------------------------------------------------
                  �����ǻ�ȡ������ģ����ص���Ϣ����
------------------------------------------------------------------------*/
/************************************************************************
������: XOS_isValidFid
���ܣ�  �ж�fid�Ƿ���Ч
���룺  fid   ����ģ���
�����  N/A
���أ�  XFALSE OR XTRUE
˵����
************************************************************************/
XPUBLIC XBOOL XOS_isValidFid(XU32 fid)
{
    if(FID_XOSMIN < fid && fid < FID_MAX)
    {
        return XTRUE;
    }
    else
    {
        return XFALSE;
    }
}


/************************************************************************
������: XOS_getFidName
���ܣ�  ��ȡ���ܿ���Ϣת�ӿ��ص�ֵ
���룺  fid   ����ģ���
�����  N/A
���أ�  ���ֻ��ָ��
˵����
************************************************************************/
XPUBLIC XCHAR* XOS_getFidName(XU32 fid)
{
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    pFidCb = MOD_getFidCb( fid);
    if(pFidCb != XNULLP)
    {
        return pFidCb->fidName;
    }
    else
    {
        return (XCHAR*)XNULLP;
    }
}


/************************************************************************
������: MOD_getFidAttSwitch
���ܣ�  ��ȡ���ܿ���Ϣת�ӿ��ص�ֵ
���룺  fid   ����ģ���
�����  N/A
���أ�  XFALSE OR XTRUE
˵����
************************************************************************/
XPUBLIC XBOOL MOD_getFidAttSwitch(XU32 fid)
{
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    pFidCb = MOD_getFidCb( fid);
    if(pFidCb != XNULLP)
    {
        return pFidCb->attSwitch;
    }
    else
    {
        /*����Ƚ�����*/
        XOS_CpsTrace(MD(FID_ROOT, PL_INFO), "MOD_getFidAttSwitch()-> fid [%d] is not in fid hash,", fid);
        return XFALSE;
    }
}


/************************************************************************
������: MOD_setFidAttSwitch
���ܣ�  set���ܿ���Ϣת�ӿ��ص�ֵ
���룺  fid   ����ģ���
valve ����״̬����XTRUE , �ر�XFALSE
�����  N/A
���أ�  XFALSE OR XTRUE
˵����
************************************************************************/
XPUBLIC XS16 MOD_setFidAttSwitch(XU32 fid, XBOOL valve)
{
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    pFidCb = MOD_getFidCb( fid);
    if(pFidCb != XNULLP)
    {
        pFidCb->attSwitch = valve;
        return XSUCC;
    }
    else
    {
        /*����Ƚ�����*/
        XOS_Trace(MD(FID_ROOT, PL_INFO), "MOD_setFidAttSwitch()-> fid [%d] is not in fid hash,", fid);
        return XERROR;
    }
}


/************************************************************************
������: MOD_getTimMntByFid
���ܣ�  ͨ�����ܿ�Ŷ�ʱ������ṹָ��
���룺  fid   ����ģ���
�����  N/A
���أ�  ������ƿ�ָ��,ʧ�ܷ��ؿ�
˵����
************************************************************************/
#ifdef XOS_NEED_OLDTIMER
XPUBLIC XVOID * MOD_getTimMntByFid(e_TIMERPRE pre,XU32 fid)
{
    t_FIDCB *pFidCb;

    pFidCb = MOD_getFidCb(fid);
    if(pFidCb == XNULLP)
    {
        return XNULLP;
    }
    if(pre ==  TIMER_PRE_LOW)
        return &(pFidCb->timerMngerLow);
    else if(pre ==  TIMER_PRE_HIGH)
        return &(pFidCb->timerMngerHig);
    else
        return XNULLP;
}
#endif

/************************************************************************
������: MOD_getTimProcFunc
���ܣ�  ͨ�����ܿ�Ż�ȡ���ܿ�ĳ�ʱ������
���룺  fid   ����ģ���
�����  N/A
���أ�  ������ƿ�ָ��,ʧ�ܷ��ؿ�
˵����
************************************************************************/
XPUBLIC  modTimerProcFunc MOD_getTimProcFunc(XU32 fid)
{
    t_FIDCB *pFidCb;
    pFidCb = MOD_getFidCb( fid);
    if(pFidCb == XNULLP)
    {
        return XNULLP;
    }
    return (modTimerProcFunc)pFidCb->timerFunc;
}


/*----------------------------------------------------------------------
                   ������ģ���ͨ�ŵĲ���
------------------------------------------------------------------------*/
/************************************************************************
������: XOS_GetLocalPID( XVOID )
����:   ��ȡ��ǰƽ̨�� PID
����:   ��
���:   ��ǰƽ̨�� PID
����:
˵��:
************************************************************************/
XPUBLIC XU32 XOS_GetLocalPID( XVOID )
{
    return g_XOS_LocalPid;
}


/************************************************************************
������: XOS_SetLocalPid(  )
����:   ���õ�ǰƽ̨�� PID
����:   ��
���:
����:
˵��:
************************************************************************/
XVOID XOS_SetLocalPID(XU32 localpid)
{
    g_XOS_LocalPid = localpid;
}


/************************************************************************
������: XOS_MsgMemAlloc
���ܣ�  �����贫����Ϣ����һ���ڴ��
���룺
fid   - ���ܿ�id
nbytes  - ��Ϣ�ĳ���
�����  N/A
���أ�  t_XOSCOMMHEAD * �� �������Ϣ�ڴ�ָ��
˵����  �����ڵ���Ϣ������ʱ��
************************************************************************/
t_XOSCOMMHEAD *XOS_MsgMemMalloc(XU32 fid, XU32 nbytes)
{
    t_XOSCOMMHEAD *pMsg ;
    XVOID *pTemp = XNULLP;

    pTemp = XOS_MemMalloc( fid, nbytes+sizeof(t_XOSCOMMHEAD));
    if(pTemp == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgMemMalloc()->mem malloc failed !");
        return (t_XOSCOMMHEAD*)XNULLP;
    }
    pMsg = (t_XOSCOMMHEAD*)pTemp;
    pMsg->length = nbytes;   /*��Ϣ�ĳ���ƽ̨�Զ���ȫ*/
    pMsg->message = (XVOID*)(((XCHAR*)pMsg)+sizeof(t_XOSCOMMHEAD));
    pMsg->traceID = 0;
    pMsg->logID = 0;

    /* add by wulei 2006.12.08 */
#ifdef XOS_MSGMEM_TRACE
    /* ������Ϣ�ڴ��ز��� */
    XOS_RecordMsgBuf(fid, pMsg);
#endif /* XOS_MSGMEM_TRACE */

    return pMsg;
}


/************************************************************************
������: XOS_MsgMemFree
���ܣ�  �ͷ�һ����Ϣ�ڴ��
���룺
fid   - ���ܿ�id
t_XOSCOMMHEAD * - ��Ϣ�ڴ��ָ��
�����  N/A
���أ�  t_XOSCOMMHEAD * �� �������Ϣ�ڴ�ָ��
˵����
************************************************************************/
XVOID XOS_MsgMemFree(XU32 fid, t_XOSCOMMHEAD *ptr)
{
#ifdef XOS_MSGMEM_TRACE
    /* ������Ϣ�ڴ��ز��� */
    XOS_ReleaseMsgBuf(fid, ptr);
#endif /* XOS_MSGMEM_TRACE */

    XOS_MemFree(fid, (XVOID*)ptr );
    return;
}

XS32 xos_msgHead2Host(t_XOSCOMMHEAD *pHead)
{
    pHead->datadest.PID = ntohl(pHead->datadest.PID);
    pHead->datadest.FID = ntohl(pHead->datadest.FID);
    pHead->datadest.FsmId = ntohl(pHead->datadest.FsmId);
    
    pHead->datasrc.PID = ntohl(pHead->datasrc.PID);
    pHead->datasrc.FID = ntohl(pHead->datasrc.FID);
    pHead->datasrc.FsmId = ntohl(pHead->datasrc.FsmId);

    pHead->msgID = ntohs(pHead->msgID);
    pHead->subID = ntohs(pHead->subID);
    pHead->prio  = ntohl(pHead->prio);
    pHead->length = ntohl(pHead->length);

    return 0;
}

XS32 xos_msgHead2Net(t_XOSCOMMHEAD *pHead)
{
//    t_XOSCOMMHEAD *pHead;

    
    pHead->datadest.PID = htonl(pHead->datadest.PID);
    pHead->datadest.FID = htonl(pHead->datadest.FID);
    pHead->datadest.FsmId = htonl(pHead->datadest.FsmId);
    
    pHead->datasrc.PID = htonl(pHead->datasrc.PID);
    pHead->datasrc.FID = htonl(pHead->datasrc.FID);
    pHead->datasrc.FsmId = htonl(pHead->datasrc.FsmId);

    pHead->msgID = htons(pHead->msgID);
    pHead->subID = htons(pHead->subID);
    pHead->prio  = htonl(pHead->prio);
    pHead->length = htonl(pHead->length);

    return 0;
}



/************************************************************************
������: XOS_MsgSend
���ܣ�  ģ���ͨ�ŵ���Ϣ���ͺ���
���룺  pMsg: ��Ϣ����ͷ��Ϣ
�����  N/A
���أ�  XSUCC OR XERROR
˵����  �����û���д����Ϣͷ������Ϣ���͵�Ŀ�ĵ�
************************************************************************/
extern XU32   g_ulMailBoxKey;
XPUBLIC XS32 XOS_MsgSend(t_XOSCOMMHEAD *pMsg)
{
    t_TIDCB *pTidCb;

    /*������ڰ�ȫ�Լ��*/
    if(pMsg == XNULLP || !XOS_isValidFid(pMsg->datadest.FID)
        ||!XOS_isValidFid(pMsg->datasrc.FID)
        ||pMsg->message != (XCHAR*)pMsg+sizeof(t_XOSCOMMHEAD))
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgSend()-> invalid Msg!pMsg: 0x%x,", pMsg);
        return XERROR;
    }

    /*��ȡ��������tid ����Ϣ*/

    pTidCb = (t_TIDCB*)XNULLP;
    pTidCb = MOD_getTidCbByFid(pMsg->datadest.FID);
    if(pTidCb == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR),
            "XOS_MsgSend()->srcFid %d,get Tid cb by dst fid[%d] failed ", pMsg->datasrc.FID,pMsg->datadest.FID);
        return XERROR;
    }

    /*������Ϣ�������ߵ���Ϣ����*/
#ifdef XOS_MSGMEM_TRACE
    XOS_UpdateMsgBuf(pMsg);
#endif /* XOS_MSGMEM_TRACE */

    /*������Ϣ�������ߵ���Ϣ����*/    
    return QUE_MsgQSend(&(pTidCb->recvMsgQue), (XVOID*)pMsg, pMsg->prio);
}





XS32 XOS_MsgDistribution(XU32 ipcIdx, XU32 remoteIdx, XU8 *p, XS32 len)
{
    t_TIDCB *pTidCb;
    t_XOSCOMMHEAD* pMsg      = XNULLP;


    if(len < sizeof(t_XOSCOMMHEAD))
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgDistribution err len < sizeof(t_XOSCOMMHEAD)");
        return XERROR;
    }

    pMsg = XOS_MsgMemMalloc( FID_IPC, len-sizeof(t_XOSCOMMHEAD) );
    if ( XNULLP == pMsg )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgDistribution err in XOS_MsgMemMalloc");
        return XERROR;
    }
    memcpy(pMsg, p, len);
    xos_msgHead2Host(pMsg);
    pMsg->message = (XVOID*)(((XCHAR*)pMsg)+sizeof(t_XOSCOMMHEAD));

    /*������ڰ�ȫ�Լ��*/
    if(pMsg == XNULLP || !XOS_isValidFid(pMsg->datadest.FID)
        ||!XOS_isValidFid(pMsg->datasrc.FID)
        ||pMsg->message != (XCHAR*)pMsg+sizeof(t_XOSCOMMHEAD))
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MsgSend()-> invalid Msg!pMsg: 0x%x,", pMsg);
        return XERROR;
    }
    
    /*��ȡ��������tid ����Ϣ*/
    pTidCb = (t_TIDCB*)XNULLP;
    pTidCb = MOD_getTidCbByFid(pMsg->datadest.FID);
    if(pTidCb == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR),
            "XOS_MsgSend()->srcFid %d,get Tid cb by dst fid[%d] failed ", pMsg->datasrc.FID,pMsg->datadest.FID);
        return XERROR;
    }

    /*������Ϣ�������ߵ���Ϣ����*/
#ifdef XOS_MSGMEM_TRACE
    XOS_UpdateMsgBuf(pMsg);
#endif /* XOS_MSGMEM_TRACE */

    /*������Ϣ�������ߵ���Ϣ����*/
    return QUE_MsgQSend(&(pTidCb->recvMsgQue), (XVOID*)pMsg, pMsg->prio);

    return XSUCC;
}



/************************************************************************
������: XOS_MsgBroadcast
���ܣ�  ��Ϣ�㲥,��ʱ�ṩ���;��ȶ�ʱ����
���룺
pMsg: ��Ϣ����ͷ��Ϣ
brdcstFunc �㲥����,���û���д,�������ƹ㲥��fid
�����  N/A
���أ�  XSUCC OR XERROR
˵����  �����û���д����Ϣͷ������Ϣ���͵�Ŀ�ĵ�
************************************************************************/
XPUBLIC XS32 XOS_MsgBroadcast(t_XOSCOMMHEAD *pMsg, brdcstFilterFunc brdcstFunc)
{
    XS32 ret;
    XS32 tidIndex;
    t_TIDCB *pTidCb;
    t_XOSCOMMHEAD *pMsgTemp;

    if(pMsg == XNULLP || brdcstFunc == XNULLP)
    {
        return XERROR;
    }

    tidIndex = XOS_ArrayGetFirstPos(g_modMnt.tidArray);
    while (tidIndex >= 0)
    {
        pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(brdcstFunc (pTidCb->tidNum))
        {
            /*����һ����Ϣ*/
            pMsgTemp = (t_XOSCOMMHEAD*)XNULLP;
            pMsgTemp = XOS_MsgMemMalloc(pMsg->datasrc.FID, pMsg->length);
            if(pMsgTemp == XNULLP)
            {
                continue;
            }
            pMsgTemp->datasrc = pMsg->datasrc;
            pMsgTemp->msgID = pMsg->msgID;
            pMsgTemp->prio = pMsg->prio;
            pMsgTemp->subID = pMsg->subID;
            pMsgTemp->datadest.PID = pMsg->datadest.PID;
            pMsgTemp->datadest.FsmId = pMsg->datadest.FsmId;
            pMsgTemp->datadest.FID = pTidCb->tidNum;
            if(pMsgTemp->length > 0)
            {
                XOS_MemCpy(pMsgTemp->message, pMsg->message, pMsg->length);
            }
            ret = XOS_MsgSend(pMsgTemp);
            if(ret != XSUCC)
            {
                XOS_MsgMemFree(pMsgTemp->datasrc.FID, pMsgTemp);
            }
        }

        tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
    }
    /*�ͷ���Ϣ*/
    XOS_MsgMemFree((XU32)FID_ROOT, pMsg);
    return XSUCC;
}


/*----------------------------------------------------------------------
                     ��ģ����صĵ��������
------------------------------------------------------------------------*/
/************************************************************************
������: MOD_getFidTraceInfo
���ܣ�  fid ��ص�trce��Ϣ
���룺  fid   ����ģ���
�����  N/A
���أ�  �ɹ�����fid ��ص�trace��Ϣ��ʧ�ܷ���XNULLP
˵����
************************************************************************/
t_FIDTRACEINFO* MOD_getFidTraceInfo(XU32 fid)
{
    t_FIDCB* pFidCb = (t_FIDCB*)XNULLP;

    pFidCb = MOD_getFidCb( fid);
    if(pFidCb != XNULLP)
    {
        return &(pFidCb->traceInfo);
    }
    else
    {
        return (t_FIDTRACEINFO*)XNULLP;
    }
}


/************************************************************************
������: MOD_setAllTraceInfo
���ܣ�  �������е�fid ��ص�trace��Ϣ
���룺
setFunc   ���ú���
param  ���ú������������
�����  N/A
���أ�  �ɹ�����XSUCC,  ʧ�ܷ���XERROR
˵����
************************************************************************/
XS32 MOD_setAllTraceInfo(setFidTraceInfo setFunc, XVOID* param,XVOID* param2)
{
    t_ALLFIDTRACE allFidTraceInfo;

    if(setFunc == XNULLP)
    {
        return XERROR;
    }
    XOS_MemSet(&allFidTraceInfo, 0, sizeof(t_ALLFIDTRACE));
    allFidTraceInfo.setTraceFunc = setFunc;
    allFidTraceInfo.param = param;
    allFidTraceInfo.param2 = param2;

    return XOS_HashWalk(g_modMnt.fidHash, MOD_setFidTrace, (XVOID*)&allFidTraceInfo);
}


/************************************************************************
������: MOD_fidInfoShow
���ܣ���ʾ���е�fid ��Ϣ
���룺
�����  N/A
���أ�  XSUCC OR XERROR
˵����fidshow �������������
************************************************************************/
XVOID MOD_fidInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    t_FIDSHOW fidShow;

    /*fid list show  */
    XOS_CliExtPrintf(pCliEnv,
        "fid list \r\n---------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-6s%-18s%-18s%-10s%-10s%-10s\r\n",
        "index",
        "fid",
        "fidName",
        "tidName",
        "attSwitch",
        "traceLev",
        "frMsgType"
        );
    XOS_MemSet(&fidShow, 0, sizeof(t_FIDSHOW));
    fidShow.pCliEnv = pCliEnv;
    fidShow.index = 0;
    /*����hash*/
    XOS_HashWalk(g_modMnt.fidHash, MOD_showFid, &fidShow);

    /*end of fid list */
    XOS_CliExtPrintf(pCliEnv,
        "---------------------------------------------------------------------------\r\n");

    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n" , fidShow.index);
    return;
}

/************************************************************************
������: MOD_GetTskPid
���ܣ���ȡ��tid��pid
���룺
�����  N/A
���أ��õ�����0��û�ҵ�����-1
˵����
************************************************************************/
XS32 MOD_GetTskPid(XU32 *pid,XU32 fid)
{    
    t_TIDCB *pTidCb = NULL;
    t_FIDCB *pFidCb = NULL;

    *pid = 0;

    pFidCb = MOD_getFidCb(fid);
    if(XNULL != pFidCb)
    {
        pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
        if(pTidCb != (t_TIDCB*)XNULLP)
        {
            if (0 != pTidCb->tskpid)
            {
                *pid = pTidCb->tskpid;
                return XSUCC;
            }
        }
    }

    return XERROR;
}

/************************************************************************
������: MOD_tidInfoShow
���ܣ���ʾ���е�tid ��Ϣ
���룺
�����  N/A
���أ�
˵����tidshow �������������
************************************************************************/
XVOID MOD_tidInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    t_TIDCB* pTidCb;
    XS32 tidIndex;
    XS32 j;

    /*tid list show  */
    XOS_CliExtPrintf(pCliEnv,
        "tid list \r\n-------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-6s%-18s%-8s%-12s%-6s%-10s%-6s\r\n",
        "index",
        "fid",
        "tidName",
        "pid",
        "tid",
        "prio",
        "stkSize",
        "msgs"
        );

    tidIndex = XOS_ArrayGetFirstPos(g_modMnt.tidArray);
    j = 0;
    while(tidIndex >= 0)
    {
        pTidCb = (t_TIDCB*)XNULLP;
        pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(pTidCb == XNULLP)
        {
            continue;
        }
        XOS_CliExtPrintf(pCliEnv,
            "%-6d%-6d%-18s%-8u0x%-10x%-6d%-10d%-6d\r\n",
            j,
            pTidCb->tidNum,
            pTidCb->tskName,
            pTidCb->tskpid,
            pTidCb->tskId,
            pTidCb->prio,
            pTidCb->stackSize,
            pTidCb->maxMsgsInQue
            );
        tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
        j++;
    }
    /*end of tid list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------\r\n");
    return ;
}


/************************************************************************
������: MOD_msgqInfoShow
���ܣ���ʾ���е�tid ��Ϣ
���룺
�����  N/A
���أ�
˵����msgqshow �������������
************************************************************************/
XVOID MOD_msgqInfoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    t_TIDCB* pTidCb;
    t_XOSMSGQ *pMsgQ;

    XS32 tidIndex;
    XS32 maxMsgs;
    XS32 maxUsage;
    XS32 curUsage;
    XS32 j;

    /*msgq list show  */
    XOS_CliExtPrintf(pCliEnv,
        "queue list \r\n-------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-18s%-10s%-10s%-10s\r\n",
        "index",
        "tidName",
        "maxMsgs",
        "maxUsage",
        "curUsage"
        );

    tidIndex = XOS_ArrayGetFirstPos(g_modMnt.tidArray);
    j = 0;
    while(tidIndex >= 0)
    {
        pTidCb = (t_TIDCB*)XNULLP;
        pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(pTidCb == XNULLP ||pTidCb->recvMsgQue.queueList == XNULLP)
        {
            tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
            continue;
        }
        pMsgQ = &(pTidCb->recvMsgQue);
        maxMsgs = XOS_listMaxSize(pMsgQ->queueList);
        maxUsage = XOS_listMaxUsage(pMsgQ->queueList);
        curUsage = XOS_listCurSize(pMsgQ->queueList);

        XOS_CliExtPrintf(pCliEnv,
            "%-6d%-18s%-10d%-10d%-10d\r\n",
            j,
            pTidCb->tskName,
            (maxMsgs == -1)? 0:pTidCb->maxMsgsInQue,
            (maxUsage == -1)? 0:maxUsage-(XS32)eMAXPrio-1,
            (curUsage == -1)? 0:curUsage-(XS32)eMAXPrio-1
            );
        tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
        j++;
    }
    /*end of msgq list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------\r\n");
    return ;
}


/**********************************
��������  : fid_msgq
����    : liu.da
�������  : 2007��12��01��
��������  : �����̶߳��еĴ��ں���
����    : void
����ֵ    : XVOID
************************************/
XPUBLIC XS32 msgqshow()
{
    t_TIDCB* pTidCb;
    t_XOSMSGQ *pMsgQ;

    XS32 tidIndex;
    XS32 maxMsgs;
    XS32 maxUsage;
    XS32 curUsage;
    XS32 j;
    /*msgq list show  */
    printf(
        "queue list \r\n-------------------------------------------------------\r\n");
    printf(
        "%-6s%-18s%-10s%-10s%-10s\r\n",
        "index",
        "tidName",
        "maxMsgs",
        "maxUsage",
        "curUsage"
        );

    tidIndex = XOS_ArrayGetFirstPos(g_modMnt.tidArray);
    j = 0;
    while(tidIndex >= 0)
    {
        pTidCb = (t_TIDCB*)XNULLP;
        pTidCb = (t_TIDCB*)XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(pTidCb == XNULLP ||pTidCb->recvMsgQue.queueList == XNULLP)
        {
            tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
            continue;
        }
        pMsgQ = &(pTidCb->recvMsgQue);
        maxMsgs = XOS_listMaxSize(pMsgQ->queueList);
        maxUsage = XOS_listMaxUsage(pMsgQ->queueList);
        curUsage = XOS_listCurSize(pMsgQ->queueList);

        printf(
            "%-6d%-18s%-10d%-10d%-10d\r\n",
            j,
            pTidCb->tskName,
            (maxMsgs == -1)? 0:pTidCb->maxMsgsInQue,
            (maxUsage == -1)? 0:maxUsage-(XS32)eMAXPrio-1,
            (curUsage == -1)? 0:curUsage-(XS32)eMAXPrio-1
            );
        tidIndex = XOS_ArrayGetNextPos(g_modMnt.tidArray, tidIndex);
        j++;
    }
    /*end of msgq list */
    return XSUCC;
}


XPUBLIC XS32 msgq_fidshow(int i_FID,int i_ShowCount)
{
    /*��ȡ��������tid ����Ϣ*/
    t_XOSMSGQ fid_msgq;
    t_TIDCB *pTidCb = (t_TIDCB*)XNULLP;
    XS32 listIndex;
    XU32 inum = 0;
    XU32 msg_count=i_ShowCount;
    t_QUEELEM*pQueElem;
    XCHAR  fid_name[32]={0};
    t_XOSCOMMHEAD *ptMsg;
    XCHAR* fidName = XNULL;
    
    if(i_FID <= 0)
    {
        printf("msgq_fidshow,usage:msgq_fidshow 4.\r\n");
        return XERROR;
    }
    if(msg_count < 10 )
    {
        msg_count =10;
    }

    /*�õ�ģ������*/
    fidName = XOS_getFidName(i_FID);
    if ( XNULLP !=  fidName)
    {
        XOS_Sprintf(fid_name,32,"%s", fidName);
    }else
    {
        XOS_Sprintf(fid_name,32,"Unknown_FID[%d]",i_FID);
    }

    pTidCb = MOD_getTidCbByFid(i_FID);
    if(pTidCb == XNULLP)
    {
        printf("msgq_fidshow,get Tid cb by fid[%d] failed.\r\n",i_FID);
        return XERROR;
    }

    if(pTidCb->recvMsgQue.queueList == XNULLP)
    {
        printf("msgq_fidshow,fid[%s] msgq current is null.\r\n",fid_name);
        return XERROR;
    }

    /*msgq list show  */
    fid_msgq=pTidCb->recvMsgQue;
    printf("fid %d queue list current has %d msgs.\r\n",
        fid_msgq.tskIndex,(XOS_listCurSize(fid_msgq.queueList)-eMAXPrio-1));

    printf("%-10s%-18s%-18s%-10s%-10s\r\n","No","srcfid","destfid","msgID","prio");
    listIndex = XOS_listHead(fid_msgq.queueList);
    while(listIndex != XERROR)
    {
        pQueElem = (t_QUEELEM*)XNULLP;
        pQueElem = (t_QUEELEM*)XOS_listGetElem(fid_msgq.queueList, listIndex);
        if ((pQueElem != XNULLP) && ((XPOINT)(pQueElem->pMsg) >  eMAXPrio)) //why?
        {
            ptMsg = (t_XOSCOMMHEAD*)(pQueElem->pMsg);
            printf( "%-10d%-18d%-18d%-10d%-10d\r\n",
                inum++,
                ptMsg->datasrc.FID,
                ptMsg->datadest.FID,
                ptMsg->msgID,
                ptMsg->prio
                );
        }
        listIndex =  XOS_listNext(fid_msgq.queueList,listIndex);
        if(inum >msg_count)
        {
            printf("msgq_fidshow,recent %d msg display finished.\r\n", msg_count);
            break;
        }
    }
    printf(  "------------------------------------------\r\n");

    return XSUCC;
}


/************************************************************************
������: MOD_mqPrtCtrl
���ܣ���Ϣ���е���Ϣ��ӡ���ƿ���.
���룺
�����  N/A
���أ�
˵����MOD_mqPrtCtrl �������������
************************************************************************/
XVOID MOD_mqPrtCtrl(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 i =0;
    XBOOL ctrl = XFALSE ;

    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        return;
    }

    ctrl = (XBOOL )atol(ppArgv[1]);

    for(i=FID_XOSMIN;i <FID_MAX; i++)
        MsgPflg[i] = ctrl;
}


#ifdef XOS_MDLMGT
XPUBLIC XS32 XOS_MMStartFid(t_XOSLOGINLIST *LoginInfo,XVOID *Para1,XVOID *Para2)
{
    t_FIDCB fidCb;
    t_FIDCB* pFidCb;
    t_TIDCB tidCb;
    t_TIDCB *pTidCb;
    t_XOSLOGINLIST pLog;
    XS32 tidIndex;
    XS32 CurModuleId = (XS32)XosMMGetCurrentLoadModule();
    XVOID *pLocation;
    XS32 ret;
    XU32 fid;
    XS32 i = 0;

    if(XNULLP == LoginInfo)
    {
        return XERROR;
    }
    if(XNULLP == LoginInfo->stack)
    {
        return XERROR;
    }
    XOS_MemCpy(&pLog,LoginInfo,sizeof(t_XOSLOGINLIST));

    XOS_MemSet(&fidCb, 0, sizeof(t_FIDCB));
    fid = pLog.stack->head.FID;
    fidCb.attSwitch = XFALSE;
    XOS_MemCpy(fidCb.fidName, pLog.stack->head.FIDName, MAX_FID_NAME_LEN);
    fidCb.fidNum = fid;
    fidCb.initFunc = pLog.stack->init.init;
    fidCb.noticeFunc = pLog.stack->init.notice;
    fidCb.MsgFunc = pLog.stack->handle.message;
    fidCb.timerFunc = pLog.stack->handle.timer;
    fidCb.pidIndex  = XOS_GetLocalPID();
    fidCb.msgFreeType = pLog.stack->memfreetype;
    fidCb.attSwitch = XFALSE;
    fidCb.noticeFlag = XFALSE;
    fidCb.logLevel = PL_ERR;
    /*��������ģ�����ʱ����ʼ������ģ���trace�����־,modified (20061108)*/
    initFidTraceInfo((&(fidCb.traceInfo))); /*��ʼ��trace ����Ϣ*/
    if(gMMFidTraceLel != 0xffffffff && gMMFidTraceLel < PL_MAX)
    {
        fidCb.traceInfo.traceLevel = gMMFidTraceLel;
        for(i = 0;i < kCLI_MAX_CLI_TASK;i++)
        {
            fidCb.traceInfo.sessionLevel[i] = gMMFidTraceLel;
        }
    }
    
    pLocation = XNULLP;

    /*��ӵ�hash��*/
    pLocation = XOS_HashElemAdd(g_modMnt.fidHash, (XVOID*)&fid, (XVOID*)&fidCb, XFALSE);
    if(pLocation == XNULLP)
    {
        return XERROR;
    }
    pFidCb = (t_FIDCB*)XNULLP;
    pFidCb = XOS_HashGetElem(g_modMnt.fidHash, pLocation);
    if(pFidCb == XNULLP)
    {
        return XERROR;
    }
    tidIndex = XOS_ArrayFind(g_modMnt.tidArray, (XVOID*)(XPOINT)pLog.TID);
    /*�ҵ����������Ѿ�����*/
    if( tidIndex >= 0 || pLog.TID == 0 )
    {
        pFidCb->tidIndex = tidIndex;
    }
    else  /*û���ҵ�����Ҫ������*/
    {
        XOS_MemSet(&tidCb, 0, sizeof(t_TIDCB));
        tidCb.prio = pLog.prio;
        tidCb.stackSize = pLog.stacksize;
        tidCb.tidNum = pLog.TID;
        XOS_MemCpy(&(tidCb.tskName), pLog.taskname, MAX_TID_NAME_LEN);

        /* �������ļ���������ʱ������ģ����Ϣ���еĴ�СΪ�������ļ��ж�ȡ��ֵ */
        if (0 != gMMModuleList[CurModuleId].wsModInitPara.nMsgQueNum)
        {
            pLog.quenum = gMMModuleList[CurModuleId].wsModInitPara.nMsgQueNum;
        }
        
        /*windows��û�����ã��������ý�С�������*/
        if ( MIN_MSGS_IN_QUE  > pLog.quenum   || MAX_MSGSNUM_IN_QUE <= pLog.quenum)
        {
            tidCb.maxMsgsInQue = MAX_MSGS_IN_QUE;
        }
        else
        {
            tidCb.maxMsgsInQue =pLog.quenum;
        }

        tidIndex = XOS_ArrayAdd(g_modMnt.tidArray, (XOS_ArrayElement)&tidCb);
        if(tidIndex < 0)
        {
            return XERROR;
        }
        pFidCb->tidIndex = tidIndex;

        pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, tidIndex);
        if(pTidCb == (t_TIDCB*)XNULLP)
        {
            return XSUCC;
        }
        /*xos ��,��ʱ������������ں����Ƚ�����*/
        if (pTidCb->tidNum== FID_TIME)
        {
            ret = fidCb.initFunc(Para1, Para2);
            if(XSUCC != ret)
            {
                MMErr("FID_TIME:XOS_MMStartFid()-> Timer initFunc is wrong!");
                return XERROR;
            }
            
#ifndef XOS_NEED_OLDTIMER        
            /*��ʱ����ִ�г�ʼ������*/
            
            /*������ʱ��������������*/
            ret = XOS_TaskCreate_Ex((pTidCb->tskName), pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)tm_excuteLowProc, XNULLP, &(pTidCb->tskId),
                 &gMMModuleList[CurModuleId].module_cpu_bind);
                
                
            if(XSUCC != ret)
            {
                MMErr("FID_TIME:XOS_MMStartFid()-> can not create task[tsk_dealtimer]!");
                return XERROR;
            }
#else
             /*�;��ȶ�ʱ��*/
            ret = XOS_TaskCreate_Ex((pTidCb->tskName), pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)Low_TimerTask, XNULLP, &(pTidCb->tskId),
                &gMMModuleList[CurModuleId].module_cpu_bind);
            if(XSUCC != ret)
            {
                XOS_Trace(MD(FID_TIME,PL_ERR),"XOS_MMStartFid()-> TIME is wrong!");
                return XERROR;
            }
#ifdef XOS_HIGHTIMER
            /*�߾��ȶ�ʱ������*/
            ret = XOS_TaskCreate_Ex("tsk_HTIM", pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)High_TimerTask, XNULLP, &(pTidCb->tskId),             
                &gMMModuleList[CurModuleId].module_cpu_bind);
            if(XSUCC != ret)
            {
                XOS_Trace(MD(FID_TIME,PL_ERR),"XOS_MMStartFid()-> HIGHTIME is wrong!");
                return XERROR;
            }
#endif
            return XSUCC;
#endif            
        }
        else if (fidCb.MsgFunc == NULL && fidCb.timerFunc == NULL)
        {
            pTidCb->recvMsgQue.init = XFALSE;
            ret = XSUCC;
        }
        else
        {
            ret = QUE_MsgQCreate(&(pTidCb->recvMsgQue), pTidCb->maxMsgsInQue);
            if(ret != XSUCC)
            {
                pTidCb->recvMsgQue.init = XFALSE;
                XOS_Trace(MD(FID_ROOT, PL_ERR),
                    "XOS_MMStartFid()->create msg que failed ! tid: %d, tskName: %s",
                    pTidCb->tidNum, pTidCb->tskName);
                return XERROR;
            }
            pTidCb->recvMsgQue.init = XTRUE;
            pTidCb->recvMsgQue.tskIndex = pTidCb->tidNum;
            ret = XOS_TaskCreate_Ex((pTidCb->tskName), pTidCb->prio, pTidCb->stackSize,
                (os_taskfunc)MOD_allTaskEntry, (XVOID*)pTidCb, &(pTidCb->tskId),            
                &gMMModuleList[CurModuleId].module_cpu_bind);
        }

        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_ROOT, PL_ERR),
                "XOS_MMStartFid()->create task failed ! tid: %d, tskName: %s",
                pTidCb->tidNum, pTidCb->tskName);
            return XERROR;
        }
    }

    /*ִ�г�ʼ������*/
    if(XNULLP == fidCb.initFunc || fidCb.fidNum == FID_TIME)
    {
        XOS_Trace(MD(FID_ROOT, PL_WARN),
            "XOS_MMStartFid()->initFunc is NULLP !");
        
        return XSUCC;
    }

    ret = fidCb.initFunc(Para1, Para2);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_ROOT, PL_WARN),
            "XOS_MMStartFid()->excute fid[%s] init func failed!",
            fidCb.fidName);
        return XERROR;
    }

    return XSUCC;
}
#endif /* XOS_MDLMGT */


/************************************************************************
������: TRC_CMDShow
���ܣ���ʾ���е�fid ��trace�����־
��ʾ��ʽΪ:

------------------------------------------
fid  fidname   traclevel  filename   linenum   timeflag   trstotskflag   outmode
------------------------------------------

���룺
�����  N/A
���أ�  XSUCC OR XERROR
˵����fidshow �������������
************************************************************************/
XVOID  TRC_CMDShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
#if 1 /* wulei del for B07 2006.11.20 */
    t_FIDSHOW fidShow;

    /*fid list show  */
    XOS_CliExtPrintf(pCliEnv,
        "fid list \r\n---------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-18s%-8s%-8s%-8s%-8s%-8s%-18s\r\n",
        "fid",
        "fidName",
        "trclev",
        "fnF",
        "lnumF",
        "timeF",
        "totaskF",
        "outmode"
        );
    XOS_MemSet(&fidShow, 0, sizeof(t_FIDSHOW));
    fidShow.pCliEnv = pCliEnv;
    fidShow.index = 0;
    /*����hash*/
    XOS_HashWalk(g_modMnt.fidHash, MOD_showFidTraceInfo, &fidShow);

    /*end of fid list */
    XOS_CliExtPrintf(pCliEnv,
        "---------------------------------------------------------------------------\r\n");

    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n" , fidShow.index);
#endif

    return;
}

RESET_PROC_FUNC pRestProcFunc = NULL;
/************************************************************************
������: XOS_ResetProcFuncReg
���ܣ�  ע��ص�����������ϵͳ����ǰ�����ƺ���
���룺  ulFuncAddr ��ע��ĺ���
�����
���أ�
˵����
************************************************************************/
XS32 XOS_ResetProcFuncReg(void * ulFuncAddr)
{
     pRestProcFunc = (RESET_PROC_FUNC)ulFuncAddr;
     return XSUCC;
}


extern XPUBLIC XS32 LOG_TskStackInfo(t_XOSTASKID *tskID,XCHAR *pReason);
/************************************************************************
������: XOS_ResetByQueFull
���ܣ�  ��Ϣ������ʱ����ƽ̨
���룺  tidNum ��Ϣ�������������
        exitFlag 0:ƽ̨����ҵ��ע��Ļص�������1:ƽ̨�˳�����
�����
���أ�
˵����
************************************************************************/
#define STACK_HEAD_LEN 256
XPUBLIC XVOID XOS_ResetByQueFull(XS32 tidNum,XS32 exitFlag)
{
    XCHAR buf[STACK_HEAD_LEN] = {0};
    t_TIDCB *pTidCb = (t_TIDCB *)XNULLP;

    pTidCb = MOD_getTidCbByTidNum(tidNum);
    if(pTidCb == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR),
            "MOD_getTidCbByTidNum()-> get tidCB fail! ");
        return ;
    }
    
    XOS_Sprintf(buf, STACK_HEAD_LEN, 
        "task(tskid = 0x%x(%d), tskname = %s) recvmsgQue is full!\r\n", 
        pTidCb->tskId, (XS32)pTidCb->tskId, pTidCb->tskName);
    LOG_TskStackInfo(&pTidCb->tskId,buf);

    /*����ǿ��ɱ�������߳�*/
#if 0
    XOS_TaskDel(pTidCb->tskId);
#endif

    if(exitFlag == 0)
    {
        if(NULL != pRestProcFunc)
        {
            XOS_Sprintf(buf, STACK_HEAD_LEN, 
            "reboot: task(tskid = 0x%x(%d), tskname = %s) recvmsgQue is full,system will reboot!!\r\n", 
            pTidCb->tskId, (XS32)pTidCb->tskId, pTidCb->tskName);

            write_to_syslog("task [%s] msg queue exceed 90% and call callback function!!!",pTidCb->tskName);
            pRestProcFunc();
        }
        else
        {
            /*ƽ̨��ҵ��δע�ᣬ��Ĭ�ϲ�����ֱ��95%�˳�*/
        }
    }
#ifdef XOS_VXWORKS
    else
    {
        /*vxƽ̨����������ҪHA���˳����̲�����*/
    }
#else
    else
    {
        printf("task [%s] msg queue exceed 95 percent and exit...\n",pTidCb->tskName);
        write_to_syslog("task [%s] msg queue exceed 95 percent and exit!!!",pTidCb->tskName);
        _exit(0);
    }
#endif
/*�˵����޸�Ϊ��Ӧ�ò���ݾ���������pRestProcFunc��ѡ�����*/
#if 0
    XOS_Reset(1);
#endif
    return;
}
/*
XS32 trace_msgq_size(const XS8* msg,...)
{
    XCHAR msg_format[1024]  = {0}; 
    XU32  msg_curlen  = 0;
#ifdef XOS_VXWORKS        
    XS32  len = 0;
    XS8*  ptr = NULL;
    FILE * fpw = NULL;
#endif
    va_list ap;
    
    
    va_start(ap, msg);
    msg_curlen = XOS_VsPrintf(msg_format, sizeof(msg_format)-1, msg, ap);
    va_end(ap);
    if (msg_curlen <= 0) {
        return XERROR;
    }
#ifdef XOS_VXWORKS    
    fpw = fopen( "/ata0a/syslog.txt", "a+" );
    if(fpw != NULL) {
        ptr = msg_format;
        while(msg_curlen > 0) {
            len = fwrite(ptr, 1, msg_curlen, fpw);
            msg_curlen -= len;
            ptr += len;
        }
        fclose(fpw);
        return XSUCC;
    }
#endif    
    return XERROR;
}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */


