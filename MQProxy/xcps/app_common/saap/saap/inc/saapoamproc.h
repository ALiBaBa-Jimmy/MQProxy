/*----------------------------------------------------------------------
    saapProc.h - 局部宏、类型、函数

    版权所有 2004 -2006 信威公司深研所SAG项目组.

    author: 张海

    修改历史记录
    --------------------
    添加版权说明.
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

//------------------------宏定义------------------------------------

#define ERR_CB_NUM                  0xFFFF

//------------------------结构和共用体定义----------------------------
#define   SAAP_TABLE_ENGR                1        // 已使用
#define   SAAP_TABLE_NULL                0        // 未使用

#define SAAP_OP_TEL_TABLE_TAG            1        //
#define SAAP_OP_EID_TABLE_TAG            8        //
#define SAAP_OP_HLR_TABLE_TAG            9        //
#define SAAP_ROUTE_TBL_DESC_LEN          32

#define SAAP_TELLNO_PREFIX_MAX          99999999

//MAPUA表格ID范围51~60
//不用agent编译时使用此选定义
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

/*本地设备号*/
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
    XS32 destDpId;   //目的信令点
    XU8 ucArrDesc[SAAP_ROUTE_TBL_DESC_LEN+1];
}T_SAAP_EID_ROUTE_TBL;

/*route table */
typedef struct _tagSaapPrefixRouteTbl
{
    XS32 routePreFix;
    XS32 destEpType; //目的设备类型
    XS32 destDpId;   //目的信令点
    XU8 ucArrDesc[SAAP_ROUTE_TBL_DESC_LEN+1];
}T_SAAP_PREFIX_ROUTE_TBL;

/*UID route table */
typedef struct _tagSaapUidRouteTbl
{
    XS32 uidPreFix;
    XS32 destEpType; //目的设备类型
    XS32 destDpId;   //目的信令点
    XU8 ucArrDesc[SAAP_ROUTE_TBL_DESC_LEN+1];
}T_SAAP_UID_ROUTE_TBL;

/*UID route table */
typedef struct _tagSaapTellnoRouteTbl
{
    XS32 uidTellNoFix;
    XS32 destEpType; //目的设备类型
    XS32 destDpId;   //目的信令点
    XU8 ucArrDesc[SAAP_ROUTE_TBL_DESC_LEN+1];
}T_SAAP_TELLNO_ROUTE_TBL;

typedef struct
{
    COMMON_HEADER_SAAP  comHead;
    SAAP_AND_TCP_PACKAGE_HEAD tcpeHead;
    XU8     ProtocolTag ;     /*协议用户表示语*/
    XU8     ucDstLableTag;    //目的标签
    XU8     ucDstLen;
    XU8     ucTranPlan;       //翻译计划
    XU8     ucDstCodePlan ;   //编号计划 1- dn ,8 -eid ,9=uid + 编码设计(0x02)

    union DstGst
    {
        XS32    GtVal;            /*目的用户全局编码，编号计划为 1. 8 时使用*/
        XU8     gtDN[8];      /*目的用户全局编码，编号计划为 9 时使用*/
    }uDstGst;
}SAAP_TO_TCPE_MSG_HEAD_T;

/*for xscale ld func relocate alignment. 2007.4.27*/
#ifdef SCALE_CPU_VX
#pragma pack(0)
#else
#pragma pack()
#endif

// saap模块本地信令点信息操作函数(注，SAG不包括HLRno与tellNO)
XU32 saap_setLocalEID( XU32 eid ,XU32 nodeType);

// saap模块号段与设备ID操作接口函数
XU32 saap_setGTTable(XU8 tag, XU32 gt, XU16 DstID, XU32 routeTabid);
XU32 saap_delGTTable(XU8 tag, XU32 gt, XU32 routeTabid);
XU32 saap_qryGTTable(XU8 tag, XU32 gt, XU32 ulRouteTabid, XU16 *pDstID);

//测试代码
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


