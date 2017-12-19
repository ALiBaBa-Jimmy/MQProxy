
/***************************************************************
**
**  Xinwei Telecom Technology co.,ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: trace_agent.c
**
**  description:  this agent connect to trace server,receives filter message and sends
**              trace message to trace server.
**
**  author: liukai
**
**  date:   2014.8.12
**
** 
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef XOS_TRACE_AGENT

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xosencap.h"
#include "xostl.h"
#include "xosarray.h"
#include "xosmem.h"
#include "trace_agent.h"
#include "xosipmi.h"
#include "xosnetinf.h"

/*-------------------------------------------------------------------------
                      ģ���ڲ��궨��
-------------------------------------------------------------------------*/
#define TA_MISSION_NUM  32  /*���֧�ֵĹ�����������*/
#define TA_OFFLINE_NUM  4   /*���֧�ֵ����߸�����������*/


#define TA_LINK_HANDLE  0x01020304  /*TA��������TS֮���SCTP��·���*/

#define TS_F1IP_PREFIX    "172.16.100.%d"     /*TS��F1��������IPǰ׺*/
#define TS_F2IP_PREFIX    "172.16.101.%d"     /*TS��F2��������IPǰ׺*/
#define TA_IP_PREFIX    "172.16.%d.%d"     /*TA��F1��������IPǰ׺*/
#define TA_F1IP_INDEX    2                  /*F1ƽ���������������ڼ���F1������IP*/
#define TA_F2IP_INDEX    3                  /*F2ƽ���������������ڼ���F2������IP*/
#define TS_IP_BASE          235                 /*TA��ip���Ķ�Ϊ220+��λ��*/

#define TA_F1_NUM       2                   /*F1����ƽ���*/
#define TA_F2_NUM       3                   /*F2����ƽ���*/

#define TS_PORT           17401             /*TS����˼����˿�*/
#define TA_IP_LEN       64                  /*IP��ַ�ַ�����󳤶�*/

#define TA_CHECK_INTERVAL   4000        /*TAÿ��4s�����TS����·����״̬*/
#define TA_CHECK_NUM        3           /*���3�Σ������Ȼ������ֹͣ���и�������*/

#define TSDATA_BEGIN_FLAG (0x7ea5)  /*Э����ʼ��־*/
#define TSDATA_END_FLAG (0x7e0d)    /*Э�������־*/
#define TA_LOG_INVALID  (0xff)      /*����������log���ټ�����Ϊ0xff��ʾ��������־*/

#define TA_IP_NUM       2               /*TS�� TA���԰�2��IP*/

#define TA_PORT_BASE        10000 /*TA��TS�˿�ʱ����ʼֵ*/
#define TA_PORT_OFFSET      32     /*TA��TS�˿�ʱ��ԪIDƫ��ֵ*/

#define TA_NE_TYPE_MIN      0       /*��Ԫ������Сֵ*/
#define TA_NE_TYPE_MAX      7       /*��Ԫ�������ֵ*/
#define TA_NE_ID_MIN        1       /*��ԪID��Сֵ*/
#define TA_NE_ID_MAX        127     /*��ԪID���ֵ*/
#define TA_PROCESS_ID_MIN        1       /*��Ԫ�߼�����ID��Сֵ*/
#define TA_PROCESS_ID_MAX        32     /*��Ԫ�߼�����ID���ֵ*/
#define TA_SLOT_ID_MIN        1       /*��λ����Сֵ*/
#define TA_SLOT_ID_MAX        14     /*��λ�����ֵ*/
#define TA_SHELF_ID_MIN        1       /*�����Сֵ*/
#define TA_SHELF_ID_MAX        25     /*������ֵ*/

#define TA_REG_NUM              32      /*ɾ����������ʱ������ע���ģ�鷢֪ͨ��Ϣ�����֧��32��ģ��*/
#define TA_NE_EMME          0x0001  /*EMME����Ԫ���Ͷ��壬��TS��ͬ��*/

#define TA_INVALID_LINK     0xFFFF  /*��Ч����·��ID����·ID*/
/*TraceID�����ù��������Ӧ��λ*/
#define TA_SET_BIT(v,x)     ((v) |= (1<< ((x) % (8 * sizeof(XU32)))))

/*TraceID��������������Ӧ��λ*/
#define TA_CLR_BIT(v,x)     ((v) &= (~(1<< ((x) % (8 * sizeof(XU32))))))

/*TraceID���ж�λ�Ƿ�����*/
#define TA_ISSET_BIT(v,x)   ((v) & (1<< ((x) % (8 * sizeof(XU32)))))
/*-------------------------------------------------------------------------
                     ģ���ڲ��ṹ��ö�ٶ���
-------------------------------------------------------------------------*/
typedef enum 
{
    TA_CHECK_TIMER = 1,
}e_taTimer;

/*TA����������ƿ�,�洢TS�·��Ĺ��������MD5ֵ*/
typedef struct t_TaMissionT t_TaMission;
struct t_TaMissionT
{
    XS32 idx;               /*�������������*/
    XU8 md5[MAX_MD5_LEN];  /*TS�·��Ĺ��������md5ֵ*/
    
    XU8 ifFlag;            /*�Ƿ����ģ��ӿ���Ϣ*/
    
    XU8 msgType;            /*��Ϣ��������:���ƣ�ý�壬ip����*/
    XU8 logLevel;          /*log���� 0~6, oxff*/

    XU16  groupID;         /*��·��ID���ӿڸ���ʹ��*/
    XU16  linkID;          /*��·ID���ӿڸ���ʹ��*/
    
    t_TaMission *next;     /*�����洢��ͬ�û���ʶ�Ĺ�������*/
};

/*�����û���ʶ��Ӧ�Ĺ���������ƿ�*/
typedef struct
{
    XU32 traceID;               /*�û���ʶ�µĿ�����TraceID*/
    XU8 tel[MAX_TEL_LEN];     /*�绰����*/
    XU16 telLen;                /*�绰���볤��*/
    XU8 imsi[MAX_IMSI_LEN];    /*IMSI*/
    XU16 imsiLen;               /*IMSI����*/
    e_IF_TYPE ifType;           /*�ӿڹ�������*/

    XBOOL flag;                 /*��־λ��XFALSE���У�XTRUEռ��*/
    t_TaMission *mission;       /*��������*/
}t_TaBlock;

/*�洢����ע�ᵽTA��ģ���FID*/
typedef struct
{
    XU32 fid[TA_REG_NUM];   /*�洢����ע���ģ���FID*/
    XU32 num;               /*�Ѿ�ע���ģ������*/
}t_reg_fids;

/*���͸�TS�ı���Ԫ�����Ϣ*/
typedef struct
{
    XU16    neType;     /*��Ԫ����*/
    XU16    neID;       /*��ԪID*/
    XU16    processID;  /*�������߼����̺�*/
    XU16    slotID;     /*�����λ��*/
    XU16    shelfID;     /*�������ڿ��*/
}t_processInfo;

typedef struct
{
    t_TaBlock   taCb[TA_MISSION_NUM];   /*����������ƿ�*/
    XU16    missionNum;                 /*TA�Ѿ����յĹ���������*/
    XU16    dataNum;                 /*TA���û������ݸ��ٹ���������*/
    t_XOSMUTEXID    mutex;             /*������*/
    t_processInfo   localInfo;        /*���͸�TS����Ԫ�����Ϣ*/
    t_reg_fids  regFids;                /*�洢ע�ᵽTA����Ԫģ���FID*/
    
    XU16    port;                      /*��TS����ʹ�õı��ض˿�*/
    XU32    taIP1;                     /*ta�󶨵ĵ�һ��IP*/
    XU32    taIP2;                     /*ta�󶨵ĵڶ���IP*/
    XS32    tsLowSlot;                  /*�յ���TS�ĵͲ�λ��*/
    XS32    tsHighSlot;                 /*�յ���TS�ĸ߲�λ��*/
    XU32    tsIP1;                     /*ts�����ĵ�һ��IP*/
    XU32    tsIP2;                     /*ts�����ĵڶ���IP*/
    XBOOL   tsFlag;                     /*��TS����������Ƿ�����*/
    XBOOL   nasFlag;                    /*�Ƿ���Ҫ��NASģ�黺���Ȩ��Ϣ*/
    XU16    expireTimes;                /*��TS������ʱ������*/
    HAPPUSER     userHandle;            /*��TS����·ҵ����*/
    HLINKHANDLE     linkHandle;         /*NTL�������·���*/
}t_TaCb;
/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/

static t_TaCb g_TaCb;       /*TAȫ�ֿ��ƿ�*/
XU8 g_interfaceVer = 1;     /*ҵ��Э��汾��*/
PTIMER ta_timer= XNULL;
XU32 g_ta_num = 0;          /*TaTrace������*/
XU32 g_ta_if_num = 0;      /*�ӿڸ��ټ�����*/
XU32 g_ta_tmg_num = 0;      /*ý����ټ�����*/
XU32 g_ta_log_num = 0;      /*log���ټ�����*/
XCONST XS8* ta_user_type[TRACE_MSG_TYPE_IPDATA+1] = 
{
    "",
    "CTRL",
    "VOICE",
    "VIDEO",
    "IP",
};

/*-------------------------------------------------------------------------
                   ģ���ڲ�����
-------------------------------------------------------------------------*/
XS32 TA_SendMsg(void *buf,XU32 len,XU32 fid,XU16 msgID);
XS32 TA_InitAckProc(t_XOSCOMMHEAD *pMsg);
XS32 TA_StartAckProc(t_XOSCOMMHEAD *pMsg);
XS32 TA_TsProc(t_XOSCOMMHEAD *pMsg);
XS32 TA_StopProc(t_XOSCOMMHEAD *pMsg);
XS32 TA_DeleteAllMission();
XS32 TA_regCli(XVOID);
XVOID TA_Show(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);
XVOID TA_Count(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);
XS32 TA_StoreUserMission(t_SCTPDATAIND *sctpData);
XS32 TA_StoreInterfaceMission(t_SCTPDATAIND *sctpData);
XS32 TA_StopTrace(t_SCTPDATAIND *sctpData);

