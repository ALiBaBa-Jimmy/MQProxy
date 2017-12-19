/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xossctp.h
**
**  description: sctp defination
**
**  author: liukai
**
**  date:   2013.9.9
**
**************************************************************/
#ifndef _XOS_SCTP_H_
#define _XOS_SCTP_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

#if defined(XOS_SCTP) && defined(XOS_LINUX)

/*-------------------------------------------------------------
                  ����ͷ�ļ�
--------------------------------------------------------------*/
#include "xostype.h"
#include "xostl.h"
#include "xosinet.h"
#include "xosmodule.h"
#include "xosarray.h"
#include "xosntl.h"

/*-------------------------------------------------------------
                  �궨��
--------------------------------------------------------------*/
#define DEFAULT_SCTP_STREAM          1       /*sctp���ӵ�Ĭ����С��������*/
#define SCTP_MAX_STREAM      10       /*sctp���ӵ�����������*/
#define SCTP_MIN_HB_INTERVAL          100       /*sctp�������С�����������λ����*/
#define SCTP_DEFAULT_HB_INTERVAL     2000    /*sctpĬ���������ʱ�䣬2000ms*/

/*��ʱ����ʱ��*/
#ifndef SCTP_CLI_RECONNECT_INTERVAL
#define SCTP_CLI_RECONNECT_INTERVAL   4000 
#endif

/*��ʱ�����Ĵ���*/
#ifndef SCTP_CLI_RECONNEC_TIMES
#define SCTP_CLI_RECONNEC_TIMES       5        /* 5��*/
#endif

#ifndef MAX_SCTP_CLI_POLL_THREAD
#define MAX_SCTP_CLI_POLL_THREAD        4     /* sctp�ͻ���֧������߳���*/
#endif

#ifndef TEMP_STRING_LEN
#define TEMP_STRING_LEN            32
#endif

#ifndef XOS_RESEND_BUF_SIZE
#define XOS_RESEND_BUF_SIZE 1000    /*�����ط�������нڵ���*/
#endif

/*-------------------------------------------------------------
                  �ṹ��ö������
--------------------------------------------------------------*/
typedef struct t_SctpResndData t_SctpResnd;
struct  t_SctpResndData
{
    XCHAR *pData;              /*���ݰ�*/
    XU32   msgLenth;           /*�������ܳ���*/
    t_SctpDataAttr attr;        /*���ݷ�����ز���*/
    t_SctpResnd   *pNextPacket; /*��һ�����ӽ����Ŀͻ���*/
};
typedef struct 
{
    XU32  rsnd_total;
    XU32  rsnd_wait;
    XU32  rsnd_success;
    XU32  rsnd_delete;
    XU32  rsnd_fail;
    XU32  rsnd_size;
    t_SctpResnd *pFirstDataReq;  /*û���ͳɹ�������DataReq��ָ��*/
}t_SctpResndPacket;

/*sctp server �ӽ����Ŀͻ�����Ϣ*/
typedef struct  sctpServCliCb t_SSCLI;
/*sctp server ��·���ƿ�*/
typedef struct
{
    HAPPUSER     userHandle;       /*�û����*/
    t_XOSUSERID  linkUser;         /*��·��ҵ��ʹ����*/
    t_XINETFD    sockFd;           /*sock ������*/
    e_LINKSTATE  linkState;        /*��·��״̬*/
    
    HLINKHANDLE linkHandle;        /*��·�����Ϊ��������ָʾʱ
    �ܿ��ٶ����õ�����*/

    XOS_SctpIncomeAuth  authFunc;  /*������֤����ָ�룬����û���������֤����Ҫ��ʼ��ΪNULL*/
    XVOID *pParam;                 /*��֤��������չ����*/
    XU32 maxCliNum;                /*�����Խ���Ŀͻ�������*/
    XU16 usageNum;                 /*�Ѿ�����ͻ��˵�����*/
    XU16 maxStream;                /*sctp���ӵ�����������*/
    XU16 pathmaxrxt;               /*·��������ֵⷥ*/
    XU32 hbInterval;               /*sctp������������*/
    t_SSCLI *pLatestCli;           /*�������Ŀͻ��˿��ƿ�ָ��*/
    t_SCTPIPADDR myAddr;           /*���˵�ַ*/
}t_SSCB;

