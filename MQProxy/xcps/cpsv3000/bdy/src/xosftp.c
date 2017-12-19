/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosftpclient.c
**
**  description:
**
**  author: zengjiandong
**
**  date:   2006.10.19
**
***************************************************************
**                          history
**
***************************************************************
**   author                     date              modification
**   zengjiandong         2006.10.19            create
**************************************************************/
#ifdef XOS_FTP_CLIENT
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "xoscfg.h"

#include "xosftp.h"
#include "xosmd5.h"

/*-------------------------------------------------------------------------
                 模块内部宏定义
-------------------------------------------------------------------------*/

#define MAX_FILENAME_LEN (255)    /*最大文件名长度*/
#define MAX_FTPCONNECTIONS (20)   /*最大允许同时连接登录的用户数*/
#define MAX_FTPTRANS_TIME (300) /*单位:秒*/


#define MAX_FTPCOMMAND_LEN (256)  /*最大的ftp命令允许长度*/

#define USER_CHECK_NUM (0xaaa)   /*用户句柄检验码*/

#define FTP_PASV_MODE (1)           /*数据连接被动模式*/
#define FTP_PORT (21)        /*ftp服务器控制连接固定端口*/

#define FTP_COMMAND_ACK_TIME (5000) /*ftp命令最大允许响应时间*/
#define FTP_TCPCONNECT_TIME (15000) /*ftp建立tcp连接的最大允许时间*/
#define FTP_PACKET_SIZE (4096) /*上传文件时每个包的大小*/

#define FTP_FILE_LIST_LEN    (8192*10)  /*文件列表大小*/

#ifdef XOS_VXWORKS
#define FTP_SEND_BUFFER_SIZE (4096)  /*VxWorks下发送窗口的大小*/
#endif

//#define FTP_TEST

/*消息队列*/
XSTATIC XOS_HLIST gFtpMsgQueue; /*队列*/
XSTATIC XCONST XS32 gFtpMsgQueueSize = 1024;/*队列大小*/
XSTATIC t_XOSMUTEXID gFtpMsgQueueMutex; /*临界区*/
XSTATIC t_XOSSEMID gFtpQuSemaphore;     /*同步信号量*/


/*重发消息队列*/
XSTATIC XOS_HLIST gFtpReMsgQueue; /*队列*/
XSTATIC XCONST XS32 gFtpReMsgQueueSize = 100;/*队列大小*/
XSTATIC t_XOSMUTEXID gFtpReMsgQueueMutex; /*临界区*/
XSTATIC t_XOSSEMID gFtpReQuSemaphore;     /*同步信号量*/
XSTATIC XVOID* FTP_CliMsgReDealTask(void* paras);


#define MAX_DATA_THREAD (4) /*任务池大小*/
#define MAX_TIMER_COUNT (4+1+45) /*定时器数量*/
#define NO_SEND_ELAPSES (0)     /*没有发送次数溢出*/
int gFtpMsgDealThread = MAX_DATA_THREAD;
XSTATIC XVOID* FTP_CliMsgDealTask(void*);


#ifdef FTP_TEST
XSTATIC PTIMER FtpTestTimer;
#define MEDIA_SEND_RTP_TIMER_UNIT (1000*1)
typedef enum
{
    MEDIA_TIMER_MSG_FTP_SEND = 10
}e_FTP_TIMER_NUM;

#endif

/*-------------------------------------------------------------------------
                 模块内部结构和枚举定义
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块内部全局变量 // = {XFALSE,  XNULL, XNULL};
-------------------------------------------------------------------------*/
 t_FTPMNT g_ftpMnt;

/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/
/************************************************************************
 函数名:FTP_BuildLinkH
 功能: 构造用户句柄
 输入: isCtrlLink  － 连接类型(高4位)，控制连接还是数据连接
                                  XTRUE表示控制连接，XFALSE表示数据连接
              userIndex  － 用户信息索引
 输出:  无
 返回:  返回用户句柄
 说明:  用户句柄组织如图，

                linkType  checkcoder          userIndex
               31      27                   15                            0 (bit)
               +----+------------+----------------+
               |        |                      |                            |
               +----+------------+----------------+

************************************************************************/
#define FTP_BuildLinkH( isCtrlLink, userIndex) \
    (XOS_HFTPCLT)(XPOINT)((((isCtrlLink)&0xf)<<28) |(USER_CHECK_NUM<<16) | (((userIndex)&0xffff)))

/************************************************************************
 函数名:FTP_IsValidLinkH
 功能: 验证链路句柄的有效性
 输入: 链路句柄
 输出:
 返回: 有效返回XTURE, 否则返回XFALSE
 说明:
************************************************************************/
#define FTP_IsValidLinkH(userHandle)  \
    ((USER_CHECK_NUM == ((XPOINT)(userHandle)&(USER_CHECK_NUM<<16))>>16)? XTRUE:XFALSE)

/************************************************************************
 函数名:FTP_GetLinkType
 功能: 通过用户句柄获取链路类型
 输入: 用户句柄
 输出: 无
 返回: 链路类型
 说明:
************************************************************************/
#define FTP_GetLinkType(userHandle)  ((XBOOL)(((XU32)(XPOINT)(userHandle))>>28))

/************************************************************************
 函数名:FTP_GetLinkIndex
 功能: 通过用户句柄获取链路Index
 输入: 用户句柄
 输出: 无
 返回: 链路索引
 说明:
************************************************************************/
#define FTP_GetLinkIndex(userHandle) ((XS32)(((XPOINT)(userHandle))&0xffff))

#define FTP_TIMER_STOP(TimerId)\
if(TimerId)\
{\
    XOS_TimerStop(FID_FTP, TimerId);\
    XOS_INIT_THDLE(TimerId);\
}

/************************************************************************
 函数名:FTP_IsValidUserHandle
 功能: 判断用户句柄是否有效
 输入: 用户句柄
 输出: pUserInfo    - 用户信息内容
 返回: XTRUE   - 有效用户;XFALSE   - 无效用户
 说明:
************************************************************************/
XSTATIC XBOOL FTP_IsValidUserHandle(XOS_HFTPCLT hFtpClt, XVOID **pUserInfo)
{
    XS32 userId = 0;
    
    /*参数验证*/
    if (XFALSE == FTP_IsValidLinkH(hFtpClt) || XNULLP == pUserInfo)
    {
        return XFALSE;
    }
    
    /*获取用户索引和用户信息*/
    userId = FTP_GetLinkIndex(hFtpClt);
    if (XNULLP == (*pUserInfo = XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, userId)))
    {
        return XFALSE;
    }
    
    /*判断用户当前状态*/
    if (eFTPSTATELOGIN != ((t_XOSFTPCLIENT*)(*pUserInfo))->curState)
    {
        return XFALSE;
    }
    
    return XTRUE;
}

/************************************************************************
 函数名:FTP_TimerSet
 功能: 设置定时器各参数
 输入: pUserInfo  - 用户信息项
              timerPara  - 定时器参数
              backPara   - 回调函数参数
              timerLen    - 定时时间长度
 输出: 无
 返回: 无
 说明:
************************************************************************/
XSTATIC XS8 FTP_TimerSet(t_XOSFTPCLIENT *pUserInfo,  t_PARA *timerPara,
                           t_BACKPARA *backPara, XU32 timerLen)
{
    if(NULL == pUserInfo || NULL == timerPara || NULL == backPara)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_TimerSet para is null\n");
        return XERROR;
    }
    
    XOS_MemSet(timerPara, 0, sizeof(t_PARA));
    timerPara->fid = FID_FTP;
    timerPara->len = timerLen;/*时间长度*/
    timerPara->mode = TIMER_TYPE_ONCE;/*定时器类型*/
    timerPara->pre = TIMER_PRE_LOW;/*定时器精度*/
    XOS_MemSet(backPara, 0, sizeof(t_BACKPARA));
    XOS_INIT_THDLE(pUserInfo->timerId);/*初始化定时器句柄*/
    backPara->para1 = (XPOINT)pUserInfo;/*设置需要的参数*/

    return XSUCC;
}

/************************************************************************
 函数名:FTP_LinkInit
 功能:  ftp链路初始化消息封装函数
 输入:  userId     - 用户索引
               linkType  - 链路类型
               isCtrlLink - 是否为控制链路
 输出: 无
 返回: 消息发送成功返回XSUCC , 发送失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32  FTP_LinkInit(XS32 userId, XU16 linkType, XBOOL isCtrlLink)
{
    t_XOSCOMMHEAD *pMsg = XNULLP;
    t_LINKINIT tLinkInit;
    
    /*申请消息需要的空间*/
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_LINKINIT));
    if (XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "FTP_LinkInit()-> malloc message memory fail!\n");
        return XERROR;
    }
    
    /*设置消息内容*/
    pMsg->datasrc.FID = FID_FTP;
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->length = sizeof(t_LINKINIT);
    pMsg->msgID = eLinkInit;
    pMsg->prio = eNormalMsgPrio;
    XOS_MemSet(&tLinkInit, 0, sizeof(t_LINKINIT));
    tLinkInit.linkType =(e_LINKTYPE)linkType;
    tLinkInit.appHandle = (HAPPUSER)FTP_BuildLinkH(isCtrlLink, (XU16)userId);
    XOS_MemCpy(pMsg->message, &tLinkInit, sizeof(t_LINKINIT));
    
    if (XSUCC != XOS_MsgSend(pMsg))/*发送消息*/
    {
        XOS_MsgMemFree(FID_FTP, pMsg);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_LinkInit()-> message send fail !\n");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
 函数名:FTP_LinkStart
 功能:  链路启动消息封装函数
 输入:  linkHandle      - 链路句柄
               linkType         - 链路类型
               pAddr            - 对端地址
 输出: 无
 返回: 消息发送成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32  FTP_LinkStart(HLINKHANDLE linkHandle, XU16 linkType, t_IPADDR *pAddr)
{
    t_XOSCOMMHEAD *pMsg = XNULLP;
    t_LINKSTART tLinkStartReq;

    if(NULL == pAddr)
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "FTP_LinkStart()-> pAddr is null!\n");
        return XERROR;
    }
    
    /*申请消息需要的空间*/
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_LINKSTART));
    if (XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_LinkStart()-> malloc message memory fail!\n");
        return XERROR;
    }
    
    /*设置消息体内容*/
    pMsg->datasrc.FID = FID_FTP;
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->length = sizeof(t_LINKSTART);
    pMsg->msgID = eLinkStart;
    pMsg->prio = eNormalMsgPrio;
    XOS_MemSet(&tLinkStartReq, 0, sizeof(t_LINKSTART));
    tLinkStartReq.linkHandle = linkHandle;
    switch(linkType)
    {
    case eTCPServer:/*tcp服务端*/
        XOS_MemCpy(&(tLinkStartReq.linkStart.tcpServerStart.myAddr), pAddr, sizeof(t_IPADDR));
        tLinkStartReq.linkStart.tcpServerStart.allownClients = 2;
        tLinkStartReq.linkStart.tcpServerStart.authenFunc = XNULLP;
        break;
    case eTCPClient:/*tcp客户端*/
        XOS_MemSet(&(tLinkStartReq.linkStart.tcpClientStart.myAddr), 0, sizeof(t_IPADDR));
        XOS_MemCpy(&(tLinkStartReq.linkStart.tcpClientStart.peerAddr), pAddr, sizeof(t_IPADDR));
        break;
    default:
        break;
    }
    XOS_MemCpy(pMsg->message, &tLinkStartReq, sizeof(t_LINKSTART));
    
    if (XSUCC != XOS_MsgSend(pMsg))/*发送消息*/
    {
        XOS_MsgMemFree(FID_FTP, pMsg);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_LinkStart()-> message send fail !\n");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
 函数名:FTP_LinkRelease
 功能:  链路释放消息封装函数
 输入:  linkHandle      - 链路句柄
 输出: 无
 返回: 消息发送成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32  FTP_LinkRelease(HLINKHANDLE linkHandle)
{
    t_XOSCOMMHEAD *pMsg = XNULLP;
    t_LINKRELEASE tLinkRelease;
    
    /*申请消息所需要的空间*/
    if (XNULLP == (pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_LINKRELEASE))))
    {
        XOS_Trace(MD(FID_FTP,PL_EXP),"FTP_LinkRelease()-> malloc msg mem fail\n");
        return  XERROR;
    }
    
    /*设置消息体的内容*/
    XOS_MemSet(&tLinkRelease, 0, sizeof(t_LINKRELEASE));
    tLinkRelease.linkHandle = linkHandle;
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID= FID_FTP;
    pMsg->length = sizeof(t_LINKRELEASE);
    pMsg->msgID = eLinkRelease;
    pMsg->prio = eNormalMsgPrio;
    pMsg->datadest.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    XOS_MemCpy(pMsg->message, &tLinkRelease, sizeof(t_LINKRELEASE));
    
    if (XSUCC != XOS_MsgSend(pMsg))/*发送消息*/
    {
        XOS_MsgMemFree(FID_FTP, pMsg);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_LinkRelease()-> message send fail !\n");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
 函数名:FTP_SingleComSend
 功能:  ftp命令发送的封装函数
 输入:  pUserInfo        - 用户信息
               pCmd             - ftp命令
               pArg1             - ftp命令的参数
               pArg2             - ftp命令的参数
 输出: 无
 返回: 消息发送成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 FTP_SingleComSend(t_XOSFTPCLIENT *pUserInfo, XS8* pCmd, XS8* pArg1, XS8* pArg2)
{
    XS32 cmdSize = 0;/*命令的长度*/
    XS8 buffer[MAX_FTPCOMMAND_LEN] = {0};/*用于保存命令*/
    XS8* pData = XNULLP;/*数据指针*/
    t_XOSCOMMHEAD *pMsg = XNULLP;/*消息头指针*/
    t_DATAREQ tDataReq;
    
    /*参数验证*/
    if (XNULLP == pUserInfo || XNULLP == pCmd || XNULLP == pArg1 || XNULLP == pArg2 ||
        XOS_StrLen(pArg1)+XOS_StrLen(pArg2)+XOS_StrLen(pCmd) > MAX_FTPCOMMAND_LEN-1)
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "FTP_SingleComSend()-> Bad input param !\n");
        return XERROR;
    }

    XOS_Sprintf(buffer,sizeof(buffer)-1, "%s%s%s", pCmd,pArg1,pArg2);
    cmdSize = (XS32)XOS_StrLen(buffer);
    
    /*申请消息空间*/
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_DATAREQ));
    if (XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_SingleComSend()-> malloc message memory fail!\n");
        return XERROR;
    }
    
    pData = (XCHAR*)XOS_MemMalloc(FID_FTP, cmdSize+2);
    if(pData == XNULL)
    {
        XOS_MsgMemFree(FID_FTP, pMsg);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_SingleComSend()-> malloc data memory fail!\n");
        return XERROR;
    }
    
    XOS_MemSet(pData, 0, cmdSize+2);
    XOS_MemCpy(pData, buffer, cmdSize);
    XOS_MemCpy(pData + cmdSize, "\r\n", 2);
    /*设置消息头内容*/
    pMsg->datasrc.FID = FID_FTP;
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->length = sizeof(t_DATAREQ);
    pMsg->msgID = eSendData;
    pMsg->prio = eNormalMsgPrio;
    
    /*填写消息中的数据内容*/
    XOS_MemSet(&tDataReq, 0, sizeof(t_DATAREQ));
    tDataReq.linkHandle = pUserInfo->linkHandle;
    XOS_MemCpy(&(tDataReq.dstAddr), &(pUserInfo->dstAddr), sizeof(t_IPADDR));
    tDataReq.msgLenth = cmdSize+2;
    tDataReq.pData = pData;
    XOS_MemCpy(pMsg->message, &tDataReq, sizeof(t_DATAREQ));
    
    if (XSUCC != XOS_MsgSend(pMsg))/*发送消息*/
    {
        /*数据指示消息，应该先释放收到数据*/
        if(pMsg->msgID == eSendData)
        {
            XOS_MemFree(FID_FTP,pData);
        }
        XOS_MsgMemFree(FID_FTP,pMsg);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_SingleComSend()-> message send fail !\n");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
 函数名:FTP_ComSendWithTimeOut
 功能:  含定时器的单一命令发送
 输入:  pUserInfo        - 用户信息
               pCmd             - ftp命令
               pArg1             - ftp命令的参数
               pArg2             - ftp命令的参数
               curEvent         - 当前事件
 输出: 无
 返回: 消息发送成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 FTP_ComSendWithTimeOut(t_XOSFTPCLIENT *pUserInfo,
                                    XS8* pCmd, XS8* pArg1, XS8* pArg2, e_FTPCUREVENT curEvent)
{
    t_PARA timerPara;
    t_BACKPARA backPara;

    if(NULL == pUserInfo || NULL == pCmd || NULL == pArg1 || NULL == pArg2)
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "FTP_ComSendWithTimeOut()-> para is null!\n");
        return XERROR;
    }
    
    pUserInfo->curEvent = curEvent;
    pUserInfo->ret = XERROR;
    
    /*设置定时器*/
    if(XSUCC != FTP_TimerSet(pUserInfo, &timerPara, &backPara, FTP_COMMAND_ACK_TIME))
    {
        XOS_INIT_THDLE(pUserInfo->timerId);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ComSendWithTimeOut()-> FTP_TimerSet  fail!\n");
        return XERROR;
    }

    /*启动定时器*/
    if (XERROR == XOS_TimerStart(&(pUserInfo->timerId), &timerPara, &backPara))
    {
        XOS_INIT_THDLE(pUserInfo->timerId);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ComSendWithTimeOut()-> timer start fail!\n");
        return XERROR;
    }
    
    /*发送ftp命令*/
    if (XERROR == FTP_SingleComSend(pUserInfo, pCmd, pArg1, pArg2))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ComSendWithTimeOut()-> Command send error!\n");

        FTP_TIMER_STOP(pUserInfo->timerId);
        return XERROR;
    }

    XOS_SemGet(&(pUserInfo->semaphore)); /*等待150*/
    
    return (pUserInfo->ret == XSUCC) ? XSUCC : XERROR;
}

/************************************************************************
 函数名:FTP_GetComReplyCode
 功能:  取应答码
 输入:  replyStr           - ftp应答信息
               dataLength       - ftp应答的长度
 输出: 无
 返回: ftp应答码
 说明:
************************************************************************/
XSTATIC XS32  FTP_GetComReplyCode(XS8* replyStr, XU32 dataLength)
{
    XS32 replyCode = 0;
    
    sscanf(replyStr, "%d", &replyCode);
    return replyCode;
}

/************************************************************************
 函数名:FTP_ReplySpecialParse
 功能:  对可能出现两条应答的消息进行处理
 输入:  replyStr           - ftp应答信息
               dataLength       - ftp应答的长度
 输出: pUserInfo         - 用户信息
 返回: 只有一条应答返回XSUCC,两条应答返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 FTP_ReplySpecialParse(XS8* replyStr, XU32 dataLength, t_XOSFTPCLIENT *pUserInfo)
{
    XS8 *pCh = 0;
    XS32 replyCode = 0;

    if(NULL == replyStr || NULL == pUserInfo)
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "FTP_ReplySpecialParse()-> para is null!\n");
        return XERROR;
    }
    
    pCh = XOS_StrStr(replyStr, "\r\n");
    if (dataLength == (XU32)(pCh - replyStr + 2))/*只是准备应答则返回*/
    {
        return XSUCC;
    }
    
    /*处理第2个应答,只有下载文件极短时出现*/
    pCh = pCh + 2;
    sscanf(pCh, "%d", &replyCode);
    if (replyCode == 226)
    {
        pUserInfo->replyInfoType = ePRE_AND_TRANS;
        XOS_SemPut(&(pUserInfo->semaphore));
    }
    else
    {
        pUserInfo->replyInfoType = ePRE_AND_TRANSFAIL;
        XOS_SemPut(&(pUserInfo->semaphore));
    }
    return XERROR;
}

