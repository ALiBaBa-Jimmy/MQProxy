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
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xoscfg.h"

#include "xosftp.h"
#include "xosmd5.h"

/*-------------------------------------------------------------------------
                 ģ���ڲ��궨��
-------------------------------------------------------------------------*/

#define MAX_FILENAME_LEN (255)    /*����ļ�������*/
#define MAX_FTPCONNECTIONS (20)   /*�������ͬʱ���ӵ�¼���û���*/
#define MAX_FTPTRANS_TIME (300) /*��λ:��*/


#define MAX_FTPCOMMAND_LEN (256)  /*����ftp����������*/

#define USER_CHECK_NUM (0xaaa)   /*�û����������*/

#define FTP_PASV_MODE (1)           /*�������ӱ���ģʽ*/
#define FTP_PORT (21)        /*ftp�������������ӹ̶��˿�*/

#define FTP_COMMAND_ACK_TIME (5000) /*ftp�������������Ӧʱ��*/
#define FTP_TCPCONNECT_TIME (15000) /*ftp����tcp���ӵ��������ʱ��*/
#define FTP_PACKET_SIZE (4096) /*�ϴ��ļ�ʱÿ�����Ĵ�С*/

#define FTP_FILE_LIST_LEN    (8192*10)  /*�ļ��б��С*/

#ifdef XOS_VXWORKS
#define FTP_SEND_BUFFER_SIZE (4096)  /*VxWorks�·��ʹ��ڵĴ�С*/
#endif

//#define FTP_TEST

/*��Ϣ����*/
XSTATIC XOS_HLIST gFtpMsgQueue; /*����*/
XSTATIC XCONST XS32 gFtpMsgQueueSize = 1024;/*���д�С*/
XSTATIC t_XOSMUTEXID gFtpMsgQueueMutex; /*�ٽ���*/
XSTATIC t_XOSSEMID gFtpQuSemaphore;     /*ͬ���ź���*/


/*�ط���Ϣ����*/
XSTATIC XOS_HLIST gFtpReMsgQueue; /*����*/
XSTATIC XCONST XS32 gFtpReMsgQueueSize = 100;/*���д�С*/
XSTATIC t_XOSMUTEXID gFtpReMsgQueueMutex; /*�ٽ���*/
XSTATIC t_XOSSEMID gFtpReQuSemaphore;     /*ͬ���ź���*/
XSTATIC XVOID* FTP_CliMsgReDealTask(void* paras);


#define MAX_DATA_THREAD (4) /*����ش�С*/
#define MAX_TIMER_COUNT (4+1+45) /*��ʱ������*/
#define NO_SEND_ELAPSES (0)     /*û�з��ʹ������*/
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
                 ģ���ڲ��ṹ��ö�ٶ���
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ��� // = {XFALSE,  XNULL, XNULL};
-------------------------------------------------------------------------*/
 t_FTPMNT g_ftpMnt;

/*-------------------------------------------------------------------------
                ģ���ڲ�����
-------------------------------------------------------------------------*/
/************************************************************************
 ������:FTP_BuildLinkH
 ����: �����û����
 ����: isCtrlLink  �� ��������(��4λ)���������ӻ�����������
                                  XTRUE��ʾ�������ӣ�XFALSE��ʾ��������
              userIndex  �� �û���Ϣ����
 ���:  ��
 ����:  �����û����
 ˵��:  �û������֯��ͼ��

                linkType  checkcoder          userIndex
               31      27                   15                            0 (bit)
               +----+------------+----------------+
               |        |                      |                            |
               +----+------------+----------------+

************************************************************************/
#define FTP_BuildLinkH( isCtrlLink, userIndex) \
    (XOS_HFTPCLT)(XPOINT)((((isCtrlLink)&0xf)<<28) |(USER_CHECK_NUM<<16) | (((userIndex)&0xffff)))

/************************************************************************
 ������:FTP_IsValidLinkH
 ����: ��֤��·�������Ч��
 ����: ��·���
 ���:
 ����: ��Ч����XTURE, ���򷵻�XFALSE
 ˵��:
************************************************************************/
#define FTP_IsValidLinkH(userHandle)  \
    ((USER_CHECK_NUM == ((XPOINT)(userHandle)&(USER_CHECK_NUM<<16))>>16)? XTRUE:XFALSE)

/************************************************************************
 ������:FTP_GetLinkType
 ����: ͨ���û������ȡ��·����
 ����: �û����
 ���: ��
 ����: ��·����
 ˵��:
************************************************************************/
#define FTP_GetLinkType(userHandle)  ((XBOOL)(((XU32)(XPOINT)(userHandle))>>28))

/************************************************************************
 ������:FTP_GetLinkIndex
 ����: ͨ���û������ȡ��·Index
 ����: �û����
 ���: ��
 ����: ��·����
 ˵��:
************************************************************************/
#define FTP_GetLinkIndex(userHandle) ((XS32)(((XPOINT)(userHandle))&0xffff))

#define FTP_TIMER_STOP(TimerId)\
if(TimerId)\
{\
    XOS_TimerStop(FID_FTP, TimerId);\
    XOS_INIT_THDLE(TimerId);\
}

/************************************************************************
 ������:FTP_IsValidUserHandle
 ����: �ж��û�����Ƿ���Ч
 ����: �û����
 ���: pUserInfo    - �û���Ϣ����
 ����: XTRUE   - ��Ч�û�;XFALSE   - ��Ч�û�
 ˵��:
************************************************************************/
XSTATIC XBOOL FTP_IsValidUserHandle(XOS_HFTPCLT hFtpClt, XVOID **pUserInfo)
{
    XS32 userId = 0;
    
    /*������֤*/
    if (XFALSE == FTP_IsValidLinkH(hFtpClt) || XNULLP == pUserInfo)
    {
        return XFALSE;
    }
    
    /*��ȡ�û��������û���Ϣ*/
    userId = FTP_GetLinkIndex(hFtpClt);
    if (XNULLP == (*pUserInfo = XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, userId)))
    {
        return XFALSE;
    }
    
    /*�ж��û���ǰ״̬*/
    if (eFTPSTATELOGIN != ((t_XOSFTPCLIENT*)(*pUserInfo))->curState)
    {
        return XFALSE;
    }
    
    return XTRUE;
}

/************************************************************************
 ������:FTP_TimerSet
 ����: ���ö�ʱ��������
 ����: pUserInfo  - �û���Ϣ��
              timerPara  - ��ʱ������
              backPara   - �ص���������
              timerLen    - ��ʱʱ�䳤��
 ���: ��
 ����: ��
 ˵��:
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
    timerPara->len = timerLen;/*ʱ�䳤��*/
    timerPara->mode = TIMER_TYPE_ONCE;/*��ʱ������*/
    timerPara->pre = TIMER_PRE_LOW;/*��ʱ������*/
    XOS_MemSet(backPara, 0, sizeof(t_BACKPARA));
    XOS_INIT_THDLE(pUserInfo->timerId);/*��ʼ����ʱ�����*/
    backPara->para1 = (XPOINT)pUserInfo;/*������Ҫ�Ĳ���*/

    return XSUCC;
}

/************************************************************************
 ������:FTP_LinkInit
 ����:  ftp��·��ʼ����Ϣ��װ����
 ����:  userId     - �û�����
               linkType  - ��·����
               isCtrlLink - �Ƿ�Ϊ������·
 ���: ��
 ����: ��Ϣ���ͳɹ�����XSUCC , ����ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XSTATIC XS32  FTP_LinkInit(XS32 userId, XU16 linkType, XBOOL isCtrlLink)
{
    t_XOSCOMMHEAD *pMsg = XNULLP;
    t_LINKINIT tLinkInit;
    
    /*������Ϣ��Ҫ�Ŀռ�*/
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_LINKINIT));
    if (XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "FTP_LinkInit()-> malloc message memory fail!\n");
        return XERROR;
    }
    
    /*������Ϣ����*/
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
    
    if (XSUCC != XOS_MsgSend(pMsg))/*������Ϣ*/
    {
        XOS_MsgMemFree(FID_FTP, pMsg);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_LinkInit()-> message send fail !\n");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
 ������:FTP_LinkStart
 ����:  ��·������Ϣ��װ����
 ����:  linkHandle      - ��·���
               linkType         - ��·����
               pAddr            - �Զ˵�ַ
 ���: ��
 ����: ��Ϣ���ͳɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
    
    /*������Ϣ��Ҫ�Ŀռ�*/
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_LINKSTART));
    if (XNULLP == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_LinkStart()-> malloc message memory fail!\n");
        return XERROR;
    }
    
    /*������Ϣ������*/
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
    case eTCPServer:/*tcp�����*/
        XOS_MemCpy(&(tLinkStartReq.linkStart.tcpServerStart.myAddr), pAddr, sizeof(t_IPADDR));
        tLinkStartReq.linkStart.tcpServerStart.allownClients = 2;
        tLinkStartReq.linkStart.tcpServerStart.authenFunc = XNULLP;
        break;
    case eTCPClient:/*tcp�ͻ���*/
        XOS_MemSet(&(tLinkStartReq.linkStart.tcpClientStart.myAddr), 0, sizeof(t_IPADDR));
        XOS_MemCpy(&(tLinkStartReq.linkStart.tcpClientStart.peerAddr), pAddr, sizeof(t_IPADDR));
        break;
    default:
        break;
    }
    XOS_MemCpy(pMsg->message, &tLinkStartReq, sizeof(t_LINKSTART));
    
    if (XSUCC != XOS_MsgSend(pMsg))/*������Ϣ*/
    {
        XOS_MsgMemFree(FID_FTP, pMsg);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_LinkStart()-> message send fail !\n");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
 ������:FTP_LinkRelease
 ����:  ��·�ͷ���Ϣ��װ����
 ����:  linkHandle      - ��·���
 ���: ��
 ����: ��Ϣ���ͳɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XSTATIC XS32  FTP_LinkRelease(HLINKHANDLE linkHandle)
{
    t_XOSCOMMHEAD *pMsg = XNULLP;
    t_LINKRELEASE tLinkRelease;
    
    /*������Ϣ����Ҫ�Ŀռ�*/
    if (XNULLP == (pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_FTP, sizeof(t_LINKRELEASE))))
    {
        XOS_Trace(MD(FID_FTP,PL_EXP),"FTP_LinkRelease()-> malloc msg mem fail\n");
        return  XERROR;
    }
    
    /*������Ϣ�������*/
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
    
    if (XSUCC != XOS_MsgSend(pMsg))/*������Ϣ*/
    {
        XOS_MsgMemFree(FID_FTP, pMsg);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_LinkRelease()-> message send fail !\n");
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
 ������:FTP_SingleComSend
 ����:  ftp����͵ķ�װ����
 ����:  pUserInfo        - �û���Ϣ
               pCmd             - ftp����
               pArg1             - ftp����Ĳ���
               pArg2             - ftp����Ĳ���
 ���: ��
 ����: ��Ϣ���ͳɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XSTATIC XS32 FTP_SingleComSend(t_XOSFTPCLIENT *pUserInfo, XS8* pCmd, XS8* pArg1, XS8* pArg2)
{
    XS32 cmdSize = 0;/*����ĳ���*/
    XS8 buffer[MAX_FTPCOMMAND_LEN] = {0};/*���ڱ�������*/
    XS8* pData = XNULLP;/*����ָ��*/
    t_XOSCOMMHEAD *pMsg = XNULLP;/*��Ϣͷָ��*/
    t_DATAREQ tDataReq;
    
    /*������֤*/
    if (XNULLP == pUserInfo || XNULLP == pCmd || XNULLP == pArg1 || XNULLP == pArg2 ||
        XOS_StrLen(pArg1)+XOS_StrLen(pArg2)+XOS_StrLen(pCmd) > MAX_FTPCOMMAND_LEN-1)
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "FTP_SingleComSend()-> Bad input param !\n");
        return XERROR;
    }

    XOS_Sprintf(buffer,sizeof(buffer)-1, "%s%s%s", pCmd,pArg1,pArg2);
    cmdSize = (XS32)XOS_StrLen(buffer);
    
    /*������Ϣ�ռ�*/
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
    /*������Ϣͷ����*/
    pMsg->datasrc.FID = FID_FTP;
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->length = sizeof(t_DATAREQ);
    pMsg->msgID = eSendData;
    pMsg->prio = eNormalMsgPrio;
    
    /*��д��Ϣ�е���������*/
    XOS_MemSet(&tDataReq, 0, sizeof(t_DATAREQ));
    tDataReq.linkHandle = pUserInfo->linkHandle;
    XOS_MemCpy(&(tDataReq.dstAddr), &(pUserInfo->dstAddr), sizeof(t_IPADDR));
    tDataReq.msgLenth = cmdSize+2;
    tDataReq.pData = pData;
    XOS_MemCpy(pMsg->message, &tDataReq, sizeof(t_DATAREQ));
    
    if (XSUCC != XOS_MsgSend(pMsg))/*������Ϣ*/
    {
        /*����ָʾ��Ϣ��Ӧ�����ͷ��յ�����*/
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
 ������:FTP_ComSendWithTimeOut
 ����:  ����ʱ���ĵ�һ�����
 ����:  pUserInfo        - �û���Ϣ
               pCmd             - ftp����
               pArg1             - ftp����Ĳ���
               pArg2             - ftp����Ĳ���
               curEvent         - ��ǰ�¼�
 ���: ��
 ����: ��Ϣ���ͳɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
    
    /*���ö�ʱ��*/
    if(XSUCC != FTP_TimerSet(pUserInfo, &timerPara, &backPara, FTP_COMMAND_ACK_TIME))
    {
        XOS_INIT_THDLE(pUserInfo->timerId);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ComSendWithTimeOut()-> FTP_TimerSet  fail!\n");
        return XERROR;
    }

    /*������ʱ��*/
    if (XERROR == XOS_TimerStart(&(pUserInfo->timerId), &timerPara, &backPara))
    {
        XOS_INIT_THDLE(pUserInfo->timerId);
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ComSendWithTimeOut()-> timer start fail!\n");
        return XERROR;
    }
    
    /*����ftp����*/
    if (XERROR == FTP_SingleComSend(pUserInfo, pCmd, pArg1, pArg2))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ComSendWithTimeOut()-> Command send error!\n");

        FTP_TIMER_STOP(pUserInfo->timerId);
        return XERROR;
    }

    XOS_SemGet(&(pUserInfo->semaphore)); /*�ȴ�150*/
    
    return (pUserInfo->ret == XSUCC) ? XSUCC : XERROR;
}

