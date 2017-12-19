
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
                  ����ͷ�ļ�
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
                      ģ���ڲ��궨��
-------------------------------------------------------------------------*/
/* ��֤��*/
#define  LINK_CHECK_NUM    0xaaa
#define INIT_TIMER_NUM        3        /*tcp + sctp server�����ط� + sctp client�����ط�*/

/*-------------------------------------------------------------------------
                     ģ���ڲ��ṹ��ö�ٶ���
-------------------------------------------------------------------------*/

#ifdef VXWORKS
  int ntl_test_closetcpsession(int con_port);
#endif

/*20080710 add below*/
PTIMER ntl_timer= XNULL;
XBOOL g_ntl_timer=XTRUE;
/*20080710 add above*/

#ifdef XOS_LINUX
PTIMER sctp_cli_timer= XNULL;    /*sctp�ͻ��������ط���ʱ��*/
PTIMER sctp_ser_timer= XNULL;    /*sctp����������ط���ʱ��*/
XBOOL g_sctp_timer=XTRUE;
#endif
/*20131022 add above*/

XEXTERN XBOOL g_ntltraceswitch;

/*��������ͳ��ֵ����NTL�쳣�������ʹ��*/
XU32 g_NtlTest1 = 0;
XU32 g_NtlTest4 = 0;
XU32 g_NtlTest6 = 0;

XS32 NTL_regCli(int cmdMode);
/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
XSTATIC  t_NTLGLOAB g_ntlCb;
XU32 g_ntlpoolsize=100;
XU32 g_tcpSerCloseFail=0;
XU32 g_tcpSerReleaseFail=0;
XS32 NTL_closeTsCli(t_TSCLI* pTcpServCli);


/*-------------------------------------------------------------------------
                   ģ���ڲ�����
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


/*2007/12/05����ͨѶģ�鷢�����ݱ�����ԭ���ӡ*/
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


/*2007/12/05����ͨѶģ�鷢�����ݱ�����ԭ���ӡ*/
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
    /*������Ϣ�ڴ�*/
    pMsg = (t_XOSCOMMHEAD*)XNULL;
    pMsg = XOS_MsgMemMalloc(FID_NTL,sizeof(t_LINKSTART));
    if(pMsg == XNULL)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"ERROR: link msg to ntl: can not malloc memory.");
        return XERROR;
    }

    /*��д��Ϣ����*/
    pMsg->datasrc.PID = XOS_GetLocalPID();
    pMsg->datasrc.FID = FID_NTL;
    pMsg->datadest.PID = XOS_GetLocalPID();
    pMsg->datadest.FID = FID_NTL;
    pMsg->prio = eFastMsgPrio;
    pMsg->msgID = eLinkStart;
    XOS_MemCpy(pMsg->message, &startLnk, sizeof(t_LINKSTART));

    /*��������*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        /*����ָʾ��Ϣ��Ӧ�����ͷ��յ�����*/
        XOS_MsgMemFree(FID_NTL, pMsg);
        XOS_Trace(FILI, FID_NTL, PL_ERR, "ERROR: NTL_RestartLink send msg failed.");
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
������:NTL_buildLinkH
����: ������·���
����:
linkType �� ��·����(����λ)
linkIndex ����·����
���:
����:  ������·���
˵��:  ��·�����֯��ͼ��

            linkType  checkcoder          linkIndex
           31      27                   15                            0 (bit)
           +----+------------+----------------+
           |        |                      |                            |
           +----+------------+----------------+

************************************************************************/
HLINKHANDLE  NTL_buildLinkH(  e_LINKTYPE linkType,XU16 linkIndex)
{
    /*LINK_CHECK_NUM,Ϊ�˶㿪vc�в���ʼ���ľֲ�������Ϊ0xcccc �����*/
    XU32 checkNum = LINK_CHECK_NUM;
    HLINKHANDLE  linkhandle;
    linkhandle = (HLINKHANDLE)(XPOINT)(((linkType&0xf)<<28) |(checkNum<<16) | ((linkIndex&0xffff)));
    return linkhandle;
}


/************************************************************************
������:isValidLinkH
����: ��֤��·�������Ч��
����: ��·���
���:
����: ��Ч����XTURE,���򷵻�XFALSE
˵��:
************************************************************************/
XBOOL NTL_isValidLinkH( HLINKHANDLE linkHandle)
{
    XU32 checkNum = LINK_CHECK_NUM; /*0xaaa*/
    return (LINK_CHECK_NUM == ((XPOINT)linkHandle&(checkNum<<16))>>16)? XTRUE:XFALSE;
}


/************************************************************************
������:NTL_getLinkType
����: ͨ����·�����ȡ��·����
����: ��·���
���:
����: ��·����
˵��:
************************************************************************/
e_LINKTYPE NTL_getLinkType( HLINKHANDLE linkHandle)
{
    return (e_LINKTYPE)(((XPOINT)linkHandle)>>28);
}


/************************************************************************
������:NTL_getLinkIndex
����: ͨ����·�����ȡ��·Index
����: ��·���
���:
����: ��·����
˵��:
************************************************************************/
XS32 NTL_getLinkIndex( HLINKHANDLE linkHandle)
{
    return (XS32)(((XPOINT)linkHandle)&0xffff);
}


/************************************************************************
������:NTL_findTclient
����:  ����tcp ���ӵĿͻ�
����:
pTserverCb ��server cb��ָ��
pClientAddr  �� �ӽ����ͻ��ĵ�ַָ��
���:
����: �ɹ����ؿ��ƿ�ָ��,���򷵻�xnullp
˵��:
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
������:NTL_tcliHashFunc
����:  ����tcpserver ����ͻ���hash ����
����:
param ��hash key ��ָ��
paramSize  ��key�Ĵ�С
hashSize ��hash Ͱ�Ĵ�С
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XU32  NTL_tcliHashFunc( XVOID *param, XS32 paramSize, XS32 hashSize)
{
    XU32 hashKey = 0;
    t_IPADDR *pAddr = NULL;

    if(param == XNULLP || (XU32)paramSize != sizeof(t_IPADDR))
    {
        /*������������,�����ڵ�һ��λ����*/
        return 0;
    }

    pAddr = (t_IPADDR*)param;

    /*Hash by IP and PORT fields */
    hashKey = hashKey << 1 ^ pAddr->ip;
    hashKey = hashKey << 1 ^ pAddr->port;

    return hashKey%((XU32)hashSize);
}

/************************************************************************
* key �ȽϺ����Ķ���
* ����  :
* key1,key2    - ���ڱȽϵ�����Keyֵ
* keySize       - keyֵ�Ĵ�С
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
������:NTL_msgTo User
����:  ������·��ʼ����Ϣ
����:
pContent ����Ϣ����ָ��
pLinkUser �� ��·ʹ����ָ��
len   �� ��Ϣ����
msgType �� ��Ϣ����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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
    /*������Ϣ�ڴ�*/
    pMsg = XOS_MsgMemMalloc(FID_NTL,(XU32)len);
    if(pMsg == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_msg ToUser()->malloc msg failed!");
        return XERROR;
    }

    /*��д��Ϣ����*/
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

    /*��������*/
    ret = XOS_MsgSend(pMsg);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"NTL_msg ToUser()->send msg type[%d] to FID[%d] failed!",msgType,pLinkUser->FID);
        /*clean up */
        /*����ָʾ��Ϣ��Ӧ�����ͷ��յ�����*/
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
������  : XOS_ReadNtlGenCfg
����    : get XOS NTL configure informations
����    : filename   XOS �����ļ���
���    :
����    : XBOOL
˵����
************************************************************************/
XBOOL XOS_ReadNtlGenCfg( t_NTLGENCFG* pNtl, XCHAR* filename )
{    
    /* none Vxworks OS */
#ifndef XOS_VXWORKS
    
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlChar* pTempStr   = XNULL;
    
#endif
    
    /*�����Ϸ��Լ��*/
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

    /*��ȡ�ļ�*/
    doc = xmlParseFile(filename);
    if (doc == XNULL)
    {
        return( XFALSE );
    }
    
    /*�ҵ����ڵ�*/
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
    
    /*�ҵ� NTL ���ڵ�*/
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
    /*���� NTL �ӽڵ�*/
    while ( cur != XNULL )
    {
        /*MAXUDPLINK �ڵ�*/
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
        /*MAXTCPCLILINK �ڵ�*/
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
        /*MAXTCPSERVLINK �ڵ�*/
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
        /*MAXSCTPCLILINK �ڵ�*/
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
        /*MAXSCTPSERVLINK �ڵ�*/
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

        /*ÿ��sctp����˿ɽ�������ͻ�������*/
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
            
        /*SCTP_HB_INTERVAL �ڵ�*/
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
        /*SCTP_RTO_MIN �ڵ�*/
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
        /*SCTP_RTO_INIT �ڵ�*/
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

        /*SCTP_SACK_TIMEOUT �ڵ�*/
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
        /*FDSPERTHRPOLLING �ڵ�*/
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
������:NTL_udpRcvFunc
����:  ����tcp ���ӵĿͻ�
����:
pTserverCb ��server cb��ָ��
pClientAddr  �� �ӽ����ͻ��ĵ�ַָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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
        /*�̸߳�����ʱ����û��sockfd��fdΪ��ʱselectֱ�ӷ���*/
        g_ntlCb.pUdpTsk[inputNo].activeFlag = XFALSE;

        /*�ȴ�start����*/
        XOS_SemGet(&(g_ntlCb.pUdpTsk[inputNo].taskSemp));
        g_ntlCb.pUdpTsk[inputNo].activeFlag = XTRUE;
        XOS_SemPut(&(g_ntlCb.pUdpTsk[inputNo].taskSemp));

        /*�Կ��ƿ���в���*/
        /*�������������ֲ�������,���ڲ��ƻ�ȫ�ֵĶ���*/
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        XOS_INET_FD_ZERO(&(udpReadSet.fdSet));
        XOS_MemCpy(&udpReadSet, &(g_ntlCb.pUdpTsk[inputNo].setInfo.readSet), sizeof(t_SOCKSET));
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));

        /*poll sock fd set*/
        setNum = 0;

        ret = XINET_Select(&(udpReadSet.fdSet), (t_FDSET*)XNULLP, (XU32 *)&pollAbideTime, &setNum);
        if(ret != XSUCC)
        {
            /*select ��ʱ*/
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
        /*��������������ݶ�*/
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

                    /*��������������*/
                    ret = XINET_RecvMsg(&(pUdpCb->sockFd),&fromAddr,&pData,
                        &len,XOS_INET_DGRAM,XOS_INET_CTR_NOCOMPATI_TCPEEN);
                    if(ret == XINET_CLOSE)
                    {
                        /*�ر���·*/
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

                    /*������Ϣ���ϲ�*/
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
                            if( (0 == (XU8)(XU32)pUdpCb->userHandle)/* ���ͨ��,��վ */
                                || (8 == (XU8)(XU32)pUdpCb->userHandle)/* SS */
                                || (9 == (XU8)(XU32)pUdpCb->userHandle) )/* ģ����� */
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

            if(setNum > 0) /*���ڿͻ��˱�ɾ�����������¼����ܱ�����*/
            {
                XOS_Trace(MD(FID_NTL,PL_ERR), "NTL_udpRcvFunc setNum %d error", setNum);
            }            
        }
        XOS_Sleep(1);
    }
}


