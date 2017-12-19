/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosinet.h
**
**  description:  for windows
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
**   wulei         2006.3.7              create
**************************************************************/
#ifndef _XOS_INET_H_
#define _XOS_INET_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "xostype.h"
#include "xostl.h"
/* environment dependent include files */
#ifdef XOS_WIN32
#ifndef IN
#define IN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef WIN2K
#include <Mswsock.h>
#endif /* WIN2K */
#else /* XOS_WIN32 */
#include <errno.h>

#include <string.h>
#include <sys/types.h>

#ifdef XOS_VXWORKS
#include <sys/times.h>
#include <ioLib.h>
#include <sockLib.h>
#include <selectLib.h>
#include <hostLib.h>
#else

#include <sys/select.h>
#include <sys/time.h>
#ifdef XOS_LINUX
#include <sys/uio.h>
#else
#include <sys/filio.h>
#endif /* XOS_LINUX */

#endif /* XOS_VXWORKS */

#endif /* XOS_WIN32 */

#if defined(XOS_SCTP) && defined(XOS_LINUX)
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/sctp.h>
#include "sctp.h"
#endif
/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/

#define INPUT_PAR_CHECK     1  /*用来控制入口参数检查*/

/*返回值类型*/
/*链路关闭*/
#define  XINET_CLOSE        2
/*- connection is in progress (only non-blocking)*/
#define  XINET_INPROGRESS   3
/*- connection is established (only non-blocking)*/
#define  XINET_ISCONN       4
/*select time out */
#define  XINET_TIMEOUT      5
/*packet size is too big */
#define  XINET_INVALID      6

/*超时重联的次数*/
#define TCP_RECONNECT_MAX   5  /* 5次*/

#define ERR_BADF            9

/*从xosinet.c中移至xosinet.h  (by zengjiandong)*/
#ifdef XOS_WIN32

#define INET_ERR             SOCKET_ERROR
#define INET_ERR_CODE        WSAGetLastError()
#define ERR_INPROGRESS       WSAEINPROGRESS
#define ERR_ISCONN           WSAEISCONN
#define ERR_WOULDBLOCK       WSAEWOULDBLOCK
#define ERR_INADDRNONE       INADDR_NONE
#define ERR_NOTCONN          WSAENOTCONN
#define ERR_ALREADY          WSAEALREADY
#define ERR_AGAIN            WSAEWOULDBLOCK
#define ERR_INVAL            WSAEINVAL
#define ERR_CONNREFUSED      WSAECONNREFUSED
#define ERR_PIPE             WSAENOTCONN
#define ERR_EINTR            WSAEINTR
/* Changed ERR_TIMEOUT for pSos compilation */
#define ERR_TIMEDOUT         WSAETIMEDOUT
#define ERR_CONNRESET        WSAECONNRESET
#define ERR_CONNABORTED      WSAECONNABORTED

#else

#define INET_ERR             -1
#define INET_ERR_CODE        errno
#define ERR_INPROGRESS       EINPROGRESS
#define ERR_ISCONN           EISCONN
#define ERR_WOULDBLOCK       EWOULDBLOCK
#define ERR_INADDRNONE       -1
#define ERR_NOTCONN          ENOTCONN
#define ERR_ALREADY          EALREADY
#define ERR_AGAIN            EAGAIN
/* EINVAL is not mapped because it is a valid error code here */
#define ERR_INVAL            0
#define ERR_EINTR            4
#define ERR_CONNREFUSED      ECONNREFUSED
#define ERR_PIPE             EPIPE
/* Changed ERR_TIMEOUT for pSos compilation */
#define ERR_TIMEDOUT         ETIMEDOUT
#define ERR_CONNRESET        ECONNRESET
#define ERR_CONNABORTED      ECONNABORTED

#endif /* XOS_WIN32 */

/*无效的sock*/
#ifdef XOS_WIN32
#define XOS_INET_INV_SOCKFD INVALID_SOCKET
#else
#define XOS_INET_INV_SOCKFD  -1
#endif