/************************************************************************
 ������:FTP_GetComReplyCode
 ����:  ȡӦ����
 ����:  replyStr           - ftpӦ����Ϣ
               dataLength       - ftpӦ��ĳ���
 ���: ��
 ����: ftpӦ����
 ˵��:
************************************************************************/
XSTATIC XS32  FTP_GetComReplyCode(XS8* replyStr, XU32 dataLength)
{
    XS32 replyCode = 0;
    
    sscanf(replyStr, "%d", &replyCode);
    return replyCode;
}

/************************************************************************
 ������:FTP_ReplySpecialParse
 ����:  �Կ��ܳ�������Ӧ�����Ϣ���д���
 ����:  replyStr           - ftpӦ����Ϣ
               dataLength       - ftpӦ��ĳ���
 ���: pUserInfo         - �û���Ϣ
 ����: ֻ��һ��Ӧ�𷵻�XSUCC,����Ӧ�𷵻�XERROR
 ˵��:
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
    if (dataLength == (XU32)(pCh - replyStr + 2))/*ֻ��׼��Ӧ���򷵻�*/
    {
        return XSUCC;
    }
    
    /*�����2��Ӧ��,ֻ�������ļ�����ʱ����*/
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
 ������:FTP_GetCurDir
 ����:  ���յ���ftpӦ�������л�ȡ��ǰ·��
 ����:  replyStr           - ftpӦ����Ϣ
               dataLength       - ftpӦ��ĳ���
 ���:  pDir                - ��ǰ·��
 ����: �ɹ�����XSUCC��ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XSTATIC XS32  FTP_GetCurDir(XS8* replyStr, XU32 dataLength, XS8* pDir, XU32 nLen)
{
    XU32 i = 0;
    XS8* pCh = XNULLP;
    
    /*��������*/
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
 ������:FTP_GetFileSize
 ����:  ���յ��������л�ȡ�ļ���С��Ϣ
 ����:  replyStr           - ftpӦ����Ϣ
               dataLength       - ftpӦ��ĳ���
 ���:  pFileSize          - �ļ���С
 ����: �ɹ�����XSUCC��ʧ�ܷ���XERROR
 ˵��:
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
 ������:FTP_GetPasvServerAddr
 ����:  ���յ��������л�ȡ�������ӵĶԶ˵�ַ
 ����:  replyStr           - ftpӦ����Ϣ
 ���:  pAddr              - ���������жԷ���ַ
 ����: �ɹ�����XSUCC��ʧ�ܷ���XERROR
 ˵��:
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
 ������:FTP_RecvInitAck
 ����:  ������·��ʼ����Ϣ��Ӧ����Ϣ
 ����:  pMsg            - ftp��·��ʼ����Ӧ��Ϣ
 ���:  ��
 ����:  ��
 ˵��:
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
    if (FTP_GetLinkType((XOS_HFTPCLT)(pInitAck->appHandle)))/*������·��ʼ��*/
    {
        pUserInfo = (t_XOSFTPCLIENT*)XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, index);
        if (XNULLP != pUserInfo)
        {
            if (eSUCC == pInitAck->lnitAckResult)/*��·��ʼ���ɹ�������start*/
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
 ������:FTP_RecvStartAck
 ����:  ������·��ʼ����Ϣ��Ӧ����Ϣ
 ����:  pMsg            - ftp��·������Ӧ��Ϣ
 ���:  ��
 ����:  ��
 ˵��:
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
    if (FTP_GetLinkType((XOS_HFTPCLT)(pStartAck->appHandle)))/*������·����*/
    {
        pUserInfo = (t_XOSFTPCLIENT*)XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, index);
        if (XNULLP != pUserInfo)
        {
            switch (pStartAck->linkStartResult)
            {
            case eSUCC:/*��·�����ɹ��������û���*/
                XOS_MemCpy(&(pUserInfo->locAddr), &(pStartAck->localAddr), sizeof(t_IPADDR));
                break;
                
            case eFAIL:/*����ʧ�ܣ��ͷ���·*/
                       /*                    FTP_LinkRelease(pUserInfo->linkHandle);
                XOS_SemPut(&(pUserInfo->semaphore));*/
                XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_StartAck()->control link start fail !\n");
                break;
                
            case eBlockWait:/*����������*/
                /*                    XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_StartAck()->control link connecting !\n");*/
                break;
                
            default:/*δ֪����Ч��Ϣ*/
                XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_StartAck()->unknowed result !\n");
                break;
            }
        }
    }
}

