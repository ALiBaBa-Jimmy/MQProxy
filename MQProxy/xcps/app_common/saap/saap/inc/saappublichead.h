/*----------------------------------------------------------------------
    saapPublicHead.h - 与业务层间的接口层定义，供双方使用

    版权所有 2004 -2006 信威公司深研所SAG项目组.

    author: 张海

    修改历史记录
    --------------------
    添加版权说明.
----------------------------------------------------------------------*/

#ifndef _SAAPPUBLICHEAD_H_
#define _SAAPPUBLICHEAD_H_
#include "../../hashlst/inc/saap_def.h"

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

    //------------------------宏定义------------------------------------

    /* 目的用户编号计划＝1的翻译项为：DN->SHLR信令编码 */
    #define SAAP_GST_DN_TO_SHLR         0x01
    #define SAAP_GST_DN                 0x01
    /* 目的用户编号计划＝8的翻译项为：SHLRID->SHLR信令编码 SAGID->SAG信令编码 SMCID->SMC信令编码 */
    #define SAAP_GST_EID_TO_SHLR        0x08
    #define SAAP_GST_EID                0x08
    /* 目的用户编号计划＝9的翻译项为：UID->SHLR节点 */
    #define SAAP_GST_UID_TO_SHLR        0x09
    #define SAAP_GST_UID                0x09

    /* 源用户编号计划＝1的翻译项为：DN->SHLR信令编码 */
    #define SAAP_INT_DN_TO_SHLR         0x21
    #define SAAP_INT_DN                 0x21
    /* 源用户编号计划＝8的翻译项为：SHLRID->SHLR信令编码 SAGID->SAG信令编码 SMCID->SMC信令编码 */
    #define SAAP_INT_EID_TO_SHLR        0x28
    #define SAAP_INT_EID                0x28
    /* 源用户编号计划＝9的翻译项为：UID->SHLR节点 */
    #define SAAP_INT_UID_TO_SHLR        0x29
    #define SAAP_INT_UID                0x29

    #define SAAP_LINT_ERROR          0x01        // 信令链路错误值

    // 协议表示语
    #define SAAP_PRO_TAG_MIN     0X7f
    #define SAAP_SMAP_MSE        0x80         // SXC<->HLR ,SMC->HLR ,HLR<->HLR SMAP消息
    #define SAAP_SHLR2SMC_MSE    0X90         // HLR->SMC msg
    #define SAAP_SHAP_MSE        0xA0         // SHAP消息   SXC<->SXC
    #define SAAP_SXC2SMC_MSE     0xB0         // SXC <-> Smc Msg
    #define SAAP_SMC2SMC_MSE     0xC0         // smc <-> smc msg
    #define SAAP_X2RTB_MSE     0xD0           // XXX <->RBT
    #define SAAP_PRO_TAG_MAX     0xD1

    #define SAAP_SPA_MSG_TYPE    0x06       //

