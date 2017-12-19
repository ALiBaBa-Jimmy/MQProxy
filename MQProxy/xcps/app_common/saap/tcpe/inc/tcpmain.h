/*----------------------------------------------------------------------
    tcpMain.h - 全局宏与类型定义

    版权所有 2004 -2006 信威公司深研所SAG项目组.

    author: 张海

    修改历史记录
    --------------------
    添加版权说明.
----------------------------------------------------------------------*/

#ifndef _TCPMAIN_H_
#define _TCPMAIN_H_

#include "xosshell.h"
#include "saap_def.h"

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

    //------------------------宏定义------------------------------------

#define SAAPANDTCP_HEAD_LEN  10   //报文头长度 参见数据结构SAAP_AND_TCP_PACKAGE_HEAD的定义

#define MAX_TCPE_HAND_TIMES  10
#define TCP_REG_TIMER_NUM    MAX_TCPEN_LINK_NUM    //注册的最大定时器数目
#define TCP_TIMER_NUM        3                     //定义的最大定时器数目


//协议规定,不能修改
#define TCP_CURR_FLAG              0x7E        //报文边界
#define TCP_HEAD_FLAG              0xA5        //报文数据区开始
#define TCP_END_FLAG               0x0D        //报文数据区结束

#define TCP_ERROR_NUM              0xFFFF

#define TCP_ROUTER_MAX_NUM         32    // 允许的最大路由次数
#define    SAAP_USER_TYPE          0x1100       // TCP/IP 中的SAAP用户类型
#define    TCP_USERTYPE_HANDSHAKE  0x1110       // TCP 握手用户类型
#define    MAX_TCPE_USER_NUM 10
#define    TCP_USERTYPE_CBUA       0x1112       // TCPE 直连数据类型
#define    TCP_USERTYPE_CBUA_2     0x1113 //CBUA2
#define    TCP_USERTYPE_VMR        0x1115 // VMR
//协议规定,不能修改

/* tcp link status */
#define   TCP_LINK_OK            0          // 正常
#define   TCP_LINK_FAIL          1          // 不通

#define   TCP_hand_reg           0x00       // 握手请求
#define   TCP_hand_ack           0x01       // 握手应答

#define   TCP_HAND_ERR           0xEEEE      // 包发不出去通知

// 路由与本地调用宏
#define   TCP_INVIKE_LOCAL       0x01       // 本地调用
#define   TCP_INVIKE_ROUTE       0x02       // 路由调用

typedef struct
{
    XU32 userType;
    XU32 userFid;
}TCPE_USER_FID_T;

// TCP/IP 封装与上层的接口数据结构
typedef struct  TCP_AND_PRO_LAYER_Interface
{
    XU16        DstDeviceID;        /*目的信令编码*/
    XU16        GuestDeviceID;      /*源的信令编码*/
    XU16        UserType;           /*用户类型*/
    XU16        RouterNum ;         /*路由次数*/
    XU16        MsgLen;             /*用户长度*/
}TCP_AND_PRO_LAYER_INTERFACE;

// TCP/IP 封装层与上层的接口结构
typedef struct TCP_AND_PRO_LAYER_MessagPBuf
{
    COMMON_HEADER_SAAP                   com;
    TCP_AND_PRO_LAYER_INTERFACE      tcpHead;
    XU8                          ucBuffer[1];
} TCP_AND_PRO_LAYER_MSG_BUF;

// TCP/IP 封装与下一层的接口数据结构
typedef struct  TCP_AND_NEXT_LAYER_Interface
{
    XU8     ucHeadFlag7E;
    XU8     ucHeadFlagA5;

    XU16        MsgLen;             /*用户长度*/
    XU16        DstDeviceID;        /*目的信令编码*/
    XU16        GuestDeviceID;      /*源的信令编码*/
    XU16        UserType;           /*用户类型*/
    XU16        RouterNum ;         /*用于记录经过了多少个路由点*/
}TCP_AND_NEX_LAYER_INTERFACE;

// TCP/IP 封装层与下层的数据包接口结构,处理数据类消息
typedef struct TCP_AND_NEX_LAYER_MessagPBuf
{
    COMMON_HEADER_SAAP                   com;
    TCP_AND_NEX_LAYER_INTERFACE      tcpHead;
    XU8                          ucBuffer[1];
} TCP_AND_NEX_LAYER_MSG_BUF;

/* TCP/IP 封装层握手消息 */
typedef struct TCP_hand_msg
{
    COMMON_HEADER_SAAP                   comHead;
    TCP_AND_NEX_LAYER_INTERFACE      tcpHead;
    XU8                          msgType;       //消息类型
    XU8                          Tailbuff[2];
} TCP_hand_msg;

// TCP/IP 封装层与下层的命令接口结构,处理管理类消息
typedef struct TCP_AND_NEX_LAYER_COM_Buf
{
    COMMON_HEADER_SAAP                   com;
    XU8                          ucBuffer[1];
} TCP_AND_NEX_LAYER_COM_BUF;

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

XS8   TCP_InitProc(XVOID *Para1, XVOID *Para2 );
XS8   TCP_MsgProc(XVOID *msg, XVOID *Para);
XS8   TCP_TimeoutProc (t_BACKPARA  *pstPara);

//平台上行包处理
XS32  Tcp_ntlmsg_proc( COMMON_HEADER_SAAP *pstMsg );

//业务下行平台的包处理,处理由业务通过tcpe发送的下行数据包
//flag 用于区分是路由类消息与本地类调用
void TCP_Svc2Tcp_DataReqProc( COMMON_HEADER_SAAP *pstMsg , XU8 flag );

//组包与重定位处理
//通过平台NTL上行到业务的包处理,处理NTL->TCPE->APP上行数据包
void Tcp_ntlmsg_packetdecodeproc( COMMON_HEADER_SAAP *pstMsg );

//处理数据指示
void TCP_Tcp2Svc_DataIndProc( COMMON_HEADER_SAAP *pstMsg, XU8 *MsgBuffer );

// TCP 握手函数,
void Tcp_lint_scan(XU16 ccbNo) ;
void TCP_hand_msg_proc( COMMON_HEADER_SAAP *pstMsg );

XS32 tcp_check_lintIndex(XU16 index); // 此函数用来检查配置链路号的有效性

/*20090722 add below*/
XS32 tcpe_setfid_byusertype(XS32 userType,XS32 userFid);
XS32 tcpe_getfid_byusertype(XS32 UserType);
/*20090722 add above*/
XU16 tcpe_getDefaultLink();

#ifdef  __cplusplus
}
#endif

#endif


