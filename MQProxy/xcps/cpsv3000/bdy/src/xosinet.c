/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosinet.c
**
**  description:  for windows
**
**  author: winner
**
**  date:   2006.3.7
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   winner         2006.3.7              create
**   zgq             2009.7.16         code review , modify
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                      包含头文件
-------------------------------------------------------------------------*/
#include "xosinet.h"
#include "xostrace.h"
#include "xoscfg.h"
#include "xosmem.h"
#include "xosencap.h"
#ifdef XOS_VXWORKS
#include <netinet/tcp.h>
#endif


/*-------------------------------------------------------------------------
                              模块内部宏定义
-------------------------------------------------------------------------*/
/* added a win2k specific defines in. */
#ifdef XOS_WIN32
#ifdef WIN2K
#ifndef SIO_UDP_CONNRESET
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#endif
#endif /*WIN2K*/
#endif /*XOS_WIN32*/

#define XOS_INET_IPV4ADDR_SIZE  4

/*sock 库的版本信息*/
#define XOS_INET_HIGH_VER     2
#define XOS_INET_LOW_VER      2

/* 转换 ASCII 和 int  */
#define XOS_INET_ATOI(_intVal, _asciiVal)                  \
{                                                          \
    _intVal = (10 * _intVal) + (_asciiVal - '0');           \
}

#define XOS_INET_GET_IPV4_ADDR_FRM_STRING(_value, _str)    \
{                                                          \
    XU16     _hiWord;                                       \
    XU16     _loWord;                                       \
                                                           \
    _hiWord = 0;                                            \
    _loWord = 0;                                            \
    _hiWord = XOS_PutHiByte(_hiWord, (_str[0]));            \
    _hiWord = XOS_PutLoByte(_hiWord, (_str[1]));            \
    _loWord = XOS_PutHiByte(_loWord, (_str[2]));            \
    _loWord = XOS_PutLoByte(_loWord, (_str[3]));            \
    _value  = XOS_PutLoWord(_value, _loWord);               \
    _value  = XOS_PutHiWord(_value, _hiWord);               \
}

/*-------------------------------------------------------------------------
                       模块内部结构和枚举定义
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                       模块内部全局变量
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                        模块内部函数
-------------------------------------------------------------------------*/
XBOOL g_ntltraceswitch=XFALSE;
XU32 g_closeSockFail=0;


/**********************************
函数名称    : XINET_AsciiToIpv4
作者        : winner
设计日期    : 2006年3月8日
功能描述    : 字符串到ip地址转换
参数        : XU8 numBytes
参数        : XCHAR* ipv4Addr
参数        : XU16 len
参数        : XCHAR* val
返回值  : XSTATIC XS16
************************************/
XSTATIC XS16 XINET_AsciiToIpv4(XU8 numBytes, XCHAR* ipv4Addr, XU16 len, XCHAR* val)
{
    XU8              byteCount;          /* Byte Count */
    XU8              idx;                /* Index for string*/

    idx = 0;
    for (byteCount = 0; byteCount < numBytes; byteCount++)
    {
        while((val[idx] != '.') && (idx < len))
        {
#if (INPUT_PAR_CHECK)
            if (val[idx] < '0' || val[idx] > '9')
            {
                /* Not a digit */
                return(XERROR);
            }
#endif /* (ERRCLASS & ERRCLS_DEBUG) */

            /* Convert Ascii to integer */
            XOS_INET_ATOI(ipv4Addr[byteCount], val[idx]);
            
            /* move to the next index */
            idx++;
        }
        idx++;
    }

    return(XSUCC);
}


/*-------------------------------------------------------------------------
                 模块接口函数
-------------------------------------------------------------------------*/
XPUBLIC XS16 XINET_Init(void)
{
#ifdef XOS_WIN32
    XU16     version;
    XS32     err;
    WSADATA data;

    version = MAKEWORD(XOS_INET_HIGH_VER, XOS_INET_LOW_VER);
    err = WSAStartup(version, &data);
    if (err != 0)
    {
        return(XERROR);
    }
#endif
    return(XSUCC);
}


/**********************************
函数名称    : XINET_GetNumRead
作者        : winner
设计日期    : 2006年3月7日
功能描述    : 获取当前可以读的数据长度
参数        : t_XINETFD *sockFd 描述符
参数        : XU32* dataLen 数据长度
返回值      : XPUBLIC XS16
XOS_SUCC      - successful
XOS_ERROR     - failed
************************************/
XPUBLIC XS16 XINET_GetNumRead(t_XINETFD *sockFd, XU32* dataLen)
{
    XS32 ret;                     /* temporary return value */

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((sockFd == XNULLP) || XOS_INET_INV_SOCK_FD(sockFd) ||
        (dataLen == XNULLP))
    {
        return(XERROR);
    }
#endif

#ifdef XOS_WIN32
    ret = ioctlsocket(sockFd->fd, FIONREAD, (u_long *)dataLen);
#else
#ifdef XOS_VXWORKS
    ret = ioctl(sockFd->fd, FIONREAD, (XS32)dataLen);
#else
    ret = ioctl(sockFd->fd, FIONREAD, dataLen);
#endif /* XOS_VXWORKS */
#endif /* XOS_WIN32 */

    /* For UDP socket assign the length of pending data in the
    socket recv buffer to largest datagram size.
    Removed recvfrom call & necessary processing for it. */

    if (ret == INET_ERR)
    {
    /* removed error check CONABORTED added for recvfrom call.
        Also return value changed from RCLOSED to XSUCC */
        /*  Check for reset connection */
        if ((INET_ERR_CODE == ERR_CONNREFUSED) ||
            (INET_ERR_CODE == ERR_CONNRESET))
        {
            *dataLen = 0;
            return(XSUCC);
        }

        /* removed error check ERR_WOULDBLOCK */
        if (INET_ERR_CODE == ERR_AGAIN)
        {
            *dataLen = 0;
            return(XERROR);
        }

        *dataLen = 0;
        //return(XERROR);
    }

    return(XSUCC);
} /* end of cmInetGetNumRead */


/*************************************************************************
函数名称    : XINET SendMsg
作者        : winner
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
***************************************************************************/
#ifdef XOS_VXWORKS
extern void printErrno(int);
extern unsigned long tickGet(void);
int g_tcpnodelay=1;
//test code below
int errSnd[1024]={0};
int errRetFail=0;
int errStat[4]={0};


int send_error_save(int errorCode)
{
    if(errorCode < 1024 && errorCode>=0)
    {
        errSnd[errorCode]++;
    }
    if(errorCode == -1)
    {
        errRetFail++;
    }
    return 0;
}


int send_error_print()
{
    int i;
    for(i=0;i<1024;i++)
    {
        if(errSnd[i]>0)
        {
            printf("\r\nerror[%d] = %d times occure, ",i,errSnd[i]);
            printErrno(i);
        }
    }
    printf("error[-1] = %d times occure\r\n",errRetFail);
    printf("send packet=%d,total len=%d\r\n",errStat[0],errStat[1]);
    printf("send okpacket=%d,total len=%d\r\n",errStat[2],errStat[3]);
    return 0;
}


int send_error_clear()
{
    memset(errSnd,0x0,sizeof(errSnd));
    memset(errStat,0x0,sizeof(errStat));
    errRetFail=0;
    return 0;
}


volatile int g_tick[3]={0};
int dog_sleep=40;
int dog_pri=255;


int xos_softdog()
{
    volatile int i=0;
    g_tick[0] = tickGet();
    g_tick[1]=g_tick[0];
    for(i=0;i<1000000;i++)
    {
        g_tick[1]=g_tick[0];
        g_tick[0] = tickGet();
        if((g_tick[0] - g_tick[1]) > g_tick[2])
        {
            g_tick[2] =g_tick[0] - g_tick[1];
        }
        if(dog_sleep >0)
        {
            taskDelay(dog_sleep);
        }
    }
    while(1)
    {
        XOS_Trace(MD(FID_COMM, PL_INFO),"soft dog feed!");
        taskDelay(10);
    }
    return 0;
}


int xos_softdogclear()
{
    g_tick[0] = tickGet();
    g_tick[1]=g_tick[0];
    g_tick[2]=0;
    return 0;
}


int xos_softdogprint()
{
    printf("soft dog max not feed time %d ticks\r\n",g_tick[2]);
    return 0;
}


int g_open=0;


int getsize_sendbuf(int sockfd)
{
    int sendbufsize,set,size=sizeof(int);
    set=getsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(char *)&sendbufsize,&size);
    if(set==-1)
    {
        printf("getsockopt sendbuf\r\n");
    }
    if(1== g_open)
    {
        printf("sockfd %d,sendbufsize = %d\r\n",sockfd,sendbufsize);
    }

    return sendbufsize;
}


/*---获取接收缓冲区大小---*/ 
int getsize_recvbuf(int sockfd)
{
    int recvbufsize,set,size=sizeof(int);
    set=getsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(char *)&recvbufsize,&size);
    if(set==-1)
    {
        perror("getsockopt recvbuf");
    }
    return recvbufsize;
}


int xos_softdoginit(int pri,int sleep_time)
{
    dog_sleep = sleep_time;
    dog_pri=pri;
    //taskSpawn("Tsk_softdog",255,0,10000,(FUNCPTR)xos_softdog,0,0,0,0,0,0,0,0,0,0);
    taskSpawn("Tsk_softdog",dog_pri,0,10000,(FUNCPTR)xos_softdog,0,0,0,0,0,0,0,0,0,0);
    g_tick[0] = tickGet();
    return 0;
}
#endif


