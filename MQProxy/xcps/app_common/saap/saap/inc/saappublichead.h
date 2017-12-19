/*----------------------------------------------------------------------
    saapPublicHead.h - ��ҵ����Ľӿڲ㶨�壬��˫��ʹ��

    ��Ȩ���� 2004 -2006 ������˾������SAG��Ŀ��.

    author: �ź�

    �޸���ʷ��¼
    --------------------
    ��Ӱ�Ȩ˵��.
----------------------------------------------------------------------*/

#ifndef _SAAPPUBLICHEAD_H_
#define _SAAPPUBLICHEAD_H_
#include "../../hashlst/inc/saap_def.h"

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

    //------------------------�궨��------------------------------------

    /* Ŀ���û���żƻ���1�ķ�����Ϊ��DN->SHLR������� */
    #define SAAP_GST_DN_TO_SHLR         0x01
    #define SAAP_GST_DN                 0x01
    /* Ŀ���û���żƻ���8�ķ�����Ϊ��SHLRID->SHLR������� SAGID->SAG������� SMCID->SMC������� */
    #define SAAP_GST_EID_TO_SHLR        0x08
    #define SAAP_GST_EID                0x08
    /* Ŀ���û���żƻ���9�ķ�����Ϊ��UID->SHLR�ڵ� */
    #define SAAP_GST_UID_TO_SHLR        0x09
    #define SAAP_GST_UID                0x09

    /* Դ�û���żƻ���1�ķ�����Ϊ��DN->SHLR������� */
    #define SAAP_INT_DN_TO_SHLR         0x21
    #define SAAP_INT_DN                 0x21
    /* Դ�û���żƻ���8�ķ�����Ϊ��SHLRID->SHLR������� SAGID->SAG������� SMCID->SMC������� */
    #define SAAP_INT_EID_TO_SHLR        0x28
    #define SAAP_INT_EID                0x28
    /* Դ�û���żƻ���9�ķ�����Ϊ��UID->SHLR�ڵ� */
    #define SAAP_INT_UID_TO_SHLR        0x29
    #define SAAP_INT_UID                0x29

    #define SAAP_LINT_ERROR          0x01        // ������·����ֵ

    // Э���ʾ��
    #define SAAP_PRO_TAG_MIN     0X7f
    #define SAAP_SMAP_MSE        0x80         // SXC<->HLR ,SMC->HLR ,HLR<->HLR SMAP��Ϣ
    #define SAAP_SHLR2SMC_MSE    0X90         // HLR->SMC msg
    #define SAAP_SHAP_MSE        0xA0         // SHAP��Ϣ   SXC<->SXC
    #define SAAP_SXC2SMC_MSE     0xB0         // SXC <-> Smc Msg
    #define SAAP_SMC2SMC_MSE     0xC0         // smc <-> smc msg
    #define SAAP_X2RTB_MSE     0xD0           // XXX <->RBT
    #define SAAP_PRO_TAG_MAX     0xD1

    #define SAAP_SPA_MSG_TYPE    0x06       //

//------------------------�ṹ�͹����嶨��----------------------------
    typedef XS32 (*SAAP_REGSRVFID_FUNC) (XS32);

    typedef enum
    {
        SAAP_DATA_REQ,
        SAAP_DATA_IND,
        SAAP_DATA_CFG,
        SAAP_NO_ROUTE,
        SAAP_MSG_ID_MAX
    }SAAP_MSG_TYPE;

    // ҵ�����SAAP���Ľӿ�Э�鶨��, �绰�Ŷ���GTʱ�������淶��SMAPЭ���е�Լ��һ��
    //�޸����ο���Э�鶨��
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
        XU8    ProtocolTag ;     /*Э���û���ʾ��*/

        struct Gst
        {
            XU8   GuestSerialNumTag;        /*Ŀ���û���żƻ���ʶ����ָָʾ���ֱ�ŷ�ʽ*/
            union GstNID
            {
                XU32    GuestNetID;         /*Ŀ���û�ȫ�ֱ��룬��żƻ�Ϊ 1. 8 ʱʹ��*/
                XU8     GuestNetIDTell[8];  /*Ŀ���û�ȫ�ֱ��룬��żƻ�Ϊ 9 ʱʹ��*/
            }uGstNID;
        }uGst;

        struct Int
        {
            XU8   InviteSerialNumTag;         /*Դ�û���żƻ���ʶ����ָָʾ���ֱ�ŷ�ʽ*/
            union IntNID
            {
                XU32    InviteNetID;          /*Ŀ���û�ȫ�ֱ��룬��żƻ�Ϊ 1.8 ʱʹ��*/
                XU8     InviteNetIDTell[8];   /*Ŀ���û�ȫ�ֱ��룬��żƻ�Ϊ 9 ʱʹ��*/
            }uIntNID;
        }uInt;

        XU16     UserMsgLen;          /*ָʾ�������ݵĳ���*/

    }APP_AND_SAAP_PACKAGE_HEAD;

    // SAAP ��Ϣ����
    typedef struct {
        //XU32  ulTraceFlag; // 0������,1����
        XU32  ullinkHandle;
        XU32  ulTraceId;
        XU32  ulSpaCliIpAddr;
        XU32  ulSpaCliPort;

        XU32  ulOptType; //�������� 0-�����������,1-���ù�������
        //XU32  proTag;
        XU32  codePlan;  //��żƻ�  //1- DN,8- EID,9-UID
        XU32  ulGt;
    }SAAP_MON_FILTER_T;

    // TCPE������������:
    typedef struct{
        XU32  ulSpaFlag; // 0������,1����
        XU32  ulSpalInkHandle;
        XU32  ulSpaCliIpAddr;
        XU32  ulSpaCliPort;

        XU32  optType; //�������� 0-�����������,1-���ù�������
        XU32  ulDstDpId;
    }TCPE_MON_FILTER_T;

    typedef struct
    {
        XU32 ulFlag;
        XU32 ulIpAddr;
        XU32 ulPort;
    }SAAP_SPA_CLI_IP_T;

//tcpe,saap��Spy analysis��Ϣ�������ݽṹ
    typedef struct
    {
        XU16 msgType;         //��Ϣ����,0x15e0,���ڸ���
        XU16 msgLen;          //��Ϣ�峤��
        XU32 TraceId;         //����0
        XU32 SendSerial;      //�������к�,����0
        XU32  TimeLabel;      //ʱ�� 0xFFFFFFFF
        XU32  SrvMsgType;     //ҵ����Ϣ���� 0x01������Ϣ 06 saap��Ϣ
        //XU8   MsgByteOrder; // �ֽ����� 0x01-NetByteOrder
        XU8  SubMsgType;      // ����Ϣ���� 1- saap2tcpeMsg,2-saap2srvMsg
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


