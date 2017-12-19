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
                  ����ͷ�ļ�
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
                �궨��
-------------------------------------------------------------------------*/

#define INPUT_PAR_CHECK     1  /*����������ڲ������*/

/*����ֵ����*/
/*��·�ر�*/
#define  XINET_CLOSE        2
/*- connection is in progress (only non-blocking)*/
#define  XINET_INPROGRESS   3
/*- connection is established (only non-blocking)*/
#define  XINET_ISCONN       4
/*select time out */
#define  XINET_TIMEOUT      5
/*packet size is too big */
#define  XINET_INVALID      6

/*��ʱ�����Ĵ���*/
#define TCP_RECONNECT_MAX   5  /* 5��*/

#define ERR_BADF            9

/*��xosinet.c������xosinet.h  (by zengjiandong)*/
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

/*��Ч��sock*/
#ifdef XOS_WIN32
#define XOS_INET_INV_SOCKFD INVALID_SOCKET
#else
#define XOS_INET_INV_SOCKFD  -1
#endif

/*���Ʊ�־λ*/
#define  XOS_INET_CTR_NOCOMPATI_TCPEEN    0   /*��������ƽ̨��tcp��װ*/
#define  XOS_INET_CTR_COMPATIBLE_TCPEEN   1   /*������ƽ̨��tcp��װ*/
#define  XOS_INET_CTR_RCV_CERTERIN_BYTES  2   /*���̶ܹ���С����*/
#define  XOS_INET_CTR_PEER_DATA           3   /*��Ӱ�����ݽ���,͵������*/

/* socket types */
#define XOS_INET_STREAM  SOCK_STREAM
#define XOS_INET_DGRAM   SOCK_DGRAM

/*����ͬʱ�����tcp �ͻ��˵�����*/
#define MIN_BACK_LOG  1
#define MAX_BACK_LOG  8

/* socket options type */
#define XOS_INET_OPT_BLOCK         0  /*  ����ѡ�� */
#define XOS_INET_OPT_REUSEADDR     1  /* ��ַ����ѡ�� */
#define XOS_INET_OPT_RX_BUF_SIZE   2  /* ���ջ�������Сѡ�� */
#define XOS_INET_OPT_TX_BUF_SIZE   3  /* ���ͻ�������Сѡ��*/
#define XOS_INET_OPT_ADD_MCAST_MBR 4  /* ����ip����ѡ�� */
#define XOS_INET_OPT_DRP_MCAST_MBR 5  /* ɾ��ip����ѡ�� */
#define XOS_INET_OPT_TCP_NODELAY   6  /* ����tcp/ipЭ��ջ��Ӧ��ѡ�� */

#ifdef XOS_LINUX
#define XOS_INET_OPT_BSD_COMPAT    7  /* BSD compatible option for Linux */
#endif /* XOS_LINUX */

#define XOS_INET_OPT_MCAST_LOOP    8  /* �㲥ѭ��ѡ�� */
#define XOS_INET_OPT_MCAST_IF      9  /* ָ���������ݷ��͵Ľӿ� */
#define XOS_INET_OPT_MCAST_TTL     14 /* ָ��TTl ��ָ*/

#define XOS_INET_OPT_BROADCAST     23 /* �㲥ѡ�� */

#define XOS_INET_OPT_KEEPALIVE     24 /* ���ӱ���ѡ��*/

#define XOS_INET_OPT_LINGER        25  /*��������tcp ״̬��*/
#define XOS_INET_OPT_SCTP_EVENT    26  /* ����sctp�׽���event���� */
#define XOS_INET_OPT_SCTP_OUTSTREAM   27  /* ����sctp���˳������� */
#define XOS_INET_OPT_SCTP_HB       28  /* ����sctp�׽���������� */
#define XOS_INET_OPT_SCTP_PEER_INSTREAM   29  /* ��ȡsctp�Զ����������� */
#define XOS_INET_OPT_SCTP_ASSOCINFO   30  /* ����sctpż��������ֵⷧ */
#define XOS_INET_OPT_SCTP_PATHMAXRXT   31  /* ����sctp·������ֵⷧ */
#define XOS_INET_OPT_SCTP_NODELAY    32  /* ����sctp�׽���nagle�㷨 */