/************************************************************************
������:NTL_noticeCloseTCli
����:  ֪ͨ�ر�tcp�Ŀͻ���
����: pTcpCliCb   �ͻ��˿��ƿ�ָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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
������:
����:  �ر�tcp�ͻ���
����:
taskNo       �����
pTcpCliCb   �ͻ��˿��ƿ�ָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: �ù��ܽӿ���Ҫ����������,�����ر�;�����ر�.
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
        
    /*���ö�д��*/
    /*��տͻ��˵�����*/
    NTL_dataReqClear(&(pTcpCliCb->packetlist));
    switch(close_type)
    {
    case NTL_SHTDWN_RECV:
        /*����read set */
        XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));
        break;
    case NTL_SHTDWN_SEND:
        /*����read set */
        XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));
        /*����write set */
        /*VxWorks�����µ���Աfd����FD_SIZEʱ,������λ�û�������λ��bitMaxsk[FD_SIZE/8]*/
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
        /*���ͻ�������Ϊ0ʱ������ͻ�������������ź������������´��������ȴ��´��пͻ�������ʱ������*/
        XOS_SemGet(&(g_ntlCb.pTcpCliTsk[taskNo].taskSemp));
    }

    if(g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum  == 0xffff)
    {
        g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum = 0;
    }

    /*�ر�socket*/
    if ( XSUCC != XINET_CloseSock(&(pTcpCliCb->sockFd)))
    {
        return XERROR;
    }

    return XSUCC;
}

/************************************************************************
������:NTL_ResetAllTcpFd
����:  �������е�tcp��д��
����:  taskNo  �����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 NTL_ResetAllTcpFd(XU32 taskNo)
{
    /*���ö�д��*/
    /*����read set */
    XOS_INET_FD_ZERO(&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));
    /*����write set */
    /*VxWorks�����µ���Աfd����FD_SIZEʱ,������λ�û�������λ��bitMaxsk[FD_SIZE/8]*/
    XOS_INET_FD_ZERO(&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));

    g_ntlCb.pTcpCliTsk[taskNo].setInfo.sockNum = 0;

    return XSUCC;
}

