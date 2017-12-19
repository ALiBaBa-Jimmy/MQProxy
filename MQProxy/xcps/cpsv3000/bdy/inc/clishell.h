/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: clishell.h
**
**  description:  命令行模块命令解释部分,本模块负责用户输入命令的解析
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

#ifndef   _CLISHELL_H_
#define _CLISHELL_H_

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

/*-------------------------------------------------------------
                  包含头文件
--------------------------------------------------------------*/
#include "cliconfig.h"

/*-------------------------------------------------------------
                  宏定义
--------------------------------------------------------------*/
#define MAX_BUFF_LEN  4096
#define XOS_MAX_PATHLEN 255

#define     CLI_ERROR                     0xA000
#define     CLI_STATUS                    0xA100

#define     ERROR_CLI                     (-10000)
#define     ERROR_CLI_FAILURE             (ERROR_CLI - 1)
#define     ERROR_CLI_RETRY               (ERROR_CLI - 2)
#define     ERROR_CLI_ABORT               (ERROR_CLI - 3)
#define     ERROR_CLI_TIMEOUT             (ERROR_CLI - 4)
#define     ERROR_CLI_READ                (ERROR_CLI - 5)
#define     ERROR_CLI_WRITE               (ERROR_CLI - 6)
#define     ERROR_CLI_INVALID_PARAM       (ERROR_CLI - 7)
#define     ERROR_CLI_INVALID_VALUE       (ERROR_CLI - 8)
#define     ERROR_CLI_INVALID_NO          (ERROR_CLI - 9)
#define     ERROR_PERM_DENIED             (ERROR_CLI - 10)
#define     ERROR_CLI_NULL_ENV            (ERROR_CLI - 11)
#define     ERROR_CLI_NULL_PARAM_LIST     (ERROR_CLI - 12)
#define     ERROR_CLI_LOGIN_FAIL          (ERROR_CLI - 13)
#define     ERROR_ADD_USER                (ERROR_CLI - 14)
#define     ERROR_CLI_WRONG_MODE          (ERROR_CLI - 15)
#define     ERROR_CLI_INVALID_SESSION     (ERROR_CLI - 16)
#define     ERROR_CLI_BAD_NODE            (ERROR_CLI - 17)
#define     ERROR_CLI_AMBIGUOUS_PARAM         (ERROR_CLI - 18)
#define     ERROR_CLI_HANDLER_EXEC_FAILED     (ERROR_CLI - 19)
#define     ERROR_CLI_MISSING_PARAM           (ERROR_CLI - 20)
#define     ERROR_CLI_NO_PARAM_DATA           (ERROR_CLI - 21)
#define     ERROR_CLI_NO_INPUT_DATA           (ERROR_CLI - 22)
#define     ERROR_CLI_INVALID_OPTION          (ERROR_CLI - 23)
#define     ERROR_CLI_NO_ERROR_MSG            (ERROR_CLI - 24)
#define     ERROR_CLI_BAD_COMMAND             (ERROR_CLI - 25)
#define     ERROR_CLI_NO_HANDLERS             (ERROR_CLI - 26)
#define     ERROR_CLI_INVALID_USER            (ERROR_CLI - 27)
#define     ERROR_CLI_AMBIGUOUS_COMMAND       (ERROR_CLI - 28)
#define     ERROR_CLI_NO_HELP                 (ERROR_CLI - 29)
#define     ERROR_CLI_CONFLICT                (ERROR_CLI - 30)

/* ----------------------------------------------------- */
#define     STATUS_CLI_LOGOUT                  (CLI_STATUS - 1)
#define     STATUS_CLI_EXIT                    (CLI_STATUS - 2)
#define     STATUS_CLI_EXIT_ALL                (CLI_STATUS - 3)
#define     STATUS_CLI_INTERNAL_COMMAND        (CLI_STATUS - 4)
#define     STATUS_CLI_NOT_INTERNAL            (CLI_STATUS - 5)
#define     STATUS_CLI_HISTORY_EXEC            (CLI_STATUS - 6)
#define     STATUS_CLI_HISTORY_EDIT            (CLI_STATUS - 7)
#define     STATUS_CLI_NO_INTERMEDIATE         (CLI_STATUS - 8)
#define     STATUS_CLI_NO_ERROR                (CLI_STATUS - 9)
#define     STATUS_CLI_EXIT_TO_ROOT            (CLI_STATUS - 10)
#define     STATUS_CLI_KILL                    (CLI_STATUS - 11)
#define     STATUS_CLI_PARAM_NODE              (CLI_STATUS - 12)
/* ----------------------------------------------------- */

#define SYS_ERROR_SOCKET_GENERAL                -1200
#define SYS_ERROR_SOCKET_CREATE                 ( SYS_ERROR_SOCKET_GENERAL - 1 )
#define SYS_ERROR_SOCKET_BIND                   ( SYS_ERROR_SOCKET_GENERAL - 2 )
#define SYS_ERROR_SOCKET_THREAD                 ( SYS_ERROR_SOCKET_GENERAL - 3 )
#define SYS_ERROR_SOCKET_LISTEN                 ( SYS_ERROR_SOCKET_GENERAL - 4 )
#define SYS_ERROR_SOCKET_ACCEPT                 ( SYS_ERROR_SOCKET_GENERAL - 5 )
#define SYS_ERROR_SOCKET_CREATE_TASK            ( SYS_ERROR_SOCKET_GENERAL - 6 )
#define SYS_ERROR_SOCKET_DELETE                 ( SYS_ERROR_SOCKET_GENERAL - 7 )
#define SYS_ERROR_SOCKET_SHARE                  ( SYS_ERROR_SOCKET_GENERAL - 8 )
#define SYS_ERROR_SOCKET_START                  ( SYS_ERROR_SOCKET_GENERAL - 9 )
#define SYS_ERROR_SOCKET_CONNECT                ( SYS_ERROR_SOCKET_GENERAL - 10 )
#define SYS_ERROR_SOCKET_TIMEOUT                ( SYS_ERROR_SOCKET_GENERAL - 11 )

/* ----------------------------------------------------- */
#define ERROR_MEMMGR_GENERAL                    -500
#define ERROR_MEMMGR_BAD_MEMSIZE                ( ERROR_MEMMGR_GENERAL - 1 )
#define ERROR_MEMMGR_INITIALIZATION             ( ERROR_MEMMGR_GENERAL - 2 )
#define ERROR_MEMMGR_NO_MEMORY                  ( ERROR_MEMMGR_GENERAL - 3 )
#define ERROR_MEMMGR_BAD_POINTER                ( ERROR_MEMMGR_GENERAL - 4 )
#define ERROR_MEMMGR_BAD_FREE                   ( ERROR_MEMMGR_GENERAL - 5 )
#define ERROR_MEMMGR_MEMORY_CORRUPTION          ( ERROR_MEMMGR_GENERAL - 6 )
#define ERROR_MEMMGR_INVALID_LENGTH             ( ERROR_MEMMGR_GENERAL - 7 )

