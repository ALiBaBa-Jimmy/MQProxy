#pragma once
//oss��ص�tag��Ϣ
#include <string>
using std::string;
#include "publictype.h"



//�����ĳ����ǿɱ��
#define  OSS_PARAM_LENGTH_UNKNOWN 0

//����ֵ��ʱ����������ֵ�Ĭ�ϵĳ���
#define  OSS_PARAM_LENGTH_DEFAULT 0

class COssTagInfo
{
public:
	COssTagInfo(void);
	~COssTagInfo(void);

	COssTagInfo& operator=(const COssTagInfo& copy);


	// tag ֵ
	XS16 m_nTag;

	// �����ĳ���
	XS16 m_nLength;
	
	// tag��Ӧ��ֵ������
	XS16 m_iType;
	// tag������
	string m_strName;
};