//------------------------结构和共用体定义----------------------------
    typedef XS32 (*SAAP_REGSRVFID_FUNC) (XS32);

    typedef enum
    {
        SAAP_DATA_REQ,
        SAAP_DATA_IND,
        SAAP_DATA_CFG,
        SAAP_NO_ROUTE,
        SAAP_MSG_ID_MAX
    }SAAP_MSG_TYPE;

    // 业务层与SAAP层间的接口协议定义, 电话号段做GT时，其编码规范与SMAP协议中的约定一样
    //修改所参考的协议定义
    typedef struct  
    {
        XU32   prtDog;
        /*
        8:prtTag
        8:srcTag
        8:dstTag
        8:reserved
        */
        XU8  prtsrcData[8];
        /*
        4byte:according to srcTag type equal 1 or 8
        8byte:according to srcTag type equal 9
        */
        XU8  prtdstData[8];
        /*
        4byte:according to srcTag type equal 1 or 8
        8byte:according to srcTag type equal 9
        */
        XU32  msgLen;
        /*
        16:tail data buffer len
        16:reserved
        */
    }t_newSaapUserPrt;
    
    typedef struct APP_AND_SAAP_PACKAGE_Head
    {
        XU8    ProtocolTag ;     /*协议用户表示语*/

        struct Gst
        {
            XU8   GuestSerialNumTag;        /*目的用户编号计划标识，其指指示哪种编号方式*/
            union GstNID
            {
                XU32    GuestNetID;         /*目的用户全局编码，编号计划为 1. 8 时使用*/
                XU8     GuestNetIDTell[8];  /*目的用户全局编码，编号计划为 9 时使用*/
            }uGstNID;
        }uGst;

        struct Int
        {
            XU8   InviteSerialNumTag;         /*源用户编号计划标识，其指指示哪种编号方式*/
            union IntNID
            {
                XU32    InviteNetID;          /*目的用户全局编码，编号计划为 1.8 时使用*/
                XU8     InviteNetIDTell[8];   /*目的用户全局编码，编号计划为 9 时使用*/
            }uIntNID;
        }uInt;

        XU16     UserMsgLen;          /*指示后续数据的长度*/

    }APP_AND_SAAP_PACKAGE_HEAD;

    // SAAP 消息过滤
    typedef struct {
        //XU32  ulTraceFlag; // 0不可用,1可用
        XU32  ullinkHandle;
        XU32  ulTraceId;
        XU32  ulSpaCliIpAddr;
        XU32  ulSpaCliPort;

        XU32  ulOptType; //操作类型 0-清除过滤条件,1-设置过滤条件
        //XU32  proTag;
        XU32  codePlan;  //编号计划  //1- DN,8- EID,9-UID
        XU32  ulGt;
    }SAAP_MON_FILTER_T;

    // TCPE过滤条件设置:
    typedef struct{
        XU32  ulSpaFlag; // 0不可用,1可用
        XU32  ulSpalInkHandle;
        XU32  ulSpaCliIpAddr;
        XU32  ulSpaCliPort;

        XU32  optType; //操作类型 0-清除过滤条件,1-设置过滤条件
        XU32  ulDstDpId;
    }TCPE_MON_FILTER_T;

    typedef struct
    {
        XU32 ulFlag;
        XU32 ulIpAddr;
        XU32 ulPort;
    }SAAP_SPA_CLI_IP_T;

//tcpe,saap的Spy analysis消息跟踪数据结构
    typedef struct
    {
        XU16 msgType;         //消息类型,0x15e0,正在跟踪
        XU16 msgLen;          //消息体长度
        XU32 TraceId;         //可填0
        XU32 SendSerial;      //发送序列号,可填0
        XU32  TimeLabel;      //时间 0xFFFFFFFF
        XU32  SrvMsgType;     //业务消息类型 0x01接收消息 06 saap消息
        //XU8   MsgByteOrder; // 字节序列 0x01-NetByteOrder
        XU8  SubMsgType;      // 子消息类型 1- saap2tcpeMsg,2-saap2srvMsg
    }SAAP_SPA_MSG_HEAD_T;

XU32 Saap_SpaCliFindCcbByIp(XU32 ip,XU32 port);
XU32 Saap_SpaClisaveData(XU32 idx,SAAP_MON_FILTER_T * psaapFilter);
XU32 Saap_SpaCliFindIdleCcb();
XU32 Saap_SpaCliInsert(XU32 idx,SAAP_MON_FILTER_T * psaapFilter);
XU32 Saap_SpaCliDelCcb(XU32 idx);
XU32 Saap_SpaCliClsSendFlag();
XU32 Saap_RegGetSrvFidByPro(SAAP_REGSRVFID_FUNC pFunc);

extern XS32 SAAP_getFidByProTag(XS32 proTab);
extern XU16 TCPE_GetLocalDpId();
/**
*get TellNo Route data
*@para nodeType 0-Hlr,1-SMC,2-RBTC
*@para *pNum ,outputPara
*@para aucRouteTell,output para, array upbound is 255
*chenxiaofeng 20120116
*
*/
XU32 SaapGetLocalTell();
XS32 SaapGetLocalRouteTell(XU32 nodeType,XU32 *pNum,XU32 aulRouteTell[]);
XU32 SaapNum2Bcd(XU32 ulNum,char *pBcd,int *pLen);
XU32 SaapRBcd2Bcd(XU32 RBcd);
XU32 SaapXwBcd2Bcd(XU32 RcvBcd);
XU32 SaapBcd2Num(XU32 RcvBcd);

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

#ifdef  __cplusplus
}
#endif

#endif


