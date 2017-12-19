
/***************************************************************
**
**  Xinwei Telecom Technology co.,ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosntl.c
**
**  description:  net transportation layer  implement
**
**  author: wangzongyou
**
**  data:   2006.3.10
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   wzy          2006.3.10            create
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "xosencap.h"
#include "xosntl.h"
#include "xosarray.h"
#include "xosmem.h"
#include "xosha.h"
#if defined(XOS_SCTP) && defined(XOS_LINUX)
#include "xossctp.h"
#endif
#if defined(XOS_SCTP) && defined(XOS_WIN32)
#include "xossctp_win.h"
#endif

/*-------------------------------------------------------------------------
                      模块内部宏定义
-------------------------------------------------------------------------*/
/* 验证码*/
#define  LINK_CHECK_NUM    0xaaa
#define INIT_TIMER_NUM        3        /*tcp + sctp server数据重发 + sctp client数据重发*/

/*-------------------------------------------------------------------------
                     模块内部结构和枚举定义
-------------------------------------------------------------------------*/

#ifdef VXWORKS
  int ntl_test_closetcpsession(int con_port);
#endif

/*20080710 add below*/
PTIMER ntl_timer= XNULL;
XBOOL g_ntl_timer=XTRUE;
/*20080710 add above*/

#ifdef XOS_LINUX
PTIMER sctp_cli_timer= XNULL;    /*sctp客户端数据重发定时器*/
PTIMER sctp_ser_timer= XNULL;    /*sctp服务端数据重发定时器*/
XBOOL g_sctp_timer=XTRUE;
#endif
/*20131022 add above*/

XEXTERN XBOOL g_ntltraceswitch;

/*以下三个统计值用于NTL异常情况分析使用*/
XU32 g_NtlTest1 = 0;
XU32 g_NtlTest4 = 0;
XU32 g_NtlTest6 = 0;

XS32 NTL_regCli(int cmdMode);
/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/
XSTATIC  t_NTLGLOAB g_ntlCb;
XU32 g_ntlpoolsize=100;
XU32 g_tcpSerCloseFail=0;
XU32 g_tcpSerReleaseFail=0;
XS32 NTL_closeTsCli(t_TSCLI* pTcpServCli);


/*-------------------------------------------------------------------------
                   模块内部函数
-------------------------------------------------------------------------*/
XS8* NTL_getLinkStateName(XS32 link_state, XCHAR *pLinkstate_name, int nLen)
{
    if(!pLinkstate_name)
    {
        return pLinkstate_name;
    }
    
    switch(link_state)
    {
    case eNullLinkState:
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "%s", "eNullLinkState");
        break;
    case eStateInited:
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "%s", "eStateInited");
        break;
    case eStateStarted:
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "%s", "eStateStarted");
        break;
    case eStateConnecting:
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "%s", "eStateConnecting");
        break;
    case eStateConnected:
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "%s", "eStateConnected");
        break;
    case eStateListening:
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "%s", "eStateListening");
        break;
    case eStateWaitClose:
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "%s", "eStateWaitClose");
        break;
    case eMaxLinkState:
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "%s", "eMaxLinkState");
        break;
    default :
        XOS_Sprintf(pLinkstate_name, (XU32)nLen, "unknown state %d", link_state);
    }
    return pLinkstate_name;
}


/*2007/12/05增加通讯模块发送数据报错误原因打印*/
XS8* NTL_getErrorTypeName(XS32 reason_code, XCHAR *pneterror_name, int nLen)
{
    if(!pneterror_name)
    {
        return pneterror_name;
    }
    
    switch(reason_code)
    {
    case eErrorNetInterrupt:
        XOS_Sprintf(pneterror_name, nLen, "%s", "Error:Net Interrupt");
        break;
    case eErrorNetBlock:
        XOS_Sprintf(pneterror_name, nLen, "%s", "Error:Net Block");
        break;
    case eErrorDstAddr:
        XOS_Sprintf(pneterror_name, nLen, "%s", "Error:Dst Addr");
        break;
    case eErrorOverflow:
        XOS_Sprintf(pneterror_name, nLen, "%s", "Error:Overflow");
        break;
    case eErrorLinkClosed:
        XOS_Sprintf(pneterror_name, nLen, "%s", "Error:LinkClosed");
        break;
    case eErrorLinkState:
        XOS_Sprintf(pneterror_name, nLen, "%s", "Error:LinkState illegal");
        break;
    case eMaxLinkState:
        XOS_Sprintf(pneterror_name, nLen, "%s", "Error:unknown reason");
        break;
    default :
        XOS_Sprintf(pneterror_name,nLen, "Error:unknown reason code %d", reason_code);
    }
    return pneterror_name;
}


/*2007/12/05增加通讯模块发送数据报错误原因打印*/
XS32 NTL_RestartLink(t_TCCB* tcpCliCb)
{
    t_LINKSTART startLnk;
    XS32 ret = 0;
    t_XOSCOMMHEAD *pMsg = XNULL;
    if(XNULL == tcpCliCb)
    {
        return XERROR;
    }
    
    XOS_MemSet((char*)&startLnk, 0x0, sizeof(t_LINKSTART));
    startLnk.linkHandle = tcpCliCb->linkHandle;
    startLnk.linkStart.tcpClientStart.myAddr.ip = tcpCliCb->myAddr.ip;
    startLnk.linkStart.tcpClientStart.myAddr.port = tcpCliCb->myAddr.port;
    startLnk.linkStart.tcpClientStart.peerAddr.ip = tcpCliCb->peerAddr.ip;
    startLnk.linkStart.tcpClientStart.peerAddr.port = tcpCliCb->peerAddr.port;
    /*分配消息内存*/
    pMsg = (t_XOSCOMMHEAD*)XNULL;
    pMsg = XOS_MsgMemMalloc(FID_NTL,sizeof(t_LINKSTART));
    if(pMsg == XNULL)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"ERROR: link msg to ntl: can not malloc memory.");
        return XERROR;
    }

    /*填写消息数据*/
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = FID_NTL;
    pMsg->prio = eFastMsgPrio;
    pMsg->msgID = eLinkStart;
    XOS_MemCpy(pMsg->message, &startLnk, sizeof(t_LINKSTART));

    /*发送数据*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        /*数据指示消息，应该先释放收到数据*/
        XOS_MsgMemFree(FID_NTL, pMsg);
        XOS_Trace(FILI, FID_NTL, PL_ERR, "ERROR: NTL_RestartLink send msg failed.");
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
函数名:NTL_buildLinkH
功能: 构造链路句柄
输入:
linkType － 链路类型(高四位)
linkIndex －链路索引
输出:
返回:  返回链路句柄
说明:  链路句柄组织如图，

            linkType  checkcoder          linkIndex
           31      27                   15                            0 (bit)
           +----+------------+----------------+
           |        |                      |                            |
           +----+------------+----------------+

************************************************************************/
HLINKHANDLE  NTL_buildLinkH(  e_LINKTYPE linkType,XU16 linkIndex)
{
    /*LINK_CHECK_NUM,为了躲开vc中不初始化的局部变量总为0xcccc 的情况*/
    XU32 checkNum = LINK_CHECK_NUM;
    HLINKHANDLE  linkhandle;
    linkhandle = (HLINKHANDLE)(XPOINT)(((linkType&0xf)<<28) |(checkNum<<16) | ((linkIndex&0xffff)));
    return linkhandle;
}


/************************************************************************
函数名:isValidLinkH
功能: 验证链路句柄的有效性
输入: 链路句柄
输出:
返回: 有效返回XTURE,否则返回XFALSE
说明:
************************************************************************/
XBOOL NTL_isValidLinkH( HLINKHANDLE linkHandle)
{
    XU32 checkNum = LINK_CHECK_NUM; /*0xaaa*/
    return (LINK_CHECK_NUM == ((XPOINT)linkHandle&(checkNum<<16))>>16)? XTRUE:XFALSE;
}


/************************************************************************
函数名:NTL_getLinkType
功能: 通过链路句柄获取链路类型
输入: 链路句柄
输出:
返回: 链路类型
说明:
************************************************************************/
e_LINKTYPE NTL_getLinkType( HLINKHANDLE linkHandle)
{
    return (e_LINKTYPE)(((XPOINT)linkHandle)>>28);
}


/************************************************************************
函数名:NTL_getLinkIndex
功能: 通过链路句柄获取链路Index
输入: 链路句柄
输出:
返回: 链路索引
说明:
************************************************************************/
XS32 NTL_getLinkIndex( HLINKHANDLE linkHandle)
{
    return (XS32)(((XPOINT)linkHandle)&0xffff);
}


/************************************************************************
函数名:NTL_findTclient
功能:  查找tcp 连接的客户
输入:
pTserverCb －server cb的指针
pClientAddr  － 接进来客户的地址指针
输出:
返回: 成功返回控制块指针,否则返回xnullp
说明:
************************************************************************/
XSTATIC t_TSCLI * NTL_findTclient(t_TSCB *pTserverCb,t_IPADDR *pClientAddr)
{
    t_TSCLI *pTservCliCb = NULL;

#ifdef INPUT_PAR_CHECK
    if(pTserverCb == XNULLP || pClientAddr == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_findTclient()->input param error!");
        return (t_TSCLI*)XNULLP;
    }

#endif
    pTservCliCb = (t_TSCLI*)XOS_HashElemFind(g_ntlCb.tSerCliH,(XVOID*)pClientAddr);
    return pTservCliCb;
}


/************************************************************************
函数名:NTL_tcliHashFunc
功能:  查找tcpserver 接入客户的hash 函数
输入:
param －hash key 的指针
paramSize  －key的大小
hashSize －hash 桶的大小
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XU32  NTL_tcliHashFunc( XVOID *param, XS32 paramSize, XS32 hashSize)
{
    XU32 hashKey = 0;
    t_IPADDR *pAddr = NULL;

    if(param == XNULLP || (XU32)paramSize != sizeof(t_IPADDR))
    {
        /*参数错误的情况,都挂在第一个位置上*/
        return 0;
    }

    pAddr = (t_IPADDR*)param;

    /*Hash by IP and PORT fields */
    hashKey = hashKey << 1 ^ pAddr->ip;
    hashKey = hashKey << 1 ^ pAddr->port;

    return hashKey%((XU32)hashSize);
}

/************************************************************************
* key 比较函数的定义
* 输入  :
* key1,key2    - 用于比较的两个Key值
* keySize       - key值的大小
************************************************************************/
XBOOL NTL_cmpIpAddr (XVOID* key1,XVOID* key2,XU32 keySize)
{
    XOS_UNUSED(keySize);

    if(key1 == XNULLP || key2 == XNULLP)
    {
        return XFALSE;
    }
    if( ((t_IPADDR*)key1)->ip == ((t_IPADDR*)key2)->ip
        && ((t_IPADDR*)key1)->port == ((t_IPADDR*)key2)->port)
    {
        return XTRUE;
    }
    else
    {
        return XFALSE;
    }
}


/************************************************************************
函数名:NTL_msgTo User
功能:  处理链路初始化消息
输入:
pContent －消息内容指针
pLinkUser － 链路使用者指针
len   － 消息长度
msgType － 消息类型
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_msgToUser(XVOID *pContent, t_XOSUSERID *pLinkUser, XS32 len, e_TLMSG msgType)
{
    XS32 ret = 0;
    t_XOSCOMMHEAD *pMsg = NULL;

#ifdef INPUT_PAR_CHECK
    if((pContent == XNULL) || (pLinkUser == XNULL) ||(len <= 0) )
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_msg ToUser()->input param error!");
        return XERROR;
    }
#endif
    /*分配消息内存*/
    pMsg = XOS_MsgMemMalloc(FID_NTL,(XU32)len);
    if(pMsg == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_msg ToUser()->malloc msg failed!");
        return XERROR;
    }

    /*填写消息数据*/
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = FID_NTL;
    pMsg->length = (XU32)len;
    pMsg->msgID = msgType;
    pMsg->prio = eNormalMsgPrio;
    if(pLinkUser == XNULL )
    {
        if(pMsg->msgID == eDataInd)
        {
            XOS_MemFree(FID_NTL, ((t_DATAIND*)pContent)->pData);
        }
        XOS_MsgMemFree(FID_NTL, pMsg);
        return XERROR;
    }
    XOS_MemCpy(&(pMsg->datadest),pLinkUser,sizeof(t_XOSUSERID));
    pMsg->datadest.PID = pMsg->datasrc.PID; // 20110322 add,
    XOS_MemCpy(pMsg->message, pContent,(XU32)len);

    /*发送数据*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"NTL_msg ToUser()->send msg type[%d] to FID[%d] failed!",msgType,pLinkUser->FID);
        /*clean up */
        /*数据指示消息，应该先释放收到数据*/
        if(pMsg->msgID == eDataInd)
        {
            XOS_MemFree(FID_NTL,((t_DATAIND*)pContent)->pData);
        }
        XOS_MsgMemFree(FID_NTL, pMsg);
        return XERROR; 
    }

    return XSUCC;
}


