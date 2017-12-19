/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: clitelnetd.c
**
**  description:  telnet server
**
**  author: zhanglei
**
**  date:   2006.9.4
**
***************************************************************
**                          history
**
***************************************************************
**   author              date              modification
**   zengguanqun         2008/01/29        add ha switch processing
     1.clear some unused trace,add the telnet close trace or send error trace
     2.adjust some local function more readable for its function implementation
**   zengguanqun         2008/06/29
     1.add comment and code review for whole telnet service
**************************************************************/
#ifdef  __cplusplus
extern   "C"  {
#endif  /*  __cplusplus */

#include "xosxml.h"
#include "xostl.h"
#include "xoshash.h"
#include "clitelnetd.h"
#include "xosencap.h"
#include "xosmem.h"
#include "xosos.h"
#ifdef CLI_LOG
#include "clishell.h"
#endif

#include "xosha.h"

extern XS8* NTL_getErrorTypeName(XS32 reason_code, XCHAR *pneterror_name, int nLen);
extern XS8 NTL_TelnetCliCloseSock(t_IPADDR ipAddr);
/*-------------------------------------------------------------------------
                                模块内部宏定义
-------------------------------------------------------------------------*/
#define TELNET_HANDLE            0x11000007
#define IP_BUFF_SIZE            64

/*-------------------------------------------------------------------------
                               模块内部数据结构
-------------------------------------------------------------------------*/

/*Telnet D 数据结构*/
typedef struct
{
    t_TelnetDCfg  telCfg;
    HLINKHANDLE   telLink;
    t_IPADDR      clientIP[kCLI_MAX_CLI_TASK+1];
    XBOOL         tcpIsReady;
} t_TelnetdCB;

/*-------------------------------------------------------------------------
                              模块内部全局变量
-------------------------------------------------------------------------*/

XSTATIC t_TelnetdCB g_telnetdCb;

/*-------------------------------------------------------------------------
                             模块内部函数
-------------------------------------------------------------------------*/
XSTATIC XS32  tlenetd_CloseLink(XVOID);
/*XSTATIC XS32  tlenetd_ReleaseLink(XVOID);*/
XSTATIC XS32  telnetd_SendMsg2Clinet(t_XOSCOMMHEAD *pMsg );
XSTATIC XS32  telnetD_msgToNtl(XVOID* pContent, XS32 s32Len,  e_TLMSG msgType);
XSTATIC XS32  telnetD_MsgeConnIndProc(t_XOSCOMMHEAD* pMsg );
XSTATIC XS32  telnetD_MsgeInitAckProc(t_XOSCOMMHEAD* pMsg );
XSTATIC XBOOL telnetd_GetSessionIDByIP(t_IPADDR clientIP, XS32* p32SID );
XSTATIC XBOOL telnetd_GetClientIPBySessionID(XS32 s32SessionID, t_IPADDR* pIPAdd );
XSTATIC XBOOL telnetd_GetnewSessionIDByClientIP(t_IPADDR clientIP, XS32* p32SID );
XSTATIC XBOOL telnetd_ReleaseSIDByClientIP(t_IPADDR clientIP, XS32 *pSID );
XSTATIC XVOID telnetd_showTelClient(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );
XSTATIC XVOID telnetd_ShutdownTelClient(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );
XSTATIC XS32  telnetD_SendMsg2CLI(XU16 msgID, XS32 s32SID );
XSTATIC XS32  telnetd_RestartLink(XVOID);
XSTATIC XS32  telnetD_dataIndProc(t_XOSCOMMHEAD* pMsg );
/*-------------------------------------------------------------------------
                                       模块接口函数
-------------------------------------------------------------------------*/

/********************************** 
函数名称    : telnet_service_ok
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
返回值      : XBOOL 
************************************/
XBOOL telnet_service_ok()
{
    XS32 sleep_max=0;
    for (sleep_max = 0; sleep_max < 10; sleep_max++)
    {
        XOS_Sleep(300);
        if (g_telnetdCb.tcpIsReady == XTRUE)
        {
            break;
        }
        else
        {
            continue;
        }
    }
    if (g_telnetdCb.tcpIsReady == XFALSE)
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR),"telnet service start failed,pls check xos.xml or check server port is used.");
#ifdef CLI_LOG
        cli_log_write(eCLIError, "telnet service start failed,pls check xos.xml or check server port is used!" );
#endif /* end  CLI_LOG  */
        
        return XFALSE;
    }
    return XTRUE;
}