/************************************************************************
 函数名:FTP_GetCurDir
 功能:  从收到的ftp应答数据中获取当前路径
 输入:  replyStr           - ftp应答信息
               dataLength       - ftp应答的长度
 输出:  pDir                - 当前路径
 返回: 成功返回XSUCC，失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32  FTP_GetCurDir(XS8* replyStr, XU32 dataLength, XS8* pDir, XU32 nLen)
{
    XU32 i = 0;
    XS8* pCh = XNULLP;
    
    /*参数检验*/
    if (XNULLP == replyStr || XNULLP == pDir || 0 == dataLength)
    {
        return XERROR;
    }

    pCh = replyStr;
    i = 0;
    while (*pCh != '"' && i != dataLength)
    {
        i++;
        pCh++;
    }
    i++; 
    pCh++;

    while (*pCh != '"' && i < dataLength && i < nLen)
    {
        *pDir++ = *pCh++;
        i++;
    }
    if(i >= nLen)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_GetCurDir i >= nLen\n");
    }
    *pDir = '\0';
    return XSUCC;
}

/************************************************************************
 函数名:FTP_GetFileSize
 功能:  从收到的数据中获取文件大小信息
 输入:  replyStr           - ftp应答信息
               dataLength       - ftp应答的长度
 输出:  pFileSize          - 文件大小
 返回: 成功返回XSUCC，失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32  FTP_GetFileSize(XS8* replyStr, XU32 dataLength, XSFILESIZE *pFileSize)
{
    XSFILESIZE m = 0;
    XSFILESIZE n = 0;

    if(NULL == replyStr || NULL == pFileSize)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_GetFileSize para is null\n");
        return XERROR;
    }
    
#ifdef XOS_ARCH_64
    sscanf(replyStr, "%lld %lld", &m, &n);
#else
    sscanf(replyStr, "%d %d", &m, &n);
#endif
    *pFileSize = n;
    return XSUCC;
}

/************************************************************************
 函数名:FTP_GetPasvServerAddr
 功能:  从收到的数据中获取数据连接的对端地址
 输入:  replyStr           - ftp应答信息
 输出:  pAddr              - 数据连接中对方地址
 返回: 成功返回XSUCC，失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32  FTP_GetPasvServerAddr(XS8* replyStr, t_IPADDR *pAddr)
{
    XS8* pCh = XNULLP;
    XS32 arg1 =0, arg2 = 0, arg3 = 0, arg4 = 0, arg5 = 0, arg6 =0;
    
    if (XNULLP == replyStr || XNULLP == pAddr)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_GetPasvServerAddr para is null\n");
        return XERROR;
    }
    pCh = XOS_StrStr(replyStr, "(");
    if (XNULLP == pCh)
    {
        return XERROR;
    }
    
    sscanf(pCh+1, "%d,%d,%d,%d,%d,%d", &arg1, &arg2, &arg3, &arg4, &arg5, &arg6);
    pAddr->ip = arg1*0x1000000 + arg2*0x10000 + arg3*0x100 + arg4;
    pAddr->port = arg5*0x100 + arg6;
    
    return XSUCC;
}

/************************************************************************
 函数名:FTP_RecvInitAck
 功能:  处理链路初始化消息的应答消息
 输入:  pMsg            - ftp链路初始化响应消息
 输出:  无
 返回:  无
 说明:
************************************************************************/
XSTATIC XVOID FTP_RecvInitAck(t_XOSCOMMHEAD *pMsg)
{
    XS32 index = 0;
    t_LINKINITACK *pInitAck = XNULLP;
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    if(NULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvInitAck para is null\n");
        return ;
    }
    pInitAck = (t_LINKINITACK*)(pMsg->message);
    index = (XS32)FTP_GetLinkIndex((XOS_HFTPCLT)(pInitAck->appHandle));
    if (FTP_GetLinkType((XOS_HFTPCLT)(pInitAck->appHandle)))/*控制链路初始化*/
    {
        pUserInfo = (t_XOSFTPCLIENT*)XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, index);
        if (XNULLP != pUserInfo)
        {
            if (eSUCC == pInitAck->lnitAckResult)/*链路初始化成功，接着start*/
            {
                pUserInfo->linkHandle = pInitAck->linkHandle;
                pUserInfo->curState = eFTPCTRLINITED;
                if (XSUCC != FTP_LinkStart(pUserInfo->linkHandle, eTCPClient, &(pUserInfo->dstAddr)))
                {
                    /*                    FTP_LinkRelease(pUserInfo->linkHandle);*/
                    XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvInitAck()-> message send fail !\n");
                }
            }
            /*            else if (eFAIL == pInitAck->lnitAckResult)
            {
            XOS_SemPut(&(pUserInfo->semaphore));
        }*/
        }
    }
}

/************************************************************************
 函数名:FTP_RecvStartAck
 功能:  处理链路初始化消息的应答消息
 输入:  pMsg            - ftp链路启动响应消息
 输出:  无
 返回:  无
 说明:
************************************************************************/
XSTATIC XVOID FTP_RecvStartAck(t_XOSCOMMHEAD *pMsg)
{
    XS32 index = 0;
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_STARTACK *pStartAck = XNULLP;

    if(NULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvStartAck para is null\n");
        return ;
    }
    pStartAck = (t_STARTACK*)(pMsg->message);
    index = (XS32)FTP_GetLinkIndex((XOS_HFTPCLT)(pStartAck->appHandle));
    if (FTP_GetLinkType((XOS_HFTPCLT)(pStartAck->appHandle)))/*控制链路启动*/
    {
        pUserInfo = (t_XOSFTPCLIENT*)XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, index);
        if (XNULLP != pUserInfo)
        {
            switch (pStartAck->linkStartResult)
            {
            case eSUCC:/*链路启动成功，发送用户名*/
                XOS_MemCpy(&(pUserInfo->locAddr), &(pStartAck->localAddr), sizeof(t_IPADDR));
                break;
                
            case eFAIL:/*启动失败，释放链路*/
                       /*                    FTP_LinkRelease(pUserInfo->linkHandle);
                XOS_SemPut(&(pUserInfo->semaphore));*/
                XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_StartAck()->control link start fail !\n");
                break;
                
            case eBlockWait:/*正在连接中*/
                /*                    XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_StartAck()->control link connecting !\n");*/
                break;
                
            default:/*未知的无效消息*/
                XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_StartAck()->unknowed result !\n");
                break;
            }
        }
    }
}

/************************************************************************
 函数名:FTP_bulidDataLink
 功能:  创建文件传输的数据连接
 输入:  pServAddr     - 对端地址
 输出:  pSockId        - socket
 返回:  成功返回XSUCC, 失败返回XERROR
 说明:  该调用是阻塞的，当网络情况不佳，或者对端
               serv没有启动时，会阻塞相当一段时间
************************************************************************/
XSTATIC XS32 FTP_bulidDataLink(t_IPADDR* pServAddr, t_XINETFD* pSockId)
{
    XS32 ret = 0;
    XS32 optVal = 0;
    //struct linger optLinger;
    
    /*参数验证*/
    if(pServAddr == XNULLP || pSockId == XNULLP)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_bulidDataLink()-> bad input param!");
        return XERROR;
    }
    
    ret = XINET_Socket(XOS_INET_STREAM, pSockId);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_bulidDataLink()-> open sock failed !");
        return XERROR;
    }
    
    /*设置成阻塞模式*/
    optVal = XOS_INET_OPT_ENABLE;
    XINET_SetOpt(pSockId, SOL_SOCKET, XOS_INET_OPT_BLOCK, (XU32 *)&optVal);

    /*Linger*/    
    //optLinger.l_onoff = 1;
    //optLinger.l_linger = 0;
    //ret = setsockopt(pSockId->fd, SOL_SOCKET, SO_LINGER, (char*)&optLinger, sizeof(optLinger));

    
#ifdef XOS_VXWORKS
    /*在VxWorks下设置发送窗口大小*/
    optVal = FTP_SEND_BUFFER_SIZE;
    XINET_SetOpt(pSockId, SOL_SOCKET, XOS_INET_OPT_TX_BUF_SIZE, &optVal);
#endif
    
    /*连接对端*/
    ret = XINET_Connect(pSockId,  pServAddr);
    if(ret != XSUCC)
    {
        XINET_CloseSock(pSockId);
        pSockId->fd = XERROR;
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_bulidDataLink()-> connect to server failed !");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
 函数名:FTP_dataTskEntry
 功能: 上传 下载文件数据的任务入口函数
 输入: pPutEnty          - 任务入口参数
 输出: 无
 返回: 无
 说明: 该任务控制数据链路，上传后任务自动退出
************************************************************************/
XSTATIC XVOID FTP_dataTskEntry(t_FTPDATALINKENTRY* pPutEnty)
{
    XSFILESIZE pendLen = 0;
    XS32  readLen = 0;
    XCHAR *pPendData = NULL;
    XCHAR  recvBuff[FTP_PACKET_SIZE] = {0};
    XSFILESIZE ret = 0;
    XSFILESIZE sumLen = 0;
    XS32 out_len = 0;
    XSFILESIZE writeRst = 0;

    if(NULL == pPutEnty)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_dataTskEntry()-> bad input param!");
        return ;
    }
    
    pendLen = pPutEnty->buffLen;
    
    switch (pPutEnty->curEvent)
    {
        /*上传文件模式*/
    case eFTPPUTFROMFILE:
        while(pendLen>0)
        {
            readLen = (XS32)XOS_MIN(FTP_PACKET_SIZE, pendLen);
            /*读取文件*/
            ret = XOS_ReadFile((XVOID*) recvBuff, 1, readLen, pPutEnty->pFile);
            if(ret == XERROR)
            {
                XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()->read file error!");
                break;
            }
            
            /*发送数据*/
            ret = XINET_SendMsg(pPutEnty->pSockId,eTCPClient,&(pPutEnty->ftpServAddr), readLen, (char*)recvBuff,&out_len);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()->put file block failed!");
                break;
            }
            
            pendLen = pendLen -readLen;
        }
        /*上传完关闭sock*/
        XINET_CloseSock(pPutEnty->pSockId);
        pPutEnty->pSockId->fd = XERROR;
        XOS_SemPut(pPutEnty->pSem);        
        break;
        
        /*上传内存块模式*/
    case eFTPPUTFROMMEM:
        pPendData = (XCHAR*)pPutEnty->pBuffer;
        
        while(pendLen>0)
        {
            readLen = (XS32)XOS_MIN(FTP_PACKET_SIZE, pendLen);
            
            /*发送数据*/
            ret = XINET_SendMsg(pPutEnty->pSockId,eTCPClient,&(pPutEnty->ftpServAddr), readLen, pPendData,&out_len);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()->put mem block failed!");
                break;
            }
            pendLen = pendLen -readLen;
            pPendData = pPendData +readLen;
            
        }
        
        /*上传完关闭sock*/
        XINET_CloseSock(pPutEnty->pSockId);
        pPutEnty->pSockId->fd = XERROR;
        XOS_SemPut(pPutEnty->pSem);        
        break;
        
    case eFTPGETTOFILE:

        pPutEnty->ulDownLen = 0;
        /*从网络上收数据*/
        while(1)
        {
            readLen = recv((pPutEnty->pSockId)->fd, (char*)recvBuff, FTP_PACKET_SIZE, 0);
            if(readLen <= 0)
            {
                /*关闭sock*/
                XINET_CloseSock(pPutEnty->pSockId);
                pPutEnty->pSockId->fd = XERROR;
                break;
            }

            writeRst = fwrite(recvBuff, 1, readLen, pPutEnty->pFile);
            if(1 > writeRst)
            {
                XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()-> fwrite failed !");
            }
            else
            {
                pPutEnty->ulDownLen += readLen;
            }

            XOS_Sleep(2);
        }     
        
        XOS_SemPut(pPutEnty->pSem);
        break;
        
    case eFTPGETTOMEM:
        
        /*从网络上收数据*/
        sumLen = 0;
        pPendData = (XCHAR*)pPutEnty->pBuffer;
        while(1)
        {
            readLen = recv((pPutEnty->pSockId)->fd, (char*)recvBuff, FTP_PACKET_SIZE, 0);
            if(readLen <= 0)
            {
                /*关闭sock*/
                XINET_CloseSock(pPutEnty->pSockId);
                pPutEnty->pSockId->fd = XERROR;
                break;
            }
            
            /*检验内存越界*/
            sumLen += readLen;
            if(sumLen > pPutEnty->buffLen)
            {
                /*关闭sock*/
                XINET_CloseSock(pPutEnty->pSockId);
                pPutEnty->pSockId->fd = XERROR;
                XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()-> recv buffer len limit !");
                break;
            }
            XOS_MemCpy(pPendData, recvBuff, readLen);
            pPendData += readLen;

            XOS_Sleep(2);
        }        

        XOS_SemPut(pPutEnty->pSem);
        break;
        
    case eFTPLIST:
        /*分配文件列表内存*/
        pPutEnty->pBuffer = XNULLP;
        pPutEnty->pBuffer = (XCHAR*)XOS_MemMalloc(FID_FTP, FTP_FILE_LIST_LEN);
        if(pPutEnty->pBuffer == XNULLP)
        {
            XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()-> XOS_MemMalloc buffer failed!");
            XINET_CloseSock((t_XINETFD*) pPutEnty->pSockId);
            pPutEnty->pSockId->fd = XERROR;
            XOS_SemPut(pPutEnty->pSem);    
            return;
        }
        pPendData = pPutEnty->pBuffer;
        
        /*从网络上收数据*/
        sumLen = 0; /*总共接收到的数据*/
        while(1)
        {
            readLen = recv((pPutEnty->pSockId)->fd, (char*)recvBuff, FTP_PACKET_SIZE, 0);
            
            if(readLen <= 0)
            {
                /*关闭sock*/
                XINET_CloseSock(pPutEnty->pSockId);
                pPutEnty->pSockId->fd = XERROR;
                pPutEnty->buffLen = sumLen;
                break;
            }
            
            /*检验内存越界*/
            sumLen +=readLen;
            if(sumLen >= FTP_FILE_LIST_LEN)
            {
                /*关闭sock*/
                XINET_CloseSock(pPutEnty->pSockId);
                pPutEnty->pSockId->fd = XERROR;
                XOS_Trace(MD(FID_FTP, PL_WARN),"FTP_dataTskEntry()-> recv data %d,buffer len limit %d.",sumLen,FTP_FILE_LIST_LEN);
                break;
            }
            XOS_MemCpy(pPendData, recvBuff, readLen);
            pPendData += readLen;

            XOS_Sleep(2);
        }
        
        XOS_SemPut(pPutEnty->pSem);        
        break;
        
    default:
        XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()->put file type error!");
        break;
     }
     return ;
}

/************************************************************************
 函数名:FTP_dataTskWithTimeOut
 功能:  含定时器的任务的执行
 输入:  pUserInfo            - 用户信息内容
               pTskEntry            - 任务入口参数
               timeLen               - 任务允许执行的时间长度
 输出:  无
 返回:  无
 说明:
************************************************************************/
XSTATIC XS32 FTP_dataTskWithTimeOut(t_XOSFTPCLIENT *pUserInfo,
                                    t_FTPDATALINKENTRY *pTskEntry, XU32 timeLen)
{
    t_PARA timerPara;
    t_BACKPARA backPara;
//    t_XOSTASKID idTask;

    if(NULL == pUserInfo || NULL == pTskEntry)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_dataTskWithTimeOut()-> para is null!");
        return XERROR;
    }
    
    /*启动定时器*/
    pUserInfo->ret = XERROR;
    if (0 != timeLen && ePREPARE_SUC == pUserInfo->replyInfoType)
    {
        FTP_TimerSet(pUserInfo, &timerPara, &backPara, timeLen*1000);
        if (XERROR == XOS_TimerStart(&(pUserInfo->timerId), &timerPara, &backPara))
        {
            XINET_CloseSock(pTskEntry->pSockId);
            pTskEntry->pSockId->fd = XERROR;
            XOS_INIT_THDLE(pUserInfo->timerId);
            return XERROR;
        }
    }
    
    /*创建一个独立的任务进行数据传输*/
    FTP_dataTskEntry(pTskEntry);
/*    
    if (XERROR == XOS_TaskCreate("FTPDataTsk", TSK_PRIO_HIGHER,10000,(os_taskfunc)FTP_dataTskEntry, pTskEntry, &idTask))
    {
        XINET_CloseSock(pTskEntry->pSockId);
        pTskEntry->pSockId->fd = XERROR;
        FTP_TIMER_STOP(pUserInfo->timerId);
        return XERROR;
    }
*/
    XOS_SemGet(pTskEntry->pSem); /*数据通道同步*/

    /*如果226不响应，则有挂起的危险，只有等到上面这个定时器超时*/
    XOS_SemGet(&(pUserInfo->semaphore));/*控制通道同步*/
    return XSUCC;
}

/*****************************************************************
 add by lyp;
 get the second reply in one massage.
 When a small file is downloaded, the mesage will have two reply
****************************************************************/
XSTATIC XS32 FTP_GetSecondReplyCode(XS8* replyStr, XU32 dataLength, XS32* replyCode)
{
    XS8* pCh = NULL;
    if(NULL == replyStr || NULL == replyCode)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_GetSecondReplyCode()-> para is null!");
        return XERROR;
    }
    
    pCh = XOS_StrStr(replyStr, "\r\n");
    if (dataLength == (XU32)(pCh - replyStr + 2))
    {
        return XERROR;
    }
    
    pCh = pCh + 2;
    sscanf(pCh, "%d", replyCode);
    return XSUCC;
}