/*控制标志位*/
#define  XOS_INET_CTR_NOCOMPATI_TCPEEN    0   /*不兼容老平台的tcp封装*/
#define  XOS_INET_CTR_COMPATIBLE_TCPEEN   1   /*兼容老平台的tcp封装*/
#define  XOS_INET_CTR_RCV_CERTERIN_BYTES  2   /*接受固定大小数据*/
#define  XOS_INET_CTR_PEER_DATA           3   /*不影响数据接收,偷看数据*/

/* socket types */
#define XOS_INET_STREAM  SOCK_STREAM
#define XOS_INET_DGRAM   SOCK_DGRAM

/*容许同时接入的tcp 客户端的数量*/
#define MIN_BACK_LOG  1
#define MAX_BACK_LOG  8

/* socket options type */
#define XOS_INET_OPT_BLOCK         0  /*  阻塞选项 */
#define XOS_INET_OPT_REUSEADDR     1  /* 地址复用选项 */
#define XOS_INET_OPT_RX_BUF_SIZE   2  /* 接收缓冲区大小选项 */
#define XOS_INET_OPT_TX_BUF_SIZE   3  /* 发送缓冲区大小选项*/
#define XOS_INET_OPT_ADD_MCAST_MBR 4  /* 增加ip分组选项 */
#define XOS_INET_OPT_DRP_MCAST_MBR 5  /* 删除ip分组选项 */
#define XOS_INET_OPT_TCP_NODELAY   6  /* 区别tcp/ip协议栈供应商选项 */

#ifdef XOS_LINUX
#define XOS_INET_OPT_BSD_COMPAT    7  /* BSD compatible option for Linux */
#endif /* XOS_LINUX */

#define XOS_INET_OPT_MCAST_LOOP    8  /* 广播循环选项 */
#define XOS_INET_OPT_MCAST_IF      9  /* 指定本地数据发送的接口 */
#define XOS_INET_OPT_MCAST_TTL     14 /* 指定TTl 的指*/

#define XOS_INET_OPT_BROADCAST     23 /* 广播选项 */

#define XOS_INET_OPT_KEEPALIVE     24 /* 连接保持选项*/

#define XOS_INET_OPT_LINGER        25  /*立即结束tcp 状态。*/
#define XOS_INET_OPT_SCTP_EVENT    26  /* 设置sctp套接字event参数 */
#define XOS_INET_OPT_SCTP_OUTSTREAM   27  /* 设置sctp本端出流数量 */
#define XOS_INET_OPT_SCTP_HB       28  /* 设置sctp套接字心跳间隔 */
#define XOS_INET_OPT_SCTP_PEER_INSTREAM   29  /* 获取sctp对端入流数数量 */
#define XOS_INET_OPT_SCTP_ASSOCINFO   30  /* 设置sctp偶联断链检测阀值 */
#define XOS_INET_OPT_SCTP_PATHMAXRXT   31  /* 设置sctp路径链检测阀值 */
#define XOS_INET_OPT_SCTP_NODELAY    32  /* 设置sctp套接字nagle算法 */



/* socket 选项的值 */
#define XOS_INET_OPT_DISABLE       0   /* enable option */
#define XOS_INET_OPT_ENABLE        1   /* disable option */

/* 设置选项的层次 */
#define XOS_INET_LEVEL_SOCKET      SOL_SOCKET  /* 设置socket 这一层的选项 */
#define XOS_INET_LEVEL_IP          IPPROTO_IP  /* ip 层的选项*/
#define XOS_INET_LEVEL_TCP         IPPROTO_TCP /* TCP 的选项 */
#define XOS_INET_LEVEL_SCTP         IPPROTO_SCTP /* SCTP 的选项 */

#define XOS_INET_MAX_MSG_LEN  0x7fff  /* max length of a message */

/*兼容老平台tcp封装tcp数据的最大长度*/
#define XOS_INET_COMPATY_TCPEEN_DATA_LEN  1900

#define XOS_INET_MAX_UDPRAW_MSGSIZE 8192 /* max size of UDP datagram */

#define XOS_INET_READ_ANY  -1  /* read any pending data */

#define XOS_INET_INADDR_ANY  INADDR_ANY  /* accepts any address */