/* socket ѡ���ֵ */
#define XOS_INET_OPT_DISABLE       0   /* enable option */
#define XOS_INET_OPT_ENABLE        1   /* disable option */

/* ����ѡ��Ĳ�� */
#define XOS_INET_LEVEL_SOCKET      SOL_SOCKET  /* ����socket ��һ���ѡ�� */
#define XOS_INET_LEVEL_IP          IPPROTO_IP  /* ip ���ѡ��*/
#define XOS_INET_LEVEL_TCP         IPPROTO_TCP /* TCP ��ѡ�� */
#define XOS_INET_LEVEL_SCTP         IPPROTO_SCTP /* SCTP ��ѡ�� */

#define XOS_INET_MAX_MSG_LEN  0x7fff  /* max length of a message */

/*������ƽ̨tcp��װtcp���ݵ���󳤶�*/
#define XOS_INET_COMPATY_TCPEEN_DATA_LEN  1900

#define XOS_INET_MAX_UDPRAW_MSGSIZE 8192 /* max size of UDP datagram */

#define XOS_INET_READ_ANY  -1  /* read any pending data */

#define XOS_INET_INADDR_ANY  INADDR_ANY  /* accepts any address */

/*sctp_bindx�Ĳ���flag����*/
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

/* �ж�sock �Ƿ���Ч*/
#ifdef XOS_WIN32
#define XOS_INET_INV_SOCK_FD(_sockFd)  ((_sockFd)->fd == INVALID_SOCKET)
typedef int socklen_t;

#else
#define XOS_INET_INV_SOCK_FD(_sockFd)  ((_sockFd)->fd < 0)
#endif /* XOS_WIN32 */

/*����������Ϊ��Ч��*/
#ifdef XOS_WIN32
#define XOS_INET_SET_INV_SOCK_FD(_sockFd)  ((_sockFd)->fd = INVALID_SOCKET)
#else
#define XOS_INET_SET_INV_SOCK_FD(_sockFd)  ((_sockFd)->fd = -1)
#endif /* XOS_WIN32 */

#ifndef SCTP_CLI_MIN_NUM
#define SCTP_CLI_MIN_NUM        1        /*sctp�ͻ�����С����*/
#endif

#ifndef SCTP_CLI_MAX_NUM
#define SCTP_CLI_MAX_NUM        512        /*sctp�ͻ����������*/
#endif

#ifndef SCTP_SERV_MIN_NUM
#define SCTP_SERV_MIN_NUM        1        /*sctp�������С����*/
#endif

#ifndef SCTP_SERV_MAX_NUM
#define SCTP_SERV_MAX_NUM        256        /*sctp������������*/
#endif

#ifndef SCTP_CLIENTS_PER_SERV_MIN_NUM
#define SCTP_CLIENTS_PER_SERV_MIN_NUM        1        /*sctp�����֧�ֽ������С�ͻ�������*/
#endif

#ifndef SCTP_CLIENTS_PER_SERV_MAX_NUM
#define SCTP_CLIENTS_PER_SERV_MAX_NUM        10000    /*sctp�����֧�ֽ������������ͻ���*/
#endif



#ifndef SCTP_HB_PRIMITIVE_MIN
#define SCTP_HB_PRIMITIVE_MIN        500        /*sctp�������������Сֵ*/
#endif

#ifndef SCTP_HB_PRIMITIVE_DEFAULT
#define SCTP_HB_PRIMITIVE_DEFAULT        2000        /*sctp�������Ĭ��ֵ*/
#endif

#ifndef SCTP_RTO_MIN_MIN
#define SCTP_RTO_MIN_MIN        400        /*sctp���ݰ��ط���һ��ʱ����Сֵ ms*/
#endif