/************************************************************************
 函数名:FTP_RecvDataInd
 功能:  处理数据消息的应答消息
 输入:  pMsg            - ftp链路启动响应消息
 输出:  无
 返回:  无
 说明:
************************************************************************/
XSTATIC XVOID FTP_RecvDataInd(t_XOSCOMMHEAD *pMsg)
{
    XS32 index = 0;
    XS32 replyCode = 0;
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_DATAIND *pDataInd = XNULLP;

    if(NULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_RecvDataInd()-> para is null!");
        return ;
    }
    
    pDataInd = (t_DATAIND*)(pMsg->message);

    index = (XS32)FTP_GetLinkIndex((XOS_HFTPCLT)(pDataInd->appHandle)); /*ntl链路索引*/
    /*控制链路消息*/
    if (FTP_GetLinkType((XOS_HFTPCLT)(pDataInd->appHandle)))/*控制链路数据指示*/
    {
        replyCode = FTP_GetComReplyCode(pDataInd->pData, pDataInd->dataLenth);
        
        pUserInfo = (t_XOSFTPCLIENT*)XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, index);

        /*对用户状态进行操作，未上锁*/
        if (XNULLP != pUserInfo)
        {
            switch (pUserInfo->curEvent)
            {

            /*登录阶段状态，用户可以随时发送用户名命令修改登录的用户*/                
            case eFTPNONE: /*建立控制链路时，收到服务器的欢迎消息*/
                if (220 == replyCode)
                {
                    pUserInfo->curEvent = eFTPNAME;
                    /*发送用户名*/
                    FTP_SingleComSend(pUserInfo, "USER ", pUserInfo->userName, "");
                }                
                break;
            case eFTPNAME:/*收到用户名应答*/
                if (230 == replyCode && 0 == XOS_StrCmp(pUserInfo->passWord, "")) //用户已登录，且密码为空
                {
                    pUserInfo->curEvent = eFTPTYPE;
                    FTP_SingleComSend(pUserInfo, "TYPE I", "", "");
                }
                else if (331 == replyCode)/*肯定应答就发送密码*/
                {
                    pUserInfo->curEvent = eFTPPASS;
                    FTP_SingleComSend(pUserInfo, "PASS ", pUserInfo->passWord, "");
                }
                break;
                
            case eFTPPASS:/*收到密码应答,发送取目录命令*/
                if (230 == replyCode)
                {
                    pUserInfo->curEvent = eFTPTYPE;
                    FTP_SingleComSend(pUserInfo, "TYPE I", "", "");
                }
                else //密码验证错误，登录失败 (可能是密码错误，也可能是权限不够，或者其它原因)
                {
                    /*停止链接定时器*/
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    /*释放信号量*/
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;                
                
            case eFTPTYPE:/*收到文件传输类型的应答*/
                if (200 == replyCode)/**/
                {
                    if (eFTPSTATEDISCON == pUserInfo->curState)/*上次断开后的再次连接*/
                    {
                        XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_RecvDataInd()->control link reconnect !\n");
                        pUserInfo->curEvent = eFTPCWD;
                        FTP_SingleComSend(pUserInfo, "CWD ", pUserInfo->curDir, "");
                    }
                    else
                    {
                        pUserInfo->curEvent = eFTPDEFDIR;
                        FTP_SingleComSend(pUserInfo, "PWD", "", "");
                    }
                }
                break;

            case eFTPCWD:/*收到改变当前目录的应答*/
                if (eFTPSTATEDISCON == pUserInfo->curState && 250 == replyCode)/*重连*/
                {
                    pUserInfo->curState = eFTPSTATELOGIN;
                    
                    /*pUserInfo->ret = XSUCC;*/
                }
                else if (eFTPSTATELOGIN == pUserInfo->curState)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    if (250 == replyCode )
                    {
                        pUserInfo->ret = XSUCC;
                    }
                    
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
            break;
                
            case eFTPDEFDIR:/*收到获取目录的应答*/
                if (257 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->curState = eFTPSTATELOGIN;
                    pUserInfo->ret = XSUCC;
                    
                    FTP_GetCurDir(pDataInd->pData, pDataInd->dataLenth, pUserInfo->defDir, sizeof(pUserInfo->defDir)-1);
                    
                    XOS_StrNcpy(pUserInfo->curDir, pUserInfo->defDir, XOS_MIN(XOS_StrLen(pUserInfo->defDir),XOS_StrLen(pUserInfo->defDir)));
                    pUserInfo->curDir[XOS_MIN(XOS_StrLen(pUserInfo->defDir),XOS_StrLen(pUserInfo->defDir))] = '\0';
                    
                    if(sizeof(pUserInfo->curDir) - XOS_StrLen(pUserInfo->curDir) < 3)
                    {
                        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvDataInd()->eFTPDEFDIR  error!\n");
                    }
                    else
                    {
                        XOS_StrNCat(pUserInfo->curDir, "/\0", 2);
                    }                    

                    /*登录成功，释放信号量*/
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
/*登录阶段*/
                
                
            case eFTPSIZE:/*收到获取文件大小的应答*/
                FTP_TIMER_STOP(pUserInfo->timerId);                
                if (213 == replyCode || 200 == replyCode)
                {
                    FTP_GetFileSize(pDataInd->pData, pDataInd->dataLenth, &(pUserInfo->fileSize));
                    pUserInfo->ret = XSUCC;
                }
                else if(550 == replyCode)
                {}
                
                XOS_SemPut(&(pUserInfo->semaphore));
                break;         
                
            case eFTPMKD:/*收到创建目录的应答*/
                FTP_TIMER_STOP(pUserInfo->timerId);
                if (257 == replyCode || 200 == replyCode)
                {
                    pUserInfo->ret = XSUCC;
                }
                XOS_SemPut(&(pUserInfo->semaphore));
                break;
                
            case eFTPRMD:/*收到删除目录的应答*/
                FTP_TIMER_STOP(pUserInfo->timerId);
                if (250 == replyCode || 200 == replyCode)
                {
                    pUserInfo->ret = XSUCC;
                }
                XOS_SemPut(&(pUserInfo->semaphore));
                break;
                
            case eFTPRMF:/*收到删除文件的应答*/
                if (250 == replyCode || 200 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else if (550 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
#ifdef FTP_PASV_MODE
            case eFTPPASV:/*收到被动方式请求的应答*/
                if (227 == replyCode)
                {
                    if (XERROR != FTP_GetPasvServerAddr(pDataInd->pData, &(pUserInfo->dataLinkAddr)))
                    {
                        pUserInfo->curEvent = pUserInfo->curTransfer;
                        /*                            pUserInfo->ret = XSUCC;*/
                        /*建立数据链路*/
                        if (XERROR == FTP_bulidDataLink(&(pUserInfo->dataLinkAddr), &(pUserInfo->dataSockId)))
                        {
                            XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvDataInd()->build data link error!\n");
                            break;
                        }
                        
                        /*发送上传下载文件的命令*/
                        switch (pUserInfo->curEvent)
                        {
                        case eFTPGETTOFILE:
                            FTP_SingleComSend(pUserInfo, "RETR ", pUserInfo->defDir, pUserInfo->RemFile);
                            break;
                        case eFTPGETTOMEM:
                            FTP_SingleComSend(pUserInfo, "RETR ", pUserInfo->defDir, pUserInfo->RemFile);
                            break;
                        case eFTPPUTFROMFILE:
                            FTP_SingleComSend(pUserInfo, "STOR ", pUserInfo->defDir, pUserInfo->RemFile);
                            break;
                        case eFTPPUTFROMMEM:
                            FTP_SingleComSend(pUserInfo, "STOR ", pUserInfo->defDir, pUserInfo->RemFile);
                            break;
                        case eFTPCREATFILE:
                            FTP_SingleComSend(pUserInfo, "STOR ", pUserInfo->RemFile, "");
                            break;
                        case eFTPLIST:
                            FTP_SingleComSend(pUserInfo, "LIST ", pUserInfo->curFolder, "");
                            break;
                            
                        default:
                            XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_RecvDataInd()::pUserInfo->curEvent as excepted %d!\n",pUserInfo->curEvent);
                            break;
                            
                        }
                    }
                }
                break;
#else
            case eFTPPORT:/*收到port命令的应答*/
                if (200 == replyCode)
                {
                    /*                        pUserInfo->curEvent = pUserInfo->curTransfer;*/
                    switch (pUserInfo->curEvent)
                    {
                    case eFTPGETTOFILE:
                        FTP_SingleComSend(pUserInfo, "RETR ", pUserInfo->defDir, pUserInfo->RemFile);
                        break;
                    case eFTPGETTOMEM:
                        FTP_SingleComSend(pUserInfo, "RETR ", pUserInfo->defDir, pUserInfo->RemFile);
                        break;
                    case eFTPPUTFROMFILE:
                        FTP_SingleComSend(pUserInfo, "STOR ", pUserInfo->defDir, pUserInfo->RemFile);
                        break;
                    case eFTPPUTFROMMEM:
                        FTP_SingleComSend(pUserInfo, "STOR ", pUserInfo->defDir, pUserInfo->RemFile);
                        break;
                    case eFTPCREATFILE:
                        FTP_SingleComSend(pUserInfo, "STOR ", pUserInfo->RemFile, "");
                        break;
                        
                    default:
                        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_RecvDataInd()::pUserInfo->curEvent as excepted %d!\n",pUserInfo->curEvent);
                        break;
                    }
                }
                break;
#endif
            case eFTPCREATFILE:/*收到创建文件的应答*/
                if (150 == replyCode || 125 == replyCode)  /*第一次响应*/
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else if (226 == replyCode)  /*第二次自动响应*/
                {
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else 
                {   
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XERROR;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
 
                break;
                
/*下载到文件成功*/                
            case eFTPGETTOFILE:
                if (150 == replyCode || 125 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                    if(XSUCC == FTP_GetSecondReplyCode(pDataInd->pData, pDataInd->dataLenth, &replyCode))
                    {
                        if(226 == replyCode)
                        {
                            XOS_Sleep(1000);
                            pUserInfo->ret = XSUCC;
                            XOS_SemPut(&(pUserInfo->semaphore)); 
                        }
                    }                    
                }
                else if (226 == replyCode)
                {
                    XOS_Sleep(100);
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else
                {
                   FTP_TIMER_STOP(pUserInfo->timerId);
                   pUserInfo->ret = XERROR;

                   XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
/*下载到内存成功*/                
            case eFTPGETTOMEM:
                if (150 == replyCode || 125 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                    if(XSUCC == FTP_GetSecondReplyCode(pDataInd->pData, pDataInd->dataLenth, &replyCode))
                    {
                        if(226 == replyCode)
                        {
                            XOS_Sleep(1000);
                            pUserInfo->ret = XSUCC;
                            XOS_SemPut(&(pUserInfo->semaphore));  
                        }
                    }
                }
                else if (226 == replyCode)
                {
                    XOS_Sleep(100);
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else
                {
                    FTP_TIMER_STOP(pUserInfo->timerId); 
                    pUserInfo->ret = XERROR;

                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
                
/*获取指定目录的文件列表成功*/                
            case eFTPLIST:
                if (150 == replyCode || 125 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    FTP_ReplySpecialParse(pDataInd->pData, pDataInd->dataLenth, pUserInfo);
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else if (226 == replyCode)
                {
                    XOS_Sleep(100);
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);    
                    pUserInfo->ret = XERROR;  
             
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;

/*从文件上传成功*/                
            case eFTPPUTFROMFILE:
                if (150 == replyCode || 125 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else if (226 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else
                {
                    FTP_TIMER_STOP(pUserInfo->timerId); 
                    pUserInfo->ret = XERROR;

                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
/*从内存上传成功*/                
            case eFTPPUTFROMMEM: /*上传文件的响应*/
                if (150 == replyCode || 125 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else if (226 == replyCode) /*文件上传成功响应*/
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);   
                    pUserInfo->ret = XERROR;

                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
                
            case eFTPRNCHECK:
                if (350 == replyCode) //重命名正在执行
                {
                    pUserInfo->curEvent = eFTPRNEXCUTE;
                    FTP_SingleComSend(pUserInfo, "RNTO ", pUserInfo->newFileName, "");
                }
                else
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XERROR;

                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;            
            case eFTPRNEXCUTE:
                if (250 == replyCode) //重命名成功
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XERROR;

                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
            case eFTPQUIT:
                if (221 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
            default:
                break;
            }
        }
    }
    XOS_MemFree(FID_FTP, pDataInd->pData);
}

/************************************************************************
 函数名:FTP_RecvStopInd
 功能:  处理链路关闭指示的消息
 输入:  pMsg            - ftp链路启动响应消息
 输出:  无
 返回:  无
 说明:
************************************************************************/
XSTATIC XVOID FTP_RecvStopInd(t_XOSCOMMHEAD *pMsg)
{
    XS32 index = 0;
    t_LINKCLOSEIND *pCloseInd = XNULLP;
    t_XOSFTPCLIENT *pUserInfo = XNULLP;

    if(NULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvStopInd() para is null !\n");
        return ;
    }
    
    pCloseInd = (t_LINKCLOSEIND*)(pMsg->message);
    index = (XS32)FTP_GetLinkIndex((XOS_HFTPCLT)(pCloseInd->appHandle));
    if (FTP_GetLinkType((XOS_HFTPCLT)(pCloseInd->appHandle)))/*控制链路关闭指示*/
    {
        pUserInfo = (t_XOSFTPCLIENT*)XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, index);
        if (XNULLP != pUserInfo)
        {
            if (pUserInfo->curState == eFTPSTATELOGIN)
            {
                pUserInfo->curState = eFTPSTATEDISCON;
            }/*修改当前状态为不可用*/

            pUserInfo->curEvent = eFTPNONE;
            
            /*处理在文件传输过程中的断开*/
            if (pUserInfo->curTransfer == eFTPGETTOFILE || pUserInfo->curTransfer == eFTPGETTOMEM ||
                pUserInfo->curTransfer == eFTPPUTFROMFILE || pUserInfo->curTransfer == eFTPPUTFROMMEM
                || eFTPLIST == pUserInfo->curTransfer)
            {
                XOS_SemPut(&(pUserInfo->semaphore));
            }
            if (pUserInfo->dataSockId.fd != XERROR)
            {
                XINET_CloseSock(&(pUserInfo->dataSockId));
            }
            
            XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvStopInd()->control link receive stop indicate !\n");
        }
    }
}

/************************************************************************
 函数名:FTP_RecvErrorSend
 功能:  处理链路数据发送错误的消息
 输入:  pMsg            - ftp链路数据发送响应消息
 输出:  无
 返回:  无
 说明:
************************************************************************/
XSTATIC XVOID FTP_RecvErrorSend(t_XOSCOMMHEAD *pMsg)
{
    t_SENDERROR *pErrorSend = XNULLP;

    if(NULL == pMsg)
    {
        return;
    }
    pErrorSend = (t_SENDERROR*)(pMsg->message);
    if(NULL == pErrorSend)
    {
        return;
    }
    
    XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvErrorSend()->receive error send, reason: %d!\n", pErrorSend->errorReson);
}

/************************************************************************
 函数名:FTP_RemoveDir
 功能:  删除目录
 输入:  pUserInfo            - ftp链路启动响应消息
               pFolderName       - 文件夹名
 输出:  无
 返回:  成功返回XSUCC,失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 FTP_RemoveDir( t_XOSFTPCLIENT *pUserInfo, XS8 *pFolderName )
{
    if(NULL == pUserInfo || NULL == pFolderName)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RemoveDir para is null");
        return XERROR;
    }
    /*发送删除目录的命令*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "RMD ", pUserInfo->curDir, pFolderName, eFTPRMD))
    {
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
 函数名:FTP_DeleteFile
 功能:  删除文件
 输入:  pUserInfo            - ftp链路启动响应消息
               pFileName            - 文件名
 输出:  无
 返回:  成功返回XSUCC,失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 FTP_DeleteFile( t_XOSFTPCLIENT *pUserInfo, XS8 *pFileName )
{
    if(NULL == pUserInfo || NULL == pFileName)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_DeleteFile para is null");
        return XERROR;
    }
    
    /*发送删除文件的命令*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "DELE ", pFileName, "", eFTPRMF))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_DeleteFile()-> Remove file fail !\n");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
 函数名:FTP_ListParser
 功能:  解析当前目录下的文件名
 输入:  pData            - 文件列表信息
 输出:  fName           - 文件名
 返回:  成功返回XSUCC,失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 FTP_ListParser(XS8* pData, XS8* fName)
{
    XS8 *pEnter = NULL;
    XS8 *pBlank = NULL;
    XS32 i = 0;

    if(NULL == pData ||NULL == fName)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ListParser() para is null !\n");
        return XERROR;
    }
        
    pEnter = XOS_StrStr(pData, "\r\n");
    if(!pEnter)
    {
        pEnter = XOS_StrStr(pData, "\n");
    }
    
    pBlank = pData;
    for (i=0; i<8; i++)/*确定文件名的起始位置*/
    {
        while (*pBlank != ' ' && pBlank < pEnter)
        {
            pBlank++;
        }
        if (pBlank >= pEnter)
        {
            return XERROR;
        }
        while (*pBlank == ' ')
        {
            pBlank++;
        }
    }
    if (pEnter - pBlank >= MAX_USERNAME_LEN)
    {
        return XERROR;
    }
    XOS_MemCpy(fName, pBlank, pEnter - pBlank);
    i = (XS32)(pEnter - pBlank);
    *(fName+i) = '\0';
    return XSUCC;
}

/************************************************************************
 函数名:FTP_ListToDele
 功能:  先获取文件列表,然后逐一删除文件和文件夹
 输入:  pUserInfo               - 用户信息
 输出:  pFolderName           - 文件夹名
 返回:  成功返回XSUCC,失败返回XERROR
 说明:
************************************************************************/
XSTATIC XS32 FTP_ListToDele(t_XOSFTPCLIENT *pUserInfo, XS8* pFolderName)
{
    XS8* pData = XNULLP, *pOffset = XNULLP;
    XS8* pLocation = XNULLP;
    XS32 dataLength = 0;
    XU32 i = 0;
    XS8 fName[MAX_USERNAME_LEN] = {0};
    XS8 fDir[MAX_DIRECTORY_LEN] = {0};
    XBOOL flag = XTRUE;
    t_FTPDATALINKENTRY tTskEntry;
    
    pUserInfo->ret = XERROR;
    pUserInfo->curTransfer = eFTPLIST;
    pUserInfo->bufferSize = 0;/*目录列表数据大小*/
    pUserInfo->pBuffer = XNULLP;/*目录列表数据*/
    pLocation = (XS8*)pUserInfo->curFolder;/*当前文件夹的路径(相对于当前目录)*/

    for (i=0; i<pUserInfo->count; i++)
    {
        pLocation = (XS8*)XOS_StrStr(pLocation, "/");
        pLocation++;
    }

    if (pLocation - (XS8*)pUserInfo->curFolder + XOS_StrLen(pFolderName) >= MAX_DIRECTORY_LEN-1)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ListToDele()-> Folder too deep !\n");
        return XERROR;
    }

    XOS_MemCpy(pLocation, pFolderName, XOS_StrLen(pFolderName)+1);
    pData = pLocation + XOS_StrLen(pFolderName);
    XOS_MemCpy(pData, "/\0", 2);
    i = (XU32)(pData + 1 - pUserInfo->curFolder);
    /*fDir删除操作时使用*/
    XOS_MemCpy(fDir, pUserInfo->curFolder, XOS_StrLen(pUserInfo->curFolder)+1);
    pUserInfo->count = pUserInfo->count + 1;/*目录深度*/
    
    /*被动方式请求*/
    pUserInfo->replyInfoType = ePREPARE_SUC;
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ListToDele()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*开始接收文件列表*/
    tTskEntry.curEvent = eFTPLIST;
    tTskEntry.pSockId = &(pUserInfo->dataSockId);
    tTskEntry.pSem = &(pUserInfo->semaphore);
    
    FTP_dataTskWithTimeOut(pUserInfo, &tTskEntry, 5);
    
    pUserInfo->curTransfer = eFTPNONE; /*传输任务完成*/
    
    /**/
    if (XSUCC != pUserInfo->ret && ePREPARE_SUC == pUserInfo->replyInfoType)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ListToDele()-> List file fail !\n");
        return XERROR;
    }
    else
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ListToDele()-> List file success %s !\n", tTskEntry.pBuffer);
    }

    pData = tTskEntry.pBuffer;
    dataLength = (XS32)tTskEntry.buffLen;
    if (dataLength<=2)
    {
        flag = XTRUE;
    }
    else if ((*(pData+dataLength-2) == '\r' && *(pData+dataLength-1) == '\n') || *(pData+dataLength-1) == '\n')
    {
        /*开始逐个解析文件列表的内容并进行删除操作*/
        pOffset = pData;
        while (pOffset - pData < dataLength)
        {
            switch (*pOffset)
            {
            case 'd':/*文件夹*/
                if (XERROR == FTP_ListParser(pOffset, fName))
                {
                    pOffset = pData + dataLength;
                    flag = XFALSE;
                    break;
                }
                if ( 0 ==XOS_StrCmp(fName, "." ) || 0 ==XOS_StrCmp(fName, ".." ))
                    break;
                XOS_MemCpy(fDir+XOS_StrLen(fDir), fName, XOS_StrLen(fName)+1);
                if (XERROR == FTP_RemoveDir(pUserInfo, fDir))
                {
                    if (XERROR == FTP_ListToDele(pUserInfo, fName) ||XERROR == FTP_RemoveDir(pUserInfo, fDir))
                    {
                        pOffset = pData + dataLength;
                        flag = XFALSE;
                        break;
                    }
                }
                break;
            case '-':/*文件*/
                if (XERROR == FTP_ListParser(pOffset, fName))
                {
                    pOffset = pData + dataLength;
                    flag = XFALSE;
                    break;
                }
                XOS_MemCpy(fDir+XOS_StrLen(fDir), fName, XOS_StrLen(fName)+1);
                if (XERROR == FTP_DeleteFile(pUserInfo, fDir))
                {
                    pOffset = pData + dataLength;
                    flag = XFALSE;
                    break;
                }
                break;
            default:
                break;
            }
            *(fDir + i) = '\0';
            do{pOffset++;}
            while (pOffset - pData < dataLength && !((*(pOffset-2) == '\r' && *(pOffset-1) == '\n')||(*(pOffset-1) == '\n')));
        }
    }
    else
    {
        flag = XFALSE;
    }
    
    *pLocation = '\0';
    pUserInfo->count = pUserInfo->count - 1;
    if(NULL != tTskEntry.pBuffer)
    {
        XOS_MemFree(FID_FTP, (XVOID*)(tTskEntry.pBuffer));
    }
    
    return (flag==XTRUE?XSUCC:XERROR);
}

/*------------------------------------------------------------------------------------------
                                  模块接口函数
-------------------------------------------------------------------------------------------*/
/************************************************************************
函数名:    XOS_FtpLogin
功能：登录ftp服务器
输入：serAddr           -ftp服务器ip
                userName         -用户登录帐号
                passwd             -用户登录密码
输出：hFtpClt             -ftp用户句柄
      该句柄既作为ntl链路的用户ID;也作为ftp的用户ID.两者是一致的.
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpLogin(t_IPADDR *serAddr, XS8 *userName, XS8 *passWd, XOS_HFTPCLT *hFtpClt)
{
    XS32 userId = 0;
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_PARA timerPara;
    t_BACKPARA backPara;
    
    /*参数检验*/
    if (XFALSE == g_ftpMnt.initialized || XNULLP == serAddr || XNULLP == userName || 0 ==XOS_StrCmp(userName, "" ) || XNULLP == passWd ||
        XNULLP == hFtpClt || XOS_StrLen(userName)>=MAX_USERNAME_LEN || XOS_StrLen(passWd)>=MAX_PASSWORD_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR),"XOS_FtpLogin()-> Bad input param !\n");
        return XERROR;
    }
    
    /*记录用户信息,分配用户空间，并返回用户ID，加入到句柄中*/
    XOS_MutexLock(&(g_ftpMnt.contrlTblLock));
    userId = XOS_ArrayAddExt(g_ftpMnt.ftpConnectTbl, (XVOID**)&pUserInfo);
    XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));

    /*分配内存失败*/
    if (XNULLP == pUserInfo)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR),"XOS_FtpLogin()-> too many ftp connection !\n");
        return XERROR;
    }
    XOS_MemSet(pUserInfo, 0, sizeof(t_XOSFTPCLIENT));
    
    memcpy(pUserInfo->userName, userName, XOS_MIN(XOS_StrLen(userName), sizeof(pUserInfo->userName)-1));
    memcpy(pUserInfo->passWord, passWd, XOS_MIN(XOS_StrLen(passWd), sizeof(pUserInfo->passWord)-1));
    XOS_MemCpy(&(pUserInfo->dstAddr), serAddr, sizeof(t_IPADDR));
    
    /*创建用户信号量*/
    pUserInfo->ret = XERROR;
    pUserInfo->dataSockId.fd = XERROR;
    if (XERROR == XOS_SemCreate(&(pUserInfo->semaphore), 0))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR),"XOS_FtpLogin()-> create semaphore fail!\n");
        goto ftpparaError;
    }
    
    /*启动连接定时器*/
    FTP_TimerSet(pUserInfo, &timerPara, &backPara, FTP_TCPCONNECT_TIME);
    XOS_INIT_THDLE(pUserInfo->timerId);
    if (XERROR == XOS_TimerStart(&(pUserInfo->timerId), &timerPara, &backPara))
    {
        XOS_SemDelete(&(pUserInfo->semaphore));
        XOS_Trace(MD(FID_FTP, PL_ERR),"XOS_FtpLogin()-> timer start fail!\n");
        goto ftpparaError;
    }
    
    /*控制链路初始化*/
    if (XSUCC != FTP_LinkInit(userId, eTCPClient, XTRUE))
    {
        XOS_SemDelete(&(pUserInfo->semaphore));
        FTP_TIMER_STOP(pUserInfo->timerId);
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpLogin()-> FTP_LinkInit send fail !\n");
        goto ftpLoginError;
    }

    /*等待信号量可用，由登录成功释放信号量*/
    XOS_SemGet(&(pUserInfo->semaphore));

    /*登录失败，由超时接口释放信号量*/
    if (eFTPSTATELOGIN != pUserInfo->curState || XSUCC != pUserInfo->ret)
    {
        XOS_SemDelete(&(pUserInfo->semaphore));
        FTP_TIMER_STOP(pUserInfo->timerId);
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpLogin()-> Login fail !\n");
        
        goto  ftpLoginError;
    }
    
    *hFtpClt = FTP_BuildLinkH( XTRUE, (XU16)userId );
    pUserInfo->curEvent = eFTPNONE;
    return XSUCC;
    