/************************************************************************
������:NTL_tcpCliRcvFunc
����:  tcp�ͻ��˵���ں���
����:taskNo  �����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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

    /*����*/
    pollAbideTime = POLL_FD_TIME_OUT;
    taskNo = (XU32)(XPOINT)taskPara;

    while(1)
    {
        /*�̸߳�����ʱ����û��sockfd��fdΪ��ʱselectֱ�ӷ���*/
        g_ntlCb.pTcpCliTsk[taskNo].activeFlag = XFALSE;
        
        XOS_SemGet(&(g_ntlCb.pTcpCliTsk[taskNo].taskSemp));
        g_ntlCb.pTcpCliTsk[taskNo].activeFlag = XTRUE;
        XOS_SemPut(&(g_ntlCb.pTcpCliTsk[taskNo].taskSemp));

        /*�������ֲ�������*/
        XOS_MemSet(&tcpCliRead,0,sizeof(t_FDSET));
        XOS_MemSet(&tcpCliWrite,0,sizeof(t_FDSET));
        pReadSet = (t_FDSET*)XNULLP;
        pWriteSet = (t_FDSET*)XNULLP;


        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));  
        /*������ƿ�Ϊ��*/
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
            /*select ��ʱ*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else  /*select ����Ӧ�����������صģ������continue�������������أ�������ѭ��*/
            {    /*��������û��ʹ�û��⣬���п���fd���ر��ˣ�ȴ�ڲ���select�������쳣*/
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
                    /*�ͻ���δ������·����·��������*/
                    if( ((pTcpCliCb->linkState != eStateConnected)&&(pTcpCliCb->linkState != eStateConnecting))
                        ||(pTcpCliCb->sockFd.fd == XOS_INET_INV_SOCKFD))
                    {
                        g_NtlTest1++;
                        continue;
                    }

                    /*��д*/
                    if((pWriteSet != XNULLP) && XOS_INET_FD_ISSET(&(pTcpCliCb->sockFd),pWriteSet))
                    {
                        setNum--;        

                        /*ͬʱ�ɶ���д��˵���쳣*/
                        if(XOS_INET_FD_ISSET(&(pTcpCliCb->sockFd),pReadSet) || pTcpCliCb->sockFd.fd <= 0)
                        {
                            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_tcpCliRcvFunc(),tcp client fd = %d disconnect,try reconnect!",
                                                             pTcpCliCb->sockFd.fd);
                            /*�ض�ʱ��*/
                            XOS_TimerStop(FID_NTL,pTcpCliCb->timerId);
                            /*ֹͣ��·*/
                            NTL_closeTCli((XS32)taskNo,pTcpCliCb,NTL_SHTDWN_SEND);
                            /*֪ͨ״̬�ı�*/
                            NTL_noticeCloseTCli( pTcpCliCb);
                            /*���͹ر�ָʾ*/
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
                                
                            /*��write �������,�ȴ���ʱ�����ڽ��д���*/
                            XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));
                            
                            continue;
                        }

                        /*�� д �������*/
                        XOS_INET_FD_CLR(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.writeSet.fdSet));
                        /*��ӵ� �� ������*/
                        XOS_INET_FD_SET(&(pTcpCliCb->sockFd),&(g_ntlCb.pTcpCliTsk[taskNo].setInfo.readSet.fdSet));

                        /*�ͻ������ӳɹ�*/
                        if(pTcpCliCb->linkState != eStateConnected)
                        {
                            pTcpCliCb->linkState = eStateConnected;
                            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_tcpCliRcvFunc(),connect tcp server successed!");
                            NTL_StopTcpClientTimer(pTcpCliCb);

                            /*���������ɹ���Ϣ���ϲ�*/
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
                    /*�ɶ������쳣����ͨ�����case������*/
                    if((pReadSet != XNULLP) && XOS_INET_FD_ISSET(&(pTcpCliCb->sockFd),pReadSet))
                    {
                        setNum--;
                        pData = (XCHAR*)XNULLP;
                        len = XOS_INET_READ_ANY;

                        /*��������������*/
                        ret = XINET_RecvMsg(&(pTcpCliCb->sockFd),(t_IPADDR*)XNULLP,
                            &pData,&len,XOS_INET_STREAM,XOS_INET_CTR_COMPATIBLE_TCPEEN);
                        if(ret == XINET_CLOSE)
                        {
                            /* if tcp server closed then notify all this client*/
                            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_tcpCliRcvFunc(),tcp client disconnect,try reconnect!");
                            /*�ض�ʱ��*/
                            pTcpCliCb->linkState = eStateWaitClose;
                            NTL_StopTcpClientTimer(pTcpCliCb);

                            NTL_closeTCli((XS32)taskNo, pTcpCliCb, NTL_SHTDWN_SEND);
                            /*֪ͨ״̬�ı�*/
                            NTL_noticeCloseTCli( pTcpCliCb);
                            /*���͹ر�ָʾ*/
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
                            /*�������ݵ��ϲ�*/
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

                    /* �Ѿ�������еı���λ��socket*/
                    if(setNum <= 0)
                    {
                        break;
                    }
                }
            }
            if(setNum > 0) /*������δ��д������������һ��select��������*/
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
������:NTL_closeTsCli
����: �ر�һ��tcp server ����Ŀͻ���
����:pTcpServCli ��server client �Ŀ��ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/
XS32 NTL_closeTsCli(t_TSCLI* pTcpServCli)
{
    if(pTcpServCli == XNULLP)
    {
        return XERROR;
    }
    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client begin:serverClis sockNum=%d",g_ntlCb.tcpServTsk.setInfo.sockNum);
    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client begin:closing client sock=%d",pTcpServCli->sockFd.fd);

    /*�ر�sock ��������Դ*/

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

    /*�������жϿ�,�޸�server cb ������*/
    (pTcpServCli->pServerElem)->usageNum--;
    /*������β���Ľڵ�*/
    if(pTcpServCli->pNextCli == XNULLP)
    {
        (pTcpServCli->pServerElem)->pLatestCli = pTcpServCli->pPreCli;
    }
    else
    {
        pTcpServCli->pNextCli->pPreCli = pTcpServCli->pPreCli;
    }
    /*����ͷ�ڵ�*/
    if( pTcpServCli->pPreCli != XNULLP)
    {
        pTcpServCli->pPreCli->pNextCli = pTcpServCli->pNextCli;
    }

    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client end: serverClis  sockNum=%d",g_ntlCb.tcpServTsk.setInfo.sockNum);
    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client end: closed  client sock=%d",pTcpServCli->sockFd.fd);
    XOS_Trace(MD(FID_NTL,PL_INFO),"Tcp server close client end: current client num=%d.\r\n",(pTcpServCli->pServerElem)->usageNum);

    /*��hash ��ɾ��*/
    XOS_HashDelByElem(g_ntlCb.tSerCliH, pTcpServCli);

    return XSUCC;
}


/************************************************************************
������:NTL_closeReqProc
����:  ������·�ر�������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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

    /*���е���Ϣ��Ҫ������ĵ���Ч��*/
    if(!NTL_isValidLinkH(pCloseReq->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�Ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_closeReqProc()->FID[%d] linkHandle invalid!", pMsg->datasrc.FID);
        return XERROR;
    }
    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pCloseReq->linkHandle);
    XOS_Trace(MD(FID_NTL,PL_DBG),"NTL_closeReqProc()->linkType:%d,linkHandle:%x!",linkType,pCloseReq->linkHandle);

    switch(linkType)
    {
    case eUDP:
        /*��ȡudp ���ƿ�*/
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(pUdpCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_closeReqProc()->FID[%d] get the udp control block failed!", pMsg->datasrc.FID);
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return XERROR;
        }
        
        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pCloseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.udpTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_closeReqProc()->get the udp taskno failed!");
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return XERROR;
        }

        /*�����·״̬*/
        if(pUdpCb->linkState != eStateStarted)
        {
            /*��·״̬����,��Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_closeReqProc()->udp link state is not started!");
            ret = (pUdpCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return ret;
        }

        
        /*�ر�socket�����fdset*/
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
        /*�ı���·״̬����ʼ��״̬*/
        pUdpCb->linkState = eStateInited;
        pUdpCb->peerAddr.ip = 0;
        pUdpCb->peerAddr.port = 0;        
        pUdpCb->myAddr.ip = 0;
        pUdpCb->myAddr.port = 0;
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));

        break;

    case eTCPClient:
        /*��ȡtcp ���ƿ�*/
        pTcpCliCb = (t_TCCB*)XNULLP;

        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));    
        pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        
        
        if(pTcpCliCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_closeReqProc()->get the tcp client control block failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            return XERROR;
        }

        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pCloseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_closeReqProc()->get the tcp client taskno failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            return XERROR;
        }

        /*���������رգ�Ҫ��ʱ����*/
        if(pMsg->datasrc.FID == FID_NTL)
        {
            /*��ʼ��*/
            pTcpCliCb->linkState = eStateInited;
            pTcpCliCb->expireTimes = 0;

            /*������ʱ������ʱ��*/            
            XOS_MemSet(&timerParam,0,sizeof(t_PARA));
            timerParam.fid = FID_NTL;
            timerParam.len = TCP_CLI_RECONNECT_INTERVAL;
            timerParam.mode = TIMER_TYPE_LOOP;
            timerParam.pre = TIMER_PRE_LOW;

            XOS_MemSet(&backPara,0,sizeof(t_BACKPARA));
            backPara.para1 = (XPOINT)pTcpCliCb;
            backPara.para2 = (XPOINT)taskNo;

            /*�����ʱ����������յ��µķ���������*/
            NTL_dataReqClear(&(pTcpCliCb->packetlist));/*20080818 added,�����·֮ǰ���յ������ݰ�*/

            XOS_INIT_THDLE (pTcpCliCb->timerId);
            XOS_TimerStart(&(pTcpCliCb->timerId),&timerParam, &backPara);
        }
        else /*�û�����ر�*/
        {
            NTL_StopTcpClientTimer(pTcpCliCb);          

            /*�������������رպ��û������رգ�����Ҫ�ٹرգ��ر�fd, ������ݣ����ö�д��*/
            if(pTcpCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
            {
                NTL_closeTCli(taskNo, pTcpCliCb, NTL_SHTDWN_SEND);
            }
            else
            {
                /*�����ʱ����������յ��µķ���������*/
                NTL_dataReqClear(&(pTcpCliCb->packetlist));/*20080818 added,�����·֮ǰ���յ������ݰ�*/
            }

            pTcpCliCb->linkState = eStateInited;
            pTcpCliCb->expireTimes = 0;
            /*���Զ˵�ַ�ÿ�*/
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
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_closeReqProc()->get the tcp server control block failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return XERROR;
        }
        if(pTcpServCb->linkState != eStateListening)
        {
            /*��·״̬����,��Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_closeReqProc()->tcp serv link [linkhandle:%x] state [%d] is wrong!"
            ,pTcpServCb->linkHandle,pTcpServCb->linkState);
            ret = (pTcpServCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return ret;
        }           
       
        
        if(0x00 == pCloseReq->cliAddr.ip && 0x00 == pCloseReq->cliAddr.port)
        {
            /*���ȹر����н���Ŀͻ���*/     
            while(pTcpServCb->pLatestCli != XNULLP)
            {
                NTL_closeTsCli(pTcpServCb->pLatestCli);
                if(pTcpServCb->usageNum == 0)
                {
                    break;
                }
            }

            /*�ر�listen ��fd*/
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

            /*�ر�fd*/
            XINET_CloseSock(&(pTcpServCb->sockFd));
            
            /*�ı���Ӧ��cb ����,��ʼ��*/
            pTcpServCb->authFunc = (XOS_TLIncomeAuth)XNULL;
            pTcpServCb->linkState = eStateInited;
            pTcpServCb->maxCliNum = 0;
            pTcpServCb->usageNum = 0;
            pTcpServCb->pLatestCli = (t_TSCLI*)XNULLP;
        }
        else
        {   

            /*����ָ���Ŀͻ��˿��ƿ�*/
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
������:NTL_closeReqForRelease
����:  ������·�ͷ�������Ϣ--ֻ����NTL_linkReleaseProc����
����:  pMsg ����Ϣָ��
���:
����:  �ɹ�����XSUCC,���򷵻�XERROR
˵��:  �˺�����NTL_closeReqProc������tcpserver�ϴ���ͬ
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

    /*���е���Ϣ��Ҫ������ĵ���Ч��*/
    if(!NTL_isValidLinkH(pReleaseReq->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�Ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ReleaseLink()->FID[%d] linkHandle invalid!", pMsg->datasrc.FID);
        return XERROR;
    }
    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pReleaseReq->linkHandle);

    switch(linkType)
    {
    case eUDP:
        /*��ȡudp ���ƿ�*/
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH,NTL_getLinkIndex(pReleaseReq->linkHandle));
        if(pUdpCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ReleaseLink()->FID[%d] get the udp control block failed!", pMsg->datasrc.FID);
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return XERROR;
        }
        
        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pReleaseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.udpTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ReleaseLink()->get the udp taskno failed!");
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return XERROR;
        }

        /*�����·״̬*/
        if(pUdpCb->linkState != eStateStarted)
        {
            /*��·״̬����,��Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_ReleaseLink()->udp link state is not started!");
            ret = (pUdpCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            return ret;
        }

        
        /*�ر�socket�����fdset*/
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
        /*�ı���·״̬����ʼ��״̬*/
        pUdpCb->linkState = eStateInited;
        pUdpCb->peerAddr.ip = 0;
        pUdpCb->peerAddr.port = 0;        
        pUdpCb->myAddr.ip = 0;
        pUdpCb->myAddr.port = 0;
        XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));

        break;

    case eTCPClient:
        /*��ȡtcp ���ƿ�*/
        pTcpCliCb = (t_TCCB*)XNULLP;

        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));        
        pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,NTL_getLinkIndex(pReleaseReq->linkHandle));
        
        if(pTcpCliCb == XNULLP)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_ReleaseLink()->get the tcp client control block failed!");
            return XERROR;
        }

        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pReleaseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
        {
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_ReleaseLink()->get the tcp client taskno failed!");
            return XERROR;
        }

		NTL_StopTcpClientTimer(pTcpCliCb);          

		/*�������������رպ��û������رգ�����Ҫ�ٹرգ��ر�fd, ������ݣ����ö�д��*/
		if(pTcpCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
		{
			NTL_closeTCli(taskNo, pTcpCliCb, NTL_SHTDWN_SEND);
		}
		else
		{
			/*�����ʱ����������յ��µķ���������*/
			NTL_dataReqClear(&(pTcpCliCb->packetlist));/*20080818 added,�����·֮ǰ���յ������ݰ�*/
		}

		pTcpCliCb->linkState = eStateInited;
		pTcpCliCb->expireTimes = 0;
		/*���Զ˵�ַ�ÿ�*/
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
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_ReleaseLink()->get the tcp server control block failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return XERROR;
        }
        if(pTcpServCb->linkState != eStateListening)
        {
            /*��·״̬����,��Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_ReleaseLink()->tcp serv  link state is wrong!");
            ret = (pTcpServCb->linkState == eStateInited)? XSUCC:XERROR;
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            return ret;
        }

        /*�ر�tcp�����*/
        ret = NTL_CloseTcpServerSocket(pTcpServCb);
        if( ret != XSUCC)
        {
            g_tcpSerCloseFail++;
        }
        
        /*�ı���Ӧ��cb ����,��ʼ��*/
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
* ����: �����ÿ��hashԪ��ͨ�õĺ���
* ����  :
* hHash   - ������
* elem    - Ԫ��
* param   - ����
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
    /* ���*/
    if(XOS_INET_FD_ISSET(&(pTservCli->sockFd),pReadSet))
    {
        /*��������������*/
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

        /*�������ݵ��ϲ��û�*/
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
������:NTL_tcpServerDataCheckHash
����:  �����ͻ���fd��Ϣ
����:
���:
����:
˵��:
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
������:NTL_tcpServerDataCheck
����:  ��¼tcpserver�Ĵ�������Ϣ
����:
���:
����:
˵��:
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
    
    /*��ӡ��д��*/
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

    /*��ӡtcpserver sock fd����*/
    XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), 
    "\r\n\r\ntcpserver socket fd count = %d\r\n\r\n", g_ntlCb.tcpServTsk.setInfo.sockNum);
    pTmp += XOS_StrLen(pTmp); 

    /*��������˿��ƿ�*/
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
    
    /*���������ⲿ�ͻ���*/
    XOS_Sprintf(pTmp, sizeof(szBuf) - XOS_StrLen(szBuf), "\r\n\r\ntcpserver client info:");
    pTmp += XOS_StrLen(pTmp); 

    write_to_syslog("%s", szBuf);

    XOS_HashWalk(g_ntlCb.tSerCliH, NTL_tcpServerDataCheckHash, NULL);

    XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));

    /*��ӡ�ļ�������*/
    pid = getpid();
    memset(szBuf, 0, sizeof(szBuf));
    XOS_Sprintf(szCmd, sizeof(szCmd),"ls /proc/%d/fd", pid);
    XOS_ExeCmd(szCmd, szBuf, sizeof(szBuf)-1);

    write_to_syslog("\r\nprocess %d file des below: \r\n%s", pid, szBuf); 
#endif

}


