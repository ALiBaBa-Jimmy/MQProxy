/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosNtl.h
**
**  description: ipc managment defination
**
**  author: wangzongyou
**
**  date:   2006.3.7
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification

**************************************************************/
#ifndef _XOS_NTL_H_
#define _XOS_NTL_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/*-------------------------------------------------------------
                  ����ͷ�ļ�
--------------------------------------------------------------*/
#include "xostype.h"
#include "xostl.h"
#include "xosinet.h"
#include "xosmodule.h"
#include "xosarray.h"
#include "xoshash.h"
#include "xosxml.h"
#include "xmlparser.h"

/*-------------------------------------------------------------
                  �궨��
--------------------------------------------------------------*/
/*�򵥵�����*/
#define  NTL_SHOW_CMD_CTL(x) ((x)%20 == 0)? (XOS_Sleep(5)) : XOS_UNUSED(x)

#define TCP_CLIENTS_PER_SERV    100    /*һ��tcp server ƽ���ý�����ٿͻ���*/
#define FDS_PER_THREAD_POLLING  256   /* һ��������ӵ�fd ����*/
#define FDS_MAX_THREAD_POLLING  1024   /* һ��������ӵ����fd ����*/
#define MAX_UDP_POLL_THREAD     10     /* udp֧������߳���*/
#define MAX_TCP_CLI_POLL_THREAD 4     /* tcp  �ͻ���֧������߳���*/
#define NTL_TSK_NAME_LEN        20

/*��ʱ����ʱ��*/
#define TCP_CLI_RECONNECT_INTERVAL   4000    /*4Sec 2007-08-ComCore adjust to 10Sec*/
/*��ʱ�����Ĵ���*/
#define TCP_CLI_RECONNEC_TIMES       5        /* 5��*/

#define NTL_SUBMSG_OLD_SOCK          1        /*for reconnect to keep old socket reconnect*/
#define NTL_SUBMSG_NEW_SOCK          2        /*for submsg to create new tcp client socket*/

/*select ��ʱʱ��*/
#ifdef XOS_LINUX
#define POLL_FD_TIME_OUT             5      /*5 mSec */
#else
#define POLL_FD_TIME_OUT             200      /*5 mSec */
#endif

/*NTL ���������ջ��С*/
#define NTL_RECV_TSK_STACK_SIZE     1024000    /*2007-09-10 changed to doubled from 30000 to 60000*/
                                              /*2008-07-28 changed to doubled from 60000 to 102400*/
/*set ����ص���Ϣ*/
typedef struct
{
    t_SOCKSET     readSet;    /*read  ��*/
    t_SOCKSET     writeSet;   /*write set */
    t_XOSMUTEXID  fdSetMutex; /*���Ｏ�϶�����̷߳���,�ʲ��û������*/
    XU16          sockNum;    /* ������sock������*/
}t_FDSETINFO;

/*������ص���Ϣ*/
typedef struct
{
    t_FDSETINFO     setInfo;     /*fdset ����Ϣ,һ�������̶߳������һ��fdset*/
    t_XOSSEMID      taskSemp;    /*�����߳��������¼�*/
    t_XOSTASKID     taskId;      /* ����id*/
    XBOOL           activeFlag;  /*�̻߳�ı�־,������*/
}t_NTLTSKINFO;

/*���紫���(NTL)��ͨ������, NTL ͨ��
�������ļ�����*/
typedef struct
{
    XU16 maxUdpLink;             /*֧�ֵ����ʹ�õ�udp��·����*/
    XU16 maxTcpCliLink;          /*֧�ֵ����ʹ�õ�tcp client ����*/
    XU16 maxTcpServLink;         /*֧�����ʹ�õ�tcp server ������*/
#ifdef XOS_SCTP
    XU16 maxSctpCliLink;         /*֧�ֵ����ʹ�õ�sctp client ����*/
    XU16 maxSctpServLink;        /*֧�����ʹ�õ�sctp server ������*/
    XU16 sctpClientsPerServ;     /*ÿ��sctp server֧�ֵĿͻ��˽�������*/

    XU32 hb_interval;            /*sctp��·�������ʱ��(ms)*/
    XU16 rto_min;                /*sctp���ݰ���һ���ط�(ms)*/
    XU16 rto_init;               /*sctp���ݰ��ڶ����ط�(ms)*/
    XU16 sack_timeout;           /*sack��ʱ����ʱ��*/
#endif
    /*��֧�ֵ�����Ϊ��ʱ����ʾ��֧�ִ�����·����*/
    
    XU16 fdsPerThrPolling;  /*һ�����߳���ѯ������������*/
} t_NTLGENCFG;