ftpLoginError: /*失败时，释放所有资源*/

    /*如果已发起过TCP连接，则释放连接, 如果ntl消息失败也要考虑*/
    //if (pUserInfo->curState != eFTPSTATEINIT || 1)
    {
        if(eFTPSTATELOGIN == pUserInfo->curState) /*say goodbye*/
        {
            if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "QUIT", "", "", eFTPQUIT))
            {
                XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpClose()-> send QUIT fail!\n");
            }
        }
        
        /*释放链路*/
        FTP_LinkRelease(pUserInfo->linkHandle);
    }

ftpparaError:
    /*删除用户资源*/
    XOS_MutexLock(&(g_ftpMnt.contrlTblLock));
    XOS_ArrayDeleteByPos(g_ftpMnt.ftpConnectTbl, userId);
    XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
    
    return XERROR;
}

/************************************************************************
函数名:    XOS_FtpGetToFile
功能：从ftp下载一个文件到本地磁盘
输入：hFtpClt              -ftp用户句柄
                remPath            -服务器文件路径
                remFile             -服务器文件名
                locPath             -本地文件路径
                locFile               -本地文件名
                time                  -下载文件允许的最长时间,单位(秒),0表示没限制
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpGetToFile( XOS_HFTPCLT hFtpClt, XS8 *pRemFile, XS8 *pLocFile, XU32 time)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_FTPDATALINKENTRY tTskEntry;
    XS32 nLen = 0;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pRemFile || XNULLP == pLocFile
        || 0 ==XOS_StrCmp(pRemFile, "" ) || 0 ==XOS_StrCmp(pLocFile, "" ))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToFile()-> Bad input param !\n");
        return XERROR;
    }
    
    /*以追加方式打开文件*/
    if (XNULL == (pUserInfo->pFile = XOS_OpenFile(pLocFile, XF_WBMODE)))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToFile()-> Open file fail !\n");
        return XERROR;
    }
    
    pUserInfo->replyInfoType = ePREPARE_SUC;
    pUserInfo->curTransfer = eFTPGETTOFILE;

    nLen = (XS32)XOS_MIN(sizeof(pUserInfo->RemFile)-1, XOS_StrLen(pRemFile));    
    XOS_MemCpy(pUserInfo->RemFile, pRemFile, nLen);
    pUserInfo->RemFile[nLen] = '\0';
    
    /*被动方式请求*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_CloseFile(&(pUserInfo->pFile));/*异常时关闭文件*/
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToFile()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*开始下载文件到磁盘文件*/
    tTskEntry.curEvent = eFTPGETTOFILE;
    tTskEntry.pSockId = &(pUserInfo->dataSockId);
    tTskEntry.pFile = pUserInfo->pFile;
    tTskEntry.pSem = &(pUserInfo->semaphore);
    tTskEntry.ulDownLen = 0;
    FTP_dataTskWithTimeOut(pUserInfo, &tTskEntry, time);

    XOS_CloseFile(&(tTskEntry.pFile));

    /*比较文件下载的大小*/
    if((XUFILESIZE)(pUserInfo->fileSize) != tTskEntry.ulDownLen)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpGetToFile()->Down file failed !\n");    
        return XERROR;
    }
    
    pUserInfo->curTransfer = eFTPNONE;
    if (XSUCC != pUserInfo->ret)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToFile()-> Get file fail !\n");
        XOS_DeleteFile(pLocFile);
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpGetToMem
功能：从ftp下载一个文件到本地内存
输入：hFtpClt              -ftp用户句柄
                remPath            -服务器文件路径
                remFile             -服务器文件名
                filePtr               -本地文件路径
                memSize          -本地文件名
                time                  -下载文件允许的最长时间,单位(秒),0表示没限制
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpGetToMem( XOS_HFTPCLT hFtpClt, XS8 *pRemFile, XS8 *pBuff, XU32 buffSize, XU32 time)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_FTPDATALINKENTRY tTskEntry;
    int nLen = 0;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pRemFile || XNULLP == pBuff
        || 0 == buffSize || 0 ==XOS_StrCmp(pRemFile, "" ))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToMem()-> Bad input param !\n");
        return XERROR;
    }
    
    pUserInfo->replyInfoType = ePREPARE_SUC;
    pUserInfo->curTransfer = eFTPGETTOMEM;
    pUserInfo->count = 0;
    pUserInfo->bufferSize = buffSize;
    pUserInfo->pBuffer = pBuff;

    nLen = (XS32)XOS_MIN(sizeof(pUserInfo->RemFile)-1, XOS_StrLen(pRemFile));        
    XOS_MemCpy(pUserInfo->RemFile, pRemFile, nLen);
    pUserInfo->RemFile[nLen] = '\0';
    /*被动方式请求*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToMem()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*开始下载文件到内存*/
    tTskEntry.curEvent = eFTPGETTOMEM;
    tTskEntry.pSockId = &(pUserInfo->dataSockId);
    tTskEntry.buffLen = pUserInfo->bufferSize;
    tTskEntry.pBuffer = pUserInfo->pBuffer;
    tTskEntry.pSem = &(pUserInfo->semaphore);    
    FTP_dataTskWithTimeOut(pUserInfo, &tTskEntry, time);
    
    pUserInfo->curTransfer = eFTPNONE;
    if (XSUCC != pUserInfo->ret)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToMem()-> Get file fail !\n");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpPutFromFile
功能：将本地磁盘的文件上传到ftp服务器
输入：hFtpClt              -ftp用户句柄
                locPath             -本地文件路径
                locFile               -本地文件名
                remPath            -服务器文件路径
                remFile             -服务器文件名
                time                  -上传文件允许的最长时间,单位(秒),0表示没限制
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpPutFromFile( XOS_HFTPCLT hFtpClt, XS8 *pLocFile, XS8 *pRemFile, XU32 time)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_FTPDATALINKENTRY tTskEntry;
    int nLen = 0;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pRemFile || XNULLP == pLocFile
        || 0 ==XOS_StrCmp(pLocFile, "" ) || 0 ==XOS_StrCmp(pRemFile, "" ))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpPutFromFile()-> Bad input param !\n");
        return XERROR;
    }
    
    if (XNULL == (pUserInfo->pFile = XOS_OpenFile(pLocFile, XF_RBMODE)))
    {
        XOS_Trace(MD(FID_FTP,PL_WARN), "XOS_FtpPutFromFile()->Open file error!\n");
        return XERROR;
    }
    pUserInfo->replyInfoType = ePREPARE_SUC;
    pUserInfo->count = 0;
    XOS_FileLen(pLocFile, &(pUserInfo->bufferSize));
    
    nLen = (XS32)XOS_MIN(sizeof(pUserInfo->RemFile)-1,XOS_StrLen(pRemFile));
    XOS_MemCpy(pUserInfo->RemFile, pRemFile, nLen);
    pUserInfo->RemFile[nLen] = '\0';
    
    pUserInfo->curTransfer = eFTPPUTFROMFILE;
    pUserInfo->retOfPut = XSUCC;
    
    /*获取建立数据连接时对端的ip和port*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_CloseFile(&(pUserInfo->pFile));  /*异常时关闭文件*/
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpPutFromFile()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*开始上传磁盘文件*/
    tTskEntry.curEvent = eFTPPUTFROMFILE;
    tTskEntry.pSockId = &(pUserInfo->dataSockId);
    tTskEntry.pFile = pUserInfo->pFile;
    tTskEntry.buffLen = pUserInfo->bufferSize;
    tTskEntry.pSem = &(pUserInfo->semaphore);
    
    FTP_dataTskWithTimeOut(pUserInfo, &tTskEntry, time);
    
    XOS_CloseFile(&(pUserInfo->pFile));
    pUserInfo->curTransfer = eFTPNONE;
    
    if (XSUCC != pUserInfo->ret || XSUCC != pUserInfo->retOfPut)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpPutFromFile()-> Store file fail !\n");
        XOS_FtpDeleteFile( hFtpClt,  pRemFile);/*删除ftp服务器上的文件*/
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpPutFromMem
功能：将本地内存的文件上传到ftp服务器
输入：hFtpClt              -ftp用户句柄
                filePtr                -本地内存文件指针
                fileSize              -本地内存文件大小
                remPath            -服务器文件路径
                remFile             -服务器文件名
                time                  -上传文件允许的最长时间,单位(秒),0表示没限制
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpPutFromMem( XOS_HFTPCLT hFtpClt, XS8 *pBuff, XU32 buffSize, XS8 *pRemFile, XU32 time)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_FTPDATALINKENTRY tTskEntry;
    int nLen = 0;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pRemFile || XNULLP == pBuff
        || 0 == buffSize || 0 ==XOS_StrCmp(pRemFile, "" ))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToMem()-> Bad input param !\n");
        return XERROR;
    }
    
    pUserInfo->ret = XERROR;
    pUserInfo->replyInfoType = ePREPARE_SUC;
    pUserInfo->count = 0;
    pUserInfo->curTransfer = eFTPPUTFROMMEM;
    pUserInfo->retOfPut = XSUCC;
    pUserInfo->pBuffer = pBuff;
    pUserInfo->bufferSize = buffSize;

    nLen = (XS32)XOS_MIN(sizeof(pUserInfo->RemFile)-1, XOS_StrLen(pRemFile));
    XOS_MemCpy(pUserInfo->RemFile, pRemFile, nLen);
    pUserInfo->RemFile[nLen] = '\0';
    
    /*被动方式请求*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpPutFromMem()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*开始上传内存文件*/
    tTskEntry.curEvent = eFTPPUTFROMMEM;
    tTskEntry.pSockId = &(pUserInfo->dataSockId);
    tTskEntry.pBuffer= pUserInfo->pBuffer;
    tTskEntry.buffLen = pUserInfo->bufferSize;
    tTskEntry.pSem = &(pUserInfo->semaphore);
    FTP_dataTskWithTimeOut(pUserInfo, &tTskEntry, time);
    
    pUserInfo->curTransfer = eFTPNONE;
    if (XSUCC != pUserInfo->ret || XSUCC != pUserInfo->retOfPut)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToMem()-> Store file fail !\n");
        XOS_FtpDeleteFile( hFtpClt,  pRemFile);
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpCurrentWorkDir
功能：取得当前工作目录
输入：hFtpClt                     -ftp用户句柄
输出：pWorkDir                  -当前工作目录
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpCurrentWorkDir( XOS_HFTPCLT hFtpClt, XVOID **pWorkDir)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pWorkDir)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpCurrentWorkDir()-> Bad input param !\n");
        return XERROR;
    }
    /*返回当前目录*/
    *pWorkDir = (XS8*)(pUserInfo->curDir) + XOS_StrLen(pUserInfo->defDir);
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpChangeWorkDir
功能：改变当前工作目录
输入：hFtpClt                    -ftp用户句柄
                pWorkDir                -新的当前工作目录
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpChangeWorkDir( XOS_HFTPCLT hFtpClt, XS8 *pWorkDir )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    int len = 0;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pWorkDir || 0 == XOS_StrCmp(pWorkDir, "") ||
        XOS_StrLen(pWorkDir)+XOS_StrLen(pUserInfo->defDir) >= MAX_DIRECTORY_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpChangeWorkDir()-> Bad input param !\n");
        return XERROR;
    }    

    
    /*发送改变目录的命令*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "CWD ", pUserInfo->defDir, pWorkDir, eFTPCWD))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpChangeWorkDir()-> Change current work directory fail !\n");
        return XERROR;
    }
    
    /*修改当前工作目录*/    
    len = (XS32)XOS_StrLen(pUserInfo->defDir) + (XS32)XOS_StrLen(pWorkDir);
    if(len < sizeof(pUserInfo->curDir))
    {
        XOS_Sprintf(pUserInfo->curDir, sizeof(pUserInfo->curDir)-1, "%s%s", pUserInfo->defDir, pWorkDir);
        pUserInfo->curDir[len] = '\0';

        if(pUserInfo->curDir[len-1] != '/' && len+1 < sizeof(pUserInfo->curDir))/*如果是win32服务器会有问题*/
        {
            pUserInfo->curDir[len] = '/';
            pUserInfo->curDir[len+1] = '\0';            
        }
        else
        {
            XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpChangeWorkDir()-> current work directory fail !\n");
        }
    }
    
    
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpGetFileSize
功能：获得ftp服务器上一个文件的大小
输入：hFtpClt             -ftp用户句柄
                pRemFile           -服务器文件
输出：N/A
返回：成功则返回文件大小
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpGetFileSize( XOS_HFTPCLT hFtpClt, XS8 *pRemFile )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    /*参数检验*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pRemFile || 0 == XOS_StrCmp(pRemFile, ""))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetFileSize()-> Bad input param !\n");
        return XERROR;
    }
    
    /*发送获取文件大小的命令*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "SIZE ", pRemFile, "", eFTPSIZE))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetFileSize()-> Get file size fail !\n");
        return XERROR;
    }
    
    return (XS32)pUserInfo->fileSize;
}

/************************************************************************
函数名:    XOS_FtpMakeDir
功能：在当前工作目录下创建一个新的文件夹
输入：hFtpClt                    -ftp用户句柄
                pNewFolder                -要创建的文件夹名
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpMakeDir( XOS_HFTPCLT hFtpClt, XS8 *pNewFolder )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pNewFolder ||
        0 == XOS_StrCmp(pNewFolder, "") || XOS_StrLen(pNewFolder) >= MAX_USERNAME_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpMakeDir()-> Bad input param !\n");
        return XERROR;
    }
    
    /*发送创建目录的命令*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "MKD ", pNewFolder, "", eFTPMKD))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpMakeDir()-> Make directory fail !\n");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpRemoveDir
功能：在当前工作目录下删除一个文件夹
输入：hFtpClt                    -ftp用户句柄
                pFolderName              -要删除的文件夹名
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpRemoveDir( XOS_HFTPCLT hFtpClt, XS8 *pFolderName )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    /*参数检验*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pFolderName ||
        0 == XOS_StrCmp(pFolderName, "") || XOS_StrLen(pFolderName) >= MAX_USERNAME_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpRemoveDir()-> Bad input param !\n");
        return XERROR;
    }
    
    pUserInfo->count = 0;/*刚开始目录深度为0*/
    XOS_MemSet(pUserInfo->curFolder, 0, MAX_DIRECTORY_LEN);
    if (XERROR == FTP_RemoveDir(pUserInfo, pFolderName))
    {
        if (XERROR == FTP_ListToDele(pUserInfo, pFolderName))
        {
            XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpRemoveDir()-> Remove directory fail !\n");
            return XERROR;
        }
        return FTP_RemoveDir(pUserInfo, pFolderName);
    }
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpRenameDir
功能：改变当前工作目录下一个文件夹的名字
输入：hFtpClt                         -ftp用户句柄
                pOldFolder                      -旧的文件夹名
                pNewFolder                     -新的文件夹名
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpRenameDir( XOS_HFTPCLT hFtpClt, XS8* pOldFolder, XS8 *pNewFolder )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    int nLen = 0;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pOldFolder || XNULLP == pNewFolder ||
        0 == XOS_StrCmp(pOldFolder, "") || 0 == XOS_StrCmp(pNewFolder, ""))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpRenameDir()-> Bad input param !\n");
        return XERROR;
    }

    nLen = (XS32)XOS_MIN(XOS_StrLen(pNewFolder),sizeof(pUserInfo->newFileName)-1);

    XOS_MemCpy(pUserInfo->newFileName, pNewFolder, nLen);
    pUserInfo->newFileName[nLen] = '\0';
    
    /*发送重命名目录的命令*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "RNFR ", pOldFolder, "", eFTPRNCHECK))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpRenameDir()-> Rename directory fail !\n");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpCreatFile
功能：在当前工作目录下创建一个新的文件
输入：hFtpClt                    -ftp用户句柄
                pFileName                  -要创建的文件的名字
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpCreatFile( XOS_HFTPCLT hFtpClt, XS8 *pFileName )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    int nLen = 0;
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pFileName ||
        0==XOS_StrCmp(pFileName, "") || XOS_StrLen(pFileName) >= MAX_USERNAME_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpCreatFile()-> Bad input param !\n");
        return XERROR;
    }
    
    pUserInfo->ret = XERROR;

    nLen = (XS32)XOS_MIN(sizeof(pUserInfo->RemFile)-1,XOS_StrLen(pFileName));
    XOS_MemCpy(pUserInfo->RemFile, pFileName, nLen);
    pUserInfo->RemFile[nLen] = '\0';
    
    pUserInfo->curTransfer = eFTPCREATFILE;
    
    /*获取建立数据连接时对端的ip和port*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpCreatFile()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*开始上传磁盘文件,这里直接关闭链路,表示上传文件已完成*/
    XINET_CloseSock(&(pUserInfo->dataSockId));
    pUserInfo->dataSockId.fd = XERROR;
    XOS_SemGet(&(pUserInfo->semaphore));
    
    if (XSUCC != pUserInfo->ret)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpCreatFile()-> Creat file fail !\n");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
