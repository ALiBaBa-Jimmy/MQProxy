#ifndef __UTIL_EXCEPTION_H__
#define __UTIL_EXCEPTION_H__

#include <string>
#include <api/diam_datatype.h>

using namespace std;

class Exception
{
public:
    Exception();
    Exception(const std::string & file, const  INT32 & line);
    Exception(const std::string & file, const  INT32 & line,const Exception & otherException);
    Exception(const std::string & file, const  INT32 & line,const std::string message);
    Exception(const std::string & file, const  INT32 & line,const std::string message,const INT32 & code);
    virtual ~Exception();
    std::string getFile() const;
    int getLine() const;
    virtual std::string getPrototypeName() const;
    virtual std::string getMessage() const;
    virtual INT32 getCode() const;
protected:
    const std::string _file;
    const INT32 _line;
    const std::string _message;
    const INT32 _code;
};

#define UTIL_ASSERT(EXPRESSION)  do{\
	if(!(EXPRESSION)) \
	{ \
	throw Exception(__FILE__,__LINE__, "The expression("#EXPRESSION")execute error!");\
	} \
	}while(0)


#endif //__UTIL_EXCEPTION_H__

