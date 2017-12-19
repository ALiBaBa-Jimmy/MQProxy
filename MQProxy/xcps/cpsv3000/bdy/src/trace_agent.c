
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
                  包含头文件
-------------------------------------------------------------------------*/
#include "xosencap.h"
#include "xostl.h"
#include "xosarray.h"
#include "xosmem.h"
#include "trace_agent.h"
#include "xosipmi.h"
#include "xosnetinf.h"

/*-------------------------------------------------------------------------
                      模块内部宏定义
-------------------------------------------------------------------------*/
#define TA_MISSION_NUM  32  /*最多支持的过滤任务数量*/
#define TA_OFFLINE_NUM  4   /*最多支持的离线跟踪任务数量*/


#define TA_LINK_HANDLE  0x01020304  /*TA建立的与TS之间的SCTP链路句柄*/

#define TS_F1IP_PREFIX    "172.16.100.%d"     /*TS端F1网卡监听IP前缀*/
#define TS_F2IP_PREFIX    "172.16.101.%d"     /*TS端F2网卡监听IP前缀*/
#define TA_IP_PREFIX    "172.16.%d.%d"     /*TA端F1网卡监听IP前缀*/
#define TA_F1IP_INDEX    2                  /*F1平面网卡索引，用于计算F1网卡的IP*/
#define TA_F2IP_INDEX    3                  /*F2平面网卡索引，用于计算F2网卡的IP*/
#define TS_IP_BASE          235                 /*TA的ip第四段为220+槽位号*/

#define TA_F1_NUM       2                   /*F1网卡平面号*/
#define TA_F2_NUM       3                   /*F2网卡平面号*/

#define TS_PORT           17401             /*TS服务端监听端口*/
#define TA_IP_LEN       64                  /*IP地址字符串最大长度*/

#define TA_CHECK_INTERVAL   4000        /*TA每隔4s检测与TS的链路连接状态*/
#define TA_CHECK_NUM        3           /*检测3次，如果仍然断链则停止所有跟踪任务*/

#define TSDATA_BEGIN_FLAG (0x7ea5)  /*协议起始标志*/
#define TSDATA_END_FLAG (0x7e0d)    /*协议结束标志*/
#define TA_LOG_INVALID  (0xff)      /*过滤条件中log跟踪级别设为0xff表示不跟踪日志*/

#define TA_IP_NUM       2               /*TS和 TA各自绑定2个IP*/

#define TA_PORT_BASE        10000 /*TA与TS端口时的起始值*/
#define TA_PORT_OFFSET      32     /*TA与TS端口时网元ID偏移值*/

#define TA_NE_TYPE_MIN      0       /*网元类型最小值*/
#define TA_NE_TYPE_MAX      7       /*网元类型最大值*/
#define TA_NE_ID_MIN        1       /*网元ID最小值*/
#define TA_NE_ID_MAX        127     /*网元ID最大值*/
#define TA_PROCESS_ID_MIN        1       /*网元逻辑进程ID最小值*/
#define TA_PROCESS_ID_MAX        32     /*网元逻辑进程ID最大值*/
#define TA_SLOT_ID_MIN        1       /*槽位号最小值*/
#define TA_SLOT_ID_MAX        14     /*槽位号最大值*/
#define TA_SHELF_ID_MIN        1       /*框号最小值*/
#define TA_SHELF_ID_MAX        25     /*框号最大值*/

#define TA_REG_NUM              32      /*删除过滤任务时给所有注册的模块发通知消息，最多支持32个模块*/
#define TA_NE_EMME          0x0001  /*EMME的网元类型定义，与TS相同步*/

#define TA_INVALID_LINK     0xFFFF  /*无效的链路组ID和链路ID*/
/*TraceID中设置过滤任务对应的位*/
#define TA_SET_BIT(v,x)     ((v) |= (1<< ((x) % (8 * sizeof(XU32)))))

/*TraceID中清除过滤任务对应的位*/
#define TA_CLR_BIT(v,x)     ((v) &= (~(1<< ((x) % (8 * sizeof(XU32))))))

/*TraceID中判断位是否被设置*/
#define TA_ISSET_BIT(v,x)   ((v) & (1<< ((x) % (8 * sizeof(XU32)))))
/*-------------------------------------------------------------------------
                     模块内部结构和枚举定义
-------------------------------------------------------------------------*/
typedef enum 
{
    TA_CHECK_TIMER = 1,
}e_taTimer;

/*TA过滤任务控制块,存储TS下发的过滤任务的MD5值*/
typedef struct t_TaMissionT t_TaMission;
struct t_TaMissionT
{
    XS32 idx;               /*过滤任务的索引*/
    XU8 md5[MAX_MD5_LEN];  /*TS下发的过滤任务的md5值*/
    
    XU8 ifFlag;            /*是否跟踪模块接口消息*/
    
    XU8 msgType;            /*消息跟踪类型:控制，媒体，ip报文*/
    XU8 logLevel;          /*log级别 0~6, oxff*/

    XU16  groupID;         /*链路组ID，接口跟踪使用*/
    XU16  linkID;          /*链路ID，接口跟踪使用*/
    
    t_TaMission *next;     /*链表，存储相同用户标识的过滤任务*/
};

