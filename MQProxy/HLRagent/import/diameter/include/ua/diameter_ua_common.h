/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename:       diameter_ua_common.h
**  description:    diameter ua 公共接口
**  author:         luozhongjie
**  data:           2014.10.21
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   luozhongjie     2014.10.21        create
**************************************************************/
#ifndef DIAMETER_UA_COMMON_H_
#define DIAMETER_UA_COMMON_H_
#include <util/diam_config.h>

//兼容C include
#ifdef __cplusplus
#include <api/diam_datatype.h>
#include <sstream>
typedef DiamINT8 DUA_INT8;
typedef DiamUINT8 DUA_UINT8;
typedef DiamINT16 DUA_INT16;
typedef DiamUINT16 DUA_UINT16;
typedef DiamINT32 DUA_INT32;
typedef DiamUINT32 DUA_UINT32;
typedef DiamINT64 DUA_INT64;
typedef DiamUINT64 DUA_UINT64;
DIAMETER_EXPORT void diam_get_link_info(std::stringstream& output);
#else
typedef char DUA_INT8;
typedef unsigned char DUA_UINT8;
typedef short DUA_INT16;
typedef unsigned short DUA_UINT16;
typedef long DUA_INT32;
typedef unsigned long DUA_UINT32;
typedef unsigned long long DUA_UINT64;
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 


#define DUA_CONST const
#define DUA_STATIC static
#define DUA_NULL 0

typedef enum
{
    DUA_RET_SUCC = 0,
    DUA_RET_ERROR ,
    DUA_RET_BREAK , // 未找到信元
    DUA_RET_INVALID_BUFFER_LENGTH ,
    DUA_RET_INVALID_MESSAGE,
    DUA_RET_INVALID_SIZE,
    DUA_RET_INVALID_OCTETSTRING_LEN_FROM_NET,
    DUA_RET_INVALID_UTF8STRING_LEN_FROM_NET,
}DUA_RETURN;
typedef enum
{
    DUA_IPV4 = 0x01,
    DUA_IPV6 = 0x02,
    DUA_IPV4_AND_IPV6 = 0x03,
}DUA_IP_TYPE;


typedef void DUA_VOID;

typedef DUA_UINT32 DuaIpv4;
typedef DUA_UINT8 DuaIpv6[16];
typedef struct SDuaIp
{
    DUA_IP_TYPE ip_type;
    DuaIpv4 ipv4;
    DuaIpv6 ipv6;
}DuaIp;

#define DUA_MemCpy ACE_OS::memcpy
#define DUA_StrNcpy(dest,src, len) ACE_OS::strncpy((DUA_INT8 *)(dest), (DUA_CONST DUA_INT8 *)(src), len)
#define DUA_StrLen ACE_OS::strlen

#define DIAMETER_UA_REQUEST_ID(code) ((code << 1) + 1)
#define DIAMETER_UA_ANSWER_ID(code) (code << 1)
#define DIAMETER_UA_IS_REQUEST(MsgId) (MsgId&0x1)

#define DIAMETER_UA_MAX_SIZE 100

typedef DUA_UINT32 DiameterUaUndefinedIE;
typedef DUA_UINT32 DiameterUaBitmap;
typedef DUA_UINT32 DiameterUaListSize;
typedef DUA_UINT32 DiameterUaCommonStringSize;
typedef DUA_UINT16 DiameterUaOctetStringLength;

typedef struct
{
    DUA_UINT32 msg_id;
    DUA_UINT32 size;
    DUA_UINT32 app_id;
    DUA_UINT32 peer_fid;

    // DiameterUaMsgInfo.avps 说明
    // 用于映射消息结构体
    // 禁止空格
    // 前缀*标识必选,对optional avp有效
    // 只支持command定制，其他avp使用product.xml中的定义
    // 结构体成员需要4字节对齐，或者使用#pragma pack(1)
    DUA_CONST DUA_INT8 *avps; 
}DiameterUaMsgInfo;

#define DUA_INIT_GROUP(group_entity) {XOS_MemSet(&(group_entity),0,sizeof(group_entity)); (group_entity).bitmap = ~0UL;}
#define DUA_CHECK_BITMAP_IS_TRUE(group_entity, elem_bitmap) (group_entity.bitmap & elem_bitmap)
#define DUA_SET_BITMAP(group_entity, elem_bitmap) {group_entity.bitmap |= elem_bitmap;}
#define DUA_CLEAR_BITMAP(group_entity, elem_bitmap) {group_entity.bitmap &= ~elem_bitmap;}

DIAMETER_EXPORT DUA_VOID DuaSetIpv4(DuaIp* ip_struct, DuaIpv4 ipv4_host);
DIAMETER_EXPORT DUA_VOID DuaSetIpv6(DuaIp* ip_struct, DuaIpv6 ipv6_host);
#ifdef __cplusplus
}
#endif // __cplusplus 
#endif // DIAMETER_UA_COMMON_H_