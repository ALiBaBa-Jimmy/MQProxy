/*----------------------------------------------------------------------
    created:    2004/09/29
    created:    29:9:2004   9:27
    filename:   E:\work\VcCallsim\src\Callsim\cssOam.c
    file path:  E:\work\VcCallsim\src\Callsim
    file base:  cssOam
    file ext:   c
    author:

    purpose:    ���ļ����ڴ��� TCP/IP ���OAM����
----------------------------------------------------------------------*/
#ifdef  __cplusplus
    extern  "C"{
#endif

#include "xosshell.h"
#include "fm_trap_api.h"
#include "../../hashlst/inc/saap_def.h"
#include "../inc/tcpmain.h"
#include "../inc/tcpoamproc.h"
#include "../inc/tcproutertable.h"
#include "syshash.h"
#include "../../saap/inc/saappublichead.h"

XEXTERN XU32 g_sys_start_time;
extern unsigned short app_register(unsigned int moduleid, unsigned int *ptableid, int count);
extern unsigned char  get_resource(unsigned int tbid, unsigned int size, unsigned int row_count);
extern unsigned char  register_tb_proc(unsigned int moduleid, unsigned int tableid, unsigned short msgtype, nms_agent_bytb pagent, t_XOSMUTEXID *plock);

// �ⲿ�����������˱���ר�õ��������ļ��ж���
extern TCP_DSTID_INDEX_IP_CCB  *gTCPDstIDToIPTbl;
extern HASH_TABLE               gTCPDstIDToIPHashTbl;
extern TCPE_USER_FID_T g_tcpe_UserFID[MAX_TCPE_USER_NUM];
extern int gtestInOneHost;
extern int gMaxTcpeLinkTimes;
// ��������һ�����������ṹ

TCP_LINT_CCB_INDEX    *g_pstTCP = XNULL;
TCPE_MON_FILTER_T     gstTcpeMonFilter;
t_TCPESTAT  gTcpeStaData;
XU8 g_Tcp_LinkHand;      // 1 ����, 0 ������
XU8 g_Tcp_Trace;         // 1 ����, 0 ������

//ha�л��¼�����
int Tcpe_register2Ha();

/**********************************
��������    : tcpe_link_inittable
����        : codeReview
�������    : 2008��10��8��
��������    : ��·���ʼ��
����        : void
����ֵ      : XS32
************************************/
XS32 tcpe_link_inittable( void )
{
    XU16 i ;
    g_pstTCP = (TCP_LINT_CCB_INDEX*)XOS_MemMalloc(FID_TCPE, sizeof(TCP_LINT_CCB_INDEX));

    if( XNULL != g_pstTCP )
    {
        //��ʼ��
        XOS_MemSet(g_pstTCP, BLANK_UCHAR,sizeof(TCP_LINT_CCB_INDEX));
        g_pstTCP->cout = 0;
    }else
    {
      printf("TCP_lint_table init tcp link table failed.\r\n");
      return XERROR;
    }

    /* ��ʼ��ͳ������ */
    XOS_MemSet(&gTcpeStaData,0,sizeof(t_TCPESTAT));

    for( i = 0 ; i < TCP_LINT_MAX ; i++ )
    {
        g_pstTCP->lintCcb[i].ucFlag = LINT_CCB_NULL;
        g_pstTCP->lintCcb[i].htTm  = XNULL ;
        g_pstTCP->lintCcb[i].ucHandState = TCP_LINK_FAIL;
        g_pstTCP->lintCcb[i].ulLinkStatus = TCPE_TCP_LINK_STATE_BUTT;
        g_pstTCP->lintCcb[i].ulLinkHandle = 0;
        XOS_MemSet(&g_pstTCP->lintCcb[i].ucArrLinkDesc,0x0,TCPE_LINK_DESC_LEN);
    }
    return XSUCC;
}

/**********************************
��������    : tcp_SetLocalDstID
����        : codeReview
�������    : 2008��10��8��
��������    : �˺��������趨���������
����        : XU16 LocId
����ֵ      : XU32
************************************/
XU32 tcp_SetLocalDstID(XU16 LocId   )
{
    g_pstTCP->GlobalDstID  = LocId ;
    return XSUCC ;
}

/**********************************
��������    : TCPE_GetLocalDpId
����        : codeReview
�������    : 2008��10��8��
��������    :
����ֵ      : XU16
************************************/
XU16 TCPE_GetLocalDpId()
{
    return g_pstTCP->GlobalDstID;
}

/**********************************
��������    : tcp_SetParentDstIDIndex
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : XU16 index
����ֵ      : XU32
************************************/
XU32 tcp_SetParentDstIDIndex(XU16 index )
{
    if ( index <= 0 || index > TCP_LINT_MAX  )
    {
        return  XERROR ;
    }
    g_pstTCP->ProIndex = index ;
    return XSUCC ;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ���º���Ϊ, TCP��װ�����к���
// ���ڱ�������ʱʹ��

/**********************************
��������    : Tcp_cmdSetTcpRouterDstID
����        : codeReview
�������    : 2008��10��8��
��������    : ���ñ�����������
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdSetTcpRouterDstID(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{

    XU16 LocId ;

    if(2 != siArgc)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\npara num err.");
        return;
    }

    LocId = (XU16)XOS_ATOL(ppArgv[1]);

    if(XSUCC == tcp_SetLocalDstID(LocId))
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nset local DpId=%d ok",LocId);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nset local DpId =%d failed.",LocId);
    }

    return;
}

/**********************************
��������    : Tcp_cmdGetTcpRouterDstID
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdGetTcpRouterDstID(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
   XOS_CliExtPrintf(pCliEnv, "\r\nlocal DpId=%d",TCPE_GetLocalDpId());
   return;
}

/**********************************
��������    : Tcp_cmdSetDefaultLink
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdSetDefaultLink(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 ulLinkIndex  = 0;
    if(2 != siArgc)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\npara num err,usage:setdefaultlink index.");
        return;
    }
    ulLinkIndex = XOS_ATOI(ppArgv[1]);
    if(BLANK_USHORT == ulLinkIndex)
    {
        g_pstTCP->ProIndex =(XU16) ulLinkIndex;
        XOS_CliExtPrintf(pCliEnv, "have delete the default link.");
        return;
    }
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nlink index should < %d",TCP_LINT_MAX);
        return;
    }
    if(g_pstTCP->lintCcb[ulLinkIndex].ucFlag != LINT_CCB_USER)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nlink(%d) is not exist.",ulLinkIndex);
        return;
    }

    g_pstTCP->ProIndex =(XU16) ulLinkIndex;

    XOS_CliExtPrintf(pCliEnv, "\r\nset default link (%d) ok.",ulLinkIndex);
    return;
}

/**********************************
��������    : Tcp_cmdAddLink
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdAddLink(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU16 usNo = 0;
    T_TCPE_LINK_TBL stTcpLinkTbl;
    if( 9 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv, "\r\npara num err.");
        return;
    }

    stTcpLinkTbl.ulInkIndex = XOS_ATOI(ppArgv[1]);
    if(TCP_LINT_MAX <= stTcpLinkTbl.ulInkIndex)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nlink index should < %d.",TCP_LINT_MAX);
        return;
    }
    if(g_pstTCP->lintCcb[stTcpLinkTbl.ulInkIndex].ucFlag != LINT_CCB_NULL)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nlink(%d) exist.",stTcpLinkTbl.ulInkIndex);
        return;
    }

    XOS_StrNcpy(&stTcpLinkTbl.szLinkDesc,ppArgv[2],TCPE_LINK_DESC_LEN);
    XOS_StrToNum(ppArgv[3],&stTcpLinkTbl.DstDpId);

    usNo = TCP_SearchDstIDToIpTable((XU16)stTcpLinkTbl.DstDpId);
    if ( usNo < TCP_ERROR_NUM)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nthe link with dstDP %d exist.",stTcpLinkTbl.DstDpId);
        return;
    }

    stTcpLinkTbl.connectType = XOS_ATOI(ppArgv[4]);

    XOS_StrtoIp(ppArgv[5],&stTcpLinkTbl.localIp);
    stTcpLinkTbl.localIp = XOS_NtoHl(stTcpLinkTbl.localIp);
    stTcpLinkTbl.localPort = XOS_ATOI(ppArgv[6]);

    XOS_StrtoIp(ppArgv[7],&stTcpLinkTbl.remoteIp);
    stTcpLinkTbl.remoteIp = XOS_NtoHl(stTcpLinkTbl.remoteIp);
    stTcpLinkTbl.remotePort = XOS_ATOI(ppArgv[8]);

    //stTcpLinkTbl.defaultLinkFlag = XOS_ATOI(ppArgv[9]);

    if(XSUCC != Tcpe_oamInsertLinkTable(&stTcpLinkTbl))
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nadd tcp link fail.");
        return;
    }
    XOS_CliExtPrintf(pCliEnv, "\r\nadd tcp link ok.");
    return;
}

/**********************************
��������    : Tcp_cmdDelLink
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdDelLink(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 ulLinkIndex  = 0;
    if( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv, "\r\npara num err.");
        return;
    }
    ulLinkIndex = XOS_ATOI(ppArgv[1]);

    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nlink index should < %d",TCP_LINT_MAX);
        return;
    }

    if(g_pstTCP->lintCcb[ulLinkIndex].ucFlag != LINT_CCB_USER)
    {
        XOS_CliExtPrintf(pCliEnv, "\r\nlink(%d) not exist.",ulLinkIndex);
        return;
    }

    if(XSUCC != tcpe_link_delete(ulLinkIndex) )
    {
        XOS_CliExtPrintf(pCliEnv, "\r\ndel link fail.");
        return;
    }

    XOS_CliExtPrintf(pCliEnv, "\r\ndel link succ.");
    return;
}

/**********************************
��������    : Tcp_cmdShowLink
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdShowLink(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 i  = 0;
    XU32 ulStartNum  = 0;
    XU32 ulNum  = TCP_LINT_MAX;
    XU8 meIp[20]= {0};
    XU8 peIp[20]= {0};

    XOS_MemSet(meIp,0,sizeof(meIp));
    XOS_MemSet(peIp,0,sizeof(peIp));

    XOS_CliExtPrintf(pCliEnv, "local DpId=0x%04x;ShakeHandFlag=%d\r\n",g_pstTCP->GlobalDstID,g_Tcp_LinkHand);
    if(2 <= siArgc)
    {
        ulStartNum = XOS_ATOI(ppArgv[1]);
        ulNum = ulStartNum+1;
    }
    XOS_CliExtPrintf(pCliEnv, "Index\t dstDp\t mode\t myIp\t\t myPort\t peIp\t\tpePort\t status\t hbSt\r\n");
    for(i = ulStartNum;i<ulNum;i++)
    {
        if(LINT_CCB_USER !=g_pstTCP->lintCcb[i].ucFlag)
        {
            continue;
        }
        XOS_CliExtPrintf(pCliEnv,"%d\t %04x\t %d\t %s\t %d\t %s\t %d\t %d\t %d\r\n",
                           i,g_pstTCP->lintCcb[i].DstDeviceID,
                           g_pstTCP->lintCcb[i].ucModel,SAAP_IptoStr(g_pstTCP->lintCcb[i].stMyAddr.ip,meIp),
                           g_pstTCP->lintCcb[i].stMyAddr.port,SAAP_IptoStr(g_pstTCP->lintCcb[i].stPeerAddr.ip,peIp),
                           g_pstTCP->lintCcb[i].stPeerAddr.port,g_pstTCP->lintCcb[i].ulLinkStatus,g_pstTCP->lintCcb[i].ucHandState);
    }
    XOS_CliExtPrintf(pCliEnv, "------------------------description---------------------\r\n");
    XOS_CliExtPrintf(pCliEnv, "link mode:   0-init null,1-udp,2-tcp client,3-tcp server\r\n");
    XOS_CliExtPrintf(pCliEnv, "link status: 1-init,2-start,3-estab,4-stop\r\n");
    XOS_CliExtPrintf(pCliEnv, "             5-init fail,6-start fail,7-listen,8-connecting\r\n");
    XOS_CliExtPrintf(pCliEnv, "hbSt state:  0-ok,1-error\r\n");
    XOS_CliExtPrintf(pCliEnv, "defaultLink: %d\r\n",g_pstTCP->ProIndex);
    return;
}

XVOID Tcp_cmdSetShakTimeoutCnt(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XOS_CliExtPrintf(pCliEnv, "current gMaxTcpeLinkTimes =%d.",gMaxTcpeLinkTimes);
    if(siArgc > 1)
    {
        gMaxTcpeLinkTimes = atoi(ppArgv[1]);
        XOS_CliExtPrintf(pCliEnv, "set gMaxTcpeLinkTimes =%d ok.",gMaxTcpeLinkTimes);
		return;
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv, "para num err.");
    }
    return;
}
/**********************************
��������    : Tcp_cmdStatClear
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdStatClear(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XOS_MemSet(&gTcpeStaData,0,sizeof(gTcpeStaData) );
    XOS_CliExtPrintf(pCliEnv, "\r\n cls tcpe stat ok");
    return;
}

/**********************************
��������    : Tcp_cmdShowStat
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdShowStat(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 i =0;
    XU32 j =0;
    XU32 ulStartLink = 0;
    XU32 ulEndLink  = TCP_LINT_MAX;

    if(2 <= siArgc)
    {
        ulStartLink = XOS_ATOI(ppArgv[1]);
        ulEndLink = ulStartLink +1;
    }

    XOS_CliExtPrintf(pCliEnv,"\r\n%-8s %-8s %-8s %-8s %-8s %-8s %-8s %-8s %-8s %-8s %-8s %-8s","Idx","SendCnt","SendErr","SendFail","RevCnt","RevErr","LnkTry","HandIn","HandAck","LnkLost","LnkStop","other");
    for(i =ulStartLink;i<ulEndLink;i++)
    {
        if(LINT_CCB_NULL ==  g_pstTCP->lintCcb[i].ucFlag)
        {
            continue;
        }
        XOS_CliExtPrintf(pCliEnv,"\r\n%-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d",i,
                             gTcpeStaData.msgStat[i].ulSendMsgCnt
                            ,gTcpeStaData.msgStat[i].ulSendErrMsgCnt
                            ,gTcpeStaData.msgStat[i].ulSendFailCnt
                            ,gTcpeStaData.msgStat[i].ulRecvMsgCnt
                            ,gTcpeStaData.msgStat[i].ulRecvErrMsgCnt
                            ,gTcpeStaData.msgStat[i].ulLinkTryCnt
                            ,gTcpeStaData.msgStat[i].ulLinkHand
                            ,gTcpeStaData.msgStat[i].ulLinkHandAck
                            ,gTcpeStaData.msgStat[i].ulLinkLoseCnt
                            ,gTcpeStaData.msgStat[i].ulLinkShutStop
                            ,gTcpeStaData.msgStat[i].ulOther
                            );
        for(j = 0; j < eOtherErrorReason +1;j++)
        {
            if(gTcpeStaData.msgStat[i].ulSendFailReason[j] !=0)
            {
                XOS_CliExtPrintf(pCliEnv,"f[%d]=%d",j,gTcpeStaData.msgStat[i].ulSendFailReason[j]);
            }
        }
    }
    XOS_CliExtPrintf(pCliEnv,"\r\n----------------------------------------------------------------------------------------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,"sum      %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d\r\n",
                     gTcpeStaData.msgIn.ulSendMsgCnt
                    ,gTcpeStaData.msgIn.ulSendErrMsgCnt
                    ,gTcpeStaData.msgIn.ulSendFailCnt
                    ,gTcpeStaData.msgIn.ulRecvMsgCnt
                    ,gTcpeStaData.msgIn.ulRecvErrMsgCnt
                    ,gTcpeStaData.msgIn.ulLinkTryCnt
                    ,gTcpeStaData.msgIn.ulLinkHand
                    ,gTcpeStaData.msgIn.ulLinkHandAck
                    ,gTcpeStaData.msgIn.ulLinkLoseCnt
                    ,gTcpeStaData.msgIn.ulLinkShutStop
                    ,gTcpeStaData.msgIn.ulOther
                    );
   XOS_CliExtPrintf(pCliEnv,"\r\n------------------------------------------\r\n");
   XOS_CliExtPrintf(pCliEnv,"\r\nextend link information explained as below:\r\n");
   XOS_CliExtPrintf(pCliEnv,"0:net interrupt;1:net  block;2:dst addr error\r\n");
   XOS_CliExtPrintf(pCliEnv,"3:data overflow;4:peer close;5:link state error\r\n");


    return;
}

/**********************************
��������    : Tcp_cmdGetLinkIdxByDpID
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdGetLinkIdxByDpID(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU32 ulDpId = 0;
    XU16 usno  =0;
    if( 2 > siArgc)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n para num err.");
        return;
    }
    XOS_StrToNum(ppArgv[1],&ulDpId);

    usno = TCP_SearchDstIDToIpTable((XU16)ulDpId);
    if ( TCP_ERROR_NUM <= usno)
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n can not find the link of the Dp(0x%04x)",ulDpId);
    }
    else
    {
        XOS_CliExtPrintf(pCliEnv,"\r\n link index=%d of Dpid(0x%04x)",gTCPDstIDToIPTbl[usno].usLintIndex,ulDpId);
    }
    return;
}

/**********************************
��������    : Tcp_cmdSetlinkhand
����        : codeReview
�������    : 2008��10��8��
��������    : ��������ģʽ����
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdSetlinkhand(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU8  sw;

    XU16  i ;

    if( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv, "set link hand flag,input para is error\r\n");
        return;
    }

    sw = (XU8)XOS_ATOL(ppArgv[1]);

    g_Tcp_LinkHand = sw;

    if ( 1 == g_Tcp_LinkHand )
    {

        // ����·����ȫ����
        for (  i = 0 ; i < TCP_LINT_MAX ; i++ )
        {
            g_pstTCP->lintCcb[i].ucHandState = TCP_LINK_OK ;
        }

        XOS_CliExtPrintf(pCliEnv,  "send link handshake msg switch on\r\n");
    }
    else
    {
       XOS_CliExtPrintf(pCliEnv,  "send link handshake msg switch off\r\n");
    }

    return;
}

/**********************************
��������    : Tcp_cmdSettrace
����        : codeReview
�������    : 2008��10��8��
��������    : ��������ģʽ����
����        : CLI_ENV *pCliEnv
����        : XS32 siArgc
����        : XCHAR **ppArgv
����ֵ      : XVOID
************************************/
XVOID Tcp_cmdSetTrace(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XU8  bOpen;

    if( 2 != siArgc )
    {
        XOS_CliExtPrintf(pCliEnv, "input para is error,usage:tcptrace [0=off/1=on]\r\n");
        return;
    }

    bOpen = (XU8)XOS_ATOL(ppArgv[1]);

    g_Tcp_Trace = bOpen;

    if ( 1 == g_Tcp_Trace )
    {
        XOS_CliExtPrintf(pCliEnv,  "tcptrace msg switch on\r\n");
    }
    else
    {
       XOS_CliExtPrintf(pCliEnv,  "tcptrace msg switch off\r\n");
    }

    return;
}

XVOID Tcp_cmdShowUserFid(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    XS32 i = 0;
    XOS_CliExtPrintf(pCliEnv,  "%s %s \t %s\r\n","Idx","USER_TYPE","FID");
    for(i=0; i< MAX_TCPE_USER_NUM; i++)
    {
        if(0 != g_tcpe_UserFID[i].userFid)
        {
            XOS_CliExtPrintf(pCliEnv,  "%d %-9x \t %-d\r\n",i+1,g_tcpe_UserFID[i].userType,g_tcpe_UserFID[i].userFid);
        }
    }
}

/**
*���ñ������Կ���
*20101119 cxf add
*/
XVOID Tcp_cmdSetLocalTestFlag(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv)
{
    if(siArgc < 2)
    {
        XOS_CliExtPrintf(pCliEnv, "para num err.");
        return;
    }
    gtestInOneHost = atoi(ppArgv[1]);
    return;
}
/**********************************
��������    : Tcp_command_reg
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : XS32 retId
����ֵ      : XS32
************************************/
XS32 Tcp_command_reg(XS32 retId)
{

    XOS_RegistCommand(retId,
        Tcp_cmdSetTcpRouterDstID,
        "setdpid",
        "set the local dpcode",
        "setdpid ID");

    XOS_RegistCommand(retId,
        Tcp_cmdGetTcpRouterDstID,
        "getdpid",
        "get the local dpcode",
        "getdpid");

    XOS_RegistCommand(retId,
        Tcp_cmdSetDefaultLink,
        "setdefaultlink",
        "set default link",
        "setdefaultlink index");

    XOS_RegistCommand(retId,
        Tcp_cmdAddLink,
        "addlink",
        "add link",
        "addlink Idx,desc,dstDp,mode(2-cli,3-serv),myIp,myPort,peerIp,peerPort");

    XOS_RegistCommand(retId,
        Tcp_cmdDelLink,
        "dellink",
        "del tcp link",
        "dellink Idx");

    XOS_RegistCommand(retId,
        Tcp_cmdShowLink,
        "showlink",
        "show one link info",
        "showlink Idx");

    XOS_RegistCommand(retId,
        Tcp_cmdSetShakTimeoutCnt,
        "setshaketimeoutcnt",
        "set tcpe link shake timeout count",
        "[cnt]");
    
    
    XOS_RegistCommand(retId,
        Tcp_cmdSetlinkhand,
        "settcphand",
        "set the tcp link handshake switch",
        "0-off,1-on");
    
    XOS_RegistCommand(retId,
        Tcp_cmdSetLocalTestFlag,
        "setlocaltest",
        "set test flag on local host.",
        "0-off,1-on");
    
    XOS_RegistCommand(retId,
        Tcp_cmdSetTrace,
        "tcptrace",
        "tcpe msg trace switch",
        "0-off,1-on");

    XOS_RegistCommand(retId,
        Tcp_cmdStatClear,
        "statclear",
        "clear tcpe stat",
        "no para");

    XOS_RegistCommand(retId,
        Tcp_cmdShowStat,
        "showstat",
        "show tcpe stat",
        "showstat Idx");

    XOS_RegistCommand(retId,
        Tcp_cmdGetLinkIdxByDpID,
        "getlink",
        "get link index by dstDp",
        "getlink dstDp");

    XOS_RegistCommand(retId,
        Tcp_cmdShowUserFid,
        "showuserfid",
        "get user fid",
        "no para");     
    return XSUCC;

}

/**********************************
��������    : TCP_command_init
����        : codeReview
�������    : 2008��10��8��
��������    :
����ֵ      : XS32
************************************/
XS32 TCP_command_init()
{
    /*---------------������ע�Ჿ��---------------------*/
    XS32 ret;
    ret= XOS_RegistCmdPrompt(SYSTEM_MODE,"tcpe","TCPE","no paramater");
    if(ret<0)
    {
        return XERROR;
    }
    Tcp_command_reg(ret);
    Tcpe_register2Ha();
    return XSUCC;
}

//review error
XS32 tcpe_link_delete(XU32 ulLinkIndex)
{
    XU16 usDstDpId  = 0;
    XU16 usno  = 0;

    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter tcpe_link_delete.");
    if(XSUCC !=tcpe_link_release(ulLinkIndex))
    {
      return XERROR;
    }

    if(g_pstTCP->ProIndex == ulLinkIndex)
    {
        g_pstTCP->ProIndex = BLANK_USHORT;
    }

    //ֹͣ��ʱ��
    XOS_TimerStop(FID_TCPE,g_pstTCP->lintCcb[ulLinkIndex].htTm);

    //�����·���зֻ���
    tcpe_link_clearbuff(ulLinkIndex);

    usDstDpId = g_pstTCP->lintCcb[ulLinkIndex].DstDeviceID;

    //ɾ��DP->linkIndex����,ɾ��Ŀ�����������·ӳ���ϵ
    usno = TCP_SearchDstIDToIpTable(usDstDpId);

    //20080928 modify
    if ( TCP_ERROR_NUM > usno)
    {
        TCP_deleteDstIDtoIPTable(usDstDpId);
    }
    tcpe_link_alarm(TCPE_TCP_ALARM_STATE_CON,ulLinkIndex);
    tcpe_link_clearstatus(ulLinkIndex);
    tcpe_link_clearstat(ulLinkIndex);

    if(TCP_LINT_MAX > ulLinkIndex)
    {
        //������ǰ�ĳ�ʼ�����룬�����ʼ��ΪFF
        XOS_MemSet(&g_pstTCP->lintCcb[ulLinkIndex], BLANK_UCHAR, sizeof(TCP_LINT_CCB));
        
        g_pstTCP->lintCcb[ulLinkIndex].ucFlag = LINT_CCB_NULL;
        g_pstTCP->lintCcb[ulLinkIndex].htTm  = XNULL ;
        g_pstTCP->lintCcb[ulLinkIndex].ucHandState = TCP_LINK_FAIL;
        g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus = TCPE_TCP_LINK_STATE_BUTT;
        XOS_MemSet(&g_pstTCP->lintCcb[ulLinkIndex].ucArrLinkDesc,0x0,TCPE_LINK_DESC_LEN);
    }
    return XSUCC;
}

XS32 tcpe_link_clearstatus(XU32 ulLinkIndex)
{
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcp link index(%d) error.",ulLinkIndex);
    }
    g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus = TCPE_TCP_LINK_STATE_BUTT;
    g_pstTCP->lintCcb[ulLinkIndex].ucFlag = LINT_CCB_NULL;
    return XSUCC;
}

/**********************************
��������    : tcpe_link_setstatus
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : XU32 ulLinkIndex
����        : XS32 linkStatus
����ֵ      : XS32
************************************/
XS32 tcpe_link_setstatus(XU32 ulLinkIndex,XS32 linkStatus)
{
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcp link index error.");
        return XERROR;
    }
    g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus = linkStatus;
    return XSUCC;
}