/*各个用户标识对应的过滤任务控制块*/
typedef struct
{
    XU32 traceID;               /*用户标识下的控制面TraceID*/
    XU8 tel[MAX_TEL_LEN];     /*电话号码*/
    XU16 telLen;                /*电话号码长度*/
    XU8 imsi[MAX_IMSI_LEN];    /*IMSI*/
    XU16 imsiLen;               /*IMSI长度*/
    e_IF_TYPE ifType;           /*接口过滤任务*/

    XBOOL flag;                 /*标志位，XFALSE空闲，XTRUE占用*/
    t_TaMission *mission;       /*过滤任务*/
}t_TaBlock;

/*存储所有注册到TA的模块的FID*/
typedef struct
{
    XU32 fid[TA_REG_NUM];   /*存储所有注册的模块的FID*/
    XU32 num;               /*已经注册的模块数量*/
}t_reg_fids;

/*发送给TS的本网元相关信息*/
typedef struct
{
    XU16    neType;     /*网元类型*/
    XU16    neID;       /*网元ID*/
    XU16    processID;  /*本进程逻辑进程号*/
    XU16    slotID;     /*本板槽位号*/
    XU16    shelfID;     /*本板所在框号*/
}t_processInfo;

typedef struct
{
    t_TaBlock   taCb[TA_MISSION_NUM];   /*过滤任务控制块*/
    XU16    missionNum;                 /*TA已经接收的过滤任务数*/
    XU16    dataNum;                 /*TA的用户面数据跟踪过滤任务数*/
    t_XOSMUTEXID    mutex;             /*互斥锁*/
    t_processInfo   localInfo;        /*发送给TS的网元相关信息*/
    t_reg_fids  regFids;                /*存储注册到TA的网元模块的FID*/
    
    XU16    port;                      /*与TS建链使用的本地端口*/
    XU32    taIP1;                     /*ta绑定的第一个IP*/
    XU32    taIP2;                     /*ta绑定的第二个IP*/
    XS32    tsLowSlot;                  /*收到的TS的低槽位号*/
    XS32    tsHighSlot;                 /*收到的TS的高槽位号*/
    XU32    tsIP1;                     /*ts监听的第一个IP*/
    XU32    tsIP2;                     /*ts监听的第二个IP*/
    XBOOL   tsFlag;                     /*与TS服务端连接是否正常*/
    XBOOL   nasFlag;                    /*是否需要让NAS模块缓存鉴权消息*/
    XU16    expireTimes;                /*与TS断链超时检测次数*/
    HAPPUSER     userHandle;            /*与TS的链路业务句柄*/
    HLINKHANDLE     linkHandle;         /*NTL保存的链路句柄*/
}t_TaCb;
/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/

static t_TaCb g_TaCb;       /*TA全局控制块*/
XU8 g_interfaceVer = 1;     /*业务协议版本号*/
PTIMER ta_timer= XNULL;
XU32 g_ta_num = 0;          /*TaTrace计数器*/
XU32 g_ta_if_num = 0;      /*接口跟踪计数器*/
XU32 g_ta_tmg_num = 0;      /*媒体跟踪计数器*/
XU32 g_ta_log_num = 0;      /*log跟踪计数器*/
XCONST XS8* ta_user_type[TRACE_MSG_TYPE_IPDATA+1] = 
{
    "",
    "CTRL",
    "VOICE",
    "VIDEO",
    "IP",
};