XS16 XINET_SendMsg(t_XINETFD *pSockFd,e_LINKTYPE linkType, t_IPADDR *pDstAddr, XS32 len, XCHAR* pData,XS32 *out_len)
{
    XS32     ret;                 /* temporary return value */
    struct  sockaddr_in remAddr; /* remote Internet address */
    t_INETSOCKADDR *pSockAddrPtr = (t_INETSOCKADDR*)XNULLP;
    XS16            sizeOfAddr = 0;

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((pSockFd == XNULLP) )
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET SendMsg, bad input param,socket null");
        return(XERROR);
    }
    if (XOS_INET_INV_SOCK_FD(pSockFd))
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET SendMsg, bad input param socket invalid");
        return(XERROR);
    }
    if (len == 0)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET SendMsg, bad input param,len is 0");
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */

    /* setup remote address */
    if (pDstAddr != XNULLP)
    {
        XOS_MemSet((XU8*)&remAddr, 0, sizeof(remAddr));
        remAddr.sin_family      = AF_INET;
        remAddr.sin_port        = XOS_INET_HTON_U16(pDstAddr->port);
        remAddr.sin_addr.s_addr = XOS_INET_HTON_U32(pDstAddr->ip);
        sizeOfAddr = sizeof(remAddr);
        pSockAddrPtr= (t_INETSOCKADDR *)&remAddr;
    }
    if (len == 0)
    {
        XOS_Trace(MD(FID_NTL, PL_DBG),"XINET SendMsg,bad input param,data len is 0");
        return(XERROR);
    }
    if ((len > 0) && ((XU32)len > XOS_INET_MAX_MSG_LEN))
    {
        XOS_Trace(MD(FID_NTL, PL_DBG),"XINET SendMsg,data too long to send");
        return eErrorOverflow;
    }
    if(XNULL != out_len)
    {
        *out_len =0;
    }
    /*20080321 adjust as below*/
    ///////////////////////
    //    /*连接的udp sock*/
    //    if (pDstAddr == XNULLP)
    //    {
    //
    //        /* VxWorks sendto has some problem
    //        * with connected UDP socket, use send */
    //#ifndef XOS_VXWORKS
    //        ret = sendto(pSockFd->fd, (XCHAR *)pData, len, 0,
    //        (t_INETSOCKADDR*)XNULLP, sizeOfAddr);
    //#else
    //        ret = send(pSockFd->fd, (XCHAR*)pData, len, 0);
    //#endif /* end of SS_VW */
    //
    //    }
    //    else
    //    {
    //        ret = sendto(pSockFd->fd, (XCHAR *)pData, len, 0, pSockAddrPtr, sizeOfAddr);
    //    }
    ///////////////////////
    switch(linkType)
    {
    case eUDP:
        if (pDstAddr == XNULLP)
        {
#ifndef XOS_VXWORKS
            ret = sendto(pSockFd->fd, (XCHAR *)pData, len, 0,
                (t_INETSOCKADDR*)XNULLP, sizeOfAddr);
#else
            ret = send(pSockFd->fd, (XCHAR*)pData, len, 0);
#endif
        }
        else
        {
            ret = sendto(pSockFd->fd, (XCHAR *)pData, len, 0, pSockAddrPtr, sizeOfAddr);
        }
        break;
    case eTCPClient:
    case eTCPServer:
#ifdef XOS_VXWORKS
        errStat[0]++;/*packets*/
        errStat[1]+=len;/*total send len*/
#endif
        ret = send(pSockFd->fd, (XCHAR*)pData, len, 0);
#ifdef XOS_VXWORKS
        if(ret >0)
        {
            errStat[2]++;/*ok packets*/
            errStat[3]+=ret;/*ok total send len*/
        }else
        {
            send_error_save(INET_ERR_CODE);              
        }
#endif
        break;
    default:
        XOS_Trace(MD(FID_NTL, PL_DBG),"XINET SendMsg,unsupport msg type");
        return(XERROR);
        break;
    }
    /*20080321 adjust as above*/
    if(XNULL != out_len)
    {
        *out_len =ret;
    }
    /*获取出错的类型*/
    if (ret == INET_ERR)
    {
        if((INET_ERR_CODE == ERR_AGAIN)||
            (INET_ERR_CODE == ERR_WOULDBLOCK)||
            ((ret<len) &&(ret>0)))  /*有数据没有发送完*/
        {
            /*default send buffer is 8192 bytes,try it*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"XINET SendMsg,net block len= %d,return %d! error info:%s\n",len,ret, XOS_StrError(INET_ERR_CODE));
            return eErrorNetBlock;
        }

        /*Check if connection was closed*/
        if ((INET_ERR_CODE == ERR_PIPE) ||
            (INET_ERR_CODE == ERR_CONNABORTED) ||
            (INET_ERR_CODE == ERR_CONNRESET))
        {
            return eErrorLinkClosed;
        }
        return XERROR;
    }
    return(XSUCC);
}


/****************************************************************
函数名称    : XINET_Select
作者        : winner
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
*****************************************************************/
XPUBLIC XS16 XINET_Select(t_FDSET*  readFdS, t_FDSET *writeFdS,
                          XU32  *mSecTimeout, XS16  *numFdS)
{
    XS32 ret;                     /* temporary return value */
    struct timeval  timeout;     /* timeout structure */
    XS32 errCode;

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if (numFdS == XNULLP)
    {
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */

    *numFdS = 0;

    if (mSecTimeout != XNULLP)
    {
        timeout.tv_sec  = *mSecTimeout / 1000;
        timeout.tv_usec = (*mSecTimeout % 1000) * 1000;
    }
    else
    {
        /* delete this infinite timeout */
        timeout.tv_sec=1;
        timeout.tv_usec=500;
    }
#ifdef XOS_LINUX
    ret = select(XOS_FD_SETSIZE, (fd_set*)readFdS, (fd_set*)writeFdS, (fd_set*)XNULLP, &timeout);
#else
    ret = select(FD_SETSIZE, (fd_set*)readFdS, (fd_set*)writeFdS, (fd_set*)XNULLP, &timeout);
#endif
    if (ret == 0)
    {
        /* timeout occured */
        return(XINET_TIMEOUT);
    }

    if (ret == INET_ERR)
    {
        /*出错处理*/
        switch(errCode = INET_ERR_CODE)
        {
        /*传入的参数中定时器参数无效
            或者三个集合都是空*/
        case ERR_INVAL:
        case ERR_EINTR:
            return(XSUCC);
        case ERR_BADF:  /*集合中出现非法文件描述符*/
        default:
            XOS_Trace(MD(FID_NTL, PL_INFO), "select error: %d,%s",INET_ERR_CODE, XOS_StrError(INET_ERR_CODE));
            return(XERROR);
        } /* end of switch */
    }

    /* return number of ready file descriptors */
    *numFdS = ret;

    return(XSUCC);
} /* end of  cmInetSelect */


/********************************************************************************
函数名称    : XINET_RecvMsg
作者        : winner
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
**************************************************************************************/
XPUBLIC XS16 XINET_RecvMsg(t_XINETFD *pSockFd, t_IPADDR* pFromAddr,
                           XCHAR** ppData, XS32* pLen, XU8 linkType, XS32 ctrFlag)
{
    XS32            ret;            /* temporary return value */
    XU32            pendLen;        /* pending data length */
    XS32            bufLen;         /* entire number of received octets */
    XS32            curLen;         /* current number of octets in buffer */
    XCHAR           *pBuffer;       /*分配消息的指针*/
    XCHAR           *pBufPtr;       /* current buffer position */
    XS32            remAddrLen;     /* length of remote address */
    t_INETSOCKADDR  remSockAddr;    /* to get packet's source IP address */
    XS32            recvLen;        /* number of received octets by recvmsg() */
    struct sockaddr_in  *pRemAddr;  /* remote Internet address */

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((pSockFd == XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd) ||
        (ppData == XNULLP) || (pLen == XNULLP))
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),  "XINET_RecvMsg()-> bad input parm pSockFd:[%x] , ppData[%x]!", pSockFd, ppData);
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */

    *ppData = (XCHAR*)XNULLP;

    /* INSURE fix 2 */

    pRemAddr = ( struct sockaddr_in  *)XNULLP;

    /* clear the structure */
    XOS_MemSet((XU8*)&remSockAddr, 0, sizeof(remSockAddr));

    /* get number of pending data */
    ret = XINET_GetNumRead(pSockFd, &pendLen);
    if (ret != XSUCC)
    {
        /* ret may be XERROR or ROUTRES */
        return(ret);
    }

    /* check if connection got closed */
    if (pendLen == 0)
    {
        /* if socket is not TCP return XSUCCDNA */
        if (linkType ==XOS_INET_STREAM )
        {
            return XINET_CLOSE;
        }
        else
        {
            XOS_Trace(MD(FID_NTL, PL_ERR),  "XINET_RecvMsg()-> udp sock recv 0 bytes");
            return XERROR;
        }
    }

    /* check if there are enough pending data to read */
    if ((*pLen == XOS_INET_READ_ANY) || ((XU32)*pLen <= pendLen))
    {
        if (*pLen == XOS_INET_READ_ANY)
        {
        /*
        For TCP it can't be > XOS_INET_MAX_MSG_LEN.
            For UDP it can't be > XOS_INET_MAX_UDPRAW_MSGSIZE. */
            if (linkType == XOS_INET_STREAM)
            {
                /* max message length is limited to control the memory usage */
                if (pendLen > XOS_INET_MAX_MSG_LEN)
                {
                    pendLen = XOS_INET_MAX_MSG_LEN;
                }

                /*支持tcp 封装*/
                if(ctrFlag == XOS_INET_CTR_COMPATIBLE_TCPEEN)
                {
                    /*tcp 封装最多支持一次接受1900个字节*/
                    if(pendLen > XOS_INET_COMPATY_TCPEEN_DATA_LEN)
                    {
                        pendLen = XOS_INET_COMPATY_TCPEEN_DATA_LEN;
                    }
                }
            }
            else
            {
                if (pendLen > XOS_INET_MAX_UDPRAW_MSGSIZE)
                {
                    pendLen = XOS_INET_MAX_UDPRAW_MSGSIZE;
                }
            }
            /* read all pending data */
            bufLen = pendLen;
            *pLen = pendLen;
        }
        else
        {
            /* max message length is limited to control the memory usage */
            if (pendLen > XOS_INET_MAX_MSG_LEN)
            {
                XOS_Trace(MD(FID_NTL, PL_ERR),  "XINET_RecvMsg()-> received %l bytes,too long!", pendLen);
                return(XERROR);
            }
            /* read data length given by user */
            bufLen = *pLen;
        }

        /* set destination Internet address structure */
        if (pFromAddr == XNULLP || linkType == XOS_INET_STREAM)
        {
            remAddrLen = 0;
        }
        else
        {
            remAddrLen = sizeof(remSockAddr);
        }

        /* allocate flat receive buffer */
        pBuffer = (XCHAR*)XNULLP;

        pBuffer = (XCHAR*)XOS_MemMalloc(FID_NTL, bufLen);
        //ret = SGetSBuf(info->region, info->pool, &recvBuf, bufLen);
        if (pBuffer == XNULLP)
        {
            XOS_Trace(MD(FID_NTL, PL_EXP),  "XINET_RecvMsg()-> malloc memory failed !");
            return XERROR;
        }
        curLen = bufLen;
        pBufPtr = pBuffer;

        /*
        * maybe needs more than one recvfrom() call to read an entire
        * message from a stream socket (TCP)
        */
        while (curLen > 0)
        {
            /* added separate recvfrom calls different OS */
#ifdef XOS_VXWORKS
            if (remAddrLen)
            {
                recvLen = recvfrom(pSockFd->fd, (XCHAR*)pBufPtr, curLen, 0,
                    &remSockAddr, (int *)&remAddrLen);
            }

            else
            {
                recvLen = recv(pSockFd->fd, (XCHAR *)pBufPtr, curLen, 0);
            }

#else
#if ( defined(XOS_SOLARIS) || defined(XOS_LINUX))
            if (remAddrLen)
            {
                recvLen = recvfrom(pSockFd->fd, (XCHAR *)pBufPtr, curLen, 0,
                    (struct sockaddr *)&remSockAddr, (socklen_t *)&remAddrLen);
            }
            else
            {
                recvLen = recvfrom(pSockFd->fd, (XCHAR *)pBufPtr, curLen, 0,
                    (t_INETSOCKADDR*)XNULLP, (socklen_t *)&remAddrLen);
            }
#else
            if (remAddrLen)
            {
                recvLen = recvfrom(pSockFd->fd, (XS8 *)pBufPtr, curLen, 0,
                    &remSockAddr, (XS32*)&remAddrLen);
            }
            else
            {
                recvLen = recvfrom(pSockFd->fd, (XS8 *)pBufPtr, curLen, 0,
                    (t_INETSOCKADDR*)XNULLP, (XS32*)&remAddrLen);
            }

#endif /* defined(XOS_SOLARIS) || defined(XOS_LINUX) */
#endif /* XOS_VXWORKS */

            if (recvLen == INET_ERR)
            {
                XOS_StrError(INET_ERR_CODE);
                //XOS_Trace(MD(FID_NTL, PL_DBG),"XINET_RecvMsg()->receive return error code %d.",recvLen);
                /* cleanup */
                /* moved cleanup here */
                XOS_MemFree(FID_NTL, pBuffer);

                /*  In Windows the recvfrom function fails
                *  with error code which maps to either WSAECONNABORTED. If
                *  this happens then cmInetRecvMsg must return RCLOSED */
                if ((INET_ERR_CODE == ERR_CONNABORTED) ||
                    (INET_ERR_CODE == ERR_CONNRESET))
                {
                    *pLen = 0;
                    return(XINET_CLOSE);
                }
                return(XERROR);
            }
            curLen -= recvLen;
            pBufPtr += recvLen;

            /*
            * a message is always read atomically on a datagram socket,
            * therefore it's ok to read less than pending data!
            */

            if (linkType == XOS_INET_DGRAM)
            {
                *pLen = recvLen;
                break;
            }
        } /* while (curLen > 0) (only for stream sockets) */

          /* For UDP, it is possible to receive
        * a 0 byte datagram, in this case just return XSUCCDNA */
        if ((linkType == XOS_INET_DGRAM) && (*pLen == 0))
        {
            XOS_MemFree(FID_NTL, pBuffer);
            XOS_Trace(MD(FID_NTL, PL_EXP),  "XINET_RecvMsg()-> receive a 0 byte udp packect !");
            return(XERROR);
        }
        *ppData = pBuffer;

        /* setup return destination Internet address */
        /* added the check of (remAddrLen > 0) */
        if ((pFromAddr != XNULLP) && (remAddrLen > 0))
        {
            pRemAddr = (struct sockaddr_in *)&remSockAddr;
            pFromAddr->port    = XOS_INET_NTOH_U16(pRemAddr->sin_port);
            pFromAddr->ip  = XOS_INET_NTOH_U32(pRemAddr->sin_addr.s_addr);
        }
    }
    else
    {
        /* not enough data pending yet */
        return(XERROR);
    }

    return(XSUCC);
} /* end of cmInetRecvMsg */