/**********************************
��������    : tcpe_link_clearbuff
����        : codeReview
�������    : 2008��10��8��
��������    : cls link buff
����        : XU32 ulLinkIndex
����ֵ      : XS32
************************************/
XS32 tcpe_link_clearbuff(XU32 ulLinkIndex)
{
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcp link index error.");
        return XERROR;
    }
    g_pstTCP->lintCcb[ulLinkIndex].bufLen = 0;
    XOS_MemSet(g_pstTCP->lintCcb[ulLinkIndex].ucBuffer,0x0,MAX_IP_DATA_LEN);
    return XSUCC;
}

/**********************************
��������    : tcpe_link_statclear
����        : 
�������    : 2008��12��8��
��������    :
����        : 
����        : 
����        : 
����ֵ      : XVOID
************************************/
XS32 tcpe_link_clearstat(int link_index)
{
    if(link_index < TCP_LINT_STAT_MAX)
    {
       XOS_MemSet(&gTcpeStaData.msgStat[link_index],0x0,sizeof(t_TCPE_LINKSTAT));
       XOS_Trace(MD(FID_TCPE,PL_DBG),"tcpe_link_statclear link %d statistics!",link_index);
    }
    return 0;
}

/**********************************
��������    : TcpTblRegToOAM
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : XU32 uiModId
����ֵ      : void
************************************/
void TcpTblRegToOAM(XU32 uiModId)
{
    XU32 aryTabList[] = {TCPE_LOCOL_DP_TABLE, TCPE_TCP_CFG_TABLE};
    t_XOSMUTEXID *ptLock = XNULL;

    (void)register_tb_proc(FID_TCPE, TCPE_LOCOL_DP_TABLE,(unsigned short)APP_SYNC_MSG, TcpeOamCallBack, ptLock);
    (void)register_tb_proc(FID_TCPE, TCPE_LOCOL_DP_TABLE,(unsigned short)SA_UPDATE_MSG, TcpeOamCallBack, ptLock);
    (void)register_tb_proc(FID_TCPE, TCPE_LOCOL_DP_TABLE,(unsigned short)SA_INSERT_MSG, TcpeOamCallBack, ptLock);
    (void)register_tb_proc(FID_TCPE, TCPE_LOCOL_DP_TABLE,(unsigned short)SA_DELETE_MSG, TcpeOamCallBack, ptLock);
    //(void)register_tb_proc(FID_TCPE, TCPE_LOCOL_DP_TABLE, SA_GET_MSG, TcpeOamCallBack, ptLock);

    (void)get_resource(TCPE_LOCOL_DP_TABLE, sizeof(T_TCPE_LOCOLDP_TBL), 1); // LOCOL dp record num is 1

    (void)register_tb_proc(FID_TCPE, TCPE_TCP_CFG_TABLE,(unsigned short)APP_SYNC_MSG, TcpeOamCallBack, ptLock);
    (void)register_tb_proc(FID_TCPE, TCPE_TCP_CFG_TABLE,(unsigned short)SA_UPDATE_MSG, TcpeOamCallBack, ptLock);
    (void)register_tb_proc(FID_TCPE, TCPE_TCP_CFG_TABLE,(unsigned short)SA_INSERT_MSG, TcpeOamCallBack, ptLock);
    (void)register_tb_proc(FID_TCPE, TCPE_TCP_CFG_TABLE,(unsigned short)SA_DELETE_MSG, TcpeOamCallBack, ptLock);
    (void)register_tb_proc(FID_TCPE, TCPE_TCP_CFG_TABLE,(unsigned short)SA_GET_MSG, TcpeOamCallBack, ptLock);

    (void)get_resource(TCPE_TCP_CFG_TABLE, sizeof(T_TCPE_LINK_TBL), TCP_LINT_MAX);

    (void)app_register(uiModId, aryTabList, sizeof(aryTabList)/sizeof(XU32));

    return;
}