/*sctp_bindx的参数flag定义*/
#define SCTP_BIND_ADD       1   /*SCTP_BINDX_ADD_ADDR*/
#define SCTP_BIND_DEL       2   /*SCTP_BINDX_REM_ADDR*/
/*macros*/

/*macros to manipulate and checking a socket file descriptor set*/
#define XOS_INET_FD_SET(_sockFd, _fdSet)    FD_SET((_sockFd)->fd, _fdSet)
#define XOS_INET_FD_CLR(_sockFd, _fdSet)    FD_CLR((_sockFd)->fd, _fdSet)
#define XOS_INET_FD_ISSET(_sockFd, _fdSet)  FD_ISSET((_sockFd)->fd, _fdSet)
#ifdef XOS_FDSET
#define XOS_INET_FD_ZERO(_fdSet)            XOS_MemSet(_fdSet, 0, sizeof(t_FDSET))
#else
#define XOS_INET_FD_ZERO(_fdSet)            FD_ZERO(_fdSet)
#endif
/* macros to convert from network to host byteorder and vice versa */
#define XOS_INET_NTOH_U64(x) (((x & 0x00000000000000ff) << 56) | \
                ((x & 0x000000000000ff00) << 40) | \
                ((x & 0x0000000000ff0000) << 24) | \
                ((x & 0x00000000ff000000) <<  8) | \
                ((x & 0x000000ff00000000) >>  8) | \
                ((x & 0x0000ff0000000000) >> 24) | \
                ((x & 0x00ff000000000000) >> 40) | \
                ((x & 0xff00000000000000) >> 56))

#define XOS_INET_HTON_U64(x) (((x & 0x00000000000000ff) << 56) | \
                ((x & 0x000000000000ff00) << 40) | \
                ((x & 0x0000000000ff0000) << 24) | \
                ((x & 0x00000000ff000000) <<  8) | \
                ((x & 0x000000ff00000000) >>  8) | \
                ((x & 0x0000ff0000000000) >> 24) | \
                ((x & 0x00ff000000000000) >> 40) | \
                ((x & 0xff00000000000000) >> 56))

#define XOS_INET_NTOH_U32(_long)  ntohl(_long)
#define XOS_INET_HTON_U32(_long)  htonl(_long)
#define XOS_INET_NTOH_U16(_word)  ntohs(_word)
#define XOS_INET_HTON_U16(_word)  htons(_word)

/* 判断sock 是否有效*/
#ifdef XOS_WIN32
#define XOS_INET_INV_SOCK_FD(_sockFd)  ((_sockFd)->fd == INVALID_SOCKET)
typedef int socklen_t;

#else
#define XOS_INET_INV_SOCK_FD(_sockFd)  ((_sockFd)->fd < 0)
#endif /* XOS_WIN32 */

/*将描述符置为无效的*/
#ifdef XOS_WIN32
#define XOS_INET_SET_INV_SOCK_FD(_sockFd)  ((_sockFd)->fd = INVALID_SOCKET)
#else
#define XOS_INET_SET_INV_SOCK_FD(_sockFd)  ((_sockFd)->fd = -1)
#endif /* XOS_WIN32 */

#ifndef SCTP_CLI_MIN_NUM
#define SCTP_CLI_MIN_NUM        1        /*sctp客户端最小数量*/
#endif

#ifndef SCTP_CLI_MAX_NUM
#define SCTP_CLI_MAX_NUM        512        /*sctp客户端最大数量*/
#endif

#ifndef SCTP_SERV_MIN_NUM
#define SCTP_SERV_MIN_NUM        1        /*sctp服务端最小数量*/
#endif

#ifndef SCTP_SERV_MAX_NUM
#define SCTP_SERV_MAX_NUM        256        /*sctp服务端最大数量*/
#endif

#ifndef SCTP_CLIENTS_PER_SERV_MIN_NUM
#define SCTP_CLIENTS_PER_SERV_MIN_NUM        1        /*sctp服务端支持接入的最小客户端数量*/
#endif