/************************************************************************
������:TA_SendMsg
����: taģ�鷢����Ϣ�ӿ�
����:buf    -���͵�����ָ��
    len     -���ݳ���
    fid     -Ŀ��ģ��fid
    msgID   -��Ϣ����
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XS32 TA_SendMsg(XVOID *buf,XU32 len,XU32 fid,XU16 msgID)
{
    XS32 ret = 0;
    t_XOSCOMMHEAD *pMsg = NULL;

    if((NULL == buf) || (0 == len) )
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"input param error!");
        return XERROR;
    }

    /*������Ϣ�ڴ�*/
    pMsg = XOS_MsgMemMalloc(FID_TA,len);
    if(XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"malloc msg failed!");
        return XERROR;
    }

    /*��д��Ϣ����*/
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = FID_TA;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = fid;
    pMsg->length = (XU32)len;
    pMsg->msgID = msgID;
    pMsg->prio = eNormalMsgPrio;

    XOS_MemCpy(pMsg->message, buf,len);

    /*��������*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_TA, PL_ERR),"send msg [%d] to FID[%d] failed!",msgID,fid);
        XOS_MsgMemFree(FID_TA, pMsg);
        return XERROR; 
    }

    return XSUCC;
}

/************************************************************************
������:TA_InitAckProc
����: ����ntl�ϱ���init ack��Ϣ��������·������Ϣ��ntl
����:pMsg   -NTL���͵���Ϣָ��
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: ����NTL���͵�initAck��Ϣ
************************************************************************/
XS32 TA_InitAckProc(t_XOSCOMMHEAD *pMsg)
{
    t_LINKINITACK *pLinkAck = NULL;
    t_LINKSTART taStart;
    XS32 ret = 0;
    
    if (XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_TA, PL_ERR), "input parameter is null!" );
        return XERROR;
    }
    
    pLinkAck = (t_LINKINITACK*)(pMsg->message);
    if(XNULLP == pLinkAck)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"pLinkAck is NULL!");
        return XERROR;
    }
        
    if ( g_TaCb.userHandle != pLinkAck->appHandle )
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"error appHandle:%d",pLinkAck->appHandle);
        return XERROR;
    }

    /*��鷵�صĽ��*/
    if( pLinkAck->lnitAckResult != eSUCC )
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"lnitAckResult != eSUCC!");
        return XERROR;
    }

    /* ��¼linkHandle */
    g_TaCb.linkHandle = pLinkAck->linkHandle;
    
    /*������·*/
    XOS_MemSet(&taStart, 0, sizeof(t_LINKSTART));
    
    taStart.linkStart.sctpClientStart.myAddr.ip[0] = g_TaCb.taIP1;
    taStart.linkStart.sctpClientStart.myAddr.ip[1] = g_TaCb.taIP2;
    taStart.linkStart.sctpClientStart.myAddr.port = g_TaCb.port;
    taStart.linkStart.sctpClientStart.myAddr.ipNum = TA_IP_NUM;

    taStart.linkStart.sctpClientStart.peerAddr.ip[0] = g_TaCb.tsIP1;
    taStart.linkStart.sctpClientStart.peerAddr.ip[1] = g_TaCb.tsIP2;
    taStart.linkStart.sctpClientStart.peerAddr.port = TS_PORT;
    taStart.linkStart.sctpClientStart.peerAddr.ipNum = TA_IP_NUM;
    
    taStart.linkStart.sctpClientStart.hbInterval = 0;
    taStart.linkStart.sctpClientStart.pathmaxrxt = 0;
    taStart.linkStart.sctpClientStart.streamNum = 0;
    
    taStart.linkHandle = pLinkAck->linkHandle;
    
    ret = TA_SendMsg( &taStart, sizeof(t_LINKSTART), FID_NTL,eLinkStart );
    if( ret != XSUCC )
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"send eLinkStart msg  failed!");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
������:TA_StartAckProc
����: ������ȷ����Ϣ
����:pMsg   -NTL���͵���Ϣָ��
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: ����NTL���͵�startAck��Ϣ
************************************************************************/
XS32 TA_StartAckProc(t_XOSCOMMHEAD *pMsg)
{
    t_SCTPSTARTACK *startAck;
    e_TIMESTATE timeState = TIMER_STATE_NULL;

    if (XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_TA, PL_ERR), "pMsg is null!" );
        return XERROR;
    }

    startAck = (t_SCTPSTARTACK*)pMsg->message;
    
    if (XNULLP == startAck)
    {
        XOS_Trace(MD(FID_TA, PL_ERR), "startAck is null!" );
        return XERROR;
    }

    if(startAck->appHandle != g_TaCb.userHandle)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"error linkhandle:%d",startAck->appHandle);
        return XERROR;
    }

    if(eBlockWait == startAck->linkStartResult)
    {
        XOS_Trace(MD(FID_TA,PL_INFO),"blockwait!");
        return XSUCC;
    }
    
    if(eSUCC == startAck->linkStartResult)
    {
        XOS_Trace(MD(FID_TA,PL_INFO),"connect to ts succ!");
        XOS_MutexLock(&(g_TaCb.mutex));
        g_TaCb.tsFlag = XTRUE;
        g_TaCb.expireTimes = 0;
        XOS_MutexUnlock(&(g_TaCb.mutex));

        /*���ӳɹ���ֹͣ������ʱ��*/
        timeState = XOS_TimerGetState(FID_TA, ta_timer);
        if(TIMER_STATE_RUN == timeState)
        {
            XOS_TimerStop(FID_TA, ta_timer);
        }
        return XSUCC;
    }

    XOS_Trace(MD(FID_TA,PL_DBG),"connect to ts fail:%d!",startAck->linkStartResult);

    return XERROR;
}

/************************************************************************
������:TA_StoreUserMission
����: �����û����ٹ�������
����:sctpData   - ts���͵ĸ�������
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XS32 TA_StoreUserMission(t_SCTPDATAIND *sctpData)
{
    t_TaStartUserTraceReq *userReq = NULL;
    t_TaMission *mission = NULL;
    t_TaMission *tail = NULL;
    XS32 i = 0;
    XS32 idle = -1;             /*�����û����ƿ������*/
    XU8 sameUser = XFALSE;      /*�¹���������û���ʶ�Ƿ������й���������ͬ*/

    if(NULL == sctpData)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"arg is null!");
        return XERROR;
    }
    
    XOS_Trace(MD(FID_TA,PL_INFO),"get user trace request from ts");
    userReq = (t_TaStartUserTraceReq *)sctpData->pData;

    if(NULL == userReq)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"arg in data is null!");
        return XERROR;
    }

    userReq->data.task.usTelLen = XOS_INET_NTOH_U16(userReq->data.task.usTelLen);
    userReq->data.task.usImsiLen = XOS_INET_NTOH_U16(userReq->data.task.usImsiLen);
    /*��ӹ�������������ƿ�*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*���α������й��������Ƿ������ͬ�û���ʶ�Ĺ�������*/
        if(XTRUE == g_TaCb.taCb[i].flag)
        {
            /*������ͬ��ʶ�Ĺ�������,��ѹ����������ӵ���������������*/
            if(((g_TaCb.taCb[i].telLen > 0) && 
                (!XOS_StrNcmp(userReq->data.task.ucTel, g_TaCb.taCb[i].tel, 
                XOS_MAX(userReq->data.task.usTelLen,g_TaCb.taCb[i].telLen))))
                ||((g_TaCb.taCb[i].imsiLen > 0) && 
                (!XOS_StrNcmp(userReq->data.task.ucImsi, g_TaCb.taCb[i].imsi, 
                XOS_MAX(userReq->data.task.usImsiLen,g_TaCb.taCb[i].imsiLen)))))
            {
                XOS_Trace(MD(FID_TA,PL_INFO),"user ID already exists in TA");
                mission = (t_TaMission *)XOS_MemMalloc(FID_TA, sizeof(t_TaMission));
                if(XNULLP == mission)
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_START_TRACE:malloc failed!");
                    XOS_MutexUnlock(&(g_TaCb.mutex));
                    XOS_MemFree(FID_TA, sctpData->pData);
                    return XERROR;
                }

                /*�ҵ����������β�ڵ㣬Ȼ�������µĹ�������*/
                tail = g_TaCb.taCb[i].mission;
                if(!tail)
                {
                    tail = mission;
                }
                else
                {
                    while(tail->next)
                    {
                        tail = tail->next;
                    }
                    tail->next = mission;
                }
                
                /*����������񵽿��ƿ�*/
                mission->idx = userReq->head.idx;
                if(mission->idx < 0 || mission->idx >= TA_MISSION_NUM)
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"invalid idx:%d!",mission->idx);
                    XOS_MutexUnlock(&(g_TaCb.mutex));
                    XOS_MemFree(FID_TA, sctpData->pData);
                    return XERROR;
                }
                XOS_StrNcpy(mission->md5, userReq->data.ucMd5,MAX_MD5_LEN);
                mission->ifFlag = userReq->data.task.ucIfFlag;
                mission->msgType = userReq->data.task.ucMsgType;
                mission->logLevel = userReq->data.task.ucLogLevel;
                /*�û�����ٲ�ʹ����·ID����*/
                mission->groupID = 0;
                mission->linkID =0;
                mission->next = NULL;
                if((TRACE_MSG_TYPE_CONTROL != mission->msgType) && (mission->msgType <= TRACE_MSG_TYPE_IPDATA))
                {
                    g_TaCb.dataNum++;
                }

                /*�޸��û����ƿ�ľֲ�TraceID*/
                TA_SET_BIT(g_TaCb.taCb[i].traceID, mission->idx);

                /*�ߵ�����˵���ҵ���ͬ���û���ʶ�Ĺ�������*/
                sameUser = XTRUE;
                g_TaCb.missionNum++;
                break;
            }
        }
        else if(idle < 0)
        {
            /*�洢��һ�����п��ƿ��������Ա��������û����ƿ�ʱֱ��ʹ��*/
            idle = i;
        }
    }
    /*��������ͬ�û���ʶ�Ĺ������񣬴����µ��û����ƿ�*/
    if(!sameUser)
    {
        XOS_Trace(MD(FID_TA,PL_INFO),"new user ID for TA");
        /*�޿��п��ƿ飬��Ӧ�ó�����������������ڴ汻��*/
        if(idle < 0 || idle >= TA_MISSION_NUM)
        {
            XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_START_TRACE:TA find no block for new mission!");
            XOS_MutexUnlock(&(g_TaCb.mutex));
            XOS_MemFree(FID_TA, sctpData->pData);
            return XERROR;
        }
        else
        {
            /*����û����ƿ������Ϣ*/
            g_TaCb.taCb[idle].flag = XTRUE;
            g_TaCb.taCb[idle].telLen = userReq->data.task.usTelLen;
            XOS_StrNcpy(g_TaCb.taCb[idle].tel, userReq->data.task.ucTel,userReq->data.task.usTelLen);
            g_TaCb.taCb[idle].imsiLen = userReq->data.task.usImsiLen;
            XOS_StrNcpy(g_TaCb.taCb[idle].imsi, userReq->data.task.ucImsi,userReq->data.task.usImsiLen);
            
            mission = (t_TaMission *)XOS_MemMalloc(FID_TA, sizeof(t_TaMission));
            if(XNULLP == mission)
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_START_TRACE:malloc failed!");
                XOS_MutexUnlock(&(g_TaCb.mutex));
                XOS_MemFree(FID_TA, sctpData->pData);
                return XERROR;
            }

            /*����������񵽿��ƿ�*/
            mission->idx = userReq->head.idx;;
            if(mission->idx < 0 || mission->idx >= TA_MISSION_NUM)
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"invalid idx:%d!",mission->idx);
                XOS_MutexUnlock(&(g_TaCb.mutex));
                XOS_MemFree(FID_TA, sctpData->pData);
                return XERROR;
            }
            XOS_StrNcpy(mission->md5, userReq->data.ucMd5,MAX_MD5_LEN);
            mission->ifFlag = userReq->data.task.ucIfFlag;
            mission->msgType = userReq->data.task.ucMsgType;
            mission->logLevel = userReq->data.task.ucLogLevel;
            /*�û�����ٲ�ʹ����·ID����*/
            mission->groupID = 0;
            mission->linkID =0;
            mission->next = NULL;

            /*�����������ӵ��û���������*/
            g_TaCb.taCb[idle].mission = mission;

            /*�����û����ƿ�ֲ�TraceID*/
            TA_SET_BIT(g_TaCb.taCb[idle].traceID, mission->idx);
            if((TRACE_MSG_TYPE_CONTROL != mission->msgType) && (mission->msgType <= TRACE_MSG_TYPE_IPDATA))
            {
                g_TaCb.dataNum++;
            }

            g_TaCb.missionNum++;
        }
    }

    return XSUCC;
}