/************************************************************************
函数名  : XOS_ReadNtlGenCfg
功能    : get XOS NTL configure informations
输入    : filename   XOS 配置文件名
输出    :
返回    : XBOOL
说明：
************************************************************************/
XBOOL XOS_ReadNtlGenCfg( t_NTLGENCFG* pNtl, XCHAR* filename )
{    
    /* none Vxworks OS */
#ifndef XOS_VXWORKS
    
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar* pTempStr   = XNULL;
    
#endif
    
    /*参数合法性检查*/
    if ( XNULL == pNtl || XNULL == filename )
    {
        return( XFALSE );
    }
    
    XOS_MemSet( pNtl, 0x00, sizeof(t_NTLGENCFG) );
    
#ifndef XOS_VXWORKS
#ifdef XOS_SCTP
    pNtl->maxSctpCliLink = SCTP_CLI_MIN_NUM;
    pNtl->maxSctpServLink = SCTP_SERV_MIN_NUM;
    pNtl->sctpClientsPerServ = SCTP_CLIENTS_PER_SERV_MIN_NUM;

    pNtl->hb_interval = SCTP_HB_PRIMITIVE_DEFAULT;
    pNtl->rto_min = SCTP_RTO_MIN_DEFAULT;
    pNtl->rto_init = SCTP_RTO_INIT_DEFAULT;
    pNtl->sack_timeout = SCTP_SACK_TIMEOUT_DEFAULT;
#endif

    /*读取文件*/
    doc = xmlParseFile(filename);
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    
    /*找到根节点*/
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    if ( XOS_StrCmp( cur->name, "MODULES") )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    
    /*找到 NTL 主节点*/
    while ( cur != XNULL )
    {
        if ( !XOS_StrCmp(cur->name, "NTLGENCFG" ) )
        {
            break;
        }
        cur = cur->next;
    }
    if ( XNULL == cur )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    cur = cur->xmlChildrenNode;
    while ( cur && xmlIsBlankNode ( cur ) )
    {
        cur = cur ->next;
    }
    if ( cur == XNULL )
    {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return( XFALSE );
    }
    /*遍历 NTL 子节点*/
    while ( cur != XNULL )
    {
        /*MAXUDPLINK 节点*/
        if ( !XOS_StrCmp(cur->name, "MAXUDPLINK" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->maxUdpLink = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        /*MAXTCPCLILINK 节点*/
        if ( !XOS_StrCmp(cur->name, "MAXTCPCLILINK" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->maxTcpCliLink = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        /*MAXTCPSERVLINK 节点*/
        if ( !XOS_StrCmp(cur->name, "MAXTCPSERVLINK" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->maxTcpServLink = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
#ifdef XOS_SCTP
        /*MAXSCTPCLILINK 节点*/
        if ( !XOS_StrCmp(cur->name, "MAXSCTPCLILINK" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->maxSctpCliLink = (XU16)atol( (char*)pTempStr );
                if(pNtl->maxSctpCliLink < SCTP_CLI_MIN_NUM || pNtl->maxSctpCliLink > SCTP_CLI_MAX_NUM)
                {
                    pNtl->maxSctpCliLink = SCTP_CLI_MIN_NUM;
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        /*MAXSCTPSERVLINK 节点*/
        if ( !XOS_StrCmp(cur->name, "MAXSCTPSERVLINK" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->maxSctpServLink = (XU16)atol( (char*)pTempStr );
                if(pNtl->maxSctpServLink < SCTP_SERV_MIN_NUM || pNtl->maxSctpServLink > SCTP_SERV_MAX_NUM)
                {
                    pNtl->maxSctpServLink = SCTP_SERV_MIN_NUM;
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }

        /*每个sctp服务端可接入的最大客户端数量*/
        if ( !XOS_StrCmp(cur->name, "SCTP_CLIENTS_PER_SERV" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->sctpClientsPerServ = (XU16)atol( (char*)pTempStr );
                if(pNtl->sctpClientsPerServ < SCTP_CLIENTS_PER_SERV_MIN_NUM || pNtl->sctpClientsPerServ > SCTP_CLIENTS_PER_SERV_MAX_NUM)
                {
                    pNtl->sctpClientsPerServ = SCTP_CLIENTS_PER_SERV_MIN_NUM;
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
            
        /*SCTP_HB_INTERVAL 节点*/
        if ( !XOS_StrCmp(cur->name, "SCTP_HB_INTERVAL" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->hb_interval = (XU16)atol( (char*)pTempStr );
                if(pNtl->hb_interval < SCTP_HB_PRIMITIVE_MIN)
                {
                    pNtl->hb_interval = SCTP_HB_PRIMITIVE_DEFAULT;
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        /*SCTP_RTO_MIN 节点*/
        if ( !XOS_StrCmp(cur->name, "SCTP_RTO_MIN" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->rto_min = (XU16)atol( (char*)pTempStr );
                if(pNtl->rto_min < SCTP_RTO_MIN_MIN || pNtl->rto_min > SCTP_RTO_MIN_MAX)
                {
                    pNtl->rto_min = SCTP_RTO_MIN_DEFAULT;
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        /*SCTP_RTO_INIT 节点*/
        if ( !XOS_StrCmp(cur->name, "SCTP_RTO_INIT" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->rto_init = (XU16)atol( (char*)pTempStr );
                if(pNtl->rto_init < SCTP_RTO_INIT_MIN || pNtl->rto_init > SCTP_RTO_INIT_MAX )
                {
                    pNtl->rto_init = SCTP_RTO_INIT_DEFAULT;
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }

        /*SCTP_SACK_TIMEOUT 节点*/
        if ( !XOS_StrCmp(cur->name, "SCTP_SACK_TIMEOUT" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->sack_timeout = (XU16)atol( (char*)pTempStr );
                if(pNtl->sack_timeout > SCTP_SACK_TIMEOUT_MAX )
                {
                    pNtl->sack_timeout = SCTP_SACK_TIMEOUT_DEFAULT;
                }
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }

#endif
        /*FDSPERTHRPOLLING 节点*/
        if ( !XOS_StrCmp(cur->name, "FDSPERTHRPOLLING" ) )
        {
            pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
            if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
            {
                pNtl->fdsPerThrPolling = (XU16)atol( (char*)pTempStr );
                xmlFree( pTempStr );
                pTempStr   = XNULL;
            }
        }
        
        cur = cur->next;
    }
    
    xmlFreeDoc(doc);
    xmlCleanupParser();
    
#endif /* end none XOS Vxworks */
    
    /* XOS Vxworks begin */
#ifdef XOS_VXWORKS
    pNtl->maxUdpLink       = 256;
    pNtl->maxTcpCliLink    = 256;
    pNtl->maxTcpServLink   = 256;
    pNtl->fdsPerThrPolling = 256;
        
#endif /*  XOS Vxworks end  */
    
    return(XTRUE);
}

/************************************************************************
函数名:NTL_udpRcvFunc
功能:  查找tcp 连接的客户
输入:
pTserverCb －server cb的指针
pClientAddr  － 接进来客户的地址指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XSTATIC XVOID  NTL_udpRcvFunc(XVOID* taskNo)
{
    t_SOCKSET udpReadSet;
    XU32 i = 0;
    XU32 inputNo = 0;
    XS16 setNum = 0;
    XS32 ret = 0;
    t_UDCB *pUdpCb = NULL;
    t_IPADDR fromAddr;
    XCHAR *pData = NULL;
    t_DATAIND dataInd;
    XS32 len = 0;
    XU32 pollAbideTime = 0;

    pollAbideTime = POLL_FD_TIME_OUT;
    inputNo=(XU32)(XPOINT)taskNo;
    while(1)
    {
        /*线程刚起来时，并没有sockfd，fd为空时select直接返回*/
        g_ntlCb.pUdpTsk[inputNo].activeFlag = XFALSE;

        /*等待start激活*/
        XOS_SemGet(&(g_ntlCb.pUdpTsk[inputNo].taskSemp));
        g_ntlCb.pUdpTsk[inputNo].activeFlag = XTRUE;
        XOS_SemPut(&(g_ntlCb.pUdpTsk[inputNo].taskSemp));

        /*对控制块进行操作*/
        /*将读集拷贝到局部变量中,便于不破坏全局的读集*/
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        XOS_INET_FD_ZERO(&(udpReadSet.fdSet));
        XOS_MemCpy(&udpReadSet, &(g_ntlCb.pUdpTsk[inputNo].setInfo.readSet), sizeof(t_SOCKSET));
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));

        /*poll sock fd set*/
        setNum = 0;

        ret = XINET_Select(&(udpReadSet.fdSet), (t_FDSET*)XNULLP, (XU32 *)&pollAbideTime, &setNum);
        if(ret != XSUCC)
        {
            /*select 超时*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_udpRcvFunc()->poll the fd set failed!");
                continue;
            }
        }
        /*如果网络上有数据读*/
        if(setNum > 0 )
        {
            XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
            for(i=(inputNo*(g_ntlCb.genCfg.fdsPerThrPolling)); i<(inputNo+1)*(g_ntlCb.genCfg.fdsPerThrPolling); i++)
            {
                pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH,i);

                if(pUdpCb == XNULLP || pUdpCb->linkState != eStateStarted || pUdpCb->sockFd.fd == XOS_INET_INV_SOCKFD) 
                {
                    continue;
                }

                if(XOS_INET_FD_ISSET(&(pUdpCb->sockFd),&(udpReadSet.fdSet)))
                {
                    setNum--;
                    XOS_MemSet(&fromAddr,0 ,sizeof(t_IPADDR));
                    pData = (XCHAR*)XNULLP;
                    len = XOS_INET_READ_ANY;

                    /*从网络上收数据*/
                    ret = XINET_RecvMsg(&(pUdpCb->sockFd),&fromAddr,&pData,
                        &len,XOS_INET_DGRAM,XOS_INET_CTR_NOCOMPATI_TCPEEN);
                    if(ret == XINET_CLOSE)
                    {
                        /*关闭链路*/
                        /*to do*/
                        continue;
                    }
                    else if(ret != XSUCC)
                    {
                        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_udpRcvFunc()->receive udp packect error[ret = %d]!",ret);
                        if(pData != (XCHAR*)XNULLP)
                        {
                            XOS_MemFree(FID_NTL,pData);
                        }
                        continue;
                    }

                    /*发送消息到上层*/
                    if((pData != XNULLP) && ( len > 0))
                    {
                        XOS_MemSet(&dataInd,0,sizeof(t_DATAIND));
                        dataInd.appHandle = pUdpCb->userHandle;
                        dataInd.dataLenth = (XU32)len;
                        dataInd.pData = pData;
                        XOS_MemCpy(&(dataInd.peerAddr),&fromAddr,sizeof(t_IPADDR));

#ifdef XOS_EBSC /* for EBSC */
#ifndef XOS_IPC_MGNT
                        if( FID_OAM == pUdpCb->linkUser.FID )
                        {
                            t_XOSCOMMHEAD *pTrueMsg = XNULL;
                            t_XOSCOMMHEAD *pOutMsg = XNULL;
                            if( (0 == (XU8)(XU32)pUdpCb->userHandle)/* 板间通信,基站 */
                                || (8 == (XU8)(XU32)pUdpCb->userHandle)/* SS */
                                || (9 == (XU8)(XU32)pUdpCb->userHandle) )/* 模拟测试 */
                            {
                                pTrueMsg = (t_XOSCOMMHEAD*)pData;
                                pTrueMsg->message = (XU8*)pTrueMsg + sizeof(t_XOSCOMMHEAD);
                                if( 1000 <= pTrueMsg->length )
                                {
                                    XOS_Trace(MD(FID_NTL,PL_ERR),"\r\n error msg_redir msglen(%d)",pTrueMsg->length);
                                    XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
                                    return;
                                }
                                pOutMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(pTrueMsg->datasrc.FID,pTrueMsg->length);
                                if( XNULL == pOutMsg )
                                {
                                    XOS_Trace(MD(FID_NTL,PL_ERR),"\r\n Fail: XOS_MsgMemMalloc() fid=%d,len=%d When NTL_udpRcvFunc()!",pTrueMsg->datasrc.FID,pTrueMsg->length);
                                    XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
                                    return;
                                }
                                XOS_MemCpy(pOutMsg->message,pTrueMsg->message,pTrueMsg->length);
                                XOS_MemCpy(pOutMsg,pTrueMsg,sizeof(t_XOSCOMMHEAD)-4);

                                if( XSUCC != XOS_MsgSend(pOutMsg) )
                                {
                                    XOS_Trace(MD(FID_NTL,PL_ERR),"\r\n Fail: XOS_MsgSend() When NTL_udpRcvFunc()!");
                                    XOS_MsgMemFree(pOutMsg->datasrc.FID,(t_XOSCOMMHEAD *)pOutMsg);
                                    XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
                                    return;
                                }
                                XOS_MemFree(FID_NTL,pData);
                                continue;
                            }
                            else
                            {
                                XOS_Trace(MD(FID_NTL,PL_ERR),"other sock(%d)", pUdpCb->userHandle);
                            }
                        }
#endif
#endif
                        NTL_msgToUser(&dataInd,&(pUdpCb->linkUser),sizeof(t_DATAIND),eDataInd);
                    }
                }
                if(setNum == 0)
                {
                    break;
                }
            }/*end of for polling */
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));

            if(setNum > 0) /*由于客户端被删除，所以有事件不能被处理*/
            {
                XOS_Trace(MD(FID_NTL,PL_ERR), "NTL_udpRcvFunc setNum %d error", setNum);
            }            
        }
        XOS_Sleep(1);
    }
}


/************************************************************************
函数名:NTL_noticeCloseTCli
功能:  通知关闭tcp的客户端
输入: pTcpCliCb   客户端控制块指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XSTATIC XS32 NTL_noticeCloseTCli(t_TCCB* pTcpCliCb)
{
    t_XOSCOMMHEAD *pCloseMsg = XNULL;
    t_LINKCLOSEREQ *pCloseReq = XNULL;
    XS32 ret = 0;

    if(!pTcpCliCb)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_noticeCloseTCli()->pTcpCliCb is null!");
        return XERROR;
    }
    pCloseMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_NTL,sizeof(t_LINKCLOSEREQ));
    if(pCloseMsg == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_noticeCloseTCli()->malloc close msg failed!");
        return XERROR;
    }
    pCloseMsg->datasrc.FID = FID_NTL;
    pCloseMsg->datasrc.PID = XOS_GetLocalPID();
    pCloseMsg->datadest.FID = FID_NTL;
    pCloseMsg->datadest.PID =  XOS_GetLocalPID();
    pCloseMsg->length = sizeof(t_LINKCLOSEREQ);
    pCloseMsg->msgID = eLinkStop;
    pCloseMsg->prio = eHAPrio;
    pCloseReq = (t_LINKCLOSEREQ*)(pCloseMsg->message);
    pCloseReq->linkHandle = pTcpCliCb->linkHandle;

    ret = XOS_MsgSend(pCloseMsg);
    if(ret != XSUCC)
    {
        XOS_MsgMemFree(FID_NTL,pCloseMsg);
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_noticeCloseTCli()->send close msg failed!");
        return XERROR;
    }

    return XSUCC;
}


/************************************************************************
函数名:
功能:  关闭tcp客户端
输入:
taskNo       任务号
pTcpCliCb   客户端控制块指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: 该功能接口需要扩充参数最好,主动关闭;被动关闭.
************************************************************************/
XS32 NTL_closeTCli(XU32 taskNo, t_TCCB *pTcpCliCb,int close_type)
{
    /*error check on parameters 2007/09/11 add below*/
    if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
    {
        return XERROR;
    }
    
    if(pTcpCliCb == XNULLP)
    {
        return (XERROR);
    }
    if(XOS_INET_INV_SOCK_FD(&(pTcpCliCb->sockFd)))
    {
        return (XERROR);
    }
        
    /*重置读写集*/
    /*清空客户端的数据*/
    NTL_dataReqClear(&(pTcpCliCb->packetlist));
    switch(close_type)
    {
    case NTL_SHTDWN_RECV:
        /*清理read set */
        XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));
        break;
    case NTL_SHTDWN_SEND:
        /*清理read set */
        XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));
        /*清理write set */
        /*VxWorks环境下当成员fd超过FD_SIZE时,其掩码位置会置其它位置bitMaxsk[FD_SIZE/8]*/
        XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));
        break;
    default:
        XOS_Trace(MD(FID_NTL,PL_MIN),"NTL closeTCli unsupport close type %d",close_type);
        return (XERROR);
    } 

#ifdef XOS_WIN321
    if(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet.fd_count == 0
        && g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet.fd_count == 0)
#else
    if(--g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum == 0)
#endif
    {
        /*当客户端数量为0时，捕获客户端任务的驱动信号量，导致其下次阻塞，等待下次有客户端任务时再启动*/
        XOS_SemGet(&(g_ntlCb.pTcpCliTsk[taskNo].taskSemp));
    }

    if(g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum  == 0xffff)
    {
        g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum = 0;
    }

    /*关闭socket*/
    if ( XSUCC != XINET_CloseSock(&(pTcpCliCb->sockFd)))
    {
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
函数名:NTL_ResetAllTcpFd
功能:  清理所有的tcp读写集
输入:  taskNo  任务号
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_ResetAllTcpFd(XU32 taskNo)
{
    /*重置读写集*/
    /*清理read set */
    XOS_INET_FD_ZERO(&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));
    /*清理write set */
    /*VxWorks环境下当成员fd超过FD_SIZE时,其掩码位置会置其它位置bitMaxsk[FD_SIZE/8]*/
    XOS_INET_FD_ZERO(&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));

    g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum = 0;

    return XSUCC;
}

/************************************************************************
函数名:NTL_tcpCliRcvFunc
功能:  tcp客户端的入口函数
输入:taskNo  任务号
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XSTATIC XVOID  NTL_tcpCliRcvFunc(XVOID* taskPara)
{
    XU32 taskNo = 0;
    t_FDSET tcpCliRead;
    t_FDSET tcpCliWrite;
    t_FDSET *pReadSet = NULL;
    t_FDSET *pWriteSet = NULL;
    t_DATAIND dataInd;
    t_TCCB *pTcpCliCb = NULL;
    t_LINKCLOSEIND closeInd;
    t_STARTACK tcpStartAck;
    XS32 ret = 0;
    XS16 setNum = 0;
    XCHAR *pData = NULL;
    XS32 len = 0;
    XU32 i = 0;
    XU32 pollAbideTime = 0;
    XU32 nDealFlg = 0;

    /*毫秒*/
    pollAbideTime = POLL_FD_TIME_OUT;
    taskNo = (XU32)(XPOINT)taskPara;

    while(1)
    {
        /*线程刚起来时，并没有sockfd，fd为空时select直接返回*/
        g_ntlCb.pTcpCliTsk[taskNo].activeFlag = XFALSE;
        
        XOS_SemGet(&(g_ntlCb.pTcpCliTsk[taskNo].taskSemp));
        g_ntlCb.pTcpCliTsk[taskNo].activeFlag = XTRUE;
        XOS_SemPut(&(g_ntlCb.pTcpCliTsk[taskNo].taskSemp));

        /*拷贝到局部变量中*/
        XOS_MemSet(&tcpCliRead,0,sizeof(t_FDSET));
        XOS_MemSet(&tcpCliWrite,0,sizeof(t_FDSET));
        pReadSet = (t_FDSET*)XNULLP;
        pWriteSet = (t_FDSET*)XNULLP;


        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));  
        /*如果控制块为空*/
        if(XOS_ArrayGetCurElemNum(g_ntlCb.tcpClientLinkH) == 0)
        {
            NTL_ResetAllTcpFd(taskNo);
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            XOS_Sleep(1);
            continue;
        }


#ifdef XOS_NEED_CHK
#ifdef XOS_WIN321
        if(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet.fd_count > 0
            ||(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet.fd_count >0))
#else
        if(g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum > 0)
#endif
        {    

            /*copy read set */
            XOS_INET_FD_ZERO(&(tcpCliRead));
            XOS_MemCpy(&tcpCliRead,&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet),sizeof(t_FDSET));
            pReadSet = &tcpCliRead;
            
            /*copy write set*/
            XOS_INET_FD_ZERO(&(tcpCliWrite));
            XOS_MemCpy(&tcpCliWrite,&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet),sizeof(t_FDSET));
            pWriteSet = &tcpCliWrite;
        }
        else
        {
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            XOS_Sleep(1);
            continue;
        }
#endif
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
        /*poll the socket */
        setNum = 0;
        ret = XINET_Select(pReadSet,pWriteSet,(XU32*)&pollAbideTime,(XS16*)&setNum);
        if(ret != XSUCC)
        {
            /*select 超时*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else  /*select 出错，应该是立即返回的，如果再continue，还是立即返回，可能死循环*/
            {    /*由于这里没有使用互斥，很有可能fd被关闭了，却在参与select，导致异常*/
                XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_tcpCliRcvFunc,select readset/writeset failed,return setNum =%d.",setNum);
                continue;
            }
        }

        nDealFlg = 0;
        if(setNum > 0)
        {
            XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));
            for(i=(taskNo*(g_ntlCb.genCfg.fdsPerThrPolling)); i<(taskNo+1)*(g_ntlCb.genCfg.fdsPerThrPolling); i++)
            {
                pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,i);
                
                if(pTcpCliCb != XNULLP)
                {
                    nDealFlg++;
                    /*客户端未启动链路或链路被重置了*/
                    if( ((pTcpCliCb->linkState != eStateConnected)&&(pTcpCliCb->linkState != eStateConnecting))
                        ||(pTcpCliCb->sockFd.fd == XOS_INET_INV_SOCKFD))
                    {
                        g_NtlTest1++;
                        continue;
                    }

                    /*可写*/
                    if((pWriteSet != XNULLP) && XOS_INET_FD_ISSET(&(pTcpCliCb->sockFd),pWriteSet))
                    {
                        setNum--;        

                        /*同时可读可写，说明异常*/
                        if(XOS_INET_FD_ISSET(&(pTcpCliCb->sockFd),pReadSet) || pTcpCliCb->sockFd.fd <= 0)
                        {
                            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_tcpCliRcvFunc(),tcp client fd = %d disconnect,try reconnect!",
                                                             pTcpCliCb->sockFd.fd);
                            /*关定时器*/
                            XOS_TimerStop(FID_NTL,pTcpCliCb->timerId);
                            /*停止链路*/
                            NTL_closeTCli((XS32)taskNo,pTcpCliCb,NTL_SHTDWN_SEND);
                            /*通知状态改变*/
                            NTL_noticeCloseTCli( pTcpCliCb);
                            /*发送关闭指示*/
                            closeInd.appHandle = pTcpCliCb->userHandle;
                            closeInd.closeReason = ePeerReq;
                            NTL_msgToUser((XVOID*)&closeInd,&(pTcpCliCb->linkUser),sizeof(t_LINKCLOSEIND),eStopInd);
                            
                            continue;
                        }
                        
                        if(XSUCC != XINET_TcpConnectCheck(&(pTcpCliCb->sockFd)))
                        {
                            XOS_Trace(MD(FID_NTL,PL_WARN),
                            "NTL_tcpCliRcvFunc()->connect tcp server failed! myaddr.ip = 0x%08x,myaddr.port = %d;peeraddr.ip = 0x%08x,peeraddr.port = %d",
                            pTcpCliCb->myAddr.ip, pTcpCliCb->myAddr.port, pTcpCliCb->peerAddr.ip, pTcpCliCb->peerAddr.port);
                                
                            /*从write 集中清除,等待定时器到期进行处理*/
                            XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));
                            
                            continue;
                        }

                        /*从 写 集中清除*/
                        XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));
                        /*添加到 读 集合中*/
                        XOS_INET_FD_SET(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));

                        /*客户端连接成功*/
                        if(pTcpCliCb->linkState != eStateConnected)
                        {
                            pTcpCliCb->linkState = eStateConnected;
                            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_tcpCliRcvFunc(),connect tcp server successed!");
                            NTL_StopTcpClientTimer(pTcpCliCb);

                            /*发送启动成功消息到上层*/
                            tcpStartAck.appHandle = pTcpCliCb->userHandle;
                            tcpStartAck.linkStartResult = eSUCC;
                            XOS_MemCpy(&(tcpStartAck.localAddr),&(pTcpCliCb->myAddr),sizeof(t_IPADDR));

                            NTL_msgToUser((XVOID*)&tcpStartAck,&(pTcpCliCb->linkUser),sizeof(t_STARTACK),eStartAck);
                        }
                    }
                    if(pTcpCliCb->sockFd.fd == XOS_INET_INV_SOCKFD)
                    {
                        continue;
                    }
                    /*可读或有异常，都通过这个case来处理*/
                    if((pReadSet != XNULLP) && XOS_INET_FD_ISSET(&(pTcpCliCb->sockFd),pReadSet))
                    {
                        setNum--;
                        pData = (XCHAR*)XNULLP;
                        len = XOS_INET_READ_ANY;

                        /*从网络上收数据*/
                        ret = XINET_RecvMsg(&(pTcpCliCb->sockFd),(t_IPADDR*)XNULLP,
                            &pData,&len,XOS_INET_STREAM,XOS_INET_CTR_COMPATIBLE_TCPEEN);
                        if(ret == XINET_CLOSE)
                        {
                            /* if tcp server closed then notify all this client*/
                            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_tcpCliRcvFunc(),tcp client disconnect,try reconnect!");
                            /*关定时器*/
                            pTcpCliCb->linkState = eStateWaitClose;
                            NTL_StopTcpClientTimer(pTcpCliCb);

                            NTL_closeTCli((XS32)taskNo, pTcpCliCb, NTL_SHTDWN_SEND);
                            /*通知状态改变*/
                            NTL_noticeCloseTCli( pTcpCliCb);
                            /*发送关闭指示*/
                            closeInd.appHandle = pTcpCliCb->userHandle;
                            closeInd.closeReason = ePeerReq;
                            NTL_msgToUser((XVOID*)&closeInd,&(pTcpCliCb->linkUser),sizeof(t_LINKCLOSEIND),eStopInd);
                        }
                        else if(ret != XSUCC)
                        {
                            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_tcpCliRcvFunc()->receive tcp data error[ret = %d]!",ret);
                        }
#ifdef TPKT_NEED
                        /*tpkt to do */
#endif
                        if(pData != XNULLP && len > 0)
                        {
                            /*发送数据到上层*/
                            XOS_MemSet(&dataInd,0,sizeof(t_DATAIND));
                            dataInd.appHandle = pTcpCliCb->userHandle;
                            dataInd.dataLenth = (XU32)len;
                            dataInd.pData = pData;
                            XOS_MemCpy(&(dataInd.peerAddr),&(pTcpCliCb->peerAddr),sizeof(t_IPADDR));

                            ret = NTL_msgToUser((XVOID*)&dataInd,&(pTcpCliCb->linkUser),sizeof(t_DATAIND),eDataInd);
                            if(ret != XSUCC)
                            {
                                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpCliRcvFunc()->send data to user error[ret = %d]!",ret);
                                continue;
                            }
                        }
                    }

                    /* 已经检查所有的被置位的socket*/
                    if(setNum <= 0)
                    {
                        break;
                    }
                }
            }
            if(setNum > 0) /*有数据未读写，即将导致下一次select立即返回*/
            {
                g_NtlTest4++;
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpCliRcvFunc()->deal data error! curFdNum = %d, nDealFlg = %d",
                          setNum, nDealFlg);
            }
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
        }
    }
}


/************************************************************************
函数名:NTL_closeTsCli
功能: 关闭一个tcp server 接入的客户端
输入:pTcpServCli －server client 的控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_closeTsCli(t_TSCLI* pTcpServCli)
{
    if(pTcpServCli == XNULLP)
    {
        return XERROR;
    }
    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client begin:serverClis sockNum=%d",g_ntlCb.tcpServTsk.setInfo.sockNum);
    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client begin:closing client sock=%d",pTcpServCli->sockFd.fd);

    /*关闭sock ，清理资源*/

#ifdef XOS_NEED_CHK
    NTL_dataReqClear(&(pTcpServCli->packetlist));/*20080729 added*/

    if(XOS_INET_INV_SOCK_FD(&(pTcpServCli->sockFd)))
    {
        return (XERROR);
    }
#endif

    XOS_INET_FD_CLR(&(pTcpServCli->sockFd),&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));

#ifdef  XOS_WIN321
    if(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet.fd_count == 0)
#else
    if(--g_ntlCb.tcpServTsk.setInfo.sockNum == 0)
#endif
    {
        XOS_SemGet(&(g_ntlCb.tcpServTsk.taskSemp));
    }
    if(g_ntlCb.tcpServTsk.setInfo.sockNum == 0xffff)
    {
        g_ntlCb.tcpServTsk.setInfo.sockNum=0;
    }
    XINET_CloseSock(&(pTcpServCli->sockFd));

    /*从链表中断开,修改server cb 的数据*/
    (pTcpServCli->pServerElem)->usageNum--;
    /*是链表尾部的节点*/
    if(pTcpServCli->pNextCli == XNULLP)
    {
        (pTcpServCli->pServerElem)->pLatestCli = pTcpServCli->pPreCli;
    }
    else
    {
        pTcpServCli->pNextCli->pPreCli = pTcpServCli->pPreCli;
    }
    /*不是头节点*/
    if( pTcpServCli->pPreCli != XNULLP)
    {
        pTcpServCli->pPreCli->pNextCli = pTcpServCli->pNextCli;
    }

    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client end: serverClis  sockNum=%d",g_ntlCb.tcpServTsk.setInfo.sockNum);
    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client end: closed  client sock=%d",pTcpServCli->sockFd.fd);
    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client end: current client num=%d.\r\n",(pTcpServCli->pServerElem)->usageNum);

    /*从hash 中删除*/
    XOS_HashDelByElem(g_ntlCb.tSerCliH, pTcpServCli);

    return XSUCC;
}


/************************************************************************
函数名:NTL_closeReqProc
功能:  处理链路关闭请求消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XSTATIC XS32 NTL_closeReqProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKCLOSEREQ *pCloseReq = NULL;
    e_LINKTYPE linkType;
    t_PARA  timerParam;
    t_BACKPARA backPara;
    t_UDCB *pUdpCb = NULL;
    t_TCCB *pTcpCliCb = NULL;
    t_TSCB *pTcpServCb = NULL;
    XU32 taskNo = 0;
    XS32 ret  = 0;
    t_TSCLI *pTcpServerClient = NULL;

    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_closeReqProc()->pMsg invalid!");
        return XERROR;
    }
    pCloseReq = (t_LINKCLOSEREQ*)(pMsg->message);
    if(!pCloseReq)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_closeReqProc()->pCloseReq invalid!");
        return XERROR;
    }

    /*下行的消息都要检查句柄的的有效性*/
    if(!NTL_isValidLinkH(pCloseReq->linkHandle))
    {
        /*链路句柄无效的消息，直接丢弃*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_closeReqProc()->FID[%d] linkHandle invalid!", pMsg->datasrc.FID);
        return XERROR;
    }
    /*获取链路类型*/
    linkType = NTL_getLinkType(pCloseReq->linkHandle);
    XOS_Trace(MD(FID_NTL,PL_DBG),"NTL_closeReqProc()->linkType:%d,linkHandle:%x!",linkType,pCloseReq->linkHandle);

    switch(linkType)
    {
    case eUDP:
        /*获取udp 控制块*/
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(pUdpCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_closeReqProc()->FID[%d] get the udp control block failed!", pMsg->datasrc.FID);
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return XERROR;
        }
        
        /*获取链路所在任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pCloseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.udpTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_closeReqProc()->get the udp taskno failed!");
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return XERROR;
        }

        /*检查链路状态*/
        if(pUdpCb->linkState != eStateStarted)
        {
            /*链路状态不对,消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_closeReqProc()->udp link state is not started!");
            ret = (pUdpCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return ret;
        }

        
        /*关闭socket，清除fdset*/
        XOS_INET_FD_CLR(&(pUdpCb->sockFd),&(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet));
#ifdef  XOS_WIN321
        if(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet.fd_count == 0)
#else
        if(--g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum == 0)
#endif
        {
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            XOS_SemGet(&(g_ntlCb.pUdpTsk[taskNo].taskSemp));
            XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        }
        if(g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum == 0xffff)
        {
            g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum = 0;
        }

        XINET_CloseSock(&(pUdpCb->sockFd));
        /*改变链路状态到初始化状态*/
        pUdpCb->linkState = eStateInited;
        pUdpCb->peerAddr.ip = 0;
        pUdpCb->peerAddr.port = 0;        
        pUdpCb->myAddr.ip = 0;
        pUdpCb->myAddr.port = 0;
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));

        break;

    case eTCPClient:
        /*获取tcp 控制块*/
        pTcpCliCb = (t_TCCB*)XNULLP;

        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));    
        pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        
        
        if(pTcpCliCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_closeReqProc()->get the tcp client control block failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            return XERROR;
        }

        /*获取链路所在任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pCloseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_closeReqProc()->get the tcp client taskno failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            return XERROR;
        }

        /*如果是网络关闭，要超时重联*/
        if(pMsg->datasrc.FID == FID_NTL)
        {
            /*初始化*/
            pTcpCliCb->linkState = eStateInited;
            pTcpCliCb->expireTimes = 0;

            /*启动超时重联定时器*/            
            XOS_MemSet(&timerParam,0,sizeof(t_PARA));
            timerParam.fid = FID_NTL;
            timerParam.len = TCP_CLI_RECONNECT_INTERVAL;
            timerParam.mode = TIMER_TYPE_LOOP;
            timerParam.pre = TIMER_PRE_LOW;

            XOS_MemSet(&backPara,0,sizeof(t_BACKPARA));
            backPara.para1 = (XPOINT)pTcpCliCb;
            backPara.para2 = (XPOINT)taskNo;

            /*在这段时间里可能又收到新的发送数据了*/
            NTL_dataReqClear(&(pTcpCliCb->packetlist));/*20080818 added,清除链路之前接收到的数据包*/

            XOS_INIT_THDLE (pTcpCliCb->timerId);
            XOS_TimerStart(&(pTcpCliCb->timerId),&timerParam, &backPara);
        }
        else /*用户请求关闭*/
        {
            NTL_StopTcpClientTimer(pTcpCliCb);          

            /*如果正处于网络关闭后，用户再来关闭，则不需要再关闭，关闭fd, 清空数据，重置读写集*/
            if(pTcpCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
            {
                NTL_closeTCli(taskNo, pTcpCliCb, NTL_SHTDWN_SEND);
            }
            else
            {
                /*在这段时间里可能又收到新的发送数据了*/
                NTL_dataReqClear(&(pTcpCliCb->packetlist));/*20080818 added,清除链路之前接收到的数据包*/
            }

            pTcpCliCb->linkState = eStateInited;
            pTcpCliCb->expireTimes = 0;
            /*将对端地址置空*/
            pTcpCliCb->peerAddr.ip = 0;
            pTcpCliCb->peerAddr.port = 0;            
            pTcpCliCb->myAddr.ip = 0;
            pTcpCliCb->myAddr.port = 0;

        }
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));

        break;
    case eTCPServer:
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        pTcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        
        if(pTcpServCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_closeReqProc()->get the tcp server control block failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return XERROR;
        }
        if(pTcpServCb->linkState != eStateListening)
        {
            /*链路状态不对,消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_closeReqProc()->tcp serv link [linkhandle:%x] state [%d] is wrong!"
            ,pTcpServCb->linkHandle,pTcpServCb->linkState);
            ret = (pTcpServCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return ret;
        }           
       
        
        if(0x00 == pCloseReq->cliAddr.ip && 0x00 == pCloseReq->cliAddr.port)
        {
            /*首先关闭所有接入的客户端*/     
            while(pTcpServCb->pLatestCli != XNULLP)
            {
                NTL_closeTsCli(pTcpServCb->pLatestCli);
                if(pTcpServCb->usageNum == 0)
                {
                    break;
                }
            }

            /*关闭listen 的fd*/
            XOS_INET_FD_CLR(&(pTcpServCb->sockFd),&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));