XU8 TcpeOamCallBack(XU32  uiTableId, XU16 usMsgId, XU32 uiSequence,XU8 ucPackEnd, tb_record *ptRow)
{
    //XS32 clrRec = 1;
    XS32  result = 0;
    T_TCPE_LINK_TBL* pTcpTbl;
    T_TCPE_LOCOLDP_TBL * pLocalDpTbl;
    if(XNULL == ptRow)
    {
        return OAM_CALL_FAIL;
    }

    while(XNULL != ptRow)
    {
        switch(uiTableId)
        {
        case TCPE_LOCOL_DP_TABLE:
        {
            pLocalDpTbl = (T_TCPE_LOCOLDP_TBL*)ptRow->panytbrow;
            if(XNULL != pLocalDpTbl)
            {
                result = Tcpe_oamLocalDpTableOpr(usMsgId, pLocalDpTbl, ptRow->head.mask);
                if(OAM_CALL_SUCC != result)
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR), "tcpeLocalDpTblOpr failed\r\n");
                    if(APP_SYNC_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_SYNC_ERROR;
                    }
                    if(SA_INSERT_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_INSERT_ERROR;
                    }
                    if(SA_UPDATE_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_UPDATE_ERROR;
                    }
                    if(SA_DELETE_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_DELETE_ERROR;
                    }
                    if(SA_GET_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_GET_ERROR;
                    }
                }
                else
                {
                    ptRow->head.err_no = OAM_RESPONSE_OK;
                }
            }

            ptRow = ptRow->next;
            break;
        }
        //
        case TCPE_TCP_CFG_TABLE:
        {
            pTcpTbl = (T_TCPE_LINK_TBL*)ptRow->panytbrow;
            if(XNULL != pTcpTbl)
            {
                result = Tcpe_oamLinkTableOpr(usMsgId, pTcpTbl, ptRow->head.mask);
                if(OAM_CALL_SUCC != result)
                {
                    XOS_Trace(MD(FID_TCPE,PL_ERR), "tcpeLinkTblOpr failed\r\n");
                    if(APP_SYNC_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_SYNC_ERROR;
                    }
                    if(SA_INSERT_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_INSERT_ERROR;
                    }
                    if(SA_UPDATE_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_UPDATE_ERROR;
                    }
                    if(SA_DELETE_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_DELETE_ERROR;
                    }
                    if(SA_GET_MSG == usMsgId)
                    {
                        ptRow->head.err_no = OAM_RESPONSE_GET_ERROR;
                    }
                }
                else
                {
                    ptRow->head.err_no = OAM_RESPONSE_OK;
                }
            }

            ptRow = ptRow->next;
            break;
        }
        default:
        {
            XOS_Trace(MD(FID_TCPE,PL_ERR), "record is not tcpe config\r\n");
            ptRow = ptRow->next;
            break;
        }
        }
    }

    return XSUCC;
}

