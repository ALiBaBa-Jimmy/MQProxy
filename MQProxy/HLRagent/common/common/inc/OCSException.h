#pragma once

#include <stdarg.h>
#include <string.h>
#include <stdio.h>


class COCSException
{
public:
	COCSException();	
	virtual ~COCSException();
	COCSException(int code, const char* fmt, ...);
	COCSException(const char* fmt, ...);
	
public:
	virtual void GetError(int& code, char* msg, int len);
	virtual const char* GetErrMsg();
	
protected:
	int m_code;
	char m_msg[2048];
};