#ifdef XOS_WIN321
            if(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet.fd_count == 0)
#else
            if(--g_ntlCb.tcpServTsk.setInfo.sockNum == 0)
#endif
            {
                XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
                XOS_SemGet(&(g_ntlCb.tcpServTsk.taskSemp));
                XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
            }
            if(g_ntlCb.tcpServTsk.setInfo.sockNum == 0xffff)
            {
                g_ntlCb.tcpServTsk.setInfo.sockNum =0;
            }

            /*关闭fd*/
            XINET_CloseSock(&(pTcpServCb->sockFd));
            
            /*改变相应的cb 数据,初始化*/
            pTcpServCb->authFunc = (XOS_TLIncomeAuth)XNULL;
            pTcpServCb->linkState = eStateInited;
            pTcpServCb->maxCliNum = 0;
            pTcpServCb->usageNum = 0;
            pTcpServCb->pLatestCli = (t_TSCLI*)XNULLP;
        }
        else
        {   

            /*查找指定的客户端控制块*/
            pTcpServerClient = (t_TSCLI*)XOS_HashElemFind(g_ntlCb.tSerCliH, (XVOID*)&(pCloseReq->cliAddr));
            if(NULL != pTcpServerClient)
            {
                XOS_Trace(MD(FID_NTL, PL_INFO),"close tcp client ip=0x%08x, port=%d", pCloseReq->cliAddr.ip, pCloseReq->cliAddr.port);
                NTL_closeTsCli(pTcpServerClient);
            }
        }
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));

        break;
#ifdef XOS_SCTP
    case eSCTPClient:
    case eSCTPServer:
        SCTP_closeReqProc(pMsg);
        break;
#endif
    default:
        break;
    }

    return XSUCC;
}


/************************************************************************
函数名:NTL_closeReqForRelease
功能:  处理链路释放请求消息--只能由NTL_linkReleaseProc调用
输入:  pMsg －消息指针
输出:
返回:  成功返回XSUCC,否则返回XERROR
说明:  此函数与NTL_closeReqProc函数在tcpserver上处理不同
************************************************************************/
XSTATIC XS32 NTL_ReleaseLink(t_XOSCOMMHEAD* pMsg)
{
    t_LINKRELEASE *pReleaseReq = NULL;
    e_LINKTYPE linkType;
    t_UDCB *pUdpCb = NULL;
    t_TCCB *pTcpCliCb = NULL;
    t_TSCB *pTcpServCb = NULL;
    XU32 taskNo = 0;
    XS32 ret  = 0;

    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ReleaseLink()->pMsg invalid!");
        return XERROR;
    }
    pReleaseReq = (t_LINKRELEASE*)(pMsg->message);
    if(!pReleaseReq)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ReleaseLink()->pCloseReq invalid!");
        return XERROR;
    }

    /*下行的消息都要检查句柄的的有效性*/
    if(!NTL_isValidLinkH(pReleaseReq->linkHandle))
    {
        /*链路句柄无效的消息，直接丢弃*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ReleaseLink()->FID[%d] linkHandle invalid!", pMsg->datasrc.FID);
        return XERROR;
    }
    /*获取链路类型*/
    linkType = NTL_getLinkType(pReleaseReq->linkHandle);

    switch(linkType)
    {
    case eUDP:
        /*获取udp 控制块*/
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH,NTL_getLinkIndex(pReleaseReq->linkHandle));
        if(pUdpCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ReleaseLink()->FID[%d] get the udp control block failed!", pMsg->datasrc.FID);
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return XERROR;
        }
        
        /*获取链路所在任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pReleaseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.udpTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ReleaseLink()->get the udp taskno failed!");
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return XERROR;
        }

        /*检查链路状态*/
        if(pUdpCb->linkState != eStateStarted)
        {
            /*链路状态不对,消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_ReleaseLink()->udp link state is not started!");
            ret = (pUdpCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return ret;
        }

        
        /*关闭socket，清除fdset*/
        XOS_INET_FD_CLR(&(pUdpCb->sockFd),&(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet));
#ifdef  XOS_WIN321
        if(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet.fd_count == 0)
#else
        if(--g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum == 0)
#endif
        {
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            XOS_SemGet(&(g_ntlCb.pUdpTsk[taskNo].taskSemp));
            XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        }
        if(g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum == 0xffff)
        {
            g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum = 0;
        }

        XINET_CloseSock(&(pUdpCb->sockFd));
        /*改变链路状态到初始化状态*/
        pUdpCb->linkState = eStateInited;
        pUdpCb->peerAddr.ip = 0;
        pUdpCb->peerAddr.port = 0;        
        pUdpCb->myAddr.ip = 0;
        pUdpCb->myAddr.port = 0;
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));

        break;

    case eTCPClient:
        /*获取tcp 控制块*/
        pTcpCliCb = (t_TCCB*)XNULLP;

        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));        
        pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,NTL_getLinkIndex(pReleaseReq->linkHandle));
        
        if(pTcpCliCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_ReleaseLink()->get the tcp client control block failed!");
            return XERROR;
        }

        /*获取链路所在任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pReleaseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
        {
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_ReleaseLink()->get the tcp client taskno failed!");
            return XERROR;
        }

		NTL_StopTcpClientTimer(pTcpCliCb);          

		/*如果正处于网络关闭后，用户再来关闭，则不需要再关闭，关闭fd, 清空数据，重置读写集*/
		if(pTcpCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
		{
			NTL_closeTCli(taskNo, pTcpCliCb, NTL_SHTDWN_SEND);
		}
		else
		{
			/*在这段时间里可能又收到新的发送数据了*/
			NTL_dataReqClear(&(pTcpCliCb->packetlist));/*20080818 added,清除链路之前接收到的数据包*/
		}

		pTcpCliCb->linkState = eStateInited;
		pTcpCliCb->expireTimes = 0;
		/*将对端地址置空*/
		pTcpCliCb->peerAddr.ip = 0;
		pTcpCliCb->peerAddr.port = 0;
		pTcpCliCb->myAddr.ip = 0;
		pTcpCliCb->myAddr.port = 0;

        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
        break;
    case eTCPServer:
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        pTcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,NTL_getLinkIndex(pReleaseReq->linkHandle));
        
        if(pTcpServCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_ReleaseLink()->get the tcp server control block failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return XERROR;
        }
        if(pTcpServCb->linkState != eStateListening)
        {
            /*链路状态不对,消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_ReleaseLink()->tcp serv  link state is wrong!");
            ret = (pTcpServCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return ret;
        }

        /*关闭tcp服务端*/
        ret = NTL_CloseTcpServerSocket(pTcpServCb);
        if( ret != XSUCC)
        {
            g_tcpSerCloseFail++;
        }
        
        /*改变相应的cb 数据,初始化*/
        pTcpServCb->authFunc = (XOS_TLIncomeAuth)XNULL;
        pTcpServCb->linkState = eStateInited;
        pTcpServCb->maxCliNum = 0;
        pTcpServCb->usageNum = 0;
        pTcpServCb->pLatestCli = (t_TSCLI*)XNULLP;
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
        break;
#ifdef XOS_SCTP
    case eSCTPClient:
    case eSCTPServer:
        ret = SCTP_ReleaseLink(pMsg);
        return ret;
#endif
    default:
        break;
    }

    return XSUCC;
}


/************************************************************************
* NTL_pollingHash
* 功能: 定义对每个hash元素通用的函数
* 输入  :
* hHash   - 对象句柄
* elem    - 元素
* param   - 参数
************************************************************************/
XSTATIC  XVOID* NTL_pollingHash(XOS_HHASH hHash,XVOID *elem,XVOID *param)
{
    t_TSCLI *pTservCli = NULL;
    t_FDSET *pReadSet = NULL;
    XCHAR *pData = NULL;
    XS32 len = 0;
    XS32 ret = 0;
    t_DATAIND dataInd;
    t_IPADDR *pAddr = NULL;
    t_LINKCLOSEIND tscCloseInd;

    if( elem == XNULLP || !param)
    {
        return param;
    }

    pTservCli = (t_TSCLI*)elem;
    pReadSet = (t_FDSET*)param;
    /* 检查*/
    if(XOS_INET_FD_ISSET(&(pTservCli->sockFd),pReadSet))
    {
        /*从网络上收数据*/
        len = XOS_INET_READ_ANY;
        ret = XINET_RecvMsg(&(pTservCli->sockFd),(t_IPADDR*)XNULLP,&pData,&len,XOS_INET_STREAM,XOS_INET_CTR_COMPATIBLE_TCPEEN);
        if(ret != XSUCC)
        {
            if(ret == XINET_CLOSE)
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_pollHash()-> tcp server close client link sock=[%d]!",pTservCli->sockFd.fd);
                XOS_MemSet(&tscCloseInd,0,sizeof(t_LINKCLOSEIND));
                tscCloseInd.appHandle = pTservCli->pServerElem->userHandle;
                tscCloseInd.closeReason = ePeerReq;
                pAddr = (t_IPADDR*)XNULLP;
                pAddr = (t_IPADDR*)XOS_HashGetKeyByElem(hHash,pTservCli);
                if(pAddr == XNULLP)
                {
                    XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_pollHash()->connect indication get peer address failed!");
                    NTL_closeTsCli(pTservCli);
                    return param;
                }
                XOS_MemCpy(&(tscCloseInd.peerAddr),pAddr,sizeof(t_IPADDR));
                NTL_msgToUser(&tscCloseInd,&(pTservCli->pServerElem->linkUser),sizeof(t_LINKCLOSEIND),eStopInd);
                NTL_closeTsCli(pTservCli);
                return param;
            }
            else
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_pollHash()-> tcp server client sock=[%d] receive msg error!",pTservCli->sockFd.fd);
                return param;
            }
        }

        /*发送数据到上层用户*/
        dataInd.appHandle = pTservCli->pServerElem->userHandle;
        dataInd.dataLenth = (XU32)len;
        dataInd.pData = pData;
        pAddr = (t_IPADDR*)XNULLP;
        pAddr = (t_IPADDR*)XOS_HashGetKeyByElem(hHash,pTservCli);
        if( pAddr != XNULLP)
        {
            XOS_MemCpy (&(dataInd.peerAddr),pAddr,sizeof(t_IPADDR));
        }
        else
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_pollHash()-> get hash key error!" );
        }

        NTL_msgToUser(&dataInd,&(pTservCli->pServerElem->linkUser),sizeof(t_DATAIND),eDataInd);
    }

    return param;
}

/************************************************************************
函数名:NTL_tcpServerDataCheckHash
功能:  遍历客户端fd信息
输入:
输出:
返回:
说明:
************************************************************************/
XSTATIC  XVOID* NTL_tcpServerDataCheckHash(XOS_HHASH hHash,XVOID *elem,XVOID *param)
{
    t_TSCLI *pTservCli = NULL;

    pTservCli = (t_TSCLI*)elem;

    if(NULL != pTservCli)
    {
        write_to_syslog("tcpserver client fd = %d\r\n", pTservCli->sockFd.fd);
    }

    return NULL;
}

/************************************************************************
函数名:NTL_tcpServerDataCheck
功能:  记录tcpserver的错误检测信息
输入:
输出:
返回:
说明:
************************************************************************/
void NTL_tcpServerDataCheck(void)
{
#ifdef XOS_LINUX
    XS32 i = 0;
    char szBuf[4096]  = {0};
    t_TSCB  *pTcpServCb = NULL;
    char *pTmp = szBuf;
    char *pRead = NULL, *pWrite = NULL;
    XS32 pid = 0;
    char szCmd[64] = {0};
    XS32 maxfdByte = (XOS_FD_SETSIZE/8);

    XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
    XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), "\r\nreadset=");
    pTmp += XOS_StrLen(pTmp); 
    
    /*打印读写集*/
    pRead = (char*)&g_ntlCb.tcpServTsk.setInfo.readSet.fdSet;
    for(i = 0; i< maxfdByte; i++)
    {
        XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), "%02x ", pRead[i]);
        pTmp += XOS_StrLen(pTmp);    
    }

    XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), "\r\nwriteset=");
    pTmp += XOS_StrLen(pTmp); 
    
    pWrite = (char*)&g_ntlCb.tcpServTsk.setInfo.writeSet.fdSet;
    for(i = 0; i< maxfdByte; i++)
    {
        XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), "%02x ", pWrite[i]);
        pTmp += XOS_StrLen(pTmp);    
    }    

    /*打印tcpserver sock fd数量*/
    XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), 
    "\r\n\r\ntcpserver socket fd count = %d\r\n\r\n", g_ntlCb.tcpServTsk.setInfo.sockNum);
    pTmp += XOS_StrLen(pTmp); 

    /*遍历服务端控制块*/
    XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), "tcpserver info:");
    pTmp += XOS_StrLen(pTmp);     
    for(i = 0; i<g_ntlCb.genCfg.maxTcpServLink; i++)
    {
        pTcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,i);

        if(pTcpServCb == XNULL)
        {
            continue;
        }

        if(pTcpServCb->sockFd.fd == XOS_INET_INV_SOCKFD)
        {
            continue;
        }

        XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), "fd=%d ", pTcpServCb->sockFd.fd);
        pTmp += XOS_StrLen(pTmp); 
    }
    
    /*遍历所有外部客户端*/
    XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), "\r\n\r\ntcpserver client info:");
    pTmp += XOS_StrLen(pTmp); 

    write_to_syslog("%s", szBuf);

    XOS_HashWalk(g_ntlCb.tSerCliH, NTL_tcpServerDataCheckHash, NULL);

    XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));

    /*打印文件描述符*/
    pid = getpid();
    memset(szBuf, 0, sizeof(szBuf));
    XOS_Sprintf(szCmd, sizeof(szCmd),"ls /proc/%d/fd", pid);
    XOS_ExeCmd(szCmd, szBuf, sizeof(szBuf)-1);

    write_to_syslog("\r\nprocess %d file des below: \r\n%s", pid, szBuf); 
#endif

}


/************************************************************************
函数名:NTL_tcpServTsk
功能:  tcp server listening function
输入:taskNo  任务号
输出:
返回:
说明:
************************************************************************/
XSTATIC XVOID NTL_tcpServTsk(XVOID* taskNo)
{
    t_FDSET  readSet;
    XS32 ret = 0;
    t_TSCB  *pTcpServCb = NULL;
    t_TSCLI    servCliCb;
    t_TSCLI *pServCliCb = NULL;
    t_CONNIND connectInd;
    XS16 setNum = 0;
    XS32 i = 0;
    t_IPADDR fromAddr;
    t_XINETFD servCliFd;
    XVOID *pLocation = NULL;
    XU32 pollAbideTime = 0;

    XOS_UNUSED(taskNo);
    pollAbideTime = POLL_FD_TIME_OUT;
    while(1)
    {
        /*线程刚起来时，并没有sockfd，fd为空时select直接返回*/
        g_ntlCb.tcpServTsk.activeFlag = XFALSE;
        XOS_SemGet(&(g_ntlCb.tcpServTsk.taskSemp));
        g_ntlCb.tcpServTsk.activeFlag = XTRUE;
        XOS_SemPut(&(g_ntlCb.tcpServTsk.taskSemp));

        /*拷贝到局部变量中，防止破坏全局的读集*/
        XOS_MemSet(&readSet,0,sizeof(t_FDSET));
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        XOS_MemCpy(&readSet,&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet),sizeof(t_FDSET));
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));

        /*select 监视*/
        setNum = 0;
        ret = XINET_Select(&readSet,(t_FDSET*) XNULLP,(XU32*)&pollAbideTime,(XS16*)&setNum);
        if(ret != XSUCC)
        {
            /*select 超时*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else /*其他错误*/
            {                
                XOS_Trace(MD(FID_NTL,PL_INFO),"tcpServer task,select readset failed,return setNum =%d.",setNum);
                #if 0
                /*当出现错误时，记录所有的状态信息*/
                NTL_tcpServerDataCheck();
                #endif
                continue;
            }
        }
        if(setNum > 0)
        {
            XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
            /*有客户端连接进来*/
            for(i = 0; i<g_ntlCb.genCfg.maxTcpServLink; i++)
            {
                pTcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,i);
                
                if(pTcpServCb == XNULL)
                {
                    continue;
                }

                if(pTcpServCb->sockFd.fd == XOS_INET_INV_SOCKFD)
                {
                    continue;
                }

                if((XOS_INET_FD_ISSET(&(pTcpServCb->sockFd),&(readSet))))
                {
                    setNum--;
                    XOS_MemSet(&fromAddr,0,sizeof(t_IPADDR));
                    /*先接收*/
                    ret = XINET_Accept(&(pTcpServCb->sockFd),&fromAddr,&servCliFd);
                    if(ret != XSUCC)
                    {
                        XOS_Trace(MD(FID_NTL,PL_WARN),"tcpServer task,accept fd %d failed!",pTcpServCb->sockFd.fd);
                        continue;
                    }

                    /*需要接入认证,但是认证不成功的*/
                    if(pTcpServCb->authFunc != XNULLP
                        && !(pTcpServCb->authFunc(pTcpServCb->userHandle,&fromAddr,pTcpServCb->pParam)))
                    {
                        /*关闭新接入的sock*/
                        XINET_CloseSock(&servCliFd);
                        XOS_Trace(MD(FID_NTL,PL_ERR),"tcpServer task,get client(ip[0x%x],port[%d]) auth failed!",
                            XOS_INET_NTOH_U32(fromAddr.ip),XOS_INET_NTOH_U16(fromAddr.port));
                        continue;
                    }

                    /*接入客户的数量不能超过最大容许接入数量*/
                    if(pTcpServCb->maxCliNum < pTcpServCb->usageNum +1)
                    {
                        /*关闭新接入的sock*/
                        XINET_CloseSock(&servCliFd);
                        XOS_Trace(MD(FID_NTL,PL_ERR),"tcpServer task,new accept client(ip[0x%x],port[%d]) is overstep maxclis %d allowned!",
                            XOS_INET_NTOH_U32(fromAddr.ip),XOS_INET_NTOH_U16(fromAddr.port),pTcpServCb->maxCliNum);
                        continue;
                    }
                    /*接入认证函数为空，意味着不需要认证*/
                    /*将接入的客户端加到hash表中*/
                    XOS_MemSet(&servCliCb,0,sizeof(t_TSCLI));
                    XOS_MemCpy(&(servCliCb.sockFd),&servCliFd,sizeof(t_XINETFD));
                    /*链表也应该保护*/
                    servCliCb.pServerElem = pTcpServCb;
                    servCliCb.pPreCli = pTcpServCb->pLatestCli;
                    servCliCb.pNextCli = (t_TSCLI*)XNULLP;

                    pLocation = XNULLP;
                    pLocation = XOS_HashElemAdd(g_ntlCb.tSerCliH,&fromAddr,(XVOID*)&servCliCb,XTRUE);
                    
                    if(pLocation == XNULLP)
                    {
                        XOS_Trace(MD(FID_NTL,PL_ERR),"tcpServer task,add new accept client to hash failed!");
                        XINET_CloseSock(&servCliFd);
                        continue;
                    }

                    /*构成链*/
                    pServCliCb = (t_TSCLI*)XNULLP;
                    pServCliCb = (t_TSCLI*)XOS_HashGetElem(g_ntlCb.tSerCliH, pLocation);
                    
                    if(pTcpServCb->pLatestCli != XNULLP && pServCliCb != XNULLP)
                    {
                        pTcpServCb->pLatestCli->pNextCli = pServCliCb;
                    }
                    pTcpServCb->pLatestCli= pServCliCb;
                    pTcpServCb->usageNum++; /*接入一个新的客户进来*/
                    
                    /*加入到read 集中*/
                    XOS_INET_FD_SET(&servCliFd,&( g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));
#ifndef XOS_WIN321
                    g_ntlCb.tcpServTsk.setInfo.sockNum++;
#endif

                    /*发送连接指示消息到上层*/
                    connectInd.appHandle = pTcpServCb->userHandle;
                    XOS_MemCpy(&(connectInd.peerAddr),&fromAddr,sizeof(t_IPADDR));
                    ret = NTL_msgToUser(&connectInd,&(pTcpServCb->linkUser),sizeof(t_CONNIND),eConnInd);
                    if(ret != XSUCC)
                    {
                        XOS_Trace(MD(FID_NTL,PL_WARN),"tcpServer task,indcate new client ip[0x%x],port[%d] connecttion to user failed!",
                            XOS_INET_NTOH_U32(fromAddr.ip),XOS_INET_NTOH_U16(fromAddr.port));
                        continue;
                    }
                }
            }

            /*有数据接收*/
            XOS_HashWalk(g_ntlCb.tSerCliH,NTL_pollingHash,&readSet);
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
       }
    }
}