/************************************************************************
������:TA_StoreInterfaceMission
����: ����ӿڸ��ٹ�������
����:sctpData   - ts���͵ĸ�������
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XS32 TA_StoreInterfaceMission(t_SCTPDATAIND *sctpData)
{
    t_TaStartIfTraceReq *ifReq = NULL;
    t_TaMission *mission = NULL;
    t_TaMission *tail = NULL;
    XS32 i = 0;
    XS32 idle = -1;             /*�����û����ƿ������*/
    XU8 sameUser = XFALSE;      /*�¹���������û���ʶ�Ƿ������й���������ͬ*/

    if(NULL == sctpData)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"arg is null!");
        return XERROR;
    }

    XOS_Trace(MD(FID_TA,PL_INFO),"get interface trace request from TS");
    ifReq = (t_TaStartIfTraceReq *)sctpData->pData;

    if(NULL == ifReq)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"arg in data is null!");
        return XERROR;
    }

    /*��ӹ�������������ƿ�*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*���α������й��������Ƿ������ͬ�ӿڵĹ�������*/
        if(XTRUE == g_TaCb.taCb[i].flag)
        {
            if(g_TaCb.taCb[i].ifType == ifReq->data.task.ucIfType )
            {
                XOS_Trace(MD(FID_TA,PL_INFO),"interface already exists in TA");
                mission = (t_TaMission *)XOS_MemMalloc(FID_TA, sizeof(t_TaMission));
                if(XNULLP == mission)
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"malloc failed!");
                    XOS_MutexUnlock(&(g_TaCb.mutex));
                    XOS_MemFree(FID_TA, sctpData->pData);
                    return XERROR;
                }
                /*�ҵ����������β�ڵ�*/
                tail = g_TaCb.taCb[i].mission;
                while(tail->next)
                {
                    tail = tail->next;
                }
                tail->next = mission;
                
                /*�����������*/
                mission->idx = ifReq->head.idx;
                if(mission->idx < 0 || mission->idx >= TA_MISSION_NUM)
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"invalid idx:%d!",mission->idx);
                    XOS_MutexUnlock(&(g_TaCb.mutex));
                    XOS_MemFree(FID_TA, sctpData->pData);
                    return XERROR;
                }
                XOS_StrNcpy(mission->md5, ifReq->data.ucMd5,MAX_MD5_LEN);
                
                /*�ӿڸ����������ģ��ӿ���Ϣ����־*/
                mission->ifFlag = 0;
                mission->logLevel = TA_LOG_INVALID;
                mission->groupID = XOS_HtoNs(ifReq->data.task.usLinkGid);
                mission->linkID = XOS_HtoNs(ifReq->data.task.usLinkId);
                mission->next = NULL;
                
                sameUser = XTRUE;
                g_TaCb.missionNum++;
                break;
            }
        }
        else if(idle < 0)
        {
            /*�洢��һ�����п��ƿ��������Ա����ֱ��ʹ��*/
            idle = i;
        }
    }
    if(!sameUser)
    {
        XOS_Trace(MD(FID_TA,PL_INFO),"new interface for TA");
        /*�޿��п��ƿ飬��Ӧ�ó�����������������ڴ汻��*/
        if(idle < 0 || idle >= TA_MISSION_NUM)
        {
            XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_START_TRACE:TA find no block for new mission!");
            XOS_MutexUnlock(&(g_TaCb.mutex));
            XOS_MemFree(FID_TA, sctpData->pData);
            return XERROR;
        }
        else
        {
            /*����û����ƿ������Ϣ*/
            g_TaCb.taCb[idle].flag = XTRUE;
            g_TaCb.taCb[idle].ifType = (e_IF_TYPE)ifReq->data.task.ucIfType;
            
            mission = (t_TaMission *)XOS_MemMalloc(FID_TA, sizeof(t_TaMission));
            if(XNULLP == mission)
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_START_TRACE:malloc failed!");
                XOS_MutexUnlock(&(g_TaCb.mutex));
                XOS_MemFree(FID_TA, sctpData->pData);
                return XERROR;
            }
            /*������������ƿ�*/
            mission->idx = ifReq->head.idx;
            if(mission->idx < 0 || mission->idx >= TA_MISSION_NUM)
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"invalid idx:%d!",mission->idx);
                XOS_MutexUnlock(&(g_TaCb.mutex));
                XOS_MemFree(FID_TA, sctpData->pData);
                return XERROR;
            }
            XOS_StrNcpy(mission->md5, ifReq->data.ucMd5,MAX_MD5_LEN);
            
            /*�ӿڸ����������ģ��ӿ���Ϣ����־*/
            mission->ifFlag = 0;
            mission->logLevel = TA_LOG_INVALID;
            mission->groupID = XOS_HtoNs(ifReq->data.task.usLinkGid);
            mission->linkID = XOS_HtoNs(ifReq->data.task.usLinkId);
            mission->next = NULL;

            /*�����������ӵ����ƿ����������*/
            g_TaCb.taCb[idle].mission = mission;

            g_TaCb.missionNum++;
        }
    }
    
    return XSUCC;
}

/************************************************************************
������:TA_StopTrace
����: ֹͣ��������
����:sctpData   - ts���͵�ֹͣ��������
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XS32 TA_StopTrace(t_SCTPDATAIND *sctpData)
{
    t_TaMission *mission = NULL;
    t_TaMission *prev = NULL;
    t_TaStopTraceReq *stopReq = NULL;
    XS32 ret = 0;
    XS32 i = 0;
    XS32 idx = -1;              /*�����������������*/
    XU8 stopFlag = XFALSE;      /*�ҵ���Ҫֹͣ�Ĺ�������ı�ʶ*/
    t_DelTrace delMsg;
    
    if(NULL == sctpData)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"arg is null!");
        return XERROR;
    }

    stopReq = (t_TaStopTraceReq *)sctpData->pData;
    if(NULL == stopReq)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"arg in data is null!");
        return XERROR;
    }

    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        if(g_TaCb.taCb[i].flag)
        {
            mission = g_TaCb.taCb[i].mission;
            if(!mission)
            {
                /*���ƿ���ڣ����ǹ�����������Ϊ�գ�˵�����쳣��*/
                XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_STOP_TRACE:mission NULL!");
                XOS_MemFree(FID_TA, sctpData->pData);
                return XERROR;
            }
            
            prev = mission;
            while(mission)
            {
                if(!XOS_StrNcmp(mission->md5, stopReq->data.ucMd5, MAX_MD5_LEN))
                {
                    stopFlag = XTRUE;
                    idx = mission->idx;
                    break;
                }
                prev = mission;
                mission = mission->next;
            }
        }
        /*����ҵ���������������ڹ��˿��ƿ���ɾ��*/
        if(stopFlag)
        {
            XOS_Trace(MD(FID_TA,PL_INFO),"find the trace mission which user wants to stop");

            if((TRACE_MSG_TYPE_CONTROL == mission->msgType)&&(mission->msgType <= TRACE_MSG_TYPE_IPDATA))
            {
                g_TaCb.dataNum--;
            }

            /*����������ɾ��������*/
            if(prev == mission)
            {
                /*Ҫɾ���Ĺ�������������ͷ*/
                g_TaCb.taCb[i].mission = mission->next;
            }
            else
            {
                /*Ҫɾ���Ĺ�������������ͷ*/
                prev->next = mission->next;
            }
            /*�ͷ��ڴ�*/
            XOS_MemFree(FID_TA, mission);
            mission = NULL;
            
            /*������Ϣ��ע�ᵽTA����Ԫģ�飬���û���ʶ��ֹͣ����*/
            if(g_TaCb.regFids.num > 0 && g_TaCb.regFids.num < TA_REG_NUM)
            {
                XOS_MemSet(&delMsg, 0,sizeof(delMsg));
                delMsg.traceID = g_TaCb.taCb[i].traceID;
                delMsg.index = idx;
                XOS_StrNcpy(delMsg.tel,g_TaCb.taCb[i].tel,g_TaCb.taCb[i].telLen);
                XOS_StrNcpy(delMsg.imsi,g_TaCb.taCb[i].imsi,g_TaCb.taCb[i].imsiLen);

                for(i = 0;i < g_TaCb.regFids.num;i++)
                {
                    ret = TA_SendMsg(&delMsg,sizeof(delMsg),g_TaCb.regFids.fid[i],TA_DEL_REQ);
                    if( XERROR == ret )
                    {
                        XOS_Trace(MD(FID_TA,PL_ERR),"send TA_DEL_REQ msg to module[%d] failed!",g_TaCb.regFids.fid[i]);
                    }
                }
            }
            /*ɾ������������޸��û����ƿ�ľֲ�TraceID*/
            TA_CLR_BIT(g_TaCb.taCb[i].traceID, idx);
                
            /*�û����ƿ�����û�й�����������Ҫ�ͷ��û����ƿ�*/
            if(!g_TaCb.taCb[i].mission)
            {
                /*�ͷſ��ƿ�*/
                XOS_MemSet(&(g_TaCb.taCb[i]), 0, sizeof(t_TaBlock));
            }

            g_TaCb.missionNum--;
            break;
        }
    }
    if(!stopFlag)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_STOP_TRACE:mission not found!");
    }

    /*��û�й�������ʱ����NAS����Ϣֹͣ�����Ȩ��Ϣ*/
    if(TA_NE_EMME == g_TaCb.localInfo.neType)
    {
        if(0 == g_TaCb.missionNum && XTRUE == g_TaCb.nasFlag)
        {
            XOS_Trace(MD(FID_TA,PL_INFO),"send msg to NAS,to stop saving identify msg");
            /*������Ϣ����ʵ�����ݣ�����i�����Ǳ��⴫���ָ�뵼�·��ͽӿ�ʧ��*/
            ret = TA_SendMsg(&i, sizeof(i), FID_EMME,MME_NOT_SAVE_IDENTIFY);
            if( ret != XSUCC )
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"send MME_NOT_SAVE_IDENTIFY msg  failed!");
                XOS_MemFree(FID_TA, sctpData->pData);
                return XERROR;
            }
            g_TaCb.nasFlag = XFALSE;
        }
    }

    return XSUCC;
}

/************************************************************************
������:TA_TsProc
����: ����TS���͵�����
����:pMsg   -NTL���͵���Ϣָ��
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XS32 TA_TsProc(t_XOSCOMMHEAD *pMsg)
{
    t_TaMsgHead *head = NULL;
    t_TaTraceType *traceType = NULL;
    t_TaSynInfoRsp *synRsp = NULL;
    t_SCTPDATAIND *sctpData = NULL;
    t_SCTPDATAREQ dataReq;
    XS32 ret = 0;
    XS32 i = 0;
    XU16 tsCmd = 0;
    
    if(NULL == pMsg)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"input param error!");
        return XERROR;
    }

    sctpData = (t_SCTPDATAIND*)pMsg->message;
    if(NULL == sctpData)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"no data in msg!");
        return XERROR;
    }
    if(TA_LINK_HANDLE != (XPOINT)sctpData->appHandle )
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"apphandle is error!");
        return XERROR;
    }
    head = (t_TaMsgHead *)sctpData->pData;
    tsCmd = XOS_INET_NTOH_U16(head->usCmd);
    XOS_Trace(MD(FID_TA,PL_ERR),"ts cmd:%x",tsCmd);
    switch(tsCmd)
    {
        case TA_CMD_GET_IMSI_REQ:
            XOS_Trace(MD(FID_TA,PL_INFO),"TA_CMD_GET_IMSI_REQ!");
            if(TA_NE_EMME != g_TaCb.localInfo.neType)
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"get TA_CMD_GET_IMSI_REQ msg from ts,but this is not EMME!");
                break;
            }
            /*TS���͵�����IMSI����Ϣ��ֱ��ת����eMME*/
            ret = TA_SendMsg(sctpData->pData, sctpData->dataLenth, FID_EMME, MME_IMSI_REQ);
            if(ret != XSUCC)
            {
                XOS_MemFree(FID_TA, sctpData->pData);
                XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_GET_IMSI_REQ:send msg to eMME failed!");
                return XERROR;
            }
            break;
            
        case TA_CMD_GET_TEL_REQ:
            XOS_Trace(MD(FID_TA,PL_INFO),"TA_CMD_GET_TEL_REQ!");
            if(TA_NE_EMME != g_TaCb.localInfo.neType)
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"get TA_CMD_GET_TEL_REQ msg from ts,but this is not EMME!");
                break;
            }

            /*TS���͵�����绰�������Ϣ��ֱ��ת����eMME*/
            ret = TA_SendMsg(sctpData->pData, sctpData->dataLenth, FID_EMME, MME_TEL_REQ);
            if(ret != XSUCC)
            {
                XOS_MemFree(FID_TA, sctpData->pData);
                XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_GET_TEL_REQ:send msg to eMME failed!");
                return XERROR;
            }
            break;
            
        case TA_CMD_START_TRACE:
            XOS_Trace(MD(FID_TA,PL_INFO),"TA_CMD_START_TRACE!");
            XOS_MutexLock(&(g_TaCb.mutex));
            if(g_TaCb.missionNum >= TA_MISSION_NUM)
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_START_TRACE:TA is full!");
                XOS_MutexUnlock(&(g_TaCb.mutex));
                XOS_MemFree(FID_TA, sctpData->pData);
                return XERROR;
            }
            traceType = (t_TaTraceType *)sctpData->pData;
            /*����ǿ�ʼ�û����ٵ���Ϣ*/
            if(TRACE_TYPE_USER == traceType->ucTraceType)
            {
                TA_StoreUserMission(sctpData);
            }
            /*����ǿ�ʼ�ӿڸ��ٵ���Ϣ*/
            else if(TRACE_TYPE_IF == traceType->ucTraceType)
            {
                TA_StoreInterfaceMission(sctpData);
            }

            /*���й�������ʱ����NAS����Ϣ�����Ȩ��Ϣ*/
            if(TA_NE_EMME == g_TaCb.localInfo.neType)
            {
                if(g_TaCb.missionNum > 0 && XFALSE == g_TaCb.nasFlag)
                {
                    XOS_Trace(MD(FID_TA,PL_INFO),"send msg to NAS,to save identify msg");
                    /*������Ϣ����ʵ�����ݣ�����i�����Ǳ��⴫���ָ�뵼�·��ͽӿ�ʧ��*/
                    ret = TA_SendMsg(&i, sizeof(i), FID_EMME,MME_SAVE_IDENTIFY);
                    if( ret != XSUCC )
                    {
                        XOS_MutexUnlock(&(g_TaCb.mutex));
                        XOS_Trace(MD(FID_TA,PL_ERR),"TA_TsProc: send MME_SAVE_IDENTIFY msg  failed!");
                        XOS_MemFree(FID_TA, sctpData->pData);
                        return XERROR;
                    }
                    g_TaCb.nasFlag = XTRUE;
                }
            }
            XOS_MutexUnlock(&(g_TaCb.mutex));
            break;
            
        case TA_CMD_STOP_TRACE:
            XOS_Trace(MD(FID_TA,PL_INFO),"TA_CMD_STOP_TRACE!");
            XOS_MutexLock(&(g_TaCb.mutex));
            TA_StopTrace(sctpData);
            XOS_MutexUnlock(&(g_TaCb.mutex));
            break;
            
        case TA_CMD_SYN_INFO_REQ:
            /*TS���͵���Ԫ��Ϣͬ������*/
            XOS_Trace(MD(FID_TA,PL_INFO),"TA_CMD_SYN_INFO_REQ!");
            synRsp = (t_TaSynInfoRsp*)XOS_MemMalloc(FID_TA, sizeof(t_TaSynInfoRsp));

            synRsp->head.usBeginFlag = XOS_NtoHs(TSDATA_BEGIN_FLAG);
            synRsp->head.usCmd = XOS_NtoHs(TA_CMD_SYN_INFO_RSP);
            synRsp->head.usBodyLen = XOS_NtoHs(sizeof(t_TaSynInfoReqData));
            synRsp->head.ulSerial = XOS_NtoHs(head->ulSerial);
            
            synRsp->data.usNeType = XOS_NtoHs(g_TaCb.localInfo.neType);
            synRsp->data.usNeId = XOS_NtoHs(g_TaCb.localInfo.neID);
            synRsp->data.usLogicId = XOS_NtoHs(g_TaCb.localInfo.processID);
            synRsp->data.usSlotNum = XOS_NtoHs(g_TaCb.localInfo.slotID);

            synRsp->tail.usEndFlag = XOS_NtoHs(TSDATA_END_FLAG);

            XOS_MemSet(&dataReq, 0, sizeof(t_SCTPDATAREQ));
            dataReq.linkHandle = g_TaCb.linkHandle;
            dataReq.msgLenth = sizeof(t_TaSynInfoRsp);
            dataReq.pData = (XCHAR *)synRsp;

            ret = TA_SendMsg(&dataReq, sizeof(t_SCTPDATAREQ), FID_NTL, eSendData);
            if(ret != XSUCC)
            {
                XOS_MemFree(FID_TA, synRsp);
                XOS_MemFree(FID_TA, sctpData->pData);
                XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_SYN_INFO_REQ:send msg to NTL failed!");
            }

            break;
        default:
            XOS_Trace(MD(FID_TA,PL_ERR),"ts cmd is:%d!",tsCmd);
            break;
            
    }
    if(NULL != sctpData->pData)
    {
        XOS_MemFree(FID_TA, sctpData->pData);
    }
    return XSUCC;
}

