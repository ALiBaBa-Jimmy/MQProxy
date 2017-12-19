/*----------------------------------------------------------------------
    tcpOamProc.h - 局部宏、类型、函数

    版权所有 2004 -2006 信威公司深研所SAG项目组.

    author: 张海

    修改历史记录:wangcaiyu
    修改历史记录:zengguanqun 2008-10-08
    --------------------
    添加版权说明.
----------------------------------------------------------------------*/

#ifndef _TCPOAMPROC_H_
#define _TCPOAMPROC_H_

#include "oam_public_data_def.h"
#include "oam_tab_def.h"
#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

//------------------------宏定义------------------------------------
#define SAG_PERFORM_STAT_INCLUDE 1
#define  TCP_LINT_MAX       MAX_TCPEN_LINK_NUM

/* fsm status */
#define   LINT_CCB_NULL                          0   // 空
#define   LINT_CCB_USER                          1   // 已使用

//定时器宏,加起始偏移800,区别默认值
#define   TCP_HAND_TIMER                         801   // 链路握手
#define   TCP_TIMER_INIT                         901
#define   TCP_TIMER_START                        902
//#define   TCP_LINK_INIT                          802   //初始化
//#define   TCP_LINK_START                         803   // 配置

#define   TCP_TWO_SECOND                         2000   // 2 秒

//------------------------结构和共用体定义----------------------------
XEXTERN XU8 g_Tcp_LinkHand;      // 1 发送, 0 不发送
XEXTERN XU8 g_Tcp_Trace;         // 1 发送, 0 不发送

XEXTERN t_TCPESTAT  gTcpeStaData;

#define TCPE_LINK_DESC_LEN  32

//不用agent编译时使用此选定义
#if 0
typedef enum
{
    TCPE_LOCOL_DP_TABLE = 1,
    TCPE_TCP_CFG_TABLE = 2, //tcpe链路配置
    TCPE_CFG_TABLE_BUTT
}TCPE_TABLE;
#endif

#if 1
typedef enum
{
    TCPE_LOCOL_DP_TABLE = xwTcpeLocalTable,
    TCPE_TCP_CFG_TABLE  = xwTcpeLinkTable, //tcpe链路配置
    TCPE_CFG_TABLE_BUTT
}TCPE_TABLE;
#endif

// TCP/IP封装层链路数据结构
typedef struct _tagTcpLintCcb
{
    int i;

    XU16                  usIndex ;       // 索引值
    XU8                   ucFlag;         // 0 为空

    XU8                   ucModel ;       // 连接模式
    XU8                   ucHandState;        // 状态 握手是否正常

    MSG_OWNER_SAAP        stUser;         // 业务使用者(用于IP层的通信)
    MSG_OWNER_SAAP        stProvider;     // 业务提供者(用于IP层的通信)
    SYS_IP_ADDR           stPeerAddr;     // 对端的IP地址
    SYS_IP_ADDR           stMyAddr;       // 自己的IP地址

    XU16                  DstDeviceID ;   // 目的信令点编码

    //用于握手类数据结构
    PTIMER                htTm;           // 定时器句柄
    PTIMER                linkTm;           // 定时器句柄
    XU16                  hankCount;      // 握手记数器，每发送一次请求 + 1，收到一次应答 -1

    XU16                  bufLen ;                        // 记录缓冲包的长度
    XU8                   ucBuffer[ MAX_IP_DATA_LEN ];    // 接收消息缓冲区

    //add by wangcaiyu 2006-11-29
    XU8                  ucType;                         // 目的信令点的类型 0 SSTP 1 HLR 2 MCBTS
    XU16                 usNatLinkIdx;                   // NAT模块分配的链路号索引

    XU8                  ucArrLinkDesc[TCPE_LINK_DESC_LEN]; //32 +1
    XU32                 ulLinkStatus;                      //TCP连接状态
    XU32                 ulAppHandle;
    XU32                 ulLinkHandle;
    XS32                 ulDefultLinkFlag;
    //end by wangcaiy*/

}TCP_LINT_CCB;

// TCP/IP封装层链路数据结构
typedef struct _tagTcpLintCcbIndex
{

    XU16                    GlobalDstID;           // 本地信令点编码
    XU16                    ProIndex;              // 默认上一级节点索引
    XU16                    cout;                  // 当前总链路数
    TCP_LINT_CCB            lintCcb[TCP_LINT_MAX];

}TCP_LINT_CCB_INDEX;

/*本地信令点*/
typedef struct _tagTcpLocaoDp
{
    XS32 LocolDp;//8888
}T_TCPE_LOCOLDP_TBL;

//TCPE链路配置
typedef struct _tagTcpeTcpTbl
{
    XS32     ulInkIndex;        //链路索引
    XS32     DstDpId;           //目的信令点
    XS32     connectType;       // 2-客户端,3-服务器
    XS32     localIp;           //本端ip
    XS32     localPort;
    XS32     remoteIp;
    XS32     remotePort;
    XS32     defaultLinkFlag;
    XU8      szLinkDesc[TCPE_LINK_DESC_LEN*2];
    XS32     linkStatus;
}T_TCPE_LINK_TBL;

//tcp链路状态
enum TCPE_TCP_LINK_STATE
{

    TCPE_TCP_LINK_STATE_NULL        =0,        //未分配
    TCPE_TCP_LINK_STATE_INIT        =1,        //初始化
    TCPE_TCP_LINK_STATE_START       =2,        //启动
    TCPE_TCP_LINK_STATE_BUILD       =3,        //建立

    TCPE_TCP_LINK_STATE_STOP        =4,        //停止

    TCPE_TCP_LINK_STATE_INIT_FAIL   =5,    //初始化失败
    TCPE_TCP_LINK_STATE_START_FAIL  =6,    //启动失败
    TCPE_TCP_LINK_STATE_LISTEN      =7,        //服务器端 listen
    TCPE_TCP_LINK_STATE_CONNING     =8,       //客户端正在连接
    TCPE_TCP_LINK_STATE_BUTT
};

enum TCPE_TCP_ALARM_STATE
{
    TCPE_TCP_ALARM_STATE_CON       =0,         //连接建立
    TCPE_TCP_ALARM_STATE_DISC      =1,         //连接未建立

    TCPE_TCP_ALARM_STATE_BUTT
};

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

// TCP lint 链路初始化
XS32 tcpe_link_inittable( void );

// 此函数完成本机信令点编码设置 , 注 TCPECAP 层只关心信令点，设备ID为GT码，由SAAP层处理
XU32 tcp_SetLocalDstID(XU16 LocId );

// 此函数完成上一级信令点的配置
XU32 tcp_SetParentDstIDIndex(XU16 index );

XS32 TCP_command_init();

void TcpTblRegToOAM(XU32 uiModId);
XU8  TcpeOamCallBack(XU32  uiTableId, XU16 usMsgId, XU32 uiSequence,XU8 ucPackEnd, tb_record *ptRow);

//tcpe localDp table oam msg process
XS32 Tcpe_oamLocalDpTableOpr(XU16 usMsgId, T_TCPE_LOCOLDP_TBL* pTcpTbl, char* mask);
XS32 Tcpe_oamSyncLocalDpTable(T_TCPE_LOCOLDP_TBL * pLocalDpTbl);
XS32 Tcpe_oamUpdateLocalDpTable(T_TCPE_LOCOLDP_TBL * pLocalDpTbl, char* mask);

//tcpe table oam msg process
XS32 Tcpe_oamLinkTableOpr(XU16 usMsgId, T_TCPE_LINK_TBL* pTcpTbl, char* mask);
XS32 Tcpe_oamSyncLinkTable(T_TCPE_LINK_TBL* pTcpTbl);
XS32 Tcpe_oamInsertLinkTable(T_TCPE_LINK_TBL* pTcpTbl);
XS32 Tcpe_oamUpdateLinkTable(T_TCPE_LINK_TBL* pTcpTbl, char* mask);
XS32 Tcpe_oamDeleteLinkTable(T_TCPE_LINK_TBL * pTcpTbl);
XS32 Tcpe_oamGetLinkTableStatus(T_TCPE_LINK_TBL * pTcpTbl);

//link maintenance
XS32 tcpe_link_delete(XU32 ulLinkIndex);
XS32 tcpe_link_clearstatus(XU32 ulLinkIndex);
XS32 tcpe_link_setstatus(XU32 ulLinkIndex,XS32 linkStatus);
XS32 tcpe_link_clearbuff(XU32 ulLinkIndex);
XS32 tcpe_link_clearstat(int link_index);

//link alert ntl msg trigger
XS32 tcpe_link_init(XU32 ulLinkIndex);
XS32 tcpe_link_initack( t_LINKINITACK* pLinkInitAck);
XS32 tcpe_link_startack(t_STARTACK* pLinkStartAck);
XS32 tcpe_link_errorsend(t_SENDERROR* pIpSendErrMsg);
XS32 tcpe_link_start(XU32 ulLinkIndex );
XS32 tcpe_link_release(XU32 ulLinkIndex);

XS32 tcpe_link_alarm(XU8 ucAlarmState, XS32 ulLinkIndex);
XVOID Tcp_cmdShowUserFid(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XVOID Tcp_cmdSetLocalTestFlag(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
extern XU16 TCPE_GetLocalDpId();
#ifdef  __cplusplus
}
#endif

#endif


