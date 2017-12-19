#include "OssTagInfo.h"

COssTagInfo::COssTagInfo(void)
: m_nTag(0)
, m_nLength(0)
, m_iType(0)
{
}

COssTagInfo::~COssTagInfo(void)
{
	
}


COssTagInfo& COssTagInfo::operator=(const COssTagInfo& copy)
{
	if(&copy != this)
	{
		m_iType = copy.m_iType;
		m_nLength = copy.m_nLength;
		m_nTag = copy.m_nTag;
		m_strName = copy.m_strName;
	}
	return *this;
}
