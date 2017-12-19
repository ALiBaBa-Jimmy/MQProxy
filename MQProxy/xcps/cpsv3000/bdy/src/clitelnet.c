/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: clitelnet.c
**
**  description:   该模块为处理标准TELNET 输入输出
**
**  author: zhanglei
**
**  date:   2006.3.7
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   zhanglei         2006.3.7              create
**************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#include "cliconfig.h"
#include "clishell.h"
#include "clitelnet.h"
#include "clicmds.h"
#include "xosmodule.h"
#include "xoscfg.h"
#include "xosencap.h"
#include "xosmem.h"

/*-------------------------------------------------------------------------
                 模块内部宏定义
-------------------------------------------------------------------------*/

#ifndef __USE_OTHER_TELNETD__

#define DOUBLE_BYTE(low, high)  (low + (high << 8))
#define __INVERT_RFC__

/*-------------------------------------------------------------------------
                 模块内部结构和枚举定义
-------------------------------------------------------------------------*/

#ifdef __CLI_DEBUG_TELNET__
static FILE *telnetDebug;
#endif /* __CLI_DEBUG_TELNET__ */

static t_XOS_OPT_INIT requiredOptions[] =
{
    { kCLI_TELOPT_ECHO       ,  kCLI_TC_WONT, XTRUE,   XFALSE},
    { kCLI_TELOPT_SGA        ,  0,            XTRUE,   XFALSE},
    { kCLI_TELOPT_TTYPE      ,  kCLI_TC_DO,   XTRUE,   XFALSE},
    { kCLI_TELOPT_NAWS       ,  kCLI_TC_DO,   XTRUE,   XFALSE},
    { kCLI_TELOPT_LFLOW      ,  0,            XFALSE,  XTRUE},
    { kCLI_TELOPT_LINEMODE   ,  kCLI_TC_WILL, XFALSE,  XTRUE},
    { kCLI_TELOPT_STATUS     ,  0,            XTRUE,   XFALSE}
};

/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/
XSTATIC t_XOS_PAIR_STATE * TELNET_NewOption(CLI_ENV *pCliEnv, XU8 option, XCHAR desired );
XSTATIC t_XOS_PAIR_STATE * TELNET_GetOption(CLI_ENV *pCliEnv, XU8 option);
XSTATIC XU8   XOS_CliTelnetStateChange(CLI_ENV *pCliEnv, XCHAR from, XU8 option, XU8 action);
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                模块接口函数
-------------------------------------------------------------------------*/

/************************************************************************
 函数名: TELNET_NewOption(  )
 功能:
 输入:   pCliEnv:
         option:
         desired:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC t_XOS_PAIR_STATE * TELNET_NewOption(CLI_ENV *pCliEnv, XU8 option, XCHAR desired )
{
    t_XOS_PAIR_STATE   *pOption     = MMISC_GetOptHandled(pCliEnv);
    e_XOS_TEL_STATE     hostState   = TS_Invalid;
    e_XOS_TEL_STATE     clientState = TS_Invalid;
    XS32               index = 0;
    
    for (index = 0; index < kCLI_MAX_OPT_HANDLED; index++, pOption++)
    {
        if (pOption->option == option)
        {
            return pOption;
        }
        
        if (pOption->option == 0)
        {
            pOption->option             = option;
            pOption->desired            = desired;
            pOption->client.count       = 0;
            pOption->client.name        = 'C';
            pOption->client.optState    = (XU8)clientState;
            pOption->client.queueState  = QUEUE_Empty;
            pOption->host.count         = 0;
            pOption->host.name          = 'H';
            pOption->host.optState      = (XU8)hostState;
            pOption->host.queueState    = QUEUE_Empty;
            
            return pOption;
        }
    }

    return XNULL;
}

/************************************************************************
 函数名: TELNET_GetOption(  )
 功能:
 输入:   pCliEnv:
         option:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC t_XOS_PAIR_STATE * TELNET_GetOption(CLI_ENV *pCliEnv, XU8 option)
{
    XS32       index;
    t_XOS_PAIR_STATE *pOption = MMISC_GetOptHandled(pCliEnv);
    
    /* find existing option */
    for (index = 0; index < kCLI_MAX_OPT_HANDLED; index++, pOption++)
    {
        if (pOption->option == option)
        {
            return pOption;
        }
    }
    
    /* create default new option */
    pOption = TELNET_NewOption(pCliEnv, option, XFALSE);
    
    return pOption;
}

