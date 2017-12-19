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
                  包含头文件
--------------------------------------------------------------*/
#include "xostype.h"
#include "xostl.h"
#include "xosinet.h"
#include "xosmodule.h"
#include "xosarray.h"
#include "xosntl.h"
#include "sctp_win.h"
/*-------------------------------------------------------------
                  宏定义
--------------------------------------------------------------*/
#define DEFAULT_SCTP_STREAM          1       /*sctp连接的默认最小出流数量*/
#define SCTP_MAX_STREAM      10       /*sctp连接的最大出流数量*/
#define SCTP_MIN_HB_INTERVAL          100       /*sctp服务端最小心跳间隔，单位毫秒*/
#define SCTP_DEFAULT_HB_INTERVAL     2000    /*sctp默认心跳间隔时间，2000ms*/

/*超时重联时间*/
#ifndef SCTP_CLI_RECONNECT_INTERVAL
#define SCTP_CLI_RECONNECT_INTERVAL   4000 
#endif

/*超时重联的次数*/
#ifndef SCTP_CLI_RECONNEC_TIMES
#define SCTP_CLI_RECONNEC_TIMES       5        /* 5次*/
#endif

#ifndef MAX_SCTP_CLI_POLL_THREAD
#define MAX_SCTP_CLI_POLL_THREAD    4     /* sctp客户端支持最大线程数*/
#endif

#ifndef SCTP_CLIENTS_PER_SERV
#define SCTP_CLIENTS_PER_SERV    10    /*一个sctp server 平均用接入多少客户端*/
#endif

#ifndef TEMP_STRING_LEN
#define TEMP_STRING_LEN         32
#endif

/*-------------------------------------------------------------
                  结构和枚举声明
--------------------------------------------------------------*/

/*sctp server 接进来的客户端信息*/
typedef struct  sctpServCliCb t_SSCLI;
/*sctp server 链路控制块*/
typedef struct
{
    HAPPUSER     userHandle;       /*用户句柄*/
    t_XOSUSERID      linkUser;     /*链路的业务使用者*/
    e_LINKSTATE  linkState;        /*链路的状态*/
    XU16         instance;          /*sctp链路句柄*/
    
    HLINKHANDLE linkHandle;        /*链路句柄，为了在数据指示时
    能快速而设置的冗余*/

    XU32 maxCliNum;                /*最多可以接入的客户端数量*/
    XU16 usageNum;                 /*已经接入客户端的数量*/
    XU16 maxStream;                 /*sctp连接的最大出流数量*/
    XU32 hbInterval;                 /*sctp服务端心跳间隔*/
    t_SSCLI *pLatestCli;           /*最近接入的客户端控制快指针*/
    t_SCTPIPADDR myAddr;               /*本端地址*/
}t_SSCB;

struct sctpServCliCb
{
    XU32        assocID;            /*偶联ID*/
    t_IPADDR    destAddr;       /*连到服务端的客户端的主地址*/
    t_SSCB*       pServerElem;    /*指向sctp  server 存储信息的指针*/
    t_SSCLI       *pPreCli;       /*前一个连接进来的客户端*/
    t_SSCLI       *pNextCli;      /*后一个连接进来的客户端*/
};

/*sctp client 链路控制块*/
typedef struct
{
    HAPPUSER        userHandle;     /*用户句柄*/
    t_XOSUSERID     linkUser;       /*链路的业务使用者*/
    XU16         instance;          /*sctp链路句柄*/
    XU32            assocID;            /*偶联ID*/
    HLINKHANDLE     linkHandle;     /*链路句柄，为了在数据指示时能
    快速而设置的冗余*/
    e_LINKSTATE     linkState;      /*链路状态*/
    t_SCTPIPADDR        peerAddr;       /*对端地址*/
    t_SCTPIPADDR        myAddr;         /*本端地址*/
    XU16 maxStream;                 /*sctp连接的最大出入流数量*/
}t_SCCB;

/*管理模块的全局管理结构*/
typedef struct
{
    XBOOL isInited;              /*是否初始化*/

    XOS_HARRAY sctpClientLinkH;   /*sctp 客户端忙闲链句柄*/
    t_XOSMUTEXID sctpClientLinkMutex; /*sctp 客户端链路互斥变量*/
    XOS_HARRAY sctpServerLinkH;   /*sctp server 忙闲链句柄*/
    t_XOSMUTEXID sctpServerLinkMutex; /*sctp server 忙闲链互斥变量*/
    
    /*sctp 接入的客户端，会在不同的线程中修改，应当加互斥保护*/
    XOS_HHASH   tSctpCliH;        /*sctp server 接入的客户端*/
    t_XOSMUTEXID hashMutex;      /*互斥琐*/

    XBOOL          isGenCfg;       /*是否做了通用配置*/
    t_NTLGENCFG    genCfg;         /*通用配置数据*/
}t_SCTPGLOAB;

