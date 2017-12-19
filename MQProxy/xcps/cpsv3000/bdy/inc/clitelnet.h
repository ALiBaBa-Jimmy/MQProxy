
/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: clicmds.h
**
**  description:  命令行 Telnet 服务器
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

#ifndef _CLI_TELNET_H_
#define _CLI_TELNET_H_

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

#include "cliconfig.h"

/*
 * Definitions for the TELNET protocol.
 */
#define kCLI_TC_IAC     255        /* interpret as command: */
#define kCLI_TC_DONT    254        /* you are not to use option */
#define kCLI_TC_DO      253        /* please, you use option */
#define kCLI_TC_WONT    252        /* I won't use option */
#define kCLI_TC_WILL    251        /* I will use option */
#define kCLI_TC_SB      250        /* interpret as subnegotiation */
#define kCLI_TC_GA      249        /* you may reverse the line */
#define kCLI_TC_EL      248        /* erase the current line */
#define kCLI_TC_EC      247        /* erase the current character */
#define kCLI_TC_AYT     246        /* are you there */
#define kCLI_TC_AO      245        /* abort output--but let prog finish */
#define kCLI_TC_IP      244        /* interrupt process--permanently */
#define kCLI_TC_BREAK   243        /* break */
#define kCLI_TC_DM      242        /* data mark--for connect. cleaning */
#define kCLI_TC_NOP     241        /* nop */
#define kCLI_TC_SE      240        /* end sub negotiation */
#define kCLI_TC_EOR     239        /* end of record (transparent mode) */
#define kCLI_TC_ABORT   238        /* Abort process */
#define kCLI_TC_SUSP    237        /* Suspend process */
#define kCLI_TC_EOF     236        /* End of file */
#define kCLI_TC_SYNCH   242        /* for telfunc calls */



 /*
 * States for the telnet session
 */

enum keCLI_TS {
    kCLI_TS_INVALID = 0,            /* safety check */
    kCLI_TS_CR,                     /* CR-LF ->'s CR */
    kCLI_TS_DATA,                   /* base state */
    kCLI_TS_CURSOR,                 /* cursor movement */
    kCLI_TS_ESC     = kESC,         /* escape sequence */
    kCLI_TS_IAC     = kCLI_TC_IAC,  /* look for double IAC's */
    kCLI_TS_SB      = kCLI_TC_SB,   /* sub option */
    kCLI_TS_WILL    = kCLI_TC_WILL, /* will option negotiation */
    kCLI_TS_WONT    = kCLI_TC_WONT, /* wont " */
    kCLI_TS_DO      = kCLI_TC_DO,   /* do " */
    kCLI_TS_DONT    = kCLI_TC_DONT  /* dont " */
};

/* telnet options */
#define kCLI_TELOPT_INVALID         -1  /* no option selected */
#define kCLI_TELOPT_BINARY           0  /* 8-bit data path */
#define kCLI_TELOPT_ECHO             1  /* echo */
#define kCLI_TELOPT_RCP              2  /* prepare to reconnect */
#define kCLI_TELOPT_SGA              3  /* suppress go ahead */
#define kCLI_TELOPT_NAMS             4  /* approximate message size */
#define kCLI_TELOPT_STATUS           5  /* give status */
#define kCLI_TELOPT_TM               6  /* timing mark */
#define kCLI_TELOPT_RCTE             7  /* remote controlled transmission and echo */
#define kCLI_TELOPT_NAOL             8  /* negotiate about output line width */
#define kCLI_TELOPT_NAOP             9  /* negotiate about output page size */
#define kCLI_TELOPT_NAOCRD          10  /* negotiate about CR disposition */
#define kCLI_TELOPT_NAOHTS          11  /* negotiate about horizontal tabstops */
#define kCLI_TELOPT_NAOHTD          12  /* negotiate about horizontal tab disposition */
#define kCLI_TELOPT_NAOFFD          13  /* negotiate about formfeed disposition */
#define kCLI_TELOPT_NAOVTS          14  /* negotiate about vertical tab stops */
#define kCLI_TELOPT_NAOVTD          15  /* negotiate about vertical tab disposition */
#define kCLI_TELOPT_NAOLFD          16  /* negotiate about output LF disposition */
#define kCLI_TELOPT_XASCII          17  /* extended ascic character set */
#define kCLI_TELOPT_LOGOUT          18  /* force logout */
#define kCLI_TELOPT_BM              19  /* byte macro */
#define kCLI_TELOPT_DET             20  /* data entry terminal */
#define kCLI_TELOPT_SUPDUP          21  /* supdup protocol */
#define kCLI_TELOPT_SUPDUPOUTPUT    22  /* supdup output */
#define kCLI_TELOPT_SNDLOC          23  /* send location */
#define kCLI_TELOPT_TTYPE           24  /* terminal type */
#define kCLI_TELOPT_EOR             25  /* end or record */
#define kCLI_TELOPT_TUID            26  /* TACACS user identification */
#define kCLI_TELOPT_OUTMRK          27  /* output marking */
#define kCLI_TELOPT_TTYLOC          28  /* terminal location number */
#define kCLI_TELOPT_3270REGIME      29  /* 3270 regime */
#define kCLI_TELOPT_X3PAD           30  /* X.3 PAD */
#define kCLI_TELOPT_NAWS            31  /* window size */
#define kCLI_TELOPT_TSPEED          32  /* terminal speed */
#define kCLI_TELOPT_LFLOW           33  /* remote flow control */
#define kCLI_TELOPT_LINEMODE        34  /* Linemode option */
#define kCLI_TELOPT_XDISPLOC        35  /* X Display Location */
#define kCLI_TELOPT_OLD_ENVIRON     36  /* Old - Environment variables */
#define kCLI_TELOPT_AUTHENTICATION  37  /* Authenticate */
#define kCLI_TELOPT_ENCRYPT         38  /* Encryption option */
#define kCLI_TELOPT_NEW_ENVIRON     39  /* New - Environment variables */
#define kCLI_TELOPT_EXOPL          255  /* extended-options-LIST */

#pragma pack(1)

/* CMD 参数*/
typedef struct _XOS_DBT_INFO
{
    XU8  option;
    XCHAR     *description;
} t_XOS_DBT_INFO;

typedef struct _XOS_DBT_DES
{
    t_XOS_DBT_INFO *info;
    XCHAR   *type;
    XU16   count;
} t_XOS_DBT_DES;

typedef struct _XOS_OPT_INIT
{
    XU8  option;
    XU8  action;
    XCHAR   desired;
    XCHAR   toggle;
} t_XOS_OPT_INIT;

#pragma pack()

typedef enum _XOS_TEL_STATE
{
    TS_Invalid = 0,
    TS_No,
    TS_WantNo,
    TS_WantYes,
    TS_Yes
} e_XOS_TEL_STATE;


/* 
*   函数声明 
*/

XPUBLIC XS32  XOS_CliTelnetHandshake( CLI_ENV *pCliEnv, XU8 option, XU8 value);
XPUBLIC XS32  XOS_CliTelnetInit(CLI_ENV *pCliEnv);
XPUBLIC XS32  XOS_CliTelnetRecv(CLI_ENV *pEnv, XU8 charIn, XCHAR *pBuf, XS32 *bytesReturned);
XPUBLIC XS32  XOS_CliTelnetSend(CLI_ENV *pEnv, XCHAR *pBuf, XS32  bufLen);

#ifdef __cplusplus
}
#endif /* _ _cplusplus */


#endif /* _CLI_TELNET_H__      */