函数名:    XOS_FtpDeleteFile
功能：在当前工作目录下删除一个存在的文件
输入：hFtpClt                    -ftp用户句柄
                pFileName                  -当前工作目录下的一个文件的名字
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpDeleteFile( XOS_HFTPCLT hFtpClt, XS8 *pFileName )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    if(NULL == pFileName)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpDeleteFile()-> Para is null!\n");
        return XERROR;
    }
    
    /*参数验证*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pFileName || 0==XOS_StrCmp(pFileName, ""))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpDeleteFile()-> Bad input param !\n");
        return XERROR;
    }
    
    return FTP_DeleteFile(pUserInfo, pFileName);
}

/************************************************************************
函数名:    XOS_FtpRenameFile
功能：将当前工作目录下的一个文件改名
输入：hFtpClt                    -ftp用户句柄
                pOldFileName             -当前工作目录下的一个文件的名字
                pNewFileName           -新的文件名
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpRenameFile( XOS_HFTPCLT hFtpClt, XS8 *pOldFileName, XS8 *pNewFileName )
{
    if(NULL == pOldFileName || NULL == pNewFileName)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpRenameFile()-> Bad input param !\n");
        return XERROR;
    }
    
    return XOS_FtpRenameDir( hFtpClt, pOldFileName, pNewFileName);
}

/************************************************************************
函数名:    XOS_FtpClose
功能：与ftp服务器断开控制连接,退出登录
输入：hFtpClt                    -ftp用户句柄
输出：N/A
返回：成功则返回XSUCC
          失败则返回XERROR
说明：
************************************************************************/
XPUBLIC XS32 XOS_FtpClose( XOS_HFTPCLT *hFtpClt )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    XS32 userId = 0;
    
    /*参数验证*/
    if (XNULLP == hFtpClt || XFALSE == FTP_IsValidLinkH(*hFtpClt))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpClose()-> Bad input param !\n");
        return XERROR;
    }
    
    userId = FTP_GetLinkIndex(*hFtpClt);
    if (XNULLP == (pUserInfo = XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, userId)))
    {
        return XERROR;
    }
    
    if (eFTPSTATELOGIN == pUserInfo->curState)
    {
        if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "QUIT", "", "", eFTPQUIT))
        {
            XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpClose()-> send QUIT fail!\n");
        }
    }

    /*释放链路*/
    FTP_LinkRelease(pUserInfo->linkHandle);
    /*此时，如果数据链路已建立，需要关闭数据链路*/
    if(XOS_INET_INV_SOCKFD != pUserInfo->dataSockId.fd)
    { 
        XINET_CloseSock(&pUserInfo->dataSockId);
    }    
    
    XOS_MutexLock(&(g_ftpMnt.contrlTblLock));
    XOS_SemDelete(&(pUserInfo->semaphore));
    XOS_ArrayDeleteByPos(g_ftpMnt.ftpConnectTbl, userId);
    XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
    *hFtpClt = XNULLP;
    return XSUCC;
}

/************************************************************************
 函数名:XOS_ListFile
 功能:  获取目录中的文件列表。add by lyp for 计费台
 输入: 无
 输出: 无
 返回: 无
 说明:
************************************************************************/
XPUBLIC XS32 XOS_ListFile( XOS_HFTPCLT hFtpClt, XS8 *pFolderName, XS8 **ppData )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    XS8 *pTmp = XNULLP;
    XS8* pData = XNULLP,*pOffset = XNULLP;
    XS8* pLocation = XNULLP;
    XSFILESIZE dataLength = 0;
    XU32 i = 0;
    XS8 fName[MAX_USERNAME_LEN] = {0};
    XS8 fDir[MAX_DIRECTORY_LEN] = {0};
    XBOOL flag = XTRUE;
    t_FTPDATALINKENTRY tTskEntry;
    
    /*参数检验*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pFolderName ||
        0 == XOS_StrCmp(pFolderName, "") || XOS_StrLen(pFolderName) >= MAX_USERNAME_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_ListFile()-> Bad input param !\n");
        return XERROR;
    }
    
    XOS_MemSet(fName,0x0,sizeof(fName));
    XOS_MemSet(fDir,0x0,sizeof(fDir));
    XOS_MemSet(&tTskEntry,0x0,sizeof(tTskEntry));
    
    pUserInfo->ret = XERROR;
    pUserInfo->curTransfer = eFTPLIST;
    pUserInfo->bufferSize = 0;/*目录列表数据大小*/
    pUserInfo->pBuffer = XNULLP;/*目录列表数据*/
    pLocation = (XS8*)pUserInfo->curFolder;/*当前文件夹的路径(相对于当前目录)*/
    for (i=0; i<pUserInfo->count; i++)
    {
        pLocation = (XS8*)XOS_StrStr(pLocation, "/");
        pLocation++;
    }
    if (pLocation - (XS8*)pUserInfo->curFolder + XOS_StrLen(pFolderName) >= MAX_DIRECTORY_LEN-1)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_ListFile()-> Folder too deep!");
        return XERROR;
    }
    
    XOS_MemCpy(pLocation, pFolderName, XOS_StrLen(pFolderName)+1);
    pData = pLocation + XOS_StrLen(pFolderName);
    XOS_MemCpy(pData, "/\0", 2);
    i = (XU32)(pData + 1 - pUserInfo->curFolder);
    /*fDir删除操作时使用*/
    XOS_MemCpy(fDir, pUserInfo->curFolder, XOS_StrLen(pUserInfo->curFolder)+1);
    pUserInfo->count++;/*目录深度*/
    
    /*被动方式请求*/
    pUserInfo->replyInfoType = ePREPARE_SUC;
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_ListFile()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*开始接收文件列表*/
    tTskEntry.curEvent = eFTPLIST;
    tTskEntry.pSockId = &(pUserInfo->dataSockId);
    tTskEntry.pSem = &(pUserInfo->semaphore);
    FTP_dataTskWithTimeOut(pUserInfo, &tTskEntry, 5);
    
    if (pUserInfo->replyInfoType == ePREPARE_SUC)
    {
        XOS_SemGet(&(pUserInfo->semaphore));
    }
    
    if (XSUCC != pUserInfo->ret && ePREPARE_SUC == pUserInfo->replyInfoType)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_ListFile()-> List file fail !\n");
        return XERROR;
    }
    pData = tTskEntry.pBuffer;
    dataLength = tTskEntry.buffLen;
    if(dataLength > FTP_FILE_LIST_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_ListFile()-> receive buffer len %d error!",dataLength);
        return XERROR;
    }
    //(*ppData) = XOS_MemMalloc(FID_FTP,dataLength +1 );
    (*ppData) = malloc((XS32)dataLength +1 );
    pTmp = *ppData;
    *pTmp = '\0';
    //XOS_MemCpy(*ppData,pData,dataLength +1);
    
    if (dataLength<=2)
    {
        flag = XTRUE;
    }
    else if (*(pData+dataLength-2) == '\r' && *(pData+dataLength-1) == '\n')
    {
        /*开始逐个解析文件列表的内容并进行删除操作*/
        pOffset = pData;
        while (pOffset - pData < dataLength)
        {
            switch (*pOffset)
            {
            case 'd':/*文件夹*/
                break;
            case '-':/*文件*/
                if (XERROR == FTP_ListParser(pOffset, fName))
                {
                    pOffset = pData + dataLength;
                    flag = XFALSE;
                    break;
                }
                /*将文件名合成一个带分隔符的字符串*/
                XOS_StrNCat(pTmp,fName,XOS_StrLen(fName));
                XOS_StrNCat(pTmp,"/",1);
                break;
            default:
                break;
            }
            *(fDir + i) = '\0';
            do
            {
                pOffset++;
            }
            while (pOffset - pData < dataLength && !(*(pOffset-2) == '\r' && *(pOffset-1) == '\n'));
        }
        
    }
    else
    {
        flag = XFALSE;
    }
    
    *pLocation = '\0';
    pUserInfo->count = pUserInfo->count - 1;
    XOS_MemFree(FID_FTP, (XVOID*)(tTskEntry.pBuffer));
    return (flag==XTRUE?XSUCC:XERROR);
}

/************************************************************************
 函数名:XOS_FtpInit
 功能:  ftp初始化处理函数入口
 输入: 无
 输出: 无
 返回: 无
 说明:
************************************************************************/
XS8 XOS_FtpInit( XVOID *t, XVOID *v )
{

#ifdef FTP_TEST
    t_PARA timerpara;
    t_BACKPARA backpara = {0};
#endif    

        
    XOS_UNUSED(t);
    XOS_UNUSED(v);
    
    /*创建互斥量*/
    if ( XSUCC != XOS_MutexCreate(&(g_ftpMnt.contrlTblLock)))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpInit()-> create mutex lock failed!\n");
        return XERROR;
    }
    
    /* 创建和链路相关的资源*/
    g_ftpMnt.ftpConnectTbl = XOS_ArrayConstruct(sizeof(t_XOSFTPCLIENT), MAX_FTPCONNECTIONS, "ftpClient");
    if (g_ftpMnt.ftpConnectTbl == XNULLP)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpInit()-> construct array fail!\n");
        XOS_MutexDelete(&(g_ftpMnt.contrlTblLock));
        return XERROR ;
    }
    
    if (XSUCC != XOS_TimerReg(FID_FTP, 500, MAX_TIMER_COUNT, 0))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpInit()-> register timer error!\n");
        XOS_MutexDelete(&(g_ftpMnt.contrlTblLock));
        XOS_ArrayDestruct(g_ftpMnt.ftpConnectTbl);
        return XERROR;
    }
    
    g_ftpMnt.initialized = XTRUE;
    XOS_FtpInitMsgQueue();
    XOS_FtpInitReMsgQueue();
    XOS_FtpInitMsgDealTaskPool();

    FTP_CliInit();

#ifdef FTP_TEST
    timerpara.fid = FID_FTP;
    timerpara.len = MEDIA_SEND_RTP_TIMER_UNIT;
    timerpara.mode = TIMER_TYPE_ONCE;
    timerpara.pre  = TIMER_PRE_LOW;
    backpara.para2 = MEDIA_TIMER_MSG_FTP_SEND;
    if (XSUCC != XOS_TimerStart(&FtpTestTimer, &timerpara, &backpara))
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "ftp start test timer error!");
        return XERROR;
    }
#endif        
    return XSUCC;
}
/************************************************************************
 函数名:XOS_FtpInitMsgQueue
 功能:  ftp 模块消息队列初始化
 输入:  pBackPara －定时器指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 XOS_FtpInitMsgQueue()
{
    /*创建消息队列*/
    gFtpMsgQueue = XOS_listConstruct(sizeof(t_XOSCOMMHEAD), gFtpMsgQueueSize, "gFtpMsgQueue");

    if(XNULL == gFtpMsgQueue)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "gFtpMsgQueue: init    table failed");
        return XERROR;
    }

    XOS_listClear(gFtpMsgQueue);
    
    XOS_listSetCompareFunc(gFtpMsgQueue, (nodeCmpFunc)NULL);/*设置比较函数为空*/

    /*创建消息队列临界区*/
    if ( XSUCC != XOS_MutexCreate( &gFtpMsgQueueMutex) )
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "XOS_MutexCreate gFtpMsgQueueMutex failed!");
        return XERROR;
    }

    /*创建同步信号量*/
    if (XERROR == XOS_SemCreate(&(gFtpQuSemaphore), 0))
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "XOS_MutexCreate XOS_SemCreate failed!");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
 函数名:XOS_FtpInitReMsgQueue
 功能:  ftp 模块重发消息队列初始化
 输入:  
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 XOS_FtpInitReMsgQueue()
{
    /*创建消息队列*/
    gFtpReMsgQueue = XOS_listConstruct(sizeof(t_ReFtpMsg), gFtpReMsgQueueSize, "gFtpMsgQueue");

    if(XNULL == gFtpReMsgQueue)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "gFtpReMsgQueue: init    table failed");
        return XERROR;
    }

    XOS_listClear(gFtpReMsgQueue);
    
    XOS_listSetCompareFunc(gFtpReMsgQueue, (nodeCmpFunc)NULL);/*设置比较函数为空*/

    /*创建消息队列临界区*/
    if ( XSUCC != XOS_MutexCreate( &gFtpReMsgQueueMutex) )
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "XOS_MutexCreate gFtpReMsgQueueMutex failed!");
        return XERROR;
    }

    /*创建同步信号量*/
    if (XERROR == XOS_SemCreate(&(gFtpReQuSemaphore), 0))
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "XOS_MutexCreate XOS_SemCreate failed!");
        return XERROR;
    }

    return XSUCC;
}


/************************************************************************
 函数名:XOS_FtpAddMsgQueue
 功能:  根据指针增加队列中的元素
 输入:  pBackPara －定时器指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 XOS_FtpAddMsgQueue(t_XOSCOMMHEAD *pMsg)
{
    XS32 pos = 0;
    t_XOSCOMMHEAD *pHead = NULL;
    if(XNULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpAddMsgQueue: pMsg is NULL");
        return FTP_QUE_NULL;
    }

    /*队列满*/
    if(XOS_listCurSize(gFtpMsgQueue) == XOS_listMaxSize(gFtpMsgQueue))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "gFtpMsgQueue  is full");
        return FTP_QUE_FULL;
    }
    
    pos = XOS_listAddTail(gFtpMsgQueue, pMsg);
    if(0 > pos)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpAddMsgQueue: XOS_listAddTail failed ");
        return FTP_QUE_FAI;
    }
    else /*如果增加节点成功，则开辟message指针内容*/
    {
        /*获取节点*/
        pHead = (t_XOSCOMMHEAD*)XOS_listGetElem(gFtpMsgQueue, pos);
        /*分配内存,并拷贝消息*/
        if(pHead)
        {
            pHead->message = XOS_FtpMallocMsgMem(pMsg->msgID, pMsg->message);    
        }
        else
        {
             XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpAddMsgQueue: XOS_listGetElem failed ");
        }
    }

    return FTP_QUE_SUC;

}
/************************************************************************
 函数名:XOS_FtpAddReMsgQueue
 功能:  根据指针增加重发队列中的元素
 输入:  pMsg 重发消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 XOS_FtpAddReMsgQueue(t_ReFtpMsg *pMsg)
{
    XS32 pos = 0;
    if(XNULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpAddMsgQueue: pMsg is NULL");
        return FTP_QUE_NULL;
    }

    /*队列满*/
    if(XOS_listCurSize(gFtpReMsgQueue) == XOS_listMaxSize(gFtpReMsgQueue))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "gFtpReMsgQueue  is full\n");
        return FTP_QUE_FULL;
    }
    
    pos = XOS_listAddTail(gFtpReMsgQueue, pMsg);
    if(0 > pos)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpAddMsgQueue: XOS_listAddTail failed ");
        return FTP_QUE_FAI;
    }

    return FTP_QUE_SUC;

}



/************************************************************************
 函数名:XOS_FtpClearMsgQueue
 功能:  根据指针删除队列中的元素
 输入:  pBackPara －定时器指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 XOS_FtpClearMsgQueue(t_XOSCOMMHEAD *pMsg)
{
    XS32 pos = -1;
    if(XNULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpClearMsgQueue: pEvent is NULL");
        return XERROR;
    }
    
    pos = XOS_listGetByPtr(gFtpMsgQueue, pMsg);
    if(0 > pos)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpClearMsgQueue: XOS_ArrayGetByPtr failed");
        return XERROR;
    }

    return XOS_listDelete(gFtpMsgQueue, pos);
}

/************************************************************************
 函数名:XOS_FtpClearReMsgQueue
 功能:  根据指针删除队列中的元素
 输入:  pBackPara －定时器指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 XOS_FtpClearReMsgQueue(t_ReFtpMsg *pMsg)
{
    XS32 pos = -1;
    if(XNULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpClearMsgQueue: pEvent is NULL");
        return XERROR;
    }
    
    pos = XOS_listGetByPtr(gFtpReMsgQueue, pMsg);
    if(0 > pos)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpClearMsgQueue: XOS_ArrayGetByPtr failed");
        return XERROR;
    }

    return XOS_listDelete(gFtpReMsgQueue, pos);
}


/************************************************************************
 函数名:XOS_FtpPutMsg
 功能:  将消息插入到循环队列
 输入:  pMsg －消息指针
 输出:
 返回: 成功返回FTP_QUE_SUC , 失败返回XERROR
 说明:
************************************************************************/
int XOS_FtpPutMsg(t_XOSCOMMHEAD *pMsg) 
{ 
    int status = 0; 
    XS32 head_pos = -1;
    t_XOSCOMMHEAD *pHeadMsg = XNULL;

    XOS_MutexLock(&gFtpMsgQueueMutex);

    status = XOS_FtpAddMsgQueue(pMsg);

    if(FTP_QUE_FULL == status) /*队列满，则删除第一个，构造循环队列*/
    {
        //XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutMsg: head_pos is full");
        head_pos = XOS_listHead(gFtpMsgQueue);   

        if (XERROR == head_pos)    
        {                          
            XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutMsg: head_pos is null");
            XOS_MutexUnlock(&gFtpMsgQueueMutex);
            return status;
        }

        pHeadMsg = (t_XOSCOMMHEAD*)XOS_listGetElem(gFtpMsgQueue, head_pos);
        
        XOS_FtpClearMsgMem(pHeadMsg);/*删除消息里的内存*/
        XOS_FtpClearMsgQueue(pHeadMsg);/*删除*/

        /*再次增加*/
        status = XOS_FtpAddMsgQueue(pMsg);
        
    }    

    XOS_SemPut(&gFtpQuSemaphore); /*挂出信号灯*/
    XOS_MutexUnlock(&gFtpMsgQueueMutex);

    return status; 
} 