/************************************************************************
������:TA_StopProc
����: ����NTL�ϱ��Ķ�����Ϣ
����:pMsg   -NTL���͵���Ϣָ��
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XS32 TA_StopProc(t_XOSCOMMHEAD *pMsg)
{
    t_LINKCLOSEIND *closeInd = NULL;
    t_PARA timerPara;
    t_BACKPARA backPara;

    if ( NULL == pMsg )
    {
        XOS_Trace(MD(FID_TA, PL_ERR), "input parameter is null!" );
        return XERROR;
    }
    
    closeInd = (t_LINKCLOSEIND*)(pMsg->message);
    if ( g_TaCb.userHandle != closeInd->appHandle )
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"error appHandle:%x",closeInd->appHandle);
        return XERROR;
    }
    XOS_Trace(MD(FID_TA, PL_INFO), "link closed:%d!",closeInd->closeReason);

    XOS_MutexLock(&(g_TaCb.mutex));
    g_TaCb.tsFlag = XFALSE;
    XOS_MutexUnlock(&(g_TaCb.mutex));

    /*��·�˿���NTL��ʼ�ͻ���������TA������ʱ�������·״̬*/
    timerPara.fid  = FID_TA;
    timerPara.len  = TA_CHECK_INTERVAL;
    timerPara.mode = TIMER_TYPE_LOOP;
    timerPara.pre  = TIMER_PRE_LOW;
    backPara.para1 = TA_CHECK_TIMER;

    if(XSUCC != XOS_TimerStart(&ta_timer, &timerPara, &backPara))
    {
        XOS_Trace(MD(FID_TA, PL_ERR), "start timer failed!");
    }

    return XSUCC;
}

/************************************************************************
������:TA_DeleteAllMission
����: ɾ�����й�������
����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XS32 TA_DeleteAllMission()
{
    XS32 i = 0;
    XS32 ret = 0;
    t_TaMission *mission = NULL;

    for(i  = 0;i < TA_MISSION_NUM;i++)
    {
        if(g_TaCb.taCb[i].flag)
        {
            /*ɾ���û����ƿ��µĹ�������*/
            mission = g_TaCb.taCb[i].mission;
            while(mission)
            {
                g_TaCb.taCb[i].mission = mission->next;
                XOS_MemFree(FID_TA,mission);
                mission = g_TaCb.taCb[i].mission;
            }

            /*����û����ƿ�*/
            XOS_MemSet(&(g_TaCb.taCb[i]), 0, sizeof(t_TaBlock));
        }
    }

    g_TaCb.missionNum = 0;

    /*��û�й�������ʱ����NAS����Ϣֹͣ�����Ȩ��Ϣ*/
    if(TA_NE_EMME == g_TaCb.localInfo.neType)
    {
        if(XTRUE == g_TaCb.nasFlag)
        {
            /*������Ϣ����ʵ�����ݣ�����i�����Ǳ��⴫���ָ�뵼�·��ͽӿ�ʧ��*/
            ret = TA_SendMsg(&i, sizeof(i), FID_EMME,MME_NOT_SAVE_IDENTIFY);
            if( ret != XSUCC )
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"send MME_NOT_SAVE_IDENTIFY msg  failed!");
                return XERROR;
            }
            g_TaCb.nasFlag = XFALSE;
        }
    }

    return XSUCC;
}

/************************************************************************
������:TA_Init
����: ��ʼ��trace agentģ��
����:
���:
����:
˵��: ע�ᵽģ�������
************************************************************************/
XS8 TA_Init(XVOID *p1, XVOID *p2)
{
    XS32 ret = 0;
    XU32 taNetask = 0;
    t_LINKINIT linkInit;
    XS8 taIP1[TA_IP_LEN] = {0};
    XS8 taIP2[TA_IP_LEN] = {0};
    XS8 tsIP1[TA_IP_LEN] = {0};
    XS8 tsIP2[TA_IP_LEN] = {0};
    XS32 neType = 0;
    XS32 neID = 0;
    XS32 processID = 0;
    XS32 slotID = 0;
    XS32 shelfID = 0;
    XS32 tsHighSlot = 0;
    XS32 tsLowSlot = 0;
    XOS_Trace(MD(FID_ROOT, PL_DBG), "begin");

    /*��ʼ��ȫ�ֱ���*/
    XOS_MemSet(&g_TaCb, 0, sizeof(g_TaCb));

    ret = XOS_MutexCreate(&g_TaCb.mutex);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"init semaphore failed!");
        return XERROR;
    }

    /*ע��������*/
    TA_regCli();
    
#ifdef XOS_LINUX
    /*������Ԫ��Ϣ�����˶˿ڣ�TS�Ĳ�λ�ŵ���Ϣ*/
    neType = XOS_GetNeTypeId();
    if(neType < TA_NE_TYPE_MIN || neType >TA_NE_TYPE_MAX)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"get neType failed:%d!",neType);
        return XSUCC;
    }
    g_TaCb.localInfo.neType = neType;

    neID = XOS_GetNeId();
    if(neID < TA_NE_ID_MIN || neID > TA_NE_ID_MAX)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"get neID failed:%d!",neID);
        return XSUCC;
    }
    g_TaCb.localInfo.neID = neID;

    processID = XOS_GetLogicPid();
    if(processID < TA_PROCESS_ID_MIN || processID > TA_PROCESS_ID_MAX)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"get processID failed:%d!",processID);
        return XSUCC;
    }
    g_TaCb.localInfo.processID = processID;

    slotID = XOS_GetSlotNum();
    if(slotID < TA_SLOT_ID_MIN || slotID > TA_SLOT_ID_MAX)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"get slotID failed:%d!",slotID);
        return XSUCC;
    }
    g_TaCb.localInfo.slotID = slotID;

    shelfID = XOS_GetShelfNum();
    if(shelfID < TA_SHELF_ID_MIN || shelfID > TA_SHELF_ID_MAX)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"get shelfID failed:%d!",shelfID);
        return XSUCC;
    }
    g_TaCb.localInfo.shelfID = shelfID;
        
    tsHighSlot = XOS_GetTsHighSlot();
    if(tsHighSlot < TA_SLOT_ID_MIN || tsHighSlot > TA_SLOT_ID_MAX)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"get tsHighSlot failed:%d!",tsHighSlot);
        return XSUCC;
    }
    g_TaCb.tsHighSlot = tsHighSlot;
    
    tsLowSlot = XOS_GetTsLowSlot();
    /*����ĵͲ�λ��Ϊ0ʱ����ʾTS���ڵ���ģʽ*/
    if(tsLowSlot < 0 || tsLowSlot > TA_SLOT_ID_MAX)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"get tsLowSlot failed:%d!",tsLowSlot);
        return XSUCC;
    }
    g_TaCb.tsLowSlot = tsLowSlot;

    g_TaCb.userHandle = (HAPPUSER)TA_LINK_HANDLE;

    ret = XOS_TimerReg(FID_NTL, 500, 1, 0);
    if( ret != XSUCC)
    {
        return XERROR;
    }

    /*TAʹ��F1��F2����������IP��TS����������ip���ݿ�ţ���λ�ţ�ƽ��ż������*/
    /*172.16.[(���-1)*10+ƽ���].[��λ��]*/
    XOS_Sprintf(taIP1, TA_IP_LEN, TA_IP_PREFIX,(g_TaCb.localInfo.shelfID -1)*10 + TA_F1IP_INDEX,slotID);
    XOS_StrtoIp(taIP1, &g_TaCb.taIP1);
    
    XOS_Sprintf(taIP2, TA_IP_LEN, TA_IP_PREFIX,(g_TaCb.localInfo.shelfID - 1)*10 + TA_F2IP_INDEX,slotID);
    XOS_StrtoIp(taIP2, &g_TaCb.taIP2);

    g_TaCb.port = TA_PORT_BASE + neID * TA_PORT_OFFSET + processID;

    /*TS����Ϊ����ģʽ*/
    if( 0 == tsLowSlot )
    {
        XOS_Trace(MD(FID_TA, PL_INFO), "TS is single mode!");
        XOS_Sprintf(tsIP1, TA_IP_LEN, TS_F1IP_PREFIX,TS_IP_BASE+tsHighSlot);
        XOS_StrtoIp(tsIP1, &g_TaCb.tsIP1);

        XOS_Sprintf(tsIP2, TA_IP_LEN, TS_F2IP_PREFIX,TS_IP_BASE+tsHighSlot);
        XOS_StrtoIp(tsIP2, &g_TaCb.tsIP2);
    }
    /*TS����ΪHAģʽ*/
    else
    {
        XOS_Trace(MD(FID_TA, PL_INFO ), "TS is HA mode!");
        XOS_Sprintf(tsIP1, TA_IP_LEN, TS_F1IP_PREFIX,TS_IP_BASE+tsLowSlot);
        XOS_StrtoIp(tsIP1, &g_TaCb.tsIP1);

        XOS_Sprintf(tsIP2, TA_IP_LEN, TS_F2IP_PREFIX,TS_IP_BASE+tsHighSlot);
        XOS_StrtoIp(tsIP2, &g_TaCb.tsIP2);
    }
    /*������TS֮���SCTP��·��������·��ʼ����Ϣ��NTL*/
    linkInit.appHandle = (HAPPUSER)TA_LINK_HANDLE;
    linkInit.linkType = eSCTPClient;
    linkInit.ctrlFlag = eNullLinkCtrl;

    /*������Ϣ��NTL������·��ʼ��*/
    ret = TA_SendMsg(&linkInit, sizeof(linkInit), FID_NTL, eLinkInit);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_TA, PL_ERR ), "send msg to ntl error,start sctp client failed!");
        return XERROR;
    }