/********************************** 
函数名称    : TelnetD_Init
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        :  XVOID *p1
参数        : XVOID *p2
返回值      : XS8 
************************************/
XS8 TelnetD_Init(XVOID *p1, XVOID *p2 )
{
    XS32 ret  = 0;
    p1 = p1;
    p2 = p2;
    
    XOS_MemSet(&g_telnetdCb, 0x00, sizeof(t_TelnetdCB));
    /*读配置文件*/
#ifndef XOS_EW_START
    if(!XML_GetTelDCfgFromXosXml(&(g_telnetdCb.telCfg), "xos.xml") )
#else
        if(!XML_GetTelDCfgFromXosXml(&(g_telnetdCb.telCfg), XOS_CliGetXmlName( )) )
#endif
        {
            XOS_Trace(MD(FID_TELNETD, PL_ERR),"read telnet server configuration from file xos.xml failed!");
#ifdef CLI_LOG
            cli_log_write(eCLIError, "read telnet server configure from xos.xml failed!" );
#endif /* end  CLI_LOG  */
            return XERROR;
        }

        if ( 1 > g_telnetdCb.telCfg.maxTelClients || kCLI_MAX_CLI_TASK < g_telnetdCb.telCfg.maxTelClients )
        {
            XOS_Trace(MD(FID_TELNETD, PL_ERR),"telnet cfg(connect number:%d) error!",g_telnetdCb.telCfg.maxTelClients);
#ifdef CLI_LOG
            cli_log_write(eCLIError,
                "telnet client connect number %d is so biger than XOS CLI's kCLI_MAX_CLI_TASK = %d !",
                g_telnetdCb.telCfg.maxTelClients,kCLI_MAX_CLI_TASK);
#endif /*endCLI_LOG  */
            return XERROR;
        }
        
        g_telnetdCb.tcpIsReady = XFALSE;
        ret = XOS_RegistCmdPrompt(SYSTEM_MODE, "plat", "plat", "no parameter" );
        if ( 0 > ret )
        {
            XOS_Trace(MD(FID_CLI, PL_ERR), "telnetd register command xos>plat failed!" );
        }
        if ( 0 > XOS_RegistCommand(ret,telnetd_showTelClient,
            "showtelc","display CLI telnet connection list.","no parameter"))
        {
            XOS_Trace(MD(FID_CLI, PL_ERR), "telnetd register command xos>plat>showtelc failed!" );
        }
        
        if ( 0 > XOS_RegistCommand(ret,telnetd_ShutdownTelClient,
            "shuttelc","close CLI Telnet client connection","usage: shuttelc <SID>\r\ne.g:\r\n shuttelc 1\r\n") )
        {
            XOS_Trace(MD(FID_CLI, PL_ERR), "telnetd register command xos>plat>shuttelc failed!" );
        }
        
#ifdef CLI_LOG
        cli_log_write(eCLINormal, "telnet d init successfully!" );
#endif /* end  CLI_LOG  */
        
        return XSUCC;
}

/********************************** 
函数名称    : TelnetD_notice
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        : XVOID* p1
参数        : XVOID* p2
返回值      : XS8 成功返回XSUCC , 失败返回XERROR
************************************/
XS8 TelnetD_notice(XVOID* p1, XVOID* p2)
{
    XS32 ret  = 0;
    t_LINKINIT linkInit;
    
    p1 = p1;
    p2 = p2;
    
    /*启动 TCP Server 链路*/
    linkInit.appHandle = (HAPPUSER)TELNET_HANDLE;
    linkInit.linkType  = eTCPServer;
    linkInit.ctrlFlag  = eCompatibleTcpeen;
    ret = telnetD_msgToNtl(&linkInit, sizeof(t_LINKINIT), eLinkInit );
    if( XSUCC != ret )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR ), "send msg to ntl error,start Telnet Server failed!");
#ifdef CLI_LOG
        cli_log_write(eCLIError, "telnetD_msgToNtl() send msg to ntl error,start Telnet Server failed!" );
#endif /* end  CLI_LOG  */
        return XERROR;
    }
    if(XFALSE == telnet_service_ok())
    {
        XOS_Trace(FILI, FID_COMM, PL_ERR, "telnet service start failed.");
        // return XERROR; //20111222 avoid here return err,will bring up the system init fail.
    }
    else
    {
#ifdef CLI_LOG
        cli_log_write(eCLINormal, "telnetd notice successfully!" );
#endif 
    }
#ifdef VXWORKS
    // 应SAG 要求，注释此行 2012-10-22
    //telnet_register2Ha();
#endif
    
/* end  CLI_LOG  */
    return XSUCC;
}