/*-------------------------------------------------------------------------
                接口函数
-------------------------------------------------------------------------*/
/************************************************************************
函数名:SCTP_genCfgProc
功能:  处理通用配置消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: SCTP 在收到通用配置消息后启动各子任务，完成各
子任务的初始化。
************************************************************************/
XS32 SCTP_genCfgProc(t_NTLGENCFG* pGenCfg);

/************************************************************************
函数名:SCTP_channel_find_function
功能: 根据用户参数查找通道是否已分配
输出:
返回: 成功返回XTRUE,失败返回XERROR
说明:
************************************************************************/
XBOOL SCTP_channel_find_function(XOS_ArrayElement element1, XVOID *param);

/************************************************************************
函数名:SCTP_closeTsCli
功能: 关闭一个sctp server 接入的客户端
输入:pSctpServCli －server client 的控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_closeTsCli(t_SSCLI* pSctpServCli);

/************************************************************************
函数名:SCTP_CloseSctpServerSocket
功能:  关闭sctpserver
输入:  pSctpServCb －sctpserver控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: sctpserver需要调用方的全局保护
************************************************************************/
void SCTP_CloseSctpServerSocket(t_SSCB *pSctpServCb);

/************************************************************************
函数名:SCTP_closeReqForRelease
功能:  处理链路释放请求消息--只能由SCTP_linkReleaseProc调用
输入:  pMsg －消息指针
输出:
返回:  成功返回XSUCC,否则返回XERROR
说明:  此函数与SCTP_closeReqProc函数在sctpserver上处理不同
************************************************************************/
XS32 SCTP_ReleaseLink(t_XOSCOMMHEAD* pMsg);

/************************************************************************
函数名:SCTP_closeReqProc
功能:  处理链路关闭请求消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_closeReqProc(t_XOSCOMMHEAD* pMsg);

/************************************************************************
函数名:SCTP_dataReqProc
功能: 将数据发送到网络
输入: pMsg  －数据发送的指针
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明:
************************************************************************/
XS32 SCTP_dataReqProc(t_XOSCOMMHEAD *pMsg);

/************************************************************************
函数名:SCTP_ResetLinkByReapplyEntry
功能:  因重复申请链路而停止原来的链路
输入:  e_LINKTYPE linkType      -连接类型
        HAPPUSER *pUserHandle   -连接的用户句柄
        XS32 *pnRtnFind         -控制块的索引值
输出:   t_LINKINITACK *linkInitAck  -处理结果
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_ResetLinkByReapplyEntry(e_LINKTYPE linkType, t_LINKINITACK *linkInitAck, HAPPUSER *pUserHandle, XS32 *pnRtnFind);

/************************************************************************
函数名:SCTP_CloseServerSocket
功能:  关闭sctpserver
输入:  pServCb －sctpserver控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: sctpserver需要调用方的全局保护
************************************************************************/
void SCTP_CloseServerSocket(t_SSCB *pServCb);

/************************************************************************
函数名:SCTP_DeleteCB
功能:  释放控制块
输入:  linkType －链路类型
       linkIndex--链路索引

输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_DeleteCB(e_LINKTYPE linkType, XS32 linkIndex);

/************************************************************************
函数名:SCTP_linkInitProc
功能:  处理链路初始化消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_linkInitProc(t_XOSCOMMHEAD* pMsg);

/************************************************************************
函数名:SCTP_linkStarttProc
功能:  处理链路启动消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_linkStartProc(t_XOSCOMMHEAD* pMsg);

/************************************************************************
函数名:SCTP_linkReleaseProc
功能:  处理链路释放消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_linkReleaseProc(t_XOSCOMMHEAD* pMsg);

/************************************************************************
函数名:SCTP_RestartLink
功能:  重新连接sctp链接
输入:   t_SCCB* sctpCliCb
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 SCTP_RestartLink(t_SCCB* sctpCliCb);

/************************************************************************
函数名:SCTP_findClient
功能:  查找sctp 连接的客户
输入:
pTserverCb －server cb的指针
pClientAddr  － 接进来客户的地址指针
输出:
返回: 成功返回控制块指针,否则返回xnullp
说明:
************************************************************************/
t_SSCLI * SCTP_findClient(t_SSCB *pTserverCb,t_IPADDR *pClientAddr);

XS32 SCTP_init();

/************************************************************************
函数名:SCTP_CliCfgShow
功能: 显示sctp client 配置信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID SCTP_CliCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

/************************************************************************
函数名:SCTP_ServCfgShow
功能: 显示sctp client 配置信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID SCTP_ServCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

/************************************************************************
函数名:SCTP_ServCfgShow
功能: 显示sctp client 配置信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID SCTP_msgQueueShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

#endif

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _XOS_SCTP_H_ */