#ifndef SCTP_RTO_MIN_MAX
#define SCTP_RTO_MIN_MAX        5000        /*sctp���ݰ��ط���һ��ʱ�����ֵ ms*/
#endif

#ifndef SCTP_RTO_MIN_DEFAULT
#define SCTP_RTO_MIN_DEFAULT        1000        /*sctp���ݰ��ط���һ��ʱ��Ĭ��ֵ ms*/
#endif

#ifndef SCTP_RTO_INIT_MIN
#define SCTP_RTO_INIT_MIN        800        /*sctp���ݰ��ط��ڶ���ʱ����Сֵ ms*/
#endif

#ifndef SCTP_RTO_INIT_MAX
#define SCTP_RTO_INIT_MAX        5000        /*sctp���ݰ��ط��ڶ���ʱ�����ֵ ms*/
#endif

#ifndef SCTP_RTO_INIT_DEFAULT
#define SCTP_RTO_INIT_DEFAULT        1500        /*sctp���ݰ��ط��ڶ���ʱ��Ĭ��ֵ ms*/
#endif

#ifndef SCTP_SACK_TIMEOUT_DEFAULT
#define SCTP_SACK_TIMEOUT_DEFAULT        200        /*sackĬ���ӳٷ��ͼ��ms*/
#endif

#ifndef SCTP_SACK_TIMEOUT_MAX
#define SCTP_SACK_TIMEOUT_MAX        500            /*sack����ӳٷ��ͼ��ms*/
#endif



#ifndef SCTP_PATH_RETRANS_MIN
#define SCTP_PATH_RETRANS_MIN        1        /*sctp·�������������Сֵ*/
#endif

#ifndef SCTP_PATH_RETRANS_MAX
#define SCTP_PATH_RETRANS_MAX        5        /*sctp·��������������ֵ*/
#endif

#ifndef SCTP_PATH_RETRANS_DEFAULT
#define SCTP_PATH_RETRANS_DEFAULT        2        /*sctp·�����������Ĭ��ֵ*/
#endif

/*-------------------------------------------------------------------------
                �ṹ��ö������
-------------------------------------------------------------------------*/
/*SOCK �ĵ�ַ�ṹ*/
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

/* sockfd �ķ�װ */
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
   // XS32        setNum;     /*set ������sock ������*/
   //t_XOSMUTEXID  mutexId;   /*set �ᱻ���̸߳��ģ�Ӧ���û���������*/
#endif
}t_SOCKSET;

/*-------------------------------------------------------------------------
API ����
-------------------------------------------------------------------------*/
/**********************************
��������    : XINET_Init
����        : wangzongyou
�������    : 2006��3��7��
��������    : ��ʼ��sock lib��
����        : void
����ֵ      : XPUBLIC XS16
              XOS_SUCC  -   �ɹ�
              XOS_ERROR    -   ��Ҫָ��������
˵�����ڵ������е�sock����ǰ���ã�ֻ��window����Ҫ��
��ȡ������Ϣʱ�õ���
************************************/
XPUBLIC XS16 XINET_Init( void);

/**********************************
��������    : XINET_GetNumRead
����        : wangzongyou
�������    : 2006��3��7��
��������    : ��ȡ��ǰ���Զ������ݳ���
����        : t_XINETFD *sockFd ������
����        : XU32* dataLen ���ݳ���
����ֵ      : XPUBLIC XS16
              XOS_SUCC      - successful
              XOS_ERROR     - failed
************************************/
XPUBLIC XS16 XINET_GetNumRead(t_XINETFD *sockFd, XU32* dataLen);