/********************************** 
函数名称    : TelnetD_msgProc
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 任务消息处理函数
参数        :  XVOID *pMsg1
参数        : XVOID *pMsg2
返回值      : XS8 成功返回XSUCC , 失败返回XERROR
************************************/
XS8 TelnetD_msgProc(XVOID *pMsg1, XVOID *pMsg2 )
{
    t_STARTACK     *pStartAck     = XNULL;
    t_SENDERROR    *pErrorSend    = XNULL;
    t_LINKCLOSEIND *pLinkCloseInd = XNULL;
    t_XOSCOMMHEAD  *pMsg          = XNULL;
    XS32           ret            = 0;
    XS32           s32SessionID   = 0;
    t_DATAIND      *pDataInd      = XNULL;
    XS8            s8Ret          = 0;
    XCHAR neterror_name[32] = {0};
    pMsg2 = pMsg2;
    
    /*参数入口检查*/
    if( pMsg1 == XNULLP )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"TelnetD_msgProc,input parameter is null!");
        return XERROR;
    }
    
    pMsg = (t_XOSCOMMHEAD*)pMsg1;
    if ( FID_CLI == pMsg->datasrc.FID )
    {
        /*发送数据  APP->NTL */
        ret = telnetd_SendMsg2Clinet( pMsg );
        if( ret != XSUCC )
        {
            XOS_Trace(MD(FID_TELNETD,PL_ERR),"telnetd_SendMsg2Clinet() send msg to client failed!");
            return XERROR;
        }
        return XSUCC;
    }
    
    if ( FID_NTL == pMsg->datasrc.FID )
    {
        switch (pMsg->msgID)
        {
        case eInitAck:
            {
                if ( XSUCC != telnetD_MsgeInitAckProc( pMsg ) )
                {
                    XOS_Trace(MD(FID_TELNETD,PL_ERR),"msg InitAck Proc failed!" );
                    return XERROR;
                }
            }
            break;
        case eStartAck:
            {
                pStartAck = (t_STARTACK*)(pMsg->message);
                if( pStartAck->linkStartResult == eSUCC )
                {
                    g_telnetdCb.tcpIsReady = XTRUE;
                }
                else
                {
                    XOS_Trace(MD(FID_TELNETD,PL_ERR),"TelnetD_msgProc() eStartAck msg result is wrong!");
                    return XERROR;
                }
            }
            break;
        case eErrorSend:
            {
                pErrorSend = (t_SENDERROR*)(pMsg->message);
#ifdef CLI_LOG
                cli_log_write( eCLIError, "data send(NTL ->APP) %s", NTL_getErrorTypeName((XS32)pErrorSend->errorReson, neterror_name, sizeof(neterror_name)-1) );
#endif /*CLI_LOG*/
				
                XOS_Trace(MD(FID_TELNETD, PL_INFO), "data send(NTL ->APP)%s", NTL_getErrorTypeName((XS32)pErrorSend->errorReson, neterror_name, sizeof(neterror_name)-1));
            }
            break;
            
            /*连接指示 NTL ->APP */
        case eConnInd:
            {
                if ( XSUCC != telnetD_MsgeConnIndProc( pMsg ) )
                {
                    XOS_Trace(MD(FID_TELNETD,PL_ERR),"Msg ConnInd Proc failed!" );
#ifdef CLI_LOG
                    cli_log_write(eCLIError, "Msg ConnInd Proc failed!" );
#endif /*CLI_LOG*/
                    return XERROR;
                }
            }
            break;
            
            /*收到数据 NTL ->APP*/
        case eDataInd:
            {
                pDataInd = (t_DATAIND*)(pMsg->message);
                if ( XNULL != pDataInd )
                {
#ifdef CLI_LOG
                    cli_log_write( eCLIDebug, "TELNETD received msg eDataInd!" );
#endif /*CLI_LOG*/
                    if ( XSUCC != telnetD_dataIndProc( pMsg ) )
                    {
                        XOS_Trace(MD(FID_TELNETD,PL_ERR),"eDataInd -> telnetD_dataIndProc error" );
#ifdef CLI_LOG
                        cli_log_write( eCLIError, "eDataInd -> telnetD_dataIndProc error" );
#endif /*CLI_LOG*/
                        s8Ret = XERROR;
                    }
                    s8Ret = XSUCC;
                    
                    if ( XNULL != pDataInd->pData )
                    {
                        XOS_MemFree(FID_TELNETD, pDataInd->pData );
                    }
                }
                return s8Ret;
            }
            break;
            
            /* 链路关闭指示 NTL ->APP*/
        case eStopInd:
            {
                
#ifdef CLI_LOG
                cli_log_write(eCLIDebug, " TELNETD received msg eStopInd!" );
#endif /*CLI_LOG*/
                pLinkCloseInd = (t_LINKCLOSEIND*)(pMsg->message);
                if ( XNULL == pLinkCloseInd )
                {
                    XOS_Trace(MD(FID_TELNETD,PL_ERR),"XNULL == pLinkCloseInd" );
                    return XERROR;
                }
                XOS_Trace(MD(FID_TELNETD, PL_INFO), "receive the eStopInd msg from ip[0x%x]\r\n",pLinkCloseInd->peerAddr.ip);
                s32SessionID = 0;
                if( telnetd_ReleaseSIDByClientIP( pLinkCloseInd->peerAddr, &s32SessionID )  )
                {
                    if ( XSUCC == telnetD_SendMsg2CLI( MSG_IP_CLOSEIND, s32SessionID ) )
                    {
#ifdef CLI_LOG
                        cli_log_write( eCLINormal,"telnet client closed,session id[%d]; Addr[0x%x:%d]",
                            s32SessionID,pLinkCloseInd->peerAddr.ip,pLinkCloseInd->peerAddr.port);
#endif
                        return XSUCC;
                    }
                    else
                    {
                        XOS_Trace(MD(FID_TELNETD, PL_ERR),
                            "send msg 2 CLI, try to close client(ip: %x, port: %d, session id: %d) failed!",
                            pLinkCloseInd->peerAddr.ip,pLinkCloseInd->peerAddr.port,s32SessionID);
#ifdef CLI_LOG
                        cli_log_write( eCLIError,
                            "send msg 2 CLI, try to close client(ip: %x, port: %d, session id: %d) failed!",
                            pLinkCloseInd->peerAddr.ip,pLinkCloseInd->peerAddr.port,s32SessionID );
#endif
                    }
                }
            }
            break;
        default:
            break;
        }
    }
    
    if((FID_CLI != pMsg->datasrc.FID) || (FID_NTL != pMsg->datasrc.FID))
    {
        switch (pMsg->msgID)
        {
        case eLinkStop:
            tlenetd_CloseLink();
            break;
            
        case eLinkStart:
            telnetd_RestartLink();
            break;
            
        default:
            break;
        }
    }
    return XSUCC;
}

/********************************** 
函数名称    : tlenetd_CloseLink
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 给予NTL发送关闭链路请求
参数        : XVOID
返回值      : XSTATIC XS32 成功返回XSUCC , 失败返回XERROR
************************************/
XSTATIC XS32 tlenetd_CloseLink(XVOID)
{
    t_LINKCLOSEREQ    telnetClose;
    XS32 s32Ret    = XERROR;
    
    /*关闭链路*/
    XOS_MemSet(&telnetClose, 0x00, sizeof(t_LINKCLOSEREQ) );
    
    telnetClose.linkHandle = g_telnetdCb.telLink;
    
    s32Ret = telnetD_msgToNtl( &telnetClose, sizeof(t_LINKCLOSEREQ), eLinkStop );
    if( s32Ret != XSUCC )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"tlenetd_CloseLink send eLinkStop msg  failed!");
        return XERROR;
    }
    
    return XSUCC;
    
}