/************************************************************************
������:NTL_tcpServTsk
����:  tcp server listening function
����:taskNo  �����
���:
����:
˵��:
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
        /*�̸߳�����ʱ����û��sockfd��fdΪ��ʱselectֱ�ӷ���*/
        g_ntlCb.tcpServTsk.activeFlag = XFALSE;
        XOS_SemGet(&(g_ntlCb.tcpServTsk.taskSemp));
        g_ntlCb.tcpServTsk.activeFlag = XTRUE;
        XOS_SemPut(&(g_ntlCb.tcpServTsk.taskSemp));

        /*�������ֲ������У���ֹ�ƻ�ȫ�ֵĶ���*/
        XOS_MemSet(&readSet,0,sizeof(t_FDSET));
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        XOS_MemCpy(&readSet,&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet),sizeof(t_FDSET));
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));

        /*select ����*/
        setNum = 0;
        ret = XINET_Select(&readSet,(t_FDSET*) XNULLP,(XU32*)&pollAbideTime,(XS16*)&setNum);
        if(ret != XSUCC)
        {
            /*select ��ʱ*/
            if(ret == XINET_TIMEOUT)
            {
                continue;
            }
            else /*��������*/
            {                
                XOS_Trace(MD(FID_NTL,PL_INFO),"tcpServer task,select readset failed,return setNum =%d.",setNum);
                #if 0
                /*�����ִ���ʱ����¼���е�״̬��Ϣ*/
                NTL_tcpServerDataCheck();
                #endif
                continue;
            }
        }
        if(setNum > 0)
        {
            XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
            /*�пͻ������ӽ���*/
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
                    /*�Ƚ���*/
                    ret = XINET_Accept(&(pTcpServCb->sockFd),&fromAddr,&servCliFd);
                    if(ret != XSUCC)
                    {
                        XOS_Trace(MD(FID_NTL,PL_WARN),"tcpServer task,accept fd %d failed!",pTcpServCb->sockFd.fd);
                        continue;
                    }

                    /*��Ҫ������֤,������֤���ɹ���*/
                    if(pTcpServCb->authFunc != XNULLP
                        && !(pTcpServCb->authFunc(pTcpServCb->userHandle,&fromAddr,pTcpServCb->pParam)))
                    {
                        /*�ر��½����sock*/
                        XINET_CloseSock(&servCliFd);
                        XOS_Trace(MD(FID_NTL,PL_ERR),"tcpServer task,get client(ip[0x%x],port[%d]) auth failed!",
                            XOS_INET_NTOH_U32(fromAddr.ip),XOS_INET_NTOH_U16(fromAddr.port));
                        continue;
                    }

                    /*����ͻ����������ܳ�����������������*/
                    if(pTcpServCb->maxCliNum < pTcpServCb->usageNum +1)
                    {
                        /*�ر��½����sock*/
                        XINET_CloseSock(&servCliFd);
                        XOS_Trace(MD(FID_NTL,PL_ERR),"tcpServer task,new accept client(ip[0x%x],port[%d]) is overstep maxclis %d allowned!",
                            XOS_INET_NTOH_U32(fromAddr.ip),XOS_INET_NTOH_U16(fromAddr.port),pTcpServCb->maxCliNum);
                        continue;
                    }
                    /*������֤����Ϊ�գ���ζ�Ų���Ҫ��֤*/
                    /*������Ŀͻ��˼ӵ�hash����*/
                    XOS_MemSet(&servCliCb,0,sizeof(t_TSCLI));
                    XOS_MemCpy(&(servCliCb.sockFd),&servCliFd,sizeof(t_XINETFD));
                    /*����ҲӦ�ñ���*/
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

                    /*������*/
                    pServCliCb = (t_TSCLI*)XNULLP;
                    pServCliCb = (t_TSCLI*)XOS_HashGetElem(g_ntlCb.tSerCliH, pLocation);
                    
                    if(pTcpServCb->pLatestCli != XNULLP && pServCliCb != XNULLP)
                    {
                        pTcpServCb->pLatestCli->pNextCli = pServCliCb;
                    }
                    pTcpServCb->pLatestCli= pServCliCb;
                    pTcpServCb->usageNum++; /*����һ���µĿͻ�����*/
                    
                    /*���뵽read ����*/
                    XOS_INET_FD_SET(&servCliFd,&( g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));
#ifndef XOS_WIN321
                    g_ntlCb.tcpServTsk.setInfo.sockNum++;
#endif

                    /*��������ָʾ��Ϣ���ϲ�*/
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

            /*�����ݽ���*/
            XOS_HashWalk(g_ntlCb.tSerCliH,NTL_pollingHash,&readSet);
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
       }
    }
}


/************************************************************************
������:NTL_genCfgProc
����:  ����ͨ��������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: ntl ���յ�ͨ��������Ϣ����������������ɸ�
������ĳ�ʼ����
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

    /*����������Ϣ*/
    XOS_MemCpy(&(g_ntlCb.genCfg),pGenCfg,sizeof(t_NTLGENCFG));

    /*ȷ��һ���̼߳��ӵ�����������*/
    if(pGenCfg->fdsPerThrPolling > 0 )
    {
        pollingNum = pGenCfg->fdsPerThrPolling;
    }
    else
    {
        pollingNum = FDS_PER_THREAD_POLLING;  /*Ĭ��һ���̼߳���256 ��������*/
        g_ntlCb.genCfg.fdsPerThrPolling = pollingNum;
    }

    /*ȷ�������񼯵�����*/
    /*udp tasks */
    g_ntlCb.udpTskNo = (pGenCfg->maxUdpLink%pollingNum)
        ?(pGenCfg->maxUdpLink/pollingNum + 1)
        :(pGenCfg->maxUdpLink/pollingNum);

    /*tcp cli tasks */
    g_ntlCb.tcpCliTskNo = (pGenCfg->maxTcpCliLink%pollingNum)
        ?(pGenCfg->maxTcpCliLink/pollingNum + 1)
        :(pGenCfg->maxTcpCliLink/pollingNum);

    /*��һ����ȫ�Լ��*/
    /*udp��tcpcli  ���õ����񳬹����������,˵��ͨ�����ò�����*/
    if(g_ntlCb.udpTskNo > MAX_UDP_POLL_THREAD
        || g_ntlCb.tcpCliTskNo > MAX_TCP_CLI_POLL_THREAD)
    {
        XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_genCfgProc()->gen cfg is not reasonable!");
        return XERROR;
    }

    /*����������ص���Դ*/
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

    /*udp link �����������*/
    if (g_ntlCb.udpTskNo > 0)
    {
        /*����udp��·���ƿ���Դ*/
        g_ntlCb.udpLinkH = XNULL;
        g_ntlCb.udpLinkH =  XOS_ArrayConstruct(sizeof(t_UDCB), pGenCfg->maxUdpLink, "udplinkH");
        if(XNULL == g_ntlCb.udpLinkH)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->construct udp array failed!");
            goto genCfgError;
        }
        XOS_ArraySetCompareFunc(g_ntlCb.udpLinkH, NTL_channel_find_function);

        /*udp��·���ƿ黥�����*/
        ret = XOS_MutexCreate(&(g_ntlCb.udpLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> udp thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        } 

        /*��������*/
        for (i = 0; i < g_ntlCb.udpTskNo; i++)
        {        

            /*����������ź���*/
            ret = XOS_SemCreate(&(g_ntlCb.pUdpTsk[i].taskSemp),  0);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> udp thread[%d]  semaphore  failed!",i);
                goto genCfgError;
            }           

            /*read set ������*/
            ret = XOS_MutexCreate(&(g_ntlCb.pUdpTsk[i].setInfo.fdSetMutex));
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> udp thread mutex[%d]  failed!",i);
                goto genCfgError;
            }

            /*��ʼ��read set*/
            XOS_INET_FD_ZERO(&(g_ntlCb.pUdpTsk[i].setInfo.readSet.fdSet));
            XOS_INET_FD_ZERO(&(g_ntlCb.pUdpTsk[i].setInfo.writeSet.fdSet));

            /*��ʼ����ǰ�����ڵ�socket����*/
            g_ntlCb.pUdpTsk[i].setInfo.sockNum = 0;

             /*����udp����*/
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

    /* tcp client  link �����������*/
    if (g_ntlCb.tcpCliTskNo > 0 )
    {
        /*����tcp client���ƿ���Դ*/
        g_ntlCb.tcpClientLinkH = XNULL;
        g_ntlCb.tcpClientLinkH =  XOS_ArrayConstruct(sizeof(t_TCCB), pGenCfg->maxTcpCliLink, "tcpClilinkH");
        if(XNULL == g_ntlCb.tcpClientLinkH)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->construct tcpCli array failed!");
            goto genCfgError;
        }
        XOS_ArraySetCompareFunc(g_ntlCb.tcpClientLinkH, NTL_channel_find_function);

        /*tcp�ͻ�����·���ƿ黥�����*/
        ret = XOS_MutexCreate(&(g_ntlCb.tcpClientLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcpclient thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        } 

        /*��������*/
        for (i=0; i<g_ntlCb.tcpCliTskNo; i++)
        {
            /*����������ź���*/
            ret = XOS_SemCreate(&(g_ntlCb.pTcpCliTsk[i].taskSemp), 0);
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcpCli thread[%d]  semaphore  failed!",i);
                goto genCfgError;
            }           

            /* fd read write set ������*/
            ret = XOS_MutexCreate(&(g_ntlCb.pTcpCliTsk[i].setInfo.fdSetMutex));
            if(ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcp cli  thread fdset mutex[%d]  failed!",i);
                goto genCfgError;
            }

            /*��ʼ�� set*/
            XOS_INET_FD_ZERO(&(g_ntlCb.pTcpCliTsk[i].setInfo.readSet.fdSet));
            XOS_INET_FD_ZERO(&(g_ntlCb.pTcpCliTsk[i].setInfo.writeSet.fdSet));

             /*����tcp client����*/
            XOS_MemSet(taskName,0,NTL_TSK_NAME_LEN);
            XOS_Sprintf(taskName, sizeof(taskName)-1, "Tsk_tcpc%d", i);
            ret = XOS_TaskCreate(taskName,TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)NTL_tcpCliRcvFunc,
                (XVOID *)(XPOINT)i,&(g_ntlCb.pTcpCliTsk[i].taskId));
            if (ret != XSUCC)
            {
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcpCli thread[%d]  failed!",i);
                goto genCfgError;
            }

            /*��ʼ����ʱ��*/
            /*to do */
        }
    }

    /*tcp server ��ص�����,tcp serverĿǰ���õ�������,�Ժ������չ*/
    if(pGenCfg->maxTcpServLink > 0)
    {
        /*����tcp server ���ƿ�����*/
        g_ntlCb.tcpServerLinkH = XNULL;
        g_ntlCb.tcpServerLinkH = XOS_ArrayConstruct( sizeof(t_TSCB),pGenCfg->maxTcpServLink,"tcpServH");
        if(XNULL == g_ntlCb.tcpServerLinkH )
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->construct tcpServ array failed!");
            goto genCfgError;
        }
 
        XOS_ArraySetCompareFunc(g_ntlCb.tcpServerLinkH, NTL_channel_find_function);

        /*tcp�������·���ƿ黥�����*/
        ret = XOS_MutexCreate(&(g_ntlCb.tcpServerLinkMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcpserver thread[%d]  semaphore  failed!",i);
            goto genCfgError;
        }

        /*��������ͻ��˵�hash*/
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

        /*��ʼ��hash �Ļ�����*/
        ret = XOS_MutexCreate(&(g_ntlCb.hashMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()->create  mutex for hash failed!");
            goto genCfgError;
        }
        
        /*����������ź���*/
        ret = XOS_SemCreate(&(g_ntlCb.tcpServTsk.taskSemp),0);
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tServ thread entry semaphore  failed!");
            goto genCfgError;
        }        

        /*read set ������*/
        ret = XOS_MutexCreate(&(g_ntlCb.tcpServTsk.setInfo.fdSetMutex));
        if(ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_genCfgProc()-> tcp serv thread mutex failed!");
            goto genCfgError;
        }

        /*��ʼ�� set*/
        XOS_INET_FD_ZERO(&(g_ntlCb.tcpServTsk.setInfo.readSet.fdSet));
        XOS_INET_FD_ZERO(&(g_ntlCb.tcpServTsk.setInfo.writeSet.fdSet));

        /*����tcp server ������*/
        XOS_Sprintf(taskName, sizeof(taskName)-1, "Tsk_tcps");
        ret = XOS_TaskCreate(taskName,TSK_PRIO_NORMAL,NTL_RECV_TSK_STACK_SIZE,(os_taskfunc)NTL_tcpServTsk,
            (XVOID *)0,&(g_ntlCb.tcpServTsk.taskId));
    }

    /*sctp pci ��صĳ�ʼ������չ*/
    g_ntlCb.isGenCfg = XTRUE;
    return XSUCC;