#ifndef SCTP_CLIENTS_PER_SERV_MAX_NUM
#define SCTP_CLIENTS_PER_SERV_MAX_NUM        10000    /*sctp服务端支持接入的最大数量客户端*/
#endif



#ifndef SCTP_HB_PRIMITIVE_MIN
#define SCTP_HB_PRIMITIVE_MIN        500        /*sctp心跳间隔设置最小值*/
#endif

#ifndef SCTP_HB_PRIMITIVE_DEFAULT
#define SCTP_HB_PRIMITIVE_DEFAULT        2000        /*sctp心跳间隔默认值*/
#endif

#ifndef SCTP_RTO_MIN_MIN
#define SCTP_RTO_MIN_MIN        400        /*sctp数据包重发第一次时间最小值 ms*/
#endif

#ifndef SCTP_RTO_MIN_MAX
#define SCTP_RTO_MIN_MAX        5000        /*sctp数据包重发第一次时间最大值 ms*/
#endif

#ifndef SCTP_RTO_MIN_DEFAULT
#define SCTP_RTO_MIN_DEFAULT        1000        /*sctp数据包重发第一次时间默认值 ms*/
#endif

#ifndef SCTP_RTO_INIT_MIN
#define SCTP_RTO_INIT_MIN        800        /*sctp数据包重发第二次时间最小值 ms*/
#endif

#ifndef SCTP_RTO_INIT_MAX
#define SCTP_RTO_INIT_MAX        5000        /*sctp数据包重发第二次时间最大值 ms*/
#endif

#ifndef SCTP_RTO_INIT_DEFAULT
#define SCTP_RTO_INIT_DEFAULT        1500        /*sctp数据包重发第二次时间默认值 ms*/
#endif

#ifndef SCTP_SACK_TIMEOUT_DEFAULT
#define SCTP_SACK_TIMEOUT_DEFAULT        200        /*sack默认延迟发送间隔ms*/
#endif

#ifndef SCTP_SACK_TIMEOUT_MAX
#define SCTP_SACK_TIMEOUT_MAX        500            /*sack最大延迟发送间隔ms*/
#endif



#ifndef SCTP_PATH_RETRANS_MIN
#define SCTP_PATH_RETRANS_MIN        1        /*sctp路径错误计数器最小值*/
#endif

#ifndef SCTP_PATH_RETRANS_MAX
#define SCTP_PATH_RETRANS_MAX        5        /*sctp路径错误计数器最大值*/
#endif

#ifndef SCTP_PATH_RETRANS_DEFAULT
#define SCTP_PATH_RETRANS_DEFAULT        2        /*sctp路径错误计数器默认值*/
#endif

/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/
/*SOCK 的地址结构*/
typedef struct sockaddr t_INETSOCKADDR;

/*sock discriptor*/

#ifdef XOS_WIN32
typedef SOCKET t_SOCKFD;
#else
#ifdef XOS_LINUX
typedef XS32 t_SOCKFD;
#else
typedef XS16 t_SOCKFD;
#endif
#endif /* XOS_WIN32 */

/* sockfd 的封装 */
typedef struct
{
   t_SOCKFD fd;              /* socket descriptor */
   XU8     blocking;         /* true if socket is blocking */
}t_XINETFD;

#ifdef XOS_FDSET
#ifdef XOS_ARCH_64
#define DEFINE_MAX_FD   (((XOS_MAX_FD+64)/64)*64)
#else
#define DEFINE_MAX_FD   (((XOS_MAX_FD+32)/32)*32)
#endif
typedef struct xos_fd_set
{
#ifdef XOS_ARCH_64
#ifdef __USE_XOPEN
    long fds_bits[DEFINE_MAX_FD/64]; 
#else
    long __fds_bits[DEFINE_MAX_FD/64]; 
#endif

#else

#ifdef __USE_XOPEN
    int fds_bits[DEFINE_MAX_FD/32]; 
#else
    int __fds_bits[DEFINE_MAX_FD/32]; 
#endif
#endif
}t_FDSET;
#else
typedef fd_set t_FDSET;      /* socket file descriptor set */
#endif

