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
                  包含头文件
-------------------------------------------------------------------------*/
#include "xostype.h"

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
//#define  aaa

XOS_DECLARE_HANDLE(HAPPUSER);   /*用户链路控制句柄*/
XOS_DECLARE_HANDLE(HLINKHANDLE); /*传输层链路句柄*/

/*-------------------------------------------------------------------------
                                 结构和枚举声明
-------------------------------------------------------------------------*/


/************************************************************************
函数名: XOS_TLIncomeAuth
功能：接入认证函数
输入：
incomeAddr  -- 接入的ip地址
param          -- 接入的扩展参数
输出：
返回：认证成功返回XTRUE, 失败返回XFALSE
说明：当用户不需要接入认证时,不用实现此函数
************************************************************************/
typedef XBOOL (*XOS_TLIncomeAuth)( HAPPUSER userHandle, t_IPADDR * incomeAddr, XVOID* param);


/************************************************************************
函数名: XOS_SctpIncomeAuth
功能：接入认证函数
输入：
incomeAddr  -- 接入的ip地址池
param       -- 接入的扩展参数
输出：
返回：认证成功返回XTRUE, 失败返回XFALSE
说明：当用户不需要接入认证时,不用实现此函数
************************************************************************/
typedef XBOOL (*XOS_SctpIncomeAuth)( HAPPUSER userHandle, t_SCTPIPADDR * incomeAddr, XVOID* param);



/*传输层和上层交互的消息类型*/
typedef enum
{
    eMinTlMsg,
    eLinkInit ,                /*初始化链路            APP->NTL  */
    eInitAck,                   /*链路初始化确认    NTL ->APP */
    eLinkStart,            /*启动链路                   APP->NTL */
    eStartAck,             /*链路启动确认        NTL ->APP */
    eSendData,             /*发送数据                   APP->NTL */
    eErrorSend,               /* 数据发送错误        NTL ->APP*/
    eConnInd,                    /*连接指示                        NTL ->APP */
    eDataInd,            /*收到数据                   NTL ->APP*/
    eLinkStop,            /*关闭链路                   APP->NTL */
    eStopInd,                  /* 链路关闭指示        NTL ->APP*/
    eLinkRelease,              /*释放链路                   APP->NTL*/

    /*sctp*/
    eSctpInitAck,                   /*链路初始化确认    NTL ->APP */
    eSctpStartAck,             /*链路启动确认        NTL ->APP */
    eSctpErrorSend,               /* 数据发送错误        NTL ->APP*/
    eSctpDataInd,            /*收到数据                   NTL ->APP*/
    eSctpConnInd,                    /*连接指示             NTL ->APP */
    eSctpStopInd,                  /* 链路关闭指示        NTL ->APP*/
    
    /*待扩展*/
    eStaticReq,                 /*链路信息统计请求   APP->NTL*/
    eStaticAck,                 /*链路统计信息确认   NTL ->APP*/
    
    eMaxTlMsg
}e_TLMSG;


/*服务链路的类型定义*/
typedef enum 
{
    eNullLinkType,
    eUDP,
    eTCPClient,
    eTCPServer,
    eSCTPClient,
    eSCTPServer,
    /*裸层ip协议可以在此扩展,目前只支持UDP,TCP,SCTP*/
    eSCTP, /*此类型已废弃，实现为eSCTPClient、eSCTPServer，此值用于兼容原有代码*/
    ePCI,
    eOtherLinkType
}e_LINKTYPE;

/*确认结果类型*/
typedef enum
{
    eNullResult,
    eSUCC,        /*成功*/
    eFAIL,         /*失败*/
    eBlockWait, /*连接等待中*/
    eReapply,   /*资源重复申请*/
    eInvalidInstrm,    /*对端设置的入流数太小，非法*/
    eOtherResult
}e_RESULT;

/*错误类型*/
typedef enum
{
    eErrorNetInterrupt,   /*网络中断*/
    eErrorNetBlock,    /*网络阻塞*/
    eErrorDstAddr,  /*目标地址错误,没有配置对端地址或者*/
    eErrorOverflow, /*数据溢出，数据长度太长等*/    
    eErrorLinkClosed,   /*对端关闭连接*/
    eErrorLinkState,    /*链路状态不对,没有启动或关闭了*/

    eErrorStreamNum,        /*sctp,发送数据指定流号错误*/
    /*待扩展,便于上层的错误处理*/
    
    eOtherErrorReason
}e_ERRORTYPE;  /*错误的原因的类型*/