/* ----------------------------------------------------- */

#define ERROR_GENERAL                           -100
#define ERROR_GENERAL_NO_DATA                   ( ERROR_GENERAL - 1  )
#define ERROR_GENERAL_NOT_FOUND                 ( ERROR_GENERAL - 2  )
#define ERROR_GENERAL_ACCESS_DENIED             ( ERROR_GENERAL - 3  )
#define ERROR_GENERAL_NOT_EQUAL                 ( ERROR_GENERAL - 4  )
#define ERROR_GENERAL_ILLEGAL_VALUE             ( ERROR_GENERAL - 5  )
#define ERROR_GENERAL_CREATE_TASK               ( ERROR_GENERAL - 6  )
#define ERROR_GENERAL_NULL_POINTER              ( ERROR_GENERAL - 7  )
#define ERROR_GENERAL_DATA_AMBIG                ( ERROR_GENERAL - 8  )
#define ERROR_GENERAL_FILE_NOT_FOUND            ( ERROR_GENERAL - 9  )
#define ERROR_GENERAL_BUFFER_OVERRUN            ( ERROR_GENERAL - 10 )
#define ERROR_GENERAL_INVALID_RAPIDMARK         ( ERROR_GENERAL - 11 )
#define ERROR_GENERAL_OUT_OF_RANGE              ( ERROR_GENERAL - 12 )
#define ERROR_GENERAL_INVALID_PATH              ( ERROR_GENERAL - 13 )

#define SYS_ERROR_GENERAL                       -1000
#define SYS_ERROR_NO_MEMORY                     ( SYS_ERROR_GENERAL - 1 )
#define SYS_ERROR_MUTEX_CREATE                  ( SYS_ERROR_GENERAL - 2)

#define kTELNET_CONNECTION              1
#define kCONSOLE_CONNECTION             2

#ifndef kCLI_TIMEOUT
#define kCLI_TIMEOUT                    1800
#endif
#define kTELNET_TIMEOUT_IN_MINUTES      (kCLI_TIMEOUT/60)

/* number of CliChannel objects in array (# TCP threads) */
#define kTELNET_QUEUE_SIZE 10

/*鉴权状态*/
#define kTELNET_NOT_LOGIN           0
#define kTELNET_USRNAME_LOGIN       1
#define kTELNET_PASSWD_LOGIN        2
#define kTELNET_LOGIN_OK            3
#define kTELNET_USRNAME_INPUT       4
#define kTELNET_LOGIN_PROCESS       5
#define kTELNET_USR_PROCESS         6

/*-----------------------------------------------------------------------*/

/* DOS console key codes */
#define kCLI_DOSKEY_ESC     ((char)(0xE0))
#define kCLI_DOSKEY_UP      ((char)(0x48))
#define kCLI_DOSKEY_DN      ((char)(0x50))
#define kCLI_DOSKEY_RT      ((char)(0x4D))
#define kCLI_DOSKEY_LT      ((char)(0x4B))
#define kCLI_DOSKEY_PAGE_UP ((char)(0x49))
#define kCLI_DOSKEY_PAGE_DN ((char)(0x51))
#define kCLI_DOSKEY_DEL     ((char)(0x53))

/*----------------------------------------------------------------------*/

#ifndef kCRLF
#define kCRLF       "\x0D\x0A"
#define kCRLFSize   2
#endif

#ifndef kCR
#define kCR     ((char)(0x0d))
#endif

#ifndef kLF
#define kLF     ((char)(0x0a))
#endif

#ifndef kBS
#define kBS     ((char)(0x08))
#endif

#ifndef kDEL
#define kDEL    ((char)(0x7F))
#endif

#ifndef kTAB
#define kTAB    ((char)(0x09))
#endif

#ifndef kHELP
#define kHELP   ((char)(0x3F))
#endif

#ifndef kESC
#define kESC    ((char)(0x1b))
#endif

#ifndef kCHAR_NULL
#define kCHAR_NULL ((char)(0x00))
#endif

/*-----------------------------------------------------------------------*/

/* Login Specifications */
#define kCLI_MAX_LOGIN_ATTEMPTS 5
#define kCLI_MAX_LOGIN_LEN      16
#define kCLI_MAX_PASSWORD_LEN   16
#define kCLI_LOGIN_PROMPT       "Login:"
#define kCLI_PASSWORD_PROMPT    "Passwd:"

/* Shell 相关 */
/* Prompt Displayed by the Shell */
#define kCLI_DEFAULT_PROMPT "XOS"

/* Future Extensions */

/* various system options */
#define kCLI_DEFAULT_WIDTH                  80      /* default assumed screen width     */
#define kCLI_DEFAULT_HEIGHT                 24      /* default assumed screen height    */
#define kCLI_OPT_BUF_SIZE                   40      /* buffer for telnet suboptions     */
#define kCLI_MAX_PROMPT_TAIL_LEN            8       /* max size of tail-end of prompt   */
#define kCLI_INTERMEDIATE_PROMPT            ">"     /* after line continuation char     */
#define kCLI_DEFAULT_PROMPT_TAIL            "# "    /* always appended to prompt        */
#define kCLI_PROMPT_DELIMITER               "-"     /* separator in intermediate mode   */
#define kCLI_SYSTEM_HELP                    "help"  /* internal system help             */
#define kCLI_HELP_PREFIX                    " "     /* precedes help text               */
#define kCLI_HELP_DELIMITER                 " - "   /* put between cmd and help         */
#define kCLI_CMD_NO                         "no"    /* "no" command                     */
#define kCLI_COL_SEPARATOR                  "\t"
#define kCLI_ROW_SEPARATOR                  "\r\n"
#define kCLI_PARAM_LEFT_BRACKET             "<"     /* displaying parameters            */
#define kCLI_PARAM_RIGHT_BRACKET            ">"     /* displaying parameters            */
#define kCLI_HELP_WIDTH                     20     /* width of help item area          */
#define kCLI_CHAR_COMMENT                   '#'     /* ignore all text after this       */
#define kCLI_DEFAULT_PATH                   "C:\\"  /* default path for exec'ing files  */
#define kCLI_DEFAULT_LOGIN                  "admin" /* default login for development    */
#define kCLI_DEFAULT_PASSWORD               "admin" /* default password for development */
#define kCLI_DEFAULT_ACCESS                 "10"    /* default access level for dev.    */
#define kCLI_MOTD                           "motd"
#define kCLI_PRINT_CANCEL                   'Q'     /* character to cancel printing     */
#define kCLI_TASK_STACK_SIZE                8       /* depth of command queueing stack  */
#define kCLI_HELP_KEYS_WIDTH                45      /* tab stop for edit keys labels    */
#define kCLI_TERM_TYPE_SIZE                 16      /* size for storing terminal name   */
#define kCLI_MAX_OPT_HANDLED                32      /* buffer for telnet options        */
#define kCLI_ERROR_TEXT_SIZE                64
#define kCLI_MAX_LOG_STRING                 64