/************************************************************************
 ������:FTP_bulidDataLink
 ����:  �����ļ��������������
 ����:  pServAddr     - �Զ˵�ַ
 ���:  pSockId        - socket
 ����:  �ɹ�����XSUCC, ʧ�ܷ���XERROR
 ˵��:  �õ����������ģ�������������ѣ����߶Զ�
               servû������ʱ���������൱һ��ʱ��
************************************************************************/
XSTATIC XS32 FTP_bulidDataLink(t_IPADDR* pServAddr, t_XINETFD* pSockId)
{
    XS32 ret = 0;
    XS32 optVal = 0;
    //struct linger optLinger;
    
    /*������֤*/
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
    
    /*���ó�����ģʽ*/
    optVal = XOS_INET_OPT_ENABLE;
    XINET_SetOpt(pSockId, SOL_SOCKET, XOS_INET_OPT_BLOCK, (XU32 *)&optVal);

    /*Linger*/    
    //optLinger.l_onoff = 1;
    //optLinger.l_linger = 0;
    //ret = setsockopt(pSockId->fd, SOL_SOCKET, SO_LINGER, (char*)&optLinger, sizeof(optLinger));

    
#ifdef XOS_VXWORKS
    /*��VxWorks�����÷��ʹ��ڴ�С*/
    optVal = FTP_SEND_BUFFER_SIZE;
    XINET_SetOpt(pSockId, SOL_SOCKET, XOS_INET_OPT_TX_BUF_SIZE, &optVal);
#endif
    
    /*���ӶԶ�*/
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
 ������:FTP_dataTskEntry
 ����: �ϴ� �����ļ����ݵ�������ں���
 ����: pPutEnty          - ������ڲ���
 ���: ��
 ����: ��
 ˵��: ���������������·���ϴ��������Զ��˳�
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
        /*�ϴ��ļ�ģʽ*/
    case eFTPPUTFROMFILE:
        while(pendLen>0)
        {
            readLen = (XS32)XOS_MIN(FTP_PACKET_SIZE, pendLen);
            /*��ȡ�ļ�*/
            ret = XOS_ReadFile((XVOID*) recvBuff, 1, readLen, pPutEnty->pFile);
            if(ret == XERROR)
            {
                XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()->read file error!");
                break;
            }
            
            /*��������*/
            ret = XINET_SendMsg(pPutEnty->pSockId,eTCPClient,&(pPutEnty->ftpServAddr), readLen, (char*)recvBuff,&out_len);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()->put file block failed!");
                break;
            }
            
            pendLen = pendLen -readLen;
        }
        /*�ϴ���ر�sock*/
        XINET_CloseSock(pPutEnty->pSockId);
        pPutEnty->pSockId->fd = XERROR;
        XOS_SemPut(pPutEnty->pSem);        
        break;
        
        /*�ϴ��ڴ��ģʽ*/
    case eFTPPUTFROMMEM:
        pPendData = (XCHAR*)pPutEnty->pBuffer;
        
        while(pendLen>0)
        {
            readLen = (XS32)XOS_MIN(FTP_PACKET_SIZE, pendLen);
            
            /*��������*/
            ret = XINET_SendMsg(pPutEnty->pSockId,eTCPClient,&(pPutEnty->ftpServAddr), readLen, pPendData,&out_len);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_FTP, PL_ERR),  " FTP_dataTskEntry()->put mem block failed!");
                break;
            }
            pendLen = pendLen -readLen;
            pPendData = pPendData +readLen;
            
        }
        
        /*�ϴ���ر�sock*/
        XINET_CloseSock(pPutEnty->pSockId);
        pPutEnty->pSockId->fd = XERROR;
        XOS_SemPut(pPutEnty->pSem);        
        break;
        
    case eFTPGETTOFILE:

        pPutEnty->ulDownLen = 0;
        /*��������������*/
        while(1)
        {
            readLen = recv((pPutEnty->pSockId)->fd, (char*)recvBuff, FTP_PACKET_SIZE, 0);
            if(readLen <= 0)
            {
                /*�ر�sock*/
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
        
        /*��������������*/
        sumLen = 0;
        pPendData = (XCHAR*)pPutEnty->pBuffer;
        while(1)
        {
            readLen = recv((pPutEnty->pSockId)->fd, (char*)recvBuff, FTP_PACKET_SIZE, 0);
            if(readLen <= 0)
            {
                /*�ر�sock*/
                XINET_CloseSock(pPutEnty->pSockId);
                pPutEnty->pSockId->fd = XERROR;
                break;
            }
            
            /*�����ڴ�Խ��*/
            sumLen += readLen;
            if(sumLen > pPutEnty->buffLen)
            {
                /*�ر�sock*/
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
        /*�����ļ��б��ڴ�*/
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
        
        /*��������������*/
        sumLen = 0; /*�ܹ����յ�������*/
        while(1)
        {
            readLen = recv((pPutEnty->pSockId)->fd, (char*)recvBuff, FTP_PACKET_SIZE, 0);
            
            if(readLen <= 0)
            {
                /*�ر�sock*/
                XINET_CloseSock(pPutEnty->pSockId);
                pPutEnty->pSockId->fd = XERROR;
                pPutEnty->buffLen = sumLen;
                break;
            }
            
            /*�����ڴ�Խ��*/
            sumLen +=readLen;
            if(sumLen >= FTP_FILE_LIST_LEN)
            {
                /*�ر�sock*/
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
 ������:FTP_dataTskWithTimeOut
 ����:  ����ʱ���������ִ��
 ����:  pUserInfo            - �û���Ϣ����
               pTskEntry            - ������ڲ���
               timeLen               - ��������ִ�е�ʱ�䳤��
 ���:  ��
 ����:  ��
 ˵��:
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
    
    /*������ʱ��*/
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
    
    /*����һ������������������ݴ���*/
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
    XOS_SemGet(pTskEntry->pSem); /*����ͨ��ͬ��*/

    /*���226����Ӧ�����й����Σ�գ�ֻ�еȵ����������ʱ����ʱ*/
    XOS_SemGet(&(pUserInfo->semaphore));/*����ͨ��ͬ��*/
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
 ������:FTP_RecvDataInd
 ����:  ����������Ϣ��Ӧ����Ϣ
 ����:  pMsg            - ftp��·������Ӧ��Ϣ
 ���:  ��
 ����:  ��
 ˵��:
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

    index = (XS32)FTP_GetLinkIndex((XOS_HFTPCLT)(pDataInd->appHandle)); /*ntl��·����*/
    /*������·��Ϣ*/
    if (FTP_GetLinkType((XOS_HFTPCLT)(pDataInd->appHandle)))/*������·����ָʾ*/
    {
        replyCode = FTP_GetComReplyCode(pDataInd->pData, pDataInd->dataLenth);
        
        pUserInfo = (t_XOSFTPCLIENT*)XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, index);

        /*���û�״̬���в�����δ����*/
        if (XNULLP != pUserInfo)
        {
            switch (pUserInfo->curEvent)
            {

            /*��¼�׶�״̬���û�������ʱ�����û��������޸ĵ�¼���û�*/                
            case eFTPNONE: /*����������·ʱ���յ��������Ļ�ӭ��Ϣ*/
                if (220 == replyCode)
                {
                    pUserInfo->curEvent = eFTPNAME;
                    /*�����û���*/
                    FTP_SingleComSend(pUserInfo, "USER ", pUserInfo->userName, "");
                }                
                break;
            case eFTPNAME:/*�յ��û���Ӧ��*/
                if (230 == replyCode && 0 == XOS_StrCmp(pUserInfo->passWord, "")) //�û��ѵ�¼��������Ϊ��
                {
                    pUserInfo->curEvent = eFTPTYPE;
                    FTP_SingleComSend(pUserInfo, "TYPE I", "", "");
                }
                else if (331 == replyCode)/*�϶�Ӧ��ͷ�������*/
                {
                    pUserInfo->curEvent = eFTPPASS;
                    FTP_SingleComSend(pUserInfo, "PASS ", pUserInfo->passWord, "");
                }
                break;
                
            case eFTPPASS:/*�յ�����Ӧ��,����ȡĿ¼����*/
                if (230 == replyCode)
                {
                    pUserInfo->curEvent = eFTPTYPE;
                    FTP_SingleComSend(pUserInfo, "TYPE I", "", "");
                }
                else //������֤���󣬵�¼ʧ�� (�������������Ҳ������Ȩ�޲�������������ԭ��)
                {
                    /*ֹͣ���Ӷ�ʱ��*/
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    /*�ͷ��ź���*/
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;                
                
            case eFTPTYPE:/*�յ��ļ��������͵�Ӧ��*/
                if (200 == replyCode)/**/
                {
                    if (eFTPSTATEDISCON == pUserInfo->curState)/*�ϴζϿ�����ٴ�����*/
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

            case eFTPCWD:/*�յ��ı䵱ǰĿ¼��Ӧ��*/
                if (eFTPSTATEDISCON == pUserInfo->curState && 250 == replyCode)/*����*/
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
                
            case eFTPDEFDIR:/*�յ���ȡĿ¼��Ӧ��*/
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

                    /*��¼�ɹ����ͷ��ź���*/
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                break;
/*��¼�׶�*/
                
                
            case eFTPSIZE:/*�յ���ȡ�ļ���С��Ӧ��*/
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
                
            case eFTPMKD:/*�յ�����Ŀ¼��Ӧ��*/
                FTP_TIMER_STOP(pUserInfo->timerId);
                if (257 == replyCode || 200 == replyCode)
                {
                    pUserInfo->ret = XSUCC;
                }
                XOS_SemPut(&(pUserInfo->semaphore));
                break;
                
            case eFTPRMD:/*�յ�ɾ��Ŀ¼��Ӧ��*/
                FTP_TIMER_STOP(pUserInfo->timerId);
                if (250 == replyCode || 200 == replyCode)
                {
                    pUserInfo->ret = XSUCC;
                }
                XOS_SemPut(&(pUserInfo->semaphore));
                break;
                
            case eFTPRMF:/*�յ�ɾ���ļ���Ӧ��*/
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
            case eFTPPASV:/*�յ�������ʽ�����Ӧ��*/
                if (227 == replyCode)
                {
                    if (XERROR != FTP_GetPasvServerAddr(pDataInd->pData, &(pUserInfo->dataLinkAddr)))
                    {
                        pUserInfo->curEvent = pUserInfo->curTransfer;
                        /*                            pUserInfo->ret = XSUCC;*/
                        /*����������·*/
                        if (XERROR == FTP_bulidDataLink(&(pUserInfo->dataLinkAddr), &(pUserInfo->dataSockId)))
                        {
                            XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RecvDataInd()->build data link error!\n");
                            break;
                        }
                        
                        /*�����ϴ������ļ�������*/
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
            case eFTPPORT:/*�յ�port�����Ӧ��*/
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
            case eFTPCREATFILE:/*�յ������ļ���Ӧ��*/
                if (150 == replyCode || 125 == replyCode)  /*��һ����Ӧ*/
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else if (226 == replyCode)  /*�ڶ����Զ���Ӧ*/
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
                
/*���ص��ļ��ɹ�*/                
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
/*���ص��ڴ�ɹ�*/                
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
                
/*��ȡָ��Ŀ¼���ļ��б�ɹ�*/                
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

/*���ļ��ϴ��ɹ�*/                
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
/*���ڴ��ϴ��ɹ�*/                
            case eFTPPUTFROMMEM: /*�ϴ��ļ�����Ӧ*/
                if (150 == replyCode || 125 == replyCode)
                {
                    FTP_TIMER_STOP(pUserInfo->timerId);
                    pUserInfo->ret = XSUCC;
                    XOS_SemPut(&(pUserInfo->semaphore));
                }
                else if (226 == replyCode) /*�ļ��ϴ��ɹ���Ӧ*/
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
                if (350 == replyCode) //����������ִ��
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
                if (250 == replyCode) //�������ɹ�
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
 ������:FTP_RecvStopInd
 ����:  ������·�ر�ָʾ����Ϣ
 ����:  pMsg            - ftp��·������Ӧ��Ϣ
 ���:  ��
 ����:  ��
 ˵��:
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
    if (FTP_GetLinkType((XOS_HFTPCLT)(pCloseInd->appHandle)))/*������·�ر�ָʾ*/
    {
        pUserInfo = (t_XOSFTPCLIENT*)XOS_ArrayGetElemByPos(g_ftpMnt.ftpConnectTbl, index);
        if (XNULLP != pUserInfo)
        {
            if (pUserInfo->curState == eFTPSTATELOGIN)
            {
                pUserInfo->curState = eFTPSTATEDISCON;
            }/*�޸ĵ�ǰ״̬Ϊ������*/

            pUserInfo->curEvent = eFTPNONE;
            
            /*�������ļ���������еĶϿ�*/
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
 ������:FTP_RecvErrorSend
 ����:  ������·���ݷ��ʹ������Ϣ
 ����:  pMsg            - ftp��·���ݷ�����Ӧ��Ϣ
 ���:  ��
 ����:  ��
 ˵��:
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
 ������:FTP_RemoveDir
 ����:  ɾ��Ŀ¼
 ����:  pUserInfo            - ftp��·������Ӧ��Ϣ
               pFolderName       - �ļ�����
 ���:  ��
 ����:  �ɹ�����XSUCC,ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XSTATIC XS32 FTP_RemoveDir( t_XOSFTPCLIENT *pUserInfo, XS8 *pFolderName )
{
    if(NULL == pUserInfo || NULL == pFolderName)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_RemoveDir para is null");
        return XERROR;
    }
    /*����ɾ��Ŀ¼������*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "RMD ", pUserInfo->curDir, pFolderName, eFTPRMD))
    {
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
 ������:FTP_DeleteFile
 ����:  ɾ���ļ�
 ����:  pUserInfo            - ftp��·������Ӧ��Ϣ
               pFileName            - �ļ���
 ���:  ��
 ����:  �ɹ�����XSUCC,ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XSTATIC XS32 FTP_DeleteFile( t_XOSFTPCLIENT *pUserInfo, XS8 *pFileName )
{
    if(NULL == pUserInfo || NULL == pFileName)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_DeleteFile para is null");
        return XERROR;
    }
    
    /*����ɾ���ļ�������*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "DELE ", pFileName, "", eFTPRMF))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_DeleteFile()-> Remove file fail !\n");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
 ������:FTP_ListParser
 ����:  ������ǰĿ¼�µ��ļ���
 ����:  pData            - �ļ��б���Ϣ
 ���:  fName           - �ļ���
 ����:  �ɹ�����XSUCC,ʧ�ܷ���XERROR
 ˵��:
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
    for (i=0; i<8; i++)/*ȷ���ļ�������ʼλ��*/
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
 ������:FTP_ListToDele
 ����:  �Ȼ�ȡ�ļ��б�,Ȼ����һɾ���ļ����ļ���
 ����:  pUserInfo               - �û���Ϣ
 ���:  pFolderName           - �ļ�����
 ����:  �ɹ�����XSUCC,ʧ�ܷ���XERROR
 ˵��:
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
    pUserInfo->bufferSize = 0;/*Ŀ¼�б����ݴ�С*/
    pUserInfo->pBuffer = XNULLP;/*Ŀ¼�б�����*/
    pLocation = (XS8*)pUserInfo->curFolder;/*��ǰ�ļ��е�·��(����ڵ�ǰĿ¼)*/

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
    /*fDirɾ������ʱʹ��*/
    XOS_MemCpy(fDir, pUserInfo->curFolder, XOS_StrLen(pUserInfo->curFolder)+1);
    pUserInfo->count = pUserInfo->count + 1;/*Ŀ¼���*/
    
    /*������ʽ����*/
    pUserInfo->replyInfoType = ePREPARE_SUC;
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "FTP_ListToDele()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*��ʼ�����ļ��б�*/
    tTskEntry.curEvent = eFTPLIST;
    tTskEntry.pSockId = &(pUserInfo->dataSockId);
    tTskEntry.pSem = &(pUserInfo->semaphore);
    
    FTP_dataTskWithTimeOut(pUserInfo, &tTskEntry, 5);
    
    pUserInfo->curTransfer = eFTPNONE; /*�����������*/
    
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
        /*��ʼ��������ļ��б�����ݲ�����ɾ������*/
        pOffset = pData;
        while (pOffset - pData < dataLength)
        {
            switch (*pOffset)
            {
            case 'd':/*�ļ���*/
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
            case '-':/*�ļ�*/
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
                                  ģ��ӿں���
-------------------------------------------------------------------------------------------*/
/************************************************************************
������:    XOS_FtpLogin
���ܣ���¼ftp������
���룺serAddr           -ftp������ip
                userName         -�û���¼�ʺ�
                passwd             -�û���¼����
�����hFtpClt             -ftp�û����
      �þ������Ϊntl��·���û�ID;Ҳ��Ϊftp���û�ID.������һ�µ�.
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpLogin(t_IPADDR *serAddr, XS8 *userName, XS8 *passWd, XOS_HFTPCLT *hFtpClt)
{
    XS32 userId = 0;
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_PARA timerPara;
    t_BACKPARA backPara;
    
    /*��������*/
    if (XFALSE == g_ftpMnt.initialized || XNULLP == serAddr || XNULLP == userName || 0 ==XOS_StrCmp(userName, "" ) || XNULLP == passWd ||
        XNULLP == hFtpClt || XOS_StrLen(userName)>=MAX_USERNAME_LEN || XOS_StrLen(passWd)>=MAX_PASSWORD_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR),"XOS_FtpLogin()-> Bad input param !\n");
        return XERROR;
    }
    
    /*��¼�û���Ϣ,�����û��ռ䣬�������û�ID�����뵽�����*/
    XOS_MutexLock(&(g_ftpMnt.contrlTblLock));
    userId = XOS_ArrayAddExt(g_ftpMnt.ftpConnectTbl, (XVOID**)&pUserInfo);
    XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));

    /*�����ڴ�ʧ��*/
    if (XNULLP == pUserInfo)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR),"XOS_FtpLogin()-> too many ftp connection !\n");
        return XERROR;
    }
    XOS_MemSet(pUserInfo, 0, sizeof(t_XOSFTPCLIENT));
    
    memcpy(pUserInfo->userName, userName, XOS_MIN(XOS_StrLen(userName), sizeof(pUserInfo->userName)-1));
    memcpy(pUserInfo->passWord, passWd, XOS_MIN(XOS_StrLen(passWd), sizeof(pUserInfo->passWord)-1));
    XOS_MemCpy(&(pUserInfo->dstAddr), serAddr, sizeof(t_IPADDR));
    
    /*�����û��ź���*/
    pUserInfo->ret = XERROR;
    pUserInfo->dataSockId.fd = XERROR;
    if (XERROR == XOS_SemCreate(&(pUserInfo->semaphore), 0))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR),"XOS_FtpLogin()-> create semaphore fail!\n");
        goto ftpparaError;
    }
    
    /*�������Ӷ�ʱ��*/
    FTP_TimerSet(pUserInfo, &timerPara, &backPara, FTP_TCPCONNECT_TIME);
    XOS_INIT_THDLE(pUserInfo->timerId);
    if (XERROR == XOS_TimerStart(&(pUserInfo->timerId), &timerPara, &backPara))
    {
        XOS_SemDelete(&(pUserInfo->semaphore));
        XOS_Trace(MD(FID_FTP, PL_ERR),"XOS_FtpLogin()-> timer start fail!\n");
        goto ftpparaError;
    }
    
    /*������·��ʼ��*/
    if (XSUCC != FTP_LinkInit(userId, eTCPClient, XTRUE))
    {
        XOS_SemDelete(&(pUserInfo->semaphore));
        FTP_TIMER_STOP(pUserInfo->timerId);
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpLogin()-> FTP_LinkInit send fail !\n");
        goto ftpLoginError;
    }

    /*�ȴ��ź������ã��ɵ�¼�ɹ��ͷ��ź���*/
    XOS_SemGet(&(pUserInfo->semaphore));

    /*��¼ʧ�ܣ��ɳ�ʱ�ӿ��ͷ��ź���*/
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
    
ftpLoginError: /*ʧ��ʱ���ͷ�������Դ*/

    /*����ѷ����TCP���ӣ����ͷ�����, ���ntl��Ϣʧ��ҲҪ����*/
    //if (pUserInfo->curState != eFTPSTATEINIT || 1)
    {
        if(eFTPSTATELOGIN == pUserInfo->curState) /*say goodbye*/
        {
            if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "QUIT", "", "", eFTPQUIT))
            {
                XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpClose()-> send QUIT fail!\n");
            }
        }
        
        /*�ͷ���·*/
        FTP_LinkRelease(pUserInfo->linkHandle);
    }

ftpparaError:
    /*ɾ���û���Դ*/
    XOS_MutexLock(&(g_ftpMnt.contrlTblLock));
    XOS_ArrayDeleteByPos(g_ftpMnt.ftpConnectTbl, userId);
    XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
    
    return XERROR;
}

/************************************************************************
������:    XOS_FtpGetToFile
���ܣ���ftp����һ���ļ������ش���
���룺hFtpClt              -ftp�û����
                remPath            -�������ļ�·��
                remFile             -�������ļ���
                locPath             -�����ļ�·��
                locFile               -�����ļ���
                time                  -�����ļ�������ʱ��,��λ(��),0��ʾû����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpGetToFile( XOS_HFTPCLT hFtpClt, XS8 *pRemFile, XS8 *pLocFile, XU32 time)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_FTPDATALINKENTRY tTskEntry;
    XS32 nLen = 0;
    
    /*������֤*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pRemFile || XNULLP == pLocFile
        || 0 ==XOS_StrCmp(pRemFile, "" ) || 0 ==XOS_StrCmp(pLocFile, "" ))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToFile()-> Bad input param !\n");
        return XERROR;
    }
    
    /*��׷�ӷ�ʽ���ļ�*/
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
    
    /*������ʽ����*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_CloseFile(&(pUserInfo->pFile));/*�쳣ʱ�ر��ļ�*/
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToFile()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*��ʼ�����ļ��������ļ�*/
    tTskEntry.curEvent = eFTPGETTOFILE;
    tTskEntry.pSockId = &(pUserInfo->dataSockId);
    tTskEntry.pFile = pUserInfo->pFile;
    tTskEntry.pSem = &(pUserInfo->semaphore);
    tTskEntry.ulDownLen = 0;
    FTP_dataTskWithTimeOut(pUserInfo, &tTskEntry, time);

    XOS_CloseFile(&(tTskEntry.pFile));

    /*�Ƚ��ļ����صĴ�С*/
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
������:    XOS_FtpGetToMem
���ܣ���ftp����һ���ļ��������ڴ�
���룺hFtpClt              -ftp�û����
                remPath            -�������ļ�·��
                remFile             -�������ļ���
                filePtr               -�����ļ�·��
                memSize          -�����ļ���
                time                  -�����ļ�������ʱ��,��λ(��),0��ʾû����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpGetToMem( XOS_HFTPCLT hFtpClt, XS8 *pRemFile, XS8 *pBuff, XU32 buffSize, XU32 time)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_FTPDATALINKENTRY tTskEntry;
    int nLen = 0;
    
    /*������֤*/
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
    /*������ʽ����*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetToMem()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*��ʼ�����ļ����ڴ�*/
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
������:    XOS_FtpPutFromFile
���ܣ������ش��̵��ļ��ϴ���ftp������
���룺hFtpClt              -ftp�û����
                locPath             -�����ļ�·��
                locFile               -�����ļ���
                remPath            -�������ļ�·��
                remFile             -�������ļ���
                time                  -�ϴ��ļ�������ʱ��,��λ(��),0��ʾû����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpPutFromFile( XOS_HFTPCLT hFtpClt, XS8 *pLocFile, XS8 *pRemFile, XU32 time)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_FTPDATALINKENTRY tTskEntry;
    int nLen = 0;
    
    /*������֤*/
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
    
    /*��ȡ������������ʱ�Զ˵�ip��port*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_CloseFile(&(pUserInfo->pFile));  /*�쳣ʱ�ر��ļ�*/
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpPutFromFile()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*��ʼ�ϴ������ļ�*/
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
        XOS_FtpDeleteFile( hFtpClt,  pRemFile);/*ɾ��ftp�������ϵ��ļ�*/
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:    XOS_FtpPutFromMem
���ܣ��������ڴ���ļ��ϴ���ftp������
���룺hFtpClt              -ftp�û����
                filePtr                -�����ڴ��ļ�ָ��
                fileSize              -�����ڴ��ļ���С
                remPath            -�������ļ�·��
                remFile             -�������ļ���
                time                  -�ϴ��ļ�������ʱ��,��λ(��),0��ʾû����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpPutFromMem( XOS_HFTPCLT hFtpClt, XS8 *pBuff, XU32 buffSize, XS8 *pRemFile, XU32 time)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    t_FTPDATALINKENTRY tTskEntry;
    int nLen = 0;
    
    /*������֤*/
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
    
    /*������ʽ����*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpPutFromMem()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*��ʼ�ϴ��ڴ��ļ�*/
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
������:    XOS_FtpCurrentWorkDir
���ܣ�ȡ�õ�ǰ����Ŀ¼
���룺hFtpClt                     -ftp�û����
�����pWorkDir                  -��ǰ����Ŀ¼
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpCurrentWorkDir( XOS_HFTPCLT hFtpClt, XVOID **pWorkDir)
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    /*������֤*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pWorkDir)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpCurrentWorkDir()-> Bad input param !\n");
        return XERROR;
    }
    /*���ص�ǰĿ¼*/
    *pWorkDir = (XS8*)(pUserInfo->curDir) + XOS_StrLen(pUserInfo->defDir);
    return XSUCC;
}

/************************************************************************
������:    XOS_FtpChangeWorkDir
���ܣ��ı䵱ǰ����Ŀ¼
���룺hFtpClt                    -ftp�û����
                pWorkDir                -�µĵ�ǰ����Ŀ¼
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpChangeWorkDir( XOS_HFTPCLT hFtpClt, XS8 *pWorkDir )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    int len = 0;
    
    /*������֤*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pWorkDir || 0 == XOS_StrCmp(pWorkDir, "") ||
        XOS_StrLen(pWorkDir)+XOS_StrLen(pUserInfo->defDir) >= MAX_DIRECTORY_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpChangeWorkDir()-> Bad input param !\n");
        return XERROR;
    }    

    
    /*���͸ı�Ŀ¼������*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "CWD ", pUserInfo->defDir, pWorkDir, eFTPCWD))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpChangeWorkDir()-> Change current work directory fail !\n");
        return XERROR;
    }
    
    /*�޸ĵ�ǰ����Ŀ¼*/    
    len = (XS32)XOS_StrLen(pUserInfo->defDir) + (XS32)XOS_StrLen(pWorkDir);
    if(len < sizeof(pUserInfo->curDir))
    {
        XOS_Sprintf(pUserInfo->curDir, sizeof(pUserInfo->curDir)-1, "%s%s", pUserInfo->defDir, pWorkDir);
        pUserInfo->curDir[len] = '\0';

        if(pUserInfo->curDir[len-1] != '/' && len+1 < sizeof(pUserInfo->curDir))/*�����win32��������������*/
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
������:    XOS_FtpGetFileSize
���ܣ����ftp��������һ���ļ��Ĵ�С
���룺hFtpClt             -ftp�û����
                pRemFile           -�������ļ�
�����N/A
���أ��ɹ��򷵻��ļ���С
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpGetFileSize( XOS_HFTPCLT hFtpClt, XS8 *pRemFile )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    /*��������*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pRemFile || 0 == XOS_StrCmp(pRemFile, ""))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetFileSize()-> Bad input param !\n");
        return XERROR;
    }
    
    /*���ͻ�ȡ�ļ���С������*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "SIZE ", pRemFile, "", eFTPSIZE))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpGetFileSize()-> Get file size fail !\n");
        return XERROR;
    }
    
    return (XS32)pUserInfo->fileSize;
}

/************************************************************************
������:    XOS_FtpMakeDir
���ܣ��ڵ�ǰ����Ŀ¼�´���һ���µ��ļ���
���룺hFtpClt                    -ftp�û����
                pNewFolder                -Ҫ�������ļ�����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpMakeDir( XOS_HFTPCLT hFtpClt, XS8 *pNewFolder )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    /*������֤*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pNewFolder ||
        0 == XOS_StrCmp(pNewFolder, "") || XOS_StrLen(pNewFolder) >= MAX_USERNAME_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpMakeDir()-> Bad input param !\n");
        return XERROR;
    }
    
    /*���ʹ���Ŀ¼������*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "MKD ", pNewFolder, "", eFTPMKD))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpMakeDir()-> Make directory fail !\n");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
������:    XOS_FtpRemoveDir
���ܣ��ڵ�ǰ����Ŀ¼��ɾ��һ���ļ���
���룺hFtpClt                    -ftp�û����
                pFolderName              -Ҫɾ�����ļ�����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpRemoveDir( XOS_HFTPCLT hFtpClt, XS8 *pFolderName )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    
    /*��������*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pFolderName ||
        0 == XOS_StrCmp(pFolderName, "") || XOS_StrLen(pFolderName) >= MAX_USERNAME_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpRemoveDir()-> Bad input param !\n");
        return XERROR;
    }
    
    pUserInfo->count = 0;/*�տ�ʼĿ¼���Ϊ0*/
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
������:    XOS_FtpRenameDir
���ܣ��ı䵱ǰ����Ŀ¼��һ���ļ��е�����
���룺hFtpClt                         -ftp�û����
                pOldFolder                      -�ɵ��ļ�����
                pNewFolder                     -�µ��ļ�����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpRenameDir( XOS_HFTPCLT hFtpClt, XS8* pOldFolder, XS8 *pNewFolder )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    int nLen = 0;
    
    /*������֤*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pOldFolder || XNULLP == pNewFolder ||
        0 == XOS_StrCmp(pOldFolder, "") || 0 == XOS_StrCmp(pNewFolder, ""))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpRenameDir()-> Bad input param !\n");
        return XERROR;
    }

    nLen = (XS32)XOS_MIN(XOS_StrLen(pNewFolder),sizeof(pUserInfo->newFileName)-1);

    XOS_MemCpy(pUserInfo->newFileName, pNewFolder, nLen);
    pUserInfo->newFileName[nLen] = '\0';
    
    /*����������Ŀ¼������*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "RNFR ", pOldFolder, "", eFTPRNCHECK))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpRenameDir()-> Rename directory fail !\n");
        return XERROR;
    }
    
    return XSUCC;
}

/************************************************************************
������:    XOS_FtpCreatFile
���ܣ��ڵ�ǰ����Ŀ¼�´���һ���µ��ļ�
���룺hFtpClt                    -ftp�û����
                pFileName                  -Ҫ�������ļ�������
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpCreatFile( XOS_HFTPCLT hFtpClt, XS8 *pFileName )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    int nLen = 0;
    
    /*������֤*/
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
    
    /*��ȡ������������ʱ�Զ˵�ip��port*/
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpCreatFile()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*��ʼ�ϴ������ļ�,����ֱ�ӹر���·,��ʾ�ϴ��ļ������*/
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
������:    XOS_FtpDeleteFile
���ܣ��ڵ�ǰ����Ŀ¼��ɾ��һ�����ڵ��ļ�
���룺hFtpClt                    -ftp�û����
                pFileName                  -��ǰ����Ŀ¼�µ�һ���ļ�������
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpDeleteFile( XOS_HFTPCLT hFtpClt, XS8 *pFileName )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    if(NULL == pFileName)
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpDeleteFile()-> Para is null!\n");
        return XERROR;
    }
    
    /*������֤*/
    if (!FTP_IsValidUserHandle(hFtpClt, (XVOID**)&pUserInfo) || XNULLP == pFileName || 0==XOS_StrCmp(pFileName, ""))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_FtpDeleteFile()-> Bad input param !\n");
        return XERROR;
    }
    
    return FTP_DeleteFile(pUserInfo, pFileName);
}