/************************************************************************
函数名:NTL_genCfgProc
功能:  处理通用配置消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: ntl 在收到通用配置消息后启动各子任务，完成各
子任务的初始化。
************************************************************************/
XSTATIC XS32 NTL_genCfgProc(t_NTLGENCFG* pGenCfg)
{
    XU16 pollingNum = 0;
    //XOS_HARRAY  tempHandle;
    XCHAR taskName[NTL_TSK_NAME_LEN] = {0};
    XS32 i = 0;
    XS32 ret = 0;
    XU32 hashElems = 0;

    if(pGenCfg == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->bad input param!");
        return XERROR;
    }

    /*保存配置信息*/
    XOS_MemCpy(&(g_ntlCb.genCfg),pGenCfg,sizeof(t_NTLGENCFG));

    /*确定一个线程监视的描述符数量*/
    if(pGenCfg->fdsPerThrPolling > 0 )
    {
        pollingNum = pGenCfg->fdsPerThrPolling;
    }
    else
    {
        pollingNum = FDS_PER_THREAD_POLLING;  /*默认一个线程监视256 个描述符*/
        g_ntlCb.genCfg.fdsPerThrPolling = pollingNum;
    }

    /*确定各任务集的数量*/
    /*udp tasks */
    g_ntlCb.udpTskNo = (pGenCfg->maxUdpLink%pollingNum)
        ?(pGenCfg->maxUdpLink/pollingNum + 1)
        :(pGenCfg->maxUdpLink/pollingNum);

    /*tcp cli tasks */
    g_ntlCb.tcpCliTskNo = (pGenCfg->maxTcpCliLink%pollingNum)
        ?(pGenCfg->maxTcpCliLink/pollingNum + 1)
        :(pGenCfg->maxTcpCliLink/pollingNum);

    /*做一个安全性检查*/
    /*udp或tcpcli  配置的任务超过最大任务数,说明通用配置不合理*/
    if(g_ntlCb.udpTskNo > MAX_UDP_POLL_THREAD
        || g_ntlCb.tcpCliTskNo > MAX_TCP_CLI_POLL_THREAD)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_genCfgProc()->gen cfg is not reasonable!");
        return XERROR;
    }

    /*分配任务相关的资源*/
    if(g_ntlCb.udpTskNo > 0)
    {
        g_ntlCb.pUdpTsk =
            (t_NTLTSKINFO*)XOS_MemMalloc(FID_NTL,sizeof(t_NTLTSKINFO)*(g_ntlCb.udpTskNo));
        if(g_ntlCb.pUdpTsk)
        {
            memset((char*)g_ntlCb.pUdpTsk, 0, sizeof(t_NTLTSKINFO)*(g_ntlCb.udpTskNo));
        }
        else
        {
             XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->malloc tsk space failed!");
             goto genCfgError;
        }
    }
    
    if(g_ntlCb.tcpCliTskNo > 0)
    {
        g_ntlCb.pTcpCliTsk =
            (t_NTLTSKINFO*)XOS_MemMalloc(FID_NTL,sizeof(t_NTLTSKINFO)*(g_ntlCb.tcpCliTskNo));

        if(g_ntlCb.pTcpCliTsk)
        {
            memset((char*)g_ntlCb.pTcpCliTsk, 0, sizeof(t_NTLTSKINFO)*(g_ntlCb.tcpCliTskNo));    
        }
        else
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->malloc tsk space failed!");
            goto genCfgError;
        }
    }

    /*udp link 任务相关启动*/
    if (g_ntlCb.udpTskNo > 0)
    {
        /*分配udp链路控制块资源*/
        g_ntlCb.udpLinkH = XNULL;
        g_ntlCb.udpLinkH =  XOS_ArrayConstruct(sizeof(t_UDCB), pGenCfg->maxUdpLink, "udplinkH");
        if(XNULL == g_ntlCb.udpLinkH)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->construct udp array failed!");
            goto genCfgError;
        }
        XOS_ArraySetCompareFunc(g_ntlCb.udpLinkH, NTL_channel_find_function);

        /*udp链路控制块互斥变量*/
        ret = XOS_MutexCreate(&(g_ntlCb.udpLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> udp thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        } 

        /*启动任务集*/
        for (i = 0; i < g_ntlCb.udpTskNo; i++)
        {        

            /*任务的驱动信号量*/
            ret = XOS_SemCreate(&(g_ntlCb.pUdpTsk[i].taskSemp),  0);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> udp thread[%d]  semaphore  failed!",i);
                goto genCfgError;
            }           

            /*read set 互斥琐*/
            ret = XOS_MutexCreate(&(g_ntlCb.pUdpTsk[i].setInfo.fdSetMutex));
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> udp thread mutex[%d]  failed!",i);
                goto genCfgError;
            }

            /*初始化read set*/
            XOS_INET_FD_ZERO(&(g_ntlCb.pUdpTsk[i].setInfo.readSet.fdSet));
            XOS_INET_FD_ZERO(&(g_ntlCb.pUdpTsk[i].setInfo.writeSet.fdSet));

            /*初始化当前任务内的socket数量*/
            g_ntlCb.pUdpTsk[i].setInfo.sockNum = 0;

             /*创建udp任务*/
            XOS_MemSet(taskName,0,NTL_TSK_NAME_LEN);
            XOS_Sprintf(taskName, sizeof(taskName)-1, "Tsk_udp%d", i);
            ret = XOS_TaskCreate(taskName,TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)NTL_udpRcvFunc,
                (XVOID *)(XPOINT)i,&(g_ntlCb.pUdpTsk[i].taskId));
            if (ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> udp thread[%d]  failed!",i);
                goto genCfgError;
            }
        }
    }

    /* tcp client  link 任务相关启动*/
    if (g_ntlCb.tcpCliTskNo > 0 )
    {
        /*分配tcp client控制块资源*/
        g_ntlCb.tcpClientLinkH = XNULL;
        g_ntlCb.tcpClientLinkH =  XOS_ArrayConstruct(sizeof(t_TCCB), pGenCfg->maxTcpCliLink, "tcpClilinkH");
        if(XNULL == g_ntlCb.tcpClientLinkH)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->construct tcpCli array failed!");
            goto genCfgError;
        }
        XOS_ArraySetCompareFunc(g_ntlCb.tcpClientLinkH, NTL_channel_find_function);

        /*tcp客户端链路控制块互斥变量*/
        ret = XOS_MutexCreate(&(g_ntlCb.tcpClientLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcpclient thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        } 

        /*启动任务集*/
        for (i=0; i<g_ntlCb.tcpCliTskNo; i++)
        {
            /*任务的驱动信号量*/
            ret = XOS_SemCreate(&(g_ntlCb.pTcpCliTsk[i].taskSemp), 0);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcpCli thread[%d]  semaphore  failed!",i);
                goto genCfgError;
            }           

            /* fd read write set 互斥琐*/
            ret = XOS_MutexCreate(&(g_ntlCb.pTcpCliTsk[i].setInfo.fdSetMutex));
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcp cli  thread fdset mutex[%d]  failed!",i);
                goto genCfgError;
            }

            /*初始化 set*/
            XOS_INET_FD_ZERO(&(g_ntlCb.pTcpCliTsk[i].setInfo.readSet.fdSet));
            XOS_INET_FD_ZERO(&(g_ntlCb.pTcpCliTsk[i].setInfo.writeSet.fdSet));

             /*创建tcp client任务*/
            XOS_MemSet(taskName,0,NTL_TSK_NAME_LEN);
            XOS_Sprintf(taskName, sizeof(taskName)-1, "Tsk_tcpc%d", i);
            ret = XOS_TaskCreate(taskName,TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)NTL_tcpCliRcvFunc,
                (XVOID *)(XPOINT)i,&(g_ntlCb.pTcpCliTsk[i].taskId));
            if (ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcpCli thread[%d]  failed!",i);
                goto genCfgError;
            }

            /*初始化定时器*/
            /*to do */
        }
    }

    /*tcp server 相关的配置,tcp server目前采用单任务处理,以后可以扩展*/
    if(pGenCfg->maxTcpServLink > 0)
    {
        /*创建tcp server 控制块数组*/
        g_ntlCb.tcpServerLinkH = XNULL;
        g_ntlCb.tcpServerLinkH = XOS_ArrayConstruct( sizeof(t_TSCB),pGenCfg->maxTcpServLink,"tcpServH");
        if(XNULL == g_ntlCb.tcpServerLinkH )
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->construct tcpServ array failed!");
            goto genCfgError;
        }
 
        XOS_ArraySetCompareFunc(g_ntlCb.tcpServerLinkH, NTL_channel_find_function);

        /*tcp服务端链路控制块互斥变量*/
        ret = XOS_MutexCreate(&(g_ntlCb.tcpServerLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcpserver thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        }

        /*创建接入客户端的hash*/
        hashElems = (pGenCfg->maxTcpServLink)*TCP_CLIENTS_PER_SERV;
        g_ntlCb.tSerCliH = XOS_HashConstruct(hashElems, hashElems, sizeof(t_IPADDR),
            sizeof(t_TSCLI),"tcpServCliH");
        
        if(g_ntlCb.tSerCliH == XNULL)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->construct tcpServCli hash failed!");
            goto genCfgError;
        }

        /*set the hash func */
        ret = XOS_HashSetHashFunc(g_ntlCb.tSerCliH, NTL_tcliHashFunc);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->set  hashfunc failed!");
            goto genCfgError;
        }

        /*set the hash key compare func*/
        ret = XOS_HashSetKeyCompareFunc(g_ntlCb.tSerCliH, NTL_cmpIpAddr);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->set hash key compare failed!");
            goto genCfgError;
        }

        /*初始化hash 的互斥琐*/
        ret = XOS_MutexCreate(&(g_ntlCb.hashMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->create  mutex for hash failed!");
            goto genCfgError;
        }
        
        /*任务的驱动信号量*/
        ret = XOS_SemCreate(&(g_ntlCb.tcpServTsk.taskSemp),0);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tServ thread entry semaphore  failed!");
            goto genCfgError;
        }        

        /*read set 互斥琐*/
        ret = XOS_MutexCreate(&(g_ntlCb.tcpServTsk.setInfo.fdSetMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcp serv thread mutex failed!");
            goto genCfgError;
        }

        /*初始化 set*/
        XOS_INET_FD_ZERO(&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));
        XOS_INET_FD_ZERO(&(g_ntlCb.tcpServTsk.setInfo.writeSet.fdSet));

        /*创建tcp server 的任务*/
        XOS_Sprintf(taskName, sizeof(taskName)-1, "Tsk_tcps");
        ret = XOS_TaskCreate(taskName,TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)NTL_tcpServTsk,
            (XVOID *)0,&(g_ntlCb.tcpServTsk.taskId));
    }

    /*sctp pci 相关的初始化带扩展*/
    g_ntlCb.isGenCfg = XTRUE;
    return XSUCC;

genCfgError:
    /*释放资源*/
    XOS_ArrayDestruct(g_ntlCb.udpLinkH);
    XOS_ArrayDestruct(g_ntlCb.tcpClientLinkH);
    XOS_ArrayDestruct(g_ntlCb.tcpServerLinkH);
    XOS_HashDestruct(g_ntlCb.tSerCliH);

    /*中止任务释放琐*/
    for( i= 0; i< g_ntlCb.udpTskNo; i++)
    {
        XOS_TaskDel(g_ntlCb.pUdpTsk[i].taskId);
        XOS_SemDelete(&(g_ntlCb.pUdpTsk[i].taskSemp));
        XOS_MutexDelete(&(g_ntlCb.pUdpTsk[i].setInfo.fdSetMutex));
    }

    for(i=0; i<g_ntlCb.tcpCliTskNo; i++)
    {
        XOS_TaskDel(g_ntlCb.pTcpCliTsk[i].taskId);
        XOS_SemDelete(&(g_ntlCb.pTcpCliTsk[i].taskSemp));
        XOS_MutexDelete(&(g_ntlCb.pTcpCliTsk[i].setInfo.fdSetMutex));
    }

    XOS_TaskDel(g_ntlCb.tcpServTsk.taskId);
    XOS_MutexDelete(&(g_ntlCb.hashMutex));
    XOS_SemDelete(&(g_ntlCb.tcpServTsk.taskSemp));
    XOS_MutexDelete(&(g_ntlCb.tcpServTsk.setInfo.fdSetMutex));

    /*释放内存*/
    if(g_ntlCb.pTcpCliTsk != XNULLP)
    {
        XOS_MemFree(FID_NTL, g_ntlCb.pTcpCliTsk);
    }
    if(g_ntlCb.pUdpTsk != XNULLP)
    {
        XOS_MemFree(FID_NTL,g_ntlCb.pUdpTsk);
    }

    /*清空*/
    XOS_MemSet(&g_ntlCb,0,sizeof(t_NTLGLOAB));
    /*设置成没有初始化*/
    g_ntlCb.isGenCfg = XFALSE;
    return XERROR;
}


/************************************************************************
函数名:NTL_linkInitProc
功能:  处理链路初始化消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XSTATIC XS32 NTL_linkInitProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKINIT *pLinkInit = XNULL;
    t_UDCB *pUdpCb = XNULL;
    t_TCCB *pTcpCliCb = XNULL;
    t_TSCB *pTcpSrvCb = XNULL;
    t_LINKINITACK linkInitAck;
    t_Link_Index LinkIndex;
    XS32 linkIndex = -1;
    XS32 ret = 0;
    XS32 nRtnFind = XERROR;
    
    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkInitProc()->pMsg is null!");
        return XERROR;
    }
    XOS_MemSet(&linkInitAck,0,sizeof(t_LINKINITACK));
    pLinkInit = (t_LINKINIT *)(pMsg->message);
    if(!pLinkInit)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkInitProc()->pLinkInit is null!");
        return XERROR;
    }
    
    LinkIndex.linkUser = (t_XOSUSERID*)&pMsg->datasrc;
    LinkIndex.userHandle =  pLinkInit->appHandle;
    LinkIndex.linkType = pLinkInit->linkType;

    XOS_Trace(MD(FID_NTL,PL_DBG),"NTL_linkInitProc()->linkType:%d,userHandle:%x!",pLinkInit->linkType,LinkIndex.userHandle);

    switch (pLinkInit->linkType)
    {
    case eUDP:
        pUdpCb =(t_UDCB*)XNULLP;
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        nRtnFind = XOS_ArrayFind(g_ntlCb.udpLinkH,&LinkIndex);
        
        /*查找是否重复申请*/
        if(XERROR != nRtnFind)
        {
            /*重置链路*/
            NTL_ResetLinkByReapplyEntry(eUDP, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkInitProc()->add the udp cb was reset!");
        }
        else 
        {
            /*添加到忙闲链中*/
            linkIndex = XOS_ArrayAddExt(g_ntlCb.udpLinkH,(XOS_ArrayElement *) &pUdpCb);
            
            if((linkIndex >= 0) && (pUdpCb != XNULLP))
            {
                /*初始化控制块参数*/
                pUdpCb->userHandle = pLinkInit->appHandle;
                XOS_MemCpy(&(pUdpCb->linkUser),&(pMsg->datasrc),sizeof(t_XOSUSERID));
                pUdpCb->linkHandle = NTL_buildLinkH(eUDP,(XU16)linkIndex);
                pUdpCb->linkState = eStateInited;
                pUdpCb->myAddr.ip = 0;
                pUdpCb->myAddr.port = 0;
                pUdpCb->peerAddr.ip = 0;
                pUdpCb->peerAddr.port = 0;
                pUdpCb->sockFd.fd = XOS_INET_INV_SOCKFD;                

                /*回复eLinkInitAck的参数*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle = pUdpCb->linkHandle;
            }
            else
            {
                /*回复链路确认失败*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkInitProc()->add the udp cb to array failed!");
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eFAIL;
            }
        }
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
        break;

    case eTCPClient:
        
        pTcpCliCb = (t_TCCB*)XNULLP;
        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));
        nRtnFind = XOS_ArrayFind(g_ntlCb.tcpClientLinkH, &LinkIndex);
        
        
        if(XERROR != nRtnFind)
        {
            /*重置链路*/
            NTL_ResetLinkByReapplyEntry(eTCPClient, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkInitProc()->add the tcp client cb was reset!");
            
        }
        else
        {
            /*添加到忙闲链中*/
            linkIndex = XOS_ArrayAddExt(g_ntlCb.tcpClientLinkH,(XOS_ArrayElement *) &pTcpCliCb);
            
            if((linkIndex >= 0) && (pTcpCliCb != XNULLP))
            {
                /*初始化控制块参数*/
                pTcpCliCb->userHandle = pLinkInit->appHandle;
                XOS_MemCpy(&(pTcpCliCb->linkUser),&(pMsg->datasrc),sizeof(t_XOSUSERID));
                pTcpCliCb->linkHandle = NTL_buildLinkH(eTCPClient,(XU16)linkIndex);
                pTcpCliCb->linkState = eStateInited;
                pTcpCliCb->myAddr.ip = 0;
                pTcpCliCb->myAddr.port = 0;
                pTcpCliCb->peerAddr.ip = 0;
                pTcpCliCb->peerAddr.port = 0;
                pTcpCliCb->sockFd.fd = XOS_INET_INV_SOCKFD;    
                memset((char*)&(pTcpCliCb->packetlist), 0, sizeof(t_ResndPacket));
                XOS_INIT_THDLE(pTcpCliCb->timerId);
                pTcpCliCb->expireTimes = 0;
                

                /*回复eLinkInitAck的参数*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle= pTcpCliCb->linkHandle;
            }
            else
            {
                /*回复链路确认失败*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkInitProc()->add the tcp client cb to array failed!");
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eFAIL;
            }
        }
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
        break;

    case eTCPServer:
        pTcpSrvCb = (t_TSCB*)XNULLP;
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        nRtnFind = XOS_ArrayFind(g_ntlCb.tcpServerLinkH, &LinkIndex);
        
        if(XERROR != nRtnFind)
        {
            /*重置链路*/
            NTL_ResetLinkByReapplyEntry(eTCPServer, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkInitProc()->add the tcp server cb was reset!");
        }
        else
        {
            /*添加到忙闲链中*/
            linkIndex = XOS_ArrayAddExt(g_ntlCb.tcpServerLinkH,(XOS_ArrayElement *)&pTcpSrvCb);
            
            if((linkIndex >= 0) && (pTcpSrvCb != XNULLP))
            {
                /*填写控制块参数*/
                pTcpSrvCb->userHandle = pLinkInit->appHandle;
                XOS_MemCpy(&(pTcpSrvCb->linkUser),&(pMsg->datasrc),sizeof(t_XOSUSERID));
                pTcpSrvCb->linkHandle = NTL_buildLinkH(eTCPServer,(XU16)linkIndex);
                pTcpSrvCb->linkState = eStateInited;
                pTcpSrvCb->myAddr.ip = 0;
                pTcpSrvCb->myAddr.port = 0;
                pTcpSrvCb->sockFd.fd = XOS_INET_INV_SOCKFD;    
                pTcpSrvCb->authFunc = NULL;
                pTcpSrvCb->pParam = NULL;
                pTcpSrvCb->maxCliNum = 0;
                pTcpSrvCb->usageNum = 0;
                pTcpSrvCb->pLatestCli = NULL;

                /*回复eLinkInitAck的参数*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle= pTcpSrvCb->linkHandle;
            }
            else
            {
                /*回复链路确认失败*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkInitProc()->add the tcp server cb to array failed!");
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eFAIL;
            }
        }
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
        break;
#ifdef XOS_SCTP
    case eSCTPClient:
    case eSCTPServer:
        SCTP_linkInitProc(pMsg);
        return XSUCC;
#endif
    default:
        /*回复链路确认失败*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkInitProc()->not support link type:%d!",pLinkInit->linkType);
        linkInitAck.appHandle = pLinkInit->appHandle;
        linkInitAck.lnitAckResult = eFAIL;
        break;
    }

    /*回复initAck 消息*/
    ret = NTL_msgToUser(&linkInitAck,&(pMsg->datasrc),sizeof(t_LINKINITACK),eInitAck);

    
    /*回复消息失败，该元素将不再使用，应该清除*/
    if((ret != XSUCC) && (linkInitAck.lnitAckResult == eSUCC))
    {
        /*清空资源*/
        NTL_DeleteCB(pLinkInit->linkType, linkIndex);        
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
函数名:NTL_DeleteCB
功能:  释放控制块
输入:  linkType －链路类型
       linkIndex--链路索引

输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_DeleteCB(e_LINKTYPE linkType, int linkIndex)
{
    XS32 nRst = 0;
    switch (linkType)
    {
        case eUDP:
            XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
            nRst = XOS_ArrayDeleteByPos(g_ntlCb.udpLinkH, linkIndex);
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            break;

        case eTCPClient:
            XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));
            nRst = XOS_ArrayDeleteByPos(g_ntlCb.tcpClientLinkH, linkIndex);
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            break;

        case eTCPServer:
            XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
            nRst = XOS_ArrayDeleteByPos(g_ntlCb.tcpServerLinkH, linkIndex);
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            break;
            
        default:
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_DeleteCB()->unknown link type coming!");
            break;
    }

    if(XSUCC != nRst)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_DeleteCB()->delete failed");
    }
    return nRst;
}

/************************************************************************
函数名:NTL_linkReleaseProc
功能:  处理链路释放消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XSTATIC XS32 NTL_linkReleaseProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKRELEASE *pLinkRelease = NULL;
    t_TSCB *pTcpServCb = NULL;
    e_LINKTYPE linkType;
    XS32 linkIndex = 0;
    XS16 ret = 0;

    if(NULL == pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkReleaseProc()->pMsg is null!");
        return XERROR;
    }
    
    /*先关闭链路,再释放资源*/
    ret = NTL_ReleaseLink(pMsg);
    
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkReleaseProc()->close link error!");
        return XERROR;
    }
    pLinkRelease = (t_LINKRELEASE*)(pMsg->message);
    
    if(NULL == pLinkRelease)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkReleaseProc()->pLinkRelease is null!");
        return XERROR;
    }

    if(!NTL_isValidLinkH(pLinkRelease->linkHandle))
    {
        /*链路句柄无效的消息，直接丢弃*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkReleaseProc()->linkHandle invalid!");
        return XERROR;
    }
    /*获取链路类型*/
    linkType = NTL_getLinkType(pLinkRelease->linkHandle);
    linkIndex = NTL_getLinkIndex(pLinkRelease->linkHandle);
    XOS_Trace(MD(FID_NTL,PL_DBG),"NTL_linkReleaseProc()->linkType:%d,linkHandle:%x!",linkType,pLinkRelease->linkHandle);
    switch (linkType)
    {
    case eUDP:
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        XOS_ArrayDeleteByPos(g_ntlCb.udpLinkH,linkIndex);
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
        break;

    case eTCPClient:
        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));        
        XOS_ArrayDeleteByPos(g_ntlCb.tcpClientLinkH,linkIndex);
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
        break;

    case eTCPServer:
        pTcpServCb = (t_TSCB*)XNULLP;
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        pTcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,linkIndex);
        
        if(pTcpServCb != XNULLP)
        {
            /*先删除所有接入的客户*/
            if( pTcpServCb->pLatestCli != XNULLP)
            {
                while(pTcpServCb->pLatestCli != XNULLP)
                {
                    NTL_closeTsCli(pTcpServCb->pLatestCli);
                    if(pTcpServCb->usageNum == 0)
                    {
                        break;
                    }
                }
            }
            if( XOS_INET_INV_SOCKFD != pTcpServCb->sockFd.fd)
            {
                   ret = NTL_CloseTcpServerSocket(pTcpServCb);
                if( ret != XSUCC)
                {
                    g_tcpSerReleaseFail++;
                }
            }

            /*再清除server对应的控制块*/
            XOS_ArrayDeleteByPos(g_ntlCb.tcpServerLinkH,linkIndex);
        }
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
        
        break;
#ifdef XOS_SCTP
    case eSCTPClient:
    case eSCTPServer:
        SCTP_linkReleaseProc(pMsg);
        break;
#endif
    case ePCI:
        break;

    default:
        break;
    }

    return XSUCC;
}


/************************************************************************
函数名:NTL_tcpServStart
功能:  处理链路启动消息
输入:pMsg －消息指针
输出:pStartAck －启动确认消息
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XSTATIC XS32 NTL_tcpServStart(t_XOSCOMMHEAD* pMsg,t_STARTACK* pStartAck)
{
    t_TCPSERSTART *pTcpServStart = NULL;
    t_LINKSTART *pLinkStart = NULL;
    t_TSCB *pTcpServCb = NULL;
    XCHAR szLinkStateName[32] = {0};
    XS32 ret = 0;

    if(!pMsg || !pStartAck)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->pMsg is null!");
        return XERROR;
    }

    pLinkStart = (t_LINKSTART*)(pMsg->message);
    if(!pLinkStart)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->pLinkStart is null!");
        return XERROR;
    }
    
    pTcpServStart = &(pLinkStart->linkStart.tcpServerStart);

    /*获取tcp server 控制块*/
    pTcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
    
    if(pTcpServCb == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->get tcp serv control block failed!");
        return XERROR;
    }

    /*检查链路状态和相关参数*/
    if((pTcpServCb->linkState != eStateInited )
        ||(pTcpServStart->allownClients == 0)
        ||(pTcpServStart->allownClients > (XU32)((g_ntlCb.genCfg.maxTcpServLink)*TCP_CLIENTS_PER_SERV)))
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->link state [%s]  error or allownClients[%d] error!",NTL_getLinkStateName(pTcpServCb->linkState,szLinkStateName,sizeof(szLinkStateName)-1),pTcpServStart->allownClients);
        goto errorProc;
    }

    /*如果状态正确，则可能是上次忘记关闭了*/
    NTL_CloseTcpServerSocket(pTcpServCb);    
    
    /*启动sockect*/
    ret = XINET_Socket(XOS_INET_STREAM,&(pTcpServCb->sockFd));
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->create sockFd failed");
        goto errorProc;
    }

    /*绑定端口*/
    ret =  XINET_Bind(&(pTcpServCb->sockFd),&(pTcpServStart->myAddr));
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->XINET Bind sockFd failed");
        goto errorProc;
    }
    /*确定本端地址*/
    XINET_GetSockName(&(pTcpServCb->sockFd),&(pTcpServCb->myAddr));

    /*listen*/
    ret = XINET_Listen(&(pTcpServCb->sockFd),MAX_BACK_LOG);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->XINET_Listen sockFd %d failed",pTcpServCb->sockFd.fd);
        goto errorProc;
    }

    //printf("NTL_tcpServStart: Listen ok srcFid=%d, port=%d\n", pMsg->datasrc.FID, pTcpServStart->myAddr.port );

    /*添加到read 集中*/
#ifdef XOS_WIN321
    if(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet.fd_count == 0)
#else
    if(g_ntlCb.tcpServTsk.setInfo.sockNum++ == 0)
#endif
    {
        XOS_INET_FD_SET(&(pTcpServCb->sockFd),&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));
        XOS_SemPut(&(g_ntlCb.tcpServTsk.taskSemp));
    }
    else
    {
        XOS_INET_FD_SET(&(pTcpServCb->sockFd),&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));
    }

    /*填写参数*/
    pTcpServCb->authFunc = pTcpServStart->authenFunc;
    pTcpServCb->pParam = pTcpServStart->pParam;
    pTcpServCb->maxCliNum = pTcpServStart->allownClients;
    pTcpServCb->usageNum = 0;
    pTcpServCb->linkState = eStateListening;
    pTcpServCb->pLatestCli = (t_TSCLI*)XNULLP;

    /*填写输出参数*/
    pStartAck->appHandle = pTcpServCb->userHandle;
    pStartAck->linkStartResult = eSUCC;
    XOS_MemCpy(&(pStartAck->localAddr),&(pTcpServCb->myAddr),sizeof(t_IPADDR));
    
    return XSUCC;