/*************************************************************
函数名称    : XINET_Socket
作者        : winner
设计日期    : 2006年3月7日
功能描述    : 打开一个socket
输入参数    : XU8 type
XOS_INET_STREAM   (TCP)
XOS_INET_DGRAM    (UDP)
输出参数    : t_XINETFD* sockFd － 描述符
返回值      : XPUBLIC XS16
XOS_SUCC  - 成功
XOS_ERROR - 主要指参数错误
说明：默认是非阻塞模式
**************************************************************/
XPUBLIC XS16 XINET_Socket(XU8 type,  t_XINETFD* sockFd)
{
    XS32 ret=0; /* temporary return value */
    XU32 optVal=0;

#ifdef WIN2K
    XS32 bytesReturned=0;
    XBOOL bNewBehavior= XFALSE;
#endif /* XOS_WIN32 && WIN2K */

    /* create socket */
    sockFd->fd = socket(AF_INET, type, 0);

    if (XOS_INET_INV_SOCK_FD(sockFd))
    {
        /* Set sockFd->fd to invalid socket */
        sockFd->fd = XOS_INET_INV_SOCKFD;
        return(XERROR);
    }

    /* set default options ,非阻塞*/
    optVal = XOS_INET_OPT_DISABLE;
    ret = XINET_SetOpt(sockFd, SOL_SOCKET, XOS_INET_OPT_BLOCK, &optVal);
    if (ret != XSUCC)
    {
        ret = XINET_CloseSock(sockFd);
        return(XERROR);
    }

    /*tcp的sock 应该打开 reuse 选项*/
    if(type == XOS_INET_STREAM)
    {
        optVal = XOS_INET_OPT_DISABLE;
        ret = XINET_SetOpt(sockFd, SOL_SOCKET, XOS_INET_OPT_REUSEADDR, &optVal);
        /*为了安全，设置保活定时器*/
        /*the follow opt usage to keep the socket alive,but usually not*/
        optVal = XOS_INET_OPT_ENABLE;
        ret = XINET_SetOpt(sockFd, SOL_SOCKET, XOS_INET_OPT_KEEPALIVE, &optVal);
        if (ret != XSUCC)
        {
            ret = XINET_CloseSock(sockFd);
            return(XERROR);
        }
        optVal = XOS_INET_OPT_ENABLE;
        ret = XINET_SetOpt(sockFd, XOS_INET_LEVEL_TCP, XOS_INET_OPT_TCP_NODELAY, &optVal);

        #if 0 /*此标志不能进行统一设置，需根据不同的业务自行设置，否则，异常退出可能造成有些业务数据丢失*/
        optVal = XOS_INET_OPT_ENABLE;
        XINET_SetOpt(sockFd, SOL_SOCKET, XOS_INET_OPT_LINGER, (XU32 *)&optVal);
        #endif
    }

#ifdef XOS_LINUX
    optVal = XOS_INET_OPT_ENABLE;
    ret = XINET_SetOpt(sockFd, SOL_SOCKET, XOS_INET_OPT_BSD_COMPAT, &optVal);
    if (ret != XSUCC)
    {
        ret = XINET_CloseSock(sockFd);
        return(XERROR);
    }
#endif /* XOS_LINUX */

    /*window 2000 下udp 的sock不太一样*/
#if (defined(XOS_WIN32) && defined(WIN2K))
    if(type == XOS_INET_DGRAM)
    {
        ret = WSAIoctl(sockFd->fd, SIO_UDP_CONNRESET, &bNewBehavior,
            sizeof(bNewBehavior), XNULLP, 0, &bytesReturned,
            XNULLP, XNULLP);
        if(ret == INET_ERR)
        {
            ret = XINET_CloseSock(sockFd);
            return(XERROR);
        }
    }
#endif /* WIN2K && XOS_WIN32 */

    return(XSUCC);
} /* end of cmInetSocket */


/*****************************************************
函数名称    : XINET_Bind
作者        : winner
设计日期    : 2006年3月7日
功能描述    : 绑定一个socket
输入参数    : t_XINETFD *pSockFd  － 描述符
输入参数    : t_IPADDR *pMyAddr   －本地地址
返回值      : XPUBLIC XS16
XOS_SUCC  -  成功
XOS_ERROR -  主要指参数错误
*****************************************************/
XPUBLIC XS16 XINET_Bind(t_XINETFD   *pSockFd,  t_IPADDR *pMyAddr)
{
    XS32 ret;
    struct sockaddr_in srcAddr;
    XU32    sizeOfAddr;
    t_INETSOCKADDR *sockAddrPtr;

#if (INPUT_PAR_CHECK)
    if ((pSockFd == XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd) || (pMyAddr == XNULLP) )
    {
        return(XERROR);
    }
#endif
    XOS_MemSet((XU8*)&srcAddr, 0, sizeof(srcAddr));
    srcAddr.sin_family      = AF_INET;
    srcAddr.sin_port        = XOS_INET_HTON_U16(pMyAddr->port);
    srcAddr.sin_addr.s_addr = XOS_INET_HTON_U32(pMyAddr->ip);
    sizeOfAddr              = sizeof(struct sockaddr_in);
    sockAddrPtr             = (t_INETSOCKADDR *)&srcAddr;

    ret = bind(pSockFd->fd,sockAddrPtr,sizeOfAddr);
    if (ret == INET_ERR)
    {
        //add 2007/09/09
        XOS_Trace(MD(FID_NTL, PL_ERR),"XINET_Bind sockFd %d failed,error info(%s),input addr[ip=0x%x;port=%d]",
            pSockFd->fd,XOS_StrError(INET_ERR_CODE),pMyAddr->ip, pMyAddr->port);

        ret = XINET_CloseSock(pSockFd);
        return(XERROR);
    }

    return(XSUCC);
} /* end of cmInetBind */


/*****************************************************
函数名称    : XINET_IsTcpPortUnused
作者        : maliming
设计日期    : 2014年3月10日
功能描述    : 测试tcp端口可用
输入参数    : t_IPADDR *pMyAddr   －本地地址
返回值      : XPUBLIC XS16
XOS_SUCC  -      端口可用
XOS_ERROR -  主要指参数错误或端口不可用
*****************************************************/
XPUBLIC XS16 XINET_IsTcpPortUnused(t_IPADDR *pMyAddr)
{
    XS32 ret = 0;
    t_XINETFD sockFd;
    struct sockaddr_in srcAddr;
    XU32    sizeOfAddr = 0;
    t_INETSOCKADDR *sockAddrPtr = NULL;

#if (INPUT_PAR_CHECK)
    if ((pMyAddr == XNULLP) )
    {
        return(XERROR);
    }
#endif

    ret = XINET_Socket(XOS_INET_STREAM, &(sockFd));
    if(XSUCC != ret)/*资源不足*/
    {
        return XERROR;
    }

    XOS_MemSet((XU8*)&srcAddr, 0, sizeof(srcAddr));
    srcAddr.sin_family      = AF_INET;
    srcAddr.sin_port        = XOS_INET_HTON_U16(pMyAddr->port);
    srcAddr.sin_addr.s_addr = XOS_INET_HTON_U32(pMyAddr->ip);
    sizeOfAddr              = sizeof(struct sockaddr_in);
    sockAddrPtr             = (t_INETSOCKADDR *)&srcAddr;

    ret = bind(sockFd.fd,sockAddrPtr,sizeOfAddr);
    if (ret == INET_ERR) /*端口被占用*/
    {
        ret = XINET_CloseSock(&sockFd);
        return XERROR;
    }

    XINET_CloseSock(&sockFd);

    return XSUCC;
} 