XS32 Tcpe_oamLinkTableOpr(XU16 usMsgId, T_TCPE_LINK_TBL* pTcpTbl, char* mask)
{
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter process Tcpe_oamLinkTableOpr\r\n");
    if(XNULL == pTcpTbl)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR)," pSaabLocalInfoTbl is Null");
        return XERROR;
    }

    switch(usMsgId)
    {
        case APP_SYNC_MSG:
        {
            return Tcpe_oamSyncLinkTable(pTcpTbl);
        }

        case SA_UPDATE_MSG:
        {
            return Tcpe_oamUpdateLinkTable(pTcpTbl, mask);
        }

        case SA_INSERT_MSG:
        {
            return Tcpe_oamInsertLinkTable(pTcpTbl);
        }

        case SA_DELETE_MSG:
        {
            return Tcpe_oamDeleteLinkTable(pTcpTbl);
        }

        case SA_GET_MSG:
        {
            return Tcpe_oamGetLinkTableStatus(pTcpTbl);
        }

        default:
        {
            break;
        }
    }

    return OAM_CALL_SUCC;
}

XS32 Tcpe_oamSyncLinkTable(T_TCPE_LINK_TBL* pTcpTbl)
{
    XU32 lintIndex = BLANK_ULONG;
    XU16 usDstDpId  = 0;
    XU16 usno  = 0;
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter Tcpe_oamSyncLinkTable()");

    lintIndex = pTcpTbl->ulInkIndex;
    if(TCP_LINT_MAX <= lintIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"link index should < %d",TCP_LINT_MAX);
    }
    //�����·���ǳ��ν���,��Ҫ��ɾ����ǰ��
    if(TCPE_TCP_LINK_STATE_BUTT !=g_pstTCP->lintCcb[lintIndex].ulLinkStatus)
    {
        // del link
        if( XSUCC != tcpe_link_delete(lintIndex) )
        {
            XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe del link fail.");
            return XERROR;
        }
    }

    //���IP��Port�Ƿ��ظ�
    {
        XU32 tmpLnkIdx;
        for (tmpLnkIdx = 0; tmpLnkIdx < TCP_LINT_MAX; tmpLnkIdx++)
        {
            if (LINT_CCB_NULL != g_pstTCP->lintCcb[tmpLnkIdx].ucFlag && 
                g_pstTCP->lintCcb[tmpLnkIdx].stMyAddr.ip == XOS_NtoHl(pTcpTbl->localIp) && 
                g_pstTCP->lintCcb[tmpLnkIdx].stMyAddr.port == (XU16)pTcpTbl->localPort)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR), "Tcpe_oamSyncLinkTable: ip&port same. Error!");
                return XERROR;
            }
        }
    }
    
    //������·��������
    g_pstTCP->lintCcb[lintIndex].stMyAddr.ip = XOS_NtoHl(pTcpTbl->localIp);
    g_pstTCP->lintCcb[lintIndex].stMyAddr.port = (XU16)pTcpTbl->localPort;
    g_pstTCP->lintCcb[lintIndex].stPeerAddr.ip = XOS_NtoHl(pTcpTbl->remoteIp);
    g_pstTCP->lintCcb[lintIndex].stPeerAddr.port = pTcpTbl->remotePort;

    g_pstTCP->lintCcb[lintIndex].DstDeviceID = (XU16)pTcpTbl->DstDpId;
    g_pstTCP->lintCcb[lintIndex].ucModel = (XU8)pTcpTbl->connectType;
    g_pstTCP->lintCcb[lintIndex].ulDefultLinkFlag = pTcpTbl->defaultLinkFlag;
    if(g_pstTCP->ProIndex == lintIndex) //ԭ������Ĭ����·
    {
        if(0 == pTcpTbl->defaultLinkFlag)
        {
            g_pstTCP->ProIndex = BLANK_USHORT;
        }
    }
    else
    {
        if(1 == pTcpTbl->defaultLinkFlag)
        {
            if(BLANK_USHORT != g_pstTCP->ProIndex)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe_oamSyncLinkTable: have config the default link.");
                return XERROR;
            }
            g_pstTCP->ProIndex = lintIndex;
        }
    }
    usDstDpId = (XU16) pTcpTbl->DstDpId;
    //����DP->linkIndex����
    usno = TCP_SearchDstIDToIpTable(usDstDpId);
    if ( TCP_ERROR_NUM > usno)
    {
        TCP_deleteDstIDtoIPTable(usDstDpId);
    }

    tcp_SetDstIDtoLintTable(usDstDpId,(XU16)lintIndex);

    //��·����
    XOS_MemSet(g_pstTCP->lintCcb[lintIndex].ucArrLinkDesc,0x0,TCPE_LINK_DESC_LEN);
    XOS_MemCpy(g_pstTCP->lintCcb[lintIndex].ucArrLinkDesc,pTcpTbl->szLinkDesc,TCPE_LINK_DESC_LEN);

    //g_pstTCP->lintCcb[lintIndex].ulLinkStatus = TCPE_TCP_LINK_STATE_INIT;
    g_pstTCP->lintCcb[lintIndex].ucFlag = LINT_CCB_USER;
    if(XSUCC != tcpe_link_init(lintIndex) )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe_oamSyncLinkTable:call tcpe_link_init failed.");
        return XERROR;
    }
    XOS_Trace(MD(FID_TCPE,PL_MIN), "out Tcpe_oamSyncLinkTable()");
    return OAM_CALL_SUCC;
}