/*ipc����ģ���ȫ�ֹ���ṹ*/
typedef struct
{
    XBOOL isInited;              /*�Ƿ��ʼ��*/

    XOS_HARRAY udpLinkH;         /*���udp��·��Ϣ�ľ��*/
    t_XOSMUTEXID udpLinkMutex;   /*udp ��·��Ϣ���*/
    XOS_HARRAY tcpClientLinkH;   /*tcp �ͻ���æ�������*/
    t_XOSMUTEXID tcpClientLinkMutex; /*tcp �ͻ�����·�������*/
    XOS_HARRAY tcpServerLinkH;   /*tcp server æ�������*/
    t_XOSMUTEXID tcpServerLinkMutex; /*tcp server æ�����������*/

    /*tcp ����Ŀͻ��ˣ����ڲ�ͬ���߳����޸ģ�Ӧ���ӻ��Ᵽ��*/
    XOS_HHASH   tSerCliH;        /*tcp server ����Ŀͻ���*/
    t_XOSMUTEXID hashMutex;      /*������*/

    /*��������ص�����*/
    t_NTLTSKINFO   tcpServTsk;     /*tcp server tsk ,ֻɨ����û�пͻ��������*/
    XS32           servTskNo;      /*tcp  serv ����ĸ��� */
    t_NTLTSKINFO*  pUdpTsk;        /*udp Task*/
    XS32           udpTskNo;       /*udp ����ĸ���*/
    t_NTLTSKINFO*  pTcpCliTsk;     /*tcp Task*/
    XS32           tcpCliTskNo;    /*tcp client ���������*/

    XBOOL          isGenCfg;       /*�Ƿ�����ͨ������*/
    t_NTLGENCFG    genCfg;         /*ͨ����������*/
}t_NTLGLOAB;

/*-------------------------------------------------------------
                  �ṹ��ö������
--------------------------------------------------------------*/
/*��·״̬*/
typedef enum
{
    eNullLinkState,        /*״̬��*/
    eStateInited,          /*�Ѿ���ʼ����*/
    eStateStarted,         /*�Ѿ�������*/
    eStateConnecting,      /*��������״̬*/
    eStateConnected,       /*�����Ѿ����������Է������ݵ�״̬*/
    eStateListening ,      /*����״̬*/
    eStateWaitClose,       /*�ȴ��ر�״̬��֧�ְ�ر�ʱ�õ�*/

    eMaxLinkState
}e_LINKSTATE;

/*20080710 add below*/
typedef struct  t_ResndDataReq t_RsndData;
struct  t_ResndDataReq
{
    XCHAR *pData;              /*���ݰ�*/
    XU32   msgLenth;           /*�������ܳ���*/
    XU32   i_offset;           /*����������ƫ��ֵ*/
    t_RsndData   *pNextPacket; /*��һ�����ӽ����Ŀͻ���*/
};
typedef struct 
{
    XU32  rsnd_total;
    XU32  rsnd_wait;
    XU32  rsnd_success;
    XU32  rsnd_delete;
    XU32  rsnd_fail;
    XU32  rsnd_size;
    t_RsndData *pFirstDataReq;  /*û���ͳɹ�������DataReq��ָ��20080710add*/
}t_ResndPacket;

typedef enum 
{
    eTCPResendTimer=eMaxTlMsg+1,
        eTCPResendCheck,
        eSCTPCliResendTimer,
        eSCTPCliResendCheck,
        eSCTPSerResendTimer,
        eSCTPSerResendCheck,
        eSCTPReconnect
}e_DataREQRsndMSg;

/*20080710 add above*/

/*tcp server �ӽ����Ŀͻ�����Ϣ*/
typedef struct  tcpServCliCb t_TSCLI;

/*tcp server ��·���ƿ�*/
typedef struct
{
    HAPPUSER     userHandle;       /*�û����*/
    t_XOSUSERID      linkUser;     /*��·��ҵ��ʹ����*/
    t_XINETFD     sockFd;          /*sock ������*/
    e_LINKSTATE  linkState;        /*��·��״̬*/
    
    HLINKHANDLE linkHandle;        /*��·�����Ϊ��������ָʾʱ
    �ܿ��ٶ����õ�����*/

    XOS_TLIncomeAuth  authFunc;    /*������֤����*/
    XVOID *pParam;                 /*��֤������ָ��*/
    XU16 maxCliNum;                /*�����Խ���Ŀͻ�������*/
    XU16 usageNum;                 /*�Ѿ�����ͻ��˵�����*/
    t_TSCLI *pLatestCli;           /*�������Ŀͻ��˿��ƿ�ָ��*/
    t_IPADDR myAddr;               /*���˵�ַ*/
}t_TSCB;