/************************************************************************
������:    XOS_FtpRenameFile
���ܣ�����ǰ����Ŀ¼�µ�һ���ļ�����
���룺hFtpClt                    -ftp�û����
                pOldFileName             -��ǰ����Ŀ¼�µ�һ���ļ�������
                pNewFileName           -�µ��ļ���
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
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
������:    XOS_FtpClose
���ܣ���ftp�������Ͽ���������,�˳���¼
���룺hFtpClt                    -ftp�û����
�����N/A
���أ��ɹ��򷵻�XSUCC
          ʧ���򷵻�XERROR
˵����
************************************************************************/
XPUBLIC XS32 XOS_FtpClose( XOS_HFTPCLT *hFtpClt )
{
    t_XOSFTPCLIENT *pUserInfo = XNULLP;
    XS32 userId = 0;
    
    /*������֤*/
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

    /*�ͷ���·*/
    FTP_LinkRelease(pUserInfo->linkHandle);
    /*��ʱ�����������·�ѽ�������Ҫ�ر�������·*/
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
 ������:XOS_ListFile
 ����:  ��ȡĿ¼�е��ļ��б�add by lyp for �Ʒ�̨
 ����: ��
 ���: ��
 ����: ��
 ˵��:
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
    
    /*��������*/
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
    pUserInfo->bufferSize = 0;/*Ŀ¼�б����ݴ�С*/
    pUserInfo->pBuffer = XNULLP;/*Ŀ¼�б�����*/
    pLocation = (XS8*)pUserInfo->curFolder;/*��ǰ�ļ��е�·��(����ڵ�ǰĿ¼)*/
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
    /*fDirɾ������ʱʹ��*/
    XOS_MemCpy(fDir, pUserInfo->curFolder, XOS_StrLen(pUserInfo->curFolder)+1);
    pUserInfo->count++;/*Ŀ¼���*/
    
    /*������ʽ����*/
    pUserInfo->replyInfoType = ePREPARE_SUC;
    if (XERROR == FTP_ComSendWithTimeOut(pUserInfo, "PASV", "", "", eFTPPASV))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "XOS_ListFile()-> PASV mode fail!\n");
        return XERROR;
    }
    
    /*��ʼ�����ļ��б�*/
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
        /*��ʼ��������ļ��б�����ݲ�����ɾ������*/
        pOffset = pData;
        while (pOffset - pData < dataLength)
        {
            switch (*pOffset)
            {
            case 'd':/*�ļ���*/
                break;
            case '-':/*�ļ�*/
                if (XERROR == FTP_ListParser(pOffset, fName))
                {
                    pOffset = pData + dataLength;
                    flag = XFALSE;
                    break;
                }
                /*���ļ����ϳ�һ�����ָ������ַ���*/
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
 ������:XOS_FtpInit
 ����:  ftp��ʼ�����������
 ����: ��
 ���: ��
 ����: ��
 ˵��:
************************************************************************/
XS8 XOS_FtpInit( XVOID *t, XVOID *v )
{

#ifdef FTP_TEST
    t_PARA timerpara;
    t_BACKPARA backpara = {0};
#endif    

        
    XOS_UNUSED(t);
    XOS_UNUSED(v);
    
    /*����������*/
    if ( XSUCC != XOS_MutexCreate(&(g_ftpMnt.contrlTblLock)))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpInit()-> create mutex lock failed!\n");
        return XERROR;
    }
    
    /* ��������·��ص���Դ*/
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
 ������:XOS_FtpInitMsgQueue
 ����:  ftp ģ����Ϣ���г�ʼ��
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS32 XOS_FtpInitMsgQueue()
{
    /*������Ϣ����*/
    gFtpMsgQueue = XOS_listConstruct(sizeof(t_XOSCOMMHEAD), gFtpMsgQueueSize, "gFtpMsgQueue");

    if(XNULL == gFtpMsgQueue)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "gFtpMsgQueue: init    table failed");
        return XERROR;
    }

    XOS_listClear(gFtpMsgQueue);
    
    XOS_listSetCompareFunc(gFtpMsgQueue, (nodeCmpFunc)NULL);/*���ñȽϺ���Ϊ��*/

    /*������Ϣ�����ٽ���*/
    if ( XSUCC != XOS_MutexCreate( &gFtpMsgQueueMutex) )
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "XOS_MutexCreate gFtpMsgQueueMutex failed!");
        return XERROR;
    }

    /*����ͬ���ź���*/
    if (XERROR == XOS_SemCreate(&(gFtpQuSemaphore), 0))
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "XOS_MutexCreate XOS_SemCreate failed!");
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
 ������:XOS_FtpInitReMsgQueue
 ����:  ftp ģ���ط���Ϣ���г�ʼ��
 ����:  
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS32 XOS_FtpInitReMsgQueue()
{
    /*������Ϣ����*/
    gFtpReMsgQueue = XOS_listConstruct(sizeof(t_ReFtpMsg), gFtpReMsgQueueSize, "gFtpMsgQueue");

    if(XNULL == gFtpReMsgQueue)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "gFtpReMsgQueue: init    table failed");
        return XERROR;
    }

    XOS_listClear(gFtpReMsgQueue);
    
    XOS_listSetCompareFunc(gFtpReMsgQueue, (nodeCmpFunc)NULL);/*���ñȽϺ���Ϊ��*/

    /*������Ϣ�����ٽ���*/
    if ( XSUCC != XOS_MutexCreate( &gFtpReMsgQueueMutex) )
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "XOS_MutexCreate gFtpReMsgQueueMutex failed!");
        return XERROR;
    }

    /*����ͬ���ź���*/
    if (XERROR == XOS_SemCreate(&(gFtpReQuSemaphore), 0))
    {
        XOS_Trace(MD(FID_FTP, PL_EXP), "XOS_MutexCreate XOS_SemCreate failed!");
        return XERROR;
    }

    return XSUCC;
}