XS32 Tcpe_oamUpdateLinkTable(T_TCPE_LINK_TBL* pTcpTbl, char* mask)
{
    XU32 ulLinkIndex = BLANK_ULONG;
    XU16 usDstDpId  = 0;
    XU16 usno  = 0;
    XU8 ucRestartFlag = FALSE;

    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter Tcpe_oamUpdateLinkTable");
    if(XNULL == pTcpTbl)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe_oamUpdateLinkTable input para is null");
        return XERROR;
    }

    ulLinkIndex = pTcpTbl->ulInkIndex;

    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe link index error.");
        return XERROR;
    }

    //������·��û�г�ʼ���ɹ�,��ʱ��֧�ָ����޸�ɾ���ӿ�
    if( TCPE_TCP_LINK_STATE_INIT == g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR), "Tcpe_oamUpdateLinkTable() Link is in init,pls wait!");
        return XERROR;
    }

    if(OAM_MASK_BIT_ONE == (mask[0] & OAM_MASK_BIT_ONE)) // Ŀ�������ı�,Ҫ����HASH��
    {
        //���½���DP->linkIndex����,��ɾ��ԭ��������HASH
        usno = TCP_SearchDstIDToIpTable(g_pstTCP->lintCcb[ulLinkIndex].DstDeviceID);
        if ( TCP_ERROR_NUM > usno)
        {
            TCP_deleteDstIDtoIPTable(g_pstTCP->lintCcb[ulLinkIndex].DstDeviceID);
        }
        usDstDpId = pTcpTbl->DstDpId; //�µ�Ŀ�������
        if(XSUCC != tcp_SetDstIDtoLintTable(usDstDpId,(XU16)ulLinkIndex) )
        {
            XOS_Trace(MD(FID_TCPE,PL_ERR),"set dp -> Index fail.");
            return XERROR;
        }
        g_pstTCP->lintCcb[ulLinkIndex].DstDeviceID = pTcpTbl->DstDpId;
    }

    if(OAM_MASK_BIT_TWO == (mask[0] & OAM_MASK_BIT_TWO))
    {
        if(eTCPServer != pTcpTbl->connectType && eTCPClient != pTcpTbl->connectType)
        {
            XOS_Trace(MD(FID_TCPE,PL_ERR),"connType(%d) err.",pTcpTbl->connectType);
            return XERROR;
        }
        ucRestartFlag = TRUE;
        g_pstTCP->lintCcb[ulLinkIndex].ucModel= (XU8)pTcpTbl->connectType;
    }

    //���IP��Port�Ƿ��ظ�
    {
        XU32 tmpLnkIdx, localIP, localPort;
        if(OAM_MASK_BIT_THREE == (mask[0] & OAM_MASK_BIT_THREE))
        {
            localIP = XOS_NtoHl(pTcpTbl->localIp);
        }
        else
        {
            localIP = g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.ip;
        }

        if(OAM_MASK_BIT_FOUR == (mask[0] & OAM_MASK_BIT_FOUR))
        {
            localPort = (XU32)pTcpTbl->localPort;
        }
        else
        {
            localPort = g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.port;
        }

        for (tmpLnkIdx = 0; tmpLnkIdx < TCP_LINT_MAX; tmpLnkIdx++)
        {
            if (ulLinkIndex != tmpLnkIdx && 
                LINT_CCB_NULL != g_pstTCP->lintCcb[tmpLnkIdx].ucFlag && 
                g_pstTCP->lintCcb[tmpLnkIdx].stMyAddr.ip == localIP && 
                g_pstTCP->lintCcb[tmpLnkIdx].stMyAddr.port == (XU16)localPort)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR), "Tcpe_oamUpdateLinkTable: ip&port same. Error!");
                return XERROR;
            }
        }
    }

    if(OAM_MASK_BIT_THREE == (mask[0] & OAM_MASK_BIT_THREE))
    {
        if( g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.ip != XOS_NtoHl((XU32)pTcpTbl->localIp) )
        {
            ucRestartFlag = TRUE;
            g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.ip = XOS_NtoHl((XU32)pTcpTbl->localIp);
        }
    }
    if(OAM_MASK_BIT_FOUR == (mask[0] & OAM_MASK_BIT_FOUR))
    {
        if( g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.port != (XU32)pTcpTbl->localPort )
        {
            ucRestartFlag = TRUE;
            g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.port = (XU16)pTcpTbl->localPort;
        }
    }

    if(OAM_MASK_BIT_FIVE == (mask[0] & OAM_MASK_BIT_FIVE))
    {
        if( g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.ip != XOS_NtoHl((XU32)pTcpTbl->remoteIp) )
        {
            ucRestartFlag = TRUE;
            g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.ip = XOS_NtoHl((XU32)pTcpTbl->remoteIp);
        }
    }

    if(OAM_MASK_BIT_SIX == (mask[0] & OAM_MASK_BIT_SIX))
    {
        if( g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.port!= (XU32)pTcpTbl->remotePort)
        {
            ucRestartFlag = TRUE;
            g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.port = (XU16)pTcpTbl->remotePort;
        }
    }
    // Ĭ����·��ʶ
    if(OAM_MASK_BIT_SEVEN== (mask[0] & OAM_MASK_BIT_SEVEN))
    {

        if(1 == pTcpTbl->defaultLinkFlag)
        {
            if(BLANK_USHORT != g_pstTCP->ProIndex)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"the default have set to link(%d),please cancel it first.",g_pstTCP->ProIndex);
                return XERROR;
            }
            g_pstTCP->ProIndex = (XU16)ulLinkIndex;
        }
        else
        {
            if(g_pstTCP->ProIndex == ulLinkIndex) //��Ĭ�ϱ�Ϊ��Ĭ��,���޸�g_pstTcp->ProIndex
            {
                g_pstTCP->ProIndex = BLANK_USHORT;
            }
        }
        g_pstTCP->lintCcb[ulLinkIndex].ulDefultLinkFlag = pTcpTbl->defaultLinkFlag;
    }

    //�����Ҫ�޸�IP/PORT����Ϣ,��Ҫ�ȸ�����·.
    if(TRUE == ucRestartFlag)
    {
        //���ݷ����ı�,����ʧ�ܵĻ�,����ԭ����·
        if(g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus == TCPE_TCP_LINK_STATE_BUILD)
        {
            tcpe_link_alarm(TCPE_TCP_ALARM_STATE_DISC,ulLinkIndex);
        }
        tcpe_link_release(ulLinkIndex);
        //��ʼ����������
        tcpe_link_setstatus(ulLinkIndex, TCPE_TCP_LINK_STATE_BUTT);

        //�����·���зֻ���
        tcpe_link_clearbuff(ulLinkIndex);
        
        //���½�������
        if( XSUCC != tcpe_link_init(ulLinkIndex) )
        {
            XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe_oamUpdateLinkTable:call tcpe_link_init link index(%d) failed", ulLinkIndex);

            //������·״̬
            tcpe_link_setstatus( ulLinkIndex,TCPE_TCP_LINK_STATE_INIT_FAIL);
            //return OAM_CALL_FAIL;
        }
    }
    else  //�ޱ仯����Ŀǰ�Ѿ�������״̬
    {
        XOS_Trace(MD(FID_TCPE,PL_INFO), "tcpeUpdtTcpRcd() cfg not changed!");
    }

    XOS_Trace(MD(FID_TCPE,PL_MIN), "out Tcpe_oamUpdateLinkTable");
    return OAM_CALL_SUCC;
}

XS32 Tcpe_oamInsertLinkTable(T_TCPE_LINK_TBL* pTcpTbl)
{
    XU32 ulLinkIndex = 0;
    XU16 usccno  = 0;
    XU16 usDstDpId= 0;
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter process Tcpe_oamInsertLinkTable()");

    ulLinkIndex = pTcpTbl->ulInkIndex;
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"add tcp link index error.");
        return XERROR;
    }
    if(eTCPServer != pTcpTbl->connectType && eTCPClient != pTcpTbl->connectType)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"connType(%d) err.",pTcpTbl->connectType);
        return XERROR;
    }

    //�����·���ǳ��ν���,��Ҫ��ɾ����ǰ��
    if( TCPE_TCP_LINK_STATE_BUTT != g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus)
    {
        if( XSUCC != tcpe_link_delete(ulLinkIndex) )
        {
            XOS_Trace(MD(FID_TCPE,PL_ERR), "Tcpe_oamInsertLinkTable() Link existed but del fail!");
            return XERROR;
        }
    }

    //���IP��Port�Ƿ��ظ�
    {
        XU32 tmpLnkIdx;
        for (tmpLnkIdx = 0; tmpLnkIdx < TCP_LINT_MAX; tmpLnkIdx++)
        {
            if (LINT_CCB_NULL != g_pstTCP->lintCcb[tmpLnkIdx].ucFlag && 
                g_pstTCP->lintCcb[tmpLnkIdx].stMyAddr.ip == XOS_NtoHl(pTcpTbl->localIp) && 
                g_pstTCP->lintCcb[tmpLnkIdx].stMyAddr.port == (XU16)pTcpTbl->localPort)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR), "Tcpe_oamInsertLinkTable: ip&port same. Error!");
                return XERROR;
            }
        }
    }

    //������·��������
    g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.ip = XOS_NtoHl(pTcpTbl->remoteIp);
    g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.port = (XU16)pTcpTbl->remotePort;
    g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.ip = XOS_NtoHl(pTcpTbl->localIp);
    g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.port = (XU16)pTcpTbl->localPort;
    g_pstTCP->lintCcb[ulLinkIndex].DstDeviceID = (XU16)pTcpTbl->DstDpId;
    g_pstTCP->lintCcb[ulLinkIndex].ucModel = (XU8)pTcpTbl->connectType;
    g_pstTCP->lintCcb[ulLinkIndex].ulDefultLinkFlag = pTcpTbl->defaultLinkFlag;
    //8888XOS_MemCpy(g_pstTCP->lintCcb[ulLinkIndex].ucArrLinkDesc,pTcpTbl->szLinkDesc,TCPE_LINK_DESC_LEN);

    //ԭ������Ĭ����·
    if(g_pstTCP->ProIndex == ulLinkIndex)
    {
        //���Ĭ����·��������·��ͬ����,����Ĭ����·
        if(0 == pTcpTbl->defaultLinkFlag)
        {
            g_pstTCP->ProIndex = BLANK_USHORT;
        }
    }
    else
    {
        if(1 == pTcpTbl->defaultLinkFlag)
        {
            if(BLANK_USHORT != g_pstTCP->ProIndex)
            {
                XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe_oamInsertLinkTable: have config the default link.");
                return XERROR;
            }
            g_pstTCP->ProIndex = ulLinkIndex;
        }
    }
    g_pstTCP->lintCcb[ulLinkIndex].ucFlag = LINT_CCB_USER;

    usDstDpId = (XU16) pTcpTbl->DstDpId;

    //����DP->linkIndex����
    //usccno �д������ǽڵ���hash���е�����
    usccno = TCP_SearchDstIDToIpTable(usDstDpId);
    //20080928�޸�
    if( usccno< TCP_ERROR_NUM)
    {
        TCP_deleteDstIDtoIPTable(usDstDpId);
    }

    //9999 �Ƿ���Ҫ�жϷ���ֵ
    if(XSUCC !=tcp_SetDstIDtoLintTable(usDstDpId,(XU16)ulLinkIndex))
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe_oamInsertLinkTable:call tcp_SetDstIDtoLintTable with linkIndex(%d) failed",ulLinkIndex);
        return XERROR;
    }

    //��ʼ����·����״̬
    if( XSUCC != tcpe_link_init(ulLinkIndex) )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe_oamInsertLinkTable:call tcpe_link_init with linkIndex(%d) failed ",ulLinkIndex);

        //������·״̬
        tcpe_link_setstatus( ulLinkIndex, TCPE_TCP_LINK_STATE_INIT_FAIL);
        return XERROR;
    }

    return OAM_CALL_SUCC;
}