/********************************** 
函数名称    : tlenetd_ReleaseLink
作者        : Jeff.Zeng
设计日期    : 2008年1月12日
功能描述    : 
参数        : XVOID
返回值      : XSTATIC XS32 
************************************/
#ifdef VXWORKS
XSTATIC XS32 tlenetd_ReleaseLink(XVOID)
{
    t_LINKRELEASE   telnetRelease;
    XS32 s32Ret    = XERROR;
    
    /*关闭链路*/
    XOS_MemSet(&telnetRelease, 0x00, sizeof(t_LINKRELEASE));
    
    telnetRelease.linkHandle = g_telnetdCb.telLink;
    
    s32Ret = telnetD_msgToNtl(&telnetRelease, sizeof(t_LINKRELEASE),eLinkRelease);
    if( s32Ret != XSUCC )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"tlenetd_ReleaseLink send eLinkRelease msg  failed!");
        return XERROR;
    }
    
    /*释放了所有客户端后,需清除本地资源,复位其IP地址域,其它保留为初始配置*/
    XOS_MemSet(g_telnetdCb.clientIP,0x0,sizeof(g_telnetdCb.clientIP));
    g_telnetdCb.tcpIsReady = XFALSE;
    return XSUCC;
    
}
#endif
/********************************** 
函数名称    : telnetd_RestartLink
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 给予NTL发送重启链路请求
参数        : XVOID
返回值      : XSTATIC XS32 
************************************/
XSTATIC XS32 telnetd_RestartLink(XVOID)
{
    t_LINKSTART    telnetStart;
    XS32 s32Ret    = XERROR;
    
    /*启动链路*/
    XOS_MemSet(&telnetStart, 0x00, sizeof(t_LINKSTART) );
    
    telnetStart.linkStart.tcpServerStart.allownClients = g_telnetdCb.telCfg.maxTelClients;
    telnetStart.linkStart.tcpServerStart.authenFunc    = XNULL;
    telnetStart.linkStart.tcpServerStart.myAddr.ip     = 0;
    telnetStart.linkStart.tcpServerStart.myAddr.port   = g_telnetdCb.telCfg.port;
    
    telnetStart.linkHandle = g_telnetdCb.telLink;
    
    s32Ret = telnetD_msgToNtl( &telnetStart, sizeof(t_LINKSTART), eLinkStart );
    if( s32Ret != XSUCC )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"telnetd_RestartLink send eLinkStart msg failed!");
        return XERROR;
    }
    
    return XSUCC;
    
}

/********************************** 
函数名称    : telnetd_SendMsg2Clinet
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 发送消息到 Telnet Client
参数        : t_XOSCOMMHEAD *pMsg -需要转发的消息
返回值      : XSTATIC XS32 成功返回XSUCC , 失败返回XERROR
************************************/
XSTATIC XS32 telnetd_SendMsg2Clinet(t_XOSCOMMHEAD *pMsg )
{
    t_IPADDR clientAddr;
    t_DATAREQ dataReq;
    XCHAR* pData = XNULL;
    
    if (pMsg == XNULLP)
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "telnetd_SendMsg2Clinet() input parameter is null" );
        return XERROR;
    }
    
    /*先检查 Telnet 的服务是不是成功*/
    if ( !g_telnetdCb.tcpIsReady )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "telnet server not ready,g_telnetdCb.tcpIsReady = XFALSE." );
        return XERROR;
    }
    
    /*查找对应的 telnet client 地址*/
    if ( !telnetd_GetClientIPBySessionID( pMsg->subID, &clientAddr ) )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "telnetd_GetClientIPBySessionID() failed!" );
        return XERROR;
    }
    
    pData = XOS_MemMalloc( FID_TELNETD, pMsg->length );
    if ( XNULL == pData )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "data malloc length %d failed!", pMsg->length );
        return XERROR;
    }
    XOS_MemSet( pData, 0x00, pMsg->length );
    XOS_MemCpy( pData, pMsg->message, pMsg->length );
    
#ifdef CLI_LOG
    cli_log_write( eCLIDebug, " rev from CLI msg send NTL: %s", (XCHAR*)pMsg->message );
#endif /*CLI_LOG*/
    
    /*构造数据请求消息*/
    XOS_MemSet( &dataReq, 0, sizeof(t_DATAREQ) );
    
    dataReq.dstAddr.ip   = clientAddr.ip;
    dataReq.dstAddr.port = clientAddr.port;
    
    dataReq.linkHandle = g_telnetdCb.telLink;
    dataReq.msgLenth   = pMsg->length;
    dataReq.pData      = (XCHAR*)pData;
    
#ifdef CLI_LOG
    cli_log_write( eCLIDebug, "msg send 2 NTL: %s", dataReq.pData );
#endif /*CLI_LOG*/
    
    /*发送消息到 ntl */
    if ( XSUCC != telnetD_msgToNtl( &dataReq, sizeof(t_DATAREQ), eSendData ) )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "telnetD_msgToNtl failed!" );
        return XERROR;
    }
    
    return XSUCC;
}

/********************************** 
函数名称    : telnetD_msgToNtl
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        : XVOID* pContent
参数        : XS32 s32Len
参数        : e_TLMSG msgType
返回值      : XSTATIC XS32 
************************************/
XSTATIC XS32 telnetD_msgToNtl(XVOID* pContent, XS32 s32Len,  e_TLMSG msgType)
{
    XS32 s32Ret         = 0;
    t_XOSCOMMHEAD *pMsg = XNULL;
    
    if( XNULLP       == pContent
        || 0         >= s32Len
        || eMinTlMsg >= msgType
        || eMaxTlMsg <= msgType )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "telnetD_msgToNtl() input parameter is null!" );
        return XERROR;
    }
    
    /*分配消息内存*/
    pMsg = (t_XOSCOMMHEAD*)XNULLP;
    pMsg = XOS_MsgMemMalloc( (XU32)FID_TELNETD, s32Len );
    if( pMsg == XNULLP )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "XOS_MsgMemMalloc failed!" );
        return XERROR;
    }
    
    /*填写消息数据*/
    pMsg->datasrc.PID  = XOS_GetLocalPID();
    pMsg->datasrc.FID  = (XU32)FID_TELNETD;
    
    pMsg->length       = s32Len;
    pMsg->msgID        = msgType;
    pMsg->prio         = eNormalMsgPrio;
    
    pMsg->datadest.FID = (XU32)FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    XOS_MemCpy( pMsg->message, pContent, s32Len );
    
    /*发送数据*/
    s32Ret = XOS_MsgSend( pMsg );
    if( XSUCC != s32Ret )
    {
        /*数据指示消息，应该先释放收到数据*/
        if(pMsg->msgID == eSendData)
        {
            XOS_MemFree(FID_TELNETD,((t_DATAREQ*)pContent)->pData);
        }
        
        /*其他消息类型应该将消息内存释放*/
        XOS_MsgMemFree( (XU32)FID_TELNETD, pMsg );
        
//        XOS_Trace(MD(FID_TELNETD, PL_ERR), "XOS_MsgSend() failed!\r\n" );
        return XERROR;
    }
    
    return XSUCC;
}