/*-----------------------------------------------------------------------*/

#ifdef __CLI_DEBUG_TELNET__
static XCHAR log_start = XFALSE;
static XCHAR   ErrorMsg[64];
#endif /* __CLI_DEBUG_TELNET__ */

#ifdef __CLI_DEBUG_TELNET__

#define kFORMAT_TELNET_SHOW "%-6s  %-4s  %-8s  %-8s  %-16s  %s\r\n"
static struct TelStateInfo
{
    e_XOS_TEL_STATE  state;
    XCHAR    *pName;
} TelStateInfo[] =
{
    {TS_Invalid,    "Invalid"   },
    {TS_No,         "No"        },
    {TS_WantNo,     "Want No"   },
    {TS_WantYes,    "Want Yes"  },
    {TS_Yes,        "Yes"       }
};

/************************************************************************
 函数名: TELNET_Show(  )
 功能:
 输入:   pCliEnv:
         from:
         option:
         pMsg:

 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_Show(CLI_ENV *pCliEnv, XCHAR from, XU8 option, XU8 action, XCHAR *pMsg)
{
    t_XOS_PAIR_STATE *pOption  = TELNET_GetOption(pCliEnv, option);
    XCHAR     *pSide    = ('H' == from ? "Host  " : "Client");
    XCHAR     *pInfo    = telnet_optionString(option);
    XCHAR     *pAction  = " - ";
    XCHAR     *pHost    = XNULL;
    XCHAR     *pClient  = XNULL;
    XCHAR     buffer[256] = {0};
    
    if (0 == action)
        return;
    /*check sub-data index range firstly*/
    if((pOption->host.optState<=TS_Yes)&&(pOption->client.optState<=TS_Yes))
    {
        pHost    = TelStateInfo[pOption->host.optState].pName;
        pClient  = TelStateInfo[pOption->client.optState].pName;
    }
    else
    {
        return;
    }
    
    ErrorMsg[0] = 0;
    
    switch (action)
    {
    case kCLI_TC_WILL:
        pAction = "WILL";
        break;
    case kCLI_TC_WONT:
        pAction = "WONT";
        break;
    case kCLI_TC_DO:
        pAction = "DO";
        break;
    case kCLI_TC_DONT:
        pAction = "DONT";
        break;
    default:
        pAction = "HUH?";
        break;
    }
    
    if (XFALSE == log_start)
    {
        sprintf(buffer, kFORMAT_TELNET_SHOW,
            "Side", "Req.", "Host", "Client", "Option", "Message");
        LogWrite(telnetDebug, buffer);
        sprintf(buffer, kFORMAT_TELNET_SHOW,
            "----", "----", "----", "------", "------", "-------");
        LogWrite(telnetDebug, buffer);
        log_start = XTRUE;
    }
    
    sprintf(buffer, kFORMAT_TELNET_SHOW,
        pSide, pAction, pHost, pClient, pInfo, ErrorMsg);
    LogWrite(telnetDebug, buffer);
    
    ret = XOS_CliExtWriteStrLine(pCliEnv, buffer);
}
#else
#define TELNET_Show(pCliEnv, from, option, action, pMsg)
#endif /* __CLI_DEBUG_TELNET__ */

/*-----------------------------------------------------------------------*/