#define kCLI_MSG_LOGIN_FAILED               "Login incorrect:"
#define kCLI_MSG_LOGIN_OKAY                 "Login correct:"
#define kCLI_MORE_TEXT                      "Press any key to continue(Q quit)"

/* help and error messages */
#define kCLI_MSG_FAIL                       "Error: unable to open session"
#define kCLI_MSG_PARAMS_AVAIL               "Available parameters:"
#define kCLI_MSG_ERROR_PREFIX               "ERROR:"

#define kCLI_MSG_LOGOUT_IDLE                "\r\nIdle with %d minutes without input.\r\n"
#define kCLI_MSG_LOGOUT_REMAINING           "Idle %d then exit.\r\n"
#define kCLI_MSG_LOGOUT_NOW                 "System logout"

/* error message text */
#define kMSG_ERROR_CLI_INVALID_PARAM        "Invalid parameter."
#define kMSG_ERROR_CLI_AMBIGUOUS_PARAM      "Too many parameters."
#define kMSG_ERROR_CLI_DEFAULT              "Invalid format."
#define kMSG_ERROR_CLI_BAD_COMMAND          "Unknown command"
#define kMSG_ERROR_CLI_BAD_COMMAND1         "ERROR:bad command."
#define kMSG_ERROR_CLI_PER_COMMAND          "Can not excute,admin needed."

/* 命令行状态定义 */
#define kCLI_FLAG_LOGON                     0x00000001 /* user is logged onto system        */
#define kCLI_FLAG_ECHO                      0x00000002 /* enables output                    */
#define kCLI_FLAG_MODE                      0x00000004 /* disallow intermediate modes       */
#define kCLI_FLAG_IGNORE                    0x00000008 /* ignore extra parameters           */
#define kCLI_FLAG_FLUSH                     0x00000010 /* need to delete parameter LIST     */
#define kCLI_FLAG_WARN                      0x00000020 /* detailed error descriptions       */
#define kCLI_FLAG_HISTORY                   0x00000040 /* do not clear input line           */
#define kCLI_FLAG_RAPID                     0x00000080 /* interpret rapidmarks as data      */
#define kCLI_FLAG_SNMPAUTO                  0x00000100 /* auto-commit snmp changes          */
#define kCLI_FLAG_HELPCHILD                 0x00000200 /* display child nodes in help       */
#define kCLI_FLAG_HARDWRAP                  0x00000400 /* use hard wrap in output           */
#define kCLI_FLAG_UPDATE                    0x00000800 /* update input buffer for print     */
#define kCLI_FLAG_MYPROMPT                  0x00001000 /* use custom prompt                 */
#define kCLI_FLAG_INPUT                     0x00002000 /* in input mode (for logging)       */
#define kCLI_FLAG_DASHES                    0x00004000 /* uses dashes in error line         */
#define kCLI_FLAG_SNMPATOM                  0x00008000 /* commit write for each snmp var    */
#define kCLI_FLAG_MORE                      0x00010000 /* paginate output                   */
#define kCLI_FLAG_THISHELP                  0x00020000 /* show help for current node        */
#define kCLI_FLAG_PARAMHELP                 0x00040000 /* show help for parameters used     */
#define kCLI_FLAG_LOG_INPUT                 0x00080000 /* enable logging input              */
#define kCLI_FLAG_LOG_IO                    0x00100000 /* enable logging input and output   */
#define kCLI_FLAG_LOG_OUTPUT                0x00200000 /* enable logging output             */

/* 调试选项 */
#define kCLI_FLAG_PARAMS                    0x02000000 /* dump parameter LIST for debugging */
#define kCLI_FLAG_TELNET                    0x04000000 /* dump telnet stream to console     */
#define kCLI_FLAG_STACK                     0x08000000 /* dump parameter LIST to console    */
#define kCLI_FLAG_EXEC                      0x10000000 /* dump exec name to console         */
#define kCLI_FLAG_KILL                      0x80000000 /* exit thread                       */

/* 命令行键盘输入定义 */

/*
   vt100 cursor movement escape codes
   - abstract out to support other term types!
*/

#define kCLI_VTTERM_UP      "\x1B[%dA"
#define kCLI_VTTERM_DN      "\x1B[%dB"
#define kCLI_VTTERM_RT      "\x1B[%dC"
#define kCLI_VTTERM_LT      "\x1B[%dD"

/* telnet screen commands */
#define kCLI_TELNET_BACKSPACE   "\b \b"
#define kCLI_TELNET_CLEAR       "\33[2J"

/*
 *   cursor/keyboard escape codes
*/
#define kCLI_ESC_CURSOR   0x5b
#define kCLI_CURSOR_UP    0x41
#define kCLI_CURSOR_DOWN  0x42
#define kCLI_CURSOR_RIGHT 0x43
#define kCLI_CURSOR_LEFT  0x44

/* IOS-like escape codes */
#define kCLI_WORD_PREV               'b'
#define kCLI_WORD_NEXT               'f'
#define kCLI_WORD_UPPERCASE_TO_END   'c'
#define kCLI_WORD_DELETE_TO_END      'd'
#define kCLI_WORD_LOWERCASE_TO_END   'l'

#define kKEY_ESC_OFFSET         ((unsigned char)(0xA0))
#define kKEY_HELP               ((unsigned char) '?')
#define kKEY_MOVE_UP            ((unsigned char)(0x10))
#define kKEY_MOVE_DOWN          ((unsigned char)(0x0E))
#define kKEY_MOVE_LEFT          ((unsigned char)(0x02))
#define kKEY_MOVE_RIGHT         ((unsigned char)(0x06))
#define kKEY_LINE_START         ((unsigned char)(0x01))
#define kKEY_LINE_END           ((unsigned char)(0x05))
#define kKEY_DELETE_TO_END      ((unsigned char)(0x0B))
#define kKEY_DELETE_CHAR        ((unsigned char)(0x04))
#define kKEY_DELETE_FROM_START  ((unsigned char)(0x15))
#define kKEY_DELETE_WORD_START  ((unsigned char)(0x17))
#define kKEY_REFRESH_DISPLAY    ((unsigned char)(0x0C))
#define kKEY_TRANSPOSE          ((unsigned char)(0x14))
#define kKEY_END_OF_ENTRY       ((unsigned char)(0x1A))
#define kKEY_WORD_PREV          ((unsigned char)(kKEY_ESC_OFFSET) + 'B')
#define kKEY_WORD_NEXT          ((unsigned char)(kKEY_ESC_OFFSET) + 'F')
#define kKEY_UPPERCASE          ((unsigned char)(kKEY_ESC_OFFSET) + 'C')
#define kKEY_DELETE_WORD_END    ((unsigned char)(kKEY_ESC_OFFSET) + 'D')
#define kKEY_LOWERCASE          ((unsigned char)(kKEY_ESC_OFFSET) + 'L')
#define kKEY_DELETE             ((unsigned char)(0x7F))
#define kKEY_BREAK              ((unsigned char)(0x03))