struct sctpServCliCb
{
    t_XINETFD     sockFd;         /*sock ������*/
    t_SSCB*       pServerElem;    /*ָ��sctp  server �洢��Ϣ��ָ��*/
    t_SSCLI       *pPreCli;       /*ǰһ�����ӽ����Ŀͻ���*/
    t_SSCLI       *pNextCli;      /*��һ�����ӽ����Ŀͻ���*/
    t_SctpResndPacket   packetlist;   /*��������δ���ͳɹ��İ����ƿ�*/
};

/*sctp client ��·���ƿ�*/
typedef struct
{
    HAPPUSER        userHandle;     /*�û����*/
    t_XOSUSERID     linkUser;       /*��·��ҵ��ʹ����*/
    t_XINETFD       sockFd;         /*sock ������*/
    HLINKHANDLE     linkHandle;     /*��·�����Ϊ��������ָʾʱ��
    ���ٶ����õ�����*/
    e_LINKSTATE     linkState;      /*��·״̬*/
    t_SCTPIPADDR        peerAddr;       /*�Զ˵�ַ*/
    t_SCTPIPADDR        myAddr;         /*���˵�ַ*/
    XU16            maxStream;          /*sctp���ӵ�������������*/
    XU16            pathmaxrxt;          /*·��������ֵⷥ*/
    XU32            hbInterval;     /*�������*/
    PTIMER          timerId;        /*��ʱ���ľ��*/
    XS32            expireTimes;    /*��ʱ����ʱ����*/
    t_SctpResndPacket   packetlist;   /*��������δ���ͳɹ��İ����ƿ�*/
}t_SCCB;

/*����ģ���ȫ�ֹ���ṹ*/
typedef struct
{
    XBOOL isInited;              /*�Ƿ��ʼ��*/

    XOS_HARRAY sctpClientLinkH;   /*sctp �ͻ���æ�������*/
    t_XOSMUTEXID sctpClientLinkMutex; /*sctp �ͻ�����·�������*/
    XOS_HARRAY sctpServerLinkH;   /*sctp server æ�������*/
    t_XOSMUTEXID sctpServerLinkMutex; /*sctp server æ�����������*/
    
    /*sctp ����Ŀͻ��ˣ����ڲ�ͬ���߳����޸ģ�Ӧ���ӻ��Ᵽ��*/
    XOS_HHASH   tSctpCliH;        /*sctp server ����Ŀͻ���*/
    t_XOSMUTEXID hashMutex;      /*������*/

    /*��������ص�����*/
    t_NTLTSKINFO   sctpServTsk;     /*sctp server tsk ,ɨ����û�пͻ�����*/
    XS32           sctpSerTskNo;      /*sctp  serv ����ĸ��� */
    t_NTLTSKINFO*  pSctpCliTsk;     /*sctp Task*/
    XS32           sctpCliTskNo;    /*sctp client ���������*/

    XBOOL          isGenCfg;       /*�Ƿ�����ͨ������*/
    t_NTLGENCFG    genCfg;         /*ͨ����������*/
}t_SCTPGLOAB;

/*-------------------------------------------------------------------------
                �ӿں���
-------------------------------------------------------------------------*/
/************************************************************************
������:SCTP_genCfgProc
����:  ����ͨ��������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: SCTP ���յ�ͨ��������Ϣ����������������ɸ�
������ĳ�ʼ����
************************************************************************/
XS32 SCTP_genCfgProc(t_NTLGENCFG* pGenCfg);