#ifdef __CLI_DEBUG_TELNET__
/************************************************************************
 函数名: TELNET_Error(  )
 功能:
 输入:   pCliEnv:
         pMsg:

 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_Error(CLI_ENV *pCliEnv, XCHAR *pMsg)
{
    XOS_StrNcpy(ErrorMsg, pMsg, sizeof(ErrorMsg));
}
#else
#define TELNET_Error(pChannel, pMsg)
#endif /* __CLI_DEBUG_TELNET__ */

/*-----------------------------------------------------------------------*/

#ifdef __CLI_DEBUG_TELNET__
/************************************************************************
 函数名: TELNET_Log(  )
 功能:
 输入:   from:
         option:
         action:

 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_Log(XCHAR from, XU8 option, XU8 action)
{
    XCHAR  buffer[256];
    XCHAR *fromText   = 'H' == from ? "Host" : "Term";
    XCHAR *optionText = telnet_optionString(option);
    XCHAR *actionText;
    
    switch (action)
    {
    case kCLI_TC_WILL:
        actionText = "WILL";
        break;
    case kCLI_TC_WONT:
        actionText = "WONT";
        break;
    case kCLI_TC_DO:
        actionText = "DO";
        break;
    case kCLI_TC_DONT:
        actionText = "DONT";
        break;
    default:
        actionText = " ? ";
        break;
    }
    
    sprintf(buffer, "%-4s %-4s %s\r\n",
        fromText, actionText, optionText);
    LogWrite(telnetDebug, buffer);
}
#else
#define TELNET_Log(from, option, action)
#endif /* __CLI_DEBUG_TELNET__ */

/************************************************************************
 函数名: TELNET_StartOption(  )
 功能:
 输入:   pCliEnv:
         data:

 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_StartOption(CLI_ENV *pCliEnv, XU8 data)
{
    XCHAR  *pBuffer    = MCONN_OptBufferPtr(pCliEnv);
    XS32     index     = MCONN_GetOptBufferIndex(pCliEnv);
    
#ifdef __CLI_DEBUG_TELNET__
    XU8    subOption = MCONN_GetSubOption(pCliEnv);
    XCHAR     *info      = telnet_optionString(subOption);
#endif /* __CLI_DEBUG_TELNET__ */
    
    /* beginning capture */
    if (0 > index)
    {
#ifdef __CLI_DEBUG_TELNET__
        info     = telnet_optionString(data);
        ret = XOS_CliExtWriteStrLine(pCliEnv, "Start option data: ");
        ret = XOS_CliExtWriteStrLine(pCliEnv, info);
#endif
        
        MCONN_SetSubOption(pCliEnv, data);
        MCONN_SetOptBufferIndex(pCliEnv, 0);
        return;
    }
    
    pBuffer[index] = (XCHAR) data;
    
    /* prevent pBuffer overrun */
    if ( ++index >= kCLI_OPT_BUF_SIZE)
    {
        index = kCLI_OPT_BUF_SIZE;
    }
    
    MCONN_SetOptBufferIndex(pCliEnv, index);
}