/*sock   set*/
typedef struct
{
    t_FDSET    fdSet;
#if 0
   // XS32        setNum;     /*set 集合中sock 的数量*/
   //t_XOSMUTEXID  mutexId;   /*set 会被多线程更改，应该用互斥量控制*/
#endif
}t_SOCKSET;

/*-------------------------------------------------------------------------
API 声明
-------------------------------------------------------------------------*/
/**********************************
函数名称    : XINET_Init
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 初始化sock lib库
参数        : void
返回值      : XPUBLIC XS16
              XOS_SUCC  -   成功
              XOS_ERROR    -   主要指参数错误
说明：在调用所有的sock函数前调用，只有window下需要，
获取错误信息时用得上
************************************/
XPUBLIC XS16 XINET_Init( void);

/**********************************
函数名称    : XINET_GetNumRead
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 获取当前可以读的挲据长度
参数        : t_XINETFD *sockFd 描述符
参数        : XU32* dataLen 数据长度
返回值      : XPUBLIC XS16
              XOS_SUCC      - successful
              XOS_ERROR     - failed
************************************/
XPUBLIC XS16 XINET_GetNumRead(t_XINETFD *sockFd, XU32* dataLen);

/**********************************
函数名称    : XINET SendMsg
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 发送数据到网络
参数        : t_XINETFD *pSockFd －sock 描述符指针
参数        : t_IPADDR *pDstAddr －目标地址指针(tcp 或者连接的udp 可以为空)
参数        : XS32 len           －要发送的数据长度
参数        : XCHAR* pData       －要发送数据数据的曜地址
返回值      : XS16
XOS_SUCC          -   成功
XOS_ERROR         -   主要指参数错误
eErrorNetBlock    －网络阻塞
eErrorLinkClosed  －链路关闭
eErrorOverflow    －溢出，要发送的挲据长度超过最大长度
说明：就算是返回成功，数据也不一定发送到对端。
************************************/
XS16 XINET_SendMsg(t_XINETFD *pSockFd,e_LINKTYPE linkType, t_IPADDR *pDstAddr, XS32 len, XCHAR* pData,XS32 *out_len);

/**********************************
函数名称    : XINET_Select
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 轮询监视socket
参数        : t_FDSET*  readFdS   －read集的指针
参数        : t_FDSET *writeFdS   － write集的指针
参数        : XU32  *mSecTimeout  －超时时间；单位毫秒
参数        : XS16  *numFdS       －监测到sock被改变的数量
返回值      : XPUBLIC XS16
成功返回XOS_SUCC, 失败返回XOS_ERROR，
超时返回ERR_TIMEDOUT
说明: 当超时时间指针为空时，无限时的等待
************************************/
XPUBLIC XS16 XINET_Select(t_FDSET*  readFdS, t_FDSET *writeFdS,
                 XU32  *mSecTimeout, XS16  *numFdS);

/**********************************
函数名称    : XINET_RecvMsg
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 从网络上接收挲君
输入参数    : t_XINETFD *pSockFd   －sock 描述符指针
输入参数    : t_IPADDR* pFromAddr  －目标地址指针(tcp 或者连接的udp 可以为空)
输入参数    : XS32* pLen           － 输入时，－1表示接收所有的数据
输入参数    : XU8 linkType         － 链路的类型
              (XOS_INET_STREAM =TCP ; XOS_INET_DGRAM =UDP )
输入参数    : XS32 ctrFlag         － 控制标志位
输出参数    : XCHAR** ppData       －输出接受到的数据指针
输出参数    : XS32* pLen           － 输入时，－1表示接收所有的数据
返回值      : XPUBLIC XS16
            XSUCC -   成功
            XERROR   -   主要指参数错误
            XINET_CLOSE     - 对端关闭
说明：就算是返回成功，数据也不一定发送到对端。
************************************/
XPUBLIC XS16 XINET_RecvMsg(t_XINETFD *pSockFd, t_IPADDR* pFromAddr,
XCHAR** ppData, XS32* pLen, XU8 linkType, XS32 ctrFlag);