/************************************************************************
������:SCTP_Opt
����:  ����������������������·��ż����·���Ķ�����ֵⷥ
����:  pCliCb ��sctp�ͻ��˿��ƿ�ָ��
        peerAddrNum - �Զ˰󶨵�ַ����
���:
����:  �ɹ�����XSUCC,���򷵻�XERROR
˵��:  
************************************************************************/
XS32 SCTP_Opt(t_SCCB *pCliCb,XS32 peerAddrNum);

/************************************************************************
������:SCTP_AcceptClient
����:  ����һ���ͻ������ӣ�����ӵ�����
����:  pServCb ��sctp����˿��ƿ�ָ��
���:
����:  �ɹ�����XSUCC,���򷵻�XERROR
˵��:  ֻ��SCTP_servTsk�е���
************************************************************************/
XS32 SCTP_AcceptClient(t_SSCB  *pServCb);

/************************************************************************
������:SCTP_channel_find_function
����: �����û���������ͨ���Ƿ��ѷ���
���:
����: �ɹ�����XTRUE,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XBOOL SCTP_channel_find_function(XOS_ArrayElement element1, XVOID *param);

/************************************************************************
������:SCTP_CloseSctpServerSocket
����:  �ر�sctpserver
����:  pSctpServCb ��sctpserver���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: sctpserver��Ҫ���÷���ȫ�ֱ���
************************************************************************/
void SCTP_CloseSctpServerSocket(t_SSCB *pSctpServCb);

/************************************************************************
������:SCTP_StartClientTimer
����:  ����sctp�ͻ��˵�������ʱ��
����:  pSctpCb ��sctp���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_StartClientTimer(t_SCCB *pSctpCb, XU32 taskNo);

/************************************************************************
������:SCTP_SetClientFd
����:  sctp client select ��λ����
����:  pSctpCb ����Ϣָ��
       fdFlg  --0:read,1:write,2:read&write
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
void SCTP_SetClientFd(t_SCCB *pSctpCb, XU32 taskNo, e_ADDSETFLAG fdFlg);

/************************************************************************
������:SCTP_StopClientTimer
����:  ֹͣsctp�ͻ��˵Ķ�ʱ��
����:  pSctpCb ��sctp���ƿ�ָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 SCTP_StopClientTimer(t_SCCB *pSctpCb);

/************************************************************************
������:SCTP_ResetAllFd
����:  �������е�sctp��д��
����:  taskNo  �����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_ResetAllFd(XU32 taskNo);

/************************************************************************
������:SCTP_closeReqForRelease
����:  ������·�ͷ�������Ϣ--ֻ����SCTP_linkReleaseProc����
����:  pMsg ����Ϣָ��
���:
����:  �ɹ�����XSUCC,���򷵻�XERROR
˵��:  �˺�����SCTP_closeReqProc������sctpserver�ϴ���ͬ
************************************************************************/
XS32 SCTP_ReleaseLink(t_XOSCOMMHEAD* pMsg);