XS32 Tcpe_oamDeleteLinkTable(T_TCPE_LINK_TBL * pTcpTbl)
{
    XU32 ulLinkIndex = 0;
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter process Tcpe_oamDeleteLinkTable()");
    ulLinkIndex = pTcpTbl->ulInkIndex;
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcp link index error.");
        return XERROR;
    }

    //ɾ����·,ֹͣ��ʱ��
    if( XSUCC != tcpe_link_delete(ulLinkIndex) )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcpe_oamDeleteLinkTable() delLink failed linkIndex(%d)\r\n", ulLinkIndex);
        return XERROR;
    }

    return OAM_CALL_SUCC;
}

XS32 Tcpe_oamGetLinkTableStatus(T_TCPE_LINK_TBL * pTcpTbl)
{
    XU32 ulLinkIndex = BLANK_ULONG;
    ulLinkIndex = pTcpTbl->ulInkIndex;
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcp link index error.");
        return XERROR;
    }
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter process Tcpe_oamGetLinkTableStatus()");
    XOS_Trace(MD(FID_TCPE,PL_MIN), "link[%d],status =0x%04x,handState=%d",
    ulLinkIndex,g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus,
    g_pstTCP->lintCcb[ulLinkIndex].ucHandState);

    //��д״̬�ֶ�
    if( (TCPE_TCP_LINK_STATE_BUILD == g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus)
      &&(TCP_LINK_OK == g_pstTCP->lintCcb[ulLinkIndex].ucHandState)) //����
    {
        pTcpTbl->linkStatus=  TCPE_TCP_ALARM_STATE_CON;   //���ӽ����ɹ�
        XOS_Trace(MD(FID_TCPE,PL_MIN), "link status is connect");
    }
    else
    {
        pTcpTbl->linkStatus  =  TCPE_TCP_ALARM_STATE_DISC;   //����δ�����ɹ�
        XOS_Trace(MD(FID_TCPE,PL_MIN), "link status is disconnect");
    }

    return OAM_CALL_SUCC;
}


XS32 Tcpe_oamLocalDpTableOpr(XU16 usMsgId, T_TCPE_LOCOLDP_TBL* pTcpTbl, char* mask)
{
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter process Tcpe_oamLocalDpTableOpr\r\n");
    if(XNULL == pTcpTbl)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"table is null");
        return XERROR;
    }

    switch(usMsgId)
    {
        case APP_SYNC_MSG:
        {
            return Tcpe_oamSyncLocalDpTable(pTcpTbl);
        }

        case SA_UPDATE_MSG:
        {
            return Tcpe_oamUpdateLocalDpTable(pTcpTbl, mask);
        }

        case SA_INSERT_MSG:
        {
            return Tcpe_oamSyncLocalDpTable(pTcpTbl);
        }

        default:
        {
            break;
        }
    }

    return OAM_CALL_SUCC;
}

XS32 Tcpe_oamSyncLocalDpTable(T_TCPE_LOCOLDP_TBL * pLocalDpTbl)
{
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter process Tcpe_oamSyncLocalDpTable()");
    if(0 == (XU16)pLocalDpTbl->LocolDp)
    {
        return XERROR;
    }
    g_pstTCP->GlobalDstID = (XU16)pLocalDpTbl->LocolDp;
    return OAM_CALL_SUCC;
}


XS32 Tcpe_oamUpdateLocalDpTable(T_TCPE_LOCOLDP_TBL * pLocalDpTbl, char* mask)
{
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter process Tcpe_oamUpdateLocalDpTable()");
    if(XNULL == pLocalDpTbl)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"table is null");
        return XERROR;
    }
    if(0 == (XU16)pLocalDpTbl->LocolDp)
    {
        return XERROR;
    }
    g_pstTCP->GlobalDstID = (XU16)pLocalDpTbl->LocolDp;
    return OAM_CALL_SUCC;
}

/**********************************
��������    : Tcpe LinkInit
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : XU32 ulLinkIndex
����ֵ      : XS32
************************************/
XS32 tcpe_link_init(XU32 ulLinkIndex)
{
    t_LINKINIT* pLinkHead = XNULL;
    t_XOSCOMMHEAD* pLinkInit = XNULL;
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter tcpe_link_init.");
    //if rebuild link,sleep a while maybe graceful
    //XOS_Sleep(200);//mantis 0000339
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_init:link index is error.");
        return XERROR;
    }

    //��¼��·״̬
    tcpe_link_setstatus(ulLinkIndex,TCPE_TCP_LINK_STATE_INIT);
    g_pstTCP->lintCcb[ulLinkIndex].ulLinkHandle = 0;

    //������·��ʼ����Ϣ��ntl
    pLinkInit = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_TCPE, sizeof(t_LINKINIT));

    if(XNULL == pLinkInit)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_init:call XOS_MsgMemMalloc failed!");
        return XERROR;
    }

    //����ʼ���ڴ棬����message��len�ڵ���XOS_MsgMemMallocʱ����д
    pLinkInit->datasrc.PID    = XOS_GetLocalPID();
    pLinkInit->datasrc.FID    = FID_TCPE;
    pLinkInit->datasrc.FsmId  = 0;
    pLinkInit->datadest.PID   = XOS_GetLocalPID();
    pLinkInit->datadest.FID   = FID_NTL;
    pLinkInit->datadest.FsmId = 0;
    pLinkInit->msgID          = eLinkInit;
    pLinkInit->prio           = eNormalMsgPrio;

    pLinkHead = (t_LINKINIT*)(pLinkInit + 1);
    pLinkHead->linkType = g_pstTCP->lintCcb[ulLinkIndex].ucModel;
    pLinkHead->ctrlFlag = eCompatibleTcpeen;
    g_pstTCP->lintCcb[ulLinkIndex].ucHandState  = TCP_LINK_FAIL; 
    pLinkHead->appHandle = (HAPPUSER)ulLinkIndex;

    //������Ϣ
    if(XSUCC != XOS_MsgSend(pLinkInit))
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR), "tcpe_link_init:call XOS_MsgSend failed!");
        XOS_MsgMemFree(FID_TCPE,  pLinkInit);
        return XERROR;
    }
    // ����ڴ˴�������ʱ��,��VC�²���ʱ�����п���linkAck�Ȼ�,��ʱ��δ��
    XOS_Trace(MD(FID_TCPE,PL_MIN), "out tcpe_link_init.");
    return XSUCC;
}
/**********************************
��������    : tcpe_link_initack
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : t_LINKINITACK* pLinkInitAck
����ֵ      : XS32
************************************/
XS32 tcpe_link_initack( t_LINKINITACK* pLinkInitAck)
{
    XU32 ulLinkStatus  = 0;
    XU32 ulLinkIndex  = BLANK_ULONG;
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter tcpe_link_initack.");
    if( XNULL == pLinkInitAck )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_initack,input para is null!");
        return XERROR;
    }
    ulLinkIndex = (XU32)pLinkInitAck->appHandle;
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_initack,link index is error.");
        return XERROR;
    }
    ulLinkStatus = g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus;

    //�жϱ���״̬
    if( (TCPE_TCP_LINK_STATE_INIT != ulLinkStatus)
      &&(TCPE_TCP_LINK_STATE_INIT_FAIL != ulLinkStatus ) )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR), "tcpe_link_initack,link(%d) linkstatus(%d) do not matched!",ulLinkIndex,ulLinkStatus);
        return XERROR;
    }

    //�жϽ��
    if( eSUCC != pLinkInitAck->lnitAckResult)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR), "tcpe_link_initack,link(%d) init ack result(%d) failed!",ulLinkIndex,pLinkInitAck->lnitAckResult);
        //����ʧ��,������·״̬
        tcpe_link_setstatus(ulLinkIndex,TCPE_TCP_LINK_STATE_INIT_FAIL);
        return XERROR;
    }

    //��·��ʼ���ɹ�,������·״̬
    tcpe_link_setstatus( ulLinkIndex, TCPE_TCP_LINK_STATE_START);

    //��¼�²���·���
    g_pstTCP->lintCcb[ulLinkIndex].ulLinkHandle =(XU32) pLinkInitAck->linkHandle;

    //������·
    if( XSUCC != tcpe_link_start(ulLinkIndex) )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_initack:call tcpe_link_start link index (%d) failed!",ulLinkIndex);

        //������·״̬
        tcpe_link_setstatus( ulLinkIndex,TCPE_TCP_LINK_STATE_START_FAIL);

        return XERROR;
    }

    XOS_Trace(MD(FID_TCPE,PL_MIN), "out tcpe_link_initack.");

    return XSUCC;
}