/**********************************
函数名称    : XINET_Socket
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 打开一个socket
输入参数        : XU8 type
              －XOS_INET_STREAM   (TCP)
              - XOS_INET_DGRAM    (UDP)
输出参数    : t_XINETFD* sockFd － 描述符
返回值      : XPUBLIC XS16
              XOS_SUCC  - 成功
              XOS_ERROR - 主要指参数错误
说明：默认是非阻塞模式
************************************/
XPUBLIC XS16 XINET_Socket(XU8 type,  t_XINETFD* sockFd);

/**********************************
函数名称    : XINET_Bind
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 绑定一个socket
输入参数    : t_XINETFD *pSockFd  － 描述符
输入参数    : t_IPADDR *pMyAddr   －本地地址
返回值      : XPUBLIC XS16
                XOS_SUCC  -  成功
                XOS_ERROR -  主要指参数错误
************************************/
XPUBLIC XS16 XINET_Bind(t_XINETFD *pSockFd,t_IPADDR *pMyAddr);

/*****************************************************
函数名称    : XINET_IsTcpPortUnused
作者        : maliming
设计日期    : 2014年3月10日
功能描述    : 测试tcp端口可用
输入参数    : t_IPADDR *pMyAddr   －本地地址
返回值      : XPUBLIC XS16
XOS_SUCC  -	  端口可用
XOS_ERROR -  主要指参数错误或端口不可用
*****************************************************/
XPUBLIC XS16 XINET_IsTcpPortUnused(t_IPADDR *pMyAddr);

/**********************************
函数名称    : XINET_CloseSock
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    :
参数        : t_XINETFD *sockFd
返回值      : XPUBLIC XS16 － 描述符
                XOS_SUCC  -  成功
                XOS_ERROR -  主要指参数错误
************************************/
XPUBLIC XS16 XINET_CloseSock(t_XINETFD *sockFd);

/**********************************
函数名称    : XINET_Connect
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 建立一个连接
输入参数        : t_XINETFD   *pSockFd － 描述符
输入参数        : t_IPADDR *pServAddr  －对端地址
返回值      : XPUBLIC XS16
XOS_SUCC         - successful
XOS_SUCCDNA      - resource temporarily unavaiable
RINPROGRESS - connection is in progress (only non-blocking)
RISCONN     - connection is established (only non-blocking)
XOS_ERROR     - failed
XINET_CLOSE  - 连接超时或连接拒绝
************************************/
XPUBLIC XS16 XINET_Connect(t_XINETFD   *pSockFd, t_IPADDR *pServAddr);

/**********************************
函数名称    : XINET_Listen
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 监听一个描述符
参数        : t_XINETFD *pSockFd － 描述符
参数        : XS16 backLog       －容许同时连接的最大个数
返回值      : XPUBLIC XS16
                XOS_SUCC  -  成功
                XOS_ERROR -  错误
************************************/
XPUBLIC XS16 XINET_Listen(t_XINETFD *pSockFd, XS16 backLog);

/**********************************
函数名称    : XINET_StrToIpAddr
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 字符串到ip地址转换
输入参数    : XU16  len   － 字符串的长度
输入参数    : XCHAR *val  －字符串首地址
输出参数    : XU32 *ip    －地址
返回值      : XPUBLIC XS16
                XOS_SUCC  -  成功
                XOS_ERROR -  错误
************************************/
XPUBLIC XS16 XINET_StrToIpAddr(XU16  len, XCHAR *val, XU32 *ip);

/**********************************
函数名称    : XINET_SetOpt
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 设置io 选项
输入:
参数        : t_XINETFD *sockFd － 描述符
参数        : XU32 level        －选项层次
参数        : XU32 type         －设置选项的类型
参数        : XU32* value       －设置选项的值
输出：
返回值      : XPUBLIC XS16
                XOS_SUCC  -  成功
                XOS_ERROR -  错误
说明： 第一步只支持阻塞和非阻塞选项，其他用到时再加
************************************/
XPUBLIC XS16 XINET_SetOpt(t_XINETFD *sockFd, XU32 level,XU32 type, XU32* value);


