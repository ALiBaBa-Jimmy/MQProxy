/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��8��31��
**************************************************************************/
#ifndef __DIAM_SESSION_ATTRIBUTES_H__
#define __DIAM_SESSION_ATTRIBUTES_H__

#include <api/diam_define.h>
#include <api/diam_message.h>

class DiamJobData {};

/*
diameter �ỰID��ʽ:
<DiameterIdentity>;<high 32 bits>;<low 32 bits>[;<optional value>]
*/
class CDiamSessionCounter
{
public:
    CDiamSessionCounter(DiamUINT32 h = 0,  DiamUINT32 l = 0);
    DiamUINT32 &High();
    DiamUINT32 &Low();
    int operator=(CDiamSessionCounter &cntr);
    int operator==(CDiamSessionCounter &cntr);
    int operator<(CDiamSessionCounter &cntr);
    int operator++();

protected:
    //�ỰID��
    DiamUINT32 m_High;
    DiamUINT32 m_Low;
};

class DiameterSessionId : public CDiamSessionCounter
{
public:
    DiameterSessionId();
    virtual ~DiameterSessionId();
    bool operator==(DiameterSessionId &id);
    bool operator=(DiameterSessionId &id);
    std::string &DiameterId();
    std::string &OptionalValue();
    DiamRetCode Get(DiamMsg &msg);
    DiamRetCode Set(DiamMsg &msg);
    bool IsEmpty();
    void Reset();
    void Dump(char *buf = NULL);
    void Dump(std::string &dump);
private:
    std::string m_DiameterId; //session id ǰ��һ��ͨ�������������������
    std::string m_OptionalVal;
};

class CDiamSessionAttributes : public DiamJobData
{
public:
    DiamUINT32 &ApplicationId();
    DiameterSessionId &SessionId();
    //CScholarAttribute<std::string> &DestinationHost();
    //CScholarAttribute<std::string> &DestinationRealm();
    //CScholarAttribute<std::string> &Username();
    //���ûỰ,�Ự��Ϊ����
    void Reset();

private:
    DiameterSessionId m_SessionId;
    DiamUINT32 m_ApplicationId;
    //CScholarAttribute<std::string> m_DestinationHost;
    //CScholarAttribute<std::string> m_DestinationRealm;
    //CScholarAttribute<std::string> m_Username;
};

#endif