/*******************************************************
函数名称    : XINET_CloseSock
作者        : winner
设计日期    : 2006年3月7日
功能描述    :
参数        : t_XINETFD *sockFd
返回值      : XPUBLIC XS16 － 描述符
XOS_SUCC  -  成功
XOS_ERROR -  主要指参数错误
*********************************************************/
XPUBLIC XS16 XINET_CloseSock(t_XINETFD *pSockFd)
{
    XS32 ret = 0;

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((pSockFd == XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd))
    {
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */
#ifdef XOS_NEED_CHK
#ifdef XOS_WIN32
    ret = closesocket(pSockFd->fd);
#else
    ret = close(pSockFd->fd);
#endif /* XOS_WIN32 */
#endif
    //add 2007/09/12 8888 ,it maybe dangerous
    
    if (ret == INET_ERR)
    {
        g_closeSockFail++;
        XOS_Trace(MD(FID_NTL, PL_WARN), "close sock : %d failed", pSockFd->fd);
        return(XERROR);
    }
    pSockFd->fd = XOS_INET_INV_SOCKFD;
    return(XSUCC);
}


/*****************************************************************
函数名称    : XINET_Connect
作者        : winner
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
*****************************************************************/
XPUBLIC XS16 XINET_Connect(t_XINETFD   *pSockFd, t_IPADDR *pServAddr)
{
    XS32 ret;
    struct sockaddr_in remote_addr;
    XS32    sizeOfAddr;

    if ((pSockFd == XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd) ||
        (pServAddr == XNULLP))
    {
        return(XERROR);
    }

#ifdef XOS_VXWORKS
    remote_addr.sin_len = sizeof(struct sockaddr_in);
#endif
    XOS_MemSet((void*)(&remote_addr),0x00,sizeof(struct sockaddr_in));
    remote_addr.sin_family      = AF_INET;
    remote_addr.sin_addr.s_addr = XOS_INET_HTON_U32(pServAddr->ip);
    remote_addr.sin_port        = XOS_INET_HTON_U16(pServAddr->port);
    sizeOfAddr                  = sizeof(struct sockaddr_in);
    /*8888,connect parameter 2,3 confuse,check*/
    ret = connect(pSockFd->fd,(t_INETSOCKADDR *)&remote_addr,sizeOfAddr);
    if (ret == INET_ERR)
    {

        switch (INET_ERR_CODE)
        {
            /* non-blocking: connection is in progress */
        case ERR_INPROGRESS:
            return(XINET_INPROGRESS);
            break;

            /*
            * non-blocking: connection is established
            * blocking    : connection is already established
            */
        case ERR_ISCONN: /* connect ok */
            return (XSUCC);
            break;

            /* resource temporarily unavailable */
        case ERR_WOULDBLOCK:
            /* non-blocking: connection is in progress */
            return(XINET_INPROGRESS);
            break;

        case ERR_ALREADY:
            return(XINET_INPROGRESS);
            break;

        case ERR_INVAL:
            return(XINET_INPROGRESS);
            break;

            /*  Check for connection refused and timeout errors */
        case ERR_CONNREFUSED:
        case ERR_TIMEDOUT:
            XOS_Trace(MD(FID_NTL, PL_WARN), "connect error: %s",XOS_StrError(INET_ERR_CODE));

            return XINET_CLOSE;
            break;

            /* it is a real error */
        default:
            XOS_Trace(MD(FID_NTL, PL_EXP), "connect error: %s",XOS_StrError(INET_ERR_CODE));

            return(XERROR);
            break;
        }
    }

    return(XSUCC);
} /* end of cmInetConnect */


/*******************************************************
函数名称    : XINET_Listen
作者        : winner
设计日期    : 2006年3月7日
功能描述    : 监听一个描述符
参数        : t_XINETFD *pSockFd － 描述符
参数        : XS16 backLog       －容许同时连接的最大个数
返回值      : XPUBLIC XS16
XOS_SUCC  -  成功
XOS_ERROR -  错误
*********************************************************/
XPUBLIC XS16 XINET_Listen(t_XINETFD *pSockFd, XS16 backLog)
{
    XS32 ret;

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((pSockFd == (t_XINETFD*)XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd) ||
        (backLog < MIN_BACK_LOG) || (backLog > MAX_BACK_LOG))
    {
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */

    ret = listen(pSockFd->fd, backLog);
    if (ret == INET_ERR)
    {
        return(XERROR);
    }

    return(XSUCC);
} /* end of cmInetListen */


/*******************************************************
函数名称    : XINET_StrToIpAddr
作者        : winner
设计日期    : 2006年3月7日
功能描述    : 字符串到ip地址转换
输入参数    : XU16  len   － 字符串的长度
输入参数    : XCHAR *val  －字符串首地址
输出参数    : XU32 *ip    －地址
返回值      : XPUBLIC XS16
XOS_SUCC  -  成功
XOS_ERROR -  错误
*********************************************************/
XPUBLIC XS16 XINET_StrToIpAddr(XU16  len, XCHAR *val, XU32 *ip)
{
    XU8  idx; /* Index for string*/
    XU8  ipv4[XOS_INET_IPV4ADDR_SIZE]; /* IPV4 Address bytes */
    idx = 0;

    XOS_MemSet((XU8 *)ipv4, 0, XOS_INET_IPV4ADDR_SIZE);

    /* Check for IP Address */
    while ((val[idx] != '.') && (val[idx] != ':') && (idx < len))
    {
#if (INPUT_PAR_CHECK)
        if (((val[idx] < '0') || (val[idx] > '9')) &&
            ((val[idx] < 'a') || (val[idx] > 'f')) &&
            ((val[idx] < 'A') || (val[idx] > 'F')))
        {
            /* Not a digit */
            return(XERROR);
        }
#endif /* (ERRCLASS & ERRCLS_DEBUG) */

        /* Convert Ascii to integer */
        XOS_INET_ATOI(ipv4[0], val[idx]);

        idx++; /* move to the next character */
    } /* while, try to determine IPV4 or IPV6 */

    if (val[idx] == '.')
    {
        idx++;
        XINET_AsciiToIpv4(3, (XCHAR*)&(ipv4[1]), (XU16)(len - idx), &(val[idx]));

        XOS_INET_GET_IPV4_ADDR_FRM_STRING( *ip, ipv4);
    } /* if, IPV4 */

    return(XSUCC);
} /* cmInetConvertStrToIpAddr */

XPUBLIC XS16 XINET_TcpConnectCheck(t_XINETFD *sockFd)
{
    struct sockaddr_in s;
    int i = 0;
#ifdef XOS_NEED_CHK
    int get_rt=0;
#endif

    if ((sockFd == XNULLP) || XOS_INET_INV_SOCK_FD(sockFd))
    {
        return (XERROR);
    }
    i = sizeof(s);
    memset(&s,0x0,i);
#ifdef XOS_NEED_CHK
#ifdef XOS_VXWORKS
    get_rt=getpeername(sockFd->fd, (struct sockaddr*)&s, &i);
    if(get_rt != XSUCC)
    {
        return  XERROR;
    }else
    {
        return  XSUCC;
    }
#endif
    //#if ( XOS_LINUX || XOS_SOLARIS || XOS_WIN32 )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_WIN32) )
    get_rt=getpeername(sockFd->fd, (struct sockaddr*)&s, (socklen_t *)&i);
    if(get_rt == 0)
    {
        return XSUCC;
    }
    else
    {
        return  XERROR;
    }
#endif
#endif
    //HELP!  NEED TO RESUME THREAD THAT WAS SUSPENDED ABOVE.
    //ONCE 's' FILE DESCRIPTOR IS WRITABLE, CHECK STATUS WITH getsockopt
    /*
    int  errorVal=0;
    int  errorSize=0;
    int  get_rt=0;
    errorSize = sizeof(errorVal);
    get_rt = getsockopt(sockFd->fd, SOL_SOCKET, SO_ERROR,(char *)&errorVal,&errorSize);
    if( get_rt  < 0 || errorVal!=0)
    {
    XOS_StrError(errorVal);
    *value = errorVal;
    return XINET_INPROGRESS;
    }
    */
    return XERROR;
}


/*******************************************************
函数名称    : XINET_SetOpt
作者        : winner
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
*********************************************************/
XPUBLIC XS16 XINET_SetOpt(t_XINETFD *sockFd, XU32 level,XU32 type, XU32* value)
{
    XS32  ret = XSUCC;              /* temporary return value */
    XU32  disable  = 0;            /* disable  option */
    XU32  enable  = 1;             /* enable  option */
#if defined(XOS_SCTP) && defined(XOS_LINUX)
    struct sctp_event_subscribe events;
    struct sctp_initmsg initmsg;
    struct sctp_paddrparams params;
    struct sctp_assocparams assocparm;
#endif
#if 0
    /* added for IPv4 options */
#ifdef IPV4_OPTS_SUPPORTED
#ifdef XOS_WIN32
    int disableOpt = 0;
#endif /* XOS_WIN32 */
#endif /* IPV4_OPTS_SUPPORTED */
    XU8   lpEnable = 1;           /* multicast loop enable  */
    XU8   lpDisable = 0;          /* multicast loop disable  */
#ifdef XOS_WIN32
#endif /* XOS_WIN32 */
    struct ip_mreq stMreq;
    CmInetMCastInf *mCast;
#endif

    XBOOL  boolEnable = XTRUE;      /* enable option */
    XBOOL  boolDisable = XFALSE;    /* disable option */
    XU32    *optVal;
    struct linger optLinger;

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((sockFd == XNULLP) || XOS_INET_INV_SOCK_FD(sockFd))
    {
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */

    switch (type)
    {
    case XOS_INET_OPT_BLOCK:
        //To make a socket non-blocking :
        //int val = 1; /* Set to 0 for blocking I / O */
        //ioctl (sock, FIONBIO, &val);
        optVal = (XU32*)value;
        switch(*optVal)
        {
        case XOS_INET_OPT_ENABLE:
#ifdef XOS_WIN32
            ret = ioctlsocket(sockFd->fd, FIONBIO, (u_long *)&disable );
#else
#ifdef XOS_VXWORKS
            ret = ioctl(sockFd->fd, (XS32)FIONBIO, (XS32)&disable );
#else
            ret = ioctl(sockFd->fd, (XS32)FIONBIO, &disable );

#endif /* XOS_VXWORKS */
            
#endif /* XOS_WIN32 */
            sockFd->blocking = 1;
            break;

        case XOS_INET_OPT_DISABLE:
#ifdef XOS_WIN32
            ret = ioctlsocket(sockFd->fd, FIONBIO, (u_long*)&enable );
#else
#ifdef XOS_VXWORKS
            ret = ioctl(sockFd->fd, (XS32)FIONBIO, (XS32)&enable );
#else
            ret = ioctl(sockFd->fd, (XS32)FIONBIO, &enable );
#endif /* XOS_VXWORKS */

#endif /* XOS_WIN32 */

            sockFd->blocking = 0;
            break;

        default:
            /* wrong value */
            return(XERROR);
            break;
        }
        break;

        case XOS_INET_OPT_RX_BUF_SIZE:
            optVal = (XU32*)value;
            ret = setsockopt(sockFd->fd, level, SO_RCVBUF,
                (char*)optVal, sizeof(*optVal));
            break;

        case XOS_INET_OPT_TX_BUF_SIZE:
            optVal = (XU32*)value;
            ret = setsockopt(sockFd->fd, level, SO_SNDBUF,
                (char*)optVal, sizeof(*optVal));
            break;

        case XOS_INET_OPT_REUSEADDR:
            optVal = (XU32*)value;
            if (*optVal == XOS_INET_OPT_ENABLE)
            {
                ret = setsockopt(sockFd->fd, level, SO_REUSEADDR,
                    (char*)&boolEnable, sizeof(boolEnable));
            }
            else if (*optVal == XOS_INET_OPT_DISABLE)
            {
                ret = setsockopt(sockFd->fd, level, SO_REUSEADDR,
                    (char*)&boolDisable, sizeof(boolDisable));
            }
            break;

        case  XOS_INET_OPT_LINGER:
            XOS_MemSet(&optLinger, 0, sizeof(optLinger));
            optVal = (XU32*)value;
            if (*optVal == XOS_INET_OPT_ENABLE)
            {
                optLinger.l_onoff = 1;
                optLinger.l_linger = 0;
                ret = setsockopt(sockFd->fd, level, SO_LINGER,
                    (char*)&optLinger, sizeof(optLinger));
            }
            else if (*optVal == XOS_INET_OPT_DISABLE)
            {
                optLinger.l_onoff = 0;
                optLinger.l_linger = 1;
                ret = setsockopt(sockFd->fd, level, SO_LINGER,
                    (char*)&optLinger, sizeof(optLinger));
            }
            break;

        case XOS_INET_OPT_BROADCAST:
            optVal = (XU32*)value;
            if (*optVal == XOS_INET_OPT_ENABLE)
            {
                ret = setsockopt(sockFd->fd, level, SO_BROADCAST,
                    (char*)&boolEnable, sizeof(boolEnable));
            }
            else if (*optVal == XOS_INET_OPT_DISABLE)
            {
                ret = setsockopt(sockFd->fd, level, SO_BROADCAST,
                    (char*)&boolDisable, sizeof(boolDisable));
            }
            break;

        case XOS_INET_OPT_KEEPALIVE:
            optVal = (XU32*)value;
            if (*optVal == XOS_INET_OPT_ENABLE)
            {
                ret = setsockopt(sockFd->fd, level, SO_KEEPALIVE,
                    (char*)&boolEnable, sizeof(boolEnable));
            }
            else if (*optVal == XOS_INET_OPT_DISABLE)
            {
                ret = setsockopt(sockFd->fd, level, SO_KEEPALIVE,
                    (char*)&boolDisable, sizeof(boolDisable));
            }
            break;


        case XOS_INET_OPT_TCP_NODELAY:
#ifdef XOS_VXWORKS
            if(g_tcpnodelay==1)
            {
                optVal = (XU32*)value;
                if (*optVal == XOS_INET_OPT_ENABLE)
                {
                    ret = setsockopt(sockFd->fd, level, TCP_NODELAY,
                        (char*)&boolEnable, sizeof(boolEnable));
                }
                else if (*optVal == XOS_INET_OPT_DISABLE)
                {
#ifndef SS_WINCE
                    ret = setsockopt(sockFd->fd, level, TCP_NODELAY,
                        (char*)&boolDisable, sizeof(boolDisable));
#endif /* SS_WINCE */
                }
            }
#endif
            break;

#if defined(XOS_SCTP) && defined(XOS_LINUX)
        case XOS_INET_OPT_SCTP_NODELAY:
        optVal = (XU32*)value;
        if (*optVal == XOS_INET_OPT_ENABLE)
        {
            ret = setsockopt(sockFd->fd, level, SCTP_NODELAY,(char*)&boolEnable, sizeof(boolEnable));
        }
        else if (*optVal == XOS_INET_OPT_DISABLE)
        {
            ret = setsockopt(sockFd->fd, level, SCTP_NODELAY,(char*)&boolDisable, sizeof(boolDisable));
        }

        break;


        case XOS_INET_OPT_SCTP_EVENT:
            memset((void *)&events, 0, sizeof(events));
            events.sctp_data_io_event = *value;
            ret = setsockopt(sockFd->fd, level, SCTP_EVENTS,
                        &events, sizeof(struct sctp_event_subscribe)-1); /*sizeof()-1,兼容低版本少一个字节成员*/
            break;
            
        case XOS_INET_OPT_SCTP_OUTSTREAM:
            memset((void *)&initmsg, 0, sizeof(initmsg));
            initmsg.sinit_num_ostreams = *value;
            ret = setsockopt(sockFd->fd, level, SCTP_INITMSG,
                        &initmsg, sizeof(struct sctp_initmsg));
            break;

            
        case XOS_INET_OPT_SCTP_HB:
            memset(&params,0,sizeof(params));
            params.spp_hbinterval = *value;
            params.spp_flags = SPP_HB_ENABLE;
            ret = setsockopt(sockFd->fd,level,SCTP_PEER_ADDR_PARAMS,(const void *)&params,sizeof(struct sctp_paddrparams));
            break;

        case XOS_INET_OPT_SCTP_PATHMAXRXT:
            memset(&params,0,sizeof(params));
            params.spp_pathmaxrxt = *value;
            params.spp_flags = SPP_PMTUD_ENABLE;
            ret = setsockopt(sockFd->fd,level,SCTP_PEER_ADDR_PARAMS,(const void *)&params,sizeof(struct sctp_paddrparams));
            break;

        case XOS_INET_OPT_SCTP_ASSOCINFO:
            memset(&assocparm,0,sizeof(assocparm));
            assocparm.sasoc_asocmaxrxt = *value;
            ret = setsockopt(sockFd->fd,level,SCTP_ASSOCINFO,(const void *)&assocparm,sizeof(struct sctp_assocparams));
            break;

#endif
#if 0

        case XOS_INET_OPT_ADD_MCAST_MBR:
            mCast = (CmInetMCastInf*)value;

            /* Copy the addresses to stMreq structure */
            stMreq.imr_multiaddr.s_addr = XOS_INET_HTON_XU32(mCast->mCastAddr);
            stMreq.imr_interface.s_addr = XOS_INET_HTON_XU32(mCast->localAddr);

            ret = setsockopt(sockFd->fd, level, IP_ADD_MEMBERSHIP,
                (char*)&stMreq, sizeof(stMreq));
            break;

        case XOS_INET_OPT_DRP_MCAST_MBR:
            mCast = (CmInetMCastInf*)value;

            /* Copy the addresses to stMreq structure */
#ifdef SS_PS
            stMreq.imr_mcastaddr.s_addr = XOS_INET_HTON_XU32(mCast->mCastAddr);
#else
            stMreq.imr_multiaddr.s_addr = XOS_INET_HTON_XU32(mCast->mCastAddr);
#endif
            stMreq.imr_interface.s_addr = XOS_INET_HTON_XU32(mCast->localAddr);

            ret = setsockopt(sockFd->fd, level, IP_DROP_MEMBERSHIP,
                (char*)&stMreq, sizeof(stMreq));
            break;

#if (defined(SUNOS)|| defined(XOS_WIN32) || defined(SS_PS) || defined(SS_VW_MCAST) \
            || defined(HPOS))
        case XOS_INET_OPT_MCAST_LOOP:
            optVal = (XU32*)value;
            if (*optVal == XOS_INET_OPT_ENABLE)
            {
#ifdef SS_VW
                ret = setsockopt(sockFd->fd, level, IP_MULTICAST_LOOP,
                    (char *)&lpEnable, sizeof(lpEnable));
#else
                ret = setsockopt(sockFd->fd, level, IP_MULTICAST_LOOP,
                    (CONSTANT char *)&lpEnable, sizeof(lpEnable));
#endif /* SS_VW */
            }
            else
            {
#ifdef SS_VW
                ret = setsockopt(sockFd->fd, level, IP_MULTICAST_LOOP,
                    (char *)&lpDisable, sizeof(lpDisable));
#else
                ret = setsockopt(sockFd->fd, level, IP_MULTICAST_LOOP,
                    (CONSTANT char *)&lpDisable, sizeof(lpDisable));
#endif /* SS_VW */
            }
            break;

        case XOS_INET_OPT_MCAST_IF:
            optVal = (XU32*)value;
            *optVal = XOS_INET_HTON_XU32((XU32)*optVal);
            ret = setsockopt(sockFd->fd, level, IP_MULTICAST_IF,
                (char *)optVal, sizeof(struct in_addr));
            break;

        case XOS_INET_OPT_MCAST_TTL:
            optVal = (XU32*)value;
            /* remove CONSTANT in setsockopt for VW */
#ifdef SS_VW
            ret = setsockopt(sockFd->fd, level, IP_MULTICAST_TTL,
                (char *)optVal, sizeof(XU8));
#else
            ret = setsockopt(sockFd->fd, level, IP_MULTICAST_TTL,
                (CONSTANT char *)optVal, sizeof(XU8));
#endif /* SS_VW */
            break;
#endif /* SUNOS || XOS_WIN32 || SS_PS || SS_VW_MCAST || HPOS */

        default:
            /* wrong socket option type */
            return(XERROR);
            break;
#endif
    }

    if (ret == INET_ERR)
    {
        return(XERROR);
    }
    return(XSUCC);
}
/* end of cmInetSetOpt */


/*******************************************************
函数名称    : XINET_Accept
作者        : winner
设计日期    : 2006年3月7日
功能描述    : 接受一个连接
输入参数    : t_XINETFD   *pSockFd  - 描述符
输出参数    : t_IPADDR *pFromAddr   -对端地址
输出参数        : t_XINETFD  *pNewSockFd
返回值      : XPUBLIC XS16
XOS_SUCC  -  成功
XOS_ERROR -  错误
说明：默认是非阻塞模式
*********************************************************/
XPUBLIC XS16 XINET_Accept(t_XINETFD   *pSockFd, t_IPADDR *pFromAddr, t_XINETFD  *pNewSockFd)
{
    XS32 ret;                         /* temporary return value */
    XS32 addrLen;                     /* address structure length */
    struct sockaddr_in  *peerAddr;   /* calling Internet address/port */
    t_INETSOCKADDR sockAddr;
    //struct sockaddr_in sockAddr;
    XU32 optVal;

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((pSockFd == XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd))
    {
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */

    /* change t_INETSOCKADDR to sockAddr */
    addrLen = sizeof(sockAddr);

    /* INSURE fix */
#if ( defined(SUNOS) || defined(XOS_LINUX))
    pNewSockFd->fd = accept(pSockFd->fd, (t_INETSOCKADDR*)&sockAddr,
        (socklen_t *)&addrLen);
#else
    pNewSockFd->fd = accept(pSockFd->fd, (t_INETSOCKADDR*)&sockAddr,
        (int*)&addrLen);
#endif /* SUNOS || SS_LINUX */

    /* added for IPv6/IPv4 socket distinguishing */
    if (XOS_INET_INV_SOCK_FD(pNewSockFd))
    {
        if (INET_ERR_CODE == ERR_WOULDBLOCK)
        {
            /* no connection present to accept */
            return(XERROR);
        }
        else
        {
            XOS_Trace(MD(FID_NTL, PL_ERR),"XINET_Accept error info:%s",XOS_StrError(INET_ERR_CODE));
            return(XERROR);
        }
    }

    /* set default options for new socket file descriptor */
    optVal = XOS_INET_OPT_ENABLE;
    XINET_SetOpt(pNewSockFd, SOL_SOCKET, XOS_INET_OPT_LINGER, &optVal);

    optVal = XOS_INET_OPT_DISABLE;
    ret = XINET_SetOpt(pNewSockFd, SOL_SOCKET, XOS_INET_OPT_BLOCK, &optVal);
    if ( ret != XSUCC)
    {
        ret = XINET_CloseSock(pNewSockFd);
        return(XERROR);
    }

    peerAddr = (struct sockaddr_in *)&sockAddr;
    pFromAddr->port    = XOS_INET_NTOH_U16(peerAddr->sin_port);
    pFromAddr->ip = XOS_INET_NTOH_U32(peerAddr->sin_addr.s_addr);

    return(XSUCC);
} /* end of cmInetAccept */


/*******************************************************
函数名称    : XINET_GetSockName
作者        : winner
设计日期    : 2006年3月7日
功能描述    : 获取sock 相关联的本地地址
输入参数    : t_XINETFD *sockFd -描述符
输出参数    : t_IPADDR* locAddr号   -本地地址(主机字节序)
返回值      : XPUBLIC XS16
XOS_SUCC  - 成功
XOS_ERROR - 主要指参数错误
*********************************************************/
XPUBLIC XS16 XINET_GetSockName(t_XINETFD *sockFd, t_IPADDR* locAddr)
{
    struct sockaddr_in *sockAddr;
    t_INETSOCKADDR lclSockAddr;
    XU32  size;
    XS16  ret;
    XS32  errCode;

#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((sockFd == XNULLP) || XOS_INET_INV_SOCK_FD(sockFd) ||
        (locAddr == XNULLP))
    {
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */

    XOS_MemSet((XU8*)&lclSockAddr, 0, sizeof(lclSockAddr));
    size = sizeof(lclSockAddr);

#ifdef XOS_LINUX
    ret = getsockname(sockFd->fd, (t_INETSOCKADDR*)&lclSockAddr,
        (socklen_t *)&size);
#else
    ret = getsockname(sockFd->fd, (t_INETSOCKADDR*)&lclSockAddr, (int*)&size);
#endif

    if(ret == INET_ERR)
    {
        switch(errCode = INET_ERR_CODE)
        {
        case ERR_INVAL:
            sockAddr = (struct sockaddr_in *)&lclSockAddr;
            locAddr->port = XOS_INET_NTOH_U16(sockAddr->sin_port);
            return(XSUCC);

        default:
            return(XERROR);
        }/* end of switch */
    }

    sockAddr = (struct sockaddr_in *)&lclSockAddr;
    locAddr->port    = XOS_INET_NTOH_U16(sockAddr->sin_port);
    locAddr->ip = XOS_INET_NTOH_U32(sockAddr->sin_addr.s_addr);

    return(XSUCC);
}

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
XPUBLIC XS16  XINET_GetMac(XU8 *ifname, XU8 *mac)   
{
#ifdef XOS_LINUX
#ifndef IFNAMSIZ    
#define IFNAMSIZ (16)
#endif

    int sock,   ret;   
    struct ifreq ifr;   

    if(NULL == ifname || NULL == mac)
    {
        return XERROR;
    }

    /*接口名称长度*/
    if(strlen((char*)ifname) >= IFNAMSIZ)
    {
       return XERROR;
    }
    
    sock = socket(AF_INET, SOCK_STREAM, 0);   
    if(XOS_INET_INV_SOCKFD == sock)
    {   
        perror("socket");   
        return   XERROR;   
    }
    
    memset(&ifr, 0, sizeof(ifr));   
    strcpy(ifr.ifr_name, (char*)ifname);   
    ret = ioctl(sock, SIOCGIFHWADDR, &ifr, sizeof(ifr));  
    close(sock); 
    
    if(ret == 0)   
    {
        memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);   
    }   
    else
    {
        perror("ioctl");  
        return XERROR;
    }    
#endif    
    return XSUCC;   
}


/*******************************************************
函数名称    : XINET_End
作者        : winner
设计日期    : 2006年3月7日
功能描述    :
参数        : void
返回值      : XPUBLIC XS16
*********************************************************/
XPUBLIC XS16 XINET_End(void)
{
#ifdef XOS_WIN32
    XS32     err;
    err = WSACleanup();
    if (err != 0)
    {
        return(XERROR);
    }
#endif
    return(XSUCC);
}

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
XPUBLIC XS16 XINET_GetOpt(t_XINETFD *sockFd, XU32 level,XU32 type, XU32* value)
{
    XS32 ret = XSUCC;              /* temporary return value */
    XS32 len = 0;
#if defined(XOS_SCTP) && defined(XOS_LINUX)
    struct sctp_status status;
#endif
#if (INPUT_PAR_CHECK)
    /* error check on parameters */
    if ((sockFd == XNULLP) || XOS_INET_INV_SOCK_FD(sockFd))
    {
        return(XERROR);
    }
    if (value == XNULLP) 
    {
        return(XERROR);
    }

#endif /* INPUT_PAR_CHECK */

    switch (type)
    {
#if defined(XOS_SCTP) && defined(XOS_LINUX)
    case XOS_INET_OPT_SCTP_PEER_INSTREAM:
        XOS_MemSet(&status,0,sizeof(status));
        len = sizeof(status);

        ret = getsockopt(sockFd->fd, level,SCTP_STATUS,&status,(socklen_t *)&len);
        if(ret == INET_ERR)
        {
            XOS_Trace(MD(FID_NTL, PL_ERR),"XINET_GetOpt,fd:%d,len:%d,err info:%s",sockFd->fd,len,XOS_StrError(INET_ERR_CODE));
            return XERROR;
        }
        *value = status.sstat_outstrms;
        break;
#endif
    default:
    break;
    }
    if (ret == INET_ERR)
    {
        return(XERROR);
    }
    return(XSUCC);

}

/*************************************************************
函数名称    : XINET_SctpSocket
作者        : liukai
设计日期    : 2013年9月22日
功能描述    : 创建一个socket
输入参数    : XU32 stream      - 流数量
输出参数    : t_XINETFD* sockFd － 描述符
返回值      : XPUBLIC XS16
XOS_SUCC  - 成功
XOS_ERROR - 主要指参数错误
说明：默认是非阻塞模式
**************************************************************/
XPUBLIC XS16 XINET_SctpSocket(t_XINETFD* sockFd, XU32 stream)
{
    XS32 ret=0; /* temporary return value */
    XU32 optVal=0;

    sockFd->fd = socket(AF_INET, XOS_INET_STREAM, IPPROTO_SCTP);
    if (XOS_INET_INV_SOCK_FD(sockFd))
    {
        /* Set sockFd->fd to invalid socket */
        sockFd->fd = XOS_INET_INV_SOCKFD;
        return(XERROR);
    }

    /* set default options ,非阻塞*/
    optVal = XOS_INET_OPT_DISABLE;
    ret = XINET_SetOpt(sockFd, SOL_SOCKET, XOS_INET_OPT_BLOCK, &optVal);
    if (ret != XSUCC)
    {
        ret = XINET_CloseSock(sockFd);
        return(XERROR);
    }

    /*类似tcp协议，参考tcp设置*/
    optVal = XOS_INET_OPT_DISABLE;
    ret = XINET_SetOpt(sockFd, SOL_SOCKET, XOS_INET_OPT_REUSEADDR, &optVal);
    
    /*为了安全，设置保活定时器*/
    optVal = XOS_INET_OPT_ENABLE;
    ret = XINET_SetOpt(sockFd, SOL_SOCKET, XOS_INET_OPT_KEEPALIVE, &optVal);
    if (ret != XSUCC)
    {
        ret = XINET_CloseSock(sockFd);
        return(XERROR);
    }

    /*关闭nagle算法*/
    optVal = XOS_INET_OPT_ENABLE;
    ret = XINET_SetOpt(sockFd, XOS_INET_LEVEL_SCTP, XOS_INET_OPT_SCTP_NODELAY, &optVal);
    if (ret != XSUCC)
    {
        ret = XINET_CloseSock(sockFd);
        return(XERROR);
    }
    
    /*设置事件属性，接收数据时获取相关状态*/
    optVal = XOS_INET_OPT_ENABLE;
    ret = XINET_SetOpt( sockFd, XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_EVENT,&optVal);
    if (ret != XSUCC)
    {
        ret = XINET_CloseSock(sockFd);
        return(XERROR);
    }

    /*设置本端出流数量，入流数量由对端决定*/
    optVal = stream;
    ret = XINET_SetOpt( sockFd, XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_OUTSTREAM,&optVal);
    if (ret != XSUCC)
    {
        ret = XINET_CloseSock(sockFd);
        return(XERROR);
    }

    return(XSUCC);
} /* end of cmInetSocket */

/**********************************
函数名称    : XINET_SctpBind
作者        : liukai
设计日期    : 2013年9月13日
功能描述    : 绑定一个socket
输入参数    : t_XINETFD *pSockFd  － 描述符
            : t_SCTPIPADDR *pMyAddr   －本地地址集
            : XPUBLIC XS16 XS32 flags  - SCTP_BINDX_ADD_ADDR 增加地址；
                                        - SCTP_BINDX_ADD_REM 删除地址
返回值          XOS_SUCC  -  成功
                XOS_ERROR -  主要指参数错误
************************************/
XPUBLIC XS16 XINET_SctpBind(t_XINETFD *pSockFd, t_SCTPIPADDR *pMyAddr, XS32 flags)
{
    XS32 ret = 0,i = 0;
    struct sockaddr_in *srcAddr = NULL;
    XU32    sizeOfAddr;

    if (pSockFd == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpBind, bad input param,socket null");
        return(XERROR);
    }
    if(XOS_INET_INV_SOCK_FD(pSockFd))
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpBind, bad input param,socket invalid");
        return(XERROR);
    }
    if(pMyAddr == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpBind, bad input param,pMyAddr null");
        return(XERROR);
    }
    if(pMyAddr->ipNum <= 0 || pMyAddr->ipNum > SCTP_ADDR_NUM)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpBind, bad input param,invalid ip number ");
        return(XERROR);
    }
    if(flags != SCTP_BIND_ADD && flags != SCTP_BIND_DEL)
    {
        flags = SCTP_BIND_ADD;
    }
    sizeOfAddr = sizeof(struct sockaddr_in) * pMyAddr->ipNum;
    srcAddr = XOS_MemMalloc(FID_NTL,sizeOfAddr);
    if (srcAddr == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_EXP),  "XINET_SctpBind()-> malloc memory failed !");
        return XERROR;
    }


    for(i = 0;i < pMyAddr->ipNum;i++)
    {
        srcAddr[i].sin_family      = AF_INET;
        srcAddr[i].sin_port        = XOS_INET_HTON_U16(pMyAddr->port);
        srcAddr[i].sin_addr.s_addr = XOS_INET_HTON_U32(pMyAddr->ip[i]);
    }

    ret = sctp_bindx(pSockFd->fd,(t_INETSOCKADDR *)srcAddr,pMyAddr->ipNum,flags);
    XOS_MemFree(FID_NTL,srcAddr);
    if (ret == INET_ERR)
    {
        XOS_Trace(MD(FID_NTL, PL_DBG),"XINET_SctpBind sockFd %d failed,error info(%s),input addr[ip0=0x%lx;ip1=0x%lx;port=%d;num=%d,flags=%d]",
            pSockFd->fd,XOS_StrError(INET_ERR_CODE),pMyAddr->ip[0],pMyAddr->ip[1], pMyAddr->port,pMyAddr->ipNum,flags);

        ret = XINET_CloseSock(pSockFd);
        return(XERROR);
    }
    return(XSUCC);
}