struct tcpServCliCb
{
    t_XINETFD     sockFd;         /*sock ������*/
    t_TSCB*       pServerElem;    /*ָ��tcp  server �洢��Ϣ��ָ��*/
    t_TSCLI       *pPreCli;       /*ǰһ�����ӽ����Ŀͻ���*/
    t_TSCLI       *pNextCli;      /*��һ�����ӽ����Ŀͻ���*/
    t_ResndPacket   packetlist;   /*��������δ���ͳɹ��İ����ƿ�20080710 add*/
    //  t_IPADDR    peerAddr;       /*��Ӧ�ͻ��˵�ip��ַ*/  /*��Ϊkey��*/
};

/*udp ��·���ƿ�*/
typedef struct
{
    HAPPUSER     userHandle;        /*�û����*/
    t_XOSUSERID      linkUser;      /*��·��ҵ��ʹ����*/
    t_XINETFD     sockFd;           /*sock ������*/
    t_IPADDR      peerAddr;         /*�Զ˵ĵ�ַ*/
    t_IPADDR      myAddr;           /*���˵�ַ*/
    e_LINKSTATE  linkState;         /*��·��״̬*/
    HLINKHANDLE linkHandle;         /*��·�����Ϊ��������ָʾʱ��
    ���ٶ����õ�����*/
}t_UDCB;

/*tcp client ��·���ƿ�*/
typedef struct
{
    HAPPUSER        userHandle;     /*�û����*/
    t_XOSUSERID     linkUser;       /*��·��ҵ��ʹ����*/
    t_XINETFD       sockFd;         /*sock ������*/
    HLINKHANDLE     linkHandle;     /*��·�����Ϊ��������ָʾʱ��
    ���ٶ����õ�����*/
    e_LINKSTATE     linkState;      /*��·״̬*/
    t_IPADDR        peerAddr;       /*�Զ˵�ַ*/
    t_IPADDR        myAddr;         /*���˵�ַ*/
    PTIMER          timerId;        /*��ʱ���ľ��*/
    XS32            expireTimes;    /*��ʱ����ʱ����*/
    t_ResndPacket   packetlist;   /*��������δ���ͳɹ��İ����ƿ�20080710 add*/
}t_TCCB;

/*������·��Դ��ѯ*/
typedef struct
{
    t_XOSUSERID *linkUser; /*��·��ҵ��ʹ����*/
    HAPPUSER userHandle;  /*�û����*/
    e_LINKTYPE  linkType;    /*������·������*/
}t_Link_Index;

#if 0 /*�ĳɶ������ļ�����*/
/*ͨ��������Ϣ������ntl ģ��ֻ��ִ��һ��ͨ�����á�
��ִ����ͨ��������Ϣ���Ժ��յ���ͨ��������Ϣ��Ч*/
typedef struct
{
    XU16 maxUdpLink;        /*֧�ֵ����ʹ�õ�udp��·����*/
    XU16 maxTcpCliLink;     /*֧�ֵ����ʹ�õ�tcp client ����*/
    XU16 maxTcpServLink;    /*֧�����ʹ�õ�tcp server ������*/
    
    /*��֧�ֵ�����Ϊ��ʱ����ʾ��֧�ִ�����·����*/
    
    XU16 fdsPerThrPolling;  /*һ�����߳���ѯ������������*/
    
}t_NTLGENCFG;
#endif

//20081019����socket�쳣�رշ�����
#define NTL_SHTDWN_RECV   0x0001  /* Read file descriptor group list */
#define NTL_SHTDWN_SEND   0x0002  /* Write file descriptor group list */
#define NTL_SHTDWN_BOTH   (NTL_SHTDWN_RECV | NTL_SHTDWN_SEND)  


/*-------------------------------------------------------------------------
                �ӿں���
-------------------------------------------------------------------------*/
XBOOL XOS_ReadNtlGenCfg( t_NTLGENCFG* pNtl, XCHAR* filename );

HLINKHANDLE  NTL_buildLinkH(  e_LINKTYPE linkType,XU16 linkIndex);
e_LINKTYPE NTL_getLinkType( HLINKHANDLE linkHandle);
XS32 NTL_getLinkIndex( HLINKHANDLE linkHandle);
XBOOL NTL_isValidLinkH( HLINKHANDLE linkHandle);
XU32  NTL_tcliHashFunc( XVOID *param, XS32 paramSize, XS32 hashSize);
XBOOL NTL_cmpIpAddr (XVOID* key1,XVOID* key2,XU32 keySize);
XS32 NTL_msgToUser(XVOID *pContent, t_XOSUSERID *pLinkUser, XS32 len, e_TLMSG msgType);
XS32 NTL_dataReqClear(t_ResndPacket *pPacklist);
XS32 NTL_dataReqSaveProc(t_ResndPacket *pPacklist,t_DATAREQ *pDataReq,XS32 out_len);