/********************************** 
函数名称    : telnetD_dataIndProc
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 处理从ntl收到的数据
参数        : t_XOSCOMMHEAD* pMsg－消息内容的首地址
返回值      : XSTATIC XS32 成功返回XSUCC , 失败返回XERROR
************************************/
XSTATIC XS32 telnetD_dataIndProc(t_XOSCOMMHEAD* pMsg )
{
    t_DATAIND     *pDataInd   = XNULL;
    XS32 s32SubID             = 0;
    t_XOSCOMMHEAD* pTelnetMsg = XNULL;
    
    if( XNULL == pMsg )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "telnetD_dataIndProc() input parameter is null!" );
        return XERROR;
    }
    pDataInd = (t_DATAIND*)(pMsg->message);
    if( XNULLP == pDataInd || XNULLP == pDataInd->pData )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "XNULLP == pDataInd or XNULLP == pDataInd->pData!" );
        return XERROR;
    }
    
    switch ( (XPOINT)pDataInd->appHandle )
    {
    case TELNET_HANDLE:
        {
            if ( !telnetd_GetSessionIDByIP( pDataInd->peerAddr, &s32SubID ) )
            {
                XOS_Trace(MD(FID_TELNETD, PL_ERR),
                    "unkonw client ip %x, port:%d",
                    pDataInd->peerAddr.ip,
                    pDataInd->peerAddr.port  );
#ifdef CLI_LOG
                cli_log_write( eCLIError, "unkonw client ip %x, port:%d",
                    pDataInd->peerAddr.ip,
                    pDataInd->peerAddr.port );
#endif /* end  CLI_LOG  */
                
                return XERROR;
            }
            
            if ( pDataInd->dataLenth < 1 )
            {
                XOS_Trace(MD(FID_TELNETD, PL_ERR), "pDataInd->dataLenth < 1" );
                return XERROR;
            }
            
            pTelnetMsg = XOS_MsgMemMalloc( FID_TELNETD, pDataInd->dataLenth );
            if ( XNULLP == pTelnetMsg )
            {
                XOS_Trace(MD(FID_TELNETD, PL_ERR), "XOS_MsgMemMalloc() failed!" );
                return XERROR;
            }
            
            pTelnetMsg->datasrc.FID    = FID_TELNETD;
            pTelnetMsg->datasrc.FsmId  = 0;
            pTelnetMsg->datasrc.PID    = XOS_GetLocalPID();
            
            pTelnetMsg->datadest.FID   = FID_CLI;
            pTelnetMsg->datadest.FsmId = 0;
            pTelnetMsg->datadest.PID   = XOS_GetLocalPID();
            pTelnetMsg->prio           = eAdnMsgPrio;
            pTelnetMsg->msgID          = MSG_IP_DATAIND;
            pTelnetMsg->subID          = s32SubID;
            
            XOS_MemCpy( pTelnetMsg->message, pDataInd->pData,  pDataInd->dataLenth );
            
#ifdef CLI_LOG
            cli_log_write( eCLIDebug, "msg send NTL 2 CLI: %s.", (XCHAR*)pDataInd->pData );
#endif /* end  CLI_LOG  */
            
            if ( XSUCC != XOS_MsgSend( pTelnetMsg ) )
            {
                /*出错处理*/
                XOS_MsgMemFree( FID_TELNETD, pTelnetMsg );
            }
            
            return XSUCC;
        }
        break;
        
    default:
        break;
    }
    
    return XERROR;
}

/********************************** 
函数名称    : telnetD_MsgeConnIndProc
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 处理从ntl收到的eConnInd消息
参数        : t_XOSCOMMHEAD* pMsg－消息内容的首地址
返回值      : XSTATIC XS32 成功返回XSUCC , 失败返回XERROR
************************************/
XSTATIC XS32 telnetD_MsgeConnIndProc(t_XOSCOMMHEAD* pMsg )
{
    t_DATAIND      *pDataInd    = XNULL;
    t_XOSCOMMHEAD  *pSen2CLIMsg = XNULL;
    XS32 s32SID = 0;
    
    if ( XNULL == pMsg )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "telnetD_MsgeConnIndProc() input parameter is null!" );
        return XERROR;
    }
    
    pDataInd = (t_DATAIND*)(pMsg->message);
    if( pDataInd == XNULL )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"pDataInd == XNULL!");
        return XERROR;
    }
    
    if ( TELNET_HANDLE != (XPOINT)pDataInd->appHandle )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"TELNET_HANDLE !=  pDataInd->appHandle.");
        return XERROR;
    }
    
    if ( !telnetd_GetnewSessionIDByClientIP( pDataInd->peerAddr, &s32SID ) )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR)," telnetd_GetnewSessionIDByClientIP failed.");
        return XERROR;
    }
    
