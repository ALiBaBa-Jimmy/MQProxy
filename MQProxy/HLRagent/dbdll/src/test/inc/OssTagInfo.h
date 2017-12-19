#pragma once
//oss相关的tag信息
#include <string>
using std::string;
#include "publictype.h"



//参数的长度是可变的
#define  OSS_PARAM_LENGTH_UNKNOWN 0

//设置值的时候采用数据字典默认的长度
#define  OSS_PARAM_LENGTH_DEFAULT 0

class COssTagInfo
{
public:
	COssTagInfo(void);
	~COssTagInfo(void);

	COssTagInfo& operator=(const COssTagInfo& copy);


	// tag 值
	XS16 m_nTag;

	// 参数的长度
	XS16 m_nLength;
	
	// tag对应的值的类型
	XS16 m_iType;
	// tag的名称
	string m_strName;
};
