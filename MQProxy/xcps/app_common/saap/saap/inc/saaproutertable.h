/*----------------------------------------------------------------------
saapRouterTable.h - ȫ�ֺ������Ͷ���

��Ȩ���� 2004 -2006 ������˾������BSC��Ŀ��.

author: �ź�

�޸���ʷ��¼
--------------------
��Ӱ�Ȩ˵��.
----------------------------------------------------------------------*/

#ifndef _SAAPROUTERTABLE_H_
#define _SAAPROUTERTABLE_H_

#ifdef  __cplusplus
    extern  "C"
    {
#endif

#pragma pack(1)

//------------------------�궨��------------------------------------
#define    SAAP_LOCAL_PREFIX_NUM    255
#define    SAAP_MAX_SAAPCCB_NUM     255
#define    SAAP_ERROR_NUM           0xFFFF

/* HASH�������� */
#define SAAP_HASH_TABLE_SIZE    SAAP_MAX_SAAPCCB_NUM    /*HASH  ��ߴ�*/
#define SAAP_HASH_KEY_MASK      SAAP_MAX_SAAPCCB_NUM    /*HASH  ��ؼ�������*/

//------------------------�ṹ�͹����嶨��----------------------------
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

// SAAP ����GT���ݽṹ��װ
typedef struct SAAP_GLOBAL_TELLNO_Struct
{
    XU8      flag ;      // ��������, 1 ��ʾ������Ч
    XU32     tellNo;     // �����豸��
}SAAP_GLOBAL_TELLNO_STRUCT;

typedef struct SAAP_GLOBAL_HLRNO_Struct
{
    XU8      flag ;   // ��������, 1 ��ʾ������Ч
    XU16     hlrNo;   // �����豸��
}SAAP_GLOBAL_HLRNO_STRUCT;

typedef struct SAAP_GLOBAL_EID_tag
{
    XU8      flag ;        // ��������, 1 ��ʾ������Ч
    XU32     eidNo;        // �����豸��
    XU32     nodeType ;    //�豸����
}SAAP_GLOBAL_EID_STRUCT;

typedef struct
{
    XU8      flag;       // ��������, 1 ��ʾ������Ч
    XU32     nodeType;   // �����豸��
}SAAP_GLOBAL_NODE_TYPE_STRUCT;

typedef struct tag_SAAP_GLOBAL
{
    SAAP_GLOBAL_EID_STRUCT          eidNoCfg;                       // �����豸��(ֻ��һ��)
    SAAP_GLOBAL_HLRNO_STRUCT        hlrNoCfg[SAAP_LOCAL_PREFIX_NUM];   // ����hlr�Ŷ�ֵ(hlrר��)
    SAAP_GLOBAL_TELLNO_STRUCT       tellNoCfg[SAAP_LOCAL_PREFIX_NUM];  // ����tell�Ŷ�ֵ(hlrר��)
}SAAP_GLOBAL_STRUCT;

// tell�Ŷ� ����Ŀ��������������ݽṹ
typedef struct _tagSAAPTelDstDeviceIDCcb
{
    XU32      tellNo;           // �Ŷ�: ���� + �ֺ�(���4λ)
    XU16      DstID ;           // ������������ֵ
}SAAP_TELL_INDEX_DSTDEVICEID_CCB;

// hlr�Ŷ�����Ŀ��������������ݽṹ
typedef struct _tagSAAPHlrDstDeviceIDCcb
{
    XU16      hlrNo;            // HLR�Ŷ�ֵ, 16bit
    XU16      DstID ;           // ������������ֵ
}SAAP_HLR_INDEX_DSTDEVICEID_CCB;

// Ŀ���豸ID����Ŀ��������������ݽṹ
typedef struct _tagSAAPPidDstDeviceIDCcb
{
    XU32                 pidNo;           // �豸��(32bit)
    XU16                 DstID;           // ������������ֵ
}SAAP_PID_INDEX_DSTDEVICEID_CCB;

#ifdef SCALE_CPU_VX
    /*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

//------------------------��������------------------------------------

//����ȫ���ڴ� �� HASH ��
XS32 SAAP_MallocGlobalMemory(void);

// tell �Ŷα��������
XU16 SAAP_InsertTelToDstIDTable(XU32 ulHashKey, XU32 ulHashValue,XU32 ulRouteTblId);
XU16 SAAP_SearchTellToDstIDTable(XU32 ulHashKey,XU32 ulRouteTblId);
XU16 SAAP_DelTellToDstIDTable(XU16 usIpNo);

void SAAP_InitTellToDstIDCb(XU16 usCcNo);

//ͨ��Hash �ؼ��ּ���Hash���±�
XS32 SAAP_GetTellToDstIDHashIndexFromHashKey(XU32 ulHashKey,XU32 *pulHashCntnIndex);

// hlr �Ŷα��������
XU16 SAAP_InsertHlrToDstIDTable(XU32 ulHashKey, XU32 ulHashValue,XU32 ulRouteTblId);
XU16 SAAP_SearchHlrToDstIDTable(XU32 ulHashKey,XU32 ulRouteTblId);
XU16 SAAP_DelHlrToDstIDTable(XU16 usIpNo);

void SAAP_InitHlrToDstIDCb(XU16 usCcNo);

//ͨ��Hash �ؼ��ּ���Hash���±�
XS32 SAAP_GetHlrToDstIDHashIndexFromHashKey(XU32 ulHashKey,XU32 *pulHashCntnIndex);

// �豸ID���������
XU16 SAAP_InsertPidToDstIDTable(XU32 ulHashKey, XU32 ulHashValue);
XU16 SAAP_SearchPidToDstIDTable(XU32 ulHashKey);
XU16 SAAP_DelPidToDstIDTable(XU16 usIpNo);
void SAAP_InitPidToDstIDCb(XU16 usCcNo);
XS32 SAAP_GetPidToDstIDHashIndexFromHashKey(XU32 ulHashKey,
XU32 *pulHashCntnIndex);//ͨ��Hash �ؼ��ּ���Hash���±�

#ifdef  __cplusplus
    }
#endif

#endif