/************************************************************************
 函数名: TELNET_OptWindowSize(  )
 功能:
 输入:   pCliEnv:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_OptWindowSize(CLI_ENV *pCliEnv)
{
    XU8     *buffer = (XU8 *)MCONN_OptBufferPtr(pCliEnv);
    XS16    width  = 0;
    XS16    height = 0;

    width  = (XS16)DOUBLE_BYTE(buffer[1], buffer[0]);
    height = (XS16)DOUBLE_BYTE(buffer[3], buffer[2]);

    MSCRN_SetWidth(pCliEnv, width);
    MSCRN_SetHeight(pCliEnv, height);
}

/************************************************************************
 函数名: XOS_CliTelnetStatus(  )
 功能:   显示当前设置
 输入:   pCliEnv:
 输出:   无
 返回:
 说明:
************************************************************************
XSTATIC XVOID XOS_CliTelnetStatus(CLI_ENV *pCliEnv)
{
    t_XOS_PAIR_STATE   *pOptions  = MMISC_GetOptHandled(pCliEnv);
    XS32        index;
    XCHAR       *pHost;
    XCHAR       *pClient;
    XCHAR       *pInfo     = XNULL;
    XCHAR       *pWant;
    XCHAR        buffer[128] = {0};

#define STATUS_FORMAT   "%-30s   %-4s  %-8s   %-8s"

    ret = XOS_CliExtWriteStrLine(pCliEnv, "");
    sprintf(buffer, STATUS_FORMAT, "Option", "Want", "Host", "Client");
    ret = XOS_CliExtWriteStrLine(pCliEnv, buffer);

    if (XNULL == pOptions)
        return;

    for (index = 0; index < kCLI_MAX_OPT_HANDLED; index++, pOptions++)
    {
        if (0 == pOptions->option)
            continue;

        pInfo   = telnet_optionString(pOptions->option);
        pHost   = TELNET_StateInfo(pOptions->host.optState);
        pClient = TELNET_StateInfo(pOptions->client.optState);
        pWant   = pOptions->desired ? "Yes" : "No";

        sprintf(buffer, STATUS_FORMAT, pInfo, pWant, pHost, pClient);
        XOS_CliExtWriteStrLine(pCliEnv, buffer);
    }
}
*/

/************************************************************************
 函数名: TELNET_Enable(  )
 功能:
 输入:   pCliEnv:
         option:
         enable:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XU8 TELNET_Enable(CLI_ENV *pCliEnv, XU8 option, XCHAR enable)
{
    XU8    reply     = 0;
    t_XOS_PAIR_STATE   *pOption   = TELNET_GetOption(pCliEnv, option);
    t_XOS_OPTION_STATE *pStatus   = &pOption->client;
    
    /* keep track of whether we want this option */
    pOption->desired = enable;
    
    /*
    
      If we decide to ask them to enable:
      NO            them=WANTYES, send DO.
      YES           Error: Already enabled.
      WANTNO  EMPTY If we are queueing requests, themq=OPPOSITE;
      otherwise, Error: Cannot initiate new request
      in the middle of negotiation.
      OPPOSITE Error: Already queued an enable request.
      WANTYES EMPTY Error: Already negotiating for enable.
      OPPOSITE themq=EMPTY.
    */
    
    if (enable)
    {
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            pStatus->optState = TS_WantYes;
#ifdef __INVERT_RFC__
            reply = kCLI_TC_WILL;
#else
            reply = kCLI_TC_DO;
#endif
            break;
        case TS_Yes:
            TELNET_Error(pCliEnv, "Already enabled");
            break;
        case TS_WantNo:
            TELNET_Error(pCliEnv, "Waiting for NO");
            break;
        case TS_WantYes:
            if (QUEUE_Opposite == pStatus->queueState)
            {
                pStatus->queueState = QUEUE_Empty;
            }
            break;
        default:
            TELNET_Error(pCliEnv, "Unknown state");
            break;
        }
    }
    else
    {
    /*
    If we decide to ask them to disable:
    NO            Error: Already disabled.
    YES           them=WANTNO, send DONT.
    WANTNO  EMPTY Error: Already negotiating for disable.
    OPPOSITE themq=EMPTY.
    WANTYES EMPTY If we are queueing requests, themq=OPPOSITE;
    otherwise, Error: Cannot initiate new request
    in the middle of negotiation.
    OPPOSITE Error: Already queued a disable request.
        */
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            TELNET_Error(pCliEnv, "Already disabled");
            break;
        case TS_Yes:
            pStatus->optState = TS_WantNo;
#ifdef __INVERT_RFC__
            reply = kCLI_TC_WONT;
#else
            reply = kCLI_TC_DONT;
#endif
            break;
        case TS_WantNo:
            if (QUEUE_Opposite == pStatus->queueState)
            {
                pStatus->queueState = QUEUE_Empty;
            }
            else
            {
                TELNET_Error(pCliEnv, "Queue is empty!");
            }
            break;
        case TS_WantYes:
            TELNET_Error(pCliEnv, "Unexpected DONT");
            break;
        default:
            TELNET_Error(pCliEnv, "Unknown state");
            break;
        }
    }
    
    TELNET_Show(pCliEnv, 'H', option, reply, "");
    
    return reply;
}

