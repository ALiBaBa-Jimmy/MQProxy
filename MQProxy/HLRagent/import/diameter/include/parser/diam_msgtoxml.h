#ifndef __DIAM_MSG_TO_XML_H__
#define __DIAM_MSG_TO_XML_H__

#include <sstream>
#include <string>
#include <parser/diam_datatype.h>
#include <api/diam_message.h>

class AAAXmlElement
{
public:
    AAAXmlElement(const char *name) :
        m_name(name) {
    }
    void SetText(const char *text);
    void SetText(DiamUINT32 num);
    void SetText(DiamUINT64 num);
    void SetText(DiamURI &uri);
    void SetAttribute(const char *name, const char *value);
    void SetAttribute(const char *name, DiamUINT32 num);
    void SetAttribute(const char *name, DiamUINT64 num);
    std::string Output();
    void Reset();

private:
    std::string m_name;
    std::string m_attributes;
    std::string m_value;
};

class AAAXmlWriter
{
public:
    AAAXmlWriter() { };

    void writeToString(DiamMsg *msg, std::string &output);

protected:
    DiamRetCode Walk(DiamBody &avplist, std::string &output);
};

/*!
 *  <Message>
 *     <version>value</version>
 *     <flags request="value" proxiable="value" error="value" retrans="value"></flags>
 *     <code>value</code>
 *     <appId>value</appId>
 *     <HopId>value</HopId>
 *     <EndId>value</EndId>
 *     <avp>
 *        <"avpname">value</avp>
 *          .
 *          .
 *        <"avpname">
 *           <"avpname">value</"avpname">
 *           <"avpname">value</"avpname">
 *               .
 *               .
 *           <"avpname">
 *              <"avpname">value</"avpname">
 *                 .
 *                 .
 *              </"avpname">
 *        </"avpname">
 *     </avp>
 *  </Message>
 */

class AAADiameterMsgToXML
{
public:
    static void Convert(DiamMsg *msg);
};

#endif // __DIAM_MSG_TO_XML_H__
