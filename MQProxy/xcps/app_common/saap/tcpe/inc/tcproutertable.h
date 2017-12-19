/*----------------------------------------------------------------------
    saapRouterTable.h - ȫ�ֺ������Ͷ���

    ��Ȩ���� 2004 -2006 ������˾������BSC��Ŀ��.

    author: �ź�

    �޸���ʷ��¼
    --------------------
    ��Ӱ�Ȩ˵��.
----------------------------------------------------------------------*/

#ifndef _TCPROUTERTABLE_H_
#define _TCPROUTERTABLE_H_

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

//------------------------�궨��------------------------------------

#define    TCP_MAX_IPROUTER_NUM     255

#define    TCP_ERROR_NUM   0xFFFF

/* HASH�������� */
#define  TCP_DSTID_TO_LINT_HTABLE_SIZE     TCP_MAX_IPROUTER_NUM     /*HASH  ��ߴ�*/
#define  TCP_IDSTID_TO_LINT_HKEY_MASK      TCP_MAX_IPROUTER_NUM     /*HASH  ��ؼ�������*/

//------------------------�ṹ�͹����嶨��----------------------------

// �˴����������ṹ��Ϊ�˴�DSTID��IP�����ܴ�IP��DSTID�ĸ�Ч����

// �����������������ݽṹ
typedef struct _tagTCPDstID_ip_Ccb
{
    XU16      DstDeviceID ;     // Ŀ����������
    XU16      usLintIndex ;     // ��·������

}TCP_DSTID_INDEX_IP_CCB;

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

//------------------------��������------------------------------------

//ͨ��Hash �ؼ��ּ���Hash���±�
XS32 TCP_GetDstIDToIPHashIndexFromHashKey(XU32 ulHashKey,XU32 *pulHashCntnIndex);
XU16 TCP_InsertDstIDToIpTable(XU16 ulHashKey, XU16 ulHashValue);
XU16 TCP_SearchDstIDToIpTable(XU16 ulHashKey);

void TCP_InitDstIDToIpCcb(XU16 usCcbNo);

XS32 TCP_init_DstID_table(void) ; // �ڴ����ϣ���ʼ����ʽ

// �˺�������������·���ñ������
XU32 tcp_SetDstIDtoLintTable( XU16  DstId , XU16 LintNo );
XU32 TCP_deleteDstIDtoIPTable(XU16 DstId);

#ifdef  __cplusplus
}
#endif

#endif