genCfgError:
    /*�ͷ���Դ*/
    XOS_ArrayDestruct(g_ntlCb.udpLinkH);
    XOS_ArrayDestruct(g_ntlCb.tcpClientLinkH);
    XOS_ArrayDestruct(g_ntlCb.tcpServerLinkH);
    XOS_HashDestruct(g_ntlCb.tSerCliH);

    /*��ֹ�����ͷ���*/
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

    /*�ͷ��ڴ�*/
    if(g_ntlCb.pTcpCliTsk != XNULLP)
    {
        XOS_MemFree(FID_NTL, g_ntlCb.pTcpCliTsk);
    }
    if(g_ntlCb.pUdpTsk != XNULLP)
    {
        XOS_MemFree(FID_NTL,g_ntlCb.pUdpTsk);
    }

    /*���*/
    XOS_MemSet(&g_ntlCb,0,sizeof(t_NTLGLOAB));
    /*���ó�û�г�ʼ��*/
    g_ntlCb.isGenCfg = XFALSE;
    return XERROR;
}


/************************************************************************
������:NTL_linkInitProc
����:  ������·��ʼ����Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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
        
        /*�����Ƿ��ظ�����*/
        if(XERROR != nRtnFind)
        {
            /*������·*/
            NTL_ResetLinkByReapplyEntry(eUDP, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkInitProc()->add the udp cb was reset!");
        }
        else 
        {
            /*��ӵ�æ������*/
            linkIndex = XOS_ArrayAddExt(g_ntlCb.udpLinkH,(XOS_ArrayElement *) &pUdpCb);
            
            if((linkIndex >= 0) && (pUdpCb != XNULLP))
            {
                /*��ʼ�����ƿ����*/
                pUdpCb->userHandle = pLinkInit->appHandle;
                XOS_MemCpy(&(pUdpCb->linkUser),&(pMsg->datasrc),sizeof(t_XOSUSERID));
                pUdpCb->linkHandle = NTL_buildLinkH(eUDP,(XU16)linkIndex);
                pUdpCb->linkState = eStateInited;
                pUdpCb->myAddr.ip = 0;
                pUdpCb->myAddr.port = 0;
                pUdpCb->peerAddr.ip = 0;
                pUdpCb->peerAddr.port = 0;
                pUdpCb->sockFd.fd = XOS_INET_INV_SOCKFD;                

                /*�ظ�eLinkInitAck�Ĳ���*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle = pUdpCb->linkHandle;
            }
            else
            {
                /*�ظ���·ȷ��ʧ��*/
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
            /*������·*/
            NTL_ResetLinkByReapplyEntry(eTCPClient, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkInitProc()->add the tcp client cb was reset!");
            
        }
        else
        {
            /*��ӵ�æ������*/
            linkIndex = XOS_ArrayAddExt(g_ntlCb.tcpClientLinkH,(XOS_ArrayElement *) &pTcpCliCb);
            
            if((linkIndex >= 0) && (pTcpCliCb != XNULLP))
            {
                /*��ʼ�����ƿ����*/
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
                

                /*�ظ�eLinkInitAck�Ĳ���*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle= pTcpCliCb->linkHandle;
            }
            else
            {
                /*�ظ���·ȷ��ʧ��*/
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
            /*������·*/
            NTL_ResetLinkByReapplyEntry(eTCPServer, &linkInitAck, &(pLinkInit->appHandle), &nRtnFind);
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkInitProc()->add the tcp server cb was reset!");
        }
        else
        {
            /*��ӵ�æ������*/
            linkIndex = XOS_ArrayAddExt(g_ntlCb.tcpServerLinkH,(XOS_ArrayElement *)&pTcpSrvCb);
            
            if((linkIndex >= 0) && (pTcpSrvCb != XNULLP))
            {
                /*��д���ƿ����*/
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

                /*�ظ�eLinkInitAck�Ĳ���*/
                linkInitAck.appHandle = pLinkInit->appHandle;
                linkInitAck.lnitAckResult = eSUCC;
                linkInitAck.linkHandle= pTcpSrvCb->linkHandle;
            }
            else
            {
                /*�ظ���·ȷ��ʧ��*/
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
        /*�ظ���·ȷ��ʧ��*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkInitProc()->not support link type:%d!",pLinkInit->linkType);
        linkInitAck.appHandle = pLinkInit->appHandle;
        linkInitAck.lnitAckResult = eFAIL;
        break;
    }

    /*�ظ�initAck ��Ϣ*/
    ret = NTL_msgToUser(&linkInitAck,&(pMsg->datasrc),sizeof(t_LINKINITACK),eInitAck);

    
    /*�ظ���Ϣʧ�ܣ���Ԫ�ؽ�����ʹ�ã�Ӧ�����*/
    if((ret != XSUCC) && (linkInitAck.lnitAckResult == eSUCC))
    {
        /*�����Դ*/
        NTL_DeleteCB(pLinkInit->linkType, linkIndex);        
        return XERROR;
    }
    return XSUCC;
}

/************************************************************************
������:NTL_DeleteCB
����:  �ͷſ��ƿ�
����:  linkType ����·����
       linkIndex--��·����

���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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
������:NTL_linkReleaseProc
����:  ������·�ͷ���Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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
    
    /*�ȹر���·,���ͷ���Դ*/
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
        /*��·�����Ч����Ϣ��ֱ�Ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkReleaseProc()->linkHandle invalid!");
        return XERROR;
    }
    /*��ȡ��·����*/
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
            /*��ɾ�����н���Ŀͻ�*/
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

            /*�����server��Ӧ�Ŀ��ƿ�*/
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
������:NTL_tcpServStart
����:  ������·������Ϣ
����:pMsg ����Ϣָ��
���:pStartAck ������ȷ����Ϣ
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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

    /*��ȡtcp server ���ƿ�*/
    pTcpServCb = (t_TSCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpServerLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
    
    if(pTcpServCb == XNULLP)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->get tcp serv control block failed!");
        return XERROR;
    }

    /*�����·״̬����ز���*/
    if((pTcpServCb->linkState != eStateInited )
        ||(pTcpServStart->allownClients == 0)
        ||(pTcpServStart->allownClients > (XU32)((g_ntlCb.genCfg.maxTcpServLink)*TCP_CLIENTS_PER_SERV)))
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->link state [%s]  error or allownClients[%d] error!",NTL_getLinkStateName(pTcpServCb->linkState,szLinkStateName,sizeof(szLinkStateName)-1),pTcpServStart->allownClients);
        goto errorProc;
    }

    /*���״̬��ȷ����������ϴ����ǹر���*/
    NTL_CloseTcpServerSocket(pTcpServCb);    
    
    /*����sockect*/
    ret = XINET_Socket(XOS_INET_STREAM,&(pTcpServCb->sockFd));
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->create sockFd failed");
        goto errorProc;
    }

    /*�󶨶˿�*/
    ret =  XINET_Bind(&(pTcpServCb->sockFd),&(pTcpServStart->myAddr));
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->XINET Bind sockFd failed");
        goto errorProc;
    }
    /*ȷ�����˵�ַ*/
    XINET_GetSockName(&(pTcpServCb->sockFd),&(pTcpServCb->myAddr));

    /*listen*/
    ret = XINET_Listen(&(pTcpServCb->sockFd),MAX_BACK_LOG);
    if (ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_tcpServStart()->XINET_Listen sockFd %d failed",pTcpServCb->sockFd.fd);
        goto errorProc;
    }

    //printf("NTL_tcpServStart: Listen ok srcFid=%d, port=%d\n", pMsg->datasrc.FID, pTcpServStart->myAddr.port );

    /*��ӵ�read ����*/
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

    /*��д����*/
    pTcpServCb->authFunc = pTcpServStart->authenFunc;
    pTcpServCb->pParam = pTcpServStart->pParam;
    pTcpServCb->maxCliNum = pTcpServStart->allownClients;
    pTcpServCb->usageNum = 0;
    pTcpServCb->linkState = eStateListening;
    pTcpServCb->pLatestCli = (t_TSCLI*)XNULLP;

    /*��д�������*/
    pStartAck->appHandle = pTcpServCb->userHandle;
    pStartAck->linkStartResult = eSUCC;
    XOS_MemCpy(&(pStartAck->localAddr),&(pTcpServCb->myAddr),sizeof(t_IPADDR));
    
    return XSUCC;

errorProc:
    {
        /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
        pStartAck->appHandle = pTcpServCb->userHandle;
        pStartAck->linkStartResult = eFAIL;
        /*ɾ������Ŀ��ƿ�,2008/03/21 add below*/
        //XOS_ArrayDeleteByPos(g_ntlCb.tcpServerLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
        /*ɾ������Ŀ��ƿ�,2008/03/21 add above*/
        return XSUCC;
    }
}
/************************************************************************
������:NTL_StartTcpClientTimer
����:  ����tcp�ͻ��˵�������ʱ��
����:  pTcpCb ��tcp���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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
������:NTL_SetTcpClientFd
����:  tcp client select ��λ����
����:  pTcpCb ����Ϣָ��
       fdFlg  --0:��,1:д, 2:����д
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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
������:NTL_SetUdpSelectFd
����:  udp select ��λ����
����:  pUdpCb ��udp���ƿ�
       taskNo  --�����
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
************************************************************************/    
void NTL_SetUdpSelectFd(t_UDCB *pUdpCb, XU32 taskNo)
{
    if(!pUdpCb)
    {
        return;
    }
    
    /*��ӵ�readset��*/
#ifdef XOS_WIN321
    if (g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet.fd_count == 0)
    {
        XOS_INET_FD_SET(&(pUdpCb->sockFd),&(g_ntlCb.pUdpTsk[taskNo].setInfo.readSet.fdSet));
        /*��һ������udp����ʱ������udp���������ź�*/
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
        /*��һ������udp����ʱ������udp���������ź�*/
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
������:NTL_StartUdpLink
����:  ����udp��·
����:    pDatasrc ����ϢԴָ��
        pLinkStart--��·����ָ��

���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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

    /*udp һ����Ҫ�����ظ�startack ��Ϣ*/
    pUdpStart = &(pLinkStart->linkStart.udpStart);
    
    XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
    do{
        /*��ȡudp ���ƿ�*/
        pUdpCb = (t_UDCB*)XNULLP;
        
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH, NTL_getLinkIndex(pLinkStart->linkHandle));
        
        if(pUdpCb == XNULLP)
        {
           /*���ִ�������أ�û�а취�ظ�startAck ��Ϣ�����Խ���Ϣ����*/
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] get the udp control block failed!", pDatasrc->FID);
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
           return XERROR;
        }

        /*��ȡ��·���ڵ������*/
        taskNo = (XU32)NTL_getLinkIndex(pLinkStart->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.udpTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] taskNo %d is invalid", pUdpCb->linkUser.FID, taskNo);
            startAck.appHandle = pUdpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            break;
        }

        /*�����·��״̬�����Ƿ�Ϊ�ظ�����*/
        if(pUdpCb->linkState != eStateInited)
        {
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] the udp link state [%s] is wrong!",pUdpCb->linkUser.FID,
                                         NTL_getLinkStateName(pUdpCb->linkState, szLinkStateName, sizeof(szLinkStateName)-1));
           /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
           startAck.appHandle = pUdpCb->userHandle;
           startAck.linkStartResult = eFAIL;
           break;
        }

        /*���fd��ȷ����رգ��������ϴ����ǹر���*/
        if (pUdpCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
           /*�ر�socket�����fdset*/
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] pUdpCb->sockFd.fd %d forget close", pUdpCb->linkUser.FID,
                                        pUdpCb->sockFd.fd);
           NTL_CloseUdpSocket(pUdpCb, taskNo);
        }   

        /*��������sock*/
        ret = XINET_Socket(XOS_INET_DGRAM, &(pUdpCb->sockFd));
        if (ret != XSUCC)
        {
           /*��дstartAck �ظ���Ϣ,���ؽ��Ϊfailed*/
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] XINET_Socket failed", pUdpCb->linkUser.FID);
           startAck.appHandle = pUdpCb->userHandle;
           startAck.linkStartResult = eFAIL;
           break;
        }

        ret = XINET_Bind(&(pUdpCb->sockFd), &(pUdpStart->myAddr));
        if (ret != XSUCC)/*�󶨲��ɹ�,�����Ƕ˿��Ѿ�ʹ����*/
        {
           /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
           XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_StartUdpLink()->FID[%d] XINET_Bind failed", pUdpCb->linkUser.FID);
           startAck.appHandle = pUdpCb->userHandle;
           startAck.linkStartResult = eFAIL;
           break;
        }

        /*�޸�udp���ƿ���Ϣ*/
        pUdpCb->linkState = eStateStarted;
        XINET_GetSockName(&(pUdpCb->sockFd),&(pUdpCb->myAddr));
        XOS_MemCpy(&(pUdpCb->peerAddr), &(pUdpStart->peerAddr), sizeof(t_IPADDR));      

        /*���뵽read set*/
        NTL_SetUdpSelectFd(pUdpCb, taskNo);

        /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
        startAck.appHandle = pUdpCb->userHandle;
        XOS_MemCpy(&(startAck.localAddr),&(pUdpCb->myAddr),sizeof(t_IPADDR));
        startAck.linkStartResult = eSUCC;
        break;
        
    }while(0);
    XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));


    /*����startAck ��Ϣ���ϲ�*/
    ret = NTL_msgToUser(&startAck, pDatasrc, sizeof(t_STARTACK), eStartAck);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_StartUdpLink()->send  msg startAck to user failed!");
        return XERROR;
    }

    return XSUCC;

}