/*******************************************************
函数名称    : XINET_SctpAccept
作者        : liukai
设计日期    : 2013年9月24日
功能描述    : 接受一个sctp连接
输入参数    : t_XINETFD   *pSockFd  - 描述符
输入参数    : XU16 hb               - 设置客户端心跳间隔
输出参数    : t_SCTPIPADDR *pFromAddr   -对端所有绑定地址
输出参数    : t_XINETFD  *pNewSockFd
返回值      : XPUBLIC XS16
XOS_SUCC  -  成功
XOS_ERROR -  错误
说明：默认是非阻塞模式
*********************************************************/
XPUBLIC XS16 XINET_SctpAccept(t_XINETFD   *pSockFd, t_SCTPIPADDR *pFromAddr, XU32 hb, t_XINETFD  *pNewSockFd)
{
    XS32 ret = 0;                         /* temporary return value */
    XS32 addrNum = 0;                     /* address structure length */
    struct sockaddr_in  *peerAddr = NULL;   /* calling Internet address/port */
    t_INETSOCKADDR *sockAddr = NULL;
    XU32 optVal;
    XU32 i;
#if (INPUT_PAR_CHECK)
    if ((pSockFd == XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd))
    {
        return(XERROR);
    }
#endif /* INPUT_PAR_CHECK */

    pNewSockFd->fd = accept(pSockFd->fd, NULL,0);

    /* added for IPv6/IPv4 socket distinguishing */
    if (XOS_INET_INV_SOCK_FD(pNewSockFd))
    {
        if (INET_ERR_CODE == ERR_WOULDBLOCK)
        {
            /* no connection present to accept */
            return(XERROR);
        }
        else
        {
            XOS_Trace(MD(FID_NTL, PL_ERR),"XINET_SctpAccept error info:%s",XOS_StrError(INET_ERR_CODE));
            return(XERROR);
        }
    }
    /*非阻塞*/
    optVal = XOS_INET_OPT_DISABLE;
    ret = XINET_SetOpt(pNewSockFd, SOL_SOCKET, XOS_INET_OPT_BLOCK, &optVal);
    if ( ret != XSUCC)
    {
        ret = XINET_CloseSock(pNewSockFd);
        return(XERROR);
    }

    /*关闭nagle算法*/
    optVal = XOS_INET_OPT_ENABLE;
    ret = XINET_SetOpt(pNewSockFd, XOS_INET_LEVEL_SCTP, XOS_INET_OPT_SCTP_NODELAY, &optVal);
    if (ret != XSUCC)
    {
        ret = XINET_CloseSock(pNewSockFd);
        return(XERROR);
    }

    /*非优雅断开,直接发送abort,类似于tcp协议的reset*/
    optVal = XOS_INET_OPT_ENABLE;
    XINET_SetOpt(pNewSockFd, SOL_SOCKET, XOS_INET_OPT_LINGER, &optVal);
    if(hb > 0)
    {
        optVal = hb;
        ret = XINET_SetOpt( pNewSockFd, XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_HB,&optVal);
    }

    XOS_Trace(MD(FID_NTL, PL_DBG),"XINET_SctpAccept,hb:%d,ret:%d",hb,ret);

    /*设置事件接收,接收数据时才能获取相关状态*/
    optVal = XOS_INET_OPT_ENABLE;
    ret = XINET_SetOpt( pNewSockFd, XOS_INET_LEVEL_SCTP,XOS_INET_OPT_SCTP_EVENT,&optVal);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL, PL_INFO),"XINET_SctpAccept,set event failed!");
        ret = XINET_CloseSock(pNewSockFd);
        return(XERROR);
    }
    
    /*确认四次握手成功*/
    addrNum = sctp_getpaddrs(pNewSockFd->fd, 0, &sockAddr);
    if(addrNum <=0 )
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"XINET_SctpAccept,get peer address failed");
        return(XERROR);
    }
    if(addrNum > SCTP_ADDR_NUM)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"XINET_SctpAccept: peer client bind [%d] IP \
            address,exceeds SCTP_ADDR_NUM=8,will be truncated to 8");
        addrNum = SCTP_ADDR_NUM;
    }
    peerAddr = (struct sockaddr_in *)sockAddr;
    for(i =0; i < addrNum; i++)
    {
        pFromAddr->ip[i] = XOS_INET_NTOH_U32(peerAddr[i].sin_addr.s_addr);
    }
    pFromAddr->port = XOS_INET_NTOH_U16(peerAddr[0].sin_port);
    pFromAddr->ipNum = i;
    sctp_freepaddrs(sockAddr);
    XOS_Trace(MD(FID_NTL, PL_DBG),"XINET_SctpAccept,peer ip:%x,port:%d",pFromAddr->ip[0],pFromAddr->port);
    return(XSUCC);
}

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
XPUBLIC XS16 XINET_SctpConnect(t_XINETFD *pSockFd, t_SCTPIPADDR *pServAddr,XS32 *id)
{
    XS32 ret = 0;
    struct sockaddr_in *remote_addr = NULL;
    XS32    sizeOfAddr, i = 0;

    if (pSockFd == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpConnect, bad input param,socket null");
        return(XERROR);
    }
    if(XOS_INET_INV_SOCK_FD(pSockFd))
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpConnect, bad input param,socket invalid");
        return(XERROR);
    }
    if(pServAddr == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpConnect, bad input param,pServAddr null");
        return(XERROR);
    }
    if(pServAddr->ipNum <= 0 || pServAddr->ipNum > SCTP_ADDR_NUM)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpConnect, bad input param,invalid ip number ");
        return(XERROR);
    }
    
    sizeOfAddr = sizeof(struct sockaddr_in) * pServAddr->ipNum;

    remote_addr = (struct sockaddr_in*)XOS_MemMalloc(FID_NTL,sizeOfAddr);
    if (remote_addr == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_EXP),  "XINET_SctpConnect()-> malloc memory failed !");
        return XERROR;
    }
    for(i = 0;i < pServAddr->ipNum;i++)
    {
        remote_addr[i].sin_family      = AF_INET;
        remote_addr[i].sin_addr.s_addr = XOS_INET_HTON_U32(pServAddr->ip[i]);
        remote_addr[i].sin_port        = XOS_INET_HTON_U16(pServAddr->port);
    }
    
    ret = sctp_connectx(pSockFd->fd,(t_INETSOCKADDR *)remote_addr,pServAddr->ipNum,id);
    XOS_MemFree(FID_NTL,remote_addr);
    if (ret == INET_ERR)
    {
        switch (INET_ERR_CODE)
        {
            /* non-blocking: connection is in progress */
        case ERR_INPROGRESS:
            return(XINET_INPROGRESS);

            /*
            * non-blocking: connection is established
            * blocking    : connection is already established
            */
        case ERR_ISCONN: /* connect ok */
            return (XSUCC);

            /* resource temporarily unavailable */
        case ERR_WOULDBLOCK:
            /* non-blocking: connection is in progress */
            return(XINET_INPROGRESS);

        case ERR_ALREADY:
            return(XINET_INPROGRESS);

        case ERR_INVAL:
            return(XINET_INPROGRESS);

            /*  Check for connection refused and timeout errors */
        case ERR_CONNREFUSED:
        case ERR_TIMEDOUT:
            XOS_Trace(MD(FID_NTL, PL_WARN), "connect error info: %s",XOS_StrError(INET_ERR_CODE));

            return XINET_CLOSE;

            /* it is a real error */
        default:
            XOS_Trace(MD(FID_NTL, PL_EXP), "connect error info: %s",XOS_StrError(INET_ERR_CODE));

            return(XERROR);
        }
    }
    return(XSUCC);
}