#define LEAST_RECENT_HIST(x)    (CLI_MAX(((x)->iNumCmds - (x)->iMaxHistCmds + 1), 0))
#define HIST_BUFFER_FULL(x)     ((x)->bufferIndex >= ((x)->iMaxHistCmds - 1))
#define HistBuffPtr(x, y)       (((XCHAR *)&((x)->pHistBuff[(y)].histCmd)) )

#define __ENABLE_CUSTOM_STRUCT__

/*2007/12/17 adjust from 32 to 64 for reg extend limit,sag liuda advised*/
#define     CLI_MAX_CMD_LEN             64

//#if ( XOS_WIN32 | XOS_LINUX | XOS_SOLARIS )
#if defined (XOS_WIN32) || defined (XOS_LINUX) || defined (XOS_SOLARIS)
#define     CLI_MAX_CMD_HELP_LEN        128
#define     CLI_MAX_ARG_LEN             256
/*2007/12/05 adjust from 50 to 64 for reg extend limit */
#define     CLI_MAX_CMD_NUM             80
#endif

#ifdef XOS_VXWORKS
#define     CLI_MAX_CMD_HELP_LEN        128
#define     CLI_MAX_ARG_LEN             256
/*2007/12/05 adjust from 32 to 64 for reg extend limit*/
#define     CLI_MAX_CMD_NUM             80
#endif

#define     CLI_MAX_CURSOR_LEN          60

#define     CLI_MAX_ARG_NUM             16
#define     CLI_MAX_INPUT_LEN           256
#define     CLI_REQ_EXPIRE_TIME         5000  /*five seconds*/

#define     K_MAX_ENV_SIZE              19

/*命令列表初始化命令个数 ? help clear exit || quit*/
#define CLI_INIT_CMDS_NUM  4
#define INDEX_CMD_HELP     0
#define INDEX_CMD_HELP1    1
#define INDEX_CMD_CLEAR    2
#define INDEX_CMD_QUIT     3
#define INDEX_CMD_VERSION  4
#define INDEX_CMD_SID      5

#define kenv_HTTP_VERSION        14

/* cache object access */
#define K_CACHE_NO                0
#define K_CACHE_READ              1
#define K_CACHE_WRITE             2
#define K_CACHE_READWRITE         (K_CACHE_READ | K_CACHE_WRITE)

#define K_TCP_LISTEN_BACKLOG      20
#define K_TCPD_TASKNAME_LENGTH    16

/* Transmission Flow Constants */
#define K_FLOW_OFF    0
#define K_FLOW_ON     1

/*
* 访问宏定义
*/

#define CLIENV(pEnv)                            (pEnv)->pCli

#define CLI_EnableFeature(pEnv, x)              (CLIENV(pEnv)->cliFlags |=  (x))
#define CLI_DisableFeature(pEnv, x)             (CLIENV(pEnv)->cliFlags &= ~(x))
#define CLI_IsEnabled(pEnv, x)                  (CLIENV(pEnv)->cliFlags &   (x))
#define CLI_NotEnabled(pEnv, x)                 (!(CLIENV(pEnv)->cliFlags & (x)))

#define MCONN_GetSubOption(pEnv)                (CLIENV(pEnv)->subOption)
#define MCONN_SetSubOption(pEnv, X)             (CLIENV(pEnv)->subOption = (X))
#define MCONN_OptBufferPtr(pEnv)                (XCHAR *) &(CLIENV(pEnv)->optBuffer)
#define MCONN_GetOptBufferIndex(pEnv)           (CLIENV(pEnv)->optBufferIndex)
#define MCONN_SetOptBufferIndex(pEnv, X)        (CLIENV(pEnv)->optBufferIndex = (X))
#define MCONN_GetRecvState(pEnv)                (CLIENV(pEnv)->recvState)
#define MCONN_SetRecvState(pEnv, X)             (CLIENV(pEnv)->recvState = (X))
#define MCONN_TermType(pEnv)                    (CLIENV(pEnv)->terminalType)

#define MCHAN_Env(pChan)                        ((t_XOS_GEN_ENVIRON *)((pChan)->env))
#define MCHAN_CliEnv(pChan)                     (MCHAN_Env(pChan)->pCli)

#define MMISC_GetOptHandled(pEnv)               (CLIENV(pEnv)->optHandled)
#define MSCRN_GetWidth(pEnv)                    (CLIENV(pEnv)->screenWidth)
#define MSCRN_GetHeight(pEnv)                   (CLIENV(pEnv)->screenHeight)
#define MSCRN_SetWidth(pEnv,  x)                (CLIENV(pEnv)->screenWidth  = (x))
#define MSCRN_SetHeight(pEnv, y)                (CLIENV(pEnv)->screenHeight = (y))
#define MEDIT_GetLength(pEnv)                   (CLIENV(pEnv)->cmdEditInfo.lineLength)
#define MEDIT_GetCursor(pEnv)                   (CLIENV(pEnv)->cmdEditInfo.cursorPos)
#define MEDIT_SetLength(pEnv, x)                (CLIENV(pEnv)->cmdEditInfo.lineLength = (x))
#define MEDIT_SetCursor(pEnv, y)                (CLIENV(pEnv)->cmdEditInfo.cursorPos  = (y))
#define MEDIT_GetKeyStat(pEnv)                  (CLIENV(pEnv)->cmdEditInfo.keyState)
#define MEDIT_SetKeyStat(pEnv, x)               (CLIENV(pEnv)->cmdEditInfo.keyState = (x))

#define MEDIT_BufPtr(pEnv)                      (CLIENV(pEnv)->cmdEditInfo.inputArray)
#define MEDIT_Prompt(pEnv)                      (CLIENV(pEnv)->prompt)
#define MEDIT_PromptLen(pEnv)                   (CLIENV(pEnv)->promptLength)
#define MEDIT_EditInfoPtr(pEnv)                 (&(CLIENV(pEnv)->cmdEditInfo))

#define MHIST_History(pEnv)                     (&(CLIENV(pEnv)->histInfo))
#define MHIST_TempBuf(hist)                     (hist->tempHist)