/************************************************************************
������:NTL_linkStarttProc
����:  ������·������Ϣ
����:pMsg ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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

    /*���е�������Ϣ(elinkInit ����)��Ҫ��֤��·�������Ч�ԣ�
    �Է�ֹ������޸ĵ�����������*/
    if(!NTL_isValidLinkH(pLinkStart->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�Ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStarttProc()->FID[%d] linkHandle[%d] invalid!", pMsg->datasrc.FID,
                                     pLinkStart->linkHandle);
        return XERROR;
    }

    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pLinkStart->linkHandle);
    XOS_MemSet(&startAck,0,sizeof(t_STARTACK));
    XOS_Trace(MD(FID_NTL,PL_DBG),"NTL_linkStarttProc()->linkType:%d,linkHandle:%x!",linkType,pLinkStart->linkHandle);
    switch (linkType)
    {
    case eUDP:

        return NTL_StartUdpLink(&(pMsg->datasrc), pLinkStart);       

    case eTCPClient:
        pTcpCliStart = &(pLinkStart->linkStart.tcpClientStart);

        /*��ȡtcp client  ���ƿ�*/
        pTcpCb = (t_TCCB*)XNULLP;
        
        XOS_MutexLock(&(g_ntlCb.tcpClientLinkMutex));
        pTcpCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,NTL_getLinkIndex(pLinkStart->linkHandle));
        
        
        if(pTcpCb == XNULLP)
        {
            /*���ִ�������أ�û�а취�ظ�startAck ��Ϣ�����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->get the tcp client control block failed!");
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            return XERROR;
        }

         /*��ȡ��·���������*/
         taskNo = (XU32)NTL_getLinkIndex(pLinkStart->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
         if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
         {
              XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client taskNo %d is invalid!",pTcpCb->linkUser.FID, taskNo);
             startAck.appHandle = pTcpCb->userHandle;
             startAck.linkStartResult = eFAIL;
             XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
             break;
         }                 

         /*�����·��״̬�����Ƿ�Ϊ�ظ�����*/
         if(pTcpCb->linkState != eStateInited)
         {
             XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] the tcp client link state [%s] is wrong!",
                                           pTcpCb->linkUser.FID, NTL_getLinkStateName(pTcpCb->linkState, szLinkStateName, sizeof(szLinkStateName)-1));
             /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
             startAck.appHandle = pTcpCb->userHandle;
             startAck.linkStartResult = eFAIL;
             XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
             break;
         }


           /*״̬��ȷ��fdȴ�򿪣���Ҫ�رգ��������ϴ����ǹر���*/
        if (pTcpCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client forget close old fd!",pTcpCb->linkUser.FID);
            NTL_closeTCli(taskNo, pTcpCb, NTL_SHTDWN_SEND);
        }     
        

        /*��������sock*/
        ret = XINET_Socket(XOS_INET_STREAM,&(pTcpCb->sockFd));
        if (ret != XSUCC)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client socket failed!",pTcpCb->linkUser.FID);
            /*��дstartAck �ظ���Ϣ,���ؽ��Ϊefailed*/
            startAck.appHandle = pTcpCb->userHandle;
            startAck.linkStartResult = eFAIL;
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            break;
        }


        /*����ҵ������linger*/
          if (pTcpCb->linkUser.FID != FID_FTP)
        {
            optVal = XOS_INET_OPT_ENABLE;
            XINET_SetOpt(&pTcpCb->sockFd, SOL_SOCKET, XOS_INET_OPT_LINGER, (XU32 *)&optVal);            
        }
        
        /*����tcp �ͻ��˵ķ��񣬲�care �󶨵Ľ��*/
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

        /*���ӵ��Զ�*/
        ret = XINET_Connect(&(pTcpCb->sockFd),&(pTcpCliStart->peerAddr));
        if(ret != XSUCC)
        {

            pTcpCb->linkState = eStateConnecting;
            XOS_IptoStr(pTcpCliStart->peerAddr.ip,szTemp);
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_linkStartProc(),FID[%d] tcp client the %dth connect[%s:%d] failed ,start timer for pTcpCb[0x%x] reconnect!",
                pTcpCb->linkUser.FID,pTcpCb->expireTimes ,szTemp,pTcpCliStart->peerAddr.port,pTcpCb);

            /*������Ӳ��ɹ�,��������,������ʱ��,��֤��·����*/            
            if(XSUCC != NTL_StartTcpClientTimer(pTcpCb, taskNo))
            {
                /*��ʱ������ʧ�ܴ�������*/
                XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_linkStartProc()->FID[%d] tcp client start timer failed!",pTcpCb->linkUser.FID);
                startAck.appHandle = pTcpCb->userHandle;
                startAck.linkStartResult = eFAIL;
                XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
                break;
            }

            /*��ӵ�writeset��*/
            NTL_SetTcpClientFd(pTcpCb, taskNo, eWrite);
            
            startAck.appHandle = pTcpCb->userHandle;
            XOS_MemCpy(&(startAck.localAddr),&(pTcpCb->myAddr),sizeof(t_IPADDR));
            startAck.linkStartResult = eBlockWait;     
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));       
            break;
        }

        /*���������ӳɹ��Ŀ����Ժ�С��Ҫ�����ӱ����Ķ˿ڣ�һ���ǿ������ӳɹ���*/
        pTcpCb->linkState = eStateConnected;
        NTL_StopTcpClientTimer(pTcpCb);

        /*��ӵ�readset��*/
        NTL_SetTcpClientFd(pTcpCb, taskNo, eRead);

        /*��дstartAck �ظ���Ϣ,���ؽ��Ϊ�ɹ�*/
        startAck.appHandle = pTcpCb->userHandle;
        XOS_MemCpy(&(startAck.localAddr), &(pTcpCb->myAddr), sizeof(t_IPADDR));
        startAck.linkStartResult = eSUCC;
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));

        break;

    case eTCPServer:
        /*tcp server �������Ƚϸ��ӣ�����һ����������*/
        XOS_MutexLock(&(g_ntlCb.tcpServerLinkMutex));
        ret = NTL_tcpServStart( pMsg,&startAck);
        XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
        if(ret == XERROR)
        {
            /*���ش���Ͳ��ûظ�start Ack ��Ϣ*/
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

    /*����startAck ��Ϣ���ϲ�*/
    ret = NTL_msgToUser(&startAck,&(pMsg->datasrc),sizeof(t_STARTACK),eStartAck);
    if(ret != XSUCC)
    {
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_linkStartProc()->send  msg startAck to user failed!");
        return XERROR;
    }

    return XSUCC;
}


