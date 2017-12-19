#ifndef __DIAM_PARSERAVPVALUE_H__
#define __DIAM_PARSERAVPVALUE_H__

#include <util/diam_ace.h>
#include <api/diam_resultcode.h>
#include <parser/diam_parseravp.h>
#include <parser/diam_parserqavplist.h>
#include <parser/diam_dictlib.h>
#include <parser/diam_ctnmanager.h>

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_loops.hpp>
#include <boost/spirit/include/classic_for.hpp>
#include <boost/spirit/include/classic_grammar.hpp>
#include <boost/spirit/include/classic_rule.hpp>

using namespace boost::spirit;

extern ACE_Mutex AvpGrammarMutex_S;

class AnyParser : public DiamAvpValueParser
{
public:
    void parseRawToApp();
    void parseAppToRaw();
};

class Integer32Parser : public DiamAvpValueParser
{
    friend class ACE_Singleton<Integer32Parser, ACE_Recursive_Thread_Mutex>;
public:
    void parseRawToApp();
    void parseAppToRaw();
};


class Integer64Parser : public DiamAvpValueParser
{
    friend class ACE_Singleton<Integer64Parser, ACE_Recursive_Thread_Mutex>;
public:
    void parseRawToApp();
    void parseAppToRaw();
};

class OctetStringParser : public DiamAvpValueParser
{
    friend class ACE_Singleton<OctetStringParser, ACE_Recursive_Thread_Mutex>;
public:
    void parseRawToApp();
    void parseAppToRaw();
};

class UTF8StringParser : public DiamAvpValueParser
{
    friend class ACE_Singleton<UTF8StringParser, ACE_Recursive_Thread_Mutex>;
public:
    void parseRawToApp();
    void parseAppToRaw();
};

class AddressParser : public OctetStringParser
{
public:
    void parseRawToApp();
    void parseAppToRaw();
};

class DiamidentParser : public UTF8StringParser
{
public:

    void parseRawToApp();

    void parseAppToRaw();
};

class DiamURLParser : public UTF8StringParser
{
public:
    DiamURLParser();

    void parseRawToApp();

    void parseAppToRaw();
private:
    DiamURI uri_;
};

class IPFilterRuleParser : public UTF8StringParser
{
public:
    IPFilterRuleParser();
    void parseRawToApp();

    void parseAppToRaw();

private:
    DiamIPFilterRule ipfilter_rule_;
};

class GroupedParser : public DiamAvpValueParser
{
public:
    void parseRawToApp();

    void parseAppToRaw();
};

typedef Integer32Parser TimeParser;

class UTF8Checker
{
public:

    UTF8Checker() {}

    int operator()(const char *data, unsigned length, bool nullCheck=false);
};


#endif // __DIAM_PARSERAVPVALUE_H__