#define MMISC_GetChannel(pEnv)                  (CLIENV(pEnv)->pChannel)
#define MCONN_GetWriteHandle(pEnv)              (CLIENV(pEnv)->pCliWriteHandle)
#define MCONN_GetReadHandle(pEnv)               (CLIENV(pEnv)->pCliReadHandle)
#define MCONN_SetWriteHandle(pEnv, X)           (CLIENV(pEnv)->pCliWriteHandle = (X))
#define MCONN_SetReadHandle(pEnv, X)            (CLIENV(pEnv)->pCliReadHandle = (X))
#define MCONN_GetConnType(pEnv)                 (CLIENV(pEnv)->typeConn)
#define MCONN_SetConnType(pEnv, X)              (CLIENV(pEnv)->typeConn = (X))
#define MCONN_GetSock(pEnv)                     (CLIENV(pEnv)->pChannel->sock)
#define MMISC_GetRootParam(pEnv)                (CLIENV(pEnv)->pParamRoot)
#define MMISC_SetRootParam(pEnv,pList)          (CLIENV(pEnv)->pParamRoot = (pList))
#define MMISC_GetInput(pEnv)                    (CLIENV(pEnv)->chMore)
#define MMISC_SetInput(pEnv, depth)             (CLIENV(pEnv)->chMore = (depth))
#define MMISC_GetFsmId(pEnv)                    (CLIENV(pEnv)->fsmId)
#define MMISC_SetFsmId(pEnv, x)                 (CLIENV(pEnv)->fsmId = (x))

#define MMISC_GetAccess(pEnv)                   (pEnv->UserLevel)
#define MMISC_SetAccess(pEnv, access)           (pEnv->UserLevel = (access))
#define MMISC_PrevRootPtr(pEnv)                 (CLIENV(pEnv)->pPrevRoot)
#define MMISC_Login(pEnv)                       (CLIENV(pEnv)->login)
#define MMISC_Passwd(pEnv)                      (CLIENV(pEnv)->passwd)

#define MMISC_AliasCmd(pEnv)                    (CLIENV(pEnv)->aliasTable.aliasCmd)
#define MMISC_AliasPtr(pEnv)                   &(CLIENV(pEnv)->aliasTable)
#define MMISC_GetErrorPos(pTokens)              (pTokens->errorPos)
#define MMISC_SetErrorPos(pTokens, x)           (pTokens->errorPos = (x))
#define MMISC_GetErrorText(pEnv)                (CLIENV(pEnv)->errorText)
#define MMISC_SetErrorText(pEnv, msg)           STRNCPY(CLIENV(pEnv)->errorText, (msg), kCLI_ERROR_TEXT_SIZE)
#define MMISC_ClearErrorText(pEnv)              (*(CLIENV(pEnv)->errorText) = '\0')
#define MMISC_GetNodePrompt(x)                  (NULL != (x) ? (x)->pPrompt : NULL)
#define MMISC_GetLoginStat(pEnv)                (CLIENV(pEnv)->loginStat)
#define MMISC_SetLoginStat(pEnv, x)             (CLIENV(pEnv)->loginStat = (x))

#define MEDIT_CopyFromInput(pEnv, dest) \
        XOS_StrNcpy( (dest), CLIENV(pEnv)->cmdEditInfo.inputArray, kCLI_MAX_LOGIN_LEN)

#define ENVOUT(pEnv)                ((pEnv)->output)
#define MMISC_OutputPtr(pEnv)       (&(ENVOUT(pEnv)))
#define MMISC_OutputBuffer(pEnv)    (ENVOUT(pEnv).pOutput)

/* 操作宏定义*/
#define NULL_STRING(x)          ((NULL == (x)) || ('\0' == *(x)))
#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))
#define DIGIT_TO_CHAR(Digit)    ((XCHAR)(Digit - '0'))
#define CLI_MIN(x, y)   ( (x < y) ? (x) : (y) )
#define CLI_MAX(x, y)   ( (x > y) ? (x) : (y) )
#define TOUPPER(x)  ( (('a' <= x) && ('z' >= x)) ? (x - 'a' + 'A') : x )
#define TOLOWER(x)  ( (('A' <= x) && ('Z' >= x)) ? (x - 'A' + 'a') : x )

/*
#ifndef CLI_AUTH_CALLBACK_FN
#define CLI_AUTH_CALLBACK_FN    cli_taskValidateLogin
#endif
*/
#ifndef TELNET_SEND_FN
#define TELNET_SEND_FN XOS_CliTelnetSend
#endif

#ifndef TELNET_RECV_FN
#define TELNET_RECV_FN XOS_CliTelnetRecv
#endif

#define GET_PRINT_FLAG(out, flag)   (0 != ((out)->flags & (flag)))
#define SET_PRINT_FLAG(out, flag)   ((out)->flags |= (flag))
#define CLR_PRINT_FLAG(out, flag)   ((out)->flags &= ~(flag))
#define WHITE_SPACE(text)           ((' ' == (text)) || (kCR == (text)) || (kLF == (text)) || (kCHAR_NULL == (text)))

#define kPRINT_FLAG_NOPRINT  0x00000001 /* ignore print requests        */

/* XOS_SystemExit( XS32 s32ExitCode ) 输入参数标准说明 */
#define XOS_EXIT_SUCCESS 0 /*which represents a value of 0*/  
#define XOS_EXIT_FAILURE 1 /*which represents a value of 1. */
#define XOS_EXIT_FAILURE1 2 /* 其他值有待于扩展 */

#undef kEOL

#ifdef __EOL_USES_LF__
#define kEOL "\n"
#endif

#ifdef __EOL_USES_CR__
#define kEOL "\r"
#endif

#ifndef kEOL
#define kEOL "\r\n"
#endif

#define LOCAL_CLI_SESSION 0

/* 终端界面发送给 CLI Server 的字符串长度 */
#define SEND_STRING_LEN 2

#define MSG_IP_DATAIND    0
#define MSG_IP_CLOSEIND   1
#define MSG_IP_CONNECTIND 2
#define MSG_REMOTE_CLI_REQ 3
#define MSG_REMOTE_CLI_RSP 4

#define kEOL_SIZE (sizeof(kEOL) - 1)
/*系统命令根模式*/
#define SYSTEM_MODE 0
#define SYSTEM_EXIT_QUIT_INDEX 3

/*保存main函数的参数*/
#define MAIN_ARG_NUM    8       /*最多支持保留8个参数*/
#define MAIN_ARG_LEN    257      /*每个参数最大长度256字节，保留一个结束符*/

/*-------------------------------------------------------------
                  结构和枚举声明
--------------------------------------------------------------*/
enum _XOS_QUEUE_STATE
{
    QUEUE_None,
    QUEUE_Empty,
    QUEUE_Opposite
};

enum  _XOS_THREAD_STATES
{
    kThreadCreate,
    kThreadIdle,
    kThreadSnmpSync,
    kThreadWorking,
    kThreadFinished,
    kThreadDead
};

