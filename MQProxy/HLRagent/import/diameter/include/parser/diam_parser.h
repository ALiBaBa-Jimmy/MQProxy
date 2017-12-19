/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��9��
**************************************************************************/
#ifndef __DIAM_PARSER_H__
#define __DIAM_PARSER_H__

#include <parser/diam_msgbuffer.h>
#include <parser/diam_avpcontainer.h>
#include <api/diam_message.h>

class CDiamEmptyClass {};

/**************************************************************************
��    ��: AAAParser
�� �� ��: ��Ϣ������ģ����
ʱ    ��: 2012��5��9��
**************************************************************************/
template <class RAWDATA, class APPDATA, class DICTDATA = CDiamEmptyClass>
class CDiamParser
{
public:
    //����/��������
    CDiamParser() {}
    virtual ~CDiamParser() {};

    //����Ϣ���н����ͷ�װ
    virtual void parseRawToApp();
    virtual void parseAppToRaw();

    //�������ݶ���ͽ�����װ����
    void setRawData(RAWDATA data) {
        rawData = data;
    }
    void setAppData(APPDATA data) {
        appData = data;
    }
    void setDictData(DICTDATA data) {
        dictData = data;
    }

    //��ȡԭʼ����
    RAWDATA getRawData() {
        return rawData;
    }
    template <class T> void getRawData(T*& data) {
        data = (T*)rawData;
    }

    //��ȡ�����������
    APPDATA getAppData() {
        return appData;
    }
    template <class T> void getAppData(T*& data) {
        data = (T*)appData;
    }

    //��ȡ�ֵ�����
    DICTDATA getDictData() {
        return dictData;
    }
    template <class T> void getDictData(T*& data) {
        data = (T*)dictData;
    }

private:
    RAWDATA rawData;
    APPDATA appData;
    DICTDATA dictData;
};
//diameter��Ϣͷ������
typedef CDiamParser<DiamMsgBlock*, DiamHeader*, ParseOption> DiamMsgHeaderParser;
//diameter��Ϣ�غɷ�����
typedef CDiamParser<DiamMsgBlock*, DiamBody*, CDictObject*>  DiamMsgPayloadParser;
//avp value������
typedef CDiamParser<DiamMsgBlock*, CAvpContainerEntry*, CDictObjectAvp*> DiamAvpValueParser;

template<> void DiamMsgHeaderParser::parseRawToApp();
template<> void DiamMsgHeaderParser::parseAppToRaw();

template<> void DiamMsgPayloadParser::parseRawToApp();
template<> void DiamMsgPayloadParser::parseAppToRaw();

template<> void DiamAvpValueParser::parseRawToApp();
template<> void DiamAvpValueParser::parseAppToRaw();

class DiamMsgCodec
{
public:
    static DiamMsgBlock* EncodeDiamMsg(DiamMsg* msg);
    static DiamMsg* DecodeDiamMsg();
};




#endif /*__DIAM_PARSER_H__*/


