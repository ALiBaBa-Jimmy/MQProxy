/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月9日
**************************************************************************/
#ifndef __DIAM_PARSER_H__
#define __DIAM_PARSER_H__

#include <parser/diam_msgbuffer.h>
#include <parser/diam_avpcontainer.h>
#include <api/diam_message.h>

class CDiamEmptyClass {};

/**************************************************************************
类    名: AAAParser
类 功 能: 消息解析的模板类
时    间: 2012年5月9日
**************************************************************************/
template <class RAWDATA, class APPDATA, class DICTDATA = CDiamEmptyClass>
class CDiamParser
{
public:
    //构造/析构函数
    CDiamParser() {}
    virtual ~CDiamParser() {};

    //对消息进行解析和封装
    virtual void parseRawToApp();
    virtual void parseAppToRaw();

    //设置数据对象和解析封装对象
    void setRawData(RAWDATA data) {
        rawData = data;
    }
    void setAppData(APPDATA data) {
        appData = data;
    }
    void setDictData(DICTDATA data) {
        dictData = data;
    }

    //获取原始数据
    RAWDATA getRawData() {
        return rawData;
    }
    template <class T> void getRawData(T*& data) {
        data = (T*)rawData;
    }

    //获取解析后的数据
    APPDATA getAppData() {
        return appData;
    }
    template <class T> void getAppData(T*& data) {
        data = (T*)appData;
    }

    //获取字典数据
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
//diameter消息头分析类
typedef CDiamParser<DiamMsgBlock*, DiamHeader*, ParseOption> DiamMsgHeaderParser;
//diameter消息载荷分析类
typedef CDiamParser<DiamMsgBlock*, DiamBody*, CDictObject*>  DiamMsgPayloadParser;
//avp value分析类
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