/*-------------------------------------------------------------------------
                   模块内部函数
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
函数名:TA_SendMsg
功能: ta模块发送消息接口
输入:buf    -发送的数据指针
    len     -数据长度
    fid     -目的模块fid
    msgID   -消息类型
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: 
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

    /*分配消息内存*/
    pMsg = XOS_MsgMemMalloc(FID_TA,len);
    if(XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"malloc msg failed!");
        return XERROR;
    }

    /*填写消息数据*/
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = FID_TA;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = fid;
    pMsg->length = (XU32)len;
    pMsg->msgID = msgID;
    pMsg->prio = eNormalMsgPrio;

    XOS_MemCpy(pMsg->message, buf,len);

    /*发送数据*/
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
函数名:TA_InitAckProc
功能: 处理ntl上报的init ack消息，发送链路启动消息给ntl
输入:pMsg   -NTL发送的消息指针
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 处理NTL发送的initAck消息
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

    /*检查返回的结果*/
    if( pLinkAck->lnitAckResult != eSUCC )
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"lnitAckResult != eSUCC!");
        return XERROR;
    }

    /* 记录linkHandle */
    g_TaCb.linkHandle = pLinkAck->linkHandle;
    
    /*启动链路*/
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
函数名:TA_StartAckProc
功能: 处理建链确认消息
输入:pMsg   -NTL发送的消息指针
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 处理NTL发送的startAck消息
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

        /*连接成功，停止重连定时器*/
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
函数名:TA_StoreUserMission
功能: 保存用户跟踪过滤任务
输入:sctpData   - ts发送的跟踪数据
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 
************************************************************************/
XS32 TA_StoreUserMission(t_SCTPDATAIND *sctpData)
{
    t_TaStartUserTraceReq *userReq = NULL;
    t_TaMission *mission = NULL;
    t_TaMission *tail = NULL;
    XS32 i = 0;
    XS32 idle = -1;             /*新增用户控制块的索引*/
    XU8 sameUser = XFALSE;      /*新过滤任务的用户标识是否与现有过滤任务相同*/

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
    /*添加过滤任务到任务控制块*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*依次遍历现有过滤任务，是否存在相同用户标识的过滤任务*/
        if(XTRUE == g_TaCb.taCb[i].flag)
        {
            /*存在相同标识的过滤任务,则把过滤任务连接到已有任务的链表后*/
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

                /*找到任务链表的尾节点，然后链接新的过滤任务*/
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
                
                /*保存过滤任务到控制块*/
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
                /*用户面跟踪不使用链路ID参数*/
                mission->groupID = 0;
                mission->linkID =0;
                mission->next = NULL;
                if((TRACE_MSG_TYPE_CONTROL != mission->msgType) && (mission->msgType <= TRACE_MSG_TYPE_IPDATA))
                {
                    g_TaCb.dataNum++;
                }

                /*修改用户控制块的局部TraceID*/
                TA_SET_BIT(g_TaCb.taCb[i].traceID, mission->idx);

                /*走到这里说明找到了同样用户标识的过滤任务*/
                sameUser = XTRUE;
                g_TaCb.missionNum++;
                break;
            }
        }
        else if(idle < 0)
        {
            /*存储第一个空闲控制块索引，以便后续添加用户控制块时直接使用*/
            idle = i;
        }
    }
    /*不存在相同用户标识的过滤任务，创建新的用户控制块*/
    if(!sameUser)
    {
        XOS_Trace(MD(FID_TA,PL_INFO),"new user ID for TA");
        /*无空闲控制块，不应该出现这种情况，除非内存被踩*/
        if(idle < 0 || idle >= TA_MISSION_NUM)
        {
            XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_START_TRACE:TA find no block for new mission!");
            XOS_MutexUnlock(&(g_TaCb.mutex));
            XOS_MemFree(FID_TA, sctpData->pData);
            return XERROR;
        }
        else
        {
            /*填充用户控制块相关信息*/
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

            /*保存过滤任务到控制块*/
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
            /*用户面跟踪不使用链路ID参数*/
            mission->groupID = 0;
            mission->linkID =0;
            mission->next = NULL;

            /*过滤任务链接到用户任务链表*/
            g_TaCb.taCb[idle].mission = mission;

            /*设置用户控制块局部TraceID*/
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
函数名:TA_StoreInterfaceMission
功能: 保存接口跟踪过滤任务
输入:sctpData   - ts发送的跟踪数据
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 
************************************************************************/
XS32 TA_StoreInterfaceMission(t_SCTPDATAIND *sctpData)
{
    t_TaStartIfTraceReq *ifReq = NULL;
    t_TaMission *mission = NULL;
    t_TaMission *tail = NULL;
    XS32 i = 0;
    XS32 idle = -1;             /*新增用户控制块的索引*/
    XU8 sameUser = XFALSE;      /*新过滤任务的用户标识是否与现有过滤任务相同*/

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

    /*添加过滤任务到任务控制块*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*依次遍历现有过滤任务，是否存在相同接口的过滤任务*/
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
                /*找到任务链表的尾节点*/
                tail = g_TaCb.taCb[i].mission;
                while(tail->next)
                {
                    tail = tail->next;
                }
                tail->next = mission;
                
                /*保存过滤任务*/
                mission->idx = ifReq->head.idx;
                if(mission->idx < 0 || mission->idx >= TA_MISSION_NUM)
                {
                    XOS_Trace(MD(FID_TA,PL_ERR),"invalid idx:%d!",mission->idx);
                    XOS_MutexUnlock(&(g_TaCb.mutex));
                    XOS_MemFree(FID_TA, sctpData->pData);
                    return XERROR;
                }
                XOS_StrNcpy(mission->md5, ifReq->data.ucMd5,MAX_MD5_LEN);
                
                /*接口跟踪无需跟踪模块接口消息、日志*/
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
            /*存储第一个空闲控制块索引，以便后续直接使用*/
            idle = i;
        }
    }
    if(!sameUser)
    {
        XOS_Trace(MD(FID_TA,PL_INFO),"new interface for TA");
        /*无空闲控制块，不应该出现这种情况，除非内存被踩*/
        if(idle < 0 || idle >= TA_MISSION_NUM)
        {
            XOS_Trace(MD(FID_TA,PL_ERR),"TA_CMD_START_TRACE:TA find no block for new mission!");
            XOS_MutexUnlock(&(g_TaCb.mutex));
            XOS_MemFree(FID_TA, sctpData->pData);
            return XERROR;
        }
        else
        {
            /*填充用户控制块相关信息*/
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
            /*填充过滤任务控制块*/
            mission->idx = ifReq->head.idx;
            if(mission->idx < 0 || mission->idx >= TA_MISSION_NUM)
            {
                XOS_Trace(MD(FID_TA,PL_ERR),"invalid idx:%d!",mission->idx);
                XOS_MutexUnlock(&(g_TaCb.mutex));
                XOS_MemFree(FID_TA, sctpData->pData);
                return XERROR;
            }
            XOS_StrNcpy(mission->md5, ifReq->data.ucMd5,MAX_MD5_LEN);
            
            /*接口跟踪无需跟踪模块接口消息、日志*/
            mission->ifFlag = 0;
            mission->logLevel = TA_LOG_INVALID;
            mission->groupID = XOS_HtoNs(ifReq->data.task.usLinkGid);
            mission->linkID = XOS_HtoNs(ifReq->data.task.usLinkId);
            mission->next = NULL;

            /*过滤任务链接到控制块的任务链表*/
            g_TaCb.taCb[idle].mission = mission;

            g_TaCb.missionNum++;
        }
    }
    
    return XSUCC;
}