/************************************************************************
������:NTL_timerProc
����:  ntl ģ�鶨ʱ����Ϣ���������
����:  pMsg ����Ϣָ��
���:
����: �ɹ�����XOS_XSUCC , ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS8 NTL_timerProc( t_BACKPARA* pParam);

/************************************************************************
������:NTL_msgProc
����:  ntl ģ����Ϣ���������
����:  pMsg ����Ϣָ��
���:
����: �ɹ�����XOS_XSUCC , ʧ�ܷ���XERROR
˵��: ����Ϣ��������ntl��������Ϣ��ڣ�edataSend
��Ϣ���ڴ˺�������ķ�Χ��
************************************************************************/
XS8 NTL_msgProc(XVOID* pMsgP, XVOID*sb );

/************************************************************************
������:NTL_Init
����: ������ntlģ��
����:
���:
����:
˵��: ע�ᵽģ�������
************************************************************************/
XS8 NTL_Init(void*p1, void*p2);
XS8 NTL_TelnetCliCloseSock(t_IPADDR ipAddr);
/************************************************************************
������:NTL_channel_find_function
����: �����û���������ͨ���Ƿ��ѷ���
���:
����: �ɹ�����XTRUE,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XBOOL NTL_channel_find_function(XOS_ArrayElement element1, XVOID *param);

/************************************************************************
������:NTL_StopLinkByReapply
����:  ���ظ�������·��ֹͣԭ������·
����:  pCloseReq ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 NTL_ResetLinkByReapply(t_LINKCLOSEREQ* pCloseReq);

/************************************************************************
������:NTL_ResetLinkByReapplyEntry
����:  ���ظ�������·��ֹͣԭ������·
����:  pCloseReq ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 NTL_ResetLinkByReapplyEntry(e_LINKTYPE linkType, t_LINKINITACK *linkInitAck, HAPPUSER *pUserHandle, XS32 *pnRtnFind);

/************************************************************************
������:NTL_CloseUdpSocket
����:  �ر�udp��socket�������ö���
����:  pUdpCb ��udp���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: �˺�����udp���ƿ���Ҫ���÷�����ȫ�ֱ���
************************************************************************/
XS16 NTL_CloseUdpSocket(t_UDCB *pUdpCb, XU32 taskNo);

/************************************************************************
������:NTL_CloseTcpServerSocket
����:  �ر�tcpserver
����:  pTcpServCb ��tcpserver���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: tcpserver��Ҫ���÷���ȫ�ֱ���
************************************************************************/
XS32 NTL_CloseTcpServerSocket(t_TSCB *pTcpServCb);

/************************************************************************
������:NTL_StartTcpClientTimer
����:  ����tcp�ͻ��˵�������ʱ��
����:  pTcpCb ��tcp���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 NTL_StartTcpClientTimer(t_TCCB *pTcpCb, XU32 taskNo);

/************************************************************************
������:NTL_SetTcpClientFd
����:  tcp client select ��λ����
����:  pTcpCb ����Ϣָ��
       fdFlg  --0:��,1:д
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/            
void NTL_SetTcpClientFd(t_TCCB *pTcpCb, XU32 taskNo, e_ADDSETFLAG fdFlg);

/************************************************************************
������:NTL_StopTcpClientTimer
����:  ֹͣtcp�ͻ��˵Ķ�ʱ��
����:  pTcpCb ��tcp���ƿ�ָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 NTL_StopTcpClientTimer(t_TCCB *pTcpCb);

/************************************************************************
������:NTL_SetUdpSelectFd
����:  udp select ��λ����
����:  pUdpCb ��udp���ƿ�
       taskNo  --�����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/    
void NTL_SetUdpSelectFd(t_UDCB *pUdpCb, XU32 taskNo);

/************************************************************************
������:NTL_DeleteCB
����:  �ͷſ��ƿ�
����:  linkType ����·����
       linkIndex--��·����

���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 NTL_DeleteCB(e_LINKTYPE linkType, int linkIndex);

/************************************************************************
������:NTL_StartUdpLink
����:  ����udp��·
����:    pDatasrc ����ϢԴָ��
        pLinkStart--��·����ָ��

���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 NTL_StartUdpLink(t_XOSUSERID *pDatasrc, t_LINKSTART *pLinkStart);

/************************************************************************
������:NTL_ResetAllTcpFd
����:  �������е�tcp��д��
����:  taskNo  �����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 NTL_ResetAllTcpFd(XU32 taskNo);



/*2007/12/05����ͨѶģ�鷢�����ݱ�����ԭ���ӡ*/
XS8* NTL_getErrorTypeName(XS32 reason_code, XCHAR *pneterror_name, int nLen);
/*2007/12/05����ͨѶģ�鷢�����ݱ�����ԭ���ӡ*/
XS8* NTL_getLinkStateName(XS32 link_state, XCHAR *pLinkstate_name, int nLen);


#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLISHELL_H_ */

