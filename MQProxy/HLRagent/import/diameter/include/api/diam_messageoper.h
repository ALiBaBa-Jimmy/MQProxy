/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��8��28��
**************************************************************************/
#ifndef __DIAM_MESSMANAGER_H__
#define __DIAM_MESSMANAGER_H__

#include <api/diam_message.h>

#ifdef  __cplusplus
extern  "C" {
#endif
/**************************************************************************
�� �� ��:
��������: ����HopByHopID��EndToEndID
��    ��:
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getHopByHop();
DIAMETER_EXPORT DiamUINT32 getEndToEnd();

/**************************************************************************
�� �� ��: setAssociateId
��������: ����ҵ��ʹ�õ���Ϣ����ID
��    ��:
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT void setAssociateId(DiamMsg& msg, DiamINT32 value);

/**************************************************************************
�� �� ��: getAssociateId
��������: ��ȡҵ��ʹ�õ���Ϣ����ID
��    ��:
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getAssociateId(DiamMsg& msg);

/**************************************************************************
�� �� ��: requestMsg
��������: ����һ���յ�������Ϣ
��    ��: DiameterApplicationId DiameterCommandCode
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT void initDiameterMsg(DiamMsg& msg, DiamAppId id);

/**************************************************************************
�� �� ��: requestMsg
��������: ����һ���յ�������Ϣ
��    ��: DiameterApplicationId DiameterCommandCode
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT void requestMsg(DiamMsg& msg, DiamAppId id, DiamCommandCode code);

/**************************************************************************
�� �� ��: answerMsg
��������: ����һ���յ�Ӧ����Ϣ
��    ��: DiameterApplicationId DiameterCommandCode
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT void answerMsg(DiamMsg&msg, DiamAppId id, DiamCommandCode code);

/**************************************************************************
�� �� ��: setSessionId
��������: ��sessionid��ӵ�dest��Ϣ��
��    ��: dest Ŀ����Ϣ
�� �� ֵ: void
**************************************************************************/
DIAMETER_EXPORT void setSessionId(std::string& sessionId, DiamMsg& msg);

/**************************************************************************
�� �� ��: getSessionId
��������: ��ȡ��Ϣ��sessionid
��    ��: ��Ϣ
�� �� ֵ: void
**************************************************************************/
DIAMETER_EXPORT DiamUTF8String* getSessionId(DiamMsg& msg);

/**************************************************************************
�� �� ��: DiameterMsgPrint
��������: ��ӡdiameter��Ϣ
��    ��:
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT void DiameterMsgPrint(DiamMsg& msg);

/**************************************************************************
�� �� ��: addAvp
��������: ����Ϣ�����AVP,��ӵķ�ʽ��׷�ӵ�
��    ��:
�� �� ֵ:
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
�� �� ��: void getAvp(DiameterGroup& msg, const char* name, *, unsigned int index=0);
��������: ��Group���͵�avp�л�ȡavp��ֵ
��    ��:
�� �� ֵ:
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
�� �� ��: delAvp
��������: group��ɾ��AVP
��    ��:
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT void delAvp(DiamBody& group, const char* name);
/**************************************************************************
�� �� ��: getResultCode
��������: ȡ�ط�����
��    ��: CMessage &msg: diameter��Ϣ
�� �� ֵ: ������
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getResultCode(DiamMsg &msg);

/**************************************************************************
�� �� ��: setResultCode
��������: ����Ϣ����ӷ�����
��    ��: CMessage& msg: diameter��Ϣ DiamUINT32 code:������
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT void setResultCode(DiamMsg& msg, DiamUINT32 code);

/**************************************************************************
�� �� ��: getExperimentalResultCode
��������: ��ȡBWT��չ�ķ�����
��    ��:
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT DiamUINT32 getExperimentalResultCode(DiamMsg &msg);

/**************************************************************************
�� �� ��: setExperimentalResultCode
��������: ����BWT��չ�ķ�����
��    ��:
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT void setExperimentalResultCode(DiamMsg& msg, DiamUINT32 code);

/**************************************************************************
�� �� ��: interpretedResultCode
��������:
��    ��:
�� �� ֵ:
**************************************************************************/
DIAMETER_EXPORT RCODE interpretedResultCode(DiamUINT32 code);

#ifdef __cplusplus
}
#endif

#endif //__DIAM_MESSMANAGER_H__
