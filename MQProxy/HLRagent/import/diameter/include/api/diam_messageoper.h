/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年8月28日
**************************************************************************/
#ifndef __DIAM_MESSMANAGER_H__
#define __DIAM_MESSMANAGER_H__

#include <api/diam_message.h>

#ifdef  __cplusplus
extern  "C" {
#endif
/**************************************************************************
函 数 名:
函数功能: 生成HopByHopID和EndToEndID
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getHopByHop();
DIAMETER_EXPORT DiamUINT32 getEndToEnd();

/**************************************************************************
函 数 名: setAssociateId
函数功能: 设置业务使用的消息关联ID
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT void setAssociateId(DiamMsg& msg, DiamINT32 value);

/**************************************************************************
函 数 名: getAssociateId
函数功能: 获取业务使用的消息关联ID
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getAssociateId(DiamMsg& msg);

/**************************************************************************
函 数 名: requestMsg
函数功能: 生成一个空的请求消息
参    数: DiameterApplicationId DiameterCommandCode
返 回 值:
**************************************************************************/
DIAMETER_EXPORT void initDiameterMsg(DiamMsg& msg, DiamAppId id);

/**************************************************************************
函 数 名: requestMsg
函数功能: 生成一个空的请求消息
参    数: DiameterApplicationId DiameterCommandCode
返 回 值:
**************************************************************************/
DIAMETER_EXPORT void requestMsg(DiamMsg& msg, DiamAppId id, DiamCommandCode code);

/**************************************************************************
函 数 名: answerMsg
函数功能: 生成一个空的应答消息
参    数: DiameterApplicationId DiameterCommandCode
返 回 值:
**************************************************************************/
DIAMETER_EXPORT void answerMsg(DiamMsg&msg, DiamAppId id, DiamCommandCode code);

/**************************************************************************
函 数 名: setSessionId
函数功能: 将sessionid添加到dest消息中
参    数: dest 目的消息
返 回 值: void
**************************************************************************/
DIAMETER_EXPORT void setSessionId(std::string& sessionId, DiamMsg& msg);

/**************************************************************************
函 数 名: getSessionId
函数功能: 获取消息的sessionid
参    数: 消息
返 回 值: void
**************************************************************************/
DIAMETER_EXPORT DiamUTF8String* getSessionId(DiamMsg& msg);

/**************************************************************************
函 数 名: DiameterMsgPrint
函数功能: 打印diameter消息
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT void DiameterMsgPrint(DiamMsg& msg);

/**************************************************************************
函 数 名: addAvp
函数功能: 向消息中添加AVP,添加的方式是追加的
参    数:
返 回 值:
**************************************************************************/
//DIAMETER_EXPORT void addAvpCommstring(DiamBody& msg, const char* name,DiamOctetString& value);
//DIAMETER_EXPORT void addAvpOctetstring(DiamBody& msg, const char* name, const DiamOctetString& value);
DIAMETER_EXPORT void addAvpOctetstring(DiamBody& msg, const char* name, const char* value, DiamUINT32 length = 0);
DIAMETER_EXPORT void addAvpUTF8string(DiamBody& msg, const char* name, const char* value);
DIAMETER_EXPORT void addAvpIdentity(DiamBody& msg, const char* name, const char* value);
DIAMETER_EXPORT void addAvpAddress(DiamBody& msg, const char* name, DiamAddress& value);
DIAMETER_EXPORT void addAvpInt16(DiamBody& msg, const char* name, DiamINT16 value);
DIAMETER_EXPORT void addAvpUInt16(DiamBody& msg, const char* name, DiamUINT16 value);
DIAMETER_EXPORT void addAvpInt32(DiamBody& msg, const char* name, DiamINT32 value);
DIAMETER_EXPORT void addAvpUInt32(DiamBody& msg, const char* name, DiamUINT32 value);
DIAMETER_EXPORT void addAvpInt64(DiamBody& msg, const char* name, DiamINT64 value);
DIAMETER_EXPORT void addAvpUInt64(DiamBody& msg, const char* name, DiamUINT64 value);
DIAMETER_EXPORT void addAvpURI(DiamBody& msg, const char* name, DiamURI& value);
DIAMETER_EXPORT void addAvpEnum(DiamBody& msg, const char* name, DiamEnum value);
DIAMETER_EXPORT void addAvpTime(DiamBody& msg, const char* name, DiamTime value);
DIAMETER_EXPORT DiamGroup& addAvpGroup(DiamBody& msg, const char* name);

/**************************************************************************
函 数 名: void getAvp(DiameterGroup& msg, const char* name, *, unsigned int index=0);
函数功能: 从Group类型的avp中获取avp的值
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getAvpSize(DiamBody& msg, const char* name);
//DIAMETER_EXPORT DiamOctetString*  getAvpCommstring(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamOctetString* getAvpOctetstring(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamUTF8String*  getAvpUTF8string(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamIdentity*    getAvpIdentity(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamAddress*     getAvpAddress(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamINT16*       getAvpInt16(DiamBody& group, const char* name, DiamUINT16 index = 0);
DIAMETER_EXPORT DiamUINT16*      getAvpUInt16(DiamBody& group, const char* name, DiamUINT16 index = 0);
DIAMETER_EXPORT DiamINT32*       getAvpInt32(DiamBody& group, const char* name, DiamINT32 index = 0);
DIAMETER_EXPORT DiamUINT32*      getAvpUInt32(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamINT64*       getAvpInt64(DiamBody& group, const char* name, DiamINT64 index = 0);
DIAMETER_EXPORT DiamUINT64*      getAvpUInt64(DiamBody& group, const char* name, DiamUINT64 index = 0);
DIAMETER_EXPORT DiamURI*         getAvpURI(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamEnum*        getAvpEnum(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamTime*        getAvpTime(DiamBody& group, const char* name, DiamUINT32 index = 0);
DIAMETER_EXPORT DiamGroup*       getAvpGroup(DiamBody& group, const char* name, DiamUINT32 index = 0);

/**************************************************************************
函 数 名: delAvp
函数功能: group中删除AVP
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT void delAvp(DiamBody& group, const char* name);
/**************************************************************************
函 数 名: getResultCode
函数功能: 取回返回码
参    数: CMessage &msg: diameter消息
返 回 值: 返回码
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getResultCode(DiamMsg &msg);

/**************************************************************************
函 数 名: setResultCode
函数功能: 向消息中添加返回码
参    数: CMessage& msg: diameter消息 DiamUINT32 code:返回码
返 回 值:
**************************************************************************/
DIAMETER_EXPORT void setResultCode(DiamMsg& msg, DiamUINT32 code);

/**************************************************************************
函 数 名: getExperimentalResultCode
函数功能: 获取BWT扩展的返回码
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getExperimentalResultCode(DiamMsg &msg);

/**************************************************************************
函 数 名: setExperimentalResultCode
函数功能: 设置BWT扩展的返回码
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT void setExperimentalResultCode(DiamMsg& msg, DiamUINT32 code);

/**************************************************************************
函 数 名: interpretedResultCode
函数功能:
参    数:
返 回 值:
**************************************************************************/
DIAMETER_EXPORT RCODE interpretedResultCode(DiamUINT32 code);

#ifdef __cplusplus
}
#endif

#endif //__DIAM_MESSMANAGER_H__
