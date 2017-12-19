#ifndef __UTIL_XML_SAX_PARSER_H__
#define __UTIL_XML_SAX_PARSER_H__

#include <iostream>
#include <list>
#include <map>
#include <ACEXML/common/FileCharStream.h>
#include <ACEXML/parser/parser/Parser.h>
#include <ACEXML/common/DefaultHandler.h>

using namespace std;

class util_XML_SaxException
{
public:
    util_XML_SaxException() :
        m_Code(0)
    {
    }
    util_XML_SaxException(const char *desc, ACE_UINT32 code = 0) :
        m_Code(code),
        m_description(desc)
    {
    }
    util_XML_SaxException(std::string &desc, ACE_UINT32 code = 0) :
        m_Code(code),
        m_description(desc)
    {
    }
    std::string &Description()
    {
        return m_description;
    }
    ACE_UINT32 &Code()
    {
        return m_Code;
    }
    void Print()
    {
        std::cout << "SAX Parsing exception: "
                  << m_description
                  << std::endl;
    }

protected:
    ACE_UINT32 m_Code;
    std::string m_description;
};

class util_XML_Element;
typedef std::list<util_XML_Element*> OD_Utl_XML_ElementStack;

class util_XML_Element
{
public:
    util_XML_Element(const char* name, OD_Utl_XML_ElementStack &stack) :
        m_inProcess(false),
        m_name(name),
        m_callStack(stack),
        m_parent(NULL),
        m_numInstance(0)
    {
    }
    virtual ~util_XML_Element()
    {
    }
    std::string &Name()
    {
        return m_name;
    }
    int NumInstance()
    {
        return m_numInstance;
    }
    virtual bool startElement(ACEXML_Attributes *atts)
    {
        if (m_inProcess)
        {
            std::string err = "Error: element ";
            err += m_name;
            err += "already in process";
            throw util_XML_SaxException(err);
        }
        m_numInstance ++;
        m_inProcess = true;
        if (! m_callStack.empty())
        {
            m_parent = m_callStack.front();
        }
        m_callStack.push_front(this);
        return true;
    }
    virtual bool characters(const ACEXML_Char *ch,
                            size_t start,
                            size_t ACEXML_ENV_ARG_DECL)
    {
        if (! m_inProcess)
        {
            std::string err = "Error: element ";
            err += m_name;
            err += "not in process";
            throw util_XML_SaxException(err);
        }
        return true;
    }
    virtual bool endElement()
    {
        if (! m_inProcess)
        {
            std::string err = "Error: element ";
            err += m_name;
            err += "not in process";
            throw util_XML_SaxException(err);
        }
        m_inProcess = false;
        m_callStack.pop_front();
        m_parent = NULL;
        return true;
    }
    OD_Utl_XML_ElementStack &CallStack()
    {
        return m_callStack;
    }
    util_XML_Element *Parent()
    {
        return m_parent;
    }

private:
    bool m_inProcess;
    std::string m_name;
    OD_Utl_XML_ElementStack &m_callStack;
    util_XML_Element *m_parent;
    int m_numInstance;
};

typedef std::map<std::string, util_XML_Element*> OD_Utl_XML_ElementMap;
typedef std::pair<std::string, util_XML_Element*> OD_Utl_XML_ElementPair;

class util_XML_SaxParser
{
public:
    util_XML_SaxParser(int numPasses = 1) :
        m_numPasses(numPasses),
        m_currentElement(NULL)
    {
    }
    virtual ~util_XML_SaxParser()
    {
    }
    OD_Utl_XML_ElementStack &callStack()
    {
        return m_callStack;
    }
    OD_Utl_XML_ElementMap &elementMap()
    {
        return m_elementMap;
    }
    virtual void Load(const char* xmlFile);

    virtual void characters(const ACEXML_Char *ch,
                            size_t start,
                            size_t ACEXML_ENV_ARG_DECL);
    virtual void startDocument();
    virtual void endDocument();
    virtual void startElement(const ACEXML_Char *namespaceURI,
                              const ACEXML_Char *localName,
                              const ACEXML_Char *qName,
                              ACEXML_Attributes *atts);
    virtual void endElement(const ACEXML_Char *namespaceURI,
                            const ACEXML_Char *localName,
                            const ACEXML_Char *qName);

private:
    ACE_UINT32 m_numPasses;
    util_XML_Element *m_currentElement;
    OD_Utl_XML_ElementMap m_elementMap;
    OD_Utl_XML_ElementStack m_callStack;
};

template<class T, class C>
class util_XML_RegisteredElement : public util_XML_Element
{
public:
    util_XML_RegisteredElement(T &arg, const char *name, util_XML_SaxParser &parser) :
        util_XML_Element(name, parser.callStack()),
        m_arg(arg)
    {
        parser.elementMap().insert(OD_Utl_XML_ElementPair(this->Name(), this));
    }
    T &Arg()
    {
        return m_arg;
    }

protected:
    virtual bool characters(const ACEXML_Char *ch,
                            size_t start,
                            size_t length)
    {
        if (! util_XML_Element::characters(ch, start, length))
        {
            return false;
        }
        C converter(this);
        converter.content(ch, start, length, m_arg);
        return true;
    }

protected:
    T &m_arg;
};

template<class T>
class util_XML_ContentConv
{
public:
    util_XML_ContentConv(util_XML_Element *element = 0) :
        m_element(element)
    {
    }
    virtual ~util_XML_ContentConv()
    {
    }
    virtual void content(const ACEXML_Char *ch,
                         int start,
                         int length,
                         T &arg)
    {
    }
protected:
    util_XML_Element *m_element;
};

template<class T>
class util_XML_ContentConvNull :
    public util_XML_ContentConv<T>
{
public:
    util_XML_ContentConvNull(util_XML_Element *element = 0) :
        util_XML_ContentConv<T>(element)
    {
    }
    virtual void content(const ACEXML_Char *ch,
                         int start,
                         int length,
                         T &arg)
    {
    }
};

class util_XML_ContentConvUInt32 :
    public util_XML_ContentConv<ACE_UINT32>
{
public:
    util_XML_ContentConvUInt32(util_XML_Element *element = 0) :
        util_XML_ContentConv<ACE_UINT32>(element)
    {
    }
    void content(const ACEXML_Char *ch,
                 int start,
                 int length,
                 ACE_UINT32 &arg)
    {
        arg = atoi(ch);
    }
};

class util_XML_ContentConvInt32 :
    public util_XML_ContentConv<ACE_INT32>
{
public:
    util_XML_ContentConvInt32(util_XML_Element *element = 0) :
        util_XML_ContentConv<ACE_INT32>(element)
    {
    }
    void content(const ACEXML_Char *ch,
                 int start,
                 int length,
                 ACE_INT32 &arg)
    {
        arg = atoi(ch);
    }
};

class util_XML_ContentConvString :
    public util_XML_ContentConv<std::string>
{
public:
    util_XML_ContentConvString(util_XML_Element *element = 0) :
        util_XML_ContentConv<std::string>(element)
    {
    }
    void content(const ACEXML_Char *ch,
                 int start,
                 int length,
                 std::string &arg)
    {
        arg = ch;
    }
};

typedef util_XML_RegisteredElement<ACE_INT32, util_XML_ContentConvInt32> util_XML_Int32Element;
typedef util_XML_RegisteredElement<ACE_UINT32, util_XML_ContentConvUInt32> util_XML_UInt32Element;
typedef util_XML_RegisteredElement<std::string, util_XML_ContentConvString> util_XML_StringElement;

#endif // __UTIL_XML_SAX_PARSER_H__