/************************************************************************
 函数名:XOS_FtpPutReMsg
 功能:  将消息插入到循环队列
 输入:  pMsg －消息指针
 输出:
 返回: 成功返回FTP_QUE_SUC , 失败返回XERROR
 说明:
************************************************************************/
int XOS_FtpPutReMsg(t_ReFtpMsg *pMsg) 
{ 
    int status = 0; 
    XS32 head_pos = -1;
    t_ReFtpMsg *pHeadMsg = XNULL;

    XOS_MutexLock(&gFtpReMsgQueueMutex);

    status = XOS_FtpAddReMsgQueue(pMsg);

    if(FTP_QUE_FULL == status) /*队列满，则删除第一个，构造循环队列*/
    {
        //XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutMsg: head_pos is full");
        head_pos = XOS_listHead(gFtpReMsgQueue);   

        if (XERROR == head_pos)    
        {                          
            XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg: head_pos is null");
            XOS_MutexUnlock(&gFtpReMsgQueueMutex);
            return status;
        }

        pHeadMsg = (t_ReFtpMsg*)XOS_listGetElem(gFtpReMsgQueue, head_pos);
        
        XOS_FtpClearMsgMem(&pHeadMsg->reMsg);/*删除消息里的内存*/
        XOS_FtpClearReMsgQueue(pHeadMsg);/*删除*/

        /*再次增加*/
        status = XOS_FtpAddReMsgQueue(pMsg);
        
    }    

    XOS_SemPut(&gFtpReQuSemaphore); /*挂出信号灯*/
    XOS_MutexUnlock(&gFtpReMsgQueueMutex);

    return status; 
} 


/************************************************************************
 函数名:XOS_FtpGetMsg
 功能:  从循环队列获取消息
 输入:  pMsg －消息指针
 输出:
 返回: 成功返回FTP_QUE_SUC , 失败返回XERROR
 说明:
************************************************************************/
t_XOSCOMMHEAD XOS_FtpGetMsg() 
{ 
    t_XOSCOMMHEAD m_Msg; 
    XS32 head_pos = -1;
    XS8 blockFlag = 0;
    t_XOSCOMMHEAD *pHeadMsg = XNULL;
    m_Msg.msgID = XERROR;

    XOS_MutexLock(&gFtpMsgQueueMutex);

    /*没有任务，则阻塞*/
    head_pos = XOS_listHead(gFtpMsgQueue);
    if(XERROR == head_pos)
    {
        //XOS_Trace(MD(FID_FTP, PL_ERR), "no task");
        XOS_MutexUnlock(&gFtpMsgQueueMutex);
        blockFlag = 1;
    }

#if 1
    if(blockFlag)
    {
        /*等待信号量*/
        while (1) /*假信号或被其它任务获取*/ 
        {   
            XOS_SemGet(&gFtpQuSemaphore);

            /*重新获得锁*/
            XOS_MutexLock(&gFtpMsgQueueMutex);
            head_pos = XOS_listHead(gFtpMsgQueue);
            if(XERROR == head_pos)
            {
                XOS_MutexUnlock(&gFtpMsgQueueMutex);
            }
            else
            {
                //XOS_Trace(MD(FID_FTP, PL_ERR), "get task");
                break;
            }
        }
    }    

#endif
    pHeadMsg = (t_XOSCOMMHEAD*)XOS_listGetElem(gFtpMsgQueue, head_pos);    
    /*删除*/    
    m_Msg = *pHeadMsg; 
    XOS_FtpClearMsgQueue(pHeadMsg);   

    XOS_MutexUnlock(&gFtpMsgQueueMutex);

    return m_Msg; 
} 

/************************************************************************
 函数名:XOS_FtpGetReMsg
 功能:  从重发队列获取消息
 输入:  pMsg －消息指针
 输出:
 返回: 成功返回FTP_QUE_SUC , 失败返回XERROR
 说明:
************************************************************************/
t_ReFtpMsg XOS_FtpGetReMsg() 
{ 
    t_ReFtpMsg m_Msg; 
    XS32 head_pos = -1;
    XS8 blockFlag = 0;
    t_ReFtpMsg *pHeadMsg = XNULL;
    m_Msg.reMsg.msgID = XERROR;

    XOS_MutexLock(&gFtpReMsgQueueMutex);

    /*没有任务，则阻塞*/
    head_pos = XOS_listHead(gFtpReMsgQueue);
    if(XERROR == head_pos)
    {
        //XOS_Trace(MD(FID_FTP, PL_ERR), "no task");
        XOS_MutexUnlock(&gFtpReMsgQueueMutex);
        blockFlag = 1;
    }

#if 1
    if(blockFlag)
    {
        /*等待信号量*/
        while (1) /*假信号或被其它任务获取*/ 
        {   
            XOS_SemGet(&gFtpReQuSemaphore);

            /*重新获得锁*/
            XOS_MutexLock(&gFtpReMsgQueueMutex);
            head_pos = XOS_listHead(gFtpReMsgQueue);
            if(XERROR == head_pos)
            {
                XOS_MutexUnlock(&gFtpReMsgQueueMutex);
            }
            else
            {
                //XOS_Trace(MD(FID_FTP, PL_ERR), "get task");
                break;
            }
        }
    }    

#endif
    pHeadMsg = (t_ReFtpMsg*)XOS_listGetElem(gFtpReMsgQueue, head_pos);    
    /*删除*/    
    m_Msg = *pHeadMsg; 
    XOS_FtpClearReMsgQueue(pHeadMsg);   

    XOS_MutexUnlock(&gFtpReMsgQueueMutex);

    return m_Msg; 
} 

/************************************************************************
 函数名:XOS_FtpInitMsgDealTaskPool
 功能:  初始化任务池
 输入:  pBackPara －定时器指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 XOS_FtpInitMsgDealTaskPool(void)
{
    int i = 0;
    t_XOSTASKID idTask = 0;

    if(gFtpMsgDealThread <= 0 || gFtpMsgDealThread > MAX_DATA_THREAD)
    {
        gFtpMsgDealThread = 1;
    }

    for(i = 0;i < gFtpMsgDealThread; ++i)
    {
        if (XSUCC== XOS_TaskCreate("Tsk_ftpmsg", TSK_PRIO_LOWER,10000,(os_taskfunc)FTP_CliMsgDealTask, NULL, &idTask))
        {
        }
        else
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpInitMsgDealTaskPool[%d]: XOS_TaskCreate failed", i);
            return XERROR;
        }
    }

    /*重发任务*/
    if (XSUCC== XOS_TaskCreate("Tsk_ftpremsg", TSK_PRIO_LOWER,10000,(os_taskfunc)FTP_CliMsgReDealTask, NULL, &idTask))
    {
    }
    else
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpInitMsgDealTaskPool: XOS_TaskCreate [FTP_ReMsgDealTask] failed");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
 函数名:FTP_CliMsgDealTask
 功能:  消息处理工作任务
 输入:  pBackPara －定时器指针
 输出:
 返回: 
 说明:
************************************************************************/
XSTATIC XVOID* FTP_CliMsgDealTask(void* paras)
{
    t_XOSCOMMHEAD m_Msg;

    while(1)
    {
        m_Msg.msgID = eFTP_NULL;
        
        m_Msg = XOS_FtpGetMsg();/*获取任务*/
        
        switch (m_Msg.msgID)
         {
             case eFTP_GETTOFILE:/*下载文件到磁盘文件*/
                 FTP_GetFileMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_GETTOMEM:/*下载文件到内存*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMFILE:/*上传本地磁盘文件*/
                 FTP_PutFileMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMMEM:/*上传本地内存文件*/
                 FTP_PutMemMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_LIST:/*获取指定目录下的文件列表*/
                 //FTP_RecvErrorSend(&m_Msg);
                 break;
             default:
                 break;
         }

        XOS_Sleep(100);
    }
    
    return NULL;
}

/************************************************************************
 函数名:FTP_CliMsgDealTask
 功能:  重发消息处理工作任务
 输入:  pBackPara －定时器指针
 输出:
 返回: 
 说明:
************************************************************************/
XSTATIC XVOID* FTP_CliMsgReDealTask(void* paras)
{
    t_ReFtpMsg m_Msg;

    while(1)
    {
        m_Msg.reMsg.msgID = eFTP_NULL;
        
        m_Msg = XOS_FtpGetReMsg();/*获取重发任务*/
        
        switch (m_Msg.reMsg.msgID)
         {
             case eFTP_GETTOFILE:/*下载文件到磁盘文件*/
                 FTP_GetFileReMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_GETTOMEM:/*下载文件到内存*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMFILE:/*上传本地磁盘文件*/
                 FTP_PutFileReMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMMEM:/*上传本地内存文件*/
                 FTP_PutMemReMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_LIST:/*获取指定目录下的文件列表*/
                 //FTP_RecvErrorSend(&m_Msg);
                 break;
             default:
                 break;
         }

        XOS_Sleep(3000);
    }
    
    return NULL;
}

/************************************************************************
 函数名:XOS_FtpClearMsgMem
 功能:  清除消息中的内存
 输入:  paras 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
void XOS_FtpClearMsgMem(t_XOSCOMMHEAD* paras)
{
    if(NULL != paras)
    {
        switch (paras->msgID)
        {
             case eFTP_GETTOFILE:/*下载文件到磁盘文件*/
                 FTP_FreeGETTOFILE_REQ(paras);
                 break;
                 
             case eFTP_GETTOMEM:/*下载文件到内存*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMFILE:/*上传本地磁盘文件*/
                 FTP_FreePUTFROMFILE_REQ(paras);
                 break;
                 
             case eFTP_PUTFROMMEM:/*上传本地内存文件*/
                 FTP_FreePUTFROMMEM_REQ(paras);
                 break;
                 
             case eFTP_LIST:/*获取指定目录下的文件列表*/
                 //FTP_RecvErrorSend(&m_Msg);
                 break;
             default:
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpClearMsgMem [%d] failed !\n", paras->msgID);
                 break;
        }
    }

    return;
}


/************************************************************************
 函数名:XOS_FtpClearXosMsgMem
 功能:  清除消息中的内存,在插入到消息队列出错是调用
 输入:  paras 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
void XOS_FtpClearXosMsgMem(t_XOSCOMMHEAD* paras)
{
    if(NULL != paras)
    {
        switch (paras->msgID)
        {
             case eFTP_GETTOFILE:/*下载文件到磁盘文件*/
                 FTP_FreeXosGETTOFILE_REQ(paras);
                 break;
                 
             case eFTP_GETTOMEM:/*下载文件到内存*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMFILE:/*上传本地磁盘文件*/
                 FTP_FreeXosPUTFROMFILE_REQ(paras);
                 break;
                 
             case eFTP_PUTFROMMEM:/*上传本地内存文件*/
                 FTP_FreeXosPUTFROMMEM_REQ(paras);
                 break;
                 
             case eFTP_LIST:/*获取指定目录下的文件列表*/
                 //FTP_RecvErrorSend(&m_Msg);
                 break;
             default:
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpClearMsgMem [%d] failed !\n", paras->msgID);
                 break;
        }
    }

    return;
}

/************************************************************************
 函数名:XOS_FtpMallocMsgMem
 功能:  分配message内存
 输入:  paras 消息指针
 输出:
 返回: 成功返回地址 , 失败返回NULL
 说明:
************************************************************************/
XCHAR* XOS_FtpMallocMsgMem(int msgId, XCHAR *pBuffer)
{
    XCHAR *pMessage = NULL;
    if(!pBuffer)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpMallocMsgMem pBuffer is null\n");
        return NULL;
    }
    
    switch (msgId)
    {             
         case eFTP_PUTFROMFILE:/*上传本地磁盘文件*/
             pMessage = XOS_MemMalloc(FID_FTP, sizeof(t_PUTFROMFILE_REQ));
             if(pMessage)
             {
                XOS_MemCpy(pMessage, pBuffer, sizeof(t_PUTFROMFILE_REQ));
             }
             else
             {
                XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpMallocMsgMem XOS_MemMalloc failed\n");
             }
             break;
             
         case eFTP_PUTFROMMEM:/*上传本地内存文件*/
              pMessage = XOS_MemMalloc(FID_FTP, sizeof(t_PUTFROMMEM_REQ));
             if(pMessage)
             {
                XOS_MemCpy(pMessage, pBuffer, sizeof(t_PUTFROMMEM_REQ));
             }
             else
             {
                XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpMallocMsgMem XOS_MemMalloc failed\n");
             }
             break;
        case eFTP_GETTOFILE:/*下载文件到本地文件*/
              pMessage = XOS_MemMalloc(FID_FTP, sizeof(t_GETTOFILE_REQ));
             if(pMessage)
             {
                XOS_MemCpy(pMessage, pBuffer, sizeof(t_GETTOFILE_REQ));
             }
             else
             {
                XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpMallocMsgMem XOS_MemMalloc failed\n");
             }
             break;
         default:
             pMessage = NULL;
            break;
    }
    
    return pMessage;
}

/************************************************************************
 函数名:XOS_FtpFreeMsgMem
 功能:  释放消息队列中的message指针
 输入:  paras 消息指针
 输出:
 返回: 成功返回地址 , 失败返回NULL
 说明:
************************************************************************/
void XOS_FtpFreeMsgMem(int msgId, void *pBuffer)
{
    if(NULL != pBuffer)
    {
        switch (msgId)
        {
             case eFTP_GETTOFILE:/*下载文件到磁盘文件*/
                 //FTP_PutMemMsgPro(&m_Msg);
                 break;                 
             case eFTP_GETTOMEM:/*下载文件到内存*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;                 
             case eFTP_PUTFROMFILE:/*上传本地磁盘文件*/
                 XOS_MemFree(FID_FTP, pBuffer);        
                 break;
             case eFTP_PUTFROMFILEAck:
                  break;                 
             case eFTP_PUTFROMMEM:/*上传本地内存文件*/
                 XOS_MemFree(FID_FTP, pBuffer);                      
                 break;
             case eFTP_PUTFROMMEMAck:
                  break;
             case eFTP_LIST:/*获取指定目录下的文件列表*/
                    break;
             default:
                break;
        }
    }

    return;
}

/************************************************************************
 函数名:FTP_TimerProc
 功能:  ftp 模块定时器消息处理函数入口
 输入:  pBackPara －定时器指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_TimerProc( t_BACKPARA* pBackPara)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
#ifdef FTP_TEST
    XPOINT timerid = 0;
#endif
    
    if (pBackPara == (t_BACKPARA*)XNULLP)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_timerProc()->timer PTIMER is null ,bad input param !\n");
        return XERROR;
    }
#ifdef FTP_TEST
    timerid = pBackPara->para2;
    if(timerid == MEDIA_TIMER_MSG_FTP_SEND)
    {
        XOS_FtpGetFileTest();
        //XOS_FtpPutMemTest();
        //XOS_FtpPutFileTest();
    }
#endif

    /*获取当前用户*/
    pUserInfo = (t_XOSFTPCLIENT*)(XPOINT)(pBackPara->para1);
    if ((t_XOSFTPCLIENT*)XNULLP == pUserInfo)
    {
        return XERROR;
    }

    /*操作用户数组中的资源上锁*/
    XOS_MutexLock(&(g_ftpMnt.contrlTblLock));

    /*恢复定时器*/
    XOS_INIT_THDLE(pUserInfo->timerId);
    if (XSUCC != pUserInfo->ret || eFTPSTATELOGIN != pUserInfo->curState)
    {
        pUserInfo->ret = XERROR;
    }
    else
    {
        pUserInfo->ret = XSUCC;
    }

    
    /*提示定时器到期的事件状态*/
    XOS_Trace(MD(FID_FTP, PL_WARN), "time out FTP_timerProc()-> semaphore put !,curEvent is %d\n", pUserInfo->curEvent);

    /*登录成功后，正在上传文件，超时,可能文件太大，可网络速度慢*/
    if (pUserInfo->curState == eFTPSTATELOGIN && (eFTPPUTFROMMEM == pUserInfo->curEvent || eFTPPUTFROMFILE == pUserInfo->curEvent))
    {
        XINET_CloseSock(&(pUserInfo->dataSockId));
        pUserInfo->dataSockId.fd = XERROR;
        pUserInfo->retOfPut = XERROR;

        /*释放数据通道信号量，否则，上传函数将永远挂起,原因:如果通信链路出了问题，没有响应，则只能等待超时退出*/
        XOS_SemPut(&(pUserInfo->semaphore));
        XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
        return XSUCC;
    }

    /*登录成功后，正在下载文件或目录 ，超时，可能文件太大，可网络速度慢*/
    if (pUserInfo->curState == eFTPSTATELOGIN && 
        (eFTPGETTOMEM == pUserInfo->curEvent || 
         eFTPLIST == pUserInfo->curEvent || 
         eFTPGETTOFILE == pUserInfo->curEvent))
    {
        XINET_CloseSock(&(pUserInfo->dataSockId));
        pUserInfo->dataSockId.fd = XERROR;
        
        /*释放数据通道信号量，否则，下载函数将永远挂起*/
        XOS_SemPut(&(pUserInfo->semaphore));
        XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
        return XSUCC;
    }
    
    pUserInfo->curEvent = eFTPNONE;


    /*释放控制通道信号量*/
    XOS_SemPut(&(pUserInfo->semaphore));

    /*释放用户数组锁*/
    XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
    return XSUCC;
}

/************************************************************************
 函数名:FTP_NtlMsgPro
 功能:  ftp 模块ntl消息处理
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_NtlMsgPro(t_XOSCOMMHEAD* pMsgP)
{
    t_XOSCOMMHEAD *pMsg = XNULLP;
     
     if ((pMsgP == (XVOID*)XNULLP) || ((t_XOSCOMMHEAD*)pMsgP)->message == XNULLP)
     {
         XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_msgProc()->Bad input param !\n");
         return XERROR;
     }
     pMsg = (t_XOSCOMMHEAD*)pMsgP;
     switch (pMsg->msgID)
     {
     case eInitAck:/*链路初始化应答*/
         FTP_RecvInitAck(pMsg);
         break;
         
     case eStartAck:/*链路启动应答*/
         FTP_RecvStartAck(pMsg);
         break;
         
     case eDataInd:/*收到数据指示*/
         FTP_RecvDataInd(pMsg);
         break;
         
     case eStopInd:
         FTP_RecvStopInd(pMsg);
         break;
         
     case eErrorSend:
         FTP_RecvErrorSend(pMsg);
         break;
     default:
         break;
     }
     
     return XSUCC;

}