/************************************************************************
函数名:TA_StopTrace
功能: 停止跟踪任务
输入:sctpData   - ts发送的停止请求数据
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 
************************************************************************/
XS32 TA_StopTrace(t_SCTPDATAIND *sctpData)
{
    t_TaMission *mission = NULL;
    t_TaMission *prev = NULL;
    t_TaStopTraceReq *stopReq = NULL;
    XS32 ret = 0;
    XS32 i = 0;
    XS32 idx = -1;              /*新增过滤任务的索引*/
    XU8 stopFlag = XFALSE;      /*找到所要停止的过滤任务的标识*/
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
                /*控制块存在，但是过滤任务链表为空，说明出异常了*/
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
        /*如果找到这个过滤任务，则在过滤控制块中删除*/
        if(stopFlag)
        {
            XOS_Trace(MD(FID_TA,PL_INFO),"find the trace mission which user wants to stop");

            if((TRACE_MSG_TYPE_CONTROL == mission->msgType)&&(mission->msgType <= TRACE_MSG_TYPE_IPDATA))
            {
                g_TaCb.dataNum--;
            }

            /*任务链表中删除此任务*/
            if(prev == mission)
            {
                /*要删除的过滤任务是链表头*/
                g_TaCb.taCb[i].mission = mission->next;
            }
            else
            {
                /*要删除的过滤任务不是链表头*/
                prev->next = mission->next;
            }
            /*释放内存*/
            XOS_MemFree(FID_TA, mission);
            mission = NULL;
            
            /*发送消息给注册到TA的网元模块，该用户标识已停止跟踪*/
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
            /*删除过滤任务后，修改用户控制块的局部TraceID*/
            TA_CLR_BIT(g_TaCb.taCb[i].traceID, idx);
                
            /*用户控制块中已没有过滤任务则需要释放用户控制块*/
            if(!g_TaCb.taCb[i].mission)
            {
                /*释放控制块*/
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

    /*当没有过滤条件时，给NAS发消息停止缓存鉴权消息*/
    if(TA_NE_EMME == g_TaCb.localInfo.neType)
    {
        if(0 == g_TaCb.missionNum && XTRUE == g_TaCb.nasFlag)
        {
            XOS_Trace(MD(FID_TA,PL_INFO),"send msg to NAS,to stop saving identify msg");
            /*这条消息无需实际数据，发送i仅仅是避免传入空指针导致发送接口失败*/
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
函数名:TA_TsProc
功能: 处理TS发送的数据
输入:pMsg   -NTL发送的消息指针
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 
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
            /*TS发送的请求IMSI的消息，直接转发给eMME*/
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

            /*TS发送的请求电话号码的消息，直接转发给eMME*/
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
            /*如果是开始用户跟踪的消息*/
            if(TRACE_TYPE_USER == traceType->ucTraceType)
            {
                TA_StoreUserMission(sctpData);
            }
            /*如果是开始接口跟踪的消息*/
            else if(TRACE_TYPE_IF == traceType->ucTraceType)
            {
                TA_StoreInterfaceMission(sctpData);
            }

            /*当有过滤条件时，给NAS发消息缓存鉴权消息*/
            if(TA_NE_EMME == g_TaCb.localInfo.neType)
            {
                if(g_TaCb.missionNum > 0 && XFALSE == g_TaCb.nasFlag)
                {
                    XOS_Trace(MD(FID_TA,PL_INFO),"send msg to NAS,to save identify msg");
                    /*这条消息无需实际数据，发送i仅仅是避免传入空指针导致发送接口失败*/
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
            /*TS发送的网元信息同步请求*/
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
函数名:TA_StopProc
功能: 处理NTL上报的断链消息
输入:pMsg   -NTL发送的消息指针
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 
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

    /*链路端开后，NTL开始客户端重连，TA启动定时器检测链路状态*/
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
函数名:TA_DeleteAllMission
功能: 删除所有过滤任务
输入:
输出:
返回:成功返回XSUCC , 失败返回XERROR
说明: 
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
            /*删除用户控制块下的过滤任务*/
            mission = g_TaCb.taCb[i].mission;
            while(mission)
            {
                g_TaCb.taCb[i].mission = mission->next;
                XOS_MemFree(FID_TA,mission);
                mission = g_TaCb.taCb[i].mission;
            }

            /*清空用户控制块*/
            XOS_MemSet(&(g_TaCb.taCb[i]), 0, sizeof(t_TaBlock));
        }
    }

    g_TaCb.missionNum = 0;

    /*当没有过滤条件时，给NAS发消息停止缓存鉴权消息*/
    if(TA_NE_EMME == g_TaCb.localInfo.neType)
    {
        if(XTRUE == g_TaCb.nasFlag)
        {
            /*这条消息无需实际数据，发送i仅仅是避免传入空指针导致发送接口失败*/
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
函数名:TA_Init
功能: 初始化trace agent模块
输入:
输出:
返回:
说明: 注册到模块管理中
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

    /*初始化全局变量*/
    XOS_MemSet(&g_TaCb, 0, sizeof(g_TaCb));

    ret = XOS_MutexCreate(&g_TaCb.mutex);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_TA,PL_ERR),"init semaphore failed!");
        return XERROR;
    }

    /*注册命令行*/
    TA_regCli();
    
#ifdef XOS_LINUX
    /*解析网元信息，本端端口，TS的槽位号等信息*/
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
    /*传入的低槽位号为0时，表示TS处于单机模式*/
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

    /*TA使用F1和F2网卡的物理IP与TS建链，物理ip根据框号，槽位号，平面号计算得来*/
    /*172.16.[(框号-1)*10+平面号].[槽位号]*/
    XOS_Sprintf(taIP1, TA_IP_LEN, TA_IP_PREFIX,(g_TaCb.localInfo.shelfID -1)*10 + TA_F1IP_INDEX,slotID);
    XOS_StrtoIp(taIP1, &g_TaCb.taIP1);
    
    XOS_Sprintf(taIP2, TA_IP_LEN, TA_IP_PREFIX,(g_TaCb.localInfo.shelfID - 1)*10 + TA_F2IP_INDEX,slotID);
    XOS_StrtoIp(taIP2, &g_TaCb.taIP2);

    g_TaCb.port = TA_PORT_BASE + neID * TA_PORT_OFFSET + processID;

    /*TS部署为单机模式*/
    if( 0 == tsLowSlot )
    {
        XOS_Trace(MD(FID_TA, PL_INFO), "TS is single mode!");
        XOS_Sprintf(tsIP1, TA_IP_LEN, TS_F1IP_PREFIX,TS_IP_BASE+tsHighSlot);
        XOS_StrtoIp(tsIP1, &g_TaCb.tsIP1);

        XOS_Sprintf(tsIP2, TA_IP_LEN, TS_F2IP_PREFIX,TS_IP_BASE+tsHighSlot);
        XOS_StrtoIp(tsIP2, &g_TaCb.tsIP2);
    }
    /*TS部署为HA模式*/
    else
    {
        XOS_Trace(MD(FID_TA, PL_INFO ), "TS is HA mode!");
        XOS_Sprintf(tsIP1, TA_IP_LEN, TS_F1IP_PREFIX,TS_IP_BASE+tsLowSlot);
        XOS_StrtoIp(tsIP1, &g_TaCb.tsIP1);

        XOS_Sprintf(tsIP2, TA_IP_LEN, TS_F2IP_PREFIX,TS_IP_BASE+tsHighSlot);
        XOS_StrtoIp(tsIP2, &g_TaCb.tsIP2);
    }
    /*建立与TS之间的SCTP链路，发送链路初始化消息给NTL*/
    linkInit.appHandle = (HAPPUSER)TA_LINK_HANDLE;
    linkInit.linkType = eSCTPClient;
    linkInit.ctrlFlag = eNullLinkCtrl;

    /*发送消息给NTL进行链路初始化*/
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
函数名:TA_msgProc
功能:  trace agent 模块消息处理函数入口
输入:  pMsg －消息指针
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: 此消息处理函数是trace agent主任务消息入口
消息不在此函数处理的范围内
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
        /*处理NTL上报的数据*/
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
        /*处理EMME发送的数据*/
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
                /*eMME发送的电话号码或IMSI数据，直接转发给TS*/
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
函数名:TA_timerProc
功能:  trace agent 模块定时器消息处理函数入口
输入:  pMsg －消息指针
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明:
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
            /*链路已经重连成功，超时次数清零*/
            g_TaCb.expireTimes = 0;
        }
        else
        {
            /*链路仍然处于断链状态，则超时次数加1*/
            g_TaCb.expireTimes++;
        }
        /*检测超时次数超过设定次数，则停止所有过滤任务*/
        if(g_TaCb.expireTimes >= TA_CHECK_NUM)
        {
            TA_DeleteAllMission();
        }
        XOS_MutexUnlock(&(g_TaCb.mutex));
    }
    else
    {
        /*错误参数*/
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
函数名:XOS_TaInfect
功能:  业务感染接口
输入:  userId －用户标识
输出:
返回: 返回TraceID
说明: 业务直接使用返回的TraceID,无需判断返回值合法性
************************************************************************/
XU32 XOS_TaInfect(const t_InfectId *userID)
{
    XU32 traceID = 0;
    XS32 i = 0;
    XS32 j = 0;

    /*没有跟踪任务直接返回*/
    if(0 == g_TaCb.missionNum)
    {
        return traceID;
    }

    /*入参判断*/
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
    /*匹配过滤条件*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        if(g_TaCb.taCb[i].flag)
        {
            /*依次匹配用户标识，并用各个用户的局部TraceID生成最终的TraceID*/
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
函数名:XOS_TaTrace
功能:  业务消息过滤接口
输入:  fid - 模块fid
        direct - 消息方向
        msg －消息指针
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: 
************************************************************************/
XS32 XOS_TaTrace(XU32 fid,e_TaDirection direct,const t_XOSCOMMHEAD *msg)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*跟踪这条消息的任务数*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*发送给TS的数据缓冲区指针*/
    XU32 bufSize = 0;       /*发送给TS的数据总长度*/
    XS8 *offset = NULL;
    t_TaTraceCommonData *commonData = NULL;
    t_TaMsgHead *head = NULL;
    t_SCTPDATAREQ dataReq;
    XU32 msg_len = 0;
    t_XOSCOMMHEAD *xos_head = NULL;

    if(!g_TaCb.tsFlag)
    {
        /*与TS失连*/
        return XERROR;
    }

    if(!msg)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "msg is NULL");
        return XERROR;
    }

    if(0 == msg->traceID)
    {
        /*大部分消息都不会跟踪，为提高处理效率，这里没有打印Trace*/
        return XSUCC;
    }

    /*没有跟踪任务直接返回*/
    if(0 == g_TaCb.missionNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    g_ta_num++;
    /*遍历用户控制块，查询跟踪此条消息的过滤任务*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*判断用户控制块是否使用*/
        if(g_TaCb.taCb[i].flag)
        {
            /*判断用户控制块的局部traceID是否匹配*/
            if((g_TaCb.taCb[i].traceID & msg->traceID) != 0)
            {
                mission = g_TaCb.taCb[i].mission;
                while(mission)
                {
                    /*过滤任务的索引在traceID中*/
                    if(TA_ISSET_BIT(msg->traceID,mission->idx) && TRACE_MSG_TYPE_CONTROL == mission->msgType)
                    {
                        XOS_StrNcpy(traceMd5[traceNum], mission->md5, MAX_MD5_LEN);
                        traceNum++;
                        
                        if(traceNum >= TA_MISSION_NUM)
                        {
                            /*总任务数只有32，运行到这里说明程序出错，很可能内存被踩*/
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

    /*TA发送给TS的最大trace数据长度不超过4096字节，多余的被截掉*/
    msg_len = XOS_MIN(msg->length, TA_MAX_MSG_LEN);
    /*发送给TS的跟踪消息:公共头 | MD5 | 数据公共头 | 平台消息 |尾标识*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_XOSCOMMHEAD) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }

    /*构造发送给TS的trace消息*/
    offset = (XS8 *)buf;
    /*填充公共头*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*填充MD5值的数量*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*填充所有MD5值*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*填充公共数据头*/
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
    
    /*填充消息*/
    XOS_MemCpy(offset, msg, sizeof(t_XOSCOMMHEAD));
    xos_head = (t_XOSCOMMHEAD *)offset;
    xos_head->length = msg_len;
    offset += sizeof(t_XOSCOMMHEAD);
    XOS_MemCpy(offset, msg->message, msg_len);

    offset += msg_len;
    p16Data = (XU16 *)offset;

    /*添加尾标识*/
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
函数名:XOS_TaTrace
功能:  业务消息过滤接口
输入:  srcFid - 源模块fid
        dstFid - 目的模块fid
        buf －消息指针
        len - 数据长度
        id - 用户标识
        type - 用户标识类型
        direction - 消息方向
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: 业务模块接口消息过滤接口，需要业务自己在需要的地方调用
************************************************************************/
XS32 XOS_TaDataTrace(XU32 srcFid,XU32 dstFid,const XVOID *data,XU32 len,const XU8 *id,e_IdType type,e_TaDirection direction)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*跟踪这条消息的任务数*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*发送给TS的数据缓冲区指针*/
    XU32 bufSize = 0;       /*发送给TS的数据总长度*/
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

    /*没有跟踪任务直接返回*/
    if(0 == g_TaCb.missionNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    /*匹配过滤条件*/
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
                        /*总任务数只有32，运行到这里说明程序出错，很可能内存被踩*/
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

    /*TA发送给TS的最大trace数据长度不超过4096字节，多余的被截掉*/
    msg_len = XOS_MIN(len, TA_MAX_MSG_LEN);
    /*发送给TS的跟踪消息:公共头 |MD5 | 数据公共头 | 消息 |尾标识*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_XOSCOMMHEAD) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }
    offset = (XS8 *)buf;

    /*公共头*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*填充MD5值的数量*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*填充所有MD5值*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*填充公共数据头*/
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

    /*消息*/
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
    /*填充消息*/
    XOS_MemCpy(offset, data, msg_len);

    offset += msg_len;

    /*尾标识*/
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
函数名:XOS_TmgFilter
功能:  TMG模块媒体数据过滤接口
输入:  fid - 模块fid
        buf －媒体数据缓冲区
        len - 数据长度
        msgType - 跟踪媒体数据类型，音频或视频
        traceID - 跟踪ID
输出:
返回:  成功返回XSUCC , 失败返回XERROR
说明: TMG模块专用
************************************************************************/
XS32 XOS_TmgFilter(XU32 fid,const XVOID *data,XU32 len,e_TRACE_MSG_TYPE msgType,XU32 traceID)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*跟踪这条消息的任务数*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*发送给TS的数据缓冲区指针*/
    XU32 bufSize = 0;       /*发送给TS的数据总长度*/
    XS8 *offset = NULL;
    t_TaTraceCommonData *commonData = NULL;
    t_TaMsgHead *head = NULL;
    t_SCTPDATAREQ dataReq;
    XU32 msg_len = 0;

    if(!g_TaCb.tsFlag)
    {
        /*与TS失连*/
        return XERROR;
    }

    if(!data)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "buf is NULL");
        return XERROR;
    }

    if(0 == traceID)
    {
        /*大部分消息都不会跟踪，为提高处理效率，这里没有打印Trace*/
        return XSUCC;
    }

    /*没有跟踪任务直接返回*/
    if(0 == g_TaCb.dataNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    g_ta_tmg_num++;
    /*遍历用户控制块，查询跟踪此条消息的过滤任务*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*判断用户控制块是否使用*/
        if(g_TaCb.taCb[i].flag)
        {
            /*判断用户控制块的局部traceID是否匹配*/
            if((g_TaCb.taCb[i].traceID & traceID) != 0)
            {
                mission = g_TaCb.taCb[i].mission;
                while(mission)
                {
                    /*过滤任务的索引在traceID中*/
                    if(TA_ISSET_BIT(traceID,mission->idx) && msgType == mission->msgType)
                    {
                        XOS_StrNcpy(traceMd5[traceNum], mission->md5, MAX_MD5_LEN);
                        traceNum++;
                        
                        if(traceNum >= TA_MISSION_NUM)
                        {
                            /*总任务数只有32，运行到这里说明程序出错，很可能内存被踩*/
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

    /*TA发送给TS的最大trace数据长度不超过4096字节，多余的被截掉*/
    msg_len = XOS_MIN(len, TA_MAX_MSG_LEN);
    /*发送给TS的跟踪消息:公共头 | MD5 | 数据公共头 | 媒体数据 |尾标识*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }

    /*构造发送给TS的trace消息*/
    offset = (XS8 *)buf;
    /*填充公共头*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*填充MD5值的数量*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*填充所有MD5值*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*填充公共数据头*/
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
    
    /*填充消息*/
    XOS_MemCpy(offset, data, msg_len);
    offset += msg_len;

    /*添加尾标识*/
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
函数名:XOS_TaRegDel
功能:  业务调用，注册删除自己保存的TraceID的回调函数接口
输入:  fid －业务模块的Fid
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: 有些业务需要保存TraceID,这些模块就需要注册回调来删除TraceID
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
函数名:XOS_TaInterface
功能:  网元接口消息过滤接口
输入:   fid  - 模块fid
        type －网元接口类型
        groupID - 消息源标识，多源地址时使用
        linkID - 消息目的标识，多目的地址时使用
        direct - 发送或接收的标识
        addr - 源目的IP地址和端口
        data - 数据缓冲区
        len - 数据长度
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: 如果接口两端的网元是一对一的，则源、目的标识都填0
************************************************************************/
XS32 XOS_TaInterface(XU32 fid,e_IF_TYPE type,XU16 groupID,XU16 linkID,e_TaDirection direct,t_TaAddress *addr,const XVOID *data,XU16 len)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*跟踪这条消息的任务数*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*发送给TS的数据缓冲区指针*/
    XU32 bufSize = 0;       /*发送给TS的数据总长度*/
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

    /*没有跟踪任务直接返回*/
    if(0 == g_TaCb.missionNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    /*遍历用户控制块，查询跟踪此条消息的过滤任务*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*判断用户控制块是否使用*/
        if(g_TaCb.taCb[i].flag)
        {
            /*判断接口是否被跟踪*/
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
                            /*总任务数只有32，运行到这里说明程序出错，很可能内存被踩*/
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

    /*TA发送给TS的最大trace数据长度不超过4096字节，多余的被截掉*/
    msg_len = XOS_MIN(len, TA_MAX_MSG_LEN);
    /*发送给TS的跟踪消息:公共头 |MD5 | 数据公共头 | 接口跟踪响应 |尾标识*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_TaInterfaceRsp) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }
    offset = (XS8 *)buf;
    /*公共头*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*填充MD5值的数量*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*填充所有MD5值*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*填充公共数据头*/
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
    
    /*填充消息*/
    interRsp = (t_TaInterfaceRsp *)offset;

    interRsp->ifType = type;
    interRsp->groupID = groupID;
    interRsp->linkID = linkID;
    interRsp->len = msg_len;
    offset += sizeof(t_TaInterfaceRsp);

    /*填充吐出的接口消息数据*/
    XOS_MemCpy(offset, data, msg_len);

    offset += msg_len;

    /*填充尾标识*/
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
函数名:XOS_TaIdTrace
功能:  特殊消息过滤接口
输入:  msg －过滤消息指针
        id - 过滤消息对应的用户标识
        type - 用户标识类型:电话号码或IMSI
        direction - 消息方向
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: NAS和S1AP之间鉴权流程消息中没有电话号码和IMSI，只能缓存消息，等鉴权完成后
调用此接口，传入获取的用户标识进行过滤
************************************************************************/
XS32 XOS_TaIdTrace(const t_XOSCOMMHEAD *msg,const XU8 *id,e_IdType type,e_TaDirection direction)
{
    t_TaMission *mission = NULL;
    XS32 i = 0;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*跟踪这条消息的任务数*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*发送给TS的数据缓冲区指针*/
    XU32 bufSize = 0;       /*发送给TS的数据总长度*/
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

    /*没有跟踪任务直接返回*/
    if(0 == g_TaCb.missionNum)
    {
        return XSUCC;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    /*匹配过滤条件*/
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
        /*用户标识符合跟踪条件*/
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
                        /*总任务数只有32，运行到这里说明程序出错，很可能内存被踩*/
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

    /*TA发送给TS的最大trace数据长度不超过4096字节，多余的被截掉*/
    msg_len = XOS_MIN(msg->length, TA_MAX_MSG_LEN);
    /*发送给TS的跟踪消息:公共头 |MD5 | 数据公共头 | 平台消息 |尾标识*/
    bufSize = sizeof(t_TaMsgHead) + sizeof(XU16) + traceNum * MAX_MD5_LEN + sizeof(t_TaTraceCommonData) + sizeof(t_XOSCOMMHEAD) + msg_len + sizeof(XU16);

    buf = XOS_MemMalloc(FID_TA, bufSize);
    if(XNULLP == buf)
    {
        XOS_Trace(FILI, FID_TA, PL_ERR, "malloc %d byte fail",bufSize);
        return XERROR;
    }
    offset = (XS8 *)buf;

    /*公共头*/
    head = (t_TaMsgHead *)offset;
    head->usBeginFlag = XOS_HtoNs(TSDATA_BEGIN_FLAG);
    head->usCmd = XOS_HtoNs(TA_CMD_TRACE_MSG);
    head->usBodyLen = XOS_HtoNs(bufSize - sizeof(t_TaMsgHead) - sizeof(XU16));
    head->ulSerial = 0;
    
    offset += sizeof(t_TaMsgHead);

    /*填充MD5值的数量*/
    XOS_MemCpy(offset, &traceNum, sizeof(traceNum));
    offset += sizeof(traceNum);

    /*填充所有MD5值*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*填充公共数据头*/
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
    
    /*填充消息*/
    XOS_MemCpy(offset, msg, sizeof(t_XOSCOMMHEAD));
    xos_head = (t_XOSCOMMHEAD *)offset;
    xos_head->length = msg_len;
    offset += sizeof(t_XOSCOMMHEAD);
    XOS_MemCpy(offset, msg->message, msg_len);

    offset += msg_len;

    /*尾标识*/
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
函数名:XOS_LogInfect
功能:  log感染接口
输入:  userId －用户标识
输出:
返回: 返回logID
说明: 
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
函数名:XOS_LogTrace
功能:  log过滤接口
输入: FileName          -代码所在的文件名
        ulLineNum       -代码所在行号
      ulFid           - 输入，功能块ID
      ulLevel         - 输入，打印级别
      ucFormat        - 输入，打印格式化字符串
      logId           - LogID
      cFormat         - 输入，打印格式
      ...             - 输入，打印参数
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: 
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
函数名:XOS_LogTraceX
功能:  log过滤接口
输入: FileName          -代码所在的文件名
        ulLineNum       -代码所在行号
      ulFid           - 输入，功能块ID
      ulLevel         - 输入，打印级别
      ucFormat        - 输入，打印格式化字符串
      logId           - LogID
      cFormat         - 输入，打印格式
      ...             - 输入，打印参数
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明: 
************************************************************************/
XVOID XOS_LogTraceX(const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel,XU32 logID, const XCHAR *cFormat, va_list ap )
{
    t_TaMission *mission = NULL;
    XS32 ret = 0;
    XU16 traceNum = 0;      /*跟踪这条消息的任务数*/
    XU16 *p16Data = NULL;
    XU8 traceMd5[TA_MISSION_NUM][MAX_MD5_LEN];
    XVOID *buf = NULL;      /*发送给TS的数据缓冲区指针*/
    XU32 bufSize = 0;       /*发送给TS的数据总长度*/
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

    /*没有跟踪任务直接返回*/
    if(0 == logID || 0 == g_TaCb.missionNum)
    {
        XOS_Trace(FILI, FID_TA, PL_DBG, "XOS_LogTrace:logID is 0,no one trace this log");
        return;
    }

    XOS_MutexLock(&(g_TaCb.mutex));
    g_ta_log_num++;
    /*遍历用户控制块，查询跟踪此条消息的过滤任务*/
    for(i = 0;i < TA_MISSION_NUM;i++)
    {
        /*判断用户控制块是否使用*/
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
                            /*总任务数只有32，运行到这里说明程序出错，很可能内存被踩*/
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
    
    /*发送给TS的跟踪消息共三段:公共头 |MD5 | 数据公共头 | 平台消息 |尾标识*/
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

    /*填充MD5值的数量*/
    p16Data = (XU16 *)offset;
    *p16Data = XOS_HtoNs(traceNum);
    offset += sizeof(XU16);

    /*填充所有MD5值*/
    for(i = 0;i < traceNum;i++)
    {
        XOS_MemCpy(offset, traceMd5[i], MAX_MD5_LEN);
        offset += MAX_MD5_LEN;
    }

    /*填充公共数据头*/
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
    /*填充消息*/
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
函数名:XOS_SetInterfaceVer
功能:  用于业务设置接口协议版本号
输入: ver          -接口协议版本号
输出:
返回: void
说明: 
************************************************************************/
XVOID XOS_SetInterfaceVer(XU8 ver)
{
    g_interfaceVer = ver;
}


/************************************************************************
函数名: TA_regCli()
功能:  命令行初始化
输入:
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明:
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
函数名:TA_Show
功能: 显示TA相关信息
输入:
输出:
返回:
说明: showta命令的最终执行函数
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

    /*打印通用配置信息*/
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
函数名:TA_Count
功能: 显示TA相关信息
输入:
输出:
返回:
说明: showta命令的最终执行函数
************************************************************************/
XVOID TA_Count(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,"============================\r\n");
    XOS_CliExtPrintf(pCliEnv,"trace msg num in TA\r\n");
    /*打印通用配置信息*/
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