/**********************************
��������    : XINET SendMsg
����        : wangzongyou
�������    : 2006��3��7��
��������    : �������ݵ�����
����        : t_XINETFD *pSockFd ��sock ������ָ��
����        : t_IPADDR *pDstAddr ��Ŀ���ַָ��(tcp �������ӵ�udp ����Ϊ��)
����        : XS32 len           ��Ҫ���͵����ݳ���
����        : XCHAR* pData       ��Ҫ�����������ݵ��׵�ַ
����ֵ      : XS16
XOS_SUCC          -   �ɹ�
XOS_ERROR         -   ��Ҫָ��������
eErrorNetBlock    ����������
eErrorLinkClosed  ����·�ر�
eErrorOverflow    �������Ҫ���͵����ݳ��ȳ�����󳤶�
˵���������Ƿ��سɹ�������Ҳ��һ�����͵��Զˡ�
************************************/
XS16 XINET_SendMsg(t_XINETFD *pSockFd,e_LINKTYPE linkType, t_IPADDR *pDstAddr, XS32 len, XCHAR* pData,XS32 *out_len);

/**********************************
��������    : XINET_Select
����        : wangzongyou
�������    : 2006��3��7��
��������    : ��ѯ����socket
����        : t_FDSET*  readFdS   ��read����ָ��
����        : t_FDSET *writeFdS   �� write����ָ��
����        : XU32  *mSecTimeout  ����ʱʱ�䣻��λ����
����        : XS16  *numFdS       ����⵽sock���ı������
����ֵ      : XPUBLIC XS16
�ɹ�����XOS_SUCC, ʧ�ܷ���XOS_ERROR��
��ʱ����ERR_TIMEDOUT
˵��: ����ʱʱ��ָ��Ϊ��ʱ������ʱ�ĵȴ�
************************************/
XPUBLIC XS16 XINET_Select(t_FDSET*  readFdS, t_FDSET *writeFdS,
                 XU32  *mSecTimeout, XS16  *numFdS);

/**********************************
��������    : XINET_RecvMsg
����        : wangzongyou
�������    : 2006��3��7��
��������    : �������Ͻ�������
�������    : t_XINETFD *pSockFd   ��sock ������ָ��
�������    : t_IPADDR* pFromAddr  ��Ŀ���ַָ��(tcp �������ӵ�udp ����Ϊ��)
�������    : XS32* pLen           �� ����ʱ����1��ʾ�������е�����
�������    : XU8 linkType         �� ��·������
              (XOS_INET_STREAM =TCP ; XOS_INET_DGRAM =UDP )
�������    : XS32 ctrFlag         �� ���Ʊ�־λ
�������    : XCHAR** ppData       ��������ܵ�������ָ��
�������    : XS32* pLen           �� ����ʱ����1��ʾ�������е�����
����ֵ      : XPUBLIC XS16
            XSUCC -   �ɹ�
            XERROR   -   ��Ҫָ��������
            XINET_CLOSE     - �Զ˹ر�
˵���������Ƿ��سɹ�������Ҳ��һ�����͵��Զˡ�
************************************/
XPUBLIC XS16 XINET_RecvMsg(t_XINETFD *pSockFd, t_IPADDR* pFromAddr,
XCHAR** ppData, XS32* pLen, XU8 linkType, XS32 ctrFlag);

/**********************************
��������    : XINET_Socket
����        : wangzongyou
�������    : 2006��3��7��
��������    : ��һ��socket
�������        : XU8 type
              ��XOS_INET_STREAM   (TCP)
              - XOS_INET_DGRAM    (UDP)
�������    : t_XINETFD* sockFd �� ������
����ֵ      : XPUBLIC XS16
              XOS_SUCC  - �ɹ�
              XOS_ERROR - ��Ҫָ��������
˵����Ĭ���Ƿ�����ģʽ
************************************/
XPUBLIC XS16 XINET_Socket(XU8 type,  t_XINETFD* sockFd);

/**********************************
��������    : XINET_Bind
����        : wangzongyou
�������    : 2006��3��7��
��������    : ��һ��socket
�������    : t_XINETFD *pSockFd  �� ������
�������    : t_IPADDR *pMyAddr   �����ص�ַ
����ֵ      : XPUBLIC XS16
                XOS_SUCC  -  �ɹ�
                XOS_ERROR -  ��Ҫָ��������
************************************/
XPUBLIC XS16 XINET_Bind(t_XINETFD *pSockFd,t_IPADDR *pMyAddr);