/*关闭的原因*/
typedef enum 
{
    eLinkReuse, /*链路重复使用，（重复配置或抢占）*/
    ePeerReq,  /*对端请求*/
    eNetError,  /*网络错误*/
    
    /*待扩展,便于上层的处理*/
    eOtherCloseReason
}e_CLOSERESEASON;

/*添加读写集的标志位,liukai 2013.10.15*/
typedef enum
{
    eRead = 0,
    eWrite,
    eReadWrite
}e_ADDSETFLAG;

/*链路控制字*/
typedef enum
{
    eNullLinkCtrl,
    e16kUdpPkt,                  /*udp 链路控制字, 打开后支持16k大小的udp包*/
    eCompatibleTcpeen,     /*tcp 链路控制字, 打开后兼容tcp封装,
                                     每次最多收1900个字节*/
                                     /*其他的待扩展*/
    eMaxLinkCtrl
}e_LINKCTRL;


/*为了写代码方便，定义成类型*/
/*udp 启动需要的参数*/
typedef struct 
{
    t_IPADDR myAddr; /*本端的地址*/
    t_IPADDR peerAddr; /*对端的地址*/
} t_UDPSTART;

/*tcp 的客户端链路启动*/
typedef    struct 
{
    t_IPADDR myAddr ; /*本端的地址*/
    t_IPADDR peerAddr; /*对端地址*/
    XS32  recntInteval;  /*超时重联的间隔，0表示不重联*/
}t_TCPCLISTART ;  

/*tcp 的服务器端*/
typedef  struct 
{
    t_IPADDR myAddr;/*本端的地址*/
    XU32 allownClients; /*容许接入客户端的数量*/
    XOS_TLIncomeAuth  authenFunc; /*接入认证函数指针， NULL表示不认证*/
    void *pParam;                 /*认证函数的参数*/
} t_TCPSERSTART;

/*sctp 的客户端链路启动*/
typedef    struct 
{
    t_SCTPIPADDR myAddr ; /*本端的地址*/
    t_SCTPIPADDR peerAddr; /*对端地址*/
    XU16  pathmaxrxt;  /*路径断链检测伐值，用户如不关心则填0*/
    XU16  streamNum;      /*sctp客户端的出入流数量，用户如不关心则填0*/
    XU32  hbInterval;       /*心跳间隔时间，单位毫秒，用户如不关心则填0*/
}t_SCTPCLISTART;

/*sctp 的服务器端*/
typedef  struct
{
    t_SCTPIPADDR myAddr;            /*本端的地址*/
    XU16 allownClients;             /*容许接入客户端的数量*/
    XU16 streamNum;                 /*sctp服务端的出入流数量，用户如不关心则填0*/
    XU16  pathmaxrxt;               /*路径断链检测伐值，用户如不关心则填0*/
    XU32 hbInterval;                /*心跳间隔时间，单位毫秒，用户如不关心则填0*/
    XOS_SctpIncomeAuth  authenFunc; /*接入认证函数指针， NULL表示不认证*/
    void *pParam;                   /*认证函数的参数*/
} t_SCTPSERSTART;

/*链路启动的参数*/
typedef union
{   
    t_UDPSTART udpStart; 
    t_TCPCLISTART tcpClientStart;
    t_TCPSERSTART tcpServerStart;
    t_SCTPCLISTART sctpClientStart;   
    t_SCTPSERSTART sctpServerStart;    
} u_LINKSTART;

/*链路初始化*/
typedef struct
{ 
    e_LINKTYPE  linkType;    /*服务链路的类型*/
    e_LINKCTRL  ctrlFlag;     /*链路控制字,用来扩展业务功能*/
    HAPPUSER  appHandle;   /*用户句柄，通常指索引，以后该链路
                              的所有的上行消息都会带这个标识*/
}t_LINKINIT;

/*链路初始化确认*/
typedef struct
{
    HAPPUSER appHandle;    /* 应用层句柄，配置时由应用层传入的*/
    e_RESULT  lnitAckResult;  /*初始化链路的结果，
                           当结果不为eSUCC时，以下的字段无效*/
    HLINKHANDLE linkHandle; /*链路句柄，以后该链路所有的下行
                           消息中都必须添加此标识*/
}t_LINKINITACK;