#endif
    return XSUCC;
}

/************************************************************************
������:TA_msgProc
����:  trace agent ģ����Ϣ���������
����:  pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: ����Ϣ��������trace agent��������Ϣ���
��Ϣ���ڴ˺�������ķ�Χ��
************************************************************************/
XS8 TA_msgProc(XVOID* pMsgP, XVOID* para)
{
    t_XOSCOMMHEAD *pMsg = NULL;
    t_SCTPDATAREQ dataReq;

    if(NULL == pMsgP)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"msg is NULL!");
        return XERROR;
    }
    if(NULL == ((t_XOSCOMMHEAD*)pMsgP)->message)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"msg has no data");
        return XERROR;
    }

    pMsg = (t_XOSCOMMHEAD*)pMsgP;

    if(FID_NTL == pMsg->datasrc.FID)
    {
        /*����NTL�ϱ�������*/
        switch (pMsg->msgID)
        {
            case eSctpInitAck:
                if(XSUCC != TA_InitAckProc(pMsg))
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"eInitAck:msg InitAck Proc failed!" );
                    return XERROR;
                }
                break;

            case eSctpStartAck:
                if(XSUCC != TA_StartAckProc(pMsg))
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"eStartAck:msg startAck Proc failed!" );
                    return XERROR;
                }
                break;

            case eSctpDataInd:
                if(XSUCC != TA_TsProc(pMsg))
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"eDataInd:TA_TsProc failed!" );
                    return XERROR;
                }
                break;

            case eSctpStopInd:
                if(XSUCC != TA_StopProc(pMsg))
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"eSctpStopInd:TA_StopProc failed!" );
                    return XERROR;
                }
                break;
                
            default:
                break;
        }
    }
    else if(FID_EMME == pMsg->datasrc.FID)
    {
        /*����EMME���͵�����*/
        switch (pMsg->msgID)
        {
            case MME_IMSI_RSP:
            case MME_TEL_RSP:
                XOS_MemSet(&dataReq, 0, sizeof(t_SCTPDATAREQ));
                
                dataReq.linkHandle = g_TaCb.linkHandle;
                dataReq.msgLenth = pMsg->length;
                
                dataReq.pData = (XCHAR *)XOS_MemMalloc(FID_TA, pMsg->length);
                if(XNULLP == dataReq.pData)
                {
                    XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",pMsg->length);
                    return XERROR;
                }
                /*eMME���͵ĵ绰�����IMSI���ݣ�ֱ��ת����TS*/
                XOS_MemCpy(dataReq.pData ,pMsg->message,pMsg->length);

                if(XSUCC != TA_SendMsg(&dataReq, sizeof(t_SCTPDATAREQ), FID_NTL, eSendData))
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"MME_IMSI_RSP:send imsi rsp to ts failed!" );
                    XOS_MemFree(FID_TA, dataReq.pData);
                    return XERROR;
                }
                break;

            default:
                break;
        }
    }
    else
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"MME_TEL_RSP:msg from unknown src fid:%d!",pMsg->datasrc.FID);
    }

    return XSUCC;
}

/************************************************************************
������:TA_timerProc
����:  trace agent ģ�鶨ʱ����Ϣ���������
����:  pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS8 TA_timerProc( t_BACKPARA* pParam)
{
    if(NULL == pParam)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"timer PTIMER is null ,bad input param!");
        return XERROR;
    }

    if(TA_CHECK_TIMER == pParam->para1)
    {
        XOS_MutexLock(&(g_TaCb.mutex));
        if(XTRUE == g_TaCb.tsFlag)
        {
            /*��·�Ѿ������ɹ�����ʱ��������*/
            g_TaCb.expireTimes = 0;
        }
        else
        {
            /*��·��Ȼ���ڶ���״̬����ʱ������1*/
            g_TaCb.expireTimes++;
        }
        /*��ⳬʱ���������趨��������ֹͣ���й�������*/
        if(g_TaCb.expireTimes >= TA_CHECK_NUM)
        {
            TA_DeleteAllMission();
        }
        XOS_MutexUnlock(&(g_TaCb.mutex));
    }
    else
    {
        /*�������*/
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
������:XOS_TaInfect
����:  ҵ���Ⱦ�ӿ�
����:  userId ���û���ʶ
���:
����: ����TraceID
˵��: ҵ��ֱ��ʹ�÷��ص�TraceID,�����жϷ���ֵ�Ϸ���
************************************************************************/
XU32 XOS_TaInfect(const t_InfectId *userID)
{
    XU32 traceID = 0;
    XS32 i = 0;
    XS32 j = 0;

    /*û�и�������ֱ�ӷ���*/
    if(0 == g_TaCb.missionNum)
    {
        return traceID;
    }

    /*����ж�*/
    if(!userID)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "userID is NULL");
        return traceID;
    }

    if((0 == userID->idNum) && (0 == userID->imsiNum))
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "userID has no user");
        return traceID;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    /*ƥ���������*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        if(g_TaCb.taCb[i].flag)
        {
            /*����ƥ���û���ʶ�����ø����û��ľֲ�TraceID�������յ�TraceID*/
            for(j = 0;j < userID->idNum && j < TA_USER_NUM;j++)
            {
                if((g_TaCb.taCb[i].telLen > 0) && 
                    (!XOS_StrNcmp(userID->id[j], g_TaCb.taCb[i].tel, 
                    XOS_MAX(XOS_StrLen(userID->id[j]),g_TaCb.taCb[i].telLen))))
                {
                    traceID |= g_TaCb.taCb[i].traceID;
                }
            }
            for(j = 0;j < userID->imsiNum && j < TA_USER_NUM;j++)
            {
                if((g_TaCb.taCb[i].imsiLen > 0) && 
                    (!XOS_StrNcmp(userID->imsi[j], g_TaCb.taCb[i].imsi, 
                    XOS_MAX(XOS_StrLen(userID->imsi[j]),g_TaCb.taCb[i].imsiLen))))
                {
                    traceID |= g_TaCb.taCb[i].traceID;
                }
            }
        }
    }
    
    XOS_MutexUnlock(&(g_TaCb.mutex));

    return traceID;
}