/************************************************************************
 函数名: TELNET_Required(  )
 功能:   tell client all the options we want
 输入:   pCliEnv:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XS32 TELNET_Required(CLI_ENV *pCliEnv)
{
    t_XOS_PAIR_STATE    *pOption;
    XCHAR        desired;
    XU8      action;
    XU8      option;
    XS32           index;
    t_XOS_OPT_INIT       *pInit = requiredOptions;
    XS32 nRet = 0;
    
#ifdef __CLI_DEBUG_TELNET__
    XCHAR       *pInfo;
#endif
    
    for (index = 0; index < ARRAY_SIZE(requiredOptions); index++, pInit++)
    {
        option  = pInit->option;
        desired = pInit->desired;
        
        pOption = TELNET_NewOption(pCliEnv, option, desired);
        if (XNULL == pOption)
            return CLI_ERROR;
        
#ifdef __CLI_DEBUG_TELNET__
        pInfo  = telnet_optionString(option);
#endif
        
        if (0 < pInit->action)
        {
            nRet = XOS_CliTelnetHandshake(pCliEnv, pInit->option, pInit->action);
        }
        
        if (pInit->desired)
        {
            action = TELNET_Enable(pCliEnv, option, desired);
            if (0 != action)
            {
                nRet = XOS_CliTelnetHandshake(pCliEnv, option, action);
            }
        }
    }
    
    nRet = nRet;
    return XSUCC;
}

/************************************************************************
 函数名: XOS_CliTelnetInit(  )
 功能:
 输入:   pCliEnv:
 输出:   无
 返回:
 说明:
************************************************************************/
XPUBLIC XS32 XOS_CliTelnetInit(CLI_ENV *pCliEnv)
{
    t_XOS_COM_CHAN  *pChannel = XNULL;
    XS32 optSize              = XNULL;
    
    if ( XNULL == pCliEnv )
    {
        return XTRUE;
    }
    
    pChannel = MMISC_GetChannel(pCliEnv);
    optSize  = sizeof(t_XOS_PAIR_STATE) * kCLI_MAX_OPT_HANDLED;
    
    cli_UTILInit( pCliEnv );
    
    pChannel->ThreadState    = kThreadWorking;
    
    MCONN_SetSubOption(pCliEnv,      0);
    MCONN_SetConnType(pCliEnv,       kTELNET_CONNECTION);
    MCONN_SetRecvState(pCliEnv,      kCLI_TS_DATA);
    MCONN_SetOptBufferIndex(pCliEnv, -1);
    
    MSCRN_SetWidth(pCliEnv,          kCLI_DEFAULT_WIDTH);
    MSCRN_SetHeight(pCliEnv,         kCLI_DEFAULT_HEIGHT);
    
    XOS_MemSet(MMISC_GetOptHandled(pCliEnv), 0, (XU32)optSize);
    
    //CLI_TELNETD_AddSession(pCliEnv);
    
    /* tell client options we want */
    TELNET_Required(pCliEnv);
    
    return XSUCC;
}