/************************************************************************
������:SCTP_closeReqProc
����:  ������·�ر�������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_closeReqProc(t_XOSCOMMHEAD* pMsg);

/************************************************************************
������:SCTP_dataReqProc
����: �����ݷ��͵�����
����: pMsg  �����ݷ��͵�ָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqProc(t_XOSCOMMHEAD *pMsg);

/************************************************************************
������:SCTP_dataReqSaveProc
����: �洢����ʧ�ܵ����ݵ�ӵ������
����: t_SctpResndPacket *pPacklist  - ӵ������ָ��
        t_SCTPDATAREQ *pDataReq     - �������ݽṹ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqSaveProc(t_SctpResndPacket *pPacklist,t_SCTPDATAREQ *pDataReq);

/************************************************************************
������:SCTP_dataReqSendProc
����:  �����Ŷ�����
����:  t_XINETFD *pSockFd  - socket���
        t_SctpResndPacket *pPacklist  - ӵ���Ŷ�����
        t_SCTPIPADDR *pDstAddr      - ���ݷ���Ŀ�ĵ�ַ
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqSendProc(t_XINETFD *pSockFd,t_SctpResndPacket *pPacklist,t_SCTPIPADDR *pDstAddr);

/************************************************************************
������:SCTP_dataReqTimerProc
����:  ��ʱ����Ϣ�е�sctp��֧�����������ڶ�ʱ�ط�����
����:  t_BACKPARA* pParam  ��ʱ��Ϣ��ַָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS8 SCTP_timerProc(t_BACKPARA* pParam);

/************************************************************************
������:SCTP_dataReqTimerSend
����:  ��ʱ����Ϣ�е�sctp��֧�����������ڶ�ʱ�ط�����
����:  
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqTimerSend(e_LINKTYPE type);

/************************************************************************
������:SCTP_dataReqTimerProc
����:  ��ʱ����Ϣ�е�sctp��֧�����������ڶ�ʱ�ط�����
����:  t_XOSCOMMHEAD *pMsg  ��Ϣ��ַָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_dataReqTimerProc(t_XOSCOMMHEAD *pMsg);

/************************************************************************
������:SCTP_dataReqClear
����:  ���sctpӵ������
����:  t_SctpResndPacket* pklist    -ӵ������ָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:���ŶӶ��в����,ֱ�����ͳɹ�Ϊֹ
************************************************************************/
XS32 SCTP_dataReqClear(t_SctpResndPacket *pPacklist);

/************************************************************************
������:SCTP_ResetLinkByReapplyEntry
����:  ���ظ�������·��ֹͣԭ������·
����:  e_LINKTYPE linkType      -��������
        HAPPUSER *pUserHandle   -���ӵ��û����
        XS32 *pnRtnFind         -���ƿ������ֵ
���:   t_LINKINITACK *linkInitAck  -������
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_ResetLinkByReapplyEntry(e_LINKTYPE linkType, t_LINKINITACK *linkInitAck, HAPPUSER *pUserHandle, XS32 *pnRtnFind);

/************************************************************************
������:SCTP_CloseServerSocket
����:  �ر�sctpserver
����:  pServCb ��sctpserver���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: sctpserver��Ҫ���÷���ȫ�ֱ���
************************************************************************/
void SCTP_CloseServerSocket(t_SSCB *pServCb);

/************************************************************************
������:SCTP_DeleteCB
����:  �ͷſ��ƿ�
����:  linkType ����·����
       linkIndex--��·����

���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_DeleteCB(e_LINKTYPE linkType, XS32 linkIndex);

/************************************************************************
������:SCTP_linkInitProc
����:  ������·��ʼ����Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_linkInitProc(t_XOSCOMMHEAD* pMsg);

/************************************************************************
������:SCTP_linkStarttProc
����:  ������·������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_linkStartProc(t_XOSCOMMHEAD* pMsg);

/************************************************************************
������:SCTP_linkReleaseProc
����:  ������·�ͷ���Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_linkReleaseProc(t_XOSCOMMHEAD* pMsg);

/************************************************************************
������:SCTP_RestartLink
����:  ��������sctp����
����:   t_SCCB* sctpCliCb
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_RestartLink(t_SCCB* sctpCliCb);

/************************************************************************
������:SCTP_CliCfgShow
����: ��ʾsctp client ������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_CliCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

/************************************************************************
������:SCTP_CliMsgShow
����: ��ʾsctp client ÿ��·ͳ����Ϣ��Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_CliMsgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

/************************************************************************
������:SCTP_ServMsgShow
����: ��ʾsctp server's client ÿ��·ͳ����Ϣ��Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_ServMsgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

/************************************************************************
������:SCTP_ServCfgShow
����: ��ʾsctp client ������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_ServCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

#endif

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _XOS_SCTP_H_ */