errorProc:
    {
        /*填写startAck 回复消息,返回结果为efailed*/
        pStartAck->appHandle = pTcpServCb->userHandle;
        pStartAck->linkStartResult = eFAIL;
        /*删除申请的控制块,2008/03/21 add below*/
        //XOS_ArrayDeleteByPos(g_ntlCb.tcpServerLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
        /*删除申请的控制块,2008/03/21 add above*/
        return XSUCC;
    }
}
/************************************************************************
函数名:NTL_StartTcpClientTimer
功能:  启动tcp客户端的重启定时器
输入:  pTcpCb －tcp控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_StartTcpClientTimer(t_TCCB *pTcpCb, XU32 taskNo)
{
    t_PARA timerPara;
    t_BACKPARA backPara;
    XS32 ret = 0;

    if(!pTcpCb)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartTcpClientTimer()->pTcpCb is null");
        return XERROR;
    }
    
    XOS_MemSet(&timerPara,0,sizeof(t_PARA));
    XOS_MemSet(&backPara,0,sizeof(t_BACKPARA));
    
    timerPara.fid  = FID_NTL;
    timerPara.len  = TCP_CLI_RECONNECT_INTERVAL;
    timerPara.mode = TIMER_TYPE_LOOP;
    timerPara.pre  = TIMER_PRE_LOW;
    backPara.para1 = (XPOINT)pTcpCb;
    backPara.para2 = (XPOINT)taskNo;
    XOS_INIT_THDLE (pTcpCb->timerId);    

    pTcpCb->expireTimes = 0;
    
    ret =  XOS_TimerStart(&(pTcpCb->timerId), &timerPara, &backPara);

    //XOS_Trace(MD(FID_NTL,PL_ERR),"%d start timer %d", pTcpCb->userHandle, pTcpCb->timerId);

    return ret;
    

}

          
/************************************************************************
函数名:NTL_SetTcpClientFd
功能:  tcp client select 置位操作
输入:  pTcpCb －消息指针
       fdFlg  --0:读,1:写, 2:读和写
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/            
void NTL_SetTcpClientFd(t_TCCB *pTcpCb, XU32 taskNo, e_ADDSETFLAG fdFlg)
{
    if(!pTcpCb)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_SetTcpClientFd()->pTcpCb is null");
        return;
    }
    
    switch(fdFlg)
    {
    case eRead:
        XOS_INET_FD_SET(&(pTcpCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));
        break;
        
    case eWrite:
        XOS_INET_FD_SET(&(pTcpCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));
        break;
        
    case eReadWrite:
        XOS_INET_FD_SET(&(pTcpCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));
        XOS_INET_FD_SET(&(pTcpCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));            
        break;
        
    default:
        break;
    }

#ifdef XOS_WIN321
    if (g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet.fd_count == 0
        && (g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet.fd_count == 0) )
#else
    if(g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum++ == 0)
#endif
    {
        XOS_SemPut(&(g_ntlCb.pTcpCliTsk[taskNo].taskSemp));
    }
    
}

/************************************************************************
函数名:NTL_SetUdpSelectFd
功能:  udp select 置位操作
输入:  pUdpCb －udp控制块
       taskNo  --任务号
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/    
void NTL_SetUdpSelectFd(t_UDCB *pUdpCb, XU32 taskNo)
{
    if(!pUdpCb)
    {
        return;
    }
    
    /*添加到readset中*/
#ifdef XOS_WIN321
    if (g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet.fd_count == 0)
    {
        XOS_INET_FD_SET(&(pUdpCb->sockFd),&(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet));
        /*第一次增加udp任务时，发送udp任务驱动信号*/
        XOS_SemPut(&(g_ntlCb.pUdpTsk[taskNo].taskSemp));
    }
    else
    {
        XOS_INET_FD_SET(&(pUdpCb->sockFd),&(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet));
    }
#else
    if(g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum++ == 0)
    {
        XOS_INET_FD_SET(&(pUdpCb->sockFd),&(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet));
        /*第一次增加udp任务时，发送udp任务驱动信号*/
        XOS_SemPut(&(g_ntlCb.pUdpTsk[taskNo].taskSemp));
    }
    else
    {
        XOS_INET_FD_SET(&(pUdpCb->sockFd),&(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet));
    }
#endif
    return;
}

/************************************************************************
函数名:NTL_StartUdpLink
功能:  启动udp链路
输入:    pDatasrc －消息源指针
        pLinkStart--链路启动指针

输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_StartUdpLink(t_XOSUSERID *pDatasrc, t_LINKSTART *pLinkStart)
{
    t_UDPSTART *pUdpStart = XNULL;
    t_UDCB *pUdpCb = XNULL;
    t_STARTACK startAck;
    XS32 ret = 0;
    XU32 taskNo = 0;
    XCHAR szLinkStateName[32] = {0};    

    if(!pLinkStart || !pDatasrc)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->pLinkStart invalid!");
        return XERROR;
    }

    XOS_MemSet(&startAck,0,sizeof(t_STARTACK));

    /*udp 一定需要立即回复startack 消息*/
    pUdpStart = &(pLinkStart->linkStart.udpStart);
    
    XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
    do{
        /*获取udp 控制块*/
        pUdpCb = (t_UDCB*)XNULLP;
        
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH, NTL_getLinkIndex(pLinkStart->linkHandle));
        
        if(pUdpCb == XNULLP)
        {
           /*这种错误很严重，没有办法回复startAck 消息，所以将消息丢弃*/
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] get the udp control block failed!", pDatasrc->FID);
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
           return XERROR;
        }

        /*获取链路所在的任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pLinkStart->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.udpTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] taskNo %d is invalid", pUdpCb->linkUser.FID, taskNo);
            startAck.appHandle = pUdpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            break;
        }

        /*检查链路的状态，看是否为重复启动*/
        if(pUdpCb->linkState != eStateInited)
        {
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] the udp link state [%s] is wrong!",pUdpCb->linkUser.FID,
                                         NTL_getLinkStateName(pUdpCb->linkState, szLinkStateName, sizeof(szLinkStateName)-1));
           /*填写startAck 回复消息,返回结果为efailed*/
           startAck.appHandle = pUdpCb->userHandle;
           startAck.linkStartResult = eFAIL;
           break;
        }

        /*如果fd正确，则关闭，可能是上次忘记关闭了*/
        if (pUdpCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
           /*关闭socket，清除fdset*/
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] pUdpCb->sockFd.fd %d forget close", pUdpCb->linkUser.FID,
                                        pUdpCb->sockFd.fd);
           NTL_CloseUdpSocket(pUdpCb, taskNo);
        }   

        /*启动所有sock*/
        ret = XINET_Socket(XOS_INET_DGRAM, &(pUdpCb->sockFd));
        if (ret != XSUCC)
        {
           /*填写startAck 回复消息,返回结果为failed*/
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] XINET_Socket failed", pUdpCb->linkUser.FID);
           startAck.appHandle = pUdpCb->userHandle;
           startAck.linkStartResult = eFAIL;
           break;
        }

        ret = XINET_Bind(&(pUdpCb->sockFd), &(pUdpStart->myAddr));
        if (ret != XSUCC)/*绑定不成功,可能是端口已经使用了*/
        {
           /*填写startAck 回复消息,返回结果为efailed*/
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] XINET_Bind failed", pUdpCb->linkUser.FID);
           startAck.appHandle = pUdpCb->userHandle;
           startAck.linkStartResult = eFAIL;
           break;
        }

        /*修改udp控制块信息*/
        pUdpCb->linkState = eStateStarted;
        XINET_GetSockName(&(pUdpCb->sockFd),&(pUdpCb->myAddr));
        XOS_MemCpy(&(pUdpCb->peerAddr), &(pUdpStart->peerAddr), sizeof(t_IPADDR));      

        /*加入到read set*/
        NTL_SetUdpSelectFd(pUdpCb, taskNo);

        /*填写startAck 回复消息,返回结果为efailed*/
        startAck.appHandle = pUdpCb->userHandle;
        XOS_MemCpy(&(startAck.localAddr),&(pUdpCb->myAddr),sizeof(t_IPADDR));
        startAck.linkStartResult = eSUCC;
        break;
        
    }while(0);
    XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));


    /*发送startAck 消息到上层*/
    ret = NTL_msgToUser(&startAck, pDatasrc, sizeof(t_STARTACK), eStartAck);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_StartUdpLink()->send  msg startAck to user failed!");
        return XERROR;
    }

    return XSUCC;

}


/************************************************************************
函数名:NTL_linkStarttProc
功能:  处理链路启动消息
输入:pMsg －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XSTATIC XS32 NTL_linkStartProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKSTART *pLinkStart = XNULL;
    t_TCPCLISTART *pTcpCliStart = XNULL;
    t_TCCB *pTcpCb = XNULL;
    e_LINKTYPE linkType;
    t_STARTACK startAck;
    XU32 optVal = 0;
    XS32 ret = 0;
    XU32 taskNo = 0;
    XCHAR szTemp[32] = {0};
    XCHAR szLinkStateName[32] = {0};

    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStarttProc()->pMsg invalid!");
        return XERROR;
    }
    pLinkStart = (t_LINKSTART*)(pMsg->message);
    if(!pLinkStart)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStarttProc()->pLinkStart invalid!");
        return XERROR;
    }

    /*所有的下行消息(elinkInit 除外)都要验证链路句柄的有效性，
    以防止误操作修改到其他的数据*/
    if(!NTL_isValidLinkH(pLinkStart->linkHandle))
    {
        /*链路句柄无效的消息，直接丢弃*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStarttProc()->FID[%d] linkHandle[%d] invalid!", pMsg->datasrc.FID,
                                     pLinkStart->linkHandle);
        return XERROR;
    }

    /*获取链路类型*/
    linkType = NTL_getLinkType(pLinkStart->linkHandle);
    XOS_MemSet(&startAck,0,sizeof(t_STARTACK));
    XOS_Trace(MD(FID_NTL,PL_DBG),"NTL_linkStarttProc()->linkType:%d,linkHandle:%x!",linkType,pLinkStart->linkHandle);
    switch (linkType)
    {
    case eUDP:

        return NTL_StartUdpLink(&(pMsg->datasrc), pLinkStart);       

    case eTCPClient:
        pTcpCliStart = &(pLinkStart->linkStart.tcpClientStart);

        /*获取tcp client  控制块*/
        pTcpCb = (t_TCCB*)XNULLP;
        
        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));
        pTcpCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
        
        
        if(pTcpCb == XNULLP)
        {
            /*这种错误很严重，没有办法回复startAck 消息，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->get the tcp client control block failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            return XERROR;
        }

         /*获取链路所在任务号*/
         taskNo = (XU32)NTL_getLinkIndex(pLinkStart->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
         if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
         {
              XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client taskNo %d is invalid!",pTcpCb->linkUser.FID, taskNo);
             startAck.appHandle = pTcpCb->userHandle;
             startAck.linkStartResult = eFAIL;
             XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
             break;
         }                 

         /*检查链路的状态，看是否为重复启动*/
         if(pTcpCb->linkState != eStateInited)
         {
             XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] the tcp client link state [%s] is wrong!",
                                           pTcpCb->linkUser.FID, NTL_getLinkStateName(pTcpCb->linkState, szLinkStateName, sizeof(szLinkStateName)-1));
             /*填写startAck 回复消息,返回结果为efailed*/
             startAck.appHandle = pTcpCb->userHandle;
             startAck.linkStartResult = eFAIL;
             XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
             break;
         }


           /*状态正确，fd却打开，须要关闭，可能是上次忘记关闭了*/
        if (pTcpCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client forget close old fd!",pTcpCb->linkUser.FID);
            NTL_closeTCli(taskNo, pTcpCb, NTL_SHTDWN_SEND);
        }     
        

        /*启动所有sock*/
        ret = XINET_Socket(XOS_INET_STREAM,&(pTcpCb->sockFd));
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client socket failed!",pTcpCb->linkUser.FID);
            /*填写startAck 回复消息,返回结果为efailed*/
            startAck.appHandle = pTcpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            break;
        }


        /*根据业务设置linger*/
          if (pTcpCb->linkUser.FID != FID_FTP)
        {
            optVal = XOS_INET_OPT_ENABLE;
            XINET_SetOpt(&pTcpCb->sockFd, SOL_SOCKET, XOS_INET_OPT_LINGER, (XU32 *)&optVal);            
        }
        
        /*对于tcp 客户端的服务，不care 绑定的结果*/
        ret = XINET_Bind(&(pTcpCb->sockFd),&(pTcpCliStart->myAddr));
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client bind failed!",pTcpCb->linkUser.FID);
            startAck.appHandle = pTcpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            break;
        }

        XINET_GetSockName(&(pTcpCb->sockFd),&(pTcpCb->myAddr));
        XOS_MemCpy(&(pTcpCb->peerAddr),&(pTcpCliStart->peerAddr),sizeof(t_IPADDR));        

        /*连接到对端*/
        ret = XINET_Connect(&(pTcpCb->sockFd),&(pTcpCliStart->peerAddr));
        if(ret != XSUCC)
        {

            pTcpCb->linkState = eStateConnecting;
            XOS_IptoStr(pTcpCliStart->peerAddr.ip,szTemp);
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkStartProc(),FID[%d] tcp client the %dth connect[%s:%d] failed ,start timer for pTcpCb[0x%x] reconnect!",
                pTcpCb->linkUser.FID,pTcpCb->expireTimes ,szTemp,pTcpCliStart->peerAddr.port,pTcpCb);

            /*如果连接不成功,尝试重连,启动定时器,保证链路连接*/            
            if(XSUCC != NTL_StartTcpClientTimer(pTcpCb, taskNo))
            {
                /*定时器启动失败处理，待定*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client start timer failed!",pTcpCb->linkUser.FID);
                startAck.appHandle = pTcpCb->userHandle;
                startAck.linkStartResult = eFAIL;
                XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
                break;
            }

            /*添加到writeset中*/
            NTL_SetTcpClientFd(pTcpCb, taskNo, eWrite);
            
            startAck.appHandle = pTcpCb->userHandle;
            XOS_MemCpy(&(startAck.localAddr),&(pTcpCb->myAddr),sizeof(t_IPADDR));
            startAck.linkStartResult = eBlockWait;     
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));       
            break;
        }

        /*非阻塞连接成功的可能性很小，要是连接本机的端口，一般是可以连接成功的*/
        pTcpCb->linkState = eStateConnected;
        NTL_StopTcpClientTimer(pTcpCb);

        /*添加到readset中*/
        NTL_SetTcpClientFd(pTcpCb, taskNo, eRead);

        /*填写startAck 回复消息,返回结果为成功*/
        startAck.appHandle = pTcpCb->userHandle;
        XOS_MemCpy(&(startAck.localAddr), &(pTcpCb->myAddr), sizeof(t_IPADDR));
        startAck.linkStartResult = eSUCC;
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));

        break;

    case eTCPServer:
        /*tcp server 的启动比较复杂，单独一个函数处理*/
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        ret = NTL_tcpServStart( pMsg,&startAck);
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
        if(ret == XERROR)
        {
            /*返回错误就不用回复start Ack 消息*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_linkStartProc()->start tcp server  failed!");
            return XERROR;
        }
        break;

#ifdef XOS_SCTP
    case eSCTPClient:
    case eSCTPServer:
        SCTP_linkStartProc(pMsg);
        return XSUCC;
#endif

    default:
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_linkStartProc()->not support type:%d!",linkType);
        return XERROR;
    }

    /*发送startAck 消息到上层*/
    ret = NTL_msgToUser(&startAck,&(pMsg->datasrc),sizeof(t_STARTACK),eStartAck);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_linkStartProc()->send  msg startAck to user failed!");
        return XERROR;
    }

    return XSUCC;
}


/*
已排队队列不清除,直到发送成功为止
*/
XS32 NTL_dataReqClear(t_ResndPacket *pPacklist)
{
    t_RsndData *ploopPacket = XNULL;
    t_RsndData *pdelPacket = XNULL;

    if(pPacklist == XNULL)
    {
        return (XERROR);
    }
    ploopPacket = pPacklist->pFirstDataReq;

    if(g_ntltraceswitch)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_dataReq Clear begin size = %d",pPacklist->rsnd_size);
    }
    while(ploopPacket!= XNULLP)
    {
        pdelPacket= ploopPacket;
        ploopPacket = ploopPacket->pNextPacket;
        if(XNULL != pdelPacket->pData)
        {
            XOS_MemFree(FID_NTL,(XVOID*)(pdelPacket->pData));
        }
        pdelPacket->msgLenth=0;
        pdelPacket->i_offset=0;
        XOS_MemFree((XU32)FID_NTL,pdelPacket);
        pPacklist->rsnd_size--;
        pPacklist->rsnd_delete++;
        pPacklist->pFirstDataReq=ploopPacket;
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_dataReq Clear del one size = %d",pPacklist->rsnd_size);
        }
    }
    if(g_ntltraceswitch)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_dataReq Clear end size = %d",pPacklist->rsnd_size);
    }
    pPacklist->rsnd_size=0;/*8888*/
    pPacklist->rsnd_total=0;
    pPacklist->rsnd_wait=0;
    pPacklist->rsnd_success=0;
    pPacklist->rsnd_delete=0;
    pPacklist->rsnd_fail=0;
    pPacklist->pFirstDataReq = XNULL;
    return XSUCC;
}


/*
该接口只存贮新数据包，半包或全包
*/
#ifdef XOS_NEED_CHK
XS32 NTL_dataReqSaveProc(t_ResndPacket *pPacklist,t_DATAREQ *pDataReq,XS32 out_len)
{
    t_RsndData *ptmpPacket = NULL;
    t_RsndData *ploopPacket = XNULL;
    if(XNULL == pDataReq)
    {
        return XERROR;
    }
    if((XNULL == (pDataReq->pData)) || (pDataReq->msgLenth ==0))
    {
        return XERROR;
    }
    /*out_len数据包介于*[0,msglen)之间
    0表示没发送成功
    msglen表示发送成功
    */
    if(!(out_len>=0 && out_len < (XS32)pDataReq->msgLenth))
    {
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_dataReq SaveProc,msglen %d,offset %d",pDataReq->msgLenth,out_len);
        }
        return XERROR;
    }

    if(pPacklist == XNULLP)
    {
        return XERROR;
    }
    if(pPacklist->rsnd_size >= g_ntlpoolsize)
    {
        /*如果排队队列满,将后续消息包丢掉*/
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_DataReqResndProc()-> resend size %d,queue is full!",pPacklist->rsnd_size);
        }
        return XERROR;
    }
    if (XNULL ==  (ptmpPacket = (t_RsndData *)XOS_MemMalloc((XU32)FID_NTL,sizeof(t_RsndData))))
    {
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_DataReqResndProc() XOS_MemMalloc failed!");
        }
        return XERROR;
    }
    /*将新的数据包加到链表中*/
    XOS_MemSet(ptmpPacket,0x0,sizeof(t_RsndData));
    ptmpPacket->pNextPacket=XNULL;
    /*将数据包地址拷贝到链表中*/
    ptmpPacket->pData=pDataReq->pData;
    ptmpPacket->msgLenth = pDataReq->msgLenth;
    ptmpPacket->i_offset = out_len;

    /*链表也应该保护*/

    /*空链表*/
    /*初始化时需要置全0x0,在创建时补充实现8888*/
    pPacklist->rsnd_wait++;
    if(pPacklist->pFirstDataReq == XNULL)
    {
        /*作为头节点*/
        pPacklist->pFirstDataReq=ptmpPacket;
        pPacklist->rsnd_size++;
        return XSUCC;
    }

    ploopPacket = pPacklist->pFirstDataReq;
    while(ploopPacket ->pNextPacket != XNULL)
    {
        ploopPacket = ploopPacket->pNextPacket;
    }
    ploopPacket ->pNextPacket =ptmpPacket;
    pPacklist->rsnd_size++;
    return XSUCC;
}


#endif
/*只管排队队列发送*/
XSTATIC XS32 NTL_dataReqSendProc(t_XINETFD *pSockFd,t_ResndPacket *pPacklist,e_LINKTYPE linkType,t_IPADDR *pDstAddr)
{
    t_RsndData *loopPacket = XNULL;
    t_RsndData *pdelPacket = XNULL;
    XS32 total_packet=0,success_packet=0,fail_packet=0;
    XS32 out_len = 0;
    XS32 off_set = 0;
    XS32 ret = 0;

    if ((pSockFd == XNULLP) || XOS_INET_INV_SOCK_FD(pSockFd) || XNULL == pPacklist || !pDstAddr)
    {
        return(XERROR);
    }
    loopPacket   = pPacklist->pFirstDataReq;
    total_packet = pPacklist->rsnd_size;
    if((total_packet >0) && (loopPacket == XNULL))
    {
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq ReSendProc(),unbelievable error,packlist is destroyed.");
        }
        return XERROR;
    }
    while(loopPacket != XNULL)
    {
        pdelPacket= loopPacket;
        loopPacket = loopPacket->pNextPacket;
        off_set = pdelPacket->i_offset;
        if(XNULL == pdelPacket->pData || pdelPacket->msgLenth ==0 || off_set >= (XS32)pdelPacket->msgLenth)
        {
            if(g_ntltraceswitch)
            {
                if(XNULL == pdelPacket->pData)
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq ReSendProc() msg is null.");
                }
                if(pdelPacket->msgLenth ==0)
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq ReSendProc() msg len is 0.");
                }
                if(off_set >= (XS32)pdelPacket->msgLenth)
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq ReSendProc() msglen %d, offset %d.",pdelPacket->msgLenth,off_set);
                }
            }
            if(pdelPacket->pData)
            {
                XOS_MemFree(FID_NTL,(XVOID*)(pdelPacket->pData));
            }
            XOS_MemFree((XU32)FID_NTL,pdelPacket);
            pPacklist->rsnd_size--;
            pPacklist->rsnd_delete++;
            pPacklist->pFirstDataReq=loopPacket;
            continue;
        }
        if(off_set >0)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"tcp2 send %d part packet offset %d\r\n",pdelPacket->msgLenth,off_set);
        }
        out_len=0;
        ret =XINET_SendMsg(pSockFd,linkType,pDstAddr,(pdelPacket->msgLenth-off_set),&(pdelPacket->pData[off_set]),&out_len);
        if((ret != XSUCC) || (out_len > 0 && out_len < (XS32)(pdelPacket->msgLenth-(XU32)off_set)))
        {
            pPacklist->rsnd_fail++;
            fail_packet++;

            /*如果是半包发送成功，则需要移动游标*/
            if(out_len< 0)
            {
                out_len=0;
            }
            if(out_len >0)
            {
                pdelPacket->i_offset += out_len;
                /*here need just the send return pos*/
                XOS_Trace(MD(FID_NTL,PL_WARN),"tcp3 send %d offset %d part packet %d send,ret %d\r\n",pdelPacket->msgLenth,off_set,out_len,ret);
            }
            if(g_ntltraceswitch)
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"resend packet size %d,success %d,failed %d,unsend %d\r\n",
                    total_packet,success_packet,fail_packet,pPacklist->rsnd_size);
            }
            return XERROR;/*send failed should return.*/
        }
        else
        {
            /*success clean up packet*/
            XOS_MemFree(FID_NTL,(XVOID*)(pdelPacket->pData));
            XOS_MemFree((XU32)FID_NTL,pdelPacket);

            pPacklist->rsnd_size--;
            pPacklist->rsnd_success++;
            success_packet++;
            pPacklist->pFirstDataReq=loopPacket;
        }
    }
    if(g_ntltraceswitch)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"resend packet size %d,success %d,failed %d,unsend %d\r\n",
            total_packet,success_packet,fail_packet,pPacklist->rsnd_size);
    }

    return XSUCC;
}


