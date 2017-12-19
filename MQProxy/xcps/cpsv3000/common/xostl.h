/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xostl.h
**
**  description:  
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
**   wangzongyou         2006.3.7              create  
**************************************************************/
#ifndef _XOS_TL_H_
#define _XOS_TL_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xostype.h"

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
//#define  aaa

XOS_DECLARE_HANDLE(HAPPUSER);   /*�û���·���ƾ��*/
XOS_DECLARE_HANDLE(HLINKHANDLE); /*�������·���*/

/*-------------------------------------------------------------------------
                                 �ṹ��ö������
-------------------------------------------------------------------------*/


/************************************************************************
������: XOS_TLIncomeAuth
���ܣ�������֤����
���룺
incomeAddr  -- �����ip��ַ
param          -- �������չ����
�����
���أ���֤�ɹ�����XTRUE, ʧ�ܷ���XFALSE
˵�������û�����Ҫ������֤ʱ,����ʵ�ִ˺���
************************************************************************/
typedef XBOOL (*XOS_TLIncomeAuth)( HAPPUSER userHandle, t_IPADDR * incomeAddr, XVOID* param);


/************************************************************************
������: XOS_SctpIncomeAuth
���ܣ�������֤����
���룺
incomeAddr  -- �����ip��ַ��
param       -- �������չ����
�����
���أ���֤�ɹ�����XTRUE, ʧ�ܷ���XFALSE
˵�������û�����Ҫ������֤ʱ,����ʵ�ִ˺���
************************************************************************/
typedef XBOOL (*XOS_SctpIncomeAuth)( HAPPUSER userHandle, t_SCTPIPADDR * incomeAddr, XVOID* param);



/*�������ϲ㽻������Ϣ����*/
typedef enum
{
    eMinTlMsg,
    eLinkInit ,                /*��ʼ����·            APP->NTL  */
    eInitAck,                   /*��·��ʼ��ȷ��    NTL ->APP */
    eLinkStart,            /*������·                   APP->NTL */
    eStartAck,             /*��·����ȷ��        NTL ->APP */
    eSendData,             /*��������                   APP->NTL */
    eErrorSend,               /* ���ݷ��ʹ���        NTL ->APP*/
    eConnInd,                    /*����ָʾ                        NTL ->APP */
    eDataInd,            /*�յ�����                   NTL ->APP*/
    eLinkStop,            /*�ر���·                   APP->NTL */
    eStopInd,                  /* ��·�ر�ָʾ        NTL ->APP*/
    eLinkRelease,              /*�ͷ���·                   APP->NTL*/

    /*sctp*/
    eSctpInitAck,                   /*��·��ʼ��ȷ��    NTL ->APP */
    eSctpStartAck,             /*��·����ȷ��        NTL ->APP */
    eSctpErrorSend,               /* ���ݷ��ʹ���        NTL ->APP*/
    eSctpDataInd,            /*�յ�����                   NTL ->APP*/
    eSctpConnInd,                    /*����ָʾ             NTL ->APP */
    eSctpStopInd,                  /* ��·�ر�ָʾ        NTL ->APP*/
    
    /*����չ*/
    eStaticReq,                 /*��·��Ϣͳ������   APP->NTL*/
    eStaticAck,                 /*��·ͳ����Ϣȷ��   NTL ->APP*/
    
    eMaxTlMsg
}e_TLMSG;


/*������·�����Ͷ���*/
typedef enum 
{
    eNullLinkType,
    eUDP,
    eTCPClient,
    eTCPServer,
    eSCTPClient,
    eSCTPServer,
    /*���ipЭ������ڴ���չ,Ŀǰֻ֧��UDP,TCP,SCTP*/
    eSCTP, /*�������ѷ�����ʵ��ΪeSCTPClient��eSCTPServer����ֵ���ڼ���ԭ�д���*/
    ePCI,
    eOtherLinkType
}e_LINKTYPE;

/*ȷ�Ͻ������*/
typedef enum
{
    eNullResult,
    eSUCC,        /*�ɹ�*/
    eFAIL,         /*ʧ��*/
    eBlockWait, /*���ӵȴ���*/
    eReapply,   /*��Դ�ظ�����*/
    eInvalidInstrm,    /*�Զ����õ�������̫С���Ƿ�*/
    eOtherResult
}e_RESULT;

/*��������*/
typedef enum
{
    eErrorNetInterrupt,   /*�����ж�*/
    eErrorNetBlock,    /*��������*/
    eErrorDstAddr,  /*Ŀ���ַ����,û�����öԶ˵�ַ����*/
    eErrorOverflow, /*������������ݳ���̫����*/    
    eErrorLinkClosed,   /*�Զ˹ر�����*/
    eErrorLinkState,    /*��·״̬����,û��������ر���*/

    eErrorStreamNum,        /*sctp,��������ָ�����Ŵ���*/
    /*����չ,�����ϲ�Ĵ�����*/
    
    eOtherErrorReason
}e_ERRORTYPE;  /*�����ԭ�������*/

