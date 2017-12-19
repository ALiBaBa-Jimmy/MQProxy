#ifndef _SAAP_DEF_H
#define _SAAP_DEF_H
#include "xosshell.h"
#ifdef  __cplusplus
extern  "C"
{
#endif

#define MAX_IP_DATA_LEN  (6*1460) //20101021 cxf mod it 4->6 ,because zyf say dataLen may >6k

#ifdef SCALE_CPU_VX
#define CANCLE_PACK #pragma pack(0)
#else
#define CANCLE_PACK #pragma pack()
#endif

enum
{
    SYS_IP_TYPE_NOTUSE = 0,
    SYS_IP_TYPE_TCP,
    SYS_IP_TYPE_UDP,
    SYS_IP_TYPE_RAW,
    SYS_IP_TYPE_MAX
};

#define SAAP_MAX_TELNO_LENGTH       8           /*最大的电话号码字节数8888*/
#define MAX_DIGITAL_LEN             64          /*最大号码长度8888*/
#define DIGITAL_BUFF_LEN            32          /*号码缓冲区长度*/
#define BCD_NUM_MASK                0x0f
#define BCD_NUM_MIN_VAL             0x00
#define BCD_NUM_MAX_VAL             0x0c

#define BCD_NUM_ZERO                0x0a
#define BCD_NUM_STAR                0x0b
#define BCD_NUM_WELL                0x0c

#define SVC_NATAP_IP_CFG        0x01                 /* 业务配置NATAP链路的消息 */
#define SVC_NATAP_IP_DEL        0x02                 /* 业务删除NATAP链路的消息 */
#define NATAP_SVC_CFG_ACK       0x03                 /* NATAP给业务的链路配置响应消息，带上NATAP链路号 */
#define NATAP_SVC_CFG_IND       0x04                 /* NATAP给业务的链路配置指示消息，带上NATAP链路状态 */

#define NATAP_SVC_ERR_IND       0x05                 /* NATAP给业务的错误指示消息 */
#define SVC_NATAP_ERR_ACK       0x06                 /* 业务给NATAP的错误响应，带上与错误指示一致的原因 */

#define SVC_NATAP_SVC_REQ       0x07                 /* 业务给NATAP的业务请求消息，表示要发送数据给NATAP */
#define NATAP_SVC_SVC_IND       0x08                 /* NATAP给业务的业务指示消息，表示要发送数据给业务 */
#define NATAP_SVC_ALARM         0x09                 /* NAT模块发送给上层的告警消息ID */

/* -----------------------统计相关-------------------------------------------*/
#define STAT_TO_SVC_CFG            0xcc10            /* 统计配置消息号    */
#define STAT_TO_SVC_START_REQ      0xcc11            /* 统计周期消息号    */
#define STAT_TO_SVC_START_RSP      0xcc12            /* 统计周期响应消息号*/
#define STAT_TO_SVC_CLEAR_DATA     0xcc13            /* 统计清除消息号    */


/*TCPEN相关定义*/
#define MAX_TCPEN_LINK_NUM          64       //最大tcp封装链路数
#define DEFAULT_TCPEN_STAT_LINKID   61       //默认统计台连接链路号
#define DEFAULT_TCPEN_MEDIA_LINKID  62       //默认媒体连接链路号
#define DEFAULT_TCPEN_RNMS_LINKID   63       //默认网管连接链路号
#define STAT_DPID                  ((BLANK_USHORT)-1)
#define RNMS_DPID                  (BLANK_USHORT)   //没有使用此宏,
#define MED_DPID                   ((STAT_DPID) - 1)//没有使用此宏,

//#define FID_MM 1
//#define FID_CC 3

#define FID_TCPE           1701
#define FID_SAAP           1702

//字符串转换为数字函数宏定义
#define XOS_ATOF(str)                     (atof((const char *)str))   //convert a string to a double (ANSI)
#define XOS_ATOI(str)                     (atoi((const char *)str))   //
#define XOS_ATOL(str)                     (atol((const char *)str))   //10进制

//long->字符串
#define XOS_LTOA(lValue,pStr,nRadix) ltoa((lValue),(pStr),(nRadix))
#define XOS_ULTOA(ulValue,pStr,nRadix) _ultoa((ulValue),(pStr),(nRadix)) //9999

//9999 below
#define SYS_RETURN return
//9999 above

//消息所有者属性结构
typedef struct
{
    XU32   ucModId;           //模块号
    XU32   ucFId;             //功能块号
    XU32   usFsmId;           //内部连接号
}MSG_OWNER_SAAP;

//公共消息头结构
typedef struct tagCommonHeader_saap
{
    MSG_OWNER_SAAP   stSender;       //发送者
    MSG_OWNER_SAAP   stReceiver;     //接收者
    XU16     usMsgId;                //消息类型
    XU16     subID;                  //消息子类型

    XU32     prio;                   //消息优先级,定义成与新平台消息头一样可提高转换效率
    XU32     usMsgLen;               //消息长度
    void     *message;               //消息指针；
}COMMON_HEADER_SAAP;                 //此消息应与XOS平台消息头格式环境

//通用简单消息结构
typedef struct tagCommonMessage_saap
{
    COMMON_HEADER_SAAP stHeader;
    XU8 ucBuffer[1];
}COMMON_MESSAGE_SAAP;

//-------------------------------枚举定义------------------------------------
typedef struct tagSysIPAddr
{
    XU32      ip;     //IPV4地址
    XU16      port;   //端口号
}SYS_IP_ADDR;


#define SWITCH_ON               1
#define SWITCH_OFF              0

#define FLAG_YES                1
#define FLAG_NO                 0
#define SAAP_MAX_SPA_CLI_NUM    16

/*wzy add */
#define IN           /*输入参数的标志*/
#define OUT          /*输出参数的标志*/
#define SAAP_MAX_MSG_LEN        (8*1024) //0x7fff    //板间消息最大长度

#ifdef XOS_LINUX
#define MD( ulFid, ulLevel) (XCHAR*)(__FILE__), (XU32)(__LINE__), (XCHAR*)(__FUNCTION__), (XU32 )(ulFid), (e_PRINTLEVEL)(ulLevel)
#else
#define MD( ulFid, ulLevel) (XCHAR*)(__FILE__), (XU32)(__LINE__), NULL, (XU32 )(ulFid), (e_PRINTLEVEL)(ulLevel)
#endif

#define SAAP_SWAP_HIGH4_BIT(VAL) ( ( (VAL & 0xF) << 4) | (VAL >> 4) )

//IP模块与上层之间的消息ID
enum
{
    //IP部分消息
    MSG_IP_CONFIG = 1,          //配置          OAM->IP
    MSG_IP_ADDRCHANGE,          //IP地址改变    OAM->IP
    //MSG_IP_CONNECTIND,        //连接指示      IP->USER
    MSG_IP_CLOSEREQ,            //关闭请求      USER->IP
    //MSG_IP_CLOSEIND,          //关闭指示      IP->USER
    MSG_IP_DATAREQ,             //数据请求      USER->IP
    //MSG_IP_DATAIND,           //数据指示      IP->USER
    MSG_IP_CFGACK,              // 配置确认     IP->USER
    MSG_IP_REFRESH,             // 刷新链路    USER->IP
#ifdef SYS_IP_LINK_STATIC
    MSG_IP_STATIC_RESET,        //统计复位
#endif

    //LAPD部分消息
    MSG_LAPD_CONFIG,            //配置          OAM->LAPD
    MSG_LAPD_DLESTABLISHREQ,    //建立请求      OAM->LAPD
    MSG_LAPD_DLRELEASEREQ,      //释放请求      OAM->LAPD
    MSG_LAPD_DLDATAREQ,         //数据请求      USER->LAPD
    MSG_LAPD_DLDATAIND,         //数据指示      LAPD->USER
    MSG_LAPD_STATUSIND,         //状态指示      LAPD->OAM

    MSG_EXIT,                   //退出
    MSG_MAX
};


/* TCPE 统计结构 */

#define  TCP_LINT_STAT_MAX       MAX_TCPEN_LINK_NUM  //9999
typedef enum
{
    eSendMsg=10,
    eSendErrMsg,
    eRecvMsg,
    eRecvErrMsg,
    eSendFail,
    eLinkTry,
    eLinkHand,
    eLinkHandAck,
    eLinkLose,
    eLinkShutStop,
    eOtherStat
}e_TCPE_STAT;

typedef struct
{
    //XU16                usLinkIdx;             // 链路ID，如果为BLANK_USHORT则不需要统计
    XU32                  ulSendMsgCnt;          // 链路发送的消息包计数
    XU32                  ulSendErrMsgCnt;       // 链路发送的错误消息包丢弃计数
    XU32                  ulRecvMsgCnt;          // 链路收到的消息包计数
    XU32                  ulRecvErrMsgCnt;       // 链路收到错误消息包丢弃计数
    XU32                  ulSendFailCnt;         // 发送失败次数[收到NTL的send err
    XU32                  ulLinkTryCnt;          // 链路重建链计数
    XU32                  ulLinkHand;            // 链路握手消息
    XU32                  ulLinkHandAck;         // 链路握手应答消息
    XU32                  ulLinkLoseCnt;         // 链路失连计数
    XU32                  ulLinkShutStop;        // 链路关闭次数统计
    XU32                  ulOther;               // 不可统计的错误计数
    XU32                  ulSendFailReason[eOtherErrorReason+1];
}t_TCPE_LINKSTAT;

typedef struct
{
    t_TCPE_LINKSTAT msgStat[TCP_LINT_STAT_MAX];
    t_TCPE_LINKSTAT msgIn;
}t_TCPESTAT;//结构中为全F的值不打印, 初始化为全F

typedef struct
{
    XU32 busyflag;       //是否有效客户端
    XU32 IpAddr;
    XU32 port;
    XU8 ucCodePlan;      //编号计划
    XU32 gtValue;        //GT
    XU32 ulSend2MntFlag; //发送到MNT标识,匹配过滤条件后置1,输出后清0
}SAAP_SPA_CLI_STRU_T;


typedef struct
{
    XU32 ulAllSendFlag; // 全部输出标识
    XU32 ulTraceFlag;
    XU32 cliNum;
    XU32 linkHandle;
    XU32 ulTraceId;
    SAAP_SPA_CLI_STRU_T cliData[SAAP_MAX_SPA_CLI_NUM];
}SAAP_SPA_CLI_DATA_T;

extern XS32 SYS_XOSMsg2CpsMsg(t_XOSCOMMHEAD *pxosMsg,COMMON_HEADER_SAAP *pCpsMsg);
extern XS32 SAAP_SockGetV4Addr(XCHAR *cIP, XU32 *addr);
extern int  SYS_MSGSEND( COMMON_HEADER_SAAP *psCpsMsg);
extern int  SAAP_StartTimer(PTIMER *ptHandle,XU32 fid,XU32 len,XU32 tmName,XU32 para,XU32 mode );
extern XS32 SYS_Str2Bcd(XCHAR * bcdstr);
extern char* SAAP_IptoStr( XU32 inetAddress , char *pString );
extern XS32 SAAP_StrTelToDn(XCHAR *pStrTel, XU8 *pDn );
extern int  TCPE_SendMsg2Ip( COMMON_HEADER_SAAP *psCpsMsg);
extern XU32 SAAP_MsgNeedToMnt(COMMON_HEADER_SAAP *psCpsMsg);
extern XU32 SAAP_SendMsgToMnt(COMMON_HEADER_SAAP *psCpsMsg);
XU32 SAAP_FilterSaap2Tcpe(COMMON_HEADER_SAAP *pstMsg);
XU32 SAAP_FilterSaap2Srv(COMMON_HEADER_SAAP *pstMsg);
#ifdef  __cplusplus
}
#endif
#endif