/*
���ŶӶ��в����,ֱ�����ͳɹ�Ϊֹ
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
�ýӿ�ֻ���������ݰ��������ȫ��
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
    /*out_len���ݰ�����*[0,msglen)֮��
    0��ʾû���ͳɹ�
    msglen��ʾ���ͳɹ�
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
        /*����ŶӶ�����,��������Ϣ������*/
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
    /*���µ����ݰ��ӵ�������*/
    XOS_MemSet(ptmpPacket,0x0,sizeof(t_RsndData));
    ptmpPacket->pNextPacket=XNULL;
    /*�����ݰ���ַ������������*/
    ptmpPacket->pData=pDataReq->pData;
    ptmpPacket->msgLenth = pDataReq->msgLenth;
    ptmpPacket->i_offset = out_len;

    /*����ҲӦ�ñ���*/

    /*������*/
    /*��ʼ��ʱ��Ҫ��ȫ0x0,�ڴ���ʱ����ʵ��8888*/
    pPacklist->rsnd_wait++;
    if(pPacklist->pFirstDataReq == XNULL)
    {
        /*��Ϊͷ�ڵ�*/
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
/*ֻ���ŶӶ��з���*/
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

            /*����ǰ�����ͳɹ�������Ҫ�ƶ��α�*/
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
    /*��������*/
    ret = XOS_MsgSend(msgToNtl);
    if(ret != XSUCC)
    {
        /*������Ϣ����Ӧ�ý���Ϣ�ڴ��ͷ�*/
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
    t_IPADDR *pPeerAddr = XNULL;/*�ò�����Ч*/
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
            /*�ȼ����·��״̬*/
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


/*��Ϊ����·����,���Է��ͽ����֪ͨ�û�*/
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
        /*û���Ŷ�,��������*/
        out_len=0;
        ret = XINET_SendMsg(pSockFd, eTCPClient, pIpAddr,pDataReq->msgLenth, pDataReq->pData, &out_len);
        if((ret != XSUCC) || (out_len > 0 && out_len < (XS32)pDataReq->msgLenth))
        {
            /*��������������������,�Ŷ�,����������ڰ�������,�ͻ᲻����*/
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
                /*�Ŷӳɹ�*/
                return XSUCC;
            }
            /*����*/
            pklist->rsnd_delete++;
            /*��Ϊ��ͳ�Ƽ���,���в�֪ͨ�û�����ʧ��*/
            /*�Ŷ�ʧ��,�����ݱ�����*/
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
            /*���ͳɹ���������ݰ�*/
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
        /*���Ŷ�*/
        out_len=0;
        if(XSUCC == NTL_dataReqSaveProc(pklist,pDataReq,out_len))
        {
            if(g_ntltraceswitch)
            {
                XOS_Trace(MD(FID_NTL,PL_INFO),"NTL DataReq tpkt append,resend wait packet size %d",pklist->rsnd_size);
            }
            /*�ط������ɹ��ȴ�,ֱ���ɹ�ɾ��*/
            NTL_dataReqSendProc(pSockFd,pklist,eTCPClient,pIpAddr);
            return XSUCC;
        }
        /*����*/
        pklist->rsnd_delete++;
        /*�Ŷ�ʧ��,��Ϊ��ͳ�Ƽ���,���в�֪ͨ�û�����ʧ��*/
        /*�Ŷ�ʧ��,��pDataReq���ݰ�����*/
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
������:NTL_dataReqProc
����: �����ݷ��͵�����
����: pDataReq  �����ݷ��͵�ָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
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
    /*������֤��·�������Ч��*/
    if(!NTL_isValidLinkH(pDataReq->linkHandle))
    {/*�����Ϣ����ָ�벻Ϊ�գ�Ӧ���ͷ�����*/
        if(XNULLP != pDataReq->pData)
        {
            XOS_MemFree(FID_NTL,(XVOID*)(pDataReq->pData));
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq()->linkHandle [0x%x]is invalid ,free the data pointer!",pDataReq->linkHandle);
        }
        return XERROR;
    }
    /*���ʹ���ı�־��false*/
    errorFlag = XFALSE;
    linkIndex = NTL_getLinkIndex(pDataReq->linkHandle);
    switch(NTL_getLinkType( pDataReq->linkHandle))
    {
    case eUDP:
    /*ֱ�Ӷ����ݿ��Կ������̰߳�ȫ�ģ���Ϊipcm�����߳�
        ֻ�Ը�λ���޸�һ�Σ����������еĶ�����֮ǰ�޸ĵ�*/
        XOS_MutexLock(&(g_ntlCb.udpLinkMutex));
        pUdpCb = (t_UDCB*)XOS_ArrayGetElemByPos(g_ntlCb.udpLinkH,linkIndex);
        
        if(pUdpCb == XNULLP)
        { /*�ڲ��������Ӧ�ø澯*/
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
        /*��������ǰ,�ȼ����·��״̬*/
        if(pUdpCb->linkState != eStateStarted)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorLinkState;
            sendError.userHandle = pUdpCb->userHandle;
            XOS_MutexUnlock(&(g_ntlCb.udpLinkMutex));
            break;
        }
        /*��������ʱ����dataReq ��Ϣ�жԶ˵�ַ����*/
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
            /*todo  ���͵�����*/
            ret = XINET_SendMsg(&(pUdpCb->sockFd), eUDP, pIpAddr, pDataReq->msgLenth, pDataReq->pData, &out_len);
            if(ret != XSUCC)
            {
                errorFlag = XTRUE;
                sendError.errorReson =(ret ==XERROR)?eOtherErrorReason:(e_ERRORTYPE)ret;
                sendError.userHandle = pUdpCb->userHandle;
            }
        }
        else /*û��Ŀ�귢�͵�ַ*/
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
            /*�ڲ��������Ӧ�ø澯*/
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
        /*�ȼ����·��״̬*/
        if (pTcpCliCb->linkState != eStateConnected)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eErrorLinkState;
            sendError.userHandle = pTcpCliCb->userHandle;
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            break;
        }
        /*tcp ��������ʱ�����Բ���Զ˵ĵ�ַ��Ϊ�����Ч�ʣ�Ҳ������sendto����*/
        if((pDataReq->dstAddr.ip == pTcpCliCb->peerAddr.ip) &&
            (pDataReq->dstAddr.port == pTcpCliCb->peerAddr.port))
        {
            pIpAddr = &(pDataReq->dstAddr);
        }
        /* ���͵�����*/
        //        ����Ϊԭ����,�����ط��Ľӿ�
        //        ret = XINET_ Send Msg(&(pTcpCliCb->sockFd),eTCPClient,pIpAddr,pDataReq->msgLenth,pDataReq->pData);
        //        if(ret != XSUCC)
        //        {
        /*������������ todo*/
        /*Ŀǰ��ʱû����������������*/
        //            errorFlag = XTRUE;
        //            sendError.errorReson =(ret==XERROR)?eOtherErrorReason: (e_ERRORTYPE)ret;
        //            sendError.userHandle = pTcpCliCb->userHandle;
        //        }
        //        break;
        
        /*tcp ��װ*/
#ifdef TPKT_NEED
        /*todo*/
#endif /*TPKT_NEED*/
        /*��ȡ��·���������*/
        taskNo = (XU32)NTL_getLinkIndex(pDataReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo > (XU32)g_ntlCb.tcpCliTskNo)
        {
            errorFlag = XTRUE;
            sendError.errorReson = eOtherErrorReason;
            sendError.userHandle = pTcpCliCb->userHandle;
            XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));
            break;
        }

        /*��������*/
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
        { /*�ڲ��������Ӧ�ø澯*/
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

        /*���Ҷ�Ӧ�Ŀͻ���cb*/
        pTsClient = NTL_findTclient(pTcpSrvCb, &(pDataReq->dstAddr));
        if(XNULLP == pTsClient )
        {
            /*û�ҵ���Ӧ�Ŀͻ���*/
            errorFlag = XTRUE;
            /*�п��������ӵĿͻ��˹ر�*/
            sendError.errorReson = eErrorDstAddr;
            sendError.userHandle = pTcpSrvCb->userHandle;
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_dataReq(),get tcp client ip[0x%x],port[%d] ctrlBlock failed!",pDataReq->dstAddr.ip,pDataReq->dstAddr.port);
            XOS_MutexUnlock(&(g_ntlCb.tcpServerLinkMutex));
            break;
        }
        pIpAddr = &(pDataReq->dstAddr);

        /* ���͵�����*/
        //        ����Ϊԭ����,�����ط��Ľӿ� 20080729
        //        ret = XINET_ SendMsg(&(pTsClient->sockFd),eTCPServer,pIpAddr,pDataReq->msgLenth,pDataReq->pData,&out_len);
        //        if(ret != XSUCC)
        //        {
        //            errorFlag = XTRUE;
        //            sendError.errorReson =(ret == XERROR)?eOtherErrorReason:(e_ERRORTYPE)ret;
        //            sendError.userHandle = pTcpSrvCb->userHandle;
        //        }
        /*tcp ��װ*/
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

     /*���͹����г���,����error send��Ϣ���ϲ�*/
     if(errorFlag)
     {
         /*�رպ������ϲ㷢�ʹ�����Ϣ*/
         XOS_Trace(MD(FID_NTL,PL_INFO),"srdFID[%d] to destFId[%d] with msgid[%d] and pri[%d]",
             pMsg->datasrc.FID,pMsg->datadest.FID,pMsg->msgID,pMsg->prio);
         ret = NTL_msgToUser((XVOID*)&sendError,&(pMsg->datasrc),sizeof(t_SENDERROR),eErrorSend);
         if ( ret != XSUCC )
         {
             XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_dataReq()-> send msg sendError to user failed!");
         }
     }

     /*�ͷ�����*/
     if(pDataReq->pData != XNULLP)
     {
         XOS_MemFree(FID_NTL,(XVOID*) (pDataReq->pData));
     }

     return XSUCC;
}