/*****************************************************
��������    : XINET_IsTcpPortUnused
����        : maliming
�������    : 2014��3��10��
��������    : ����tcp�˿ڿ���
�������    : t_IPADDR *pMyAddr   �����ص�ַ
����ֵ      : XPUBLIC XS16
XOS_SUCC  -	  �˿ڿ���
XOS_ERROR -  ��Ҫָ���������˿ڲ�����
*****************************************************/
XPUBLIC XS16 XINET_IsTcpPortUnused(t_IPADDR *pMyAddr);

/**********************************
��������    : XINET_CloseSock
����        : wangzongyou
�������    : 2006��3��7��
��������    :
����        : t_XINETFD *sockFd
����ֵ      : XPUBLIC XS16 �� ������
                XOS_SUCC  -  �ɹ�
                XOS_ERROR -  ��Ҫָ��������
************************************/
XPUBLIC XS16 XINET_CloseSock(t_XINETFD *sockFd);

/**********************************
��������    : XINET_Connect
����        : wangzongyou
�������    : 2006��3��7��
��������    : ����һ������
�������        : t_XINETFD   *pSockFd �� ������
�������        : t_IPADDR *pServAddr  ���Զ˵�ַ
����ֵ      : XPUBLIC XS16
XOS_SUCC         - successful
XOS_SUCCDNA      - resource temporarily unavaiable
RINPROGRESS - connection is in progress (only non-blocking)
RISCONN     - connection is established (only non-blocking)
XOS_ERROR     - failed
XINET_CLOSE  - ���ӳ�ʱ�����Ӿܾ�
************************************/
XPUBLIC XS16 XINET_Connect(t_XINETFD   *pSockFd, t_IPADDR *pServAddr);

/**********************************
��������    : XINET_Listen
����        : wangzongyou
�������    : 2006��3��7��
��������    : ����һ��������
����        : t_XINETFD *pSockFd �� ������
����        : XS16 backLog       ������ͬʱ���ӵ�������
����ֵ      : XPUBLIC XS16
                XOS_SUCC  -  �ɹ�
                XOS_ERROR -  ����
************************************/
XPUBLIC XS16 XINET_Listen(t_XINETFD *pSockFd, XS16 backLog);

/**********************************
��������    : XINET_StrToIpAddr
����        : wangzongyou
�������    : 2006��3��7��
��������    : �ַ�����ip��ַת��
�������    : XU16  len   �� �ַ����ĳ���
�������    : XCHAR *val  ���ַ����׵�ַ
�������    : XU32 *ip    ����ַ
����ֵ      : XPUBLIC XS16
                XOS_SUCC  -  �ɹ�
                XOS_ERROR -  ����
************************************/
XPUBLIC XS16 XINET_StrToIpAddr(XU16  len, XCHAR *val, XU32 *ip);

/**********************************
��������    : XINET_SetOpt
����        : wangzongyou
�������    : 2006��3��7��
��������    : ����io ѡ��
����:
����        : t_XINETFD *sockFd �� ������
����        : XU32 level        ��ѡ����
����        : XU32 type         ������ѡ�������
����        : XU32* value       ������ѡ���ֵ
�����
����ֵ      : XPUBLIC XS16
                XOS_SUCC  -  �ɹ�
                XOS_ERROR -  ����
˵���� ��һ��ֻ֧�������ͷ�����ѡ������õ�ʱ�ټ�
************************************/
XPUBLIC XS16 XINET_SetOpt(t_XINETFD *sockFd, XU32 level,XU32 type, XU32* value);


XPUBLIC XS16 XINET_TcpConnectCheck(t_XINETFD *sockFd);
/**********************************
��������    : XINET_Accept
����        : wangzongyou
�������    : 2006��3��7��
��������    : ����һ������
�������    : t_XINETFD   *pSockFd  - ������
�������    : t_IPADDR *pFromAddr   -�Զ˵�ַ
�������        : t_XINETFD  *pNewSockFd
����ֵ      : XPUBLIC XS16
                XOS_SUCC  -  �ɹ�
                XOS_ERROR -  ����
˵����Ĭ���Ƿ�����ģʽ
************************************/
XPUBLIC XS16 XINET_Accept(t_XINETFD   *pSockFd,
           t_IPADDR *pFromAddr,
           t_XINETFD  *pNewSockFd);