/*链路启动*/
typedef struct
{
    HLINKHANDLE linkHandle; /*链路句柄，在初始化链路时返回的*/
    u_LINKSTART  linkStart;   /*链路启动参数*/
}t_LINKSTART;

/*链路启动确认*/
typedef struct 
{
    HAPPUSER  appHandle;     /* 应用层句柄，配置时由应用层传入的*/
    e_RESULT   linkStartResult;    /*  链路启动确认的结果*/
    t_IPADDR   localAddr;       /*该链路实际通信端口*/
}t_STARTACK;

/*sctp链路启动确认*/
typedef struct 
{
    HAPPUSER  appHandle;     /* 应用层句柄，配置时由应用层传入的*/
    e_RESULT   linkStartResult;    /*  链路启动确认的结果*/
    t_SCTPIPADDR   localAddr;       /*该链路实际通信端口*/
}t_SCTPSTARTACK;

/*数据发送*/
typedef struct
{
    HLINKHANDLE linkHandle; /*链路句柄，在初始化链路时返回的*/
    t_IPADDR   dstAddr;    /*发送目的地址，当目的地址为空时，
                            使用链路启动时的配置的对端地址，
                            TCPserver 发送数据时此项不能为空*/
    XU32  msgLenth;         /*上层要发送消息的长度*/
    XCHAR* pData;            /*要发送的数据头指针*/ 
}t_DATAREQ;

/*SCTP数据发送相关属性*/
typedef struct
{
    XU32  ppid;             /*净荷协议标识符，可选，默认0*/
    XU16 stream;            /*指定发送数据的流号，可选，默认0*/
    XU32 context;           /*上下文，可选，默认0*/
}t_SctpDataAttr;
/*SCTP数据发送*/
typedef struct
{
    HLINKHANDLE linkHandle; /*链路句柄，在初始化链路时返回的*/
    t_IPADDR   dstAddr;    /*发送数据时此项可以为空*/
    XU32  msgLenth;         /*上层要发送消息的长度*/
    XCHAR* pData;            /*要发送的数据头指针*/ 
    t_SctpDataAttr attr;    /*数据发送相关参数，如果用户关心则置0,不可使用随机值*/
}t_SCTPDATAREQ;

/*数据发送失败指示*/
typedef struct
{
    HAPPUSER        userHandle;
    e_ERRORTYPE   errorReson;  /*数据发送错误原因*/
    //XCHAR*     pData;    /*要发送的数据头指针*/
}t_SENDERROR; 

/*SCTP数据发送失败指示*/
typedef struct
{
    HAPPUSER        userHandle;
    e_ERRORTYPE   errorReson;  /*数据发送错误原因*/
    t_IPADDR peerIp;    /*发送数据的目的地址*/
}t_SENDSCTPERROR; 

/*收到数据指示*/
typedef struct
{
    HAPPUSER appHandle;
    t_IPADDR peerAddr; /*对端地址*/
    XU32 dataLenth ;  /*接收数据长度*/
    XCHAR* pData;  /*接收到数据的头指针,需要用户释放*/
}t_DATAIND;

/*sctp收到数据指示*/
typedef struct
{
    HAPPUSER appHandle;
    t_IPADDR peerAddr; /*对端地址*/
    XU32 dataLenth ;  /*接收数据长度*/
    XCHAR* pData;  /*接收到数据的头指针,需要用户释放*/
    t_SctpDataAttr attr;    /*数据接收相关参数*/
}t_SCTPDATAIND;

/*连接指示,只有tcp server才有效*/
typedef struct
{
    HAPPUSER appHandle;
    t_IPADDR  peerAddr;   /*连接进来的客户端地址*/
}t_CONNIND;

/*链路关闭指示*/
typedef struct
{
    HAPPUSER appHandle;
    e_CLOSERESEASON closeReason;
    t_IPADDR peerAddr; /*tcp server 用此来指示哪个接入的客户端断开了*/
}t_LINKCLOSEIND;

/* 关闭链路请求*/
typedef struct
{
    HLINKHANDLE linkHandle;
    t_IPADDR  cliAddr;            /*如果是tcp server，指定该地址就是关闭指定的客户端;
                                  当ip和port 都是0 的时候，关闭所有的客户端*/
}t_LINKCLOSEREQ;

/*链路释放*/
typedef struct
{
    HLINKHANDLE linkHandle;
}t_LINKRELEASE;


/*-------------------------------------------------------------------------
API 声明
-------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*  .h*/