/*�رյ�ԭ��*/
typedef enum 
{
    eLinkReuse, /*��·�ظ�ʹ�ã����ظ����û���ռ��*/
    ePeerReq,  /*�Զ�����*/
    eNetError,  /*�������*/
    
    /*����չ,�����ϲ�Ĵ���*/
    eOtherCloseReason
}e_CLOSERESEASON;

/*��Ӷ�д���ı�־λ,liukai 2013.10.15*/
typedef enum
{
    eRead = 0,
    eWrite,
    eReadWrite
}e_ADDSETFLAG;

/*��·������*/
typedef enum
{
    eNullLinkCtrl,
    e16kUdpPkt,                  /*udp ��·������, �򿪺�֧��16k��С��udp��*/
    eCompatibleTcpeen,     /*tcp ��·������, �򿪺����tcp��װ,
                                     ÿ�������1900���ֽ�*/
                                     /*�����Ĵ���չ*/
    eMaxLinkCtrl
}e_LINKCTRL;


/*Ϊ��д���뷽�㣬���������*/
/*udp ������Ҫ�Ĳ���*/
typedef struct 
{
    t_IPADDR myAddr; /*���˵ĵ�ַ*/
    t_IPADDR peerAddr; /*�Զ˵ĵ�ַ*/
} t_UDPSTART;

/*tcp �Ŀͻ�����·����*/
typedef    struct 
{
    t_IPADDR myAddr ; /*���˵ĵ�ַ*/
    t_IPADDR peerAddr; /*�Զ˵�ַ*/
    XS32  recntInteval;  /*��ʱ�����ļ����0��ʾ������*/
}t_TCPCLISTART ;  

/*tcp �ķ�������*/
typedef  struct 
{
    t_IPADDR myAddr;/*���˵ĵ�ַ*/
    XU32 allownClients; /*�������ͻ��˵�����*/
    XOS_TLIncomeAuth  authenFunc; /*������֤����ָ�룬 NULL��ʾ����֤*/
    void *pParam;                 /*��֤�����Ĳ���*/
} t_TCPSERSTART;

/*sctp �Ŀͻ�����·����*/
typedef    struct 
{
    t_SCTPIPADDR myAddr ; /*���˵ĵ�ַ*/
    t_SCTPIPADDR peerAddr; /*�Զ˵�ַ*/
    XU16  pathmaxrxt;  /*·��������ֵⷥ���û��粻��������0*/
    XU16  streamNum;      /*sctp�ͻ��˵ĳ������������û��粻��������0*/
    XU32  hbInterval;       /*�������ʱ�䣬��λ���룬�û��粻��������0*/
}t_SCTPCLISTART;

/*sctp �ķ�������*/
typedef  struct
{
    t_SCTPIPADDR myAddr;            /*���˵ĵ�ַ*/
    XU16 allownClients;             /*�������ͻ��˵�����*/
    XU16 streamNum;                 /*sctp����˵ĳ������������û��粻��������0*/
    XU16  pathmaxrxt;               /*·��������ֵⷥ���û��粻��������0*/
    XU32 hbInterval;                /*�������ʱ�䣬��λ���룬�û��粻��������0*/
    XOS_SctpIncomeAuth  authenFunc; /*������֤����ָ�룬 NULL��ʾ����֤*/
    void *pParam;                   /*��֤�����Ĳ���*/
} t_SCTPSERSTART;

/*��·�����Ĳ���*/
typedef union
{   
    t_UDPSTART udpStart; 
    t_TCPCLISTART tcpClientStart;
    t_TCPSERSTART tcpServerStart;
    t_SCTPCLISTART sctpClientStart;   
    t_SCTPSERSTART sctpServerStart;    
} u_LINKSTART;

/*��·��ʼ��*/
typedef struct
{ 
    e_LINKTYPE  linkType;    /*������·������*/
    e_LINKCTRL  ctrlFlag;     /*��·������,������չҵ����*/
    HAPPUSER  appHandle;   /*�û������ͨ��ָ�������Ժ����·
                              �����е�������Ϣ����������ʶ*/
}t_LINKINIT;

/*��·��ʼ��ȷ��*/
typedef struct
{
    HAPPUSER appHandle;    /* Ӧ�ò���������ʱ��Ӧ�ò㴫���*/
    e_RESULT  lnitAckResult;  /*��ʼ����·�Ľ����
                           �������ΪeSUCCʱ�����µ��ֶ���Ч*/
    HLINKHANDLE linkHandle; /*��·������Ժ����·���е�����
                           ��Ϣ�ж�������Ӵ˱�ʶ*/
}t_LINKINITACK;

/*��·����*/
typedef struct
{
    HLINKHANDLE linkHandle; /*��·������ڳ�ʼ����·ʱ���ص�*/
    u_LINKSTART  linkStart;   /*��·��������*/
}t_LINKSTART;