/*-------------------------------------------------------------------------
                      ģ��ӿں���
-------------------------------------------------------------------------*/
/************************************************************************
������:NTL_StopTcpClientTimer
����:  ֹͣtcp�ͻ��˵Ķ�ʱ��
����:  pTcpCb ��tcp���ƿ�ָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 NTL_StopTcpClientTimer(t_TCCB *pTcpCb)
{
    XS32 ret = 0;

    if(!pTcpCb)
    {
        XOS_Trace(MD(FID_NTL, PL_ERR),"NTL_StopTcpClientTimer()->pTcpCb is null!");
        return XERROR;
    }
    
    /*ֹͣ��ʱ��*/
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
������:NTL_timerProc
����:  ntl ģ�鶨ʱ����Ϣ���������
����:  pMsg ����Ϣָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��:
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

    /*���ͻ������е�����*/
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
    /*����tcp�ͻ��˵�����*/
    pTcpCb = (t_TCCB*)XNULLP;

    pTcpCb = (t_TCCB*)(pParam->para1);
    if((t_TCCB*)XNULLP == pTcpCb)
    {
        XOS_MutexUnlock(&(g_ntlCb.tcpClientLinkMutex));

        return XERROR;
    }
    
    /*�жϴ�Ԫ���ڶ������Ƿ����ڿ��У���������ٴ���*/
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

    /*�����·״̬*/
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
        
        /*���԰󶨣�������ñ��˵�ַ���Ҷ˿ڲ���ͻ��Ӧ�ð󶨳ɹ�*/
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
                  ��ӵ�writeset��,�����ӳɹ�ʱ��write������λ
               else 
                  ����һ�ζ�ʱ����ʱ������ɾ������
            */
            pTcpCb->linkState = eStateConnecting;
            
            NTL_SetTcpClientFd(pTcpCb, taskNo, eWrite); 
        }
        else
        {
            pTcpCb->linkState = eStateConnected;            

            /*ֹͣ��ʱ��*/
            NTL_StopTcpClientTimer(pTcpCb);
                            
            /*��ӵ�readset��*/
            NTL_SetTcpClientFd(pTcpCb, taskNo, eRead);

            /*���������ɹ���Ϣ���ϲ�*/
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
        /*�Ѿ������ϣ��ص���ʱ��*/
        XOS_IptoStr(pTcpCb->peerAddr.ip, szTemp);
        XOS_Trace(MD(FID_NTL,PL_INFO),"NTL_timerProc(),FID[%d] tcp client  pTcpCb[0x%x] the %dth connect[%s:%d] successed!",
            pTcpCb->linkUser.FID,pTcpCb,pTcpCb->expireTimes ,szTemp,pTcpCb->peerAddr.port);

        /*ֹͣ��ʱ��*/
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
������:NTL_msgProc
����:  ntl ģ����Ϣ���������
����:  pMsg ����Ϣָ��
���:
����: �ɹ�����XOS_XSUCC,ʧ�ܷ���XERROR
˵��: ����Ϣ��������ntl��������Ϣ��ڣ�edataSend
��Ϣ���ڴ˺�������ķ�Χ��
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
        /*��·��ʼ����ֻ��һ����*/
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
#if 0  /*ntl �ĳɶ������ļ�����*/
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
������:NTL_genInfoShow
����: ��ʾNTL������Ϣ
����:
���:
����:
˵��: ntlgenshow���������ִ�к���
************************************************************************/
XVOID NTL_genInfoShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv)
{
    if(!pCliEnv || !ppArgv)
    {
        return;
    }
    
    XOS_CliExtPrintf(pCliEnv,"\n____NTL genneral config info___________ \r\n");

    /*���ntl ģ���ǲ���������*/
    if(!g_ntlCb.isGenCfg)
    {
        XOS_CliExtPrintf(pCliEnv,"sorry,NTL has't gennerally configed !\r\n  ");
        return ;
    }

    /*��ӡͨ��������Ϣ*/
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
������:NTL_threadInfoShow
����: ��ʾNTL������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
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
    /*���ntl ģ���ǲ���������*/
    if(!g_ntlCb.isGenCfg)
    {
        XOS_CliExtPrintf(pCliEnv,"sorry,NTL threads has't start  !\r\n  ");
        return ;
    }
    /*��ӡ�߳���Ϣ*/
    XOS_CliExtPrintf(pCliEnv,
        "_____common threads info______\r\nudp     threads :       %d\r\ntcpcli  threads :       %d\r\ntcpsrv  threads:        %d\r\n",
        g_ntlCb.udpTskNo,
        g_ntlCb.tcpCliTskNo,
        1
        );

    /*�߳���ϸ����б�*/
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
������:NTL_udpCfgShow
����: ��ʾNTL ��udp������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
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
        /*�����򵥵�����*/
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
������:NTL_tcpCliCfgShow
����: ��ʾNTL ��tcp client ������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
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
������:NTL_tcpCliMsgShow
����: ��ʾNTL ��tcp client ÿ��·ͳ����Ϣ��Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
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
������:NTL_tcpServMsgShow
����: ��ʾNTL ��tcp server's client ÿ��·ͳ����Ϣ��Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
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
������:NTL_tcpServCfgShow
����: ��ʾNTL ��tcp client ������Ϣ
����:
���:
����:
˵��: ntlthreedinfo���������ִ�к���
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
������:
����: ����NTL ��tcp �ط�����
����:
���:
����:
˵��:
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
    /*ֻ��23�˿ڽ��д���*/
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
������:NTL_Init
����: ��ʼ��ntlģ��
����:
���:
����:
˵��: ע�ᵽģ�������
************************************************************************/
XS8 NTL_Init(XVOID *p1, XVOID *p2)
{
    t_NTLGENCFG genCfg;
    XS32 ret = 0;
    t_PARA timerPara;
    t_BACKPARA backPara;
    XU16 timer_num = 0;
    
    /*��ʼ��tcp��ipЭ��ջ*/
    XINET_Init();

    /*�������ļ�����*/
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

    /*������������*/
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
    /*����SCTP����*/
    ret = SCTP_genCfgProc(&genCfg);
    if( ret != XSUCC)
    {
        XINET_End();
        return XERROR;
    }
#endif
    /*ע��������*/
    NTL_regCli(SYSTEM_MODE);

    /*ע�ᶨʱ��*/
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

    /*���������ط���ʱ��*/
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
    backPara.para2 = (XPOINT)eSCTPCliResendCheck;    //para1 para2��δʹ��
    backPara.para3 = (XPOINT)eSCTPCliResendTimer;
    backPara.para4 = (XPOINT)eSCTPCliResendCheck;
    if(XSUCC !=XOS_TimerStart(&sctp_cli_timer,&timerPara,&backPara))
    {
        printf("start sctp_cli_timer failed\r\n");
    }
    backPara.para1 = (XPOINT)eSCTPSerResendTimer;
    backPara.para2 = (XPOINT)eSCTPSerResendCheck;    //para1 para2��δʹ��
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
         
    /*ƽ̨��ʾ��*/
    ret = XOS_RegistCmdPrompt( cmdMode, "plat", "plat", "no parameter" );

    /*���紫����ص�����ע��*/
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
        /*����tcp �ͻ��˵ķ��񣬲�care �󶨵Ľ��*/
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

        /*���ӵ��Զ�*/
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
            /*select ��ʱ*/
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
������:NTL_channel_find_function
����: �����û���������ͨ���Ƿ��ѷ���,����ѷ��䣬������Ϊinit
���:
����: �ɹ�����XTRUE,ʧ�ܷ���XERROR
˵��:
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
������:NTL_CloseUdpSocket
����:  �ر�udp��socket�������ö���
����:  pUdpCb ��udp���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: �˺�����udp���ƿ���Ҫ���÷�����ȫ�ֱ���
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
        /*����udp����������źţ�ʹ��������*/
        XOS_SemGet(&(g_ntlCb.pUdpTsk[taskNo].taskSemp));
    }

    if(g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum == 0xffff)
    {
        g_ntlCb.pUdpTsk[taskNo].setInfo.sockNum = 0;
    }

    return XINET_CloseSock(&(pUdpCb->sockFd));
    
}

/************************************************************************
������:NTL_ResetLinkByReapply
����:  ���ظ�������·��ֹͣԭ������·
����:  pCloseReq ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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

    /*���е���Ϣ��Ҫ������ĵ���Ч��*/
    if(!NTL_isValidLinkH(pCloseReq->linkHandle))
    {
        /*��·�����Ч����Ϣ��ֱ�ӷ���*/
        XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->linkHandle invalid!");
        return XERROR;
    }
    /*��ȡ��·����*/
    linkType = NTL_getLinkType(pCloseReq->linkHandle);

    switch(linkType)
    {
    case eUDP:
        /*��ȡudp ���ƿ�*/
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
            /*�ر�socket�����fdset*/
            NTL_CloseUdpSocket(pUdpCb, taskNo);
        }       

        /*��ʼ����·״̬*/
        pUdpCb->linkState = eStateInited;
        pUdpCb->peerAddr.ip = 0;
        pUdpCb->peerAddr.port = 0;
        pUdpCb->myAddr.ip = 0;
        pUdpCb->myAddr.port = 0;
        pUdpCb->sockFd.fd = XOS_INET_INV_SOCKFD;
        break;

    case eTCPClient:
        /*��ȡtcp ���ƿ�*/
        pTcpCliCb = (t_TCCB*)XNULLP;
        pTcpCliCb = (t_TCCB*)XOS_ArrayGetElemByPos(g_ntlCb.tcpClientLinkH,NTL_getLinkIndex(pCloseReq->linkHandle));
        if(XNULLP == pTcpCliCb)
        {
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->get the tcp client control block failed!");
            return XERROR;
        }        

        taskNo = NTL_getLinkIndex(pCloseReq->linkHandle)/(g_ntlCb.genCfg.fdsPerThrPolling);
        if(taskNo >= (XU32)g_ntlCb.tcpCliTskNo)
        {
            XOS_Trace(MD(FID_NTL,PL_ERR),"NTL_ResetLinkByReapply()->tcp client taskNo is %d", taskNo);
            return XERROR;    
        }

        /*�رն�ʱ��*/
        NTL_StopTcpClientTimer(pTcpCliCb);

           /*���δ���͵����ݣ��ر�fd*/
        if (pTcpCliCb->sockFd.fd != XOS_INET_INV_SOCKFD)
        {
            NTL_closeTCli(taskNo, pTcpCliCb, NTL_SHTDWN_SEND);
        }

        /*�رպ��óɳ�ʼ��״̬*/
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
            /*���ִ�������أ���Ϊ�ϲ㷢����Ϣ�ˣ����Խ���Ϣ����*/
            XOS_Trace(MD(FID_NTL,PL_WARN),"NTL_ResetLinkByReapply()->get the tcp server control block failed!");
            return XERROR;
        }

        /*�ر�tcpserver*/
        NTL_CloseTcpServerSocket(pTcpServCb);        
        /*�ı���Ӧ��cb ����,��ʼ��*/
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
������:NTL_CloseTcpServerSocket
����:  �ر�tcpserver
����:  pTcpServCb ��tcpserver���ƿ�
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��: tcpserver��Ҫ���÷���ȫ�ֱ���
************************************************************************/
XS32 NTL_CloseTcpServerSocket(t_TSCB *pTcpServCb)
{
    XS32 ret  = 0;
    if(NULL == pTcpServCb)
    {
        return XERROR;
    }

    
    /*���ȹر����н���Ŀͻ���*/
    while(pTcpServCb->pLatestCli != XNULLP)
    {
        NTL_closeTsCli(pTcpServCb->pLatestCli);
        if(pTcpServCb->usageNum == 0)
        {
            break;
        }
    }

    /*�ر�fd*/
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
������:NTL_ResetLinkByReapplyEntry
����:  ���ظ�������·��ֹͣԭ������·
����:  pCloseReq ����Ϣָ��
���:
����: �ɹ�����XSUCC,���򷵻�XERROR
˵��:
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


