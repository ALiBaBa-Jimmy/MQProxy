#ifndef __DIAM_EXCEPTION_H__
#define __DIAM_EXCEPTION_H__



class CDiamBaseException
{
public:
    typedef enum
    {
        ALLOC_FAILURE = 0,
        INVALID_ID_TYPE,
        MISSING_SESSION_ID,
        MISSING_ORIGIN_HOST,
        MISSING_ORIGIN_REALM,
        IO_FAILURE
    } BASE_ERROR_CODE;

public:
    CDiamBaseException(int code, std::string &desc) : m_Code(code), m_Description(desc)
    {
    }
    CDiamBaseException(int code, const char* desc) : m_Code(code), m_Description(desc)
    {
    }
    int &Code()
    {
        return m_Code;
    }
    std::string &Description()
    {
        return m_Description;
    }

private:
    int m_Code;
    std::string m_Description;
};

class CDiamXMLException : public CDiamBaseException
{
public:
    typedef enum
    {
        XML_NOTE_ERROR = 0,
        XML_NOTE_INVALID ,
        AUTH_APPID_ERROR ,
        ACCT_APPID_ERROR ,
        APPID_ERROR,
        ROUTE_ERROR,
        XML_OPEN_ERROR,
        PARSER_XML_ERROR
    } XML_ERROR_CODE;

public:
    CDiamXMLException(int code, std::string &desc) : CDiamBaseException(code, desc)
    {
    }
    CDiamXMLException(int code, const char* desc) : CDiamBaseException(code, desc)
    {
    }
};

class CDiamPeerFsmException : public CDiamBaseException
{
public:
    typedef enum
    {
        ALLOC_FAILURE = 0,
        INVALID_ID_TYPE,
    } FSM_ERROR_CODE;

public:
    CDiamPeerFsmException(int code, std::string &desc) : CDiamBaseException(code, desc)
    {
    }
    CDiamPeerFsmException(int code, const char* desc) : CDiamBaseException(code, desc)
    {
    }
};
#endif //__DIAM_EXCEPTION_H__