/************************************************************************
������:XOS_TaTrace
����:  ҵ����Ϣ���˽ӿ�
����:  fid - ģ��fid
        direct - ��Ϣ����
        msg ����Ϣָ��
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XS32 XOS_TaTrace(XU32 fid,e_TaDirection direct,const t_XOSCOMMHEAD *msg)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*����������Ϣ��������*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*���͸�TS�����ݻ�����ָ��*/
    XU32 bufSize = 0;       /*���͸�TS�������ܳ���*/
    XS8 *offset = NULL;
    t_TaTraceCommonData *commonData = NULL;
    t_TaMsgHead *head = NULL;
    t_SCTPDATAREQ dataReq;
    XU32 msg_len = 0;
    t_XOSCOMMHEAD *xos_head = NULL;

    if(!g_TaCb.tsFlag)
    {
        /*��TSʧ��*/
        return XERROR;
    }

    if(!msg)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "msg is NULL");
        return XERROR;
    }

    if(0 == msg->traceID)
    {
        /*�󲿷���Ϣ��������٣�Ϊ��ߴ���Ч�ʣ�����û�д�ӡTrace*/
        return XSUCC;
    }

    /*û�и�������ֱ�ӷ���*/
    if(0 == g_TaCb.missionNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    g_ta_num++;
    /*�����û����ƿ飬��ѯ���ٴ�����Ϣ�Ĺ�������*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*�ж��û����ƿ��Ƿ�ʹ��*/
        if(g_TaCb.taCb[i].flag)
        {
            /*�ж��û����ƿ�ľֲ�traceID�Ƿ�ƥ��*/
            if((g_TaCb.taCb[i].traceID & msg->traceID) != 0)
            {
                mission = g_TaCb.taCb[i].mission;
                while(mission)
                {
                    /*���������������traceID��*/
                    if(TA_ISSET_BIT(msg->traceID,mission->idx) && TRACE_MSG_TYPE_CONTROL == mission->msgType)
                    {
                        XOS_StrNcpy(traceMd5[traceNum], mission->md5, MAX_MD5_LEN);
                        traceNum++;
                        
                        if(traceNum >= TA_MISSION_NUM)
                        {
                            /*��������ֻ��32�����е�����˵����������ܿ����ڴ汻��*/
                            XOS_MutexUnlock(&(g_TaCb.mutex));
                            XOS_Trace(FILI, FID_TA, PL_ERR, "error:mission num:%d exceeds %d",traceNum,TA_MISSION_NUM);
                            return XERROR;
                        }
                    }
                    mission = mission->next;
                }
            }
        }
    }
    XOS_MutexUnlock(&(g_TaCb.mutex));

    if(0 == traceNum)
    {
        XOS_Trace(FILI, FID_TA, PL_INFO, "no one trace this msg");
        return XSUCC;
    }

    /*TA���͸�TS�����trace���ݳ��Ȳ�����4096�ֽڣ�����ı��ص�*/
    msg_len = XOS_MIN(msg->length, TA_MAX_MSG_LEN);
    /*���͸�TS�ĸ�����Ϣ:����ͷ | MD5 | ���ݹ���ͷ | ƽ̨��Ϣ |β��ʶ*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_XOSCOMMHEAD) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }

    /*���췢�͸�TS��trace��Ϣ*/
    offset = (XS8 *)buf;
    /*��乫��ͷ*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*���MD5ֵ������*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*�������MD5ֵ*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*��乫������ͷ*/
    commonData = (t_TaTraceCommonData *)offset;
    commonData->ucProVer = g_interfaceVer;
    commonData->ucMsgDirect = direct;
    commonData->ucTraceType = TRACE_MSG_TYPE_CONTROL;
    commonData->ucNeType = XOS_HtoNs(g_TaCb.localInfo.neType);
    commonData->usNeId = XOS_HtoNs(g_TaCb.localInfo.neID);
    commonData->usLogicPid = XOS_HtoNs(g_TaCb.localInfo.processID);
    commonData->ulIPAddr = XOS_HtoNl(g_TaCb.taIP1);
    commonData->ulFID = XOS_HtoNl(fid);
    commonData->ulSeqNum = 0;
    commonData->ulTimeStamp = 0;

    offset += sizeof(t_TaTraceCommonData);
    
    /*�����Ϣ*/
    XOS_MemCpy(offset, msg, sizeof(t_XOSCOMMHEAD));
    xos_head = (t_XOSCOMMHEAD *)offset;
    xos_head->length = msg_len;
    offset += sizeof(t_XOSCOMMHEAD);
    XOS_MemCpy(offset, msg->message, msg_len);

    offset += msg_len;
    p16Data = (XU16 *)offset;

    /*���β��ʶ*/
    *p16Data = XOS_HtoNs(TSDATA_END_FLAG);

    XOS_MemSet(&dataReq, 0, sizeof(t_SCTPDATAREQ));
    dataReq.linkHandle = g_TaCb.linkHandle;
    dataReq.msgLenth = bufSize;
    dataReq.pData = (XCHAR *)buf;

    ret = TA_SendMsg(&dataReq, sizeof(t_SCTPDATAREQ), FID_NTL, eSendData);
    if(ret != XSUCC)
    {
        XOS_MemFree(FID_TA, buf);
        XOS_Trace(MD(FID_TA, PL_ERR ), "send msg to ntl error!");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:XOS_TaTrace
����:  ҵ����Ϣ���˽ӿ�
����:  srcFid - Դģ��fid
        dstFid - Ŀ��ģ��fid
        buf ����Ϣָ��
        len - ���ݳ���
        id - �û���ʶ
        type - �û���ʶ����
        direction - ��Ϣ����
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: ҵ��ģ��ӿ���Ϣ���˽ӿڣ���Ҫҵ���Լ�����Ҫ�ĵط�����
************************************************************************/
XS32 XOS_TaDataTrace(XU32 srcFid,XU32 dstFid,const XVOID *data,XU32 len,const XU8 *id,e_IdType type,e_TaDirection direction)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*����������Ϣ��������*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*���͸�TS�����ݻ�����ָ��*/
    XU32 bufSize = 0;       /*���͸�TS�������ܳ���*/
    XS8 *offset = NULL;
    t_TaTraceCommonData *commonData = NULL;
    t_TaMsgHead *head = NULL;
    t_XOSCOMMHEAD *xosMsg = NULL;
    t_SCTPDATAREQ dataReq;
    XBOOL traceFlag = XFALSE;
    XU32 msg_len = 0;

    if(!g_TaCb.tsFlag)
    {
        return XERROR;
    }

    if( NULL == data || NULL == id || type < e_TA_TEL || type > e_TA_IMSI)
    {
        XOS_Trace(FILI, FID_TA, PL_DBG, "arg is error ");
        return XERROR;
    }

    /*û�и�������ֱ�ӷ���*/
    if(0 == g_TaCb.missionNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    /*ƥ���������*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        if(g_TaCb.taCb[i].flag)
        {
            switch(type)
            {
                case e_TA_TEL:
                if(!XOS_StrNcmp(id, g_TaCb.taCb[i].tel, XOS_MAX(XOS_StrLen(id),g_TaCb.taCb[i].telLen)))
                {
                    traceFlag = XTRUE;
                }
                break;

                case e_TA_IMSI:
                if(!XOS_StrNcmp(id, g_TaCb.taCb[i].imsi, XOS_MAX(XOS_StrLen(id),g_TaCb.taCb[i].imsiLen)))
                {
                    traceFlag = XTRUE;
                }
                break;

                default:
                    break;
            }
        }
        if(traceFlag)
        {
            mission = g_TaCb.taCb[i].mission;
            while(mission)
            {
                if(mission->ifFlag)
                {
                    XOS_StrNcpy(traceMd5[traceNum], mission->md5, MAX_MD5_LEN);
                    traceNum++;
                    
                    if(traceNum >= TA_MISSION_NUM)
                    {
                        /*��������ֻ��32�����е�����˵����������ܿ����ڴ汻��*/
                        XOS_MutexUnlock(&(g_TaCb.mutex));
                        XOS_Trace(FILI, FID_TA, PL_ERR, "program error,num of mission exceeds %d",TA_MISSION_NUM);
                        return XERROR;
                    }
                }
                mission = mission->next;
            }
            break;
        }
    }
    XOS_MutexUnlock(&(g_TaCb.mutex));

    /*TA���͸�TS�����trace���ݳ��Ȳ�����4096�ֽڣ�����ı��ص�*/
    msg_len = XOS_MIN(len, TA_MAX_MSG_LEN);
    /*���͸�TS�ĸ�����Ϣ:����ͷ |MD5 | ���ݹ���ͷ | ��Ϣ |β��ʶ*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_XOSCOMMHEAD) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }
    offset = (XS8 *)buf;

    /*����ͷ*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*���MD5ֵ������*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*�������MD5ֵ*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*��乫������ͷ*/
    commonData = (t_TaTraceCommonData *)offset;
    commonData->ucProVer = g_interfaceVer;
    commonData->ucMsgDirect = direction;
    commonData->ucTraceType = TRACE_MSG_TYPE_CONTROL;
    commonData->ucNeType = XOS_HtoNs(g_TaCb.localInfo.neType);
    commonData->usNeId = XOS_HtoNs(g_TaCb.localInfo.neID);
    commonData->usLogicPid = XOS_HtoNs(g_TaCb.localInfo.processID);
    commonData->ulIPAddr = XOS_HtoNl(g_TaCb.taIP1);
    if(e_TA_SEND == direction)
    {
        commonData->ulFID = XOS_HtoNl(srcFid);
    }
    else
    {
        commonData->ulFID = XOS_HtoNl(dstFid);
    }
    commonData->ulSeqNum = 0;
    commonData->ulTimeStamp = 0;

    offset += sizeof(t_TaTraceCommonData);

    /*��Ϣ*/
    xosMsg = (t_XOSCOMMHEAD *)offset;
    xosMsg->datasrc.PID = XOS_GetLocalPID();
    xosMsg->datasrc.FID = srcFid;
    xosMsg->datasrc.FsmId = 0;
    xosMsg->datasrc.PID = XOS_GetLocalPID();
    xosMsg->datasrc.FID = dstFid;
    xosMsg->datasrc.FsmId = 0;

    xosMsg->msgID = 0;
    xosMsg->subID = 0;
    xosMsg->prio = eNormalMsgPrio;
    xosMsg->traceID = 0;
    xosMsg->logID = 0;
    xosMsg->length = msg_len;
    xosMsg->message = NULL;

    offset += sizeof(t_XOSCOMMHEAD);
    /*�����Ϣ*/
    XOS_MemCpy(offset, data, msg_len);

    offset += msg_len;

    /*β��ʶ*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(TSDATA_END_FLAG);

    XOS_MemSet(&dataReq, 0, sizeof(t_SCTPDATAREQ));
    dataReq.linkHandle = g_TaCb.linkHandle;
    dataReq.msgLenth = bufSize;
    dataReq.pData = (XCHAR *)buf;

    ret = TA_SendMsg(&dataReq, sizeof(t_SCTPDATAREQ), FID_NTL, eSendData);
    if(ret != XSUCC)
    {
        XOS_MemFree(FID_TA, buf);
        XOS_Trace(MD(FID_TA, PL_ERR ), "send msg to ntl error!");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:XOS_TmgFilter
����:  TMGģ��ý�����ݹ��˽ӿ�
����:  fid - ģ��fid
        buf ��ý�����ݻ�����
        len - ���ݳ���
        msgType - ����ý���������ͣ���Ƶ����Ƶ
        traceID - ����ID
���:
����:  �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: TMGģ��ר��
************************************************************************/
XS32 XOS_TmgFilter(XU32 fid,const XVOID *data,XU32 len,e_TRACE_MSG_TYPE msgType,XU32 traceID)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*����������Ϣ��������*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*���͸�TS�����ݻ�����ָ��*/
    XU32 bufSize = 0;       /*���͸�TS�������ܳ���*/
    XS8 *offset = NULL;
    t_TaTraceCommonData *commonData = NULL;
    t_TaMsgHead *head = NULL;
    t_SCTPDATAREQ dataReq;
    XU32 msg_len = 0;

    if(!g_TaCb.tsFlag)
    {
        /*��TSʧ��*/
        return XERROR;
    }

    if(!data)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "buf is NULL");
        return XERROR;
    }

    if(0 == traceID)
    {
        /*�󲿷���Ϣ��������٣�Ϊ��ߴ���Ч�ʣ�����û�д�ӡTrace*/
        return XSUCC;
    }

    /*û�и�������ֱ�ӷ���*/
    if(0 == g_TaCb.dataNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    g_ta_tmg_num++;
    /*�����û����ƿ飬��ѯ���ٴ�����Ϣ�Ĺ�������*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*�ж��û����ƿ��Ƿ�ʹ��*/
        if(g_TaCb.taCb[i].flag)
        {
            /*�ж��û����ƿ�ľֲ�traceID�Ƿ�ƥ��*/
            if((g_TaCb.taCb[i].traceID & traceID) != 0)
            {
                mission = g_TaCb.taCb[i].mission;
                while(mission)
                {
                    /*���������������traceID��*/
                    if(TA_ISSET_BIT(traceID,mission->idx) && msgType == mission->msgType)
                    {
                        XOS_StrNcpy(traceMd5[traceNum], mission->md5, MAX_MD5_LEN);
                        traceNum++;
                        
                        if(traceNum >= TA_MISSION_NUM)
                        {
                            /*��������ֻ��32�����е�����˵����������ܿ����ڴ汻��*/
                            XOS_MutexUnlock(&(g_TaCb.mutex));
                            XOS_Trace(FILI, FID_TA, PL_ERR, "error:mission num:%d exceeds %d",traceNum,TA_MISSION_NUM);
                            return XERROR;
                        }
                    }
                    mission = mission->next;
                }
            }
        }
    }
    XOS_MutexUnlock(&(g_TaCb.mutex));

    if(0 == traceNum)
    {
        XOS_Trace(FILI, FID_TA, PL_INFO, "no one trace this msg");
        return XSUCC;
    }

    /*TA���͸�TS�����trace���ݳ��Ȳ�����4096�ֽڣ�����ı��ص�*/
    msg_len = XOS_MIN(len, TA_MAX_MSG_LEN);
    /*���͸�TS�ĸ�����Ϣ:����ͷ | MD5 | ���ݹ���ͷ | ý������ |β��ʶ*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }

    /*���췢�͸�TS��trace��Ϣ*/
    offset = (XS8 *)buf;
    /*��乫��ͷ*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*���MD5ֵ������*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*�������MD5ֵ*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*��乫������ͷ*/
    commonData = (t_TaTraceCommonData *)offset;
    commonData->ucProVer = g_interfaceVer;
    commonData->ucMsgDirect = e_TA_SEND;
    commonData->ucTraceType = msgType;
    commonData->ucNeType = XOS_HtoNs(g_TaCb.localInfo.neType);
    commonData->usNeId = XOS_HtoNs(g_TaCb.localInfo.neID);
    commonData->usLogicPid = XOS_HtoNs(g_TaCb.localInfo.processID);
    commonData->ulIPAddr = XOS_HtoNl(g_TaCb.taIP1);
    commonData->ulFID = XOS_HtoNl(fid);
    commonData->ulSeqNum = 0;
    commonData->ulTimeStamp = 0;

    offset += sizeof(t_TaTraceCommonData);
    
    /*�����Ϣ*/
    XOS_MemCpy(offset, data, msg_len);
    offset += msg_len;

    /*���β��ʶ*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(TSDATA_END_FLAG);

    XOS_MemSet(&dataReq, 0, sizeof(t_SCTPDATAREQ));
    dataReq.linkHandle = g_TaCb.linkHandle;
    dataReq.msgLenth = bufSize;
    dataReq.pData = (XCHAR *)buf;

    ret = TA_SendMsg(&dataReq, sizeof(t_SCTPDATAREQ), FID_NTL, eSendData);
    if(ret != XSUCC)
    {
        XOS_MemFree(FID_TA, buf);
        XOS_Trace(MD(FID_TA, PL_ERR ), "send msg to ntl error!");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:XOS_TaRegDel
����:  ҵ����ã�ע��ɾ���Լ������TraceID�Ļص������ӿ�
����:  fid ��ҵ��ģ���Fid
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: ��Щҵ����Ҫ����TraceID,��Щģ�����Ҫע��ص���ɾ��TraceID
************************************************************************/
XS32 XOS_TaRegDel(XU32 fid)
{
    if(fid > FID_MAX)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "invalid fid:%d",fid);
        return XERROR;
    }

    if(g_TaCb.regFids.num >= TA_REG_NUM)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "TA is Full");
        return XERROR;
    }

    g_TaCb.regFids.fid[g_TaCb.regFids.num] = fid;
    g_TaCb.regFids.num++;
    
    return XSUCC;
}