/************************************************************************
 函数名: TELNET_SaveTerminalType(  )
 功能:
 输入:   pCliEnv:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_SaveTerminalType(CLI_ENV *pCliEnv)
{
    XCHAR  *pBuffer  = MCONN_OptBufferPtr(pCliEnv);
    XCHAR  *pName    = MCONN_TermType(pCliEnv);
    XS32  index    = MCONN_GetOptBufferIndex(pCliEnv);

    pBuffer++;
    XOS_StrNcpy(pName, pBuffer, CLI_MIN(index, kCLI_TERM_TYPE_SIZE));
}

/************************************************************************
 函数名: TELNET_SaveOption(  )
 功能:
 输入:   pCliEnv:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_SaveOption(CLI_ENV *pCliEnv)
{
    XCHAR    saved     = XTRUE;
    XU8      subOption = MCONN_GetSubOption(pCliEnv);
    
#ifdef __CLI_DEBUG_TELNET__
    XCHAR       *info      = telnet_optionString(subOption);
#endif
    
    switch (subOption)
    {
    case kCLI_TELOPT_NAWS:
        TELNET_OptWindowSize(pCliEnv);
        break;
    case kCLI_TELOPT_TTYPE:
        TELNET_SaveTerminalType(pCliEnv);
        break;
    default:
        saved = XFALSE;
        break;
    }
    
#ifdef __CLI_DEBUG_TELNET__
    if (saved)
    {
        XOS_CliExtWriteStrLine(pCliEnv, "Saved:  ");
    }
    else
    {
        XOS_CliExtWriteStrLine(pCliEnv, "Tossed: ");
    }
    XOS_CliExtWriteStrLine(pCliEnv, info);
#endif
    
    MCONN_SetOptBufferIndex(pCliEnv, -1);
    MCONN_SetRecvState(pCliEnv, kCLI_TS_DATA);
    MCONN_SetSubOption(pCliEnv, 0);
    
    saved = saved;
}

/************************************************************************
 函数名: XOS_CliTelnetSend(  )
 功能:
 输入:   pCliEnv:
 输出:   无
 返回:
 说明:
************************************************************************/
XS32 XOS_CliTelnetSend(CLI_ENV *pEnv, XCHAR *pBuf, XS32 BufLen)
{
    /*send to platform to write*/
    t_XOSCOMMHEAD *cliMsg = XNULL;
    
    if ( XNULL == pEnv || XNULL == pBuf || 0 >= BufLen )
    {
        return XERROR;
    }
    
#if XOS_TELNETS
    if ( XSUCC != TelnetServerSend2Client( (XU16)(MMISC_GetFsmId(pEnv)), pBuf, BufLen ) )
    {
        return XERROR;
    }
#endif
    
#if XOS_TELNETD
    
    cliMsg = XOS_MsgMemMalloc( FID_CLI, BufLen );
    if ( XNULLP == cliMsg )
    {
        return XERROR;
    }
    
    cliMsg->datasrc.FID   = FID_CLI;
    cliMsg->datasrc.PID   = XOS_GetLocalPID();
    cliMsg->subID         = MMISC_GetFsmId(pEnv); /*会话ID*/
    cliMsg->datadest.FID  = FID_TELNETD;
    cliMsg->datadest.PID  = XOS_GetLocalPID();
    cliMsg->prio          = eNormalMsgPrio;
    
    XOS_MemCpy( cliMsg->message, pBuf, BufLen );
    
#ifdef CLI_LOG
    cli_log_write( eCLIDebug, "msg send TELNETD: %s", (XCHAR*)cliMsg->message );
#endif /*CLI_LOG*/
    
    if ( XSUCC != XOS_MsgSend( cliMsg ) )
    {
        XOS_MsgMemFree( FID_CLI, cliMsg );
    }
#endif
    
    return XSUCC;
}

