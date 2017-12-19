/*----------------------------------------------------------------------
    saapRouterTable.h - 全局宏与类型定义

    版权所有 2004 -2006 信威公司深研所BSC项目组.

    author: 张海

    修改历史记录
    --------------------
    添加版权说明.
----------------------------------------------------------------------*/

#ifndef _TCPROUTERTABLE_H_
#define _TCPROUTERTABLE_H_

#ifdef  __cplusplus
extern  "C"
{
#endif

#pragma pack(1)

//------------------------宏定义------------------------------------

#define    TCP_MAX_IPROUTER_NUM     255

#define    TCP_ERROR_NUM   0xFFFF

/* HASH表常量定义 */
#define  TCP_DSTID_TO_LINT_HTABLE_SIZE     TCP_MAX_IPROUTER_NUM     /*HASH  表尺寸*/
#define  TCP_IDSTID_TO_LINT_HKEY_MASK      TCP_MAX_IPROUTER_NUM     /*HASH  表关键字掩码*/

//------------------------结构和共用体定义----------------------------

// 此处定义两个结构是为了从DSTID到IP，又能从IP到DSTID的高效索引

// 信令点编码索引的数据结构
typedef struct _tagTCPDstID_ip_Ccb
{
    XU16      DstDeviceID ;     // 目的信令点编码
    XU16      usLintIndex ;     // 链路索引号

}TCP_DSTID_INDEX_IP_CCB;

#ifdef SCALE_CPU_VX
/*for xscale ld func relocate alignment. 2007.4.27*/
#pragma pack(0)
#else
#pragma pack()
#endif

//------------------------函数声明------------------------------------

//通过Hash 关键字计算Hash表下标
XS32 TCP_GetDstIDToIPHashIndexFromHashKey(XU32 ulHashKey,XU32 *pulHashCntnIndex);
XU16 TCP_InsertDstIDToIpTable(XU16 ulHashKey, XU16 ulHashValue);
XU16 TCP_SearchDstIDToIpTable(XU16 ulHashKey);

void TCP_InitDstIDToIpCcb(XU16 usCcbNo);

XS32 TCP_init_DstID_table(void) ; // 内存与哈希表初始化函式

// 此函数完成信令点链路配置表的配置
XU32 tcp_SetDstIDtoLintTable( XU16  DstId , XU16 LintNo );
XU32 TCP_deleteDstIDtoIPTable(XU16 DstId);

#ifdef  __cplusplus
}
#endif

#endif