/************************************************************************
 函数名:FTP_PutMemMsgProTskEntry
 功能:  从内存上传
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_PutMemMsgProTskEntry(t_XOSCOMMHEAD* pMsg)
{
    t_PUTFROMMEM_REQ *pReq = XNULL;
    XS8 usRst = eFTP_SUCC;
    if(pMsg)
    {
        usRst = FTP_PutMemMsgDo(pMsg);
        if(eFTP_SUCC != usRst)/*第一次发送失败*/
        {
            t_ReFtpMsg reSendMsg;
            reSendMsg.nSndNum = 0; /*重发次数*/
            reSendMsg.reMsg = *pMsg;
            XOS_Trace(MD(FID_FTP, PL_WARN), "Waiting resend ftp file\n");
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(&reSendMsg))/*送入重发消息队列*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreePUTFROMMEM_REQ(pMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*如果出错，则要释放消息*/
        }
        else/*成功*/
        {   
            pReq = (t_PUTFROMMEM_REQ*)pMsg->message;
            if(pReq)
            {
                /*向客户模块发消息*/
                FTP_SendMsgToCli(pMsg->datasrc.FID, usRst, pReq->ulSerial);
                /*释放消息内存*/
                FTP_FreePUTFROMMEM_REQ(pMsg);
                return XSUCC;
            }
            return XERROR;
        }
    }
    else
    {
        return XERROR;
    }    
}
/************************************************************************
 函数名:FTP_PutMemReMsgProTskEntry
 功能:  从内存上传(重传)
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_PutMemReMsgProTskEntry(t_ReFtpMsg* pMsg)
{
    t_PUTFROMMEM_REQ *pReq = XNULL;
    
    XS8 usRst = eFTP_SUCC;
    
    if(pMsg)
    {
        usRst = FTP_PutMemMsgDo(&pMsg->reMsg);
        if(eFTP_SUCC != usRst)/*失败*/
        {
            pMsg->nSndNum ++; /*重发次数*/
            
            if(pMsg->nSndNum >=3 && NO_SEND_ELAPSES) /*发送次数超时*/
            {
                pReq = (t_PUTFROMMEM_REQ*)pMsg->reMsg.message;
                if(pReq)
                {
                    /*向客户模块发消息*/
                    FTP_SendMsgToCli(pMsg->reMsg.datasrc.FID, usRst, pReq->ulSerial);
                    /*释放消息内存*/
                    FTP_FreePUTFROMMEM_REQ(&pMsg->reMsg);

                    XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_PutMemReMsgProTskEntry failed!\n");
                    return XSUCC; 
                }
                return XERROR;
            }
            
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(pMsg))/*送入重发消息队列*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreePUTFROMMEM_REQ(&pMsg->reMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*如果出错，则要释放消息*/
        }
        else/*成功*/
        {   
            pReq = (t_PUTFROMMEM_REQ*)pMsg->reMsg.message;
            if(pReq)
            {
                /*向客户模块发消息*/
                FTP_SendMsgToCli(pMsg->reMsg.datasrc.FID, usRst, pReq->ulSerial);
                /*释放消息内存*/
                FTP_FreePUTFROMMEM_REQ(&pMsg->reMsg);

                XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_PutMemReMsgProTskEntry success!\n");
                return XSUCC;
            }
            return XERROR;
        }
    }
    else
    {
        return XERROR;
    }
}


/************************************************************************
 函数名:FTP_PutMemMsgDo
 功能:  ftp 模块ntl消息处理
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_PutMemMsgDo(t_XOSCOMMHEAD* pMsg)
{

    t_PUTFROMMEM_REQ *pReq = XNULL;
    XOS_HFTPCLT hFtpClt = NULL;
    XCHAR   FullFileName[MAX_FILENAME_LEN + 1] = {0};
    XCHAR   ucNewname[MAX_FILENAME_LEN + 1] = {0};
    XS32 usRst = eFTP_SUCC;

    if(XNULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_PutMemMsgProTskEntry!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    /*取消息中的内容*/
    pReq = (t_PUTFROMMEM_REQ*)pMsg->message;
    if(NULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "pMsg->message is null!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    if(XNULL == pReq->ucpFileName || XNULL == pReq->ucpBuffer || XNULL == pReq->ucpPass || XNULL == pReq->ucpReldir || XNULL == pReq->ucpUser)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XNULL == pReq->ucpFileName!");        
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    /*用户名或密码长度不合法*/
    if((pReq->ulUserLen > MAX_USERNAME_LEN) || pReq->ulPassLen> MAX_PASSWORD_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "pReq->ulUserLen > MAX_USERNAME_LEN  || pReq->ulPassLen> MAX_PASSWORD_LEN!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }
    
    /*登录服务器*/
    if (XSUCC != XOS_FtpLogin(&pReq->destAddr, (XS8*)pReq->ucpUser, (XS8*)pReq->ucpPass, &hFtpClt))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_PutMemMsgDo XOS_FtpLogin Failed !");
        usRst = eFTP_LOGIN_FAILED;
        goto error;
    }


    if(0 != pReq->ulRelDirLen)
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "%s/%s", pReq->ucpReldir, pReq->ucpFileName);
    }
    else
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "%s",  pReq->ucpFileName);
    }

    /*获取文件大小，表示文件存在，则先对其重命名*/
    if (XSUCC <= XOS_FtpGetFileSize(hFtpClt, FullFileName))
    {
        //XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_PutMemMsgDo The Ttp File Name Exist !");

        XOS_MemCpy(ucNewname, FullFileName, XOS_MIN((XOS_StrLen(FullFileName)-1), sizeof(ucNewname)-2));
        XOS_StrNCat(ucNewname, "b", XOS_StrLen("b"));  /*end of '\0'*/

        /*重命名原文件*/
        XOS_FtpRenameFile(hFtpClt, FullFileName, ucNewname);
    }

    if(0 != pReq->ulRelDirLen)
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "/%s/%s", pReq->ucpReldir, pReq->ucpFileName);
    }
    else
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "/%s", pReq->ucpFileName);
    }

/*上传文件*/
    /*目前不支持断点续传，最大文件大小不能超过300秒*/
    if (XSUCC != XOS_FtpPutFromMem(hFtpClt, (XS8*)pReq->ucpBuffer, pReq->ulBufferLen, FullFileName, MAX_FTPTRANS_TIME))
    {
        XOS_Trace(MD(FID_FTP, PL_INFO), "XOS_FtpPutFromMem failed !");
        XOS_FtpClose(&hFtpClt);
        usRst = eFTP_TRAN_FAILED;
        goto error;
    }

    /*释放ftp*/
    XOS_FtpClose(&hFtpClt);

error:

    return usRst;    
}


/************************************************************************
 函数名:FTP_GetFileMsgProTskEntry
 功能: 下载文件到本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_GetFileMsgProTskEntry(t_XOSCOMMHEAD* pMsg)
{
    t_GETTOFILE_REQ *pReq = XNULL;
    XS8 usRst = eFTP_SUCC;
    XU32 ulFileSize = 0;
    if(pMsg)
    {
        usRst = FTP_GetFileMsgDo(pMsg, &ulFileSize);
        pReq = (t_GETTOFILE_REQ*)pMsg->message;
        if(NULL == pReq)
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_GetFileMsgProTskEntry pReq is null\n");
            return XERROR;
        }
        
        if(eFTP_SUCC != usRst && eFTP_REMOTE_FILE_FAILED != usRst && 
           (pReq->ulMaxReDown > 0))/*第一次发送失败*/
        {
            t_ReFtpMsg reSendMsg;
            reSendMsg.nSndNum = 0; /*重发次数*/
            reSendMsg.reMsg = *pMsg;            
            XOS_Trace(MD(FID_FTP, PL_WARN), "Waiting redownload ftp file\n");
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(&reSendMsg))/*送入重发消息队列*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreeGETTOFILE_REQ(pMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*如果出错，则要释放消息*/
        }
        else/*成功或不重新下载*/
        {   
            /*向客户模块发消息*/
            FTP_Send_GettoFile_Ack_MsgToCli(pMsg->datasrc.FID, pMsg->datasrc.FsmId,usRst, pReq->ulSerial, ulFileSize);
            /*释放消息内存*/
            FTP_FreeGETTOFILE_REQ(pMsg);
            return XSUCC;
        }
    }
    else
    {
        return XERROR;
    }

}

/************************************************************************
 函数名:FTP_PutFileMsgProTskEntry
 功能: 上传本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_PutFileMsgProTskEntry(t_XOSCOMMHEAD* pMsg)
{
    t_PUTFROMFILE_REQ *pReq = XNULL;
    XS8 usRst = eFTP_SUCC;
    if(pMsg)
    {
        usRst = FTP_PutFileMsgDo(pMsg);
        if(eFTP_SUCC != usRst)/*第一次发送失败*/
        {
            t_ReFtpMsg reSendMsg;
            reSendMsg.nSndNum = 0; /*重发次数*/
            reSendMsg.reMsg = *pMsg;
            XOS_Trace(MD(FID_FTP, PL_WARN), "Waiting resend ftp file\n");
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(&reSendMsg))/*送入重发消息队列*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreePUTFROMFILE_REQ(pMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*如果出错，则要释放消息*/
        }
        else/*成功*/
        {   
            pReq = (t_PUTFROMFILE_REQ*)pMsg->message;
            if(pReq)
            {
                /*向客户模块发消息*/
                FTP_SendMsgToCli(pMsg->datasrc.FID, usRst, pReq->ulSerial);
                /*释放消息内存*/
                FTP_FreePUTFROMFILE_REQ(pMsg);
                return XSUCC;
            }
            return XERROR;
        }
    }
    else
    {
        return XERROR;
    }

}

/************************************************************************
 函数名:FTP_GetFileReMsgProTskEntry
 功能: 下载文件到本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_GetFileReMsgProTskEntry(t_ReFtpMsg* pMsg)
{    
    t_GETTOFILE_REQ *pReq = XNULL;    
    XS8 usRst = eFTP_SUCC;
    XU32 ulFileSize = 0;
    
    if(pMsg)
    {
        usRst = FTP_GetFileMsgDo(&pMsg->reMsg, &ulFileSize);
        if(eFTP_SUCC != usRst)/*失败*/
        {
            pMsg->nSndNum ++; /*重发次数*/
            pReq = (t_GETTOFILE_REQ*)pMsg->reMsg.message;
            if(pReq)
            {                
                if(pMsg->nSndNum >= pReq->ulMaxReDown) /*发送次数超时*/
                {    
                    XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_GetFileReMsgProTskEntry download  %s time out!\n", pReq->ucpSaveFilePath);

                    /*向客户模块发消息*/
                    FTP_Send_GettoFile_Ack_MsgToCli(pMsg->reMsg.datasrc.FID, pMsg->reMsg.datasrc.FsmId,usRst, pReq->ulSerial, ulFileSize);
                    /*释放消息内存*/
                    FTP_FreeGETTOFILE_REQ(&pMsg->reMsg);
                    return XSUCC;                    
                }
            }
            
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(pMsg))/*送入重发消息队列*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreeGETTOFILE_REQ(&pMsg->reMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*如果出错，则要释放消息*/
        }
        else/*成功*/
        {   
            pReq = (t_GETTOFILE_REQ*)pMsg->reMsg.message;
            if(pReq)
            {
                /*向客户模块发消息*/
                FTP_Send_GettoFile_Ack_MsgToCli(pMsg->reMsg.datasrc.FID, pMsg->reMsg.datasrc.FsmId,usRst, pReq->ulSerial, ulFileSize);
                /*释放消息内存*/
                FTP_FreeGETTOFILE_REQ(&pMsg->reMsg);
                return XSUCC;
            }
            return XERROR;
        }
    }
    else
    {
        return XERROR;
    }

}

/************************************************************************
 函数名:FTP_PutFileReMsgProTskEntry
 功能: 上传本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_PutFileReMsgProTskEntry(t_ReFtpMsg* pMsg)
{    
    t_PUTFROMFILE_REQ *pReq = XNULL;
    
    XS8 usRst = eFTP_SUCC;
    
    if(pMsg)
    {
        usRst = FTP_PutFileMsgDo(&pMsg->reMsg);
        if(eFTP_SUCC != usRst)/*失败*/
        {
            pMsg->nSndNum ++; /*重发次数*/
            
            if(pMsg->nSndNum >=3 && NO_SEND_ELAPSES) /*发送次数超时*/
            {
                pReq = (t_PUTFROMFILE_REQ*)pMsg->reMsg.message;
                if(pReq)
                {
                    /*向客户模块发消息*/
                    FTP_SendMsgToCli(pMsg->reMsg.datasrc.FID, usRst, pReq->ulSerial);
                    /*释放消息内存*/
                    FTP_FreePUTFROMFILE_REQ(&pMsg->reMsg);
                    return XSUCC;
                }
                return XERROR;
            }
            
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(pMsg))/*送入重发消息队列*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreePUTFROMFILE_REQ(&pMsg->reMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*如果出错，则要释放消息*/
        }
        else/*成功*/
        {   
            pReq = (t_PUTFROMFILE_REQ*)pMsg->reMsg.message;
            if(pReq)
            {
                /*向客户模块发消息*/
                FTP_SendMsgToCli(pMsg->reMsg.datasrc.FID, usRst, pReq->ulSerial);
                /*释放消息内存*/
                FTP_FreePUTFROMFILE_REQ(&pMsg->reMsg);
                return XSUCC;
            }
            return XERROR;
        }
    }
    else
    {
        return XERROR;
    }

}

/************************************************************************
 函数名:FTP_Snprintf_Md5String
 功能: 将md5转换为字符串
 输入:  pszMd5 需要转换的字符串
        nMd5Len  字符串长度
        pszOut   转换后的字符串
        nOutLen  转换后的字符串长度
 输出:
 返回: 
 说明:
************************************************************************/
void FTP_Snprintf_Md5String(const char *pszMd5, int nMd5Len, char *pszOut, int nOutLen)
{
    int i = 0;
    if( NULL == pszMd5 || NULL == pszOut)
    {
        return ;
    }

    if(nMd5Len * 2 > nOutLen)
    {
        return ;
    }

    for(i = 0; i< nMd5Len && *pszMd5; i++)
    {
        sprintf(pszOut, "%02x", (unsigned char)pszMd5[i]);

        pszOut += 2;
    }

    return;
}
/************************************************************************
 函数名:FTP_GetFileMsgDo
 功能: 下载文件到本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_GetFileMsgDo(t_XOSCOMMHEAD* pMsg, XU32 *pFileSize)
{
    FILE* fileptr = XNULLP;
    t_GETTOFILE_REQ *pReq = XNULL;
    XOS_HFTPCLT hFtpClt = NULL;
    XCHAR    FullFileName[MAX_FILENAME_LEN + 1] = {0};
    XS32 usRst = eFTP_SUCC;
    XCHAR saveFilename[MAX_FILENAME_LEN+1] = {0};
    XS32 ulRetSize = 0;
    char cMd5Buf[17] = {0};
    char cMd5String[33] = {0};

    if(XNULL == pMsg || NULL == pFileSize)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_PutFileMsgDo XNULL == pMsg!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    /*取消息中的内容*/
    pReq = (t_GETTOFILE_REQ*)pMsg->message;
    if(NULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "pMsg->message is null!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    if(XNULL == pReq->ucpFileName || XNULL == pReq->ucpSaveFilePath || 
       XNULL == pReq->ucpPass || XNULL == pReq->ucpReldir || XNULL == pReq->ucpUser)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XNULL == pReq->ucpFileName!");    
        usRst = eFTP_PARA_INVALID;
        goto error;
    }
    
    /*用户名或密码长度不合法*/
    if((pReq->ulUserLen > MAX_USERNAME_LEN) || pReq->ulPassLen> MAX_PASSWORD_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "pReq->ulUserLen > MAX_USERNAME_LEN    || pReq->ulPassLen> MAX_PASSWORD_LEN!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    if(0 == pReq->ulTime)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "0 == pReq->ulTime!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    /*本地文件存在,则删除*/
    XOS_MemCpy(saveFilename, pReq->ucpSaveFilePath, XOS_MIN(sizeof(saveFilename)-1, pReq->ulSaveFilePathLen));
    if( XNULLP != (fileptr = XOS_Fopen(saveFilename,"r")) )
    {        
        XOS_Fclose(fileptr);
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_Fopen file %s success, it was delete!", saveFilename);
        if(XSUCC != XOS_DeleteFile(saveFilename))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_DeleteFile %s failed!", saveFilename);
            usRst = eFTP_DEL_FILE_FAILED;
            goto error;
        }
    }
        
    /*登录服务器*/
    if (XSUCC != XOS_FtpLogin(&pReq->destAddr, (XS8*)pReq->ucpUser, (XS8*)pReq->ucpPass, &hFtpClt))
    {
        XOS_Trace(MD(FID_NTL, PL_WARN), "XOS_FtpLogin Failed !");
        usRst = eFTP_LOGIN_FAILED;
        goto error;
    }    
    
    
    if(0 != pReq->ulRelDirLen)
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "%s/%s", pReq->ucpReldir, pReq->ucpFileName);
    }
    else
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "%s",  pReq->ucpFileName);
    }
    
    /*获取文件大小，表示文件存在*/
    ulRetSize = XOS_FtpGetFileSize(hFtpClt, FullFileName);
    if (XSUCC <= ulRetSize)
    {
        *pFileSize = ulRetSize;
    }
    else
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "The Ttp File Name %s no Exist !", FullFileName);
        usRst = eFTP_REMOTE_FILE_FAILED;
        XOS_FtpClose (&hFtpClt );

        goto error ;
    }
    
    if(0 != pReq->ulRelDirLen)
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "/%s/%s", pReq->ucpReldir, pReq->ucpFileName);
    }
    else
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "/%s", pReq->ucpFileName);
    }
    
    /*下载文件*/
    if( XSUCC != XOS_FtpGetToFile(hFtpClt, FullFileName, saveFilename, pReq->ulTime))
    {
        XOS_Trace(MD(FID_FTP, PL_INFO), "XOS_FtpGetToFile failed !");
        XOS_FtpClose(&hFtpClt);
        usRst = eFTP_TRAN_FAILED;
        goto error;
    }

    /*关闭服务器连接*/
    XOS_FtpClose (&hFtpClt );

    /*md5校验*/
    if(usRst == eFTP_SUCC && XNULL != pReq->ucpMd5)
    {
        /*获取文件的md5值*/
        if(XSUCC != XOS_MDcheckfile((char*)pReq->ucpSaveFilePath, (unsigned char*)cMd5Buf, sizeof(cMd5Buf)))
        {
            usRst = eFTP_GENERATE_MD5_ERROR;
            XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpGetToFile XOS_MDcheckfile failed !");
        }
        else
        {
            FTP_Snprintf_Md5String(cMd5Buf, (int)strlen(cMd5Buf), cMd5String, sizeof(cMd5String));
            /*转换字符串为小写*/
            FTP_CoverLowerChar((char*)pReq->ucpMd5, pReq->ulMd5Len);
            if(0 != memcmp((char*)pReq->ucpMd5, (char*)cMd5String, pReq->ulMd5Len))
            {
                usRst = eFTP_MD5_ERROR;
                XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpGetToFile md5 is not match !");
            }
        }
    }
    
error:

    return usRst;    
}