/**********************************
函数名称    : XINET_SctpSendMsg
作者        : liukai
设计日期    : 2013年9月13日
功能描述    : 发送数据到网络
参数        : t_XINETFD *pSockFd －sock 描述符指针
参数        : XCHAR* pData       －要发送数据数据的地址
参数        : XS32 len           －要发送的数据长度
参数        : t_SCTPIPADDR *pDstAddr －目标地址集指针(可以为空)
参数        : t_SctpDataAttr attr   - 包含发送相关的参数
返回值      : XS16
XOS_SUCC          -   成功
XOS_ERROR         -   主要指参数错误
eErrorNetBlock    －网络阻塞
eErrorLinkClosed  －链路关闭
eErrorOverflow    －溢出，要发送的数据长度超过最大长度
************************************/
XS16 XINET_SctpSendMsg(t_XINETFD *pSockFd, XCHAR* pData, XS32 len , t_SCTPIPADDR *pDstAddr, t_SctpDataAttr attr)
{
    XS32     ret;                 /* temporary return value */

    if (pSockFd == XNULLP || XOS_INET_INV_SOCK_FD(pSockFd))
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpSendMsg, bad input param,socket null or invalid");
        return(XERROR);
    }
    if (len == 0)
    {
        XOS_Trace(MD(FID_NTL, PL_INFO),"XINET_SctpSendMsg,bad input param,data len is 0");
        return(XERROR);
    }
    if ((len > 0) && ((XU32)len > XOS_INET_MAX_MSG_LEN))
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpSendMsg,data too long to send");
        return(XERROR);
    }
    XOS_Trace(MD(FID_NTL,PL_DBG),"XINET SendMsg,ppid %d,stream:%d,context:%d",attr.ppid,attr.stream,attr.context);

    /*以消息为发送单位，要么发送成功，要么发送失败*/
    ret = sctp_sendmsg(pSockFd->fd, (void*)pData, len, NULL,0,attr.ppid,0,attr.stream,0,attr.context);    
    
    /*获取出错的类型*/
    if (ret == INET_ERR)
    {
        if((INET_ERR_CODE == ERR_AGAIN)||
            (INET_ERR_CODE == ERR_WOULDBLOCK))/*缓冲区不足*/
           
        {
            /*default send buffer is 8192 bytes,try it*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"XINET SendMsg,net block len= %d,return %d,errinfo:%s",len,ret,XOS_StrError(INET_ERR_CODE));
            return eErrorNetBlock;
        }        
        else if(INET_ERR_CODE == ERR_EINTR)  /*系统调用被中断*/
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"XINET SendMsg,net Interrupted len= %d, return %d,errinfo:%s",len,ret,XOS_StrError(INET_ERR_CODE));
            return eErrorNetBlock;
        }
        else if ((INET_ERR_CODE == ERR_PIPE) ||
                 (INET_ERR_CODE == ERR_CONNABORTED) ||
                 (INET_ERR_CODE == ERR_CONNRESET))/*Check if connection was closed*/
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"XINET SendMsg, peer closed len= %d,return %d,errinfo:%s",len,ret,XOS_StrError(INET_ERR_CODE));
            return eErrorLinkClosed;
        }
        return XERROR;
    }
    return(XSUCC);
}

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
XPUBLIC XS16 XINET_SctpRecvMsg(t_XINETFD *pSockFd, XCHAR** ppData, XS32* pLen, t_IPADDR* pFromAddr, XVOID* pSndRcvInfo)
{
    XCHAR           *pBuf = NULL;       /*分配消息的指针*/
    XS32 rcvLen = 0, flag = 0;
    XU32            remAddrLen;     /* length of remote address */
    t_INETSOCKADDR  remSockAddr;    /* to get packet's source IP address */
    struct sockaddr_in  *pRemAddr = NULL;  /* remote Internet address */
    XS32 leftSize = 0,pieceSize = 0;
    
    if (pSockFd == XNULLP || XOS_INET_INV_SOCK_FD(pSockFd))
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpRecvMsg, bad input param,socket null or invalid");
        return(XERROR);
    }
    if (ppData == XNULLP|| pLen == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"XINET_SctpRecvMsg, bad input param,ppData or pLen null");
        return(XERROR);
    }

    if(pFromAddr == NULL)
    {
        remAddrLen = 0;
    }
    else
    {
        remAddrLen = sizeof(remSockAddr);
    }
    pRemAddr = ( struct sockaddr_in  *)XNULLP;
    XOS_MemSet((XU8*)&remSockAddr, 0, sizeof(remSockAddr));

    *ppData = (XCHAR*)XNULLP;
    *pLen = 0;
    pBuf = (XCHAR *)XOS_MemMalloc(FID_NTL,XOS_INET_MAX_UDPRAW_MSGSIZE);
    if (pBuf == XNULLP)
    {
        XOS_Trace(MD(FID_NTL, PL_EXP),  "XINET_SctpRecvMsg()-> malloc memory failed !");
        return XERROR;
    }

    if(remAddrLen)
    {
        rcvLen = sctp_recvmsg(pSockFd->fd,pBuf,XOS_INET_MAX_UDPRAW_MSGSIZE,(t_INETSOCKADDR *)&remSockAddr,&remAddrLen,(struct sctp_sndrcvinfo*)pSndRcvInfo,&flag);
    }
    else
    {
        rcvLen = sctp_recvmsg(pSockFd->fd,pBuf,XOS_INET_MAX_UDPRAW_MSGSIZE,(t_INETSOCKADDR *)NULL,&remAddrLen,(struct sctp_sndrcvinfo*)pSndRcvInfo,&flag);
    }
    
    if( rcvLen == 0)
    {
        XOS_MemFree(FID_NTL,pBuf);
        return XINET_CLOSE;
    }
    else if(rcvLen == INET_ERR)
    {
        XOS_MemFree(FID_NTL, pBuf);

        /*  the sctp_recvmsg function fails
        *  with error code which maps to either ERR_EINTR. If
        *  this happens then sctp_recvmsg must return XERROR */        
        if (INET_ERR_CODE == ERR_CONNRESET)
        {
            return(XINET_CLOSE);
        }
        if (INET_ERR_CODE == ERR_TIMEDOUT)
        {
            XOS_Trace(MD(FID_NTL, PL_EXP),  "XINET_SctpRecvMsg()->communication disconnect[heartbeat timeout]!");
            return(XINET_TIMEOUT);
        }
        XOS_Trace(MD(FID_NTL, PL_WARN),  "XINET_SctpRecvMsg()->recv errinfo:%s!",XOS_StrError(INET_ERR_CODE));
        return(XERROR);
    }    
    
    if( rcvLen > 0 && (flag & MSG_EOR) == 0 )
    {
        if( rcvLen < XOS_INET_MAX_UDPRAW_MSGSIZE) /*兼容sctp分组报文接收*/
        {
            leftSize = XOS_INET_MAX_UDPRAW_MSGSIZE - rcvLen;
            while(leftSize > 0 && (flag & MSG_EOR) == 0)
            {
                flag = 0;
                pieceSize = sctp_recvmsg(pSockFd->fd,pBuf + rcvLen,leftSize,NULL,NULL,NULL,&flag);
                rcvLen += pieceSize;
                leftSize -= pieceSize;
            }
            if(flag & MSG_EOR)
            {
                goto recvSucc;
            }
            else
            {
                /*对端发送的报文超过设定的最大长度，丢弃*/
                XOS_Trace(MD(FID_NTL, PL_WARN),  "XINET_SctpRecvMsg()-> peer send a too large packet,discard it !");
                while(rcvLen > 0 && (flag & MSG_EOR) == 0)
                {
                    flag = 0;
                    rcvLen = sctp_recvmsg(pSockFd->fd,pBuf,XOS_INET_MAX_UDPRAW_MSGSIZE,NULL,NULL,NULL,&flag);
                }
                XOS_MemFree(FID_NTL, pBuf);
                return XINET_INVALID;
            }
        }
        else/*对端发送的报文超过设定的最大长度，丢弃*/
        {
            XOS_Trace(MD(FID_NTL, PL_WARN),  "XINET_SctpRecvMsg()-> peer send a too large packet,discard it !");
            while(rcvLen > 0 && (flag & MSG_EOR) == 0)
            {
                flag = 0;
                rcvLen = sctp_recvmsg(pSockFd->fd,pBuf,XOS_INET_MAX_UDPRAW_MSGSIZE,NULL,NULL,NULL,&flag);
            }
            XOS_MemFree(FID_NTL, pBuf);
            return XINET_INVALID;
        }
    }

