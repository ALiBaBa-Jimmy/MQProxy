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
**  date:   2013.11.11
**
**************************************************************/
#ifndef _XOS_SCTP_WIN_H_
#define _XOS_SCTP_WIN_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

#if defined(XOS_SCTP) && defined(XOS_WIN32)

/*-------------------------------------------------------------
                  ����ͷ�ļ�
--------------------------------------------------------------*/
#include "xostype.h"
#include "xostl.h"
#include "xosinet.h"
#include "xosmodule.h"
#include "xosarray.h"
#include "xosntl.h"
#include "sctp_win.h"
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
#define MAX_SCTP_CLI_POLL_THREAD    4     /* sctp�ͻ���֧������߳���*/
#endif

#ifndef SCTP_CLIENTS_PER_SERV
#define SCTP_CLIENTS_PER_SERV    10    /*һ��sctp server ƽ���ý�����ٿͻ���*/
#endif

#ifndef TEMP_STRING_LEN
#define TEMP_STRING_LEN         32
#endif

/*-------------------------------------------------------------
                  �ṹ��ö������
--------------------------------------------------------------*/

/*sctp server �ӽ����Ŀͻ�����Ϣ*/
typedef struct  sctpServCliCb t_SSCLI;
/*sctp server ��·���ƿ�*/
typedef struct
{
    HAPPUSER     userHandle;       /*�û����*/
    t_XOSUSERID      linkUser;     /*��·��ҵ��ʹ����*/
    e_LINKSTATE  linkState;        /*��·��״̬*/
    XU16         instance;          /*sctp��·���*/
    
    HLINKHANDLE linkHandle;        /*��·�����Ϊ��������ָʾʱ
    �ܿ��ٶ����õ�����*/

    XU32 maxCliNum;                /*�����Խ���Ŀͻ�������*/
    XU16 usageNum;                 /*�Ѿ�����ͻ��˵�����*/
    XU16 maxStream;                 /*sctp���ӵ�����������*/
    XU32 hbInterval;                 /*sctp������������*/
    t_SSCLI *pLatestCli;           /*�������Ŀͻ��˿��ƿ�ָ��*/
    t_SCTPIPADDR myAddr;               /*���˵�ַ*/
}t_SSCB;

struct sctpServCliCb
{
    XU32        assocID;            /*ż��ID*/
    t_IPADDR    destAddr;       /*��������˵Ŀͻ��˵�����ַ*/
    t_SSCB*       pServerElem;    /*ָ��sctp  server �洢��Ϣ��ָ��*/
    t_SSCLI       *pPreCli;       /*ǰһ�����ӽ����Ŀͻ���*/
    t_SSCLI       *pNextCli;      /*��һ�����ӽ����Ŀͻ���*/
};

/*sctp client ��·���ƿ�*/
typedef struct
{
    HAPPUSER        userHandle;     /*�û����*/
    t_XOSUSERID     linkUser;       /*��·��ҵ��ʹ����*/
    XU16         instance;          /*sctp��·���*/
    XU32            assocID;            /*ż��ID*/
    HLINKHANDLE     linkHandle;     /*��·�����Ϊ��������ָʾʱ��
    ���ٶ����õ�����*/
    e_LINKSTATE     linkState;      /*��·״̬*/
    t_SCTPIPADDR        peerAddr;       /*�Զ˵�ַ*/
    t_SCTPIPADDR        myAddr;         /*���˵�ַ*/
    XU16 maxStream;                 /*sctp���ӵ�������������*/
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
������:SCTP_channel_find_function
����: �����û���������ͨ���Ƿ��ѷ���
���:
����: �ɹ�����XTRUE,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XBOOL SCTP_channel_find_function(XOS_ArrayElement element1, XVOID *param);

/************************************************************************
������:SCTP_closeTsCli
����: �ر�һ��sctp server ����Ŀͻ���
����:pSctpServCli ��server client �Ŀ��ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 SCTP_closeTsCli(t_SSCLI* pSctpServCli);

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
������:SCTP_findClient
����:  ����sctp ���ӵĿͻ�
����:
pTserverCb ��server cb��ָ��
pClientAddr  �� �ӽ����ͻ��ĵ�ַָ��
���:
����: �ɹ����ؿ��ƿ�ָ��,���򷵻�xnullp
˵��:
************************************************************************/
t_SSCLI * SCTP_findClient(t_SSCB *pTserverCb,t_IPADDR *pClientAddr);

XS32 SCTP_init();

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
������:SCTP_ServCfgShow
����: ��ʾsctp client ������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_ServCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

/************************************************************************
������:SCTP_ServCfgShow
����: ��ʾsctp client ������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
************************************************************************/
XVOID SCTP_msgQueueShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

#endif

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _XOS_SCTP_H_ */

