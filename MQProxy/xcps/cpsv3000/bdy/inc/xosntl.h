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
                  包含头文件
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
                  宏定义
--------------------------------------------------------------*/
/*简单的流控*/
#define  NTL_SHOW_CMD_CTL(x) ((x)%20 == 0)? (XOS_Sleep(5)) : XOS_UNUSED(x)

#define TCP_CLIENTS_PER_SERV    100    /*一个tcp server 平均用接入多少客户端*/
#define FDS_PER_THREAD_POLLING  256   /* 一个任务监视的fd 个数*/
#define FDS_MAX_THREAD_POLLING  1024   /* 一个任务监视的最大fd 个数*/
#define MAX_UDP_POLL_THREAD     10     /* udp支持最大线程数*/
#define MAX_TCP_CLI_POLL_THREAD 4     /* tcp  客户端支持最大线程数*/
#define NTL_TSK_NAME_LEN        20

/*超时重联时间*/
#define TCP_CLI_RECONNECT_INTERVAL   4000    /*4Sec 2007-08-ComCore adjust to 10Sec*/
/*超时重联的次数*/
#define TCP_CLI_RECONNEC_TIMES       5        /* 5次*/

#define NTL_SUBMSG_OLD_SOCK          1        /*for reconnect to keep old socket reconnect*/
#define NTL_SUBMSG_NEW_SOCK          2        /*for submsg to create new tcp client socket*/

/*select 超时时间*/
#ifdef XOS_LINUX
#define POLL_FD_TIME_OUT             5      /*5 mSec */
#else
#define POLL_FD_TIME_OUT             200      /*5 mSec */
#endif

/*NTL 接收任务的栈大小*/
#define NTL_RECV_TSK_STACK_SIZE     1024000    /*2007-09-10 changed to doubled from 30000 to 60000*/
                                              /*2008-07-28 changed to doubled from 60000 to 102400*/
/*set 集相关的信息*/
typedef struct
{
    t_SOCKSET     readSet;    /*read  集*/
    t_SOCKSET     writeSet;   /*write set */
    t_XOSMUTEXID  fdSetMutex; /*这里集合都会多线程访问,故采用互斥控制*/
    XU16          sockNum;    /* 集合中sock的数量*/
}t_FDSETINFO;

/*任务相关的信息*/
typedef struct
{
    t_FDSETINFO     setInfo;     /*fdset 的信息,一个接收线程都会访问一个fdset*/
    t_XOSSEMID      taskSemp;    /*驱动线程阻塞的事件*/
    t_XOSTASKID     taskId;      /* 任务id*/
    XBOOL           activeFlag;  /*线程活动的标志,调试用*/
}t_NTLTSKINFO;

/*网络传输层(NTL)的通用配置, NTL 通过
读配置文件启动*/
typedef struct
{
    XU16 maxUdpLink;             /*支持的最多使用的udp链路数量*/
    XU16 maxTcpCliLink;          /*支持的最多使用的tcp client 数量*/
    XU16 maxTcpServLink;         /*支持最多使用的tcp server 的数量*/
#ifdef XOS_SCTP
    XU16 maxSctpCliLink;         /*支持的最多使用的sctp client 数量*/
    XU16 maxSctpServLink;        /*支持最多使用的sctp server 的数量*/
    XU16 sctpClientsPerServ;     /*每个sctp server支持的客户端接入数量*/

    XU32 hb_interval;            /*sctp链路心跳间隔时间(ms)*/
    XU16 rto_min;                /*sctp数据包第一次重发(ms)*/
    XU16 rto_init;               /*sctp数据包第二次重发(ms)*/
    XU16 sack_timeout;           /*sack延时发送时间*/
#endif
    /*当支持的数量为零时，表示不支持此种链路服务*/
    
    XU16 fdsPerThrPolling;  /*一个子线程轮询描述符的数量*/
} t_NTLGENCFG;