XPUBLIC XS16 XINET_TcpConnectCheck(t_XINETFD *sockFd);
/**********************************
函数名称    : XINET_Accept
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 接受一个连接
输入参数    : t_XINETFD   *pSockFd  - 描述符
输出参数    : t_IPADDR *pFromAddr   -对端地址
输出参数        : t_XINETFD  *pNewSockFd
返回值      : XPUBLIC XS16
                XOS_SUCC  -  成功
                XOS_ERROR -  错误
说明：默认是非阻塞模式
************************************/
XPUBLIC XS16 XINET_Accept(t_XINETFD   *pSockFd,
           t_IPADDR *pFromAddr,
           t_XINETFD  *pNewSockFd);

/**********************************
函数名称    : XINET_GetSockName
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    : 获取sock 相关联的本地地址
输入参数    : t_XINETFD *sockFd -描述符
输出参数    : t_IPADDR* locAddr号   -本地地址(主机字节序)
返回值      : XPUBLIC XS16
              XOS_SUCC  - 成功
              XOS_ERROR - 主要指参数错误
************************************/
XPUBLIC XS16 XINET_GetSockName(t_XINETFD *sockFd, t_IPADDR* locAddr);

/**********************************
函数名称    : XINET_GetMac
作者        : liuda
设计日期    : 2006年3月7日
功能描述    : 获取接口上的mac地址
输入参数    : ifname  -接口名称
输出参数    : mac     -接口上的mac地址
返回值      : XPUBLIC XS16
              XOS_SUCC  - 成功
              XOS_ERROR - 失败
************************************/
XPUBLIC XS16  XINET_GetMac(XU8 *ifname, XU8 *mac);


/**********************************
函数名称    : XINET_End
作者        : wangzongyou
设计日期    : 2006年3月7日
功能描述    :
参数        : void
返回值      : XPUBLIC XS16
************************************/
XPUBLIC XS16 XINET_End(void);

#if defined(XOS_SCTP) && defined(XOS_LINUX)
/**********************************
函数名称    : XINET_GetOpt
作者        : liukai
设计日期    : 2013.10.18
功能描述    : 获取io 选项
输入:
参数        : t_XINETFD *sockFd － 描述符
参数        : XU32 level        －选项层次
参数        : XU32 type         －获取选项的类型
输出：        : XU32* value       －获取选项的值
返回值      : XPUBLIC XS16
                XOS_SUCC  -  成功
                XOS_ERROR -  错误
************************************/
XPUBLIC XS16 XINET_GetOpt(t_XINETFD *sockFd, XU32 level,XU32 type, XU32* value);

/*************************************************************
函数名称    : XINET_SctpSocket
作者        : liukai
设计日期    : 2013年9月22日
功能描述    : 打开一个socket
输出参数    : t_XINETFD* sockFd － 描述符
               XU32 stream      - 输入输出的流数量
返回值      : XPUBLIC XS16
XOS_SUCC  - 成功
XOS_ERROR - 主要指参数错误
说明：默认是非阻塞模式
**************************************************************/
XPUBLIC XS16 XINET_SctpSocket(t_XINETFD* sockFd, XU32 stream);

/**********************************
函数名称    : XINET_SctpBind
作者        : liukai
设计日期    : 2013年9月13日
功能描述    : 绑定一个socket
输入参数    : t_XINETFD *pSockFd  － 描述符
输入参数    : t_SCTPIPADDR *pMyAddr   －本地地址集
返回值      : XPUBLIC XS16 XS32 flags  - SCTP_BINDX_ADD_ADDR 增加地址；
                                        - SCTP_BINDX_ADD_REM 删除地址
                XOS_SUCC  -  成功
                XOS_ERROR -  主要指参数错误
************************************/
XPUBLIC XS16 XINET_SctpBind(t_XINETFD *pSockFd, t_SCTPIPADDR *pMyAddr, XS32 flags);