/************************************************************************
 函数名: TELNET_Request(  )
 功能:
 输入:   pCliEnv:
         option:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_Request(CLI_ENV *pCliEnv, XU8 option)
{
    XU8       command[] = {kCLI_TC_IAC, kCLI_TC_SB, 0,
        1, kCLI_TC_IAC, kCLI_TC_SE};
    
    switch (option)
    {
    case kCLI_TELOPT_STATUS:
    case kCLI_TELOPT_TTYPE:
        break;
    default:
        return;
    }
    
    command[2] = option;
    if ( XSUCC != XOS_CliTelnetSend(pCliEnv, (XCHAR *) command, sizeof(command)) )
    {
        return;
    }
}

/************************************************************************
 函数名: TELNET_Negotiate(  )
 功能:
 输入:   pCliEnv:
         option:
         action:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XVOID TELNET_Negotiate(CLI_ENV *pCliEnv, XU8 option, XU8 action)
{
    t_XOS_PAIR_STATE   *pOption  = XNULL;
    XU8      reply     = 0;
    XS32     nRet      = 0;
    
#ifdef __CLI_DEBUG_TELNET__
    TELNET_Log('C', option, action);
#endif /* __CLI_DEBUG_TELNET__ */
    
    if (XNULL == (pOption = TELNET_GetOption(pCliEnv, option)))
    {
        return;
    }
    
    reply = XOS_CliTelnetStateChange(pCliEnv, 'C', option, action);
    
    nRet = XOS_CliTelnetHandshake(pCliEnv, option, reply);
    
    nRet = nRet;
}

/************************************************************************
 函数名: XOS_CliTelnetRecv(  )
 功能:
 输入:   pCliEnv:
         charIn:
         pBuf:
         bytesReturned:
 输出:   无
 返回:
 说明:
************************************************************************/
XPUBLIC XS32 XOS_CliTelnetRecv(CLI_ENV *pCliEnv, XU8 charIn, XCHAR *pBuf, XS32 *bytesReturned)
{
    XU8    state  = MCONN_GetRecvState(pCliEnv);
    XS32   status = XSUCC;
    
    *bytesReturned = 0;
    
    switch (state)
    {
    case kCLI_TS_DATA:
        if (kCLI_TC_IAC == charIn)
        {
            state = kCLI_TS_IAC;
        }
        else
        {
            pBuf[(*bytesReturned)++] = charIn;
        }
        break;
    case kCLI_TS_IAC:
        switch (charIn)
        {
        case kCLI_TC_IP:
        case kCLI_TC_BREAK:
        case kCLI_TC_AYT:
        case kCLI_TC_AO:
        case kCLI_TC_EC:
        case kCLI_TC_EL:
        case kCLI_TC_DM:
        case kCLI_TC_EOR:
        case kCLI_TC_EOF:
        case kCLI_TC_SUSP:
        case kCLI_TC_ABORT:
            break;
        case kCLI_TC_SB:
            state = kCLI_TS_SB;
            break;
        case kCLI_TC_WILL:
            state = kCLI_TS_WILL;
            break;
        case kCLI_TC_WONT:
            state = kCLI_TS_WONT;
            break;
        case kCLI_TC_DO:
            state = kCLI_TS_DO;
            break;
        case kCLI_TC_DONT:
            state = kCLI_TS_DONT;
            break;
        case kCLI_TC_IAC:
            pBuf[(*bytesReturned)++] = charIn;
            state = kCLI_TS_DATA;
            break;
        case kCLI_TC_SE:
            TELNET_SaveOption(pCliEnv);
            state = kCLI_TS_DATA;
            break;
        default:
            state = kCLI_TS_DATA;
            break;
        } /* switch(charIn) */
        break;
        case kCLI_TS_SB:
            if (kCLI_TC_IAC == charIn)
            {
                state = kCLI_TS_IAC;
            }
            else
            {
                TELNET_StartOption(pCliEnv, charIn);
            }
            break;
        case kCLI_TS_WILL:
        case kCLI_TS_WONT:
        case kCLI_TS_DO:
        case kCLI_TS_DONT:
            TELNET_Negotiate(pCliEnv, charIn, state);
            state = kCLI_TS_DATA;
            break;
        default:
#ifndef __CLI_DEBUG_TELNET__
            state = kCLI_TS_DATA;
            break;
#else
            status = ERROR_CLI_FAILURE;
#endif
    } /* switch (*state) */
    
    MCONN_SetRecvState(pCliEnv, state);
    return status;
}