/*��·����ȷ��*/
typedef struct 
{
    HAPPUSER  appHandle;     /* Ӧ�ò���������ʱ��Ӧ�ò㴫���*/
    e_RESULT   linkStartResult;    /*  ��·����ȷ�ϵĽ��*/
    t_IPADDR   localAddr;       /*����·ʵ��ͨ�Ŷ˿�*/
}t_STARTACK;

/*sctp��·����ȷ��*/
typedef struct 
{
    HAPPUSER  appHandle;     /* Ӧ�ò���������ʱ��Ӧ�ò㴫���*/
    e_RESULT   linkStartResult;    /*  ��·����ȷ�ϵĽ��*/
    t_SCTPIPADDR   localAddr;       /*����·ʵ��ͨ�Ŷ˿�*/
}t_SCTPSTARTACK;

/*���ݷ���*/
typedef struct
{
    HLINKHANDLE linkHandle; /*��·������ڳ�ʼ����·ʱ���ص�*/
    t_IPADDR   dstAddr;    /*����Ŀ�ĵ�ַ����Ŀ�ĵ�ַΪ��ʱ��
                            ʹ����·����ʱ�����õĶԶ˵�ַ��
                            TCPserver ��������ʱ�����Ϊ��*/
    XU32  msgLenth;         /*�ϲ�Ҫ������Ϣ�ĳ���*/
    XCHAR* pData;            /*Ҫ���͵�����ͷָ��*/ 
}t_DATAREQ;

/*SCTP���ݷ����������*/
typedef struct
{
    XU32  ppid;             /*����Э���ʶ������ѡ��Ĭ��0*/
    XU16 stream;            /*ָ���������ݵ����ţ���ѡ��Ĭ��0*/
    XU32 context;           /*�����ģ���ѡ��Ĭ��0*/
}t_SctpDataAttr;
/*SCTP���ݷ���*/
typedef struct
{
    HLINKHANDLE linkHandle; /*��·������ڳ�ʼ����·ʱ���ص�*/
    t_IPADDR   dstAddr;    /*��������ʱ�������Ϊ��*/
    XU32  msgLenth;         /*�ϲ�Ҫ������Ϣ�ĳ���*/
    XCHAR* pData;            /*Ҫ���͵�����ͷָ��*/ 
    t_SctpDataAttr attr;    /*���ݷ�����ز���������û���������0,����ʹ�����ֵ*/
}t_SCTPDATAREQ;

/*���ݷ���ʧ��ָʾ*/
typedef struct
{
    HAPPUSER        userHandle;
    e_ERRORTYPE   errorReson;  /*���ݷ��ʹ���ԭ��*/
    //XCHAR*     pData;    /*Ҫ���͵�����ͷָ��*/
}t_SENDERROR; 

/*SCTP���ݷ���ʧ��ָʾ*/
typedef struct
{
    HAPPUSER        userHandle;
    e_ERRORTYPE   errorReson;  /*���ݷ��ʹ���ԭ��*/
    t_IPADDR peerIp;    /*�������ݵ�Ŀ�ĵ�ַ*/
}t_SENDSCTPERROR; 

/*�յ�����ָʾ*/
typedef struct
{
    HAPPUSER appHandle;
    t_IPADDR peerAddr; /*�Զ˵�ַ*/
    XU32 dataLenth ;  /*�������ݳ���*/
    XCHAR* pData;  /*���յ����ݵ�ͷָ��,��Ҫ�û��ͷ�*/
}t_DATAIND;

/*sctp�յ�����ָʾ*/
typedef struct
{
    HAPPUSER appHandle;
    t_IPADDR peerAddr; /*�Զ˵�ַ*/
    XU32 dataLenth ;  /*�������ݳ���*/
    XCHAR* pData;  /*���յ����ݵ�ͷָ��,��Ҫ�û��ͷ�*/
    t_SctpDataAttr attr;    /*���ݽ�����ز���*/
}t_SCTPDATAIND;

/*����ָʾ,ֻ��tcp server����Ч*/
typedef struct
{
    HAPPUSER appHandle;
    t_IPADDR  peerAddr;   /*���ӽ����Ŀͻ��˵�ַ*/
}t_CONNIND;

/*��·�ر�ָʾ*/
typedef struct
{
    HAPPUSER appHandle;
    e_CLOSERESEASON closeReason;
    t_IPADDR peerAddr; /*tcp server �ô���ָʾ�ĸ�����Ŀͻ��˶Ͽ���*/
}t_LINKCLOSEIND;

/* �ر���·����*/
typedef struct
{
    HLINKHANDLE linkHandle;
    t_IPADDR  cliAddr;            /*�����tcp server��ָ���õ�ַ���ǹر�ָ���Ŀͻ���;
                                  ��ip��port ����0 ��ʱ�򣬹ر����еĿͻ���*/
}t_LINKCLOSEREQ;

/*��·�ͷ�*/
typedef struct
{
    HLINKHANDLE linkHandle;
}t_LINKRELEASE;


/*-------------------------------------------------------------------------
API ����
-------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*  .h*/