/************************************************************************
 ������:XOS_FtpAddMsgQueue
 ����:  ����ָ�����Ӷ����е�Ԫ��
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*������*/
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
    else /*������ӽڵ�ɹ����򿪱�messageָ������*/
    {
        /*��ȡ�ڵ�*/
        pHead = (t_XOSCOMMHEAD*)XOS_listGetElem(gFtpMsgQueue, pos);
        /*�����ڴ�,��������Ϣ*/
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
 ������:XOS_FtpAddReMsgQueue
 ����:  ����ָ�������ط������е�Ԫ��
 ����:  pMsg �ط���Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS32 XOS_FtpAddReMsgQueue(t_ReFtpMsg *pMsg)
{
    XS32 pos = 0;
    if(XNULL == pMsg)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpAddMsgQueue: pMsg is NULL");
        return FTP_QUE_NULL;
    }

    /*������*/
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
 ������:XOS_FtpClearMsgQueue
 ����:  ����ָ��ɾ�������е�Ԫ��
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:XOS_FtpClearReMsgQueue
 ����:  ����ָ��ɾ�������е�Ԫ��
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:XOS_FtpPutMsg
 ����:  ����Ϣ���뵽ѭ������
 ����:  pMsg ����Ϣָ��
 ���:
 ����: �ɹ�����FTP_QUE_SUC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
int XOS_FtpPutMsg(t_XOSCOMMHEAD *pMsg) 
{ 
    int status = 0; 
    XS32 head_pos = -1;
    t_XOSCOMMHEAD *pHeadMsg = XNULL;

    XOS_MutexLock(&gFtpMsgQueueMutex);

    status = XOS_FtpAddMsgQueue(pMsg);

    if(FTP_QUE_FULL == status) /*����������ɾ����һ��������ѭ������*/
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
        
        XOS_FtpClearMsgMem(pHeadMsg);/*ɾ����Ϣ����ڴ�*/
        XOS_FtpClearMsgQueue(pHeadMsg);/*ɾ��*/

        /*�ٴ�����*/
        status = XOS_FtpAddMsgQueue(pMsg);
        
    }    

    XOS_SemPut(&gFtpQuSemaphore); /*�ҳ��źŵ�*/
    XOS_MutexUnlock(&gFtpMsgQueueMutex);

    return status; 
} 