/**********************************
��������    : tcpe_link_start
����        : codeReview
�������    : 2008��10��8��
��������    : �ͻ���TCP LINK START
����        : XU32 ulLinkIndex
����ֵ      : XS32
************************************/
XS32 tcpe_link_start(XU32 ulLinkIndex )
{
    t_LINKSTART * pLinkHead = XNULL;
    t_XOSCOMMHEAD* pLinkStart = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_TCPE, sizeof(t_LINKSTART));
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter tcpe_link_start.");
    //��¼��·״̬
    tcpe_link_setstatus( ulLinkIndex, TCPE_TCP_LINK_STATE_START);
    if(XNULL == pLinkStart)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_start:msg mem malloc failed!r\n");
        return XERROR;
    }

    //����ʼ���ڴ棬����message��len�ڵ���XOS_MsgMemMallocʱ����д
    pLinkStart->datasrc.PID    = XOS_GetLocalPID();
    pLinkStart->datasrc.FID    = FID_TCPE;
    pLinkStart->datasrc.FsmId  = 0;
    pLinkStart->datadest.PID   = XOS_GetLocalPID();
    pLinkStart->datadest.FID   = FID_NTL;
    pLinkStart->datadest.FsmId = 0;
    pLinkStart->msgID          = eLinkStart;
    pLinkStart->prio           = eNormalMsgPrio;

    pLinkHead = (t_LINKSTART*)(pLinkStart + 1);
    pLinkHead->linkHandle =(HLINKHANDLE) g_pstTCP->lintCcb[ulLinkIndex].ulLinkHandle;
    if(eTCPClient == g_pstTCP->lintCcb[ulLinkIndex].ucModel )
    {
        pLinkHead->linkStart.tcpClientStart.myAddr.ip       = g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.ip;
        pLinkHead->linkStart.tcpClientStart.myAddr.port     = g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.port;
        pLinkHead->linkStart.tcpClientStart.peerAddr.ip     = g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.ip;
        pLinkHead->linkStart.tcpClientStart.peerAddr.port   = g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.port;
        pLinkHead->linkStart.tcpClientStart.recntInteval    = 0; // 2 // ������� ��
    }
    else if(eTCPServer == g_pstTCP->lintCcb[ulLinkIndex].ucModel)
    {
        pLinkHead->linkStart.tcpServerStart.myAddr.ip    = g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.ip;
        pLinkHead->linkStart.tcpServerStart.myAddr.port  = g_pstTCP->lintCcb[ulLinkIndex].stMyAddr.port;
        //9999 ÿtcpe serverֻ����һ������ 20090929 cxf comfirm
        pLinkHead->linkStart.tcpServerStart.allownClients = 1;
        pLinkHead->linkStart.tcpServerStart.authenFunc = XNULL;
    }
    else
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"Tcp conn mode(%d) err.",g_pstTCP->lintCcb[ulLinkIndex].ucModel);
        return XERROR;
    }
    //������Ϣ
    if(XSUCC != XOS_MsgSend(pLinkStart))
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR), "tcpe_link_start:call XOS_MsgSend() failed!");
        XOS_MsgMemFree(FID_TCPE,  pLinkStart);
        return XERROR;
    }

    XOS_Trace(MD(FID_TCPE,PL_DBG), "out tcpe_link_start.");
    return XSUCC;
}

/**********************************
��������    : tcpe_link_startack
����        : codeReview
�������    : 2008��10��8��
��������    : ������·������Ӧ��Ϣ
����        : t_STARTACK* pLinkStartAck
����ֵ      : XS32
************************************/
XS32 tcpe_link_startack(t_STARTACK* pLinkStartAck)
{
    XU32 ulLinkStatus  = 0;
    XU32 ulLinkIndex  = BLANK_ULONG;
    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter tcpe_link_startack.");
    if( XNULL == pLinkStartAck )
    {
        XOS_Trace(MD(FID_TCPE,PL_EXP),"tcpe_link_startack:input para is null!");
        return XERROR;
    }
    ulLinkIndex = (XU32)pLinkStartAck->appHandle;
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_EXP),"tcpe_link_startack:input link index %d is error!",ulLinkIndex);
        return XERROR;
    }
    ulLinkStatus = g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus;

    //�жϱ���״̬
    if(TCPE_TCP_LINK_STATE_BUTT == ulLinkStatus)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR), "tcpe_link_startack:link(%d) status(%d) do not matched.",ulLinkIndex,ulLinkStatus);
        tcpe_link_release(ulLinkIndex);
        return XERROR;
    }

    //�жϽ��//����TCP client ,start ack may return two times
    if( eSUCC != pLinkStartAck->linkStartResult)
    {
        if( eBlockWait == pLinkStartAck->linkStartResult )
        {
            //tcp client��Ҫ�ȴ���ʱ��,����ӡ����
            XOS_Trace(MD(FID_TCPE,PL_INFO), "tcpe_link_startack:tcp client link index(%d) start ack result(%d) is block waitting.",ulLinkIndex,pLinkStartAck->linkStartResult);
        }else
        {
            XOS_Trace(MD(FID_TCPE,PL_ERR), "tcpe_link_startack:link index(%d) start ack result(%d) is wrong.",ulLinkIndex,pLinkStartAck->linkStartResult);
        }

        //����ʧ��,������·״̬
        if( TCPE_TCP_LINK_STATE_START == ulLinkStatus )
        {
            if(eBlockWait == pLinkStartAck->linkStartResult)
            {
                tcpe_link_setstatus(ulLinkIndex, TCPE_TCP_LINK_STATE_CONNING);
            }
            else
            {
                tcpe_link_setstatus(ulLinkIndex, TCPE_TCP_LINK_STATE_START_FAIL);
            }
        }
        /*20090911 add below*/
        if(eFAIL == pLinkStartAck->linkStartResult)
        {
           g_pstTCP->lintCcb[ulLinkIndex].ucHandState  = TCP_LINK_FAIL; 
           tcpe_link_release(ulLinkIndex);
        }
        /*20090911 add above*/
        return XERROR;
    }

    //�澯
    /*20090911 comment below,alarm clear only because handshake resume*/
    //if( TCPE_TCP_LINK_STATE_STOP ==ulLinkStatus)
    //{
        //�����ǰ״̬��ֹͣ,��ʾʧ����ָ�,�յ���·����ȷ�ϳɹ�,�����ܸ澯ʧ����ָ�
        //tcpe_link_alarm(TCPE_TCP_ALARM_STATE_CON,ulLinkIndex);
    //}
    /*20090911 comment above*/

    //������·״̬
    if(g_pstTCP->lintCcb[ulLinkIndex].ucModel == eTCPServer)
    {
        tcpe_link_setstatus( ulLinkIndex, TCPE_TCP_LINK_STATE_LISTEN);
        XOS_Trace(MD(FID_TCPE,PL_INFO), "tcpe_link_startack:tcp server link index(%d) start ack success,set listen state.",ulLinkIndex);
    }
    else
    {
        tcpe_link_setstatus( ulLinkIndex, TCPE_TCP_LINK_STATE_BUILD);
        XOS_Trace(MD(FID_TCPE,PL_INFO), "tcpe_link_startack:tcp client link index(%d) start ack success,set connected state.",ulLinkIndex);
        //������·ҵ���״̬
        //20081205 modify below
        //g_pstTCP->lintCcb[ulLinkIndex].ucHandState   =  TCP_LINK_OK;
        //20081205 modify above        
    }

    // �������ֶ�ʱ��, ѭ��ʹ��
    // sever not send hand when rev connInd
    /*20090911 modify below,������,�ͻ��˶��������ֳ���*/
    //if(g_pstTCP->lintCcb[ulLinkIndex].ucModel == eTCPClient)
    //{
        SAAP_StartTimer(&g_pstTCP->lintCcb[ulLinkIndex].htTm,FID_TCPE, TCP_TWO_SECOND, TCP_HAND_TIMER,ulLinkIndex,TIMER_TYPE_LOOP);
    //}
    /*20090911 modify below,������,�ͻ��˶��������ֳ���*/
    //���������Ӧ��ʱ������
    // ����ֵ��ʼ��
    g_pstTCP->lintCcb[ulLinkIndex].hankCount = 0;

    // ��ճ���
    tcpe_link_clearbuff(ulLinkIndex);

    //��ǰ��·���� 1
    g_pstTCP->cout++ ;

    XOS_Trace(MD(FID_TCPE,PL_MIN), "out tcpe_link_startack.");

    return XSUCC;
}

/**********************************
��������    : tcpe_link_errorsend
����        : codeReview
�������    : 2008��10��8��
��������    : ������·������Ӧ��Ϣ
����        : t_SENDERROR* pIpSendErrMsg
����ֵ      : XS32
************************************/
XS32 tcpe_link_errorsend(t_SENDERROR* pIpSendErrMsg)
{
    XU32 ulLinkIndex  = BLANK_ULONG;
    ulLinkIndex = (XU32)pIpSendErrMsg->userHandle;
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"link Index err.");
        return XERROR;
    }
    gTcpeStaData.msgStat[ulLinkIndex].ulSendFailCnt++;
    gTcpeStaData.msgStat[ulLinkIndex].ulSendFailReason[pIpSendErrMsg->errorReson]++;
    return XSUCC;
}