/*ipc管理模块的全局管理结构*/
typedef struct
{
    XBOOL isInited;              /*是否初始化*/

    XOS_HARRAY udpLinkH;         /*存放udp链路信息的句柄*/
    t_XOSMUTEXID udpLinkMutex;   /*udp 链路信息句柄*/
    XOS_HARRAY tcpClientLinkH;   /*tcp 客户端忙闲链句柄*/
    t_XOSMUTEXID tcpClientLinkMutex; /*tcp 客户端链路互斥变量*/
    XOS_HARRAY tcpServerLinkH;   /*tcp server 忙闲链句柄*/
    t_XOSMUTEXID tcpServerLinkMutex; /*tcp server 忙闲链互斥变量*/

    /*tcp 接入的客户端，会在不同的线程中修改，应当加互斥保护*/
    XOS_HHASH   tSerCliH;        /*tcp server 接入的客户端*/
    t_XOSMUTEXID hashMutex;      /*互斥琐*/

    /*各任务相关的数据*/
    t_NTLTSKINFO   tcpServTsk;     /*tcp server tsk ,只扫描有没有客户接入进来*/
    XS32           servTskNo;      /*tcp  serv 任务的个数 */
    t_NTLTSKINFO*  pUdpTsk;        /*udp Task*/
    XS32           udpTskNo;       /*udp 任务的个数*/
    t_NTLTSKINFO*  pTcpCliTsk;     /*tcp Task*/
    XS32           tcpCliTskNo;    /*tcp client 的任务个数*/

    XBOOL          isGenCfg;       /*是否做了通用配置*/
    t_NTLGENCFG    genCfg;         /*通用配置数据*/
}t_NTLGLOAB;

/*-------------------------------------------------------------
                  结构和枚举声明
--------------------------------------------------------------*/
/*链路状态*/
typedef enum
{
    eNullLinkState,        /*状态空*/
    eStateInited,          /*已经初始化过*/
    eStateStarted,         /*已经启动过*/
    eStateConnecting,      /*正在连接状态*/
    eStateConnected,       /*连接已经建立，可以发送数据的状态*/
    eStateListening ,      /*监听状态*/
    eStateWaitClose,       /*等待关闭状态，支持半关闭时用到*/

    eMaxLinkState
}e_LINKSTATE;