/************************************************************************
 ������:XOS_FtpPutReMsg
 ����:  ����Ϣ���뵽ѭ������
 ����:  pMsg ����Ϣָ��
 ���:
 ����: �ɹ�����FTP_QUE_SUC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
int XOS_FtpPutReMsg(t_ReFtpMsg *pMsg) 
{ 
    int status = 0; 
    XS32 head_pos = -1;
    t_ReFtpMsg *pHeadMsg = XNULL;

    XOS_MutexLock(&gFtpReMsgQueueMutex);

    status = XOS_FtpAddReMsgQueue(pMsg);

    if(FTP_QUE_FULL == status) /*����������ɾ����һ��������ѭ������*/
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
        
        XOS_FtpClearMsgMem(&pHeadMsg->reMsg);/*ɾ����Ϣ����ڴ�*/
        XOS_FtpClearReMsgQueue(pHeadMsg);/*ɾ��*/

        /*�ٴ�����*/
        status = XOS_FtpAddReMsgQueue(pMsg);
        
    }    

    XOS_SemPut(&gFtpReQuSemaphore); /*�ҳ��źŵ�*/
    XOS_MutexUnlock(&gFtpReMsgQueueMutex);

    return status; 
} 


/************************************************************************
 ������:XOS_FtpGetMsg
 ����:  ��ѭ�����л�ȡ��Ϣ
 ����:  pMsg ����Ϣָ��
 ���:
 ����: �ɹ�����FTP_QUE_SUC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
t_XOSCOMMHEAD XOS_FtpGetMsg() 
{ 
    t_XOSCOMMHEAD m_Msg; 
    XS32 head_pos = -1;
    XS8 blockFlag = 0;
    t_XOSCOMMHEAD *pHeadMsg = XNULL;
    m_Msg.msgID = XERROR;

    XOS_MutexLock(&gFtpMsgQueueMutex);

    /*û������������*/
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
        /*�ȴ��ź���*/
        while (1) /*���źŻ����������ȡ*/ 
        {   
            XOS_SemGet(&gFtpQuSemaphore);

            /*���»����*/
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
    /*ɾ��*/    
    m_Msg = *pHeadMsg; 
    XOS_FtpClearMsgQueue(pHeadMsg);   

    XOS_MutexUnlock(&gFtpMsgQueueMutex);

    return m_Msg; 
} 

/************************************************************************
 ������:XOS_FtpGetReMsg
 ����:  ���ط����л�ȡ��Ϣ
 ����:  pMsg ����Ϣָ��
 ���:
 ����: �ɹ�����FTP_QUE_SUC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
t_ReFtpMsg XOS_FtpGetReMsg() 
{ 
    t_ReFtpMsg m_Msg; 
    XS32 head_pos = -1;
    XS8 blockFlag = 0;
    t_ReFtpMsg *pHeadMsg = XNULL;
    m_Msg.reMsg.msgID = XERROR;

    XOS_MutexLock(&gFtpReMsgQueueMutex);

    /*û������������*/
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
        /*�ȴ��ź���*/
        while (1) /*���źŻ����������ȡ*/ 
        {   
            XOS_SemGet(&gFtpReQuSemaphore);

            /*���»����*/
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
    /*ɾ��*/    
    m_Msg = *pHeadMsg; 
    XOS_FtpClearReMsgQueue(pHeadMsg);   

    XOS_MutexUnlock(&gFtpReMsgQueueMutex);

    return m_Msg; 
} 

/************************************************************************
 ������:XOS_FtpInitMsgDealTaskPool
 ����:  ��ʼ�������
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*�ط�����*/
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
 ������:FTP_CliMsgDealTask
 ����:  ��Ϣ����������
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: 
 ˵��:
