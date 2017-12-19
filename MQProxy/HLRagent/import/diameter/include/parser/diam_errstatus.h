#ifndef __DIAM_ERRORSTATUS_H__
#define __DIAM_ERRORSTATUS_H__

#include <string>
#include <parser/diam_dictobject.h>
#include <parser/diam_parser_dict.h>
#include <api/diam_resultcode.h>

class CErrorCode
{
public:
    CErrorCode(void)
    {
        type = DIAM_PARSE_ERROR_TYPE_NORMAL;
        code = DIAMETER_SUCCESS;
    };
    virtual ~CErrorCode()
    {
    }

    virtual void get(DIAM_PARSE_ERROR_TYPE &t, int &c)
    {
        t = type;
        c = code;
    }

    virtual void set(DIAM_PARSE_ERROR_TYPE t, int c)
    {
        type = t;
        code = c;
    }

protected:
    DIAM_PARSE_ERROR_TYPE type;
    int code;
};

class CDiamErrorCode : public CErrorCode
{
public:
    CDiamErrorCode(void)
    {
    };

    void get(DIAM_PARSE_ERROR_TYPE &type,
             int &code,
             std::string &avp)
    {
        CErrorCode::get(type, code);
        avp = this->avp;
    }

    void set(DIAM_PARSE_ERROR_TYPE type, int code, CDictObjectAvp* data);

    void get(DIAM_PARSE_ERROR_TYPE &t, int &c)
    {
        CErrorCode::get(t, c);
    }

    void set(DIAM_PARSE_ERROR_TYPE t, int c)
    {
        CErrorCode::set(t, c);
    }

    void get(int &type, int &code, std::string &avp)
    {
        get((DIAM_PARSE_ERROR_TYPE&)type, code, avp);
    }

    void get(int &t, int &c)
    {
        get((DIAM_PARSE_ERROR_TYPE&)t, c);
    }

private:
    std::string avp;   /**< errornous AVP */
};

#endif // __DIAM_ERRORSTATUS_H__

