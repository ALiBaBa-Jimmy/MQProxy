/*----------------------------------------------------------------------
    saapProc.h - �ֲ��ꡢ���͡�����

    ��Ȩ���� 2004 -2006 ������˾������SAG��Ŀ��.

    author: �ź�

    �޸���ʷ��¼
    --------------------
    ��Ӱ�Ȩ˵��.
----------------------------------------------------------------------*/

#ifndef _SAAPOAMPROC_H_
#define _SAAPOAMPROC_H_

//#include "oam_public_data_def.h"
#include "oam_tab_def.h"

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

//------------------------�궨��------------------------------------

#define ERR_CB_NUM                  0xFFFF

//------------------------�ṹ�͹����嶨��----------------------------
#define   SAAP_TABLE_ENGR                1        // ��ʹ��
#define   SAAP_TABLE_NULL                0        // δʹ��

#define SAAP_OP_TEL_TABLE_TAG            1        //
#define SAAP_OP_EID_TABLE_TAG            8        //
#define SAAP_OP_HLR_TABLE_TAG            9        //
#define SAAP_ROUTE_TBL_DESC_LEN          32

#define SAAP_TELLNO_PREFIX_MAX          99999999

//MAPUA���ID��Χ51~60
//����agent����ʱʹ�ô�ѡ����
#if 0
    typedef enum
    {
        SAAP_LOCAL_EID_TABLE= 1,
        SAAP_LOCAL_UID_PREFIX_TABLE =2,
        SAAP_LOCAL_TELL_PREFIX_TABLE =3,
        SAAP_EID_ROUTE_TABLE = 4,
        SAAP_UID_ROUTE_TABLE = 5,
        SAAP_TELLNO_REOUTE_TABLE = 6,
        SAAP_CFG_TABLE_BUTT
    }SAAP_TABLE;
#else 
    typedef enum
    {
        SAAP_LOCAL_EID_TABLE= xwSaapLocalCfg,
        SAAP_LOCAL_UID_PREFIX_TABLE =xwSaapLocalUIDTable,
        SAAP_LOCAL_TELL_PREFIX_TABLE =xwSaapLocalTelTable,
        SAAP_EID_ROUTE_TABLE = xwSaapEIDRouteTable,
        SAAP_UID_ROUTE_TABLE = xwSaapUIDRouteTable,
        SAAP_TELLNO_REOUTE_TABLE = xwSaapTelRouteTable,
        SAAP_CFG_TABLE_BUTT
    }SAAP_TABLE;
#endif

/*�����豸��*/
typedef struct _tagSaapLocalIEid
{
    XS32 localEid;
    XS32 nodeType;
}T_SAAP_LOCAL_EID_TBL;

typedef struct _tagSaapLocalPrefix
{
    XS32 index;
    XS32 localPrefix;
}T_SAAP_LOCAL_PREFIX_TBL;

typedef struct _tagSaapLocalUid
{
    XS32 index;
    XS32 localUidPrefix;
}T_SAAP_LOCAL_UID_TBL;

typedef struct _tagSaapLocalTell
{
    XS32 index;
    XS32 localTellPrefix;
}T_SAAP_LOCAL_TELL_TBL;

/*route table */
typedef struct _tagSaapEidRouteTbl
{
    XS32 Eid;
    XS32 destDpId;   //Ŀ�������
    XU8 ucArrDesc[SAAP_ROUTE_TBL_DESC_LEN+1];
}T_SAAP_EID_ROUTE_TBL;

/*route table */
typedef struct _tagSaapPrefixRouteTbl
{
    XS32 routePreFix;
    XS32 destEpType; //Ŀ���豸����
    XS32 destDpId;   //Ŀ�������
    XU8 ucArrDesc[SAAP_ROUTE_TBL_DESC_LEN+1];
}T_SAAP_PREFIX_ROUTE_TBL;

/*UID route table */
typedef struct _tagSaapUidRouteTbl
{
    XS32 uidPreFix;
    XS32 destEpType; //Ŀ���豸����
    XS32 destDpId;   //Ŀ�������
    XU8 ucArrDesc[SAAP_ROUTE_TBL_DESC_LEN+1];
}T_SAAP_UID_ROUTE_TBL;

/*UID route table */
typedef struct _tagSaapTellnoRouteTbl
{
    XS32 uidTellNoFix;
    XS32 destEpType; //Ŀ���豸����
    XS32 destDpId;   //Ŀ�������
    XU8 ucArrDesc[SAAP_ROUTE_TBL_DESC_LEN+1];
}T_SAAP_TELLNO_ROUTE_TBL;

typedef struct
{
    COMMON_HEADER_SAAP  comHead;
    SAAP_AND_TCP_PACKAGE_HEAD tcpeHead;
    XU8     ProtocolTag ;     /*Э���û���ʾ��*/
    XU8     ucDstLableTag;    //Ŀ�ı�ǩ
    XU8     ucDstLen;
    XU8     ucTranPlan;       //����ƻ�
    XU8     ucDstCodePlan ;   //��żƻ� 1- dn ,8 -eid ,9=uid + �������(0x02)

    union DstGst
    {
        XS32    GtVal;            /*Ŀ���û�ȫ�ֱ��룬��żƻ�Ϊ 1. 8 ʱʹ��*/
        XU8     gtDN[8];      /*Ŀ���û�ȫ�ֱ��룬��żƻ�Ϊ 9 ʱʹ��*/
    }uDstGst;
}SAAP_TO_TCPE_MSG_HEAD_T;

/*for xscale ld func relocate alignment. 2007.4.27*/
#ifdef SCALE_CPU_VX
#pragma pack(0)
#else
#pragma pack()
#endif

// saapģ�鱾���������Ϣ��������(ע��SAG������HLRno��tellNO)
XU32 saap_setLocalEID( XU32 eid ,XU32 nodeType);

// saapģ��Ŷ����豸ID�����ӿں���
XU32 saap_setGTTable(XU8 tag, XU32 gt, XU16 DstID, XU32 routeTabid);
XU32 saap_delGTTable(XU8 tag, XU32 gt, XU32 routeTabid);
XU32 saap_qryGTTable(XU8 tag, XU32 gt, XU32 ulRouteTabid, XU16 *pDstID);

//���Դ���
XU32 saap_testPro();

//Monitor  
//extern XU8 SaapOamCallBack(XU32  uiTableId, XU16 usMsgId, XU32 uiSequence,XU8 ucPackEnd, tb_record *ptRow);
extern XS32 SaapLocalEidTblOpr(XU16 usMsgId, T_SAAP_LOCAL_EID_TBL * pSaabLocalInfoTbl, char* mask);
extern XS32 SaapUidTellNoRouteTblOpr(XU32 tabId,XU16 usMsgId, T_SAAP_PREFIX_ROUTE_TBL* pSaapTbl, char* mask);
extern XS32 SaapLocalUidTellTblOpr(XU32 tabId,XU16 usMsgId, T_SAAP_LOCAL_PREFIX_TBL* pSaapTbl, char* mask);
XVOID SaapGetDstDpByGt(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XU32 saap_SetSaapSstNoID(  XU32 flag , XU32 index , XU32 dataFalg , XU32 hlrNo , XU32 dn );
XU32 SAAP_SetMonFilter(SAAP_MON_FILTER_T *pSaapMonFilter);

#ifdef  __cplusplus
}
#endif

#endif