#ifdef CLI_LOG
    cli_log_write(  eCLINormal,
        "new telnet client connection, Session ID: %d,IP:%x, Port:%d.",
        s32SID, pDataInd->peerAddr.ip,pDataInd->peerAddr.port );
#endif /* end  CLI_LOG  */
    
    pSen2CLIMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc( (XU32)FID_TELNETD, 1 );
    
    pSen2CLIMsg->datasrc.FID    = FID_TELNETD;
    pSen2CLIMsg->datasrc.FsmId  = 0;
    pSen2CLIMsg->datasrc.PID    = XOS_GetLocalPID();
    
    pSen2CLIMsg->datadest.FID   = FID_CLI;
    pSen2CLIMsg->datadest.FsmId = 0;
    pSen2CLIMsg->datadest.PID   = XOS_GetLocalPID();
    pSen2CLIMsg->prio           = eAdnMsgPrio;
    pSen2CLIMsg->msgID          = MSG_IP_CONNECTIND;
    pSen2CLIMsg->subID          = s32SID;
    
    XOS_MemCpy( pSen2CLIMsg->message, "", pSen2CLIMsg->length );
    
    if ( XSUCC != XOS_MsgSend( pSen2CLIMsg ) )
    {
        /*出错处理*/
        XOS_MsgMemFree( FID_TELNETD, pSen2CLIMsg );
        
        return XERROR;
    }
    
    return XSUCC;
}

/********************************** 
函数名称    : telnetD_SendMsg2CLI
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 发送消息到CLI
参数        :  XU16 msgID
参数        : XS32 s32SID
返回值      : XS32 
************************************/
XS32 telnetD_SendMsg2CLI( XU16 msgID, XS32 s32SID )
{
    t_XOSCOMMHEAD  *pSen2CLIMsg = XNULL;
    
    pSen2CLIMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc( (XU32)FID_TELNETD, 1 );
    
    pSen2CLIMsg->datasrc.FID    = FID_TELNETD;
    pSen2CLIMsg->datasrc.FsmId  = 0;
    pSen2CLIMsg->datasrc.PID    = XOS_GetLocalPID();
    
    pSen2CLIMsg->datadest.FID   = FID_CLI;
    pSen2CLIMsg->datadest.FsmId = 0;
    pSen2CLIMsg->datadest.PID   = XOS_GetLocalPID();
    pSen2CLIMsg->prio           = eAdnMsgPrio;
    pSen2CLIMsg->msgID          = msgID;
    pSen2CLIMsg->subID          = (XU16)s32SID;
    
    if ( msgID != MSG_IP_CLOSEIND )
    {
        XOS_MemCpy( pSen2CLIMsg->message, "", 1 );
    }
    
    if ( XSUCC != XOS_MsgSend( pSen2CLIMsg ) )
    {
        /*出错处理*/
        XOS_MsgMemFree( FID_TELNETD, pSen2CLIMsg );
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "XOS_MsgSend() failed!" );
        return XERROR;
    }
    
    return XSUCC;
}

/********************************** 
函数名称    : telnetD_MsgeInitAckProc
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 处理从ntl收到的 eInitAck 消息
参数        : t_XOSCOMMHEAD* pMsg－消息内容的首地址
返回值      : XSTATIC XS32 成功返回XSUCC , 失败返回XERROR
************************************/
XSTATIC XS32 telnetD_MsgeInitAckProc(t_XOSCOMMHEAD* pMsg )
{
    t_LINKINITACK  *pLinkAck = XNULL;
    t_LINKSTART    telnetStart;
    XS32 s32Ret    = -1;
    
    if ( XNULL == pMsg )
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR), "telnetD_MsgeInitAckProc() input parameter is null!" );
        return XERROR;
    }
    
    pLinkAck = (t_LINKINITACK*)(pMsg->message);
    if( pLinkAck == XNULL )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"pLinkAck == XNULL!");
        return XERROR;
    }
    
    /*检查返回的结果*/
    if( pLinkAck->lnitAckResult != eSUCC )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"lnitAckResult != eSUCC!");
        return XERROR;
    }
    
    if ( TELNET_HANDLE != (XPOINT)pLinkAck->appHandle )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"appHandle != TELNET_HANDLE!");
        return XERROR;
    }
    
    /* 记录linkHandle */
    g_telnetdCb.telLink = pLinkAck->linkHandle;
    
    /*启动链路*/
    XOS_MemSet( &telnetStart, 0x00, sizeof(t_LINKSTART) );
    
    telnetStart.linkStart.tcpServerStart.allownClients = g_telnetdCb.telCfg.maxTelClients;
    telnetStart.linkStart.tcpServerStart.authenFunc    = XNULL;
    telnetStart.linkStart.tcpServerStart.myAddr.ip     = 0;
    telnetStart.linkStart.tcpServerStart.myAddr.port   = g_telnetdCb.telCfg.port;
    
    telnetStart.linkHandle = pLinkAck->linkHandle;
    
    s32Ret = telnetD_msgToNtl( &telnetStart, sizeof(t_LINKSTART), eLinkStart );
    if( s32Ret != XSUCC )
    {
        XOS_Trace(MD(FID_TELNETD,PL_ERR),"telnetD_msgToNtl send eLinkStart msg  failed!");
        return XERROR;
    }
    
    return XSUCC;
}

/********************************** 
函数名称    : telnetd_GetSessionIDByIP
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        :  t_IPADDR clientIP
参数        : XS32* p32SID
返回值      : XSTATIC XBOOL 
************************************/
XSTATIC XBOOL telnetd_GetSessionIDByIP( t_IPADDR clientIP, XS32* p32SID )
{
    XS32 i = 0;
    
    if ( XNULL == p32SID )
    {
        return XFALSE;
    }
    
    for( i = 1; i <= kCLI_MAX_CLI_TASK; i++ )
    {
        if (( g_telnetdCb.clientIP[i].ip   == clientIP.ip )
            && ( g_telnetdCb.clientIP[i].port == clientIP.port ) )
        {
            *p32SID = i;
            return XTRUE;
        }
    }
    
    return XFALSE;
}