/**********************************
��������    : tcpe_link_release
����        : 
�������    : 2008��12��8��
��������    :
����        : XU32 ulLinkIndex
����ֵ      : XS32
************************************/
XS32 tcpe_link_release(XU32 ulLinkIndex)
{
    XU32 ulLinkStatus=0;
    t_LINKRELEASE* pLinkReleaseHead = XNULL;
    t_XOSCOMMHEAD* pLinkRelease;

    XOS_Trace(MD(FID_TCPE,PL_MIN), "enter tcpe_link_release %d.",ulLinkIndex);
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcp link index(%d) error.",ulLinkIndex);
        return XERROR;

    }
    /*tcpe link state check first*/
    ulLinkStatus = g_pstTCP->lintCcb[ulLinkIndex].ulLinkStatus;
#if 0    // 20110908 cxf del ???
    if( TCPE_TCP_LINK_STATE_INIT == ulLinkStatus )
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_release:tcpe link %d status=%d.\n",ulLinkIndex,ulLinkStatus);
        return XERROR;
    }

    if( (TCPE_TCP_LINK_STATE_INIT_FAIL == ulLinkStatus)
      ||(TCPE_TCP_LINK_STATE_BUTT <=  ulLinkStatus) )
    {
        //tcpe_link_clearstatus(ulLinkIndex);
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_release:tcpe link %d status=%d.\n",ulLinkIndex,ulLinkStatus);
        return XSUCC;
    }
    /*tcpe link state check first*/
#endif
    tcpe_link_setstatus(ulLinkIndex,TCPE_TCP_LINK_STATE_NULL);// 20110908 cxf add
    pLinkRelease = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_TCPE, sizeof(t_LINKRELEASE));
    if(XNULL == pLinkRelease)
    {
        XOS_Trace(MD(FID_TCPE,PL_ERR),"tcpe_link_release:malloc memory failed!r\n");
        return XERROR;
    }

    pLinkRelease->datasrc.PID    = XOS_GetLocalPID();
    pLinkRelease->datasrc.FID    = FID_TCPE;
    pLinkRelease->datasrc.FsmId  = 0;
    pLinkRelease->datadest.PID   = XOS_GetLocalPID();
    pLinkRelease->datadest.FID   = FID_NTL;
    pLinkRelease->datadest.FsmId = 0;
    pLinkRelease->msgID          = eLinkRelease;
    pLinkRelease->prio           = eNormalMsgPrio;

    pLinkReleaseHead = (t_LINKRELEASE*)(pLinkRelease + 1);
    pLinkReleaseHead->linkHandle  = (HLINKHANDLE)g_pstTCP->lintCcb[ulLinkIndex].ulLinkHandle;

    //������Ϣ
    if(XSUCC != XOS_MsgSend(pLinkRelease))
    {
        XOS_Trace(MD(FID_TCPE,PL_EXP), "tcpe_link_release:XOS_MsgSend() fail!");
        XOS_MsgMemFree(FID_TCPE,  pLinkRelease);
        return XERROR;
    }
    //XOS_Sleep(100); //mantis 0000339
    XOS_Trace(MD(FID_TCPE,PL_MIN), "out tcpe_link_release."); 
    return XSUCC;
}
/**********************************
��������    : TcpeGetLinkHandle
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : XU32 ulLinkIndex
����ֵ      : XU32
************************************/
XU32 TcpeGetLinkHandle(XU32 ulLinkIndex)
{
    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_EXP),"link index err.");
        return XERROR;
    }
    return g_pstTCP->lintCcb[ulLinkIndex].ulLinkHandle;
}

/**********************************
��������    : TcpeGetPeerIp
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : XU32 ulLinkIndex
����ֵ      : t_IPADDR
************************************/
t_IPADDR TcpeGetPeerIp(XU32 ulLinkIndex)
{
    t_IPADDR peerIp;
    XU8 tcptype;
    peerIp.ip = 0;
    peerIp.port  = 0;

    if(TCP_LINT_MAX <= ulLinkIndex)
    {
        XOS_Trace(MD(FID_TCPE,PL_EXP),"link index err.");
        return peerIp;
    }
    tcptype = g_pstTCP->lintCcb[ulLinkIndex].ucModel;
    if(tcptype == eTCPServer)
    {
        peerIp.ip = g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.ip;
        peerIp.port = g_pstTCP->lintCcb[ulLinkIndex].stPeerAddr.port;
    }
    if(g_Tcp_Trace)
    {
       XOS_Trace(MD(FID_TCPE,PL_MIN),"TcpeGetPeerIp tcptype=%d,addr[0x%x:%d]",tcptype,peerIp.ip,peerIp.port);
    }
    return peerIp;
}

/**********************************
��������    : tcpe_link_alarm
����        : codeReview
�������    : 2008��10��8��
��������    :
����        : XU8 ucAlarmState
����        : XS32 ulLinkIndex
����ֵ      : XS32
************************************/
XS32 tcpe_link_alarm(XU8 ucAlarmState, XS32 ulLinkIndex)
{
    XU8 szAlarmStrTmp[50] = {0};
    trap_alarm_t alarm_t;

    XOS_MemSet( &alarm_t, 0, sizeof(trap_alarm_t) );
    //return XSUCC;
    alarm_t.almar_id = xwTcpeAlarmLinkLost_e;

    if (TCPE_TCP_ALARM_STATE_CON == ucAlarmState )
    {
        alarm_t.is_active = XFALSE;
        XOS_Trace(MD(FID_TCPE,PL_MIN),"tcpe link %d alarm clear",ulLinkIndex);
    }
    else
    {
        alarm_t.is_active = XTRUE;
        XOS_Trace(MD(FID_TCPE,PL_MIN),"tcpe link %d alarm occur",ulLinkIndex);
    }

    //��д��λ��Ϣ
    XOS_MemSet(szAlarmStrTmp,0,sizeof(szAlarmStrTmp) );
    XOS_StrCat(szAlarmStrTmp, "0x");
    XOS_Sprintf(&szAlarmStrTmp[2], sizeof(szAlarmStrTmp) -2, "%x",g_pstTCP->GlobalDstID);
    //XOS_XU32NtoStr(g_pstTCP->GlobalDstID , szAlarmStrTmp, sizeof(XS32));
    XOS_StrCpy(alarm_t.location_str1, szAlarmStrTmp);

    XOS_MemSet(szAlarmStrTmp,0,sizeof(szAlarmStrTmp) );
    XOS_XU32NtoStr(ulLinkIndex , szAlarmStrTmp, sizeof(XS32));
    XOS_StrCpy(alarm_t.location_str2, szAlarmStrTmp);

    //���͸澯
    fm_send_alarm( FID_TCPE, &alarm_t );

    return XSUCC;
}

XS32 Tcpe_ha_sendevent()
{
    trap_event_t trap_event;
    unsigned int u_sysuptime=0;
    int iDay,iHour,iMin,iSec;
    /*get seconds*/
    u_sysuptime=((time(XNULL)-g_sys_start_time));
    iDay =u_sysuptime/86400;
    iHour=(u_sysuptime-iDay*86400)/3600;
    iMin=(u_sysuptime-iDay*86400-iHour*3600)/60;
    iSec=u_sysuptime-iDay*86400-iHour*3600-iMin*60;

    XOS_MemSet(&trap_event, 0, sizeof(trap_event));
    trap_event.event_id = xsTcpeHaSwitch_e;
    XOS_Sprintf(trap_event.desc_str1, TRAP_STR_MAX_LEN, "sysUpTime:%02d days %02dh:%02dm:%02ds",iDay,iHour,iMin,iSec);
    fm_send_event(FID_TCPE, &trap_event);
    return XSUCC;
}
int tcpe_link_switch()
{
    XU32 i,ulNum  = TCP_LINT_MAX;
    for(i = 0;i<ulNum;i++)
    {
        if(LINT_CCB_USER !=g_pstTCP->lintCcb[i].ucFlag)
        {
            continue;
        }
        tcpe_link_release(i);
        tcpe_link_init(i);           
    }
    return 0;
}
int Tcpe_ha_callback(int event, void * para1, void * para2)
{
    t_xosha_info * req=XNULL;
    req = (t_xosha_info *)para1;
    if(!req)
    {
      return XERROR;
    }
    switch(event)
    {
        case HA_NOTIFY_EVENT_START_SERVICE:
             Tcpe_ha_sendevent();
             //tcpe_link_switch();
             break;
        case HA_NOTIFY_EVENT_STOP_SERVICE:
             break;
        default:
             break;
    }

    return XSUCC;
}
/**********************************
��������    : Tcpe_register2Ha
����        : Jeff.Zeng
�������    : 2008��1��12��
��������    :
����ֵ      : int
************************************/
int Tcpe_register2Ha()
{
  t_ha_register_para ha_para;
  ha_para.module_id = FID_TCPE;
  ha_para.p_notify_cbf = Tcpe_ha_callback;
  ha_para.app_arg = XNULL;
  if( XSUCC != ha_app_register(&ha_para))
  {
     return ERROR;
  }
  return XSUCC;
}

//8888
XU32 TCPE_FilterTcpe2Saap(COMMON_HEADER_SAAP *pstMsg)
{
    TCP_AND_PRO_LAYER_MSG_BUF *pstTcpe2SaapMsgHead;
    XU16 usDpId = 0;

    pstTcpe2SaapMsgHead = (TCP_AND_PRO_LAYER_MSG_BUF*) pstMsg;
    if(FLAG_NO == gstTcpeMonFilter.ulSpaFlag)
    {
        return FLAG_NO;
    }
    usDpId = XOS_NtoHs(pstTcpe2SaapMsgHead->tcpHead.DstDeviceID);
    if(gstTcpeMonFilter.ulDstDpId == usDpId)
    {
        return FLAG_YES;
    }
    return FLAG_NO;
}

#ifdef __cplusplus
    }
#endif
//////////////////////////////////////////////////////////////////////////


