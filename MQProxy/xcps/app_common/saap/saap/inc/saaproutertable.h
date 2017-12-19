/*----------------------------------------------------------------------
saapRouterTable.h - 全局宏与类型定义

版权所有 2004 -2006 信威公司深研所BSC项目组.

author: 张海

修改历史记录
--------------------
添加版权说明.
----------------------------------------------------------------------*/

#ifndef _SAAPROUTERTABLE_H_
#define _SAAPROUTERTABLE_H_

#ifdef  __cplusplus
    extern  "C"
    {
#endif

#pragma pack(1)

//------------------------宏定义------------------------------------
#define    SAAP_LOCAL_PREFIX_NUM    255
#define    SAAP_MAX_SAAPCCB_NUM     255
#define    SAAP_ERROR_NUM           0xFFFF

/* HASH表常量定义 */
#define SAAP_HASH_TABLE_SIZE    SAAP_MAX_SAAPCCB_NUM    /*HASH  表尺寸*/
#define SAAP_HASH_KEY_MASK      SAAP_MAX_SAAPCCB_NUM    /*HASH  表关键字掩码*/

//------------------------结构和共用体定义----------------------------
typedef enum
{
    SAAP_NODE_TYPE_HLR,
    SAAP_NODE_TYPE_SMC,
    SAAP_NODE_TYPE_SXC,
    //SAAP_NODE_TYPE_STP,
    SAAP_NODE_TYPE_RBT, /*20111215 cxf add begin: add rbt route func*/
    SAAP_NODE_TYPE_MAX
}SAAP_NODE_TYPE_ENUM;

typedef enum
{
    SAAP_ROUTE_TBL_HLR,
    SAAP_ROUTE_TBL_SMC,
    SAAP_ROUTE_TBL_SXC,
    SAAP_ROUTE_TBL_RBT, /*20111215 cxf add begin: add rbt route func*/
    SAAP_ROUTE_TBL_MAX
}SAAP_ROUTE_TBL_ENUM;

// SAAP 本机GT数据结构封装
typedef struct SAAP_GLOBAL_TELLNO_Struct
{
    XU8      flag ;      // 开启标致, 1 表示数据有效
    XU32     tellNo;     // 本机设备号
}SAAP_GLOBAL_TELLNO_STRUCT;

typedef struct SAAP_GLOBAL_HLRNO_Struct
{
    XU8      flag ;   // 开启标致, 1 表示数据有效
    XU16     hlrNo;   // 本机设备号
}SAAP_GLOBAL_HLRNO_STRUCT;

typedef struct SAAP_GLOBAL_EID_tag
{
    XU8      flag ;        // 开启标致, 1 表示数据有效
    XU32     eidNo;        // 本机设备号
    XU32     nodeType ;    //设备类型
}SAAP_GLOBAL_EID_STRUCT;

typedef struct
{
    XU8      flag;       // 开启标致, 1 表示数据有效
    XU32     nodeType;   // 本机设备号
}SAAP_GLOBAL_NODE_TYPE_STRUCT;

typedef struct tag_SAAP_GLOBAL
{
    SAAP_GLOBAL_EID_STRUCT          eidNoCfg;                       // 本机设备号(只有一个)
    SAAP_GLOBAL_HLRNO_STRUCT        hlrNoCfg[SAAP_LOCAL_PREFIX_NUM];   // 本机hlr号段值(hlr专用)
    SAAP_GLOBAL_TELLNO_STRUCT       tellNoCfg[SAAP_LOCAL_PREFIX_NUM];  // 本机tell号段值(hlr专用)
}SAAP_GLOBAL_STRUCT;

// tell号段 索引目的信令点编码的数据结构
typedef struct _tagSAAPTelDstDeviceIDCcb
{
    XU32      tellNo;           // 号段: 区号 + 局号(最多4位)
    XU16      DstID ;           // 信令点编码索引值
}SAAP_TELL_INDEX_DSTDEVICEID_CCB;

// hlr号段索引目的信令点编码的数据结构
typedef struct _tagSAAPHlrDstDeviceIDCcb
{
    XU16      hlrNo;            // HLR号段值, 16bit
    XU16      DstID ;           // 信令点编码索引值
}SAAP_HLR_INDEX_DSTDEVICEID_CCB;

// 目标设备ID索引目的信令点编码的数据结构
typedef struct _tagSAAPPidDstDeviceIDCcb
{
    XU32                 pidNo;           // 设备号(32bit)
    XU16                 DstID;           // 信令点编码索引值
}SAAP_PID_INDEX_DSTDEVICEID_CCB;

#ifdef SCALE_CPU_VX
    /*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

//------------------------函数声明------------------------------------

//分配全局内存 与 HASH 表
XS32 SAAP_MallocGlobalMemory(void);

// tell 号段表操作函数
XU16 SAAP_InsertTelToDstIDTable(XU32 ulHashKey, XU32 ulHashValue,XU32 ulRouteTblId);
XU16 SAAP_SearchTellToDstIDTable(XU32 ulHashKey,XU32 ulRouteTblId);
XU16 SAAP_DelTellToDstIDTable(XU16 usIpNo);

void SAAP_InitTellToDstIDCb(XU16 usCcNo);

//通过Hash 关键字计算Hash表下标
XS32 SAAP_GetTellToDstIDHashIndexFromHashKey(XU32 ulHashKey,XU32 *pulHashCntnIndex);

// hlr 号段表操作函数
XU16 SAAP_InsertHlrToDstIDTable(XU32 ulHashKey, XU32 ulHashValue,XU32 ulRouteTblId);
XU16 SAAP_SearchHlrToDstIDTable(XU32 ulHashKey,XU32 ulRouteTblId);
XU16 SAAP_DelHlrToDstIDTable(XU16 usIpNo);

void SAAP_InitHlrToDstIDCb(XU16 usCcNo);

//通过Hash 关键字计算Hash表下标
XS32 SAAP_GetHlrToDstIDHashIndexFromHashKey(XU32 ulHashKey,XU32 *pulHashCntnIndex);

// 设备ID表操作函数
XU16 SAAP_InsertPidToDstIDTable(XU32 ulHashKey, XU32 ulHashValue);
XU16 SAAP_SearchPidToDstIDTable(XU32 ulHashKey);
XU16 SAAP_DelPidToDstIDTable(XU16 usIpNo);
void SAAP_InitPidToDstIDCb(XU16 usCcNo);
XS32 SAAP_GetPidToDstIDHashIndexFromHashKey(XU32 ulHashKey,
XU32 *pulHashCntnIndex);//通过Hash 关键字计算Hash表下标

#ifdef  __cplusplus
    }
#endif

#endif