************************************************************************/
XSTATIC XVOID* FTP_CliMsgDealTask(void* paras)
{
    t_XOSCOMMHEAD m_Msg;

    while(1)
    {
        m_Msg.msgID = eFTP_NULL;
        
        m_Msg = XOS_FtpGetMsg();/*��ȡ����*/
        
        switch (m_Msg.msgID)
         {
             case eFTP_GETTOFILE:/*�����ļ��������ļ�*/
                 FTP_GetFileMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_GETTOMEM:/*�����ļ����ڴ�*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMFILE:/*�ϴ����ش����ļ�*/
                 FTP_PutFileMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMMEM:/*�ϴ������ڴ��ļ�*/
                 FTP_PutMemMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_LIST:/*��ȡָ��Ŀ¼�µ��ļ��б�*/
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
 ������:FTP_CliMsgDealTask
 ����:  �ط���Ϣ����������
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: 
 ˵��:
************************************************************************/
XSTATIC XVOID* FTP_CliMsgReDealTask(void* paras)
{
    t_ReFtpMsg m_Msg;

    while(1)
    {
        m_Msg.reMsg.msgID = eFTP_NULL;
        
        m_Msg = XOS_FtpGetReMsg();/*��ȡ�ط�����*/
        
        switch (m_Msg.reMsg.msgID)
         {
             case eFTP_GETTOFILE:/*�����ļ��������ļ�*/
                 FTP_GetFileReMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_GETTOMEM:/*�����ļ����ڴ�*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMFILE:/*�ϴ����ش����ļ�*/
                 FTP_PutFileReMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMMEM:/*�ϴ������ڴ��ļ�*/
                 FTP_PutMemReMsgProTskEntry(&m_Msg);
                 break;
                 
             case eFTP_LIST:/*��ȡָ��Ŀ¼�µ��ļ��б�*/
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
 ������:XOS_FtpClearMsgMem
 ����:  �����Ϣ�е��ڴ�
 ����:  paras ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
void XOS_FtpClearMsgMem(t_XOSCOMMHEAD* paras)
{
    if(NULL != paras)
    {
        switch (paras->msgID)
        {
             case eFTP_GETTOFILE:/*�����ļ��������ļ�*/
                 FTP_FreeGETTOFILE_REQ(paras);
                 break;
                 
             case eFTP_GETTOMEM:/*�����ļ����ڴ�*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMFILE:/*�ϴ����ش����ļ�*/
                 FTP_FreePUTFROMFILE_REQ(paras);
                 break;
                 
             case eFTP_PUTFROMMEM:/*�ϴ������ڴ��ļ�*/
                 FTP_FreePUTFROMMEM_REQ(paras);
                 break;
                 
             case eFTP_LIST:/*��ȡָ��Ŀ¼�µ��ļ��б�*/
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
 ������:XOS_FtpClearXosMsgMem
 ����:  �����Ϣ�е��ڴ�,�ڲ��뵽��Ϣ���г����ǵ���
 ����:  paras ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
void XOS_FtpClearXosMsgMem(t_XOSCOMMHEAD* paras)
{
    if(NULL != paras)
    {
        switch (paras->msgID)
        {
             case eFTP_GETTOFILE:/*�����ļ��������ļ�*/
                 FTP_FreeXosGETTOFILE_REQ(paras);
                 break;
                 
             case eFTP_GETTOMEM:/*�����ļ����ڴ�*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;
                 
             case eFTP_PUTFROMFILE:/*�ϴ����ش����ļ�*/
                 FTP_FreeXosPUTFROMFILE_REQ(paras);
                 break;
                 
             case eFTP_PUTFROMMEM:/*�ϴ������ڴ��ļ�*/
                 FTP_FreeXosPUTFROMMEM_REQ(paras);
                 break;
                 
             case eFTP_LIST:/*��ȡָ��Ŀ¼�µ��ļ��б�*/
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
 ������:XOS_FtpMallocMsgMem
 ����:  ����message�ڴ�
 ����:  paras ��Ϣָ��
 ���:
 ����: �ɹ����ص�ַ , ʧ�ܷ���NULL
 ˵��:
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
         case eFTP_PUTFROMFILE:/*�ϴ����ش����ļ�*/
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
             
         case eFTP_PUTFROMMEM:/*�ϴ������ڴ��ļ�*/
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
        case eFTP_GETTOFILE:/*�����ļ��������ļ�*/
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
 ������:XOS_FtpFreeMsgMem
 ����:  �ͷ���Ϣ�����е�messageָ��
 ����:  paras ��Ϣָ��
 ���:
 ����: �ɹ����ص�ַ , ʧ�ܷ���NULL
 ˵��:
************************************************************************/
void XOS_FtpFreeMsgMem(int msgId, void *pBuffer)
{
    if(NULL != pBuffer)
    {
        switch (msgId)
        {
             case eFTP_GETTOFILE:/*�����ļ��������ļ�*/
                 //FTP_PutMemMsgPro(&m_Msg);
                 break;                 
             case eFTP_GETTOMEM:/*�����ļ����ڴ�*/
                 //FTP_RecvStartAck(&m_Msg);
                 break;                 
             case eFTP_PUTFROMFILE:/*�ϴ����ش����ļ�*/
                 XOS_MemFree(FID_FTP, pBuffer);        
                 break;
             case eFTP_PUTFROMFILEAck:
                  break;                 
             case eFTP_PUTFROMMEM:/*�ϴ������ڴ��ļ�*/
                 XOS_MemFree(FID_FTP, pBuffer);                      
                 break;
             case eFTP_PUTFROMMEMAck:
                  break;
             case eFTP_LIST:/*��ȡָ��Ŀ¼�µ��ļ��б�*/
                    break;
             default:
                break;
        }
    }

    return;
}

/************************************************************************
 ������:FTP_TimerProc
 ����:  ftp ģ�鶨ʱ����Ϣ���������
 ����:  pBackPara ����ʱ��ָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*��ȡ��ǰ�û�*/
    pUserInfo = (t_XOSFTPCLIENT*)(XPOINT)(pBackPara->para1);
    if ((t_XOSFTPCLIENT*)XNULLP == pUserInfo)
    {
        return XERROR;
    }

    /*�����û������е���Դ����*/
    XOS_MutexLock(&(g_ftpMnt.contrlTblLock));

    /*�ָ���ʱ��*/
    XOS_INIT_THDLE(pUserInfo->timerId);
    if (XSUCC != pUserInfo->ret || eFTPSTATELOGIN != pUserInfo->curState)
    {
        pUserInfo->ret = XERROR;
    }
    else
    {
        pUserInfo->ret = XSUCC;
    }

    
    /*��ʾ��ʱ�����ڵ��¼�״̬*/
    XOS_Trace(MD(FID_FTP, PL_WARN), "time out FTP_timerProc()-> semaphore put !,curEvent is %d\n", pUserInfo->curEvent);

    /*��¼�ɹ��������ϴ��ļ�����ʱ,�����ļ�̫�󣬿������ٶ���*/
    if (pUserInfo->curState == eFTPSTATELOGIN && (eFTPPUTFROMMEM == pUserInfo->curEvent || eFTPPUTFROMFILE == pUserInfo->curEvent))
    {
        XINET_CloseSock(&(pUserInfo->dataSockId));
        pUserInfo->dataSockId.fd = XERROR;
        pUserInfo->retOfPut = XERROR;

        /*�ͷ�����ͨ���ź����������ϴ���������Զ����,ԭ��:���ͨ����·�������⣬û����Ӧ����ֻ�ܵȴ���ʱ�˳�*/
        XOS_SemPut(&(pUserInfo->semaphore));
        XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
        return XSUCC;
    }

    /*��¼�ɹ������������ļ���Ŀ¼ ����ʱ�������ļ�̫�󣬿������ٶ���*/
    if (pUserInfo->curState == eFTPSTATELOGIN && 
        (eFTPGETTOMEM == pUserInfo->curEvent || 
         eFTPLIST == pUserInfo->curEvent || 
         eFTPGETTOFILE == pUserInfo->curEvent))
    {
        XINET_CloseSock(&(pUserInfo->dataSockId));
        pUserInfo->dataSockId.fd = XERROR;
        
        /*�ͷ�����ͨ���ź������������غ�������Զ����*/
        XOS_SemPut(&(pUserInfo->semaphore));
        XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
        return XSUCC;
    }
    
    pUserInfo->curEvent = eFTPNONE;


    /*�ͷſ���ͨ���ź���*/
    XOS_SemPut(&(pUserInfo->semaphore));

    /*�ͷ��û�������*/
    XOS_MutexUnlock(&(g_ftpMnt.contrlTblLock));
    return XSUCC;
}

/************************************************************************
 ������:FTP_NtlMsgPro
 ����:  ftp ģ��ntl��Ϣ����
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
     case eInitAck:/*��·��ʼ��Ӧ��*/
         FTP_RecvInitAck(pMsg);
         break;
         
     case eStartAck:/*��·����Ӧ��*/
         FTP_RecvStartAck(pMsg);
         break;
         
     case eDataInd:/*�յ�����ָʾ*/
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
 ������:FTP_PutMemMsgProTskEntry
 ����:  ���ڴ��ϴ�
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_PutMemMsgProTskEntry(t_XOSCOMMHEAD* pMsg)
{
    t_PUTFROMMEM_REQ *pReq = XNULL;
    XS8 usRst = eFTP_SUCC;
    if(pMsg)
    {
        usRst = FTP_PutMemMsgDo(pMsg);
        if(eFTP_SUCC != usRst)/*��һ�η���ʧ��*/
        {
            t_ReFtpMsg reSendMsg;
            reSendMsg.nSndNum = 0; /*�ط�����*/
            reSendMsg.reMsg = *pMsg;
            XOS_Trace(MD(FID_FTP, PL_WARN), "Waiting resend ftp file\n");
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(&reSendMsg))/*�����ط���Ϣ����*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreePUTFROMMEM_REQ(pMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*���������Ҫ�ͷ���Ϣ*/
        }
        else/*�ɹ�*/
        {   
            pReq = (t_PUTFROMMEM_REQ*)pMsg->message;
            if(pReq)
            {
                /*��ͻ�ģ�鷢��Ϣ*/
                FTP_SendMsgToCli(pMsg->datasrc.FID, usRst, pReq->ulSerial);
                /*�ͷ���Ϣ�ڴ�*/
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
 ������:FTP_PutMemReMsgProTskEntry
 ����:  ���ڴ��ϴ�(�ش�)
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_PutMemReMsgProTskEntry(t_ReFtpMsg* pMsg)
{
    t_PUTFROMMEM_REQ *pReq = XNULL;
    
    XS8 usRst = eFTP_SUCC;
    
    if(pMsg)
    {
        usRst = FTP_PutMemMsgDo(&pMsg->reMsg);
        if(eFTP_SUCC != usRst)/*ʧ��*/
        {
            pMsg->nSndNum ++; /*�ط�����*/
            
            if(pMsg->nSndNum >=3 && NO_SEND_ELAPSES) /*���ʹ�����ʱ*/
            {
                pReq = (t_PUTFROMMEM_REQ*)pMsg->reMsg.message;
                if(pReq)
                {
                    /*��ͻ�ģ�鷢��Ϣ*/
                    FTP_SendMsgToCli(pMsg->reMsg.datasrc.FID, usRst, pReq->ulSerial);
                    /*�ͷ���Ϣ�ڴ�*/
                    FTP_FreePUTFROMMEM_REQ(&pMsg->reMsg);

                    XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_PutMemReMsgProTskEntry failed!\n");
                    return XSUCC; 
                }
                return XERROR;
            }
            
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(pMsg))/*�����ط���Ϣ����*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreePUTFROMMEM_REQ(&pMsg->reMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*���������Ҫ�ͷ���Ϣ*/
        }
        else/*�ɹ�*/
        {   
            pReq = (t_PUTFROMMEM_REQ*)pMsg->reMsg.message;
            if(pReq)
            {
                /*��ͻ�ģ�鷢��Ϣ*/
                FTP_SendMsgToCli(pMsg->reMsg.datasrc.FID, usRst, pReq->ulSerial);
                /*�ͷ���Ϣ�ڴ�*/
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
 ������:FTP_PutMemMsgDo
 ����:  ftp ģ��ntl��Ϣ����
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*ȡ��Ϣ�е�����*/
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

    /*�û��������볤�Ȳ��Ϸ�*/
    if((pReq->ulUserLen > MAX_USERNAME_LEN) || pReq->ulPassLen> MAX_PASSWORD_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "pReq->ulUserLen > MAX_USERNAME_LEN  || pReq->ulPassLen> MAX_PASSWORD_LEN!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }
    
    /*��¼������*/
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

    /*��ȡ�ļ���С����ʾ�ļ����ڣ����ȶ���������*/
    if (XSUCC <= XOS_FtpGetFileSize(hFtpClt, FullFileName))
    {
        //XOS_Trace(MD(FID_FTP, PL_INFO), "FTP_PutMemMsgDo The Ttp File Name Exist !");

        XOS_MemCpy(ucNewname, FullFileName, XOS_MIN((XOS_StrLen(FullFileName)-1), sizeof(ucNewname)-2));
        XOS_StrNCat(ucNewname, "b", XOS_StrLen("b"));  /*end of '\0'*/

        /*������ԭ�ļ�*/
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

/*�ϴ��ļ�*/
    /*Ŀǰ��֧�ֶϵ�����������ļ���С���ܳ���300��*/
    if (XSUCC != XOS_FtpPutFromMem(hFtpClt, (XS8*)pReq->ucpBuffer, pReq->ulBufferLen, FullFileName, MAX_FTPTRANS_TIME))
    {
        XOS_Trace(MD(FID_FTP, PL_INFO), "XOS_FtpPutFromMem failed !");
        XOS_FtpClose(&hFtpClt);
        usRst = eFTP_TRAN_FAILED;
        goto error;
    }

    /*�ͷ�ftp*/
    XOS_FtpClose(&hFtpClt);

error:

    return usRst;    
}


/************************************************************************
 ������:FTP_GetFileMsgProTskEntry
 ����: �����ļ��������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
           (pReq->ulMaxReDown > 0))/*��һ�η���ʧ��*/
        {
            t_ReFtpMsg reSendMsg;
            reSendMsg.nSndNum = 0; /*�ط�����*/
            reSendMsg.reMsg = *pMsg;            
            XOS_Trace(MD(FID_FTP, PL_WARN), "Waiting redownload ftp file\n");
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(&reSendMsg))/*�����ط���Ϣ����*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreeGETTOFILE_REQ(pMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*���������Ҫ�ͷ���Ϣ*/
        }
        else/*�ɹ�����������*/
        {   
            /*��ͻ�ģ�鷢��Ϣ*/
            FTP_Send_GettoFile_Ack_MsgToCli(pMsg->datasrc.FID, pMsg->datasrc.FsmId,usRst, pReq->ulSerial, ulFileSize);
            /*�ͷ���Ϣ�ڴ�*/
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
 ������:FTP_PutFileMsgProTskEntry
 ����: �ϴ������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_PutFileMsgProTskEntry(t_XOSCOMMHEAD* pMsg)
{
    t_PUTFROMFILE_REQ *pReq = XNULL;
    XS8 usRst = eFTP_SUCC;
    if(pMsg)
    {
        usRst = FTP_PutFileMsgDo(pMsg);
        if(eFTP_SUCC != usRst)/*��һ�η���ʧ��*/
        {
            t_ReFtpMsg reSendMsg;
            reSendMsg.nSndNum = 0; /*�ط�����*/
            reSendMsg.reMsg = *pMsg;
            XOS_Trace(MD(FID_FTP, PL_WARN), "Waiting resend ftp file\n");
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(&reSendMsg))/*�����ط���Ϣ����*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreePUTFROMFILE_REQ(pMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*���������Ҫ�ͷ���Ϣ*/
        }
        else/*�ɹ�*/
        {   
            pReq = (t_PUTFROMFILE_REQ*)pMsg->message;
            if(pReq)
            {
                /*��ͻ�ģ�鷢��Ϣ*/
                FTP_SendMsgToCli(pMsg->datasrc.FID, usRst, pReq->ulSerial);
                /*�ͷ���Ϣ�ڴ�*/
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
 ������:FTP_GetFileReMsgProTskEntry
 ����: �����ļ��������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_GetFileReMsgProTskEntry(t_ReFtpMsg* pMsg)
{    
    t_GETTOFILE_REQ *pReq = XNULL;    
    XS8 usRst = eFTP_SUCC;
    XU32 ulFileSize = 0;
    
    if(pMsg)
    {
        usRst = FTP_GetFileMsgDo(&pMsg->reMsg, &ulFileSize);
        if(eFTP_SUCC != usRst)/*ʧ��*/
        {
            pMsg->nSndNum ++; /*�ط�����*/
            pReq = (t_GETTOFILE_REQ*)pMsg->reMsg.message;
            if(pReq)
            {                
                if(pMsg->nSndNum >= pReq->ulMaxReDown) /*���ʹ�����ʱ*/
                {    
                    XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_GetFileReMsgProTskEntry download  %s time out!\n", pReq->ucpSaveFilePath);

                    /*��ͻ�ģ�鷢��Ϣ*/
                    FTP_Send_GettoFile_Ack_MsgToCli(pMsg->reMsg.datasrc.FID, pMsg->reMsg.datasrc.FsmId,usRst, pReq->ulSerial, ulFileSize);
                    /*�ͷ���Ϣ�ڴ�*/
                    FTP_FreeGETTOFILE_REQ(&pMsg->reMsg);
                    return XSUCC;                    
                }
            }
            
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(pMsg))/*�����ط���Ϣ����*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreeGETTOFILE_REQ(&pMsg->reMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*���������Ҫ�ͷ���Ϣ*/
        }
        else/*�ɹ�*/
        {   
            pReq = (t_GETTOFILE_REQ*)pMsg->reMsg.message;
            if(pReq)
            {
                /*��ͻ�ģ�鷢��Ϣ*/
                FTP_Send_GettoFile_Ack_MsgToCli(pMsg->reMsg.datasrc.FID, pMsg->reMsg.datasrc.FsmId,usRst, pReq->ulSerial, ulFileSize);
                /*�ͷ���Ϣ�ڴ�*/
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
 ������:FTP_PutFileReMsgProTskEntry
 ����: �ϴ������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_PutFileReMsgProTskEntry(t_ReFtpMsg* pMsg)
{    
    t_PUTFROMFILE_REQ *pReq = XNULL;
    
    XS8 usRst = eFTP_SUCC;
    
    if(pMsg)
    {
        usRst = FTP_PutFileMsgDo(&pMsg->reMsg);
        if(eFTP_SUCC != usRst)/*ʧ��*/
        {
            pMsg->nSndNum ++; /*�ط�����*/
            
            if(pMsg->nSndNum >=3 && NO_SEND_ELAPSES) /*���ʹ�����ʱ*/
            {
                pReq = (t_PUTFROMFILE_REQ*)pMsg->reMsg.message;
                if(pReq)
                {
                    /*��ͻ�ģ�鷢��Ϣ*/
                    FTP_SendMsgToCli(pMsg->reMsg.datasrc.FID, usRst, pReq->ulSerial);
                    /*�ͷ���Ϣ�ڴ�*/
                    FTP_FreePUTFROMFILE_REQ(&pMsg->reMsg);
                    return XSUCC;
                }
                return XERROR;
            }
            
            if(FTP_QUE_SUC != XOS_FtpPutReMsg(pMsg))/*�����ط���Ϣ����*/
             {
                 XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutReMsg failed!\n");
                 FTP_FreePUTFROMFILE_REQ(&pMsg->reMsg);
                 return XERROR;
             }
            else
             {                 
                 return XSUCC;
             } /*���������Ҫ�ͷ���Ϣ*/
        }
        else/*�ɹ�*/
        {   
            pReq = (t_PUTFROMFILE_REQ*)pMsg->reMsg.message;
            if(pReq)
            {
                /*��ͻ�ģ�鷢��Ϣ*/
                FTP_SendMsgToCli(pMsg->reMsg.datasrc.FID, usRst, pReq->ulSerial);
                /*�ͷ���Ϣ�ڴ�*/
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
 ������:FTP_Snprintf_Md5String
 ����: ��md5ת��Ϊ�ַ���
 ����:  pszMd5 ��Ҫת�����ַ���
        nMd5Len  �ַ�������
        pszOut   ת������ַ���
        nOutLen  ת������ַ�������
 ���:
 ����: 
 ˵��:
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
 ������:FTP_GetFileMsgDo
 ����: �����ļ��������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*ȡ��Ϣ�е�����*/
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
    
    /*�û��������볤�Ȳ��Ϸ�*/
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

    /*�����ļ�����,��ɾ��*/
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
        
    /*��¼������*/
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
    
    /*��ȡ�ļ���С����ʾ�ļ�����*/
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
    
    /*�����ļ�*/
    if( XSUCC != XOS_FtpGetToFile(hFtpClt, FullFileName, saveFilename, pReq->ulTime))
    {
        XOS_Trace(MD(FID_FTP, PL_INFO), "XOS_FtpGetToFile failed !");
        XOS_FtpClose(&hFtpClt);
        usRst = eFTP_TRAN_FAILED;
        goto error;
    }

    /*�رշ���������*/
    XOS_FtpClose (&hFtpClt );

    /*md5У��*/
    if(usRst == eFTP_SUCC && XNULL != pReq->ucpMd5)
    {
        /*��ȡ�ļ���md5ֵ*/
        if(XSUCC != XOS_MDcheckfile((char*)pReq->ucpSaveFilePath, (unsigned char*)cMd5Buf, sizeof(cMd5Buf)))
        {
            usRst = eFTP_GENERATE_MD5_ERROR;
            XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpGetToFile XOS_MDcheckfile failed !");
        }
        else
        {
            FTP_Snprintf_Md5String(cMd5Buf, (int)strlen(cMd5Buf), cMd5String, sizeof(cMd5String));
            /*ת���ַ���ΪСд*/
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
 ������:FTP_CoverLowerChar
 ����: ���ַ����еĴ�д��ĸת��ΪСд��ĸ
 ����:  pszIn ��Ҫת�����ַ���
        nLen  �ַ�������
 ���:
 ����: 
 ˵��:
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
 ������:FTP_PutFileMsgDo
 ����: �ϴ������ļ���������Ϣ
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

    /*ȡ��Ϣ�е�����*/
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
    
    /*�û��������볤�Ȳ��Ϸ�*/
    if((pReq->ulUserLen > MAX_USERNAME_LEN) || pReq->ulPassLen> MAX_PASSWORD_LEN)
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "pReq->ulUserLen > MAX_USERNAME_LEN  || pReq->ulPassLen> MAX_PASSWORD_LEN!");
        usRst = eFTP_PARA_INVALID;
        goto error;
    }

    /*�����ļ��Ƿ�ɶ�*/
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
        
    /*��¼������*/
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
    
    /*��ȡ�ļ���С����ʾ�ļ����ڣ����ȶ���������*/
    if (XSUCC <= XOS_FtpGetFileSize(hFtpClt, FullFileName))
    {
        XOS_Trace(MD(FID_FTP, PL_WARN), "The Ttp File Name Exist !");

        XOS_MemCpy(ucNewname, FullFileName, XOS_MIN((XOS_StrLen(FullFileName)-1), sizeof(ucNewname)-2));
        XOS_StrNCat(ucNewname, "b", XOS_StrLen("b"));  /*end of '\0'*/

        /*������ԭ�ļ�*/
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
    
    /*�ϴ��ļ�*/
    if( XSUCC != XOS_FtpPutFromFile ( hFtpClt,uploadFilename,FullFileName,0))
    {
        XOS_Trace(MD(FID_FTP, PL_INFO), "XOS_FtpPutFromMem failed !");
        XOS_FtpClose(&hFtpClt);
        usRst = eFTP_TRAN_FAILED;
        goto error;
    }

    /*�رշ���������*/
    XOS_FtpClose (&hFtpClt );
    
error:

    return usRst;    

}

/************************************************************************
 ������:FTP_Send_GettoFile_Ack_MsgToCli
 ����:  ������Ӧ��Ϣ���ͻ���
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

   /*��������ʧ�ܣ� �������ͷ���Ϣ�ڴ�*/
  if(XERROR == XOS_MsgSend(pCpsHead))
  {
      XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
  }
  
  return XSUCC;
}

/************************************************************************
 ������:FTP_SendMsgToCli
 ����:  ������Ӧ��Ϣ���ͻ���
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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

   /*��������ʧ�ܣ� �������ͷ���Ϣ�ڴ�*/
  if(XERROR == XOS_MsgSend(pCpsHead))
  {
      XOS_MsgMemFree(pCpsHead->datasrc.FID, pCpsHead);
  }
  
  return XSUCC;
}




/************************************************************************
 ������:FTP_FreePUTFROMMEM_REQ
 ����:  �ͷŴ��ڴ��ϴ��ļ���������Ϣ
 ����:  pReq ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:FTP_FreeXosPUTFROMMEM_REQ
 ����:  �ͷŴ��ڴ��ϴ��ļ���������Ϣ
 ����:  pReq ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:FTP_FreeGETTOFILE_REQ
 ����:  �ͷŴӷ����������ļ����ļ���������Ϣ
 ����:  pReq ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:FTP_FreePUTFROMFILE_REQ
 ����:  �ͷŴӱ����ļ��ϴ��ļ���������Ϣ
 ����:  pReq ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:FTP_FreeXosPUTFROMFILE_REQ
 ����:  �ͷŴӷ����������ļ����ļ�������Ϣ
 ����:  pReq ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:FTP_FreeXosPUTFROMFILE_REQ
 ����:  �ͷŴӱ����ļ��ϴ��ļ���������Ϣ
 ����:  pReq ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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
 ������:FTP_ClientMsgPro
 ����:  ftp ģ��ͻ�������Ϣ����
 ����:  pMsg ��Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
************************************************************************/
XS8 FTP_ClientMsgPro(t_XOSCOMMHEAD* pMsg)
{
     if ((pMsg == (XVOID*)XNULLP) || ((t_XOSCOMMHEAD*)pMsg)->message == XNULLP)
     {
         XOS_Trace(MD(FID_FTP, PL_ERR), "FTP_msgProc()->Bad input param !\n");
         return XERROR;
     }

     //XOS_FtpClearXosMsgMem(pMsg); /*ֱ���ͷ���Ϣ�е��ڴ�*/
             
     //return XERROR;

     
     /*��ָ�����͵���Ϣ���뵽����*/
     if(eFTP_PUTFROMFILE == pMsg->msgID || eFTP_PUTFROMMEM == pMsg->msgID || eFTP_GETTOFILE == pMsg->msgID)
     {        
         if(FTP_QUE_SUC != XOS_FtpPutMsg(pMsg))/*������Ϣ����*/
         {
             XOS_Trace(MD(FID_FTP, PL_ERR), "XOS_FtpPutMsg failed!\n");
             XOS_FtpClearXosMsgMem(pMsg); /*�ͷ���Ϣ�е��ڴ�*/

             return XERROR;
         }    
     }

     /*��������Ϣ������*/
     if(eFTP_GETTOFILEAck == pMsg->msgID)
     {
        XOS_Trace(MD(FID_FTP, PL_ERR), "recv msgid %d\n", pMsg->msgID);
     }
     
     return XSUCC;

}

/************************************************************************
 ������:FTP_MsgProc
 ����:  ftp ģ����Ϣ���������
 ����:  pMsg ����Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��: ����Ϣ��������ntl��������Ϣ��ڣ�edataSend
              ��Ϣ���ڴ˺�������ķ�Χ��
************************************************************************/
XS8 FTP_MsgProc( XVOID* pMsgP, XVOID* sb)
{

     XS8 Res = 0 ;  /* ���ر�־*/

    t_XOSCOMMHEAD* pMsg = (t_XOSCOMMHEAD*)pMsgP;
    
    /*�ж���Ϣ��Դ*/
    switch ( pMsg->datasrc.FID )
    {
        case FID_NTL:      /* ����ƽ̨ NTL ��Ϣ*/
            Res = FTP_NtlMsgPro(pMsg) ;
            break;
        default: Res = FTP_ClientMsgPro(pMsg);/*����ͻ��˵���FTP������Ϣ*/
                 break;
    }
    
    return Res;

            
 
}
/************************************************************************
 ������:FTP_MsgProc
 ����:  ftp ģ����Ϣ���������
 ����:  pMsg ����Ϣָ��
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��: ����Ϣ��������ntl��������Ϣ��ڣ�edataSend
              ��Ϣ���ڴ˺�������ķ�Χ��
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
������: FTP_CliInit()
����:  �����г�ʼ��
����:
���:
����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 FTP_CliInit(XVOID)
{
   XS32 promptID = 0;

   promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "ftp", "Ftp module", "");

   /*��ʾftp ��Ϣ���д�С*/
   if (0 > XOS_RegistCommand(promptID, FtpCliShowMsgSize, "showMsgSize", "show ftp msg size", "�޲���"))
   {
       XOS_Trace(MD(FID_FTP, PL_ERR), "Failed to register command"  "XOS>ftp>showMsgSize!");
       return XERROR;
   }

   return XSUCC;
}

/************************************************************************
 ������: FtpCliShowMsgSize()
 ����:    show ftp msg size
 ����:
 ���:
 ����: �ɹ�����XSUCC , ʧ�ܷ���XERROR
 ˵��:
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