/**********************************
��������    : XINET_GetSockName
����        : wangzongyou
�������    : 2006��3��7��
��������    : ��ȡsock ������ı��ص�ַ
�������    : t_XINETFD *sockFd -������
�������    : t_IPADDR* locAddr��   -���ص�ַ(�����ֽ���)
����ֵ      : XPUBLIC XS16
              XOS_SUCC  - �ɹ�
              XOS_ERROR - ��Ҫָ��������
************************************/
XPUBLIC XS16 XINET_GetSockName(t_XINETFD *sockFd, t_IPADDR* locAddr);

/**********************************
��������    : XINET_GetMac
����        : liuda
�������    : 2006��3��7��
��������    : ��ȡ�ӿ��ϵ�mac��ַ
�������    : ifname  -�ӿ�����
�������    : mac     -�ӿ��ϵ�mac��ַ
����ֵ      : XPUBLIC XS16
              XOS_SUCC  - �ɹ�
              XOS_ERROR - ʧ��
************************************/
XPUBLIC XS16  XINET_GetMac(XU8 *ifname, XU8 *mac);


/**********************************
��������    : XINET_End
����        : wangzongyou
�������    : 2006��3��7��
��������    :
����        : void
����ֵ      : XPUBLIC XS16
************************************/
XPUBLIC XS16 XINET_End(void);

#if defined(XOS_SCTP) && defined(XOS_LINUX)
/**********************************
��������    : XINET_GetOpt
����        : liukai
�������    : 2013.10.18
��������    : ��ȡio ѡ��
����:
����        : t_XINETFD *sockFd �� ������
����        : XU32 level        ��ѡ����
����        : XU32 type         ����ȡѡ�������
�����        : XU32* value       ����ȡѡ���ֵ
����ֵ      : XPUBLIC XS16
                XOS_SUCC  -  �ɹ�
                XOS_ERROR -  ����
************************************/
XPUBLIC XS16 XINET_GetOpt(t_XINETFD *sockFd, XU32 level,XU32 type, XU32* value);

/*************************************************************
��������    : XINET_SctpSocket
����        : liukai
�������    : 2013��9��22��
��������    : ��һ��socket
�������    : t_XINETFD* sockFd �� ������
               XU32 stream      - ���������������
����ֵ      : XPUBLIC XS16
XOS_SUCC  - �ɹ�
XOS_ERROR - ��Ҫָ��������
˵����Ĭ���Ƿ�����ģʽ
**************************************************************/
XPUBLIC XS16 XINET_SctpSocket(t_XINETFD* sockFd, XU32 stream);

/**********************************
��������    : XINET_SctpBind
����        : liukai
�������    : 2013��9��13��
��������    : ��һ��socket
�������    : t_XINETFD *pSockFd  �� ������
�������    : t_SCTPIPADDR *pMyAddr   �����ص�ַ��
����ֵ      : XPUBLIC XS16 XS32 flags  - SCTP_BINDX_ADD_ADDR ���ӵ�ַ��
                                        - SCTP_BINDX_ADD_REM ɾ����ַ
                XOS_SUCC  -  �ɹ�
                XOS_ERROR -  ��Ҫָ��������
************************************/
XPUBLIC XS16 XINET_SctpBind(t_XINETFD *pSockFd, t_SCTPIPADDR *pMyAddr, XS32 flags);