XS32 NTL_dataReqTimerSend()
{
    t_XOSCOMMHEAD *msgToNtl = XNULL;
    XU32 len = 4;
    XU16 ret = 0;
    XSTATIC XS32 timer_type = eTCPClient;

    if(XFALSE==g_ntl_timer)
    {
        return XSUCC;
    }
    msgToNtl = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_NTL,len);
    if ( XNULL == msgToNtl )
    {
        return XERROR;
    }
    msgToNtl->datasrc.PID = XOS_GetLocalPID();
    msgToNtl->datasrc.FID = (XU32)FID_NTL;
    msgToNtl->length = len;
    msgToNtl->msgID = eTCPResendTimer;
    if(timer_type == eTCPClient)
    {
        msgToNtl->subID= eTCPClient;
        timer_type = eTCPServer;
    }
    else if(timer_type == eTCPServer)
    {
        msgToNtl->subID= eTCPServer;
        timer_type = eTCPClient;
    }

    msgToNtl->prio = eNormalMsgPrio;
    msgToNtl->datadest.PID = XOS_GetLocalPID();
    msgToNtl->datadest.FID = (XU32)FID_NTL;
    XOS_MemCpy(msgToNtl->message,&len,len);
    /*发送数据*/
    ret = XOS_MsgSend(msgToNtl);
    if(ret != XSUCC)
    {
        /*其他消息类型应该将消息内存释放*/
        XOS_MsgMemFree((XU32)FID_NTL, msgToNtl);
        return XERROR;
    }

    return XSUCC;
}


XS32 NTL_dataReqTimerProc(t_XOSCOMMHEAD *pMsg)
{
    t_TCCB *pTcpCliCb = XNULL;
    t_TSCB *pTcpSrvCb = XNULL;
    t_TSCLI *pTsClient = XNULL;
    t_IPADDR *pPeerAddr = XNULL;/*该参数无效*/
    XS32 i =0, j=0, nIndex=0;

    if(XFALSE==g_ntl_timer)
    {
        return XSUCC;
    }
    if(!pMsg)
    {
        return XERROR;
    }
    
    switch(pMsg->subID)
    {
    case eTCPClient:
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_dataReqTimerProc eTCPClient comming");
        }
        
        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));
        nIndex = XOS_ArrayGetFirstPos(g_ntlCb.tcpClientLinkH);
        for(i=nIndex; i>=0; i=XOS_ArrayGetNextPos(g_ntlCb.tcpClientLinkH,i))
        {
            pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,i);
            if(pTcpCliCb == XNULLP)
            {
                XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
                return XERROR;
            }
            /*先检查链路的状态*/
            if (pTcpCliCb->linkState != eStateConnected)
            {
                XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
                return XERROR;
            }
            if(pTcpCliCb->packetlist.rsnd_size >0)
            {
                NTL_dataReqSendProc(&(pTcpCliCb->sockFd),&(pTcpCliCb->packetlist),eTCPClient,pPeerAddr);
            }
        }
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
        break;

    case eTCPServer:
        if(g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_dataReqTimerProc eTCPServer comming");
        }
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        nIndex = XOS_ArrayGetFirstPos(g_ntlCb.tcpServerLinkH);
        for(i=nIndex; i>=0; i=XOS_ArrayGetNextPos(g_ntlCb.tcpServerLinkH,i))
        {
            pTcpSrvCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,i);
            if(pTcpSrvCb == XNULLP)
            {
                XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
                return XERROR;
            }
            if(pTcpSrvCb->usageNum > 0)
            {
                pTsClient = pTcpSrvCb->pLatestCli;
                if(XNULLP == pTsClient )
                {
                    XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
                    return XERROR;
                }
                for(j=0; j<pTcpSrvCb->usageNum; j++)
                {
                    pPeerAddr=(t_IPADDR*)XOS_HashGetKeyByElem(g_ntlCb.tSerCliH,pTsClient);
                    
                    if(pTsClient == XNULLP)
                    {
                        break;
                    }
                    if(pTsClient->packetlist.rsnd_size >0)
                    {
                        NTL_dataReqSendProc(&(pTsClient->sockFd),&(pTsClient->packetlist),eTCPServer,pPeerAddr);
                    }
                    pTsClient = pTsClient->pPreCli;
                }
            }
        }
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
        break;

    default:
        break;
    }

    return XSUCC;
}


/*因为有链路计数,所以发送结果不通知用户*/
XSTATIC XS32 NTL_dataReqtpktProc(t_XINETFD *pSockFd,t_ResndPacket* pklist,t_IPADDR *pIpAddr,t_DATAREQ *pDataReq)
{
#ifdef XOS_NEED_CHK
    XS32 ret;
    XS32 out_len;
#endif    
    //if(pSockFd == XNULL || pklist == XNULL || pIpAddr==XNULL || pDataReq == XNULL)
    if(pSockFd == XNULL || pklist == XNULL || pDataReq == XNULL)
    {
        return XERROR;
    }
    if(XNULL == pDataReq->pData || pDataReq->msgLenth == 0)
    {
        return XERROR;
    }
#ifdef XOS_NEED_CHK
    pklist->rsnd_total++;
    if(pklist->rsnd_total % (g_ntlpoolsize+1) ==0)
    {
        ret = XOS_INET_OPT_ENABLE;
        XINET_SetOpt(pSockFd, XOS_INET_LEVEL_TCP, XOS_INET_OPT_TCP_NODELAY,(XU32*)&ret);
    }
    if(pklist->rsnd_size ==0)
    {
        /*没有排队,发送数据*/
        out_len=0;
        ret = XINET_SendMsg(pSockFd, eTCPClient, pIpAddr,pDataReq->msgLenth, pDataReq->pData, &out_len);
        if((ret != XSUCC) || (out_len > 0 && out_len < (XS32)pDataReq->msgLenth))
        {
            /*网络阻塞控制冗塞控制,排队,这种情况存在半包的情况,就会不完整*/
            if(out_len< 0)
            {
                out_len=0;
            }
            if(out_len >0)
            {
                /*here need just the send return pos*/
                XOS_Trace(MD(FID_NTL,PL_WARN),"tcp1 send %d part packet %d send,ret = %d\r\n",pDataReq->msgLenth,out_len,ret);
            }
            if(XSUCC == NTL_dataReqSaveProc(pklist,pDataReq,out_len))
            {
                if(g_ntltraceswitch)
                {
                    XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq tpkt,resend wait packet size %d",pklist->rsnd_size);
                }
                /*排队成功*/
                return XSUCC;
            }
            /*计数*/
            pklist->rsnd_delete++;
            /*因为有统计计数,所有不通知用户发送失败*/
            /*排队失败,将数据报丢掉*/
            if(pDataReq->pData != XNULLP)
            {
                XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
            }
            pDataReq->pData = XNULL;
            pDataReq->msgLenth = 0;
            return XERROR;
        }
        else
        {
            pklist->rsnd_success++;
            /*发送成功，清除数据包*/
            if(pDataReq->pData != XNULLP)
            {
                XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
            }
            pDataReq->pData = XNULL;
            pDataReq->msgLenth=0;
            return XSUCC;
        }
    }
    else
    {
        /*先排队*/
        out_len=0;
        if(XSUCC == NTL_dataReqSaveProc(pklist,pDataReq,out_len))
        {
            if(g_ntltraceswitch)
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq tpkt append,resend wait packet size %d",pklist->rsnd_size);
            }
            /*重发包不成功等待,直到成功删除*/
            NTL_dataReqSendProc(pSockFd,pklist,eTCPClient,pIpAddr);
            return XSUCC;
        }
        /*计数*/
        pklist->rsnd_delete++;
        /*排队失败,因为有统计计数,所有不通知用户发送失败*/
        /*排队失败,将pDataReq数据包丢掉*/
        if(pDataReq->pData != XNULLP)
        {
            XOS_MemFree(FID_NTL,(XVOID*)(pDataReq->pData));
        }
        pDataReq->pData = XNULL;
        pDataReq->msgLenth=0;
        NTL_dataReqSendProc(pSockFd,pklist,eTCPClient,pIpAddr);
        return XERROR;
    }
#endif
    return XSUCC;
}


/************************************************************************
函数名:NTL_dataReqProc
功能: 将数据发送到网络
输入: pDataReq  －数据发送的指针
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明:
************************************************************************/
XSTATIC  XS32 NTL_dataReqProc(t_XOSCOMMHEAD *pMsg)
{
    XS32  ret = 0;
    XS32  linkIndex = 0;
    XBOOL errorFlag = XFALSE;
    t_DATAREQ *pDataReq = NULL;
    t_SENDERROR sendError;
    t_UDCB *pUdpCb = XNULL;
    t_TCCB *pTcpCliCb = XNULL;
    t_TSCB *pTcpSrvCb = XNULL;
    t_IPADDR *pIpAddr = XNULL;
    t_TSCLI *pTsClient = XNULL;
    t_LINKCLOSEIND closeInd;
    XS32 out_len = 0;
    XU32 taskNo = 0;

#ifdef INPUT_PAR_CHECK
    if(!pMsg)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_dataReq()->input param pMsg is Null!");
        return XERROR;        
    }

    pDataReq = (t_DATAREQ*)(pMsg->message);
    if(XNULLP == pDataReq)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_dataReq()->input param pMsg is Null!");
        return XERROR;
    }
#endif

    XOS_MemSet(&sendError,0,sizeof(t_SENDERROR));
    /*首先验证链路句柄的有效性*/
    if(!NTL_isValidLinkH(pDataReq->linkHandle))
    {/*如果消息内容指针不为空，应该释放内容*/
        if(XNULLP != pDataReq->pData)
        {
            XOS_MemFree(FID_NTL,(XVOID*)(pDataReq->pData));
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq()->linkHandle [0x%x]is invalid ,free the data pointer!",pDataReq->linkHandle);
        }
        return XERROR;
    }
    /*发送错误的标志置false*/
    errorFlag = XFALSE;
    linkIndex = NTL_getLinkIndex(pDataReq->linkHandle);
    switch(NTL_getLinkType( pDataReq->linkHandle))
    {
    case eUDP:
    /*直接读数据可以看成是线程安全的，因为ipcm的主线程
        只对该位置修改一次，并且在所有的读操作之前修改的*/
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH,linkIndex);
        
        if(pUdpCb == XNULLP)
        { /*内部处理错误应该告警*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq()->can't get the udp link control block data !msg from FID [%d] to dest ip[0x%x] \r\n",pMsg->datasrc.FID,pDataReq->dstAddr.ip);

            /*add by lixn 2007.8.13 for error info*/
            errorFlag = XTRUE;
            closeInd.appHandle = (HAPPUSER)0x00000000;
            closeInd.closeReason = eNetError;
            ret = NTL_msgToUser((XVOID*)&closeInd,&(pMsg->datasrc),sizeof(t_LINKCLOSEIND),eStopInd);
            if ( ret != XSUCC )
            {
                XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq()-> send msg closeind to user failed!");
            }

            /*clean up */
            if(pDataReq->pData != XNULLP)
            {
                XOS_MemFree(FID_NTL,(XVOID*)(pDataReq->pData));
            }
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            
            return XERROR;
        }
        /*发送数据前,先检查链路的状态*/
        if(pUdpCb->linkState != eStateStarted)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorLinkState;
            sendError.userHandle = pUdpCb->userHandle;
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            break;
        }
        /*发送数据时，以dataReq 消息中对端地址优先*/
        if(pDataReq->dstAddr.ip!=0 && pDataReq->dstAddr.port!=0)
        {
            pIpAddr = &(pDataReq->dstAddr);
        }
        else if(pUdpCb->peerAddr.ip!=0 && pUdpCb->peerAddr.port!=0)
        {
            pIpAddr = &(pUdpCb->peerAddr);
        }

        if(pIpAddr != XNULLP)
        {
            /*todo  发送到网络*/
            ret = XINET_SendMsg(&(pUdpCb->sockFd), eUDP, pIpAddr, pDataReq->msgLenth, pDataReq->pData, &out_len);
            if(ret != XSUCC)
            {
                errorFlag = XTRUE;
                sendError.errorReson =(ret ==XERROR)?eOtherErrorReason:(e_ERRORTYPE)ret;
                sendError.userHandle = pUdpCb->userHandle;
            }
        }
        else /*没有目标发送地址*/
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorDstAddr;
            sendError.userHandle = pUdpCb->userHandle;
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq(),send udp data,no peerAddr");
        }
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
        break;

    case eTCPClient:
        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));
        pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,linkIndex);
        
        if(pTcpCliCb == XNULLP)
        {
            /*内部处理错误应该告警*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq()->can't get the tcp client link control block data,msg from FID [%d] to dest ip[0x%x]\r\n",pMsg->datasrc.FID,pDataReq->dstAddr.ip);

            /*add by lixn 2007.8.13 for error info*/
            errorFlag = XTRUE;
            closeInd.appHandle = (HAPPUSER)0x00000000;
            closeInd.closeReason = eNetError;
            ret = NTL_msgToUser((XVOID*)&closeInd,&(pMsg->datasrc),sizeof(t_LINKCLOSEIND),eStopInd);
            if ( ret != XSUCC )
            {
                XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq()-> send msg closeind to user failed!");
            }

            /*clean up */
            XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            return XERROR;
        }
        /*先检查链路的状态*/
        if (pTcpCliCb->linkState != eStateConnected)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorLinkState;
            sendError.userHandle = pTcpCliCb->userHandle;
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            break;
        }
        /*tcp 发送数据时，可以不填对端的地址，为了提高效率，也可以用sendto发送*/
        if((pDataReq->dstAddr.ip == pTcpCliCb->peerAddr.ip) &&
            (pDataReq->dstAddr.port == pTcpCliCb->peerAddr.port))
        {
            pIpAddr = &(pDataReq->dstAddr);
        }
        /* 发送到网络*/
        //        下面为原代码,不作重发的接口
        //        ret = XINET_ Send Msg(&(pTcpCliCb->sockFd),eTCPClient,pIpAddr,pDataReq->msgLenth,pDataReq->pData);
        //        if(ret != XSUCC)
        //        {
        /*网络阻塞控制 todo*/
        /*目前暂时没有作网络冗塞控制*/
        //            errorFlag = XTRUE;
        //            sendError.errorReson =(ret==XERROR)?eOtherErrorReason: (e_ERRORTYPE)ret;
        //            sendError.userHandle = pTcpCliCb->userHandle;
        //        }
        //        break;
        
        /*tcp 封装*/
#ifdef TPKT_NEED
        /*todo*/
#endif /*TPKT_NEED*/
        /*获取链路所在任务号*/
        taskNo = (XU32)NTL_getLinkIndex(pDataReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo > (XU32)g_ntlCb.tcpCliTskNo)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eOtherErrorReason;
            sendError.userHandle = pTcpCliCb->userHandle;
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            break;
        }

        /*发送数据*/
        ret = NTL_dataReqtpktProc(&(pTcpCliCb->sockFd), &(pTcpCliCb->packetlist), pIpAddr, pDataReq);
        if(ret != XSUCC && g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq tcp client call NTL_dataReqtpktProc failed!");
        }
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));       
        return XSUCC;

    case eTCPServer:
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        pTcpSrvCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,linkIndex);
        
        if(pTcpSrvCb == XNULLP)
        { /*内部处理错误应该告警*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq()->can't get the tcp server link control block data !msg from FID [%d] to dest ip[0x%x] \r\n",pMsg->datasrc.FID,pDataReq->dstAddr.ip);

            /*add by lixn 2007.8.13 for error info*/
            errorFlag = XTRUE;
            closeInd.appHandle = (HAPPUSER)0x00000000;
            closeInd.closeReason = eNetError;
            ret = NTL_msgToUser((XVOID*)&closeInd,&(pMsg->datasrc),sizeof(t_LINKCLOSEIND),eStopInd);
            if ( ret != XSUCC )
            {
                XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq()-> send msg closeind to user failed!");
            }

            /*clean up */
            XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return XERROR;
        }

        /*查找对应的客户端cb*/
        pTsClient = NTL_findTclient(pTcpSrvCb, &(pDataReq->dstAddr));
        if(XNULLP == pTsClient )
        {
            /*没找到对应的客户端*/
            errorFlag = XTRUE;
            /*有可能是连接的客户端关闭*/
            sendError.errorReson = eErrorDstAddr;
            sendError.userHandle = pTcpSrvCb->userHandle;
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq(),get tcp client ip[0x%x],port[%d] ctrlBlock failed!",pDataReq->dstAddr.ip,pDataReq->dstAddr.port);
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            break;
        }
        pIpAddr = &(pDataReq->dstAddr);

        /* 发送到网络*/
        //        下面为原代码,不作重发的接口 20080729
        //        ret = XINET_ SendMsg(&(pTsClient->sockFd),eTCPServer,pIpAddr,pDataReq->msgLenth,pDataReq->pData,&out_len);
        //        if(ret != XSUCC)
        //        {
        //            errorFlag = XTRUE;
        //            sendError.errorReson =(ret == XERROR)?eOtherErrorReason:(e_ERRORTYPE)ret;
        //            sendError.userHandle = pTcpSrvCb->userHandle;
        //        }
        /*tcp 封装*/
#ifdef TPKT_NEED
#endif /*TPKT_NEED*/
        ret=NTL_dataReqtpktProc(&(pTsClient->sockFd),&(pTsClient->packetlist),pIpAddr,pDataReq);
        if ( ret != XSUCC && g_ntltraceswitch)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq tcp server call NTL_dataReqtpktProc failed!");
        }
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
        return XSUCC;
#ifdef XOS_SCTP
    case eSCTPClient:
    case eSCTPServer:
        SCTP_dataReqProc(pMsg);
        return XSUCC;
#endif
    case ePCI:
        break;

    default:
        break;
     }

     /*发送过程中出错,发送error send消息到上层*/
     if(errorFlag)
     {
         /*关闭后在向上层发送错误信息*/
         XOS_Trace(MD(FID_NTL,PL_INFO),"srdFID[%d] to destFId[%d] with msgid[%d] and pri[%d]",
             pMsg->datasrc.FID,pMsg->datadest.FID,pMsg->msgID,pMsg->prio);
         ret = NTL_msgToUser((XVOID*)&sendError,&(pMsg->datasrc),sizeof(t_SENDERROR),eErrorSend);
         if ( ret != XSUCC )
         {
             XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_dataReq()-> send msg sendError to user failed!");
         }
     }

     /*释放数据*/
     if(pDataReq->pData != XNULLP)
     {
         XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
     }

     return XSUCC;
}


/*-------------------------------------------------------------------------
                      模块接口函数
-------------------------------------------------------------------------*/
/************************************************************************
函数名:NTL_StopTcpClientTimer
功能:  停止tcp客户端的定时器
输入:  pTcpCb －tcp控制块指针
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明:
************************************************************************/
XS32 NTL_StopTcpClientTimer(t_TCCB *pTcpCb)
{
    XS32 ret = 0;

    if(!pTcpCb)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"NTL_StopTcpClientTimer()->pTcpCb is null!");
        return XERROR;
    }
    
    /*停止定时器*/
    if(pTcpCb->timerId == 0)
    {
        XOS_Trace(MD(FID_NTL, PL_WARN),"NTL_StopTcpClientTimer()->XOS_TimerStop pTcpCb->timerId 0!");
        return XERROR;    
    }
    
    ret = XOS_TimerStop(FID_NTL, pTcpCb->timerId);

    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"NTL_StopTcpClientTimer()->XOS_TimerStop failed!");
    }

    XOS_INIT_THDLE(pTcpCb->timerId);
    
    pTcpCb->expireTimes = 0;
    
    return ret;    
}

/************************************************************************
函数名:NTL_timerProc
功能:  ntl 模块定时器消息处理函数入口
输入:  pMsg －消息指针
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明:
************************************************************************/
XS8 NTL_timerProc( t_BACKPARA* pParam)
{
    t_TCCB *pTcpCb = NULL;
    XS32 ret = 0;
    XU32 taskNo = 0;
    XCHAR szTemp[32] = {0};
    XCHAR szLocalTemp[32] = {0};
    t_STARTACK tcpStartAck;
    XPOINT timer_src[2] = {0};
    XU32 optVal= 0;     

    if(pParam == (t_BACKPARA*)XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_timerProc()->timer PTIMER is null ,bad input param!");
        return XERROR;
    }

    /*发送缓冲区中的数据*/
    timer_src[0] = (XPOINT)pParam->para3;
    timer_src[1] = (XPOINT)pParam->para4;
#if defined(XOS_SCTP) && defined(XOS_LINUX)
    if((timer_src[0] ==  eSCTPCliResendTimer) || (timer_src[0] == eSCTPSerResendTimer) || (timer_src[0] == eSCTPReconnect))
    {
        SCTP_timerProc(pParam);
        return XSUCC;
    }
#endif
    if((timer_src[0] ==  eTCPResendTimer) && (timer_src[1] == eTCPResendCheck))
    {
        NTL_dataReqTimerSend();
        return XSUCC;
    }

    XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));
    /*处理tcp客户端的重连*/
    pTcpCb = (t_TCCB*)XNULLP;

    pTcpCb = (t_TCCB*)(pParam->para1);
    if((t_TCCB*)XNULLP == pTcpCb)
    {
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));

        return XERROR;
    }
    
    /*判断此元素在队列中是否属于空闲，如空闲则不再处理*/
    if(XFALSE == XOS_ArrayIsUesd(g_ntlCb.tcpClientLinkH, pTcpCb)) {
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_timerProc()->pTcpCb is not used again");
        return XERROR;
    }
    
    taskNo = (XU32)(XPOINT)pParam->para2;
    if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
    {
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
        return XERROR;
    }   

    /*检查链路状态*/
    switch(pTcpCb->linkState)
    {
    case  eStateInited:
        if (pTcpCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            g_NtlTest6++;
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_timerProc()->pTcpCb->sockFd.fd is %d,state is eStateInited!",pTcpCb->sockFd.fd);
            if (XSUCC != NTL_closeTCli((XS32)taskNo, pTcpCb, NTL_SHTDWN_SEND))
            {
                XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
                return XERROR;
            }
        }    
         
        /*rebuild a new socket*/
        ret = XINET_Socket(XOS_INET_STREAM,&(pTcpCb->sockFd));
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_timerProc()->XINET_Socket failed");
            break;
        }
        
          if (pTcpCb->linkUser.FID != FID_FTP)
        {
            optVal = XOS_INET_OPT_ENABLE;
            XINET_SetOpt(&pTcpCb->sockFd, SOL_SOCKET, XOS_INET_OPT_LINGER, (XU32 *)&optVal);            
        }
        
        /*尝试绑定，如果配置本端地址并且端口不冲突，应该绑定成功*/
        ret = XINET_Bind(&(pTcpCb->sockFd),&(pTcpCb->myAddr));
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_timerProc()->XINET_Bind[%d] failed", pTcpCb->sockFd.fd);
            break;
        }
        XINET_GetSockName(&(pTcpCb->sockFd),&(pTcpCb->myAddr));
        if(g_ntltraceswitch)
        {
            XOS_IptoStr(pTcpCb->peerAddr.ip,szTemp);
            XOS_IptoStr(pTcpCb->myAddr.ip,szLocalTemp);
            XOS_Trace(MD(FID_NTL,PL_DBG),"NTL_timerProc(),FID[%d] pTcpCb[0x%x] tcp client sock %d connectting.",
                pTcpCb->linkUser.FID,pTcpCb,pTcpCb->sockFd.fd);
            XOS_Trace(MD(FID_NTL,PL_DBG),"the %dth connecting local[%s:%d] => remote[%s:%d].",
                pTcpCb->expireTimes,szLocalTemp,pTcpCb->myAddr.port,szTemp,pTcpCb->peerAddr.port);
        }
        pTcpCb->expireTimes ++;
        ret = XINET_Connect(&(pTcpCb->sockFd),&(pTcpCb->peerAddr));
        if(ret != XSUCC)
        {
            /*
               if(ret != XINET_CLOSE && ret != XERROR) 
                  添加到writeset中,当连接成功时，write集会置位
               else 
                  由下一次定时器超时来进行删除处理
            */
            pTcpCb->linkState = eStateConnecting;
            
            NTL_SetTcpClientFd(pTcpCb, taskNo, eWrite); 
        }
        else
        {
            pTcpCb->linkState = eStateConnected;            

            /*停止定时器*/
            NTL_StopTcpClientTimer(pTcpCb);
                            
            /*添加到readset中*/
            NTL_SetTcpClientFd(pTcpCb, taskNo, eRead);

            /*发送启动成功消息到上层*/
            tcpStartAck.appHandle = pTcpCb->userHandle;
            tcpStartAck.linkStartResult = eSUCC;
            XOS_MemCpy(&(tcpStartAck.localAddr),&(pTcpCb->myAddr),sizeof(t_IPADDR));
            NTL_msgToUser((XVOID*)&tcpStartAck,&(pTcpCb->linkUser),sizeof(t_STARTACK),eStartAck);
        }
        break;

    case eStateConnecting:
        pTcpCb->expireTimes ++;
        if(pTcpCb->expireTimes >= TCP_CLI_RECONNEC_TIMES)
        {
            NTL_closeTCli((XS32)(taskNo), pTcpCb, NTL_SHTDWN_SEND);
            pTcpCb->linkState = eStateInited;
            pTcpCb->expireTimes = 0;
        }
        break;

    case eStateConnected:
        /*已经连接上，关掉定时器*/
        XOS_IptoStr(pTcpCb->peerAddr.ip, szTemp);
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_timerProc(),FID[%d] tcp client  pTcpCb[0x%x] the %dth connect[%s:%d] successed!",
            pTcpCb->linkUser.FID,pTcpCb,pTcpCb->expireTimes ,szTemp,pTcpCb->peerAddr.port);

        /*停止定时器*/
        NTL_StopTcpClientTimer(pTcpCb);

        break;

    default:
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_timerProc()->expire msg in bad state!");
        break;
    }
    XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));

    return XSUCC;
}