/************************************************************************
 函数名: XOS_CliTelnetHandshake(  )
 功能:
 输入:   pCliEnv:
         option:
         action:
 输出:   无
 返回:
 说明:
************************************************************************/
XPUBLIC XS32 XOS_CliTelnetHandshake(CLI_ENV *pCliEnv, XU8 option, XU8 action)
{
    XU8        command[3];
    
    if (0 == action)
    {
        return XSUCC; /* should be an error someday */
    }
    
    TELNET_Log('H', option, action);
    
    command[0] = kCLI_TC_IAC;
    command[1] = action;
    command[2] = option;
    
    return XOS_CliTelnetSend(pCliEnv, (XCHAR *) command, sizeof(command));
}

/************************************************************************
 函数名: XOS_CliTelnetStateChange(  )
 功能:
 输入:   pCliEnv:
         from:
         option:
         action:
 输出:   无
 返回:
 说明:
************************************************************************/
XSTATIC XU8 XOS_CliTelnetStateChange(CLI_ENV *pCliEnv, XCHAR from, XU8 option, XU8 action)
{
    t_XOS_PAIR_STATE   *pOption   = TELNET_GetOption(pCliEnv, option);
    t_XOS_OPTION_STATE *pStatus   = 'H' == from ? &pOption->host : &pOption->client;
    XU8    reply     = 0;
    XU8    affirm    = 0;
    XU8    negate    = 0;
    
    if (1 == option)
    {
        option = 1;
    }
    
    if (pStatus->count++ > 4)
    {
        pStatus->count = 0;
    }
    
    switch (action)
    {
    case kCLI_TS_DONT:
    case kCLI_TS_DO:
        affirm = kCLI_TS_WILL;
        negate = kCLI_TS_WONT;
        break;
    case kCLI_TS_WILL:
    case kCLI_TS_WONT:
        affirm = kCLI_TS_DO;
        negate = kCLI_TS_DONT;
        break;
    }
    
    switch (action)
    {
    case kCLI_TS_WILL:
        /* send suboption request */
        if ('C' == from)
        {
            TELNET_Request(pCliEnv, option);
        }
    case kCLI_TS_DO:
        switch (pStatus->optState)
        {
        case TS_Invalid:
        case TS_No:
            if (pOption->desired)
            {
                reply = affirm;
                pStatus->optState = TS_Yes;
            }
            else
                reply = negate;
            break;
        case TS_Yes:
            break;
        case TS_WantNo:
            if (QUEUE_Empty == pStatus->queueState)
            {
                pStatus->optState   = TS_No;
            }
            else
            {
                pStatus->optState   = TS_Yes;
                pStatus->queueState = QUEUE_Empty;
            }
            break;
        case TS_WantYes:
            if (QUEUE_Empty == pStatus->queueState)
            {
                pStatus->optState = TS_Yes;
            }
            else
            {
                pStatus->queueState = QUEUE_Empty;
                pStatus->optState = TS_WantNo;
                reply = negate;
            }
            break;
        default:
            TELNET_Error(pCliEnv, "UNKNOWN STATE");
            break;
        }
        break;
        case kCLI_TS_WONT:
        case kCLI_TS_DONT:
            switch (pStatus->optState)
            {
            case TS_Invalid:
            case TS_No:
                break;
            case TS_Yes:
                pStatus->optState = TS_No;
                reply = negate;
                break;
            case TS_WantNo:
                if (QUEUE_Empty == pStatus->queueState)
                {
                    pStatus->optState = TS_No;
                }
                else
                {
                    pStatus->queueState = QUEUE_Empty;
                    pStatus->optState = TS_WantYes;
                    reply = affirm;
                }
                break;
            case TS_WantYes:
                pStatus->optState = TS_No;
                if (QUEUE_Opposite == pStatus->queueState)
                {
                    pStatus->queueState = QUEUE_Empty;
                }
                break;
            default:
                TELNET_Error(pCliEnv, "UNKNOWN STATE");
                break;
            }
            break;
    }
    
    TELNET_Show(pCliEnv, from, option, action, "");
    
    return reply;
}

#endif /* __USE_OTHER_TELNETD__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