/*******************************************************
��������    : XINET_SctpAccept
����        : liukai
�������    : 2013��9��24��
��������    : ����һ��sctp����
�������    : t_XINETFD   *pSockFd  - ������
�������    : t_IPADDR *pFromAddr   -�Զ˵�ַ
�������        : t_XINETFD  *pNewSockFd
����ֵ      : XPUBLIC XS16
XOS_SUCC  -  �ɹ�
XOS_ERROR -  ����
˵����Ĭ���Ƿ�����ģʽ
*********************************************************/
XPUBLIC XS16 XINET_SctpAccept(t_XINETFD   *pSockFd, t_SCTPIPADDR *pFromAddr, XU32 hb, t_XINETFD  *pNewSockFd);

/**********************************
��������    : XINET_SctpConnect
����        : liukai
�������    : 2013��9��13��
��������    : ����һ������
�������        : t_XINETFD   *pSockFd �� ������
�������        : t_SCTPIPADDR *pServAddr  ���Զ˵�ַ��
�������        : XS32 *id            ��ż��ID
����ֵ      : XPUBLIC XS16
XOS_SUCC         - successful
XOS_SUCCDNA      - resource temporarily unavaiable
RINPROGRESS - connection is in progress (only non-blocking)
RISCONN     - connection is established (only non-blocking)
XOS_ERROR     - failed
XINET_CLOSE  - ���ӳ�ʱ�����Ӿܾ�
************************************/
XPUBLIC XS16 XINET_SctpConnect(t_XINETFD *pSockFd, t_SCTPIPADDR *pServAddr,XS32 *id);

/**********************************
��������    : XINET_SctpSendMsg
����        : liukai
�������    : 2013��9��13��
��������    : �������ݵ�����
����        : t_XINETFD *pSockFd ��sock ������ָ��
����        : XCHAR* pData       ��Ҫ�����������ݵĵ�ַ
����        : XS32 len           ��Ҫ���͵����ݳ���
����        : t_SCTPIPADDR *pDstAddr ��Ŀ���ַ��ָ��(����Ϊ��)
����        : XU16 stream       -�������ݵ�����
����        : XU32  ppid        -����Э���ʶ������ѡ��Ĭ��0
����        : XU32 context       -�����ģ���ѡ��Ĭ��0
����ֵ      : XS16
XOS_SUCC          -   �ɹ�
XOS_ERROR         -   ��Ҫָ��������
eErrorNetBlock    ����������
eErrorLinkClosed  ����·�ر�
eErrorOverflow    �������Ҫ���͵����ݳ��ȳ�����󳤶�
************************************/
XS16 XINET_SctpSendMsg(t_XINETFD *pSockFd, XCHAR* pData, XS32 len , t_SCTPIPADDR *pDstAddr, t_SctpDataAttr attr);

/**********************************
��������    : XINET_SctpRecvMsg
����        : liukai
�������    : 2013��9��13��
��������    : �������Ͻ�������
�������    : t_XINETFD *pSockFd   ��sock ������ָ��
�������    : XCHAR** ppData       ��������ܵ�������ָ��
�������    : XS32* pLen           �� ����ʱ����1��ʾ�������е�����
�������    : t_SCTPIPADDR* pFromAddr  ��Ŀ���ַָ��(����Ϊ��)
�������    : XVOID** ppSndRcvInfo  - ����������ݵ����ŵ���Ϣ
����ֵ      : XPUBLIC XS16
            XSUCC -   �ɹ�
            XERROR   -   ��Ҫָ��������
            XINET_CLOSE     - �Զ˹ر�
˵���������Ƿ��سɹ�������Ҳ��һ�����͵��Զˡ�
************************************/
XPUBLIC XS16 XINET_SctpRecvMsg(t_XINETFD *pSockFd, XCHAR** ppData, XS32* pLen, t_IPADDR* pFromAddr, XVOID* pSndRcvInfo);

/**********************************
��������    : XINET_SctpConnectCheck
����        : liukai
�������    : 2013��9��23��
��������    : ���sctp�����Ƿ�����
�������    : t_XINETFD *pSockFd   ��sock ������ָ��
����ֵ      : XPUBLIC XS32 ��ȡż���Զ˶˵��ip��
>0 ��������
<=0 ���ӶϿ�
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