/************************************************************************
 函数名:FTP_CoverLowerChar
 功能: 将字符串中的大写字母转换为小写字母
 输入:  pszIn 需要转换的字符串
        nLen  字符串长度
 输出:
 返回: 
 说明:
************************************************************************/
void FTP_CoverLowerChar(char *pszIn, int nLen)
{
    int i = 0;
    char *psz = pszIn;

    if(NULL == psz)
    {
        return ;
    }
    
    for(i  = 0; i< nLen && *psz; i++)
    {
        if(psz[i] >= 'A' && psz[i] <= 'Z')
        {
            psz[i] = psz[i] + ('a' - 'A');
        }
    }

    return;
}
/************************************************************************
 函数名:FTP_PutFileMsgDo
 功能: 上传本地文件的请求消息
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_PutFileMsgDo(t_XOSCOMMHEAD* pMsg)
{
    
    FILE* fileptr = XNULLP;
    t_PUTFROMFILE_REQ *pReq = XNULL;
    XOS_HFTPCLT hFtpClt = NULL;
    XCHAR    FullFileName[MAX_FILENAME_LEN + 1] = {0};
    XCHAR    ucNewname[MAX_FILENAME_LEN + 1] = {0};
    XS32 usRst = eFTP_SUCC;
    XCHAR uploadFilename[MAX_FILENAME_LEN+1] = {0};

    if(XNULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_PutFileMsgDo XNULL == pMsg!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    /*取消息中的内容*/
    pReq = (t_PUTFROMFILE_REQ*)pMsg->message;
    if(NULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "pMsg->message is null!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    if(XNULL == pReq->ucpFileName || XNULL == pReq->ucpUploadFilePath || XNULL == pReq->ucpPass || XNULL == pReq->ucpReldir || XNULL == pReq->ucpUser)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XNULL == pReq->ucpFileName!");     
        usRst = eFTP_PARA_INVALID;
        goto error;
    }
    
    /*用户名或密码长度不合法*/
    if((pReq->ulUserLen > MAX_USERNAME_LEN) || pReq->ulPassLen> MAX_PASSWORD_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "pReq->ulUserLen > MAX_USERNAME_LEN  || pReq->ulPassLen> MAX_PASSWORD_LEN!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    /*测试文件是否可读*/
    XOS_MemCpy(uploadFilename, pReq->ucpUploadFilePath, XOS_MIN(sizeof(uploadFilename)-1, pReq->ulUploadFilePathLen));
    if( XNULLP == (fileptr = XOS_Fopen(uploadFilename,"r")) )
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_Fopen failed!");
        usRst = eFTP_LOC_FILE_FAILED;
        goto error ;
    }
    else
    {
        XOS_Fclose(fileptr);
    }
        
    /*登录服务器*/
    if (XSUCC != XOS_FtpLogin(&pReq->destAddr, (XS8*)pReq->ucpUser, (XS8*)pReq->ucpPass, &hFtpClt))
    {
        XOS_Trace(MD(FID_NTL, PL_WARN), "XOS_FtpLogin Failed !");
        usRst = eFTP_LOGIN_FAILED;
        goto error;
    }    
    
    if(0 != pReq->ulRelDirLen)
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "%s/%s", pReq->ucpReldir, pReq->ucpFileName);
    }
    else
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "%s",  pReq->ucpFileName);
    }
    
    /*获取文件大小，表示文件存在，则先对其重命名*/
    if (XSUCC <= XOS_FtpGetFileSize(hFtpClt, FullFileName))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "The Ttp File Name Exist !");

        XOS_MemCpy(ucNewname, FullFileName, XOS_MIN((XOS_StrLen(FullFileName)-1), sizeof(ucNewname)-2));
        XOS_StrNCat(ucNewname, "b", XOS_StrLen("b"));  /*end of '\0'*/

        /*重命名原文件*/
        XOS_FtpRenameFile(hFtpClt, FullFileName, ucNewname);
    }
    
    if(0 != pReq->ulRelDirLen)
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "/%s/%s", pReq->ucpReldir, pReq->ucpFileName);
    }
    else
    {
        XOS_Sprintf(FullFileName, sizeof(FullFileName)-1, "/%s", pReq->ucpFileName);
    }
    
    /*上传文件*/
    if( XSUCC != XOS_FtpPutFromFile ( hFtpClt,uploadFilename,FullFileName,0))
    {
        XOS_Trace(MD(FID_FTP, PL_INFO), "XOS_FtpPutFromMem failed !");
        XOS_FtpClose(&hFtpClt);
        usRst = eFTP_TRAN_FAILED;
        goto error;
    }

    /*关闭服务器连接*/
    XOS_FtpClose (&hFtpClt );
    
error:

    return usRst;    

}

/************************************************************************
 函数名:FTP_Send_GettoFile_Ack_MsgToCli
 功能:  发送响应消息给客户端
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_Send_GettoFile_Ack_MsgToCli(XU32 ulDestFid, XU32 ulFsmId, XU32 ulRst, XU64 ulSerial, XU32 ulFileSize)
{
   XU8* pBuff = XNULL;
   t_XOSCOMMHEAD* pCpsHead = XNULL;
   t_GETTOFILE_RSP*   pClientRsp = XNULL;


   pBuff = (XU8*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_GETTOFILE_RSP));
   if( XNULL == pBuff )
   {
       return XERROR;
   }

   pCpsHead = (t_XOSCOMMHEAD*)pBuff;
   pCpsHead->datasrc.PID   = XOS_GetLocalPID();

   pCpsHead->datasrc.FID = FID_FTP;

   pCpsHead->datasrc.FsmId = BLANK_ULONG;
   pCpsHead->datadest.PID  = XOS_GetLocalPID();
   pCpsHead->datadest.FID  = ulDestFid;
   pCpsHead->datadest.FsmId = ulFsmId;
   pCpsHead->msgID           = eFTP_GETTOFILEAck;
   pCpsHead->subID           = BLANK_USHORT;
   pCpsHead->prio           = eNormalMsgPrio;
   pCpsHead->length        = sizeof(t_GETTOFILE_RSP);
   pCpsHead->message       = pBuff + sizeof(t_XOSCOMMHEAD);
   pClientRsp = (t_GETTOFILE_RSP*)(pBuff + sizeof(t_XOSCOMMHEAD));
   XOS_MemSet(pClientRsp, 0, sizeof(t_GETTOFILE_RSP));
   pClientRsp->ulResult = ulRst;
   pClientRsp->ulSerial = ulSerial;
   pClientRsp->ulFileSize = ulFileSize;

   /*发果发送失败， 则主动释放消息内存*/
  if(XERROR == XOS_MsgSend(pCpsHead))
  {
      XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
  }
  
  return XSUCC;
}

/************************************************************************
 函数名:FTP_SendMsgToCli
 功能:  发送响应消息给客户端
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_SendMsgToCli(XU32 ulDestFid, XU32 ulRst, XU64 ulSerial)
{
   XU8* pBuff = XNULL;
   t_XOSCOMMHEAD* pCpsHead = XNULL;
   t_PUTFROMMEM_RSP*   pClientRsp = XNULL;


   pBuff = (XU8*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_PUTFROMMEM_RSP));
   if( XNULL == pBuff )
   {
       return XERROR;
   }

   pCpsHead = (t_XOSCOMMHEAD*)pBuff;
   pCpsHead->datasrc.PID   = XOS_GetLocalPID();

   pCpsHead->datasrc.FID = FID_FTP;

   pCpsHead->datasrc.FsmId = BLANK_ULONG;
   pCpsHead->datadest.PID  = XOS_GetLocalPID();
   pCpsHead->datadest.FID  = ulDestFid;
   pCpsHead->datadest.FsmId = BLANK_ULONG;
   pCpsHead->msgID           = eFTP_PUTFROMMEMAck;
   pCpsHead->subID           = BLANK_USHORT;
   pCpsHead->prio           = eNormalMsgPrio;
   pCpsHead->length        = sizeof(t_PUTFROMMEM_RSP);
   pCpsHead->message       = pBuff + sizeof(t_XOSCOMMHEAD);
   pClientRsp = (t_PUTFROMMEM_RSP*)(pBuff + sizeof(t_XOSCOMMHEAD));
   XOS_MemSet(pClientRsp, 0, sizeof(t_PUTFROMMEM_RSP));
   pClientRsp->ulResult = ulRst;
   pClientRsp->ulSerial = ulSerial;

   /*发果发送失败， 则主动释放消息内存*/
  if(XERROR == XOS_MsgSend(pCpsHead))
  {
      XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
  }
  
  return XSUCC;
}




/************************************************************************
 函数名:FTP_FreePUTFROMMEM_REQ
 功能:  释放从内存上传文件的请求消息
 输入:  pReq 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_FreePUTFROMMEM_REQ(t_XOSCOMMHEAD *pMsg)
{
    int rst = XSUCC;
    t_PUTFROMMEM_REQ *pReq = XNULL;

    pReq = (t_PUTFROMMEM_REQ*)pMsg->message;
    if(XNULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), " XNULL == pReq!\n");
        return XERROR;
    }    
    
    if(XNULL != pReq->ucpBuffer)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpBuffer))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpBuffer free failed\n");
            rst = XERROR;
        }
    }

    if(XNULL != pReq->ucpReldir)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpReldir))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpReldir free failed\n");
            rst = XERROR;
        }
    }

    if(XNULL != pReq->ucpFileName)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpFileName))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpFileName free failed\n");
            rst = XERROR;
        }
    }

    if(XNULL != pReq->ucpPass)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpPass))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpPass free failed\n");
            rst = XERROR;
        }
    }

    if(XNULL != pReq->ucpUser)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpUser))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpUser free failed\n");
            rst = XERROR;
        }
    }

    if(pMsg->message != NULL)
    {
        if(XSUCC != XOS_MemFree(pMsg->datadest.FID, pMsg->message))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pMsg->message free failed\n");
            rst = XERROR;
        }
    }

    if(rst == XERROR)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR),"\n FTP_FreePUTFROMMEM_REQ %04x,%04x,%04x,%04x,%04x,%04x\n", pReq->ucpBuffer, pReq->ucpReldir, 
              pReq->ucpFileName, pReq->ucpPass, pReq->ucpUser, pMsg->message);
    }
    return XSUCC;
}

/************************************************************************
 函数名:FTP_FreeXosPUTFROMMEM_REQ
 功能:  释放从内存上传文件的请求消息
 输入:  pReq 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_FreeXosPUTFROMMEM_REQ(t_XOSCOMMHEAD *pMsg)
{
    int rst = XSUCC;
    t_PUTFROMMEM_REQ *pReq = XNULL;

    pReq = (t_PUTFROMMEM_REQ*)pMsg->message;
    if(XNULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), " XNULL == pReq!\n");
        return XERROR;
    }    
    
    if(XNULL != pReq->ucpBuffer)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpBuffer))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpBuffer free failed\n");
            rst = XERROR;
        }
    }

    if(XNULL != pReq->ucpReldir)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpReldir))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpReldir free failed\n");
            rst = XERROR;
        }
    }

    if(XNULL != pReq->ucpFileName)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpFileName))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpFileName free failed\n");
            rst = XERROR;
        }
    }

    if(XNULL != pReq->ucpPass)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpPass))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpPass free failed\n");
            rst = XERROR;
        }
    }

    if(XNULL != pReq->ucpUser)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpUser))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpUser free failed\n");
            rst = XERROR;
        }
    }    

    if(rst == XERROR)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR),"\n FTP_FreePUTFROMMEM_REQ %04x,%04x,%04x,%04x,%04x\n", pReq->ucpBuffer, pReq->ucpReldir, 
              pReq->ucpFileName, pReq->ucpPass, pReq->ucpUser);
    }
    return XSUCC;
}

/************************************************************************
 函数名:FTP_FreeGETTOFILE_REQ
 功能:  释放从服务器下载文件到文件的请求消息
 输入:  pReq 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_FreeGETTOFILE_REQ(t_XOSCOMMHEAD *pMsg)
{

    t_GETTOFILE_REQ *pReq = XNULL;
    pReq = (t_GETTOFILE_REQ*)pMsg->message;
    if(XNULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), " XNULL == pReq!\n");
        return XERROR;
    }
    
    if(XNULL != pReq->ucpSaveFilePath)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpSaveFilePath))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpSaveFilePath free failed\n");
        }
    }

    if(XNULL != pReq->ucpFileName)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpFileName))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpFileName free failed\n");
        }
    }

    if(XNULL != pReq->ucpReldir)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpReldir))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpReldir free failed\n");
        }
    }

    if(XNULL != pReq->ucpPass)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpPass))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpPass free failed\n");
        }
    }

    if(XNULL != pReq->ucpUser)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpUser))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpUser free failed\n");
        }
    }

    if(XNULL != pReq->ucpMd5)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpMd5))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpMd5 free failed\n");
        }
    }

    if(pMsg->message != NULL)
    {
        if(XSUCC != XOS_MemFree(pMsg->datadest.FID, pMsg->message))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pMsg->message free failed\n");
        }
    }

    

    return XSUCC;
}


/************************************************************************
 函数名:FTP_FreePUTFROMFILE_REQ
 功能:  释放从本地文件上传文件的请求消息
 输入:  pReq 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_FreePUTFROMFILE_REQ(t_XOSCOMMHEAD *pMsg)
{

    t_PUTFROMFILE_REQ *pReq = XNULL;
    pReq = (t_PUTFROMFILE_REQ*)pMsg->message;
    if(XNULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), " XNULL == pReq!\n");
        return XERROR;
    }
    
    if(XNULL != pReq->ucpUploadFilePath)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpUploadFilePath))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpUploadFilePath free failed\n");
        }
    }

    if(XNULL != pReq->ucpReldir)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpReldir))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpReldir free failed\n");
        }
    }

    if(XNULL != pReq->ucpFileName)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpFileName))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpFileName free failed\n");
        }
    }

    if(XNULL != pReq->ucpPass)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpPass))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpPass free failed\n");
        }
    }

    if(XNULL != pReq->ucpUser)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpUser))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpUser free failed\n");
        }
    }

    if(pMsg->message != NULL)
    {
        if(XSUCC != XOS_MemFree(pMsg->datadest.FID, pMsg->message))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pMsg->message free failed\n");
        }
    }

    return XSUCC;
}

/************************************************************************
 函数名:FTP_FreeXosPUTFROMFILE_REQ
 功能:  释放从服务器下载文件到文件请求消息
 输入:  pReq 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_FreeXosGETTOFILE_REQ(t_XOSCOMMHEAD *pMsg)
{
    t_GETTOFILE_REQ *pReq = XNULL;
    pReq = (t_GETTOFILE_REQ*)pMsg->message;
    if(XNULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), " XNULL == pReq!\n");
        return XERROR;
    }
    
    if(XNULL != pReq->ucpSaveFilePath)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpSaveFilePath))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpSaveFilePath free failed\n");
        }
    }

    if(XNULL != pReq->ucpFileName)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpFileName))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpFileName free failed\n");
        }
    }

    if(XNULL != pReq->ucpReldir)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpReldir))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpReldir free failed\n");
        }
    }

    if(XNULL != pReq->ucpPass)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpPass))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpPass free failed\n");
        }
    }

    if(XNULL != pReq->ucpUser)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpUser))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpUser free failed\n");
        }
    }

    if(XNULL != pReq->ucpMd5)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpMd5))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpMd5 free failed\n");
        }
    }
    
    return XSUCC;
}

/************************************************************************
 函数名:FTP_FreeXosPUTFROMFILE_REQ
 功能:  释放从本地文件上传文件的请求消息
 输入:  pReq 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS32 FTP_FreeXosPUTFROMFILE_REQ(t_XOSCOMMHEAD *pMsg)
{

    t_PUTFROMFILE_REQ *pReq = XNULL;
    pReq = (t_PUTFROMFILE_REQ*)pMsg->message;
    if(XNULL == pReq)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), " XNULL == pReq!\n");
        return XERROR;
    }
    
    if(XNULL != pReq->ucpUploadFilePath)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpUploadFilePath))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpUploadFilePath free failed\n");
        }
    }

    if(XNULL != pReq->ucpReldir)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpReldir))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpReldir free failed\n");
        }
    }

    if(XNULL != pReq->ucpFileName)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpFileName))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpFileName free failed\n");
        }
    }

    if(XNULL != pReq->ucpPass)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpPass))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpPass free failed\n");
        }
    }

    if(XNULL != pReq->ucpUser)
    {
        if(XSUCC != XOS_MemFree(pMsg->datasrc.FID, pReq->ucpUser))
        {
            XOS_Trace(MD(FID_FTP, PL_ERR), " pReq->ucpUser free failed\n");
        }
    }
    
    return XSUCC;
}



/************************************************************************
 函数名:FTP_ClientMsgPro
 功能:  ftp 模块客户调用消息处理
 输入:  pMsg 消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XS8 FTP_ClientMsgPro(t_XOSCOMMHEAD* pMsg)
{
     if ((pMsg == (XVOID*)XNULLP) || ((t_XOSCOMMHEAD*)pMsg)->message == XNULLP)
     {
         XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_msgProc()->Bad input param !\n");
         return XERROR;
     }

     //XOS_FtpClearXosMsgMem(pMsg); /*直接释放消息中的内存*/
             
     //return XERROR;

     
     /*将指定类型的消息加入到队列*/
     if(eFTP_PUTFROMFILE == pMsg->msgID || eFTP_PUTFROMMEM == pMsg->msgID || eFTP_GETTOFILE == pMsg->msgID)
     {        
         if(FTP_QUE_SUC != XOS_FtpPutMsg(pMsg))/*送入消息队列*/
         {
             XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutMsg failed!\n");
             XOS_FtpClearXosMsgMem(pMsg); /*释放消息中的内存*/

             return XERROR;
         }    
     }

     /*其它的消息不处理*/
     if(eFTP_GETTOFILEAck == pMsg->msgID)
     {
        XOS_Trace(MD(FID_FTP, PL_ERR), "recv msgid %d\n", pMsg->msgID);
     }
     
     return XSUCC;

}

/************************************************************************
 函数名:FTP_MsgProc
 功能:  ftp 模块消息处理函数入口
 输入:  pMsg －消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明: 此消息处理函数是ntl主任务消息入口，edataSend
              消息不在此函数处理的范围内
************************************************************************/
XS8 FTP_MsgProc( XVOID* pMsgP, XVOID* sb)
{

     XS8 Res = 0 ;  /* 返回标志*/

    t_XOSCOMMHEAD* pMsg = (t_XOSCOMMHEAD*)pMsgP;
    
    /*判断消息来源*/
    switch ( pMsg->datasrc.FID )
    {
        case FID_NTL:      /* 处理平台 NTL 消息*/
            Res = FTP_NtlMsgPro(pMsg) ;
            break;
        default: Res = FTP_ClientMsgPro(pMsg);/*处理客户端调用FTP请求消息*/
                 break;
    }
    
    return Res;

            
 
}
/************************************************************************
 函数名:FTP_MsgProc
 功能:  ftp 模块消息处理函数入口
 输入:  pMsg －消息指针
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明: 此消息处理函数是ntl主任务消息入口，edataSend
              消息不在此函数处理的范围内
************************************************************************/
int XOS_CheckFtpCtrl(char *pszBuf, int nLen ,int *pnCode)
{
    t_XOSARRAY *ra = NULL;
    *pnCode = 0;
    if(NULL == pszBuf)
    {
        return XERROR;
    }
    
    ra  = (t_XOSARRAY *)g_ftpMnt.ftpConnectTbl;
    if(NULL != ra)
    {
         XOS_Sprintf(pszBuf, nLen, "%08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x, %08x\n", 
                ra->magicVal, ra->arrayLocation,
                ra->firstVacantElement, ra->lastVacantElement, ra->maxNumOfElements,
                ra->curNumOfElements, ra->sizeofElement, ra->maxUsage, 
                ra->compare, ra->firstNodeLocation, ra->lastNodeLocation);
    }

    if(ra->firstVacantElement > MAX_FTPCONNECTIONS)
    {
        *pnCode = XERROR; /*error */
    }

    return XSUCC;

}

/************************************************************************
函数名: FTP_CliInit()
功能:  命令行初始化
输入:
输出:
返回: 成功返回XSUCC , 失败返回XERROR
说明:
************************************************************************/
XS32 FTP_CliInit(XVOID)
{
   XS32 promptID = 0;

   promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "ftp", "Ftp module", "");

   /*显示ftp 消息队列大小*/
   if (0 > XOS_RegistCommand(promptID, FtpCliShowMsgSize, "showMsgSize", "show ftp msg size", "无参数"))
   {
       XOS_Trace(MD(FID_FTP, PL_ERR), "Failed to register command"  "XOS>ftp>showMsgSize!");
       return XERROR;
   }

   return XSUCC;
}

/************************************************************************
 函数名: FtpCliShowMsgSize()
 功能:    show ftp msg size
 输入:
 输出:
 返回: 成功返回XSUCC , 失败返回XERROR
 说明:
************************************************************************/
XVOID FtpCliShowMsgSize(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XOS_CliExtPrintf(pCliEnv, "ftp msg info   : %-12s%-12s%-12s\r\n", "maxsize", "maxusage", "cursize");
    XOS_CliExtPrintf(pCliEnv, "ftp msg size   : %-12d%-12d%-12d\r\n", 
                     XOS_listMaxSize(gFtpMsgQueue), XOS_listMaxUsage(gFtpMsgQueue), XOS_listCurSize(gFtpMsgQueue));
    XOS_CliExtPrintf(pCliEnv, "ftp remsg size : %-12d%-12d%-12d\r\n", 
                     XOS_listMaxSize(gFtpReMsgQueue), XOS_listMaxUsage(gFtpReMsgQueue), XOS_listCurSize(gFtpReMsgQueue));

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*XOS_FTP_CLIENT*/