recvSucc:
    *ppData = pBuf;
    *pLen = rcvLen;
    
    if (remAddrLen > 0)
    {
        pRemAddr = (struct sockaddr_in *)&remSockAddr;
        pFromAddr->port    = XOS_INET_NTOH_U16(pRemAddr->sin_port);
        pFromAddr->ip  = XOS_INET_NTOH_U32(pRemAddr->sin_addr.s_addr);
    }

    return(XSUCC);
}
/*************************************************************
函数名称    : XINET_SctpConnectCheck
作者        : liukai
设计日期    : 2013年9月22日
功能描述    : 检测sctp连接是否正常
输入参数    : t_XINETFD *sockFd      - 连接的socket句柄
返回值      : XPUBLIC XS32 获取偶联对端端点的ip数
>0 连接正常
<=0 连接断开
**************************************************************/
XPUBLIC XS32 XINET_SctpConnectCheck(t_XINETFD *sockFd)
{
    struct sockaddr *s = NULL;
    XS32 get_rt=0;

    if ((sockFd == XNULLP) || XOS_INET_INV_SOCK_FD(sockFd))
    {
        return (XERROR);
    }

    get_rt = sctp_getpaddrs(sockFd->fd, 0, &s);
    if(get_rt > 0)
    {
        sctp_freepaddrs(s);
    }
    return get_rt;
}
#endif
#ifdef __cplusplus
}
#endif /* __cplusplus */