/************************************************************************
函数名:NTL_msgProc
功能:  ntl 模块消息处理函数入口
输入:  pMsg －消息指针
输出:
返回: 成功返回XOS_XSUCC,失败返回XERROR
说明: 此消息处理函数是ntl主任务消息入口，edataSend
消息不在此函数处理的范围内
************************************************************************/
XS8 NTL_msgProc(XVOID* pMsgP, XVOID *sb )
{
    t_XOSCOMMHEAD *pMsg = NULL;

    if( (pMsgP == (XVOID*)XNULLP) || ((t_XOSCOMMHEAD*)pMsgP)->message == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_msgProc()->input param error!");
        return XERROR;
    }

    pMsg = (t_XOSCOMMHEAD*)pMsgP;
    
    switch (pMsg->msgID)
    {
    case eLinkInit:
        /*链路初始化，只做一个绑定*/
        NTL_linkInitProc(pMsg);
        break;

    case eLinkStart:
        NTL_linkStartProc( pMsg);
        break;

    case eSendData:
        NTL_dataReqProc(pMsg);
        break;

    case eLinkStop:
        NTL_closeReqProc( pMsg);
        break;

    case eLinkRelease:
        NTL_linkReleaseProc( pMsg);
        break;

    case eTCPResendTimer:
        NTL_dataReqTimerProc( pMsg);
        break;
        
#if defined(XOS_SCTP) && defined(XOS_LINUX)
    case eSCTPCliResendTimer:
    case eSCTPSerResendTimer:
        SCTP_dataReqTimerProc( pMsg);
        break;
#endif
#if 0  /*ntl 改成读配置文件启动*/
    case eGenCfg:
        NTL_genCfgProc( pMsg);
        break;
#endif

    default:
        break;
    }

    return XSUCC;
}


/************************************************************************
函数名:NTL_genInfoShow
功能: 显示NTL配置信息
输入:
输出:
返回:
说明: ntlgenshow命令的最终执行函数
************************************************************************/
XVOID NTL_genInfoShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,"\n____NTL genneral config info___________ \r\n");

    /*检查ntl 模块是不是起来了*/
    if(!g_ntlCb.isGenCfg)
    {
        XOS_CliExtPrintf(pCliEnv,"sorry,NTL has't gennerally configed !\r\n  ");
        return ;
    }

    /*打印通用配置信息*/
    XOS_CliExtPrintf(pCliEnv,
        "max   udp     links :      %d\r\nmax   tcpcli  links :      %d\r\nmax   tcpsrv  links:       %d\r\nsocks thread  polling:     %d\r\ncur   udp     links :      %d\r\ncur   tcpcli  links :      %d\r\ncur   tcpsrv  links:       %d\r\n",
        g_ntlCb.genCfg.maxUdpLink,
        g_ntlCb.genCfg.maxTcpCliLink,
        g_ntlCb.genCfg.maxTcpServLink,
        g_ntlCb.genCfg.fdsPerThrPolling,
        XOS_ArrayGetCurElemNum(g_ntlCb.udpLinkH),
        XOS_ArrayGetCurElemNum(g_ntlCb.tcpClientLinkH),
        XOS_ArrayGetCurElemNum(g_ntlCb.tcpServerLinkH));

    return ;
}


/************************************************************************
函数名:NTL_threadInfoShow
功能: 显示NTL任务信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID NTL_threadInfoShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 i;
    XCHAR taskName[12] = {0};
    XCHAR* state[] ={"inactive", "active"};

    if(!pCliEnv || !ppArgv)
    {
        return;
    }

    XOS_CliExtPrintf(pCliEnv,"\n____________ NTL threads info___________ \r\n");
    /*检查ntl 模块是不是起来了*/
    if(!g_ntlCb.isGenCfg)
    {
        XOS_CliExtPrintf(pCliEnv,"sorry,NTL threads has't start  !\r\n  ");
        return ;
    }
    /*打印线程信息*/
    XOS_CliExtPrintf(pCliEnv,
        "_____common threads info______\r\nudp     threads :       %d\r\ntcpcli  threads :       %d\r\ntcpsrv  threads:        %d\r\n",
        g_ntlCb.udpTskNo,
        g_ntlCb.tcpCliTskNo,
        1
        );

    /*线程详细情况列表*/
    XOS_CliExtPrintf(pCliEnv,
        "\nthreads info list \r\n--------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-12s %-12s %-12s\r\n",
        "name",
        "state",
        "cursocks");
    /*udp task info */
    for(i=0; i<g_ntlCb.udpTskNo; i++)
    {
        sprintf(taskName,"udp[%d]", i);
        XOS_CliExtPrintf(pCliEnv,
            "%-12s %-12s %-12d\r\n",
            taskName,
            state[g_ntlCb.pUdpTsk[i].activeFlag],
            g_ntlCb.pUdpTsk[i].setInfo.sockNum);
    }
    /*tcp cli task info*/
    for(i=0; i<g_ntlCb.tcpCliTskNo; i++)
    {
        sprintf(taskName,"tcpcli[%d]", i);
        XOS_CliExtPrintf(pCliEnv,
            "%-12s %-12s %-12d\r\n",
            taskName,
            state[g_ntlCb.pTcpCliTsk[i].activeFlag],
            g_ntlCb.pTcpCliTsk[i].setInfo.sockNum);
    }
    /*tcp serv tsk info */
    sprintf(taskName,"tcpserv");
    XOS_CliExtPrintf(pCliEnv,
        "%-12s %-12s %-12d\r\n",
        taskName,
        state[g_ntlCb.tcpServTsk.activeFlag],
        g_ntlCb.tcpServTsk.setInfo.sockNum);

    /*end of thread info list */
    XOS_CliExtPrintf(pCliEnv,
        "--------------------------------------\r\n");
    return ;
}


/************************************************************************
函数名:NTL_udpCfgShow
功能: 显示NTL 中udp配置信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID NTL_udpCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 nIndex = -1;
    XS32 i = 0;
    XS32 j = 0;
    t_UDCB *pUdpCb = NULL;
    XCHAR* state[] =
    {
        "Null",
        "Inited",
        "Started",
        "Cncting",
        "Cncted",
        "Listen",
        "Wait",
        "Max"
    };    

    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    /*udp cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "udp config list \r\n-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-22s%-10s%-10s%-6s%-10s%-6s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "myIP",
        "mPort",
        "peerIP",
        "pPort"
        );

    nIndex = XOS_ArrayGetFirstPos(g_ntlCb.udpLinkH);
    for(j = 0, i = nIndex; i >= 0; i = XOS_ArrayGetNextPos(g_ntlCb.udpLinkH, i))
    {
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH, i);
        if(pUdpCb)
        {
            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-22s%-10s%-10x%-6d%-10x%-6d\r\n",
                i,
                pUdpCb->userHandle,
                XOS_getFidName(pUdpCb->linkUser.FID),
                state[pUdpCb->linkState],
                pUdpCb->myAddr.ip,
                pUdpCb->myAddr.port,
                pUdpCb->peerAddr.ip,
                pUdpCb->peerAddr.port);
        }
        
        j++;
        /*做个简单的流控*/
        NTL_SHOW_CMD_CTL(j);
    }
    
    /*end of udp list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------------------------------------\r\n");

    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n",j);
    return ;
}


/************************************************************************
函数名:NTL_tcpCliCfgShow
功能: 显示NTL 中tcp client 配置信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID NTL_tcpCliCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 nIndex = -1;
    XS32 i = 0;
    XS32 j = 0;
    XS32 link_index = -1;
    XS32 timeIndex = 0;
    t_TCCB *tcpCliCb = NULL;

    XCHAR* state[] =
    {
        "Null",
        "Inited",
        "Started",
        "Cncting",
        "Cncted",
        "Listen",
        "Wait",
        "Max"
    };

   

    if(!pCliEnv || !ppArgv)
    {
        return;
    }

    /*tcp client cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "tcp client  config list \r\n-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-22s%-10s%-10s%-6s%-10s%-6s%-6s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "myIP",
        "mPort",
        "peerIP",
        "pPort",
        "timId"
        );
    if((3==siArgc))
    {
        link_index = atoi(ppArgv[1]);
    }
    
    nIndex = XOS_ArrayGetFirstPos(g_ntlCb.tcpClientLinkH);
    for(j=0,i = nIndex; i>=0; i=XOS_ArrayGetNextPos(g_ntlCb.tcpClientLinkH,i))
    {
        tcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,i);
        if(tcpCliCb)
        {
            timeIndex = 0;
            
            if(tcpCliCb->timerId != XNULL)
            {
            timeIndex =(XS32)((XPOINT)(tcpCliCb->timerId)&0x0fffff);
            }

            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-22s%-10s%-10x%-6d%-10x%-6d%-6x\r\n",
                i,
                tcpCliCb->userHandle,
                XOS_getFidName(tcpCliCb->linkUser.FID),
                state[tcpCliCb->linkState],
                tcpCliCb->myAddr.ip,
                tcpCliCb->myAddr.port,
                tcpCliCb->peerAddr.ip,
                tcpCliCb->peerAddr.port,
                timeIndex);
            if(link_index == i)
            {
                if((3==siArgc))
                {
                    if(0 == XOS_StrCmp(ppArgv[2],"restart"))
                    {
                        XOS_CliExtPrintf(pCliEnv,"restart link %i\r\n",i);
                        NTL_RestartLink(tcpCliCb);
                    }
                }
            }
        }
        j++;
        NTL_SHOW_CMD_CTL(j);
    }

    /*end of tcp client list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n",j);
    return ;
}


/************************************************************************
函数名:NTL_tcpCliMsgShow
功能: 显示NTL 中tcp client 每链路统计信息信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID NTL_tcpCliMsgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 nIndex = -1;
    XS32 i =0 ;
    XS32 j = 0;
    t_TCCB* tcpCliCb = NULL;

    XCHAR* state[] =
    {
        "Null",
        "Inited",
        "Started",
        "Cncting",
        "Cncted",
        "Listen",
        "Wait",
        "Max"
    };

    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    if((2 != siArgc))
    {
        XOS_CliExtPrintf(pCliEnv,"input para err.pls input\r\n");
        return;
    }
        
    if(XOS_StrCmp(ppArgv[1], "5G") !=0 )
    {
        return;
    }
    
    /*tcp client cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "tcp client statistic list \r\n---------------------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-18s%-10s%-10s%-8s%-10s%-10s%-10s%-4s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "total",
        "wait",
        "success",
        "delete",
        "fail",
        "curr"
        );

    nIndex = XOS_ArrayGetFirstPos(g_ntlCb.tcpClientLinkH);
    for(j=0,i = nIndex; i>=0; i=XOS_ArrayGetNextPos(g_ntlCb.tcpClientLinkH,i))
    {
        tcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,i);
        if(tcpCliCb)
        {
            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-18s%-10s%-10d%-8d%-10d%-10d%-10d%-4d\r\n",
                i,tcpCliCb->userHandle,XOS_getFidName(tcpCliCb->linkUser.FID),
                state[tcpCliCb->linkState],
                tcpCliCb->packetlist.rsnd_total,
                tcpCliCb->packetlist.rsnd_wait,
                tcpCliCb->packetlist.rsnd_success,
                tcpCliCb->packetlist.rsnd_delete,
                tcpCliCb->packetlist.rsnd_fail,
                tcpCliCb->packetlist.rsnd_size
                );

        }
        j++;
        NTL_SHOW_CMD_CTL(j);
    }

    /*end of tcp client list */
    XOS_CliExtPrintf(pCliEnv,
        "---------------------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n",j);
    return ;
}


/************************************************************************
函数名:NTL_tcpServMsgShow
功能: 显示NTL 中tcp server's client 每链路统计信息信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID NTL_tcpServMsgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 i = 0, j = 0, sum = 0;
    XS32 nIndex = 0;
    t_TSCB* tcpServCb = NULL;
    t_TSCLI* pTcpServCli = NULL;
    t_IPADDR*pPeerAddr = NULL;
    XCHAR* state[] =
    {
        "Null",
        "Inited",
        "Started",
        "Cncting",
        "Cncted",
        "Listen",
        "Wait",
        "Max"
    };    

    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    if(XOS_StrCmp(ppArgv[1], "5G") !=0 )
    {
        XOS_CliExtPrintf(pCliEnv,"pls input para:5G\r\n");
        return;
    }

    XOS_CliExtPrintf(pCliEnv,"tcp server configuration list\r\n--------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-22s%-10s%-10s%-6s%-6s%-6s%-8s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "myIP",
        "mPort",
        "mClis",
        "cClis",
        "authenF"
        );

    nIndex = XOS_ArrayGetFirstPos(g_ntlCb.tcpServerLinkH);
    for(sum = 0, i = nIndex; i >= 0; i = XOS_ArrayGetNextPos(g_ntlCb.tcpServerLinkH,i))
    {
        tcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,i);
        if(tcpServCb)
        {
            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-22s%-10s%-10x%-6d%-6d%-6d%-8x\r\n",
                i,
                tcpServCb->userHandle,
                XOS_getFidName(tcpServCb->linkUser.FID),
                state[tcpServCb->linkState],
                tcpServCb->myAddr.ip,
                tcpServCb->myAddr.port,
                tcpServCb->maxCliNum,
                tcpServCb->usageNum,
                tcpServCb->authFunc);
            if(tcpServCb->usageNum > 0)
            {
                XOS_CliExtPrintf(pCliEnv,"      tcp client connect list\r\n");
                XOS_CliExtPrintf(pCliEnv,"      --------------------------------------------------------------------------------\r\n");
                XOS_CliExtPrintf(pCliEnv,
                    "      %-6s%-10s%-8s%-10s%-8s%-10s%-10s%-10s%-4s\r\n",
                    "index",
                    "peerip",
                    "prport",
                    "total",
                    "wait",
                    "success",
                    "delete",
                    "fail",
                    "curr"
                    );
                pTcpServCli = tcpServCb->pLatestCli;
                if(pTcpServCli)
                {
                    for(j=0; j<tcpServCb->usageNum; j++)
                    {
                        pPeerAddr = (t_IPADDR*)XOS_HashGetKeyByElem(g_ntlCb.tSerCliH,pTcpServCli);
                        XOS_CliExtPrintf(pCliEnv,
                            "      %-6d%-10x%-8d%-10d%-8d%-10d%-10d%-10d%-4d\r\n",
                            j,pPeerAddr->ip,pPeerAddr->port,
                            pTcpServCli->packetlist.rsnd_total,
                            pTcpServCli->packetlist.rsnd_wait,
                            pTcpServCli->packetlist.rsnd_success,
                            pTcpServCli->packetlist.rsnd_delete,
                            pTcpServCli->packetlist.rsnd_fail,
                            pTcpServCli->packetlist.rsnd_size
                            );

                        pTcpServCli = pTcpServCli->pPreCli;
                        if(pTcpServCli == XNULLP)
                        {
                            break;
                        }
                        NTL_SHOW_CMD_CTL(j);
                    }
                    XOS_CliExtPrintf(pCliEnv,"      --------------------------------------------------------------------------------\r\n");
                }
            }
            sum++;
            NTL_SHOW_CMD_CTL(sum);
        }
    }

    /*end of tcp serv clients list */
    XOS_CliExtPrintf(pCliEnv,"--------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,"total lists : %d\r\n",sum);
    return ;
}


/************************************************************************
函数名:NTL_tcpServCfgShow
功能: 显示NTL 中tcp client 配置信息
输入:
输出:
返回:
说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID NTL_tcpServCfgShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XS32 i = 0, j = 0, sum = 0;
    XS32 nIndex = 0;
    t_TSCB *tcpServCb = NULL;
    t_TSCLI *pTcpServCli = NULL;
    t_IPADDR *pPeerAddr = NULL;
    XCHAR* state[] =
    {
        "Null",
        "Inited",
        "Started",
        "Cncting",
        "Cncted",
        "Listen",
        "Wait",
        "Max"
    };    

    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,
        "tcp server configuration list\r\n-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-22s%-10s%-10s%-6s%-6s%-6s%-8s\r\n",
        "index",
        "userHander",
        "userFid",
        "linkstate",
        "myIP",
        "mPort",
        "mClis",
        "cClis",
        "authenF"
        );

    nIndex = XOS_ArrayGetFirstPos(g_ntlCb.tcpServerLinkH);
    for(sum=0,i = nIndex; i>=0; i=XOS_ArrayGetNextPos(g_ntlCb.tcpServerLinkH,i))
    {
        tcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,i);
        if(tcpServCb)
        {
        XOS_CliExtPrintf(pCliEnv,
            "%-6d%-12x%-22s%-10s%-10x%-6d%-6d%-6d%-8x\r\n",
            i,
            tcpServCb->userHandle,
            XOS_getFidName(tcpServCb->linkUser.FID),
            state[tcpServCb->linkState],
            tcpServCb->myAddr.ip,
            tcpServCb->myAddr.port,
            tcpServCb->maxCliNum,
            tcpServCb->usageNum,
            tcpServCb->authFunc);
        if(tcpServCb->usageNum > 0)
        {
            XOS_CliExtPrintf(pCliEnv,
                "      ----------------------------\r\n");
            XOS_CliExtPrintf(pCliEnv,
                "      %-12s%-10s%-8s\r\n",
                "clino",
                "peerIp",
                "pPort"
                );
            pTcpServCli = tcpServCb->pLatestCli;
            for(j=0; j<tcpServCb->usageNum; j++)
            {
                pPeerAddr =
                    (t_IPADDR*)XOS_HashGetKeyByElem(g_ntlCb.tSerCliH,pTcpServCli);
                XOS_CliExtPrintf(pCliEnv,
                    "      %-12d%-10x%-8d\r\n",
                    j,
                    pPeerAddr->ip,
                    pPeerAddr->port);

                pTcpServCli = pTcpServCli->pPreCli;
                if(pTcpServCli == XNULLP)
                {
                    break;
                }
                NTL_SHOW_CMD_CTL(j);
            }
            XOS_CliExtPrintf(pCliEnv,
                "      ----------------------------\r\n");
        }
    }
        sum++;
        NTL_SHOW_CMD_CTL(sum);
    }

    /*end of tcp serv clients list */
    XOS_CliExtPrintf(pCliEnv,
        "-------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "total lists : %d\r\n",sum);
    return ;
}


/************************************************************************
函数名:
功能: 配置NTL 中tcp 重发设置
输入:
输出:
返回:
说明:
************************************************************************/
XVOID NTL_tcpResendCfg(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU32 pool_size = 0;

    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    if(siArgc != 3)
    {
        return;
    }
    if(XOS_StrCmp(ppArgv[1], "5G") !=0 )
    {
        return;
    }
    pool_size = atoi(ppArgv[2]);
    if(!pool_size)
    {
        g_ntlpoolsize =pool_size;
        g_ntl_timer=XFALSE;
        XOS_CliExtPrintf(pCliEnv,"tcp resend pool is closed\r\n");
    }else
    {
        g_ntlpoolsize =pool_size;
        g_ntl_timer=XTRUE;
        XOS_CliExtPrintf(pCliEnv,"tcp resend pool is set to %d\r\n",g_ntlpoolsize);
    }
    return;
}


XVOID NTL_CMDtraceswitch(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XBOOL bOpen = XFALSE;

    if(!pCliEnv || !ppArgv)
    {
        return;    
    }
    
    if(siArgc==2)
    {
        if(XOS_StrCmp(ppArgv[1], "on")==0)
        {
            bOpen=XTRUE;
        }else if(XOS_StrCmp(ppArgv[1], "off")==0)
        {
            bOpen=XFALSE;
        }else
        {
            XOS_CliExtPrintf(pCliEnv,"wrong parameter,usage:ntltrace <on|off>\r\n");
            return;
        }
    }else
    {
        XOS_CliExtPrintf(pCliEnv,"wrong parameter,usage:ntltrace <on|off>\r\n");
        return;
    }
    g_ntltraceswitch = bOpen;
    return;
}


#ifdef VXWORKS
XVOID NTL_CMDCloseTcpSession(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    XU32 conn_port = 0;

    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    if(siArgc != 3)
    {
        return;
    }
    if(XOS_StrCmp(ppArgv[1], "5G") !=0 )
    {
        return;
    }
    conn_port = atoi(ppArgv[2]);
    /*只对23端口进行处理*/
    if(23 == conn_port)
    {
        ntl_test_closetcpsession(conn_port);
        XOS_CliExtPrintf(pCliEnv,"ntl_test_closetcpsession port %d tcp connection ok\r\n",conn_port);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"ntl_test_closetcpsession port %d tcp connection failed, \
                                 the only port 23 is valid\r\n",conn_port);
    }
    return;
}
#endif


/************************************************************************
函数名:NTL_Init
功能: 初始化ntl模块
输入:
输出:
返回:
说明: 注册到模块管理中
************************************************************************/
XS8 NTL_Init(XVOID *p1, XVOID *p2)
{
    t_NTLGENCFG genCfg;
    XS32 ret = 0;
    t_PARA timerPara;
    t_BACKPARA backPara;
    XU16 timer_num = 0;
    
    /*初始化tcp、ip协议栈*/
    XINET_Init();

    /*读配置文件启动*/
    XOS_MemSet(&genCfg,0,sizeof(t_NTLGENCFG));
#ifndef XOS_EW_START
    if (!XOS_ReadNtlGenCfg(&genCfg, "xos.xml"))
#else
        if (!XOS_ReadNtlGenCfg(&genCfg, XOS_CliGetXmlName( )))
#endif
        {
            XINET_End( );
            printf("read xos.xml for ntl configuraion failed\r\n");
            return XERROR;
        }

    /*启动各个任务*/
    ret = NTL_genCfgProc(&genCfg);
    if( ret != XSUCC)
    {
        XINET_End( );
        return XERROR;
    }
#if defined(XOS_SCTP) && defined(XOS_WIN32)
    ret = SCTP_init();
    if( XSUCC != ret )
    {
        printf("init sctp failed\r\n");
        XINET_End( );
        return XERROR;
    }
#endif

#ifdef XOS_SCTP
    /*启动SCTP任务*/
    ret = SCTP_genCfgProc(&genCfg);
    if( ret != XSUCC)
    {
        XINET_End();
        return XERROR;
    }
#endif
    /*注册命令行*/
    NTL_regCli(SYSTEM_MODE);

    /*注册定时器*/
#ifdef XOS_SCTP
    timer_num = genCfg.maxTcpCliLink + genCfg.maxSctpCliLink + INIT_TIMER_NUM;
#else
    timer_num = genCfg.maxTcpCliLink + INIT_TIMER_NUM;
#endif
    ret = XOS_TimerReg(FID_NTL, 500, timer_num, 0);
    if( ret != XSUCC)
    {
        XINET_End( );
        return XERROR;
    }

    /*启动数据重发定时器*/
    XOS_MemSet(&timerPara,0,sizeof(t_PARA));
    XOS_MemSet(&backPara,0,sizeof(t_BACKPARA));
    timerPara.fid  = FID_NTL;
    timerPara.len  = TCP_CLI_RECONNECT_INTERVAL;
    timerPara.mode = TIMER_TYPE_LOOP;
    timerPara.pre  = TIMER_PRE_LOW;
    backPara.para1 = (XPOINT)eTCPResendTimer;
    backPara.para2 = (XPOINT)eTCPResendCheck;
    backPara.para3 = (XPOINT)eTCPResendTimer;
    backPara.para4 = (XPOINT)eTCPResendCheck;

    XOS_INIT_THDLE(ntl_timer);
    if(XSUCC !=XOS_TimerStart(&ntl_timer,&timerPara,&backPara))
    {
        printf("start ntl timer failed\r\n");
    }
    /*20080801 add above*/
    
#if defined(XOS_SCTP) && defined(XOS_LINUX)
    XOS_INIT_THDLE(sctp_cli_timer);
    XOS_INIT_THDLE(sctp_ser_timer);
    backPara.para1 = (XPOINT)eSCTPCliResendTimer;
    backPara.para2 = (XPOINT)eSCTPCliResendCheck;    //para1 para2暂未使用
    backPara.para3 = (XPOINT)eSCTPCliResendTimer;
    backPara.para4 = (XPOINT)eSCTPCliResendCheck;
    if(XSUCC !=XOS_TimerStart(&sctp_cli_timer,&timerPara,&backPara))
    {
        printf("start sctp_cli_timer failed\r\n");
    }
    backPara.para1 = (XPOINT)eSCTPSerResendTimer;
    backPara.para2 = (XPOINT)eSCTPSerResendCheck;    //para1 para2暂未使用
    backPara.para3 = (XPOINT)eSCTPSerResendTimer;
    backPara.para4 = (XPOINT)eSCTPSerResendCheck;
    if(XSUCC !=XOS_TimerStart(&sctp_ser_timer,&timerPara,&backPara))
    {
        printf("start sctp_ser_timer failed\r\n");
    }
#endif
    return XSUCC;
}