/*20080710 add below*/
typedef struct  t_ResndDataReq t_RsndData;
struct  t_ResndDataReq
{
    XCHAR *pData;              /*数据包*/
    XU32   msgLenth;           /*缓冲区总长度*/
    XU32   i_offset;           /*缓冲区发送偏移值*/
    t_RsndData   *pNextPacket; /*后一个连接进来的客户端*/
};
typedef struct 
{
    XU32  rsnd_total;
    XU32  rsnd_wait;
    XU32  rsnd_success;
    XU32  rsnd_delete;
    XU32  rsnd_fail;
    XU32  rsnd_size;
    t_RsndData *pFirstDataReq;  /*没发送成功的数据DataReq的指针20080710add*/
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

/*tcp server 接进来的客户端信息*/
typedef struct  tcpServCliCb t_TSCLI;

/*tcp server 链路控制块*/
typedef struct
{
    HAPPUSER     userHandle;       /*用户句柄*/
    t_XOSUSERID      linkUser;     /*链路的业务使用者*/
    t_XINETFD     sockFd;          /*sock 描述符*/
    e_LINKSTATE  linkState;        /*链路的状态*/
    
    HLINKHANDLE linkHandle;        /*链路句柄，为了在数据指示时
    能快速而设置的冗余*/

    XOS_TLIncomeAuth  authFunc;    /*接入认证函数*/
    XVOID *pParam;                 /*认证函数的指针*/
    XU16 maxCliNum;                /*最多可以接入的客户端数量*/
    XU16 usageNum;                 /*已经接入客户端的数量*/
    t_TSCLI *pLatestCli;           /*最近接入的客户端控制快指针*/
    t_IPADDR myAddr;               /*本端地址*/
}t_TSCB;

struct tcpServCliCb
{
    t_XINETFD     sockFd;         /*sock 描述符*/
    t_TSCB*       pServerElem;    /*指向tcp  server 存储信息的指针*/
    t_TSCLI       *pPreCli;       /*前一个连接进来的客户端*/
    t_TSCLI       *pNextCli;      /*后一个连接进来的客户端*/
    t_ResndPacket   packetlist;   /*最近接入的未发送成功的包控制快20080710 add*/
    //  t_IPADDR    peerAddr;       /*对应客户端的ip地址*/  /*作为key了*/
};

/*udp 链路控制块*/
typedef struct
{
    HAPPUSER     userHandle;        /*用户句柄*/
    t_XOSUSERID      linkUser;      /*链路的业务使用者*/
    t_XINETFD     sockFd;           /*sock 描述符*/
    t_IPADDR      peerAddr;         /*对端的地址*/
    t_IPADDR      myAddr;           /*本端地址*/
    e_LINKSTATE  linkState;         /*链路的状态*/
    HLINKHANDLE linkHandle;         /*链路句柄，为了在数据指示时能
    快速而设置的冗余*/
}t_UDCB;

/*tcp client 链路控制块*/
typedef struct
{
    HAPPUSER        userHandle;     /*用户句柄*/
    t_XOSUSERID     linkUser;       /*链路的业务使用者*/
    t_XINETFD       sockFd;         /*sock 描述符*/
    HLINKHANDLE     linkHandle;     /*链路句柄，为了在数据指示时能
    快速而设置的冗余*/
    e_LINKSTATE     linkState;      /*链路状态*/
    t_IPADDR        peerAddr;       /*对端地址*/
    t_IPADDR        myAddr;         /*本端地址*/
    PTIMER          timerId;        /*定时器的句柄*/
    XS32            expireTimes;    /*定时器超时次数*/
    t_ResndPacket   packetlist;   /*最近接入的未发送成功的包控制快20080710 add*/
}t_TCCB;

/*用于链路资源查询*/
typedef struct
{
    t_XOSUSERID *linkUser; /*链路的业务使用者*/
    HAPPUSER userHandle;  /*用户句柄*/
    e_LINKTYPE  linkType;    /*服务链路的类型*/
}t_Link_Index;

#if 0 /*改成读配置文件启动*/
/*通用配置消息，整个ntl 模块只能执行一次通用配置。
在执行了通用配置消息后，以后收到的通用配置消息无效*/
typedef struct
{
    XU16 maxUdpLink;        /*支持的最多使用的udp链路数量*/
    XU16 maxTcpCliLink;     /*支持的最多使用的tcp client 数量*/
    XU16 maxTcpServLink;    /*支持最多使用的tcp server 的数量*/
    
    /*当支持的数量为零时，表示不支持此种链路服务*/
    
    XU16 fdsPerThrPolling;  /*一个子线程轮询描述符的数量*/
    
}t_NTLGENCFG;
#endif

//20081019定义socket异常关闭方向定义
#define NTL_SHTDWN_RECV   0x0001  /* Read file descriptor group list */
#define NTL_SHTDWN_SEND   0x0002  /* Write file descriptor group list */
#define NTL_SHTDWN_BOTH   (NTL_SHTDWN_RECV | NTL_SHTDWN_SEND)  


/*-------------------------------------------------------------------------
                接口函数
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
函数名:NTL_timerProc
功能:  ntl 模块定时器消息处理函数入口
输入:  pMsg －消息指针
输出:
返回: 成功返回XOS_XSUCC , 失败返回XERROR
说明:
************************************************************************/
XS8 NTL_timerProc( t_BACKPARA* pParam);

/************************************************************************
函数名:NTL_msgProc
功能:  ntl 模块消息处理函数入口
输入:  pMsg －消息指针
输出:
返回: 成功返回XOS_XSUCC , 失败返回XERROR
说明: 此消息处理函数是ntl主任务消息入口，edataSend
消息不在此函数处理的范围内
************************************************************************/
XS8 NTL_msgProc(XVOID* pMsgP, XVOID*sb );

/************************************************************************
函数名:NTL_Init
功能: 出世化ntl模块
输入:
输出:
返回:
说明: 注册到模块管理中
************************************************************************/
XS8 NTL_Init(void*p1, void*p2);
XS8 NTL_TelnetCliCloseSock(t_IPADDR ipAddr);
/************************************************************************
函数名:NTL_channel_find_function
功能: 根据用户参数查找通道是否已分配
输出:
返回: 成功返回XTRUE,失败返回XERROR
说明:
************************************************************************/
XBOOL NTL_channel_find_function(XOS_ArrayElement element1, XVOID *param);

/************************************************************************
函数名:NTL_StopLinkByReapply
功能:  因重复申请链路而停止原来的链路
输入:  pCloseReq －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_ResetLinkByReapply(t_LINKCLOSEREQ* pCloseReq);

/************************************************************************
函数名:NTL_ResetLinkByReapplyEntry
功能:  因重复申请链路而停止原来的链路
输入:  pCloseReq －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_ResetLinkByReapplyEntry(e_LINKTYPE linkType, t_LINKINITACK *linkInitAck, HAPPUSER *pUserHandle, XS32 *pnRtnFind);

/************************************************************************
函数名:NTL_CloseUdpSocket
功能:  关闭udp的socket，并重置读集
输入:  pUdpCb －udp控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: 此函数的udp控制块需要调用方进行全局保护
************************************************************************/
XS16 NTL_CloseUdpSocket(t_UDCB *pUdpCb, XU32 taskNo);

/************************************************************************
函数名:NTL_CloseTcpServerSocket
功能:  关闭tcpserver
输入:  pTcpServCb －tcpserver控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: tcpserver需要调用方的全局保护
************************************************************************/
XS32 NTL_CloseTcpServerSocket(t_TSCB *pTcpServCb);

/************************************************************************
函数名:NTL_StartTcpClientTimer
功能:  启动tcp客户端的重启定时器
输入:  pTcpCb －tcp控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_StartTcpClientTimer(t_TCCB *pTcpCb, XU32 taskNo);

/************************************************************************
函数名:NTL_SetTcpClientFd
功能:  tcp client select 置位操作
输入:  pTcpCb －消息指针
       fdFlg  --0:读,1:写
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/            
void NTL_SetTcpClientFd(t_TCCB *pTcpCb, XU32 taskNo, e_ADDSETFLAG fdFlg);

/************************************************************************
函数名:NTL_StopTcpClientTimer
功能:  停止tcp客户端的定时器
输入:  pTcpCb －tcp控制块指针
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明:
************************************************************************/
XS32 NTL_StopTcpClientTimer(t_TCCB *pTcpCb);

/************************************************************************
函数名:NTL_SetUdpSelectFd
功能:  udp select 置位操作
输入:  pUdpCb －udp控制块
       taskNo  --任务号
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/    
void NTL_SetUdpSelectFd(t_UDCB *pUdpCb, XU32 taskNo);

/************************************************************************
函数名:NTL_DeleteCB
功能:  释放控制块
输入:  linkType －链路类型
       linkIndex--链路索引

输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_DeleteCB(e_LINKTYPE linkType, int linkIndex);

/************************************************************************
函数名:NTL_StartUdpLink
功能:  启动udp链路
输入:    pDatasrc －消息源指针
        pLinkStart--链路启动指针

输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_StartUdpLink(t_XOSUSERID *pDatasrc, t_LINKSTART *pLinkStart);

/************************************************************************
函数名:NTL_ResetAllTcpFd
功能:  清理所有的tcp读写集
输入:  taskNo  任务号
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_ResetAllTcpFd(XU32 taskNo);



/*2007/12/05增加通讯模块发送数据报错误原因打印*/
XS8* NTL_getErrorTypeName(XS32 reason_code, XCHAR *pneterror_name, int nLen);
/*2007/12/05增加通讯模块发送数据报错误原因打印*/
XS8* NTL_getLinkStateName(XS32 link_state, XCHAR *pLinkstate_name, int nLen);


#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLISHELL_H_ */