/*
#define kKEY_CONTINUE           '\\'
#define kKEY_SEPARATOR          ';'
**/
typedef enum _XOS_KEY_STATE
{
    KEY_STATE_INVALID,
    KEY_STATE_DATA,
    KEY_STATE_ESC,
    KEY_STATE_CURSOR
} e_XOS_KEY_STATE;

#define ExitCliShellMd  (CLI_MAX_CMD_CLASS - 1)

typedef struct  _XOS_GEN_ENVIRON   t_XOS_GEN_ENVIRON;
#define CLI_ENV t_XOS_GEN_ENVIRON
typedef XVOID (*CmdHandlerFunc) (CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

typedef XS32  Access;
//typedef XVOID (*CmdHandlerFunc) (CLI_ENV* pCliEnv, XS32 siArgc, XCHAR **ppArgv);

#ifdef XOS_MDLMGT
/*保留main函数的入参，供业务调用*/
XEXTERN XU32 g_main_argc;   /*argc，入参的数量*/
XEXTERN XS8  **g_main_argv;   /*argv,入参列表*/
#endif

#pragma pack(1)

/* CMD 参数*/
typedef struct _XOS_CMD_ARG
{
    XS32        siIdx;
    XS32        siArgcnt;
    XCHAR       pscArgval[CLI_MAX_ARG_NUM+1][CLI_MAX_ARG_LEN+1];
} t_XOS_CMD_ARG;

#define CLI_MAX_USER_LEN  16
#define CLI_MAX_PWD_LEN   16
#define CLI_MAX_USER_NUM  16
typedef enum e_USERCALSS
{
   eUserSuper=0,  /* 超级用户   */
   eUserAdmin,    /* 管理员用户 */
   eUserNormal    /* 操作员用户 */

} e_USERCALSS;

/* 帐户信息 */
typedef struct _XOS_CLI_USER_INFOR
{
    e_USERCALSS    eAccLvl;
    XCHAR   szUserName[CLI_MAX_USER_LEN + 1];
    XCHAR   szPassWord[CLI_MAX_PWD_LEN + 1];
    XS32    xs32Session;

} t_XOS_CLI_USER_INFOR;

/* 打印广播开关 */
typedef struct _XOS_PIRNT_BORAD_SWITCH
{
    XS32    xs32SID;
    XS32    xbSwitch;
} t_XOS_PIRNT_BORAD_SWITCH;

/* 单条CMD  结构*/
typedef struct _XOS_CMD_DATAS
{
    /*模式*/
    XS32         siFlag;

    /*CMD 处理函数*/
    CmdHandlerFunc   pCmdHandler;

    /*CMD 名称*/
    XCHAR   pscCmdName[ CLI_MAX_CMD_LEN + 1 ];

    /* CMD 帮助字符串*/
    XCHAR   pscCmdHelpStr[ CLI_MAX_CMD_HELP_LEN + 1];

    /* CMD 参数说明*/
    XCHAR   pscParaHelpStr[ CLI_MAX_ARG_LEN + 1 ];

    XS32    m_nParent;
    XS32    m_nMode;

    /*用户组 ID*/
    e_USERCALSS    eAccLvl;

} t_XOS_CMD_DATAS;

/* 命令行结构*/
typedef struct _XOS_CLI_SHELL_CMD
{
    /* 模式代表一个CMD 层次*/
    XS32              cliMode;
    XCHAR             pscCursor[CLI_MAX_ARG_LEN + 1];   /*模式提示符*/

    /*该配置模式下的command列表*/
    t_XOS_CMD_DATAS  *pCmdLst;
    t_XOS_CMD_ARG    cmdArg;
    XU32             ulPara0;
    XU32             ulPara1;

} t_XOS_CLI_SHELL_CMD;

typedef struct _XOS_CLI_AP_CMDS_MODE
{
    XCHAR pscCurSor[CLI_MAX_CURSOR_LEN + 1];
    t_XOS_CMD_DATAS   *pModePCmds;

} t_XOS_CLI_AP_CMDS_MODE;

typedef struct _XOS_OPTION_STATE
{
    XCHAR name;
    XU8   count;
    XU8   optState;
    XU8   queueState;
} t_XOS_OPTION_STATE;

typedef struct _XOS_PAIR_STATE
{
    XU8          option;
    XCHAR        desired;
    t_XOS_OPTION_STATE host;
    t_XOS_OPTION_STATE client;
} t_XOS_PAIR_STATE;

/*   TELNET 关联结构 */
typedef struct _XOS_CLI_LIST
{
    XVOID                    *pObject;       /* pointer to data object  */
    struct  _XOS_CLI_LIST    *pNext;         /* next object in the LIST */

} t_XOS_CLI_LIST;

typedef struct _XOS_LINE_OUT
{
    XU32 flags;                       /* output settings                      */
    XS32  maxSize;                    /* maximum buffer size                  */
    XS32  length;                     /* length of buffer contents            */
    XS32  offset;                     /* offset to text not yet printed       */
    XS32  lineCount;                  /* number of lines printed this time    */
    XCHAR  *pOutput;                  /* output buffer                        */
} t_XOS_LINE_OUT;

typedef struct _XOS_UDP_PARAMS
{
    XCHAR *pRecPacket;
    XCHAR *pSendPacket;
    XU32 clientAddr;
    XU32 sendPacketLength;
    XU32 recvPacketLength;
    XU16 clientPort;

} t_XOS_UDP_PARAMS;

typedef struct _XOS_CACHE_HANDLE                  /* basename:  cac */
{
    t_XOS_CLI_LIST    *p_lstCacheObjects;     /* next object in the LIST */
    Access          AccessType;                     /* cache access (Read/Write) */

} t_XOS_CACHE_HANDLE;

typedef struct _XOS_CMD_EDITINFO
{
    XS16  lineLength;     /* character count in line buffer     */
    XS16  cursorPos;      /* offset of cursor into buffer       */
    XS16  termX;          /* horizontal pos of cursor           */
    XS16  termY;          /* vertical pos of cursor             */
    XS16  startCol;       /* track before exec handler          */
    XS16  startRow;       /* track before exec handler          */
    e_XOS_KEY_STATE       keyState;
    XCHAR     inputArray[CLI_MAX_INPUT_LEN + 1];

} t_XOS_CMD_EDITINFO;

typedef struct _XOS_CMD_HIST_BUFF
{
    XCHAR  histCmd[CLI_MAX_INPUT_LEN + 1]; /* the command */
} t_XOS_CMD_HIST_BUFF;

/*-----------------------------------------------------------------------*/

typedef struct _XOS_HIST_INFO
{
    XS32 bufferIndex;                /* points to [iNumCmds]              */
    XS32 iMaxHistCmds;             /* max num hist cmds                 */
    XS32 iCurHistCmd;                  /* current hist cmd                  */
    XS32 iNumCmds;                   /* total number of commands issued   */
    XCHAR  tempHist[CLI_MAX_INPUT_LEN + 1]; /* current command when scrolling    */
    t_XOS_CMD_HIST_BUFF *pHistBuff;
} t_XOS_HIST_INFO;


typedef struct
{
    XS32 cliSessionId;
    XU32 cmdSeqNo;
    XU32 telnetIp;
    XU16 telnetPort;
    XS32 siArgc;
    XU8 buff[1];
}t_XOS_CLI_MSG_HEAD;
/**
CLI参数的传送使用多段字符组合的方式,V格式
ARGC | SOR  | ARG0 | ARG1 | ... | ARGN
*/
/*----t_XOS_GEN_ENVIRON---*/
struct  _XOS_GEN_ENVIRON                     /* (basename:  env) */
{
    t_XOS_CACHE_HANDLE* phCacheHandle;
    Access              UserLevel;

    XU32                IpAddr;
    XS32                clientIndex;

    XCHAR               PostValid;
    XCHAR*              PostBuffer;
    XU32                PostBufferLen;

#ifdef __ENABLE_CUSTOM_STRUCT__
    XVOID*               pCustomData;            /* custom session data */
#endif

    struct  _XOS_CLI_INFO   *pCli;
    t_XOS_LINE_OUT           output;
    // 20110107 cxf add
    XU32 RemoteBid;
    t_XOS_CLI_MSG_HEAD stCliMsgHead;
    XCHAR *pStrBuf;
};

typedef XS32 WriteHandle(t_XOS_GEN_ENVIRON *, const XCHAR *pBuf, XS32  BufSize);
typedef XS32 ReadHandle(t_XOS_GEN_ENVIRON *, XU8, XCHAR *pBuf, XS32 *bytesReturned);

typedef struct _XOS_CLI_INFO
{
    XU32                      cliFlags;
    XS32                      chMore;
    XCHAR                     loginStat;
    XS32                      fsmId;
    struct _XOS_COM_CHAN      *pChannel;
    struct _XOS_GEN_ENVIRON   *pEnvironment;
    XCHAR                     login[kCLI_MAX_LOGIN_LEN + 1];
    XCHAR                     passwd[kCLI_MAX_PASSWORD_LEN + 1];
    XU32                      promptLength;
    t_XOS_CMD_EDITINFO        cmdEditInfo;
    XCHAR                      errorText[kCLI_ERROR_TEXT_SIZE + 1];
    t_XOS_PAIR_STATE          optHandled[kCLI_MAX_OPT_HANDLED];
    XU8                 subOption;
    XU8                 recvState;
    XU32                typeConn;

    WriteHandle         *pCliWriteHandle;
    ReadHandle          *pCliReadHandle;
    XS32                optBufferIndex;
    XCHAR               optBuffer[kCLI_OPT_BUF_SIZE];

    XCHAR               terminalType[kCLI_TERM_TYPE_SIZE];
    XS16                screenWidth;
    XS16                screenHeight;

    t_XOS_CLI_SHELL_CMD       cliShellCmd;
    XVOID               *pCustom;  /* customer defined */

    t_XOS_HIST_INFO            histInfo;

} t_XOS_CLI_INFO;

/*-----------------------------------------------------------------------*/
typedef struct _XOS_COM_CHAN
{
    XCHAR             InUse;      /* if TRUE, there is a request on this com channel ... */
    XS32              ThreadState;
    XU32              IpAddr;
    XU32              index;

    XVOID*            env;

} t_XOS_COM_CHAN;

/* 版本信息 */
typedef struct _XOS_CLI_VER
{
    XU32    index;
    XCHAR   szVerName[CLI_MAX_INPUT_LEN];
    XCHAR   szVerValue[CLI_MAX_INPUT_LEN];
    XBOOL   verFlag;
} t_XOS_CLI_VER;

/*20110215 CXF ADD*/
typedef enum
{
    CLI_FUNCID_GETPIDLIST = 1, //取PID列表
    CLI_FUNCID_GET_REMOTE_CLIPATH = 2, //取远端路径
    CLI_FUNCID_CHECK_PID_PATH = 3, // 检查PID是否合法
    CLI_FUNCID_MAX
}CLI_FUNCID_E;

#define MAX_CLI_REMOTE_PID_CN  100 // 最大无端PID个数
#define CLI_PRE_FIX_LEN 128

/*check cliPath - pid if valid*/
typedef int (*CliFunc_checkPathPid)(char *cliPath,XU32 PID);

/*get revmote pid list ,max pid count=100*/
typedef int  (*CliFunc_CliGetPidList)(int strLen, char* pcliPath, int* pidNum, unsigned int* pidList);
/*get remote cli Path*/
typedef int (*CliFunc_getRemoteCliPath)(char *szlocalPath, char *szOutRemotePath);
//

typedef struct
{
    CliFunc_checkPathPid pChkPathPidFunc;
    CliFunc_CliGetPidList pGetPidListFunc;
    CliFunc_getRemoteCliPath pGetRemoteCliPathFunc;
}CLI_FUNC_REG_T;    
#pragma pack()
extern t_XOS_PIRNT_BORAD_SWITCH g_paUserSIDLogined[kCLI_MAX_CLI_TASK+1];

/*-------------------------------------------------------------------------
                  API 声明
-------------------------------------------------------------------------*/
/************************************************************************
 函数名: XOS_RegAppVer 2007/11/22加入本功能,用于现场版本管理和维护
 功能:   提供各模块注册版本管理信息的接口.
 输入:   AppVerName:       模块及应用程序名称
         strVerValue:      模块及应用程序版本字符串描述
 输出:   无
 返回:   成功返回命令ID/失败返回XERROR,或者错误码
 说明:
************************************************************************/
XS32 XOS_RegAppVer(XCONST XCHAR*  AppVerName,XCONST XCHAR*  strVerValue);
XCHAR * XOS_GetVersion(XVOID);
XS32 XOS_RegNeVer(XCONST XCHAR* strVerValue);
XCHAR* XOS_GetNeVersion(XVOID);
XPUBLIC XVOID XOS_SystemExit( XS32 s32ExitCode );

/************************************************************************
 函数名: XOS_RegAppVer 2007/12/12加入本功能
 功能:   关闭程序启动成功后,关闭串口错误的打印功能
 输入:
 输出:   无
 返回:
 说明:
************************************************************************/
XS32 XOS_CloseVxPrint();

/************************************************************************
 函数名: XOS_RegistCmdPrompt(  )
 功能:   注册提示符命令.
 输入:   prarenPromptID: 上级父提示符 ID
         pscCmdName:     CMD 名称
         pscCmdHelpStr:  CMD 帮助字符串
         pscParaHelpStr: CMD 参数说明

 输出:   无
 返回:     成功返回命令ID/失败返回XERROR,或者错误码
 说明:
************************************************************************/
XS32 XOS_RegistCmdPrompt(   XCONST XS32 prarenPromptID,
                            XCONST XCHAR* pscCmdName,
                            XCONST XCHAR* pscCmdHelpStr,
                            XCONST XCHAR* pscParaHelpStr);

/************************************************************************
 函数名: XOS_RegistCommand(  )
 功能:   根据提示符 ID 注册子命令.
 输入:   promptID: 提示符 ID
         pCmdHandler:    CMD 处理函数
         pscCmdName:     CMD 名称
         pscCmdHelpStr:  CMD 帮助字符串
         pscParaHelpStr: CMD 参数说明

 输出:   无
 返回:     成功返回命令ID/失败返回XERROR,或者错误码
 说明:
************************************************************************/
XS32 XOS_RegistCommand( XCONST XS32 promptID,
                        CmdHandlerFunc pCmdHandler,
                        XCONST XCHAR* pscCmdName,
                        XCONST XCHAR* pscCmdHelpStr,
                        XCONST XCHAR* pscParaHelpStr );

/************************************************************************
 函数名: XOS_RegistCmdPromptX(  )
 功能:   注册提示符命令.
 输入:   prarenPromptID: 字符串类型,上级父提示符 ID.
         pscCmdName:     字符串类型,CMD 名称.
         pscCmdHelpStr:  字符串类型,CMD 帮助字符串.
         pscParaHelpStr: 字符串类型,CMD 参数说明.
         eAccess:        e_USERCALSS 类型, 命令权限级别.

 输出:   无
 返回:     成功返回提示符命令ID/失败返回XERROR,或者错误码
 说明:
************************************************************************/
XPUBLIC XS32 XOS_RegistCmdPromptX( XCONST XS32   prarenPromptID,
                                   XCONST XCHAR* pscCmdName,
                                   XCONST XCHAR* pscCmdHelpStr,
                                   XCONST XCHAR* pscParaHelpStr,
                                   e_USERCALSS   eAccess );

/************************************************************************
 函数名: XOS_RegistCommandX(  )
 功能:   根据提示符 ID 注册子命令.
 输入:   promptID:       提示符 ID
         pCmdHandler:    CMD 处理函数
         pscCmdName:     CMD 名称
         pscCmdHelpStr:  CMD 帮助字符串
         pscParaHelpStr: CMD 参数说明
         eAccess:        e_USERCALSS 类型, 命令权限级别.

 输出:   无
 返回:     成功返回命令ID/失败返回XERROR,或者错误码
 说明:
************************************************************************/
XS32 XOS_RegistCommandX( XCONST XS32    promptID,
                         CmdHandlerFunc pCmdHandler,
                         XCONST XCHAR*  pscCmdName,
                         XCONST XCHAR*  pscCmdHelpStr,
                         XCONST XCHAR*  pscParaHelpStr,
                         e_USERCALSS    eAccess );

/************************************************************************
 函数名: XOS_CliInforPrintf(  )
 功能:   带格式的输出函数,输出信息可以重定向到Telnet 客户端
 输入:   pFmt 格式话字符串
 输出:   无
 返回:     无
 说明:   无
************************************************************************/
XPUBLIC XS32  XOS_CliInforPrintfStr( const XCHAR* buff );
XPUBLIC XS32  XOS_CliInforPrintfSessionStr(const e_PRINTLEVEL *pFidLevel,e_PRINTLEVEL elevel,const XCHAR* buff );
XPUBLIC XS32  XOS_CliInforPrintf( const XCHAR* pFmt, ... );
XPUBLIC XS32  XOS_CliExtPrintf(CLI_ENV* pCliEnv, const XCHAR* pFmt, ...);
XPUBLIC XS32  XOS_CliExtWriteStr(CLI_ENV *pCliEnv, const XCHAR *pBuf);
XPUBLIC XS32  XOS_CliExtWriteStrLine(CLI_ENV *pCliEnv, const XCHAR *pBuf);

/*----------------------------------------------------------------------
            内部接口
------------------------------------------------------------------------*/
/************************************************************************
 函数名: XOS_TelnetBroadcastMessage(  )
 功能:   向所有 Telnet 客户端广播消息
 输入:   pMessage: 用户输入的字符
         authLevel
 输出:   无
 返回:     成功返回XSUCC/失败返回XERROR,或者错误
 说明:
************************************************************************/
XPUBLIC XS32 XOS_TelnetBroadcastMessage( XCHAR *pMessage, Access authLevel );

XPUBLIC XS8   XOS_CLIIniProc( XVOID *t, XVOID *v );
XPUBLIC XS8   XOS_CLIMsgProc( XVOID *t, XVOID *v );
XPUBLIC XS8   XOS_CLICloseProc( XVOID *t, XVOID *v );
XPUBLIC XVOID cli_UTILInit(CLI_ENV *pCliEnv);
XPUBLIC XVOID CLI_locConsoleEntry(XVOID);
XPUBLIC XVOID cli_printf_switch_cmd( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );
XPUBLIC XS32  XOS_DaemonInit( XS32 argc, XCHAR* argv[] );
XPUBLIC XS32  XOS_Ha_DaemonInit( XS32 argc, XCHAR* argv[] );

XPUBLIC XVOID cli_log_debug_cmd( CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv );

typedef enum LogLevel
{
    eCLINormal,
    eCLIError,
    eCLIDebug
} e_LOGLEVEL;
XVOID cli_log_write( e_LOGLEVEL nLevel, XCONST XCHAR* fmt, ... );
XVOID cli_log_debug( XBOOL bFlag  );

#ifdef XOS_TELNETS
typedef struct _TelData
{
    XU32 uLong;
    char buffer[256];
} TEL_DTATA;

XS8 TelnetServerSend2Client( XU16   uSubID, XCHAR* pData, XU32 u32Len );
#endif

/*## 获取可执行文件和xml文件的完全路径*/
#ifdef XOS_EW_START

#ifndef XOS_MAX_PATHLEN
#define XOS_MAX_PATHLEN 255
#endif

XS32 CLI_GetFullPathInfo( XVOID);
/*## 获取可执行文件和xml文件的完全路径*/
XS32 XOS_GetSysPath( XCHAR *szSysPath, XS32 nPathLen);
XCHAR* XOS_CliGetSysName( XVOID);

#endif

XPUBLIC XCHAR* XOS_CliGetXmlName( XVOID);

XVOID  cli_setMcbPid(XU32 ulMcbPid);
XU32   cli_getMcbPid(XVOID);
char * Cli_SetCliPrefixInfo(char *szPrefixInfo);
XVOID XOS_ShowRunMode(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv);

int XOS_RegAllCliCmd(int cmdMode);

#ifdef XOS_MDLMGT
XS8 **XOS_getMainArg(XU32 *argc);
#endif

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLISHELL_H_ */