/************************************************************************
������:XOS_TaInterface
����:  ��Ԫ�ӿ���Ϣ���˽ӿ�
����:   fid  - ģ��fid
        type ����Ԫ�ӿ�����
        groupID - ��ϢԴ��ʶ����Դ��ַʱʹ��
        linkID - ��ϢĿ�ı�ʶ����Ŀ�ĵ�ַʱʹ��
        direct - ���ͻ���յı�ʶ
        addr - ԴĿ��IP��ַ�Ͷ˿�
        data - ���ݻ�����
        len - ���ݳ���
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: ����ӿ����˵���Ԫ��һ��һ�ģ���Դ��Ŀ�ı�ʶ����0
************************************************************************/
XS32 XOS_TaInterface(XU32 fid,e_IF_TYPE type,XU16 groupID,XU16 linkID,e_TaDirection direct,t_TaAddress *addr,const XVOID *data,XU16 len)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*����������Ϣ��������*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*���͸�TS�����ݻ�����ָ��*/
    XU32 bufSize = 0;       /*���͸�TS�������ܳ���*/
    XS8 *offset = NULL;
    t_TaMsgHead *head = NULL;
    t_TaTraceCommonData *commonData = NULL;
    t_TaInterfaceRsp *interRsp = NULL;
    t_SCTPDATAREQ dataReq;
    XU32 msg_len = 0;

    if(!g_TaCb.tsFlag)
    {
        return XERROR;
    }

    if(NULL == data|| type > IF_TYPE_SGI)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "arg is error");
        return XERROR;
    }

    /*û�и�������ֱ�ӷ���*/
    if(0 == g_TaCb.missionNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    /*�����û����ƿ飬��ѯ���ٴ�����Ϣ�Ĺ�������*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*�ж��û����ƿ��Ƿ�ʹ��*/
        if(g_TaCb.taCb[i].flag)
        {
            /*�жϽӿ��Ƿ񱻸���*/
            if(g_TaCb.taCb[i].ifType == type)
            {
                mission = g_TaCb.taCb[i].mission;
                while(mission)
                {
                    if((groupID == mission->groupID && linkID == mission->linkID)||
                        (groupID == mission->groupID && TA_INVALID_LINK == mission->linkID)||
                        (TA_INVALID_LINK == mission->groupID && linkID == mission->linkID))
                    {
                        g_ta_if_num++;
                        XOS_StrNcpy(traceMd5[traceNum], mission->md5, MAX_MD5_LEN);
                        traceNum++;
                        
                        if(traceNum >= TA_MISSION_NUM)
                        {
                            /*��������ֻ��32�����е�����˵����������ܿ����ڴ汻��*/
                            XOS_MutexUnlock(&(g_TaCb.mutex));
                            XOS_Trace(FILI, FID_TA, PL_ERR, "error,mission num:%d exceeds %d",traceNum,TA_MISSION_NUM);
                            return XERROR;
                        }
                    }
                    mission = mission->next;
                }
                break;
            }
        }
    }

    XOS_MutexUnlock(&(g_TaCb.mutex));
    if(0 == traceNum)
    {
        XOS_Trace(FILI, FID_TA, PL_INFO, "no one trace this msg");
        return XSUCC;
    }

    /*TA���͸�TS�����trace���ݳ��Ȳ�����4096�ֽڣ�����ı��ص�*/
    msg_len = XOS_MIN(len, TA_MAX_MSG_LEN);
    /*���͸�TS�ĸ�����Ϣ:����ͷ |MD5 | ���ݹ���ͷ | �ӿڸ�����Ӧ |β��ʶ*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_TaInterfaceRsp) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }
    offset = (XS8 *)buf;
    /*����ͷ*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*���MD5ֵ������*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*�������MD5ֵ*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*��乫������ͷ*/
    commonData = (t_TaTraceCommonData *)offset;
    commonData->ucProVer = g_interfaceVer;
    commonData->ucMsgDirect = direct;
    commonData->ucTraceType = TRACE_TYPE_IF;
    commonData->ucNeType = XOS_HtoNs(g_TaCb.localInfo.neType);
    commonData->usNeId = XOS_HtoNs(g_TaCb.localInfo.neID);
    commonData->usLogicPid = XOS_HtoNs(g_TaCb.localInfo.processID);
    commonData->ulIPAddr = XOS_HtoNl(g_TaCb.taIP1);
    commonData->ulFID = XOS_HtoNl(fid);

    offset += sizeof(t_TaTraceCommonData);
    
    /*�����Ϣ*/
    interRsp = (t_TaInterfaceRsp *)offset;

    interRsp->ifType = type;
    interRsp->groupID = groupID;
    interRsp->linkID = linkID;
    interRsp->len = msg_len;
    offset += sizeof(t_TaInterfaceRsp);

    /*����³��Ľӿ���Ϣ����*/
    XOS_MemCpy(offset, data, msg_len);

    offset += msg_len;

    /*���β��ʶ*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(TSDATA_END_FLAG);

    XOS_MemSet(&dataReq, 0, sizeof(t_SCTPDATAREQ));
    dataReq.linkHandle = g_TaCb.linkHandle;
    dataReq.msgLenth = bufSize;
    dataReq.pData = (XCHAR *)buf;

    ret = TA_SendMsg(&dataReq, sizeof(t_SCTPDATAREQ), FID_NTL, eSendData);
    if(ret != XSUCC)
    {
        XOS_MemFree(FID_TA, buf);
        XOS_Trace(MD(FID_TA, PL_ERR ), "send msg to ntl error!");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
������:XOS_TaIdTrace
����:  ������Ϣ���˽ӿ�
����:  msg ��������Ϣָ��
        id - ������Ϣ��Ӧ���û���ʶ
        type - �û���ʶ����:�绰�����IMSI
        direction - ��Ϣ����
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: NAS��S1AP֮���Ȩ������Ϣ��û�е绰�����IMSI��ֻ�ܻ�����Ϣ���ȼ�Ȩ��ɺ�
���ô˽ӿڣ������ȡ���û���ʶ���й���
************************************************************************/
XS32 XOS_TaIdTrace(const t_XOSCOMMHEAD *msg,const XU8 *id,e_IdType type,e_TaDirection direction)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*����������Ϣ��������*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*���͸�TS�����ݻ�����ָ��*/
    XU32 bufSize = 0;       /*���͸�TS�������ܳ���*/
    XS8 *offset = NULL;
    t_TaTraceCommonData *commonData = NULL;
    t_TaMsgHead *head = NULL;
    t_SCTPDATAREQ dataReq;
    XBOOL traceFlag = XFALSE;
    XU32 msg_len = 0;
    t_XOSCOMMHEAD *xos_head = NULL;

    if(!g_TaCb.tsFlag)
    {
        return XERROR;
    }

    if(NULL == msg || NULL == id || type < e_TA_TEL || type > e_TA_IMSI)
    {
        XOS_Trace(FILI, FID_TA, PL_DBG, "arg is error ");
        return XERROR;
    }

    /*û�и�������ֱ�ӷ���*/
    if(0 == g_TaCb.missionNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    /*ƥ���������*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        if(g_TaCb.taCb[i].flag)
        {
            switch(type)
            {
                case e_TA_TEL:
                if(!XOS_StrNcmp(id, g_TaCb.taCb[i].tel, XOS_MAX(XOS_StrLen(id),g_TaCb.taCb[i].telLen)))
                {
                    traceFlag = XTRUE;
                }
                break;

                case e_TA_IMSI:
                if(!XOS_StrNcmp(id, g_TaCb.taCb[i].imsi, XOS_MAX(XOS_StrLen(id),g_TaCb.taCb[i].imsiLen)))
                {
                    traceFlag = XTRUE;
                }
                break;

                default:
                    break;
            }
        }
        /*�û���ʶ���ϸ�������*/
        if(traceFlag)
        {
            mission = g_TaCb.taCb[i].mission;
            while(mission)
            {
                if(TRACE_MSG_TYPE_CONTROL == mission->msgType)
                {
                    XOS_StrNcpy(traceMd5[traceNum], mission->md5, MAX_MD5_LEN);
                    traceNum++;
                    
                    if(traceNum >= TA_MISSION_NUM)
                    {
                        /*��������ֻ��32�����е�����˵����������ܿ����ڴ汻��*/
                        XOS_MutexUnlock(&(g_TaCb.mutex));
                        XOS_Trace(FILI, FID_TA, PL_ERR, "error,mission num:%d exceeds %d",traceNum,TA_MISSION_NUM);
                        return XERROR;
                    }
                }
                mission = mission->next;
            }
            break;
        }
    }
    XOS_MutexUnlock(&(g_TaCb.mutex));

    /*TA���͸�TS�����trace���ݳ��Ȳ�����4096�ֽڣ�����ı��ص�*/
    msg_len = XOS_MIN(msg->length, TA_MAX_MSG_LEN);
    /*���͸�TS�ĸ�����Ϣ:����ͷ |MD5 | ���ݹ���ͷ | ƽ̨��Ϣ |β��ʶ*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_XOSCOMMHEAD) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }
    offset = (XS8 *)buf;

    /*����ͷ*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*���MD5ֵ������*/
    XOS_MemCpy(offset, &traceNum, sizeof(traceNum));
    offset += sizeof(traceNum);

    /*�������MD5ֵ*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*��乫������ͷ*/
    commonData = (t_TaTraceCommonData *)offset;
    commonData->ucProVer = g_interfaceVer;
    commonData->ucMsgDirect = direction;
    commonData->ucTraceType = TRACE_TYPE_USER;
    commonData->ucNeType = g_TaCb.localInfo.neType;
    commonData->usNeId = g_TaCb.localInfo.neID;
    commonData->usLogicPid = g_TaCb.localInfo.processID;
    commonData->ulIPAddr = g_TaCb.taIP1;
    if(e_TA_SEND == direction)
    {
        commonData->ulFID = msg->datasrc.FID;
    }
    else
    {
        commonData->ulFID = msg->datadest.FID;
    }
    
    offset += sizeof(t_TaTraceCommonData);
    
    /*�����Ϣ*/
    XOS_MemCpy(offset, msg, sizeof(t_XOSCOMMHEAD));
    xos_head = (t_XOSCOMMHEAD *)offset;
    xos_head->length = msg_len;
    offset += sizeof(t_XOSCOMMHEAD);
    XOS_MemCpy(offset, msg->message, msg_len);

    offset += msg_len;

    /*β��ʶ*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(TSDATA_END_FLAG);

    XOS_MemSet(&dataReq, 0, sizeof(t_SCTPDATAREQ));
    dataReq.linkHandle = g_TaCb.linkHandle;
    dataReq.msgLenth = bufSize;
    dataReq.pData = (XCHAR *)buf;

    ret = TA_SendMsg(&dataReq, sizeof(t_SCTPDATAREQ), FID_NTL, eSendData);
    if(ret != XSUCC)
    {
        XOS_MemFree(FID_TA, buf);
        XOS_Trace(MD(FID_TA, PL_ERR ), "send msg to ntl error!");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:XOS_LogInfect
����:  log��Ⱦ�ӿ�
����:  userId ���û���ʶ
���:
����: ����logID
˵��: 
************************************************************************/
XU32 XOS_LogInfect(const t_InfectId *userID)
{
    t_TaMission *mission = NULL;
    XU32 logID = 0;
    XS32 i = 0;
    XS32 j = 0;

    if(0 == g_TaCb.missionNum)
    {
        return logID;
    }
    
    if(!userID)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "arg error");
        return logID;
    }

    if((0 == userID->idNum) && (0 == userID->imsiNum))
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "userID has no user");
        return logID;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        if(g_TaCb.taCb[i].flag)
        {
            for(j = 0;j < userID->idNum && j < TA_USER_NUM;j++)
            {
                if((g_TaCb.taCb[i].telLen > 0) && 
                    (!XOS_StrNcmp(userID->id[j], g_TaCb.taCb[i].tel, 
                    XOS_MAX(XOS_StrLen(userID->id[j]),g_TaCb.taCb[i].telLen))))
                {
                    mission = g_TaCb.taCb[i].mission;
                    while(mission)
                    {
                        if(TA_LOG_INVALID != mission->logLevel)
                        {
                            TA_SET_BIT(logID, mission->idx);
                        }
                        mission = mission->next;
                    }
                }
            }
            for(j = 0;j < userID->imsiNum && j < TA_USER_NUM;j++)
            {
                if((g_TaCb.taCb[i].imsiLen > 0) && (!XOS_StrNcmp(userID->imsi[j], g_TaCb.taCb[i].imsi, 
                    XOS_MAX(XOS_StrLen(userID->imsi[j]),g_TaCb.taCb[i].imsiLen))))
                {
                    mission = g_TaCb.taCb[i].mission;
                    while(mission)
                    {
                        if(TA_LOG_INVALID != mission->logLevel)
                        {
                            TA_SET_BIT(logID, mission->idx);
                        }
                        mission = mission->next;
                    }
                }
            }
        }
    }
    XOS_MutexUnlock(&(g_TaCb.mutex));
    
    return logID;
}