XS32 NTL_regCli(int cmdMode)
{
    XS32 ret = 0;
         
    /*平台提示符*/
    ret = XOS_RegistCmdPrompt( cmdMode, "plat", "plat", "no parameter" );

    /*网络传输相关的命令注册*/
    XOS_RegistCommand(ret, NTL_genInfoShow,    "ntlgenshow","display NTL general configuration","no parameter");
    XOS_RegistCommand(ret, NTL_threadInfoShow, "ntlthreadshow","display NTL task information"," no parameter");
    XOS_RegistCommand(ret, NTL_udpCfgShow,     "udpcfgshow","display UDP link information","no parameter");
    XOS_RegistCommand(ret, NTL_tcpCliCfgShow,  "tcpclishow","display TCP client link list","no parameter");
    XOS_RegistCommand(ret, NTL_tcpServCfgShow, "tcpservshow","display TCP server link list","no parameter");
    XOS_RegistCommand(ret, NTL_tcpCliMsgShow,  "tcpcmsg","tcpcmsg","para:5G");
    XOS_RegistCommand(ret, NTL_tcpServMsgShow, "tcpsmsg","tcpsmsg","para:5G");
#if defined(XOS_SCTP) && defined(XOS_LINUX)
    XOS_RegistCommand(ret, SCTP_CliCfgShow,  "sctpclishow","display SCTP client link list","no parameter");
    XOS_RegistCommand(ret, SCTP_ServCfgShow, "sctpservshow","display SCTP server link list","no parameter");
    XOS_RegistCommand(ret, SCTP_CliMsgShow,  "sctpcmsg","sctpcmsg","para:5G");
    XOS_RegistCommand(ret, SCTP_ServMsgShow, "sctpsmsg","sctpsmsg","para:5G");
#endif
#if defined(XOS_SCTP) && defined(XOS_WIN32)
    XOS_RegistCommand(ret, SCTP_CliCfgShow,  "sctpclishow","display SCTP client link list","no parameter");
    XOS_RegistCommand(ret, SCTP_ServCfgShow, "sctpservshow","display SCTP server link list","no parameter");
    XOS_RegistCommand(ret, SCTP_msgQueueShow, "sctplibmsgqshow","display message queue info of sctplib","no parameter");
#endif
    XOS_RegistCommand(ret, NTL_tcpResendCfg,   "tcpresendcfg","tcpresendcfg","para1:5G para2:poolsize");
    XOS_RegistCommand(ret, NTL_CMDtraceswitch, "ntltrace",  "ntltrace","para:on/off");
#ifdef VXWORKS
    XOS_RegistCommand(ret,NTL_CMDCloseTcpSession,"tcpsession","tcpsession","para1:5G para2:connport");
#endif
    return XSUCC;

}


XS8 NTL_TelnetCliCloseSock(t_IPADDR ipAddr)
{
    t_TSCLI *pTservCliCb = (t_TSCLI*)XNULLP;
    XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
    pTservCliCb = (t_TSCLI*)XOS_HashElemFind(g_ntlCb.tSerCliH, &ipAddr);
    
    if(pTservCliCb == XNULLP )
    {
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
        return XERROR;
    }
    NTL_closeTsCli(pTservCliCb);
    XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));

    return XSUCC;
}


//2008/05/16 add ntl and other test code below
int ntl_test_file(char *filename,int lineNum)
{
    FILE   *fp =XNULL;
    int    written_len=0;
    char   buf[1024] = {0};
    int    buff_len = 0;
    int    i = 0;
    int    iTotalLen = 0;
    if(!filename)
    {
        return XERROR;
    }
    fp = fopen(filename,"a");
    if(!fp)
    {
        printf("Open file error!");
        return XERROR;
    }
    for(i=0;i<lineNum;i++)
    {
        memset(buf,0x0,sizeof(buf));
        sprintf(buf,"line_%d",i);
        buff_len = (XS32)strlen(buf);
        written_len = (XS32)fwrite(buf,buff_len,1,fp);
        if(written_len<buff_len)
        {
            printf("write to file error1,written_len %d <buff_len %d\r\n",written_len,buff_len);
            return XERROR;
        }
        iTotalLen+=written_len;
        if(EOF==fputc('\n',fp))
        {
            printf("write to file error2\r\n");
            return XERROR;
        }
        memset(buf,0x0,sizeof(buf));
        sprintf(buf,"helloworld0_helloworld1_helloworld2_helloworld3_helloworld4_helloworld5_helloworld6_helloworld7_9999");
        buff_len = (XS32)strlen(buf);
        written_len = (XS32)fwrite(buf,buff_len,1,fp);
        if(written_len<buff_len)
        {
            printf("write to file error3,written_len %d <buff_len %d\r\n",written_len,buff_len);
            return XERROR;
        }
        if(EOF==fputc('\n',fp))
        {
            printf("write to file error2\r\n");
            return XERROR;
        }
        iTotalLen+=written_len;
    }
    fclose(fp);
    printf("write to file total %d bytes\r\n",iTotalLen);
    return iTotalLen;
}


int ntl_test_send(t_SOCKFD sock, unsigned char *pucData, int iLen, int delay)
{
    int i = 0, j = 0, rtry_time = 0;

    if(NULL == pucData)
    {
        return XERROR;
    }
    
    for (i = 0; i < iLen;)
    {
        //lint -e64
        j = send(sock, &pucData[i], iLen - i, 0);
        //lint +e64
        rtry_time++;
        if (j != (iLen - i))
        {
            //taskDelay(delay);
        }
        if (j < 0)
        {
            if(rtry_time > 10)
            {
                break;/*comment,if delete,will be recycle,else loss packet*/
            }
            continue;
        }
        i += j;
    }
    return i;
}


int ntl_test_socket(int peer_ip,int peer_port,int socketNum,int nbytes,int times,int delay,int closeflag)
{
    int ret = 0, i = 0, j = 0;
    static int i_sum = 0;
    int endPos = 0;
    int tst_sockFd_size[1000] = {0};
    int tst_sockFd_time[1000] = {0};
    int tst_sockFd_time_last[1000] = {0};
    int tst_sockFd_time_begin[1000] = {0};
    XCHAR szLocalTemp[32] = {0};
    XCHAR szPeerTemp[32] = {0};
    t_IPADDR  myAddr[1000];
    t_XINETFD tst_sockFd[1000];
    t_IPADDR  peerAddr;
    int sum = 0;
    char *buff = NULL;
    t_FDSET tcpCliWrite;

    //int iTick[2];

    peerAddr.ip=peer_ip;
    peerAddr.port =peer_port;
    i_sum+=socketNum;
    for(i=0;i<1000;i++)
    {
        myAddr[i].ip =0x00000000;
        myAddr[i].port = 19000+i_sum+i;
        tst_sockFd_size[i]=0;
    }
    if(socketNum <0 || socketNum > 1000)
    {
        socketNum = 1000;
    }
    if(nbytes >0)
    {
        buff=(char*)malloc(nbytes);
        if(NULL == buff)
        {
            return XERROR;
        }

        memset(buff,0x0,nbytes);
        memset(buff,0x39,nbytes-1);
    }
    printf("sockNum=%d pksize=%d pknum=%d delay %d\r\n",socketNum,nbytes,times,delay);
    for(i=0;i<socketNum;i++)
    {
        ret = XINET_Socket(XOS_INET_STREAM,&tst_sockFd[i]);
        if (ret != XSUCC)
        {
            printf("socket[%d] create tcp client socket failed!\r\n",i);
            break;
        }
        endPos=i;
        /*对于tcp 客户端的服务，不care 绑定的结果*/
        ret=XINET_Bind(&tst_sockFd[i],&myAddr[i]);
        if (ret != XSUCC)
        {
            printf("socket[%d] = %d bind port %d failed!\r\n",i,tst_sockFd[i].fd,myAddr[i].port);
            break;
        }
        printf("socket[%d] = %d bind port %d ok!\r\n",i,tst_sockFd[i].fd,myAddr[i].port);

        XINET_GetSockName(&tst_sockFd[i],&myAddr[i]);
        XOS_IptoStr(myAddr[i].ip,szLocalTemp);
        XOS_IptoStr(peer_ip,szPeerTemp);

        /*连接到对端*/
        ret = XINET_Connect(&tst_sockFd[i],&peerAddr);
        if(ret != XSUCC)
        {
            printf("socket[%d] = %d;[%s:%d] connect [%s:%d]waiting!\r\n",i,tst_sockFd[i].fd,
                szLocalTemp,myAddr[i].port,szPeerTemp,peer_port);
        }
        XOS_Sleep(100);
        XOS_MemSet(&tcpCliWrite,0,sizeof(t_FDSET));
        XOS_INET_FD_ZERO(&(tcpCliWrite));
        XOS_INET_FD_SET(&tst_sockFd[i],&(tcpCliWrite));
        j=500;/*j,sum as two temp variable*/
        ret = XINET_Select((t_FDSET*)XNULL,&tcpCliWrite,(XU32*)&j,(XS16*)&sum);
        if(ret != XSUCC)
        {
            /*select 超时*/
            if(ret == XINET_TIMEOUT)
            {
                printf("socket[%d] = %d select errinfo=%s,time out !\r\n",i,tst_sockFd[i].fd,XOS_StrError(INET_ERR_CODE));
            }
            else
            {
                printf("socket[%d] = %d select errinfo=%s!\r\n",i,tst_sockFd[i].fd,XOS_StrError(INET_ERR_CODE));
            }
            //break;
            continue;
        }

        if(XSUCC != XINET_TcpConnectCheck(&tst_sockFd[i]))
        {
            printf("XINET_TcpConnectCheck check failed!\r\n");
            continue;
        }else
        {
            printf("XINET_TcpConnectCheck check successed!\r\n");
        }

        tst_sockFd_time_begin[i]=(XS32)time(NULL);
        tst_sockFd_time_last[i]=tst_sockFd_time_begin[i];
        ret=0;
        tst_sockFd_size[i]=0;

        //iTick[0] = tickGet();
        for(j=0;j<times;j++)
        {
            tst_sockFd_time[i]=(XS32)time(NULL);
            ret=ntl_test_send(tst_sockFd[i].fd,(unsigned char*)buff,nbytes,delay);
            tst_sockFd_size[i]+=ret;
            sum+=ret;
            if(delay >0 )
            {
                XOS_Sleep(delay);
            }
            if(tst_sockFd_time[i] == tst_sockFd_time_last[i]+1)
            {
                printf("socket[%d] = %d;one second send total %d bytes!\r\n",i,tst_sockFd[i].fd,tst_sockFd_size[i]);
                tst_sockFd_time_last[i]=tst_sockFd_time[i];
            }
        }
        //iTick[1] = tickGet();
        tst_sockFd_time[i]=(XS32)time(NULL);
        printf("socket[%d] = %d send total %d bytes successed, %d second!\r\n",i,tst_sockFd[i].fd,tst_sockFd_size[i],tst_sockFd_time[i] -  tst_sockFd_time_begin[i]);
        printf("close socket[%d] = %d\r\n",i,tst_sockFd[i].fd);
        printf("\r\n");
        if(closeflag)
        {
            shutdown(tst_sockFd[i].fd,2);
            XINET_CloseSock(&tst_sockFd[i]);
        }
    }
    for(i=0;i<socketNum;i++)
    {
        if(!XOS_INET_INV_SOCK_FD(&tst_sockFd[i]))
        {
            if(closeflag)
            {
                shutdown(tst_sockFd[i].fd,2);
                XINET_CloseSock(&tst_sockFd[i]);
            }
        }
    }

    printf("card total %d bytes successed out\r\n",sum);
    printf("open total %d sockets ok\r\n",endPos+1);
    printf("\r\n");
    printf("\r\n");
    free(buff);
    return 0;
}


//2008/05/16 add ntl and other test code above
#ifdef XOS_VXWORKS
int ntl_test_getpeerport( int sock )
{
    struct sockaddr_in sin;
    
    int len = sizeof(struct sockaddr_in);
    if ( sock < 0 )
        return -1;
    if ( getpeername( sock, (t_INETSOCKADDR*)&sin,&len ) == 0 )
    {
        printf("peer port %d\r\n",XOS_NtoHs(sin.sin_port));
        return (XOS_NtoHs(sin.sin_port));
    }
    return -1;
}


int ntl_test_getlocalport( int sock )
{
    struct sockaddr_in sin;
    
    int len = sizeof(struct sockaddr_in);
    if (getsockname ( sock, (t_INETSOCKADDR*)&sin,&len ) == 0 )
    {
        printf("local port %d\r\n",XOS_NtoHs(sin.sin_port));
        return (XOS_NtoHs(sin.sin_port));
    }
    return -1;
}


int ntl_test_closetcpsession(int con_port)
{
    int fd_max = FD_SETSIZE + 1;
    int tmp_fd = 0;
    
#ifdef XOS_NEED_CHK
    for(tmp_fd =0 ;tmp_fd<fd_max;tmp_fd++)
    {
        if(ntl_test_getlocalport(tmp_fd) == con_port)
        {
            if(ntl_test_getpeerport(tmp_fd)>0)
            {
                printf("close fd %d\r\n",tmp_fd);
                shutdown(tmp_fd,2);
                close(tmp_fd);
                continue;
                //return 0;
            }
        }
    }
#endif
    return -1;
}
#endif
//2008/06/02 add ntl and other test code above

/************************************************************************
函数名:NTL_channel_find_function
功能: 根据用户参数查找通道是否已分配,如果已分配，则重置为init
输出:
返回: 成功返回XTRUE,失败返回XERROR
说明:
************************************************************************/
XBOOL NTL_channel_find_function(XOS_ArrayElement element1, XVOID *param)
{
    t_UDCB *pUdpCb = XNULL;
    t_TCCB *pTcpCliCb = XNULL;
    t_TSCB *pTcpSrvCb = XNULL;
    
    t_Link_Index *trap_target2 = (t_Link_Index *)param;
    
    if(XNULL == trap_target2)
    {
        return XFALSE;
    }

    switch(trap_target2->linkType)
    {
        case eUDP:pUdpCb = (t_UDCB *)element1;
            if(XNULL == pUdpCb)
            {
                return XFALSE;
            }
            if(pUdpCb->linkUser.FID== trap_target2->linkUser->FID &&
                pUdpCb->userHandle == trap_target2->userHandle)
            {
                return XTRUE;
            }        
            break;
        case eTCPClient:pTcpCliCb = (t_TCCB *)element1;
             if(XNULL == pTcpCliCb)
            {
                return XFALSE;
            }
            if(pTcpCliCb->linkUser.FID == trap_target2->linkUser->FID &&
                pTcpCliCb->userHandle == trap_target2->userHandle)
            {
                return XTRUE;
            }        
            break;
        case eTCPServer:pTcpSrvCb = (t_TSCB *)element1;
             if(XNULL == pTcpSrvCb)
            {
                return XFALSE;
            }
            if(pTcpSrvCb->linkUser.FID == trap_target2->linkUser->FID &&
                pTcpSrvCb->userHandle == trap_target2->userHandle)
            {
                return XTRUE;
            }        
            break;
        default:
            return XFALSE;
    }        

    return XFALSE;
}
/************************************************************************
函数名:NTL_CloseUdpSocket
功能:  关闭udp的socket，并重置读集
输入:  pUdpCb －udp控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: 此函数的udp控制块需要调用方进行全局保护
************************************************************************/
XS16 NTL_CloseUdpSocket(t_UDCB *pUdpCb, XU32 taskNo)
{
    if(!pUdpCb)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_CloseUdpSocket()->pUdpCb invalid!");
        return XERROR;
    }
    
    XOS_INET_FD_CLR(&(pUdpCb->sockFd),&(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet));
#ifdef  XOS_WIN321
    if(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet.fd_count == 0)
#else
    if(--g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum == 0)
#endif
    {
        /*捕获udp任务的驱动信号，使任务阻塞*/
        XOS_SemGet(&(g_ntlCb.pUdpTsk[taskNo].taskSemp));
    }

    if(g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum == 0xffff)
    {
        g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum = 0;
    }

    return XINET_CloseSock(&(pUdpCb->sockFd));
    
}

/************************************************************************
函数名:NTL_ResetLinkByReapply
功能:  因重复申请链路而停止原来的链路
输入:  pCloseReq －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_ResetLinkByReapply(t_LINKCLOSEREQ* pCloseReq)
{    
    e_LINKTYPE linkType;
    t_UDCB *pUdpCb = NULL;
    t_TCCB *pTcpCliCb = NULL;
    t_TSCB * pTcpServCb = NULL;
    XU32 taskNo = 0;

    if(NULL == pCloseReq)
    {
         XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->pCloseReq invalid!");
        return XERROR;
    }

    /*下行的消息都要检查句柄的的有效性*/
    if(!NTL_isValidLinkH(pCloseReq->linkHandle))
    {
        /*链路句柄无效的消息，直接返回*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->linkHandle invalid!");
        return XERROR;
    }
    /*获取链路类型*/
    linkType = NTL_getLinkType(pCloseReq->linkHandle);

    switch(linkType)
    {
    case eUDP:
        /*获取udp 控制块*/
        pUdpCb = (t_UDCB*)XNULLP;
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(XNULLP == pUdpCb)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->get the udp control block failed!");
            return XERROR;
        }

        taskNo = NTL_getLinkIndex(pCloseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.udpTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->udp taskNo is %d!", taskNo);
            return XERROR;
        }
        
        if (pUdpCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            /*关闭socket，清除fdset*/
            NTL_CloseUdpSocket(pUdpCb, taskNo);
        }       

        /*初始化链路状态*/
        pUdpCb->linkState = eStateInited;
        pUdpCb->peerAddr.ip = 0;
        pUdpCb->peerAddr.port = 0;
        pUdpCb->myAddr.ip = 0;
        pUdpCb->myAddr.port = 0;
        pUdpCb->sockFd.fd = XOS_INET_INV_SOCKFD;
        break;

    case eTCPClient:
        /*获取tcp 控制块*/
        pTcpCliCb = (t_TCCB*)XNULLP;
        pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(XNULLP == pTcpCliCb)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->get the tcp client control block failed!");
            return XERROR;
        }        

        taskNo = NTL_getLinkIndex(pCloseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->tcp client taskNo is %d", taskNo);
            return XERROR;    
        }

        /*关闭定时器*/
        NTL_StopTcpClientTimer(pTcpCliCb);

           /*清空未发送的数据，关闭fd*/
        if (pTcpCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            NTL_closeTCli(taskNo, pTcpCliCb, NTL_SHTDWN_SEND);
        }

        /*关闭后置成初始化状态*/
        pTcpCliCb->peerAddr.ip = 0;
        pTcpCliCb->peerAddr.port = 0;
        pTcpCliCb->myAddr.ip = 0;
        pTcpCliCb->myAddr.port = 0;
        pTcpCliCb->linkState = eStateInited;
        pTcpCliCb->sockFd.fd = XOS_INET_INV_SOCKFD;
        pTcpCliCb->expireTimes = 0;

        break;

    case eTCPServer:
        pTcpServCb = (t_TSCB*)XNULLP;
        pTcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(pTcpServCb == XNULLP)
        {
            /*这种错误很严重，认为上层发错消息了，所以将消息丢弃*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_ResetLinkByReapply()->get the tcp server control block failed!");
            return XERROR;
        }

        /*关闭tcpserver*/
        NTL_CloseTcpServerSocket(pTcpServCb);        
        /*改变相应的cb 数据,初始化*/
        pTcpServCb->authFunc = (XOS_TLIncomeAuth)XNULL;
        pTcpServCb->linkState = eStateInited;
        pTcpServCb->maxCliNum = 0;
        pTcpServCb->usageNum = 0;
        pTcpServCb->pLatestCli = (t_TSCLI*)XNULLP;
        pTcpServCb->myAddr.ip = 0;
        pTcpServCb->myAddr.port = 0;
        pTcpServCb->sockFd.fd = XOS_INET_INV_SOCKFD;
        pTcpServCb->pParam = NULL;
        break;

    default:
        return XERROR;
    }

    return XSUCC;
    
}

/************************************************************************
函数名:NTL_CloseTcpServerSocket
功能:  关闭tcpserver
输入:  pTcpServCb －tcpserver控制块
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明: tcpserver需要调用方的全局保护
************************************************************************/
XS32 NTL_CloseTcpServerSocket(t_TSCB *pTcpServCb)
{
    XS32 ret  = 0;
    if(NULL == pTcpServCb)
    {
        return XERROR;
    }

    
    /*首先关闭所有接入的客户端*/
    while(pTcpServCb->pLatestCli != XNULLP)
    {
        NTL_closeTsCli(pTcpServCb->pLatestCli);
        if(pTcpServCb->usageNum == 0)
        {
            break;
        }
    }

    /*关闭fd*/
    if (pTcpServCb->sockFd.fd != XOS_INET_INV_SOCKFD)
    {        
        XOS_INET_FD_CLR(&(pTcpServCb->sockFd),&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));
#ifdef XOS_WIN321
        if(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet.fd_count == 0)
#else
        if(--g_ntlCb.tcpServTsk.setInfo.sockNum == 0)
#endif
        {
            XOS_SemGet(&(g_ntlCb.tcpServTsk.taskSemp));
        }
        if(g_ntlCb.tcpServTsk.setInfo.sockNum == 0xffff)
        {
            g_ntlCb.tcpServTsk.setInfo.sockNum =0;
        }

        ret = XINET_CloseSock(&(pTcpServCb->sockFd));
        if( ret != XSUCC )
        {
            return XERROR;
        }
    }        

    return XSUCC;
}

/************************************************************************
函数名:NTL_ResetLinkByReapplyEntry
功能:  因重复申请链路而停止原来的链路
输入:  pCloseReq －消息指针
输出:
返回: 成功返回XSUCC,否则返回XERROR
说明:
************************************************************************/
XS32 NTL_ResetLinkByReapplyEntry(e_LINKTYPE linkType, t_LINKINITACK *linkInitAck, HAPPUSER *pUserHandle, XS32 *pnRtnFind)
{
    t_LINKCLOSEREQ stopReq;

    if(NULL == linkInitAck ||  NULL == pUserHandle || NULL == pnRtnFind)
    {
         XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapplyEntry()->add para is error!");
         return XERROR;
    }
    
    stopReq.cliAddr.ip = 0;
    stopReq.cliAddr.port = 0;
    stopReq.linkHandle = (HLINKHANDLE)NTL_buildLinkH(linkType,(XU16)(*pnRtnFind));
    
    NTL_ResetLinkByReapply(&stopReq);    
   
    linkInitAck->linkHandle = stopReq.linkHandle;
    linkInitAck->appHandle = *pUserHandle;
    linkInitAck->lnitAckResult = eSUCC;

    return XSUCC;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */


