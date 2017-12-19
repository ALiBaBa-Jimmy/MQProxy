/*----------------------------------------------------------------------
    saapMain.h - 全局宏与类型定义

    版权所有 2004 -2006 信威公司深研所SAG项目组.

    author: 张海

    修改历史记录
    --------------------
    添加版权说明.
----------------------------------------------------------------------*/

#ifndef _SAAPMAIN_H_
#define _SAAPMAIN_H_

#include "saappublichead.h"

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

//------------------------宏定义------------------------------------

    #define SAAP_TIMER_NUM          3
    #define SAAP_TIMER_TEST         0 //测试定时器

    #define SAAP_G_DN_GT            0x12
    #define SAAP_G_EID_GT           0x82
    #define SAAP_G_UID_GT           0x92

    #define SAAP_REG_TIMER_NUM      512      // 注册的最大定时器数目

    // hand 握手类
    #define SAAP_USER_TYPE          0x1100   // TCP/IP 中的SAAP用户类型
    #define SAAP_ROUTER_MAX_NUM     32       // 允许最大经过我节点数

    /* fsm status */
    #define   SAAP_LINT_RUNING      0        // 正常
    #define   SAAP_LINT_FAIL        1        // 不通

    #define SAAP_GUEST_TAG          0x20     // 目的标签
    #define SAAP_INVITE_TAG         0x21     // 源标签

    // OAM 配置宏
    #define   SAAP_OAM_CF_SET_GT_INFO         0xC005        // 配置HLR GT编码
    #define   SAAP_OAM_CF_DEL_GT_IFNO         0x0F          // 删除HLR GT编码

    #define   SAAP_OAM_CF_SET_EID_ROUTE       0xC00C        // 配置设备号路由选择
    #define   SAAP_OAM_CF_DEL_EID_ROUTE       0xC00D        // 删除设备号路由选择
    #define   SAAP_OAM_CF_SET_HLRNO_ROUTE     0xC00E        // 配置SHLR_NO路由选择
    #define   SAAP_OAM_CF_DEL_HLRNO_ROUTE     0xC00F        // 删除SHLR_NO路由选择
    #define   SAAP_OAM_CF_SET_TEL_ROUTE       0xC010        // 配置号码局向路由选择
    #define   SAAP_OAM_CF_DEL_TEL_ROUTE       0xC011        // 删除号码局向路由选择
    #define   SAAP_OAM_CF_ADD_TCP_LINK        0xC012        //增加TCP链路
    #define   SAAP_OAM_CF_DEL_TCP_LINK        0xC013        //删除TCP链路

    #define   SAAP_OAM_CF_HLRNO_FLAG          0x00     // hlr no 对应网管UID
    #define   SAAP_OAM_CF_DN_FLAG             0x01     // dn     对应电话号码

    // 路由与本地调用宏
    #define   SAAP_INVIKE_LOCAL       0x01      // 本地调用
    #define   SAAP_INVIKE_ROUTE       0x02      // 路由调用

typedef enum
{
    SAAP_ERR_NOT_FIND_UID_ROUTE = 1,
    SAAP_ERR_MAX
}SAAP_ERR_E;

    
//接口协议处理辅助函数与数据结构
    typedef struct  APP_OR_SAAP_COMMON_Header
    {
        COMMON_HEADER_SAAP          com;            // 消息头
        APP_AND_SAAP_PACKAGE_HEAD   appSaapHead;    //
        XU8                         ucBuffer[1];
    }APP_OR_SAAP_COMMON_HEADER;

    typedef struct  SAAP_AND_TCP_PACKAGE_Head
    {
        XU16        DstDeviceID;        /*目的信令编码*/
        XU16        InviteDeviceID;     /*源的信令编码*/
        XU16        UserType;           /*用户类型*/
        XU16        RouterNum ;         /*路由次数*/
        XU16        MsgLen;             /*用户长度*/
    }SAAP_AND_TCP_PACKAGE_HEAD; //9999 与文档有差别,长度在前面定义

    // SAAP 与 TCP/IP 封装层的接口包
    typedef struct SAAP_AND_TCP_MessagPBuf
    {
        COMMON_HEADER_SAAP           com;
        SAAP_AND_TCP_PACKAGE_HEAD    tcpHead;
        XU8                          ucBuffer[1];
    } SAAP_AND_TCP_MSG_BUF;

    // SAAP 与 TCP/IP 封装层的握手包结构
    typedef struct SAAP_HAND_TCP_MessagPBuf
    {
        COMMON_HEADER_SAAP           com;
        SAAP_AND_TCP_PACKAGE_HEAD    tcpHead  ;
        XU8                          UserType ;     // 用户类型
        XU8                          ucBuffer[1];   // 允许用户带自行数据
    } SAAP_HAND_TCP_MSG_BUF;

    typedef struct SAAP_Dn_Buf
    {
        XU8                          CellNum ;      // 区号个数
        XU16                         Cell ;         // 区号
        XU8                          TellNum ;      // 电话个数
        XU32                         Tell ;         // 号码
    } SAAP_DN_MSG_BUF;

/* shlrHEADER结构*/
typedef struct
{
    t_XOSCOMMHEAD             MsgHead;
    APP_AND_SAAP_PACKAGE_HEAD SaapHead;
    XU32                      ulGtValue;
    XU8                       DN[8];
}SAAP_MSG_TEST_MSG;

//saap 消息统计接口
typedef struct
{
    XU32 ulLinkFailNum;       /* 收到TCPE 发送失败通知的次数*/
    XU32 ulRecvMsgCnt[10];

    XU32  ulSendMsgCnt[10];    /*  SAAP层发出的SMAP消息*/
    XU32  ulRouteMsgCnt;      /*  SAAP层路由的消息 */
}t_SAAPSTAT;

#define MAX_UID_SEG_NUM 128
typedef struct
{
    XU8 ucFlag;
    XU32 ulUidStart;
    XU32 ulUidEnd;
    XU16 dstDpID;
}SAAP_UID_SEG_T;

typedef struct
{
    XU32 ulUidSegRouteFlag; /*0-none, 1-cfg form File, 2-cfg from nms*/
    XU32 ulUidSegNum;
    SAAP_UID_SEG_T stUidSeg[MAX_UID_SEG_NUM];
}SAAP_UID_SEG_CFG;

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

    //------------------------函数声明------------------------------------
    XS8 SAAP_InitProc(XVOID *Para1, XVOID *Para2 );
    XS8 SAAP_MsgProc(XVOID *msg, XVOID *Para);
    XS8 SAAP_TimeoutProc(t_BACKPARA  *pstPara);

    void  SAAP_TimeInit(void);           //定时器初始化
    void  SAAP_TimeStart(void);          //启定时器

    XS32  saap_local_Init(void);         //saap 初始化

    XS32  svc_to_saap_data_proc( COMMON_HEADER_SAAP *pMsg , XU8 flag ) ;     // 处理业务发向SAAP的消息, flag 表示为路由还是本地

    XS32  ip_to_saap_data_proc( COMMON_HEADER_SAAP *pMsg ) ;     // 处理IP 发向SAAP的消息

    // 此函数完成业务层的SAAP结构向标准SAAP包结构的转换操作
    XU32  saap_localID_to_globalID( XU8 *pSaapHead , APP_AND_SAAP_PACKAGE_HEAD *pAappSaapHead  );

    XU32  saap_globalID_to_localID( APP_AND_SAAP_PACKAGE_HEAD *pAappSaapHead  , XU8 *pSaapHead  );

    // 此函数完成业务层的SAAP结构向标准SAAP包结构的转换操作
    void  saap_GT_to_BCD( XU8 SaapHeadBCD[] , XU32 GtID  );
    XU32  saap_BCD_to_GT(XU8 flag , XU8 SaapHeadBCD[]  );

    // 此函数根据GT查找信令点编码
    XU16  saap_DstID_from_Gt(  struct Gst gstVale ,XU32 ProtocolTag ) ;

    // 此函数用于比对GT号段是否为本机号段
    XU16 saap_check_no_proc(XU32 proTag, XU8 flag , XU16 hlrno, XU32 dnno) ;
    XS32  saap_regCmd();
    XVOID saap_showsaapinfo(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_setlocalep(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_addeidroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_adduidroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_addtellnoroute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_delEidRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_delUidRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_delTellNoRoute(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);

    XVOID SaapTest(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_setlocaluidprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_setlocaltellnoprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_deleid(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_deluidprifix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_deltellnoprefix(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_statclear(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
    XVOID saap_showstat(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
    XVOID saap_setstatswitch(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
    XVOID saap_StartTestTimer(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_StopTestTimer(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_SetTestData(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XS32 SaapPeriodTest(XU32 numPerSecond);
    XVOID saap_showlocalinfo(CLI_ENV* pCliEnv);
    XVOID saap_showeidroute(CLI_ENV* pCliEnv,XU32 ulFlag);
    XVOID saap_showhlrroute(CLI_ENV* pCliEnv,XU32 ulFlag);
    XVOID saap_showsmcroute(CLI_ENV* pCliEnv,XU32 ulFlag);
    XS32 SaapSendTestMsg(XU32 ProTag, XU32 routeType,XU32 gtValue,XU8 tellNo[8]);
    void SaapTblRegToOAM(XU32 uiModId);
    XVOID saap_CliSetFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_CliClsFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_CliShowFilter(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    //
    XU16  saap_getDstID_from_UidSeg(XU32 ulUid);
    XS32 saap_SetUidSegRouteFlag(XU32 ulFlag);
    XS32 saap_addUidSegRoute(XU32 ulUidStart,XU32 ulUidEnd,XU16 dstDp);
    XS32 saap_delUidSegRoute(XS32 routeIdx);
    XVOID saap_showUidSeg(CLI_ENV* pCliEnv, XS32 siArgc,  XCHAR **ppArgv);
    XVOID saap_showrbtroute(CLI_ENV* pCliEnv,XU32 ulFlag);
#ifdef  __cplusplus
}
#endif

#endif