/********************************** 
函数名称    :  telnetd_GetClientIPBySessionID
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        :  XS32 s32SessionID
参数        : t_IPADDR* pIPAdd
返回值      : XSTATIC XBOOL 
************************************/
XSTATIC XBOOL  telnetd_GetClientIPBySessionID( XS32 s32SessionID, t_IPADDR* pIPAdd )
{
    if ( XNULL == pIPAdd  || 1 > s32SessionID || kCLI_MAX_CLI_TASK < s32SessionID )
    {
        return XFALSE;
    }
    XOS_MemSet( pIPAdd, 0x00, sizeof( t_IPADDR ) );
    
    if (  0 < s32SessionID || kCLI_MAX_CLI_TASK >= s32SessionID )
    {
        pIPAdd->ip   = g_telnetdCb.clientIP[s32SessionID].ip;
        pIPAdd->port = g_telnetdCb.clientIP[s32SessionID].port;
        
        return XTRUE;
    }
    
    return XFALSE;
}

/********************************** 
函数名称    : telnetd_ReleaseSIDByClientIP
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        :  t_IPADDR clientIP
参数        : XS32 *pSID
返回值      : XBOOL 
************************************/
XBOOL telnetd_ReleaseSIDByClientIP( t_IPADDR clientIP, XS32 *pSID )
{
    XS32 s32SessionID = -1;
    
    if ( XNULL == pSID )
    {
        return XTRUE;
    }
    
    if ( telnetd_GetSessionIDByIP( clientIP, &s32SessionID ) )
    {
        NTL_TelnetCliCloseSock(clientIP);
        g_telnetdCb.clientIP[s32SessionID].port = 0;
        g_telnetdCb.clientIP[s32SessionID].ip   = 0;
        
        *pSID = s32SessionID;
        
        return XTRUE;
    }
    
    return XFALSE;
}

/********************************** 
函数名称    : telnetd_ReleaseSID
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        : XS32 s32SID
返回值      : XBOOL 
************************************/
XBOOL telnetd_ReleaseSID( XS32 s32SID )
{
    if (  1 > s32SID
        || s32SID > g_telnetdCb.telCfg.maxTelClients
        || s32SID > kCLI_MAX_CLI_TASK )
    {
        return XFALSE;
    }
    
    g_telnetdCb.clientIP[s32SID].port = 0;
    g_telnetdCb.clientIP[s32SID].ip   = 0;
    
    return XTRUE;
    
}

/********************************** 
函数名称    : telnetd_GetnewSessionIDByClientIP
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        :  t_IPADDR clientIP
参数        : XS32* p32SID
返回值      : XSTATIC XBOOL 
************************************/
XSTATIC XBOOL telnetd_GetnewSessionIDByClientIP( t_IPADDR clientIP, XS32* p32SID )
{
    XS32  s32SID = 1;
    
    if ( XNULL == p32SID )
    {
        return XFALSE;
    }
    
    while (  0      != g_telnetdCb.clientIP[ s32SID ].ip
        && 0      != g_telnetdCb.clientIP[ s32SID ].port
        && s32SID <= g_telnetdCb.telCfg.maxTelClients
        && s32SID <= kCLI_MAX_CLI_TASK )
    {
        s32SID++;
    }
    
    if ( kCLI_MAX_CLI_TASK < s32SID )
    {
        return XFALSE;
    }
    
    /*添加*/
    g_telnetdCb.clientIP[ s32SID ].ip   = clientIP.ip;
    g_telnetdCb.clientIP[ s32SID ].port = clientIP.port;
    
    *p32SID = s32SID;
    
    return XTRUE;
}

/********************************** 
函数名称    : telnetd_ShutdownTelClient
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        :  CLI_ENV* pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID 
************************************/
XVOID telnetd_ShutdownTelClient( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv )
{
    XS32  s32Value            = 0;
    XCHAR buff[IP_BUFF_SIZE] = {0};
    XU32 client_ip;
    XU16 client_port;
    XOS_CliExtPrintf(pCliEnv,"\r\n" );
    
    if ( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv,"usage: shuttelc <SID>\r\ne.g:\r\n shuttelc 1\r\n");
        return;
    }
    
    XOS_StrToNum( ppArgv[ siArgc - 1 ], (XU32 *)&s32Value );
    if ( 1 > s32Value || kCLI_MAX_CLI_TASK < s32Value )
    {
        XOS_CliExtPrintf(pCliEnv,"SID: %d error\r\n", s32Value );
        return ;
    }
    if ( 0 == g_telnetdCb.clientIP[s32Value].ip
        && 0 == g_telnetdCb.clientIP[s32Value].port )
    {
        XOS_CliExtPrintf(pCliEnv,"SID: %d error\r\n", s32Value );
        return;
    }
    client_ip=g_telnetdCb.clientIP[s32Value].ip;
    client_port=g_telnetdCb.clientIP[s32Value].port;
    if ( XSUCC == telnetD_SendMsg2CLI( MSG_IP_CLOSEIND, s32Value ) )
    {
        if ( telnetd_ReleaseSIDByClientIP( g_telnetdCb.clientIP[s32Value], &s32Value ) )
        {
            if ( XSUCC != XOS_IpNtoStr( client_ip, buff, IP_BUFF_SIZE ) )
            {
                return ;
            }
            XOS_CliExtPrintf(pCliEnv,
                "close telnet client(Session ID: %d,IP Adress: %s port: %d).\r\n",
                s32Value, buff,client_port);
#ifdef CLI_LOG
            cli_log_write(  eCLINormal,
                "closed telnet client(Session ID: %d,IP Adress: %s port: %d).\r\n",
                s32Value,buff,client_port);
            
#endif /*CLI_LOG*/
        }
        return ;
    }
    
    return ;
}