/************************************************************************
������:XOS_LogTrace
����:  log���˽ӿ�
����: FileName          -�������ڵ��ļ���
        ulLineNum       -���������к�
      ulFid           - ���룬���ܿ�ID
      ulLevel         - ���룬��ӡ����
      ucFormat        - ���룬��ӡ��ʽ���ַ���
      logId           - LogID
      cFormat         - ���룬��ӡ��ʽ
      ...             - ���룬��ӡ����
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XVOID XOS_LogTrace(const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel,XU32 logID, const XCHAR *cFormat, ... )
{
    va_list ap;
    
    va_start(ap, cFormat);
    XOS_LogTraceX(FileName, ulLineNum, FunName,ulFid, eLevel,logID, cFormat, ap );
    va_end(ap);

    return;
}

/************************************************************************
������:XOS_LogTraceX
����:  log���˽ӿ�
����: FileName          -�������ڵ��ļ���
        ulLineNum       -���������к�
      ulFid           - ���룬���ܿ�ID
      ulLevel         - ���룬��ӡ����
      ucFormat        - ���룬��ӡ��ʽ���ַ���
      logId           - LogID
      cFormat         - ���룬��ӡ��ʽ
      ...             - ���룬��ӡ����
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��: 
************************************************************************/
XVOID XOS_LogTraceX(const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel,XU32 logID, const XCHAR *cFormat, va_list ap )
{
    t_TaMission *mission = NULL;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*����������Ϣ��������*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*���͸�TS�����ݻ�����ָ��*/
    XU32 bufSize = 0;       /*���͸�TS�������ܳ���*/
    XS8 *offset = NULL;
    t_TaTraceCommonData *commonData = NULL;
    t_TaMsgHead *head = NULL;
    t_TaLogRsp *logRsp = NULL;
    t_SCTPDATAREQ dataReq;
    XCHAR   msg_filter[MAX_TRACE_INFO_LEN+5]  = {0}; /*5 bytes for \r\n*/
    va_list aq;
    XS32 i = 0;
    XU32 logLen = 0;

    if(XNULL == cFormat || eLevel >= PL_MAX || eLevel < PL_MIN)
    {
        return;
    }

#ifdef XOS_LINUX
    va_copy(aq,ap);
#else
    aq = ap;
#endif

    XOS_TraceTa(FileName, ulLineNum, FunName,ulFid, eLevel, cFormat,ap);

    if(!g_TaCb.tsFlag)
    {
        XOS_Trace(FILI, FID_TA, PL_DBG, "XOS_LogTrace:disconnect of TS ");
        return;
    }

    /*û�и�������ֱ�ӷ���*/
    if(0 == logID || 0 == g_TaCb.missionNum)
    {
        XOS_Trace(FILI, FID_TA, PL_DBG, "XOS_LogTrace:logID is 0,no one trace this log");
        return;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    g_ta_log_num++;
    /*�����û����ƿ飬��ѯ���ٴ�����Ϣ�Ĺ�������*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*�ж��û����ƿ��Ƿ�ʹ��*/
        if(g_TaCb.taCb[i].flag)
        {
            mission = g_TaCb.taCb[i].mission;
            while(mission)
            {
                if(TA_ISSET_BIT(logID,mission->idx))
                {
                    if(mission->logLevel != 0xffff && mission->logLevel <= eLevel)
                    {
                        XOS_StrNcpy(traceMd5[traceNum], mission->md5, MAX_MD5_LEN);
                        traceNum++;
                        
                        if(traceNum >= TA_MISSION_NUM)
                        {
                            /*��������ֻ��32�����е�����˵����������ܿ����ڴ汻��*/
                            XOS_MutexUnlock(&(g_TaCb.mutex));
                            XOS_Trace(FILI, FID_TA, PL_ERR, "XOS_LogTrace:program error,num of mission exceeds %d",TA_MISSION_NUM);
                            return;
                        }
                    }
                }
                mission = mission->next;
            }
        }
    }
    XOS_MutexUnlock(&(g_TaCb.mutex));

    if(0 == traceNum)
    {
        XOS_Trace(FILI, FID_TA, PL_INFO, "XOS_LogTrace:no one trace this msg");
        return;
    }
    
    XOS_TraceInfo(FileName, ulLineNum, FunName,ulFid, eLevel, cFormat,aq,msg_filter);
    va_end(aq);

    logLen = XOS_StrLen(msg_filter);
    if(0 == logLen || logLen > MAX_TRACE_INFO_LEN)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "length of log [%d] is invalid",logLen);
        return;
    }
    
    /*���͸�TS�ĸ�����Ϣ������:����ͷ |MD5 | ���ݹ���ͷ | ƽ̨��Ϣ |β��ʶ*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_TaLogRsp) + logLen + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "XOS_LogTrace:malloc %d byte fail",bufSize);
        return;
    }
    offset = buf;

    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;

    offset += sizeof(t_TaMsgHead);

    /*���MD5ֵ������*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*�������MD5ֵ*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*��乫������ͷ*/
    commonData = (t_TaTraceCommonData *)offset;
    commonData->ucProVer = g_interfaceVer;
    commonData->ucMsgDirect = e_TA_SEND;
    commonData->ucTraceType = TRACE_TYPE_LOG;
    commonData->ucNeType = XOS_HtoNs(g_TaCb.localInfo.neType);
    commonData->usNeId = XOS_HtoNs(g_TaCb.localInfo.neID);
    commonData->usLogicPid = XOS_HtoNs(g_TaCb.localInfo.processID);
    commonData->ulIPAddr = XOS_HtoNl(g_TaCb.taIP1);
    commonData->ulFID = XOS_HtoNl(ulFid);
    commonData->ulSeqNum = 0;
    commonData->ulTimeStamp = 0;

    offset += sizeof(t_TaTraceCommonData);

    logRsp = (t_TaLogRsp *)offset;
    logRsp->logID = XOS_HtoNl(logID);
    logRsp->line = XOS_HtoNl(ulLineNum);
    logRsp->level = XOS_HtoNs(eLevel);
    logRsp->len = XOS_HtoNs(logLen);

    offset += sizeof(t_TaLogRsp);
    /*�����Ϣ*/
    XOS_MemCpy(offset, msg_filter, logLen);

    offset += logLen;
    
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(TSDATA_END_FLAG);

    XOS_MemSet(&dataReq, 0, sizeof(t_SCTPDATAREQ));
    dataReq.linkHandle = g_TaCb.linkHandle;
    dataReq.msgLenth = bufSize;
    dataReq.pData = (XCHAR *)buf;

    ret = TA_SendMsg(&dataReq, sizeof(t_SCTPDATAREQ), FID_NTL, eSendData);
    if(ret != XSUCC)
    {
        XOS_MemFree(FID_TA, buf);
        XOS_Trace(MD(FID_TA, PL_ERR ), "XOS_LogTrace:send msg to ntl error!");
        return;
    }

    return;
}

/************************************************************************
������:XOS_SetInterfaceVer
����:  ����ҵ�����ýӿ�Э��汾��
����: ver          -�ӿ�Э��汾��
���:
����: void
˵��: 
************************************************************************/
XVOID XOS_SetInterfaceVer(XU8 ver)
{
    g_interfaceVer = ver;
}


/************************************************************************
������: TA_regCli()
����:  �����г�ʼ��
����:
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 TA_regCli(XVOID)
{
    XS32 ret=0;
    XS32 reg_result = 0;
    ret = XOS_RegistCmdPrompt( SYSTEM_MODE, "plat", "plat", "no parameter" );
    if(ret<0)
    {
        return XERROR;
    }

    reg_result = XOS_RegistCommand(ret,TA_Show,"tainfo","show info of TA","no parameter");
    if(reg_result < 0)
    {
        return XERROR;
    }

    reg_result = XOS_RegistCommand(ret,TA_Count,"tacount","show num of trace msg in TA","no parameter");
    if(reg_result < 0)
    {
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:TA_Show
����: ��ʾTA�����Ϣ
����:
���:
����:
˵��: showta���������ִ�к���
************************************************************************/
XVOID TA_Show(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XCHAR taIP1[32] = {0};
    XCHAR taIP2[32] = {0};
    XCHAR tsIP1[32] = {0};
    XCHAR tsIP2[32] = {0};
    XS32 i = 0;
    t_TaMission *mission = NULL;

    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,"trace missions in TA\r\n============================\r\n");

    /*��ӡͨ��������Ϣ*/
    XOS_CliExtPrintf(pCliEnv,
        "%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\r\n",
        "mission",
        "neType",
        "neID",
        "processID",
        "shelfID",
        "slotID",
        "tsHigh",
        "tsLow",
        "tsFlag",
        "nasFlag",
        "expireTime"
        );
    XOS_CliExtPrintf(pCliEnv,
        "%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d%-10d\r\n",
        g_TaCb.missionNum,
        g_TaCb.localInfo.neType,
        g_TaCb.localInfo.neID,
        g_TaCb.localInfo.processID,
        g_TaCb.localInfo.shelfID,
        g_TaCb.localInfo.slotID,
        g_TaCb.tsHighSlot,
        g_TaCb.tsLowSlot,
        g_TaCb.tsFlag,
        g_TaCb.nasFlag,
        g_TaCb.expireTimes
        );
    XOS_CliExtPrintf(pCliEnv,"----------------------------\r\n");
    
    XOS_CliExtPrintf(pCliEnv,
        "%-10s%-20s%-20s%-20s%-20s\r\n",
        "port",
        "taIP1",
        "taIP2",
        "tsIP1",
        "tsIP2"
        );
    
    XOS_IptoStr(g_TaCb.taIP1,taIP1);
    XOS_IptoStr(g_TaCb.taIP2,taIP2);
    XOS_IptoStr(g_TaCb.tsIP1,tsIP1);
    XOS_IptoStr(g_TaCb.tsIP2,tsIP2);
    XOS_CliExtPrintf(pCliEnv,
        "%-10d%-20s%-20s%-20s%-20s\r\n",
        g_TaCb.port,
        taIP1,
        taIP2,
        tsIP1,
        tsIP2
        );

    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        if(g_TaCb.taCb[i].flag)
        {
            XOS_CliExtPrintf(pCliEnv,"============================\r\n");
            XOS_CliExtPrintf(pCliEnv,
                "%-10s%-36s%-16s%-10s\r\n",
                "traceID",
                "tel",
                "imsi",
                "interface"
                );
            XOS_CliExtPrintf(pCliEnv,
                "%-10x%-36s%-16s%-10d\r\n",
                g_TaCb.taCb[i].traceID,
                g_TaCb.taCb[i].tel,
                g_TaCb.taCb[i].imsi,
                g_TaCb.taCb[i].ifType
                );
            XOS_CliExtPrintf(pCliEnv,"----------------------------\r\n");

            mission = g_TaCb.taCb[i].mission;
            while(mission)
            {
                XOS_CliExtPrintf(pCliEnv,
                    "%-6s%-34s%-12s%-12s%-12s\r\n",
                    "idx",
                    "md5",
                    "logLevel",
                    "groupID",
                    "linkID"
                    );
                XOS_CliExtPrintf(pCliEnv,
                    "%-6d%-34s%-12d%-12d%-12d\r\n",
                    mission->idx,
                    mission->md5,
                    mission->logLevel,
                    mission->groupID,
                    mission->linkID
                    );
                XOS_CliExtPrintf(pCliEnv,"----------------------------\r\n");
                mission = mission->next;
            }
        }
    }
    return ;
}

/************************************************************************
������:TA_Count
����: ��ʾTA�����Ϣ
����:
���:
����:
˵��: showta���������ִ�к���
************************************************************************/
XVOID TA_Count(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,"============================\r\n");
    XOS_CliExtPrintf(pCliEnv,"trace msg num in TA\r\n");
    /*��ӡͨ��������Ϣ*/
    XOS_CliExtPrintf(pCliEnv,
        "%-10s%-10s%-10s%-10s\r\n",
        "TaTrace",
        "IfTrace",
        "TmgTrace",
        "LogTrace"
        );
    XOS_CliExtPrintf(pCliEnv,
        "%-10d%-10d%-10d%-10d\r\n",
        g_ta_num,
        g_ta_if_num,
        g_ta_tmg_num,
        g_ta_log_num);

    XOS_CliExtPrintf(pCliEnv,"============================\r\n");
}
#endif  /*#ifdef XOS_TRACE_AGENT*/

#ifdef __cplusplus
}
#endif /* __cplusplus */