/*******************************************************
函数名称    : XINET_SctpAccept
作者        : liukai
设计日期    : 2013年9月24日
功能描述    : 接受一个sctp连接
输入参数    : t_XINETFD   *pSockFd  - 描述符
输出参数    : t_IPADDR *pFromAddr   -对端地址
输出参数        : t_XINETFD  *pNewSockFd
返回值      : XPUBLIC XS16
XOS_SUCC  -  成功
XOS_ERROR -  错误
说明：默认是非阻塞模式
*********************************************************/
XPUBLIC XS16 XINET_SctpAccept(t_XINETFD   *pSockFd, t_SCTPIPADDR *pFromAddr, XU32 hb, t_XINETFD  *pNewSockFd);

/**********************************
函数名称    : XINET_SctpConnect
作者        : liukai
设计日期    : 2013年9月13日
功能描述    : 建立一个连接
输入参数        : t_XINETFD   *pSockFd － 描述符
输入参数        : t_SCTPIPADDR *pServAddr  －对端地址集
输出参数        : XS32 *id            －偶联ID
返回值      : XPUBLIC XS16
XOS_SUCC         - successful
XOS_SUCCDNA      - resource temporarily unavaiable
RINPROGRESS - connection is in progress (only non-blocking)
RISCONN     - connection is established (only non-blocking)
XOS_ERROR     - failed
XINET_CLOSE  - 连接超时或连接拒绝
************************************/
XPUBLIC XS16 XINET_SctpConnect(t_XINETFD *pSockFd, t_SCTPIPADDR *pServAddr,XS32 *id);

/**********************************
函数名称    : XINET_SctpSendMsg
作者        : liukai
设计日期    : 2013年9月13日
功能描述    : 发送数据到网络
参数        : t_XINETFD *pSockFd －sock 描述符指针
参数        : XCHAR* pData       －要发送数据数据的地址
参数        : XS32 len           －要发送的数据长度
参数        : t_SCTPIPADDR *pDstAddr －目标地址集指针(可以为空)
参数        : XU16 stream       -发送数据的流号
参数        : XU32  ppid        -净荷协议标识符，可选，默认0
参数        : XU32 context       -上下文，可选，默认0
返回值      : XS16
XOS_SUCC          -   成功
XOS_ERROR         -   主要指参数错误
eErrorNetBlock    －网络阻塞
eErrorLinkClosed  －链路关闭
eErrorOverflow    －溢出，要发送的数据长度超过最大长度
************************************/
XS16 XINET_SctpSendMsg(t_XINETFD *pSockFd, XCHAR* pData, XS32 len , t_SCTPIPADDR *pDstAddr, t_SctpDataAttr attr);

/**********************************
函数名称    : XINET_SctpRecvMsg
作者        : liukai
设计日期    : 2013年9月13日
功能描述    : 从网络上接收数据
输入参数    : t_XINETFD *pSockFd   －sock 描述符指针
输出参数    : XCHAR** ppData       －输出接受到的数据指针
输入参数    : XS32* pLen           － 输入时，－1表示接收所有的数据
输入参数    : t_SCTPIPADDR* pFromAddr  －目标地址指针(可以为空)
输出参数    : XVOID** ppSndRcvInfo  - 输出接收数据的流号等信息
返回值      : XPUBLIC XS16
            XSUCC -   成功
            XERROR   -   主要指参数错误
            XINET_CLOSE     - 对端关闭
说明：就算是返回成功，数据也不一定发送到对端。
************************************/
XPUBLIC XS16 XINET_SctpRecvMsg(t_XINETFD *pSockFd, XCHAR** ppData, XS32* pLen, t_IPADDR* pFromAddr, XVOID* pSndRcvInfo);

/**********************************
函数名称    : XINET_SctpConnectCheck
作者        : liukai
设计日期    : 2013年9月23日
功能描述    : 检查sctp连接是否正常
输入参数    : t_XINETFD *pSockFd   －sock 描述符指针
返回值      : XPUBLIC XS32 获取偶联对端端点的ip数
>0 连接正常
<=0 连接断开
**************************************************************/
XPUBLIC XS32 XINET_SctpConnectCheck(t_XINETFD *sockFd);
#endif
XEXTERN XBOOL g_ntltraceswitch;

extern XS32 write_to_syslog(const XS8* msg,...);
extern XVOID write_stack_to_syslog(XU32 fid);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xos.h*/