/********************************** 
函数名称    : telnetd_showTelClient
作者        : zhanglei
设计日期    : 2008年6月30日
功能描述    : 
参数        :  CLI_ENV* pCliEnv
参数        : XS32 siArgc
参数        : XCHAR **ppArgv
返回值      : XVOID 
************************************/
XVOID telnetd_showTelClient( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv )
{
    XS32 i                    = 0;
    XCHAR buff[IP_BUFF_SIZE] = {0};
    XBOOL bFound              = XFALSE;
    
    XOS_CliExtPrintf(pCliEnv,"\r\n");
    XOS_CliExtPrintf(pCliEnv,"max support clients %d\r\n",g_telnetdCb.telCfg.maxTelClients);
    for ( i = 1; i <= kCLI_MAX_CLI_TASK; i++ )
    {
        if (   0 == g_telnetdCb.clientIP[i].ip
            && 0 == g_telnetdCb.clientIP[i].port )
        {
            continue;
        }
        if ( XSUCC == XOS_IpNtoStr( g_telnetdCb.clientIP[i].ip, buff, IP_BUFF_SIZE ) )
        {
            XOS_CliExtPrintf(pCliEnv,
                "telnet client(Session ID: %d,IP Adress: %s port: %d) online.\r\n",
                i,buff,g_telnetdCb.clientIP[i].port );
            
            bFound = XTRUE;
        }
    }
    
    if ( !bFound )
    {
        XOS_CliExtPrintf(pCliEnv,"can't find telnet client connetion.\r\n");
    }
    
    return ;
}

//2008/01/02 add Telnet HA matainance below
#ifdef VXWORKS
/********************************** 
函数名称    : telnetshow
作者        : Jeff.Zeng
设计日期    : 2008年1月12日
功能描述    : 
返回值      : XPUBLIC XS32 
************************************/
XPUBLIC XS32 telnetshow()
{
    XS32 i                    = 0;
    XCHAR tmpbuff[64] = {0};
    for ( i = 1; i <= kCLI_MAX_CLI_TASK; i++ )
    {
        if (0 == g_telnetdCb.clientIP[i].ip
            && 0 == g_telnetdCb.clientIP[i].port )
        {
            continue;
        }
        if ( XSUCC == XOS_IpNtoStr( g_telnetdCb.clientIP[i].ip, tmpbuff, 64 ) )
        {
            printf("telnet client(Session ID: %d,IP Adress: %s port: %d) online.\r\n",
                i,tmpbuff,g_telnetdCb.clientIP[i].port );
            
        }
    }
    return XSUCC;
}

/********************************** 
函数名称    : telnet_ha_callback
作者        : Jeff.Zeng
设计日期    : 2008年1月12日
功能描述    : 
参数        : int event
参数        : void * para1
参数        : void * para2
返回值      : int 
************************************/
int telnet_ha_callback(int event, void * para1, void * para2)
{
    t_xosha_info * req=NULL;
    XS32 ret  = 0;
    t_LINKINIT linkInit;
    /*启动 TCP Server 链路*/
    linkInit.appHandle = (HAPPUSER)TELNET_HANDLE;
    linkInit.linkType  = eTCPServer;
    linkInit.ctrlFlag  = eCompatibleTcpeen;
    
    req = (t_xosha_info *)para1;
    if (!req)
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR ),"hello,telnet_ha_callback,input para is null.");
        return XERROR;
    }
    
    switch(event)
    {
    case HA_NOTIFY_EVENT_START_SERVICE:
        XOS_Trace(MD(FID_TELNETD, PL_INFO),"telnet_ha_callback,start service event coming.");
        /*关闭Telnet Tcp Server服务*/
        tlenetd_ReleaseLink();
        /*启动 TCP Server 链路*/
        ret = telnetD_msgToNtl( &linkInit, sizeof(t_LINKINIT), eLinkInit );
        if( XSUCC != ret )
        {
            XOS_Trace(MD(FID_TELNETD, PL_ERR ), "send msg to ntl error,start Telnet Server failed!");
            return XERROR;
        }
        break;
    case HA_NOTIFY_EVENT_STOP_SERVICE:
        XOS_Trace(MD(FID_TELNETD, PL_INFO),"telnet_ha_callback,stop service event coming.");
        /*关闭Telnet Tcp Server服务*/
        tlenetd_ReleaseLink();
        /*启动 TCP Server 链路*/
        ret = telnetD_msgToNtl( &linkInit, sizeof(t_LINKINIT), eLinkInit );
        if( XSUCC != ret )
        {
            XOS_Trace(MD(FID_TELNETD,PL_ERR), "send msg to ntl error,start Telnet Server failed!");
            return XERROR;
        }
        break;
    default:
        break;
    }
    
    return XSUCC;
}

/********************************** 
函数名称    : telnet_register2Ha
作者        : Jeff.Zeng
设计日期    : 2008年1月12日
功能描述    : 
返回值      : int 
************************************/
int telnet_register2Ha()
{
    t_ha_register_para ha_para;
    ha_para.module_id = FID_TELNETD;
    ha_para.p_notify_cbf = telnet_ha_callback;
    ha_para.app_arg = XNULL;
    if( XSUCC != ha_app_register(&ha_para))
    {
        XOS_Trace(MD(FID_TELNETD, PL_ERR ),"Failed to register telnet module to the HA manager!");
        return ERROR;
    }
    return XSUCC;
}

#endif
//2008/01/02 add Telnet HA matainance above

#ifdef  __cplusplus
}
#endif  /*  __cplusplus */


