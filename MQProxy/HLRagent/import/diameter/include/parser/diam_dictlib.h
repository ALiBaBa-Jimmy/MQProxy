/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��8��
**************************************************************************/
#ifndef __DIAM_DICTLIB_H__
#define __DIAM_DICTLIB_H__

#include <list>
#include <util/diam_ace.h>
#include <parser/diam_dictobject.h>
#include <parser/diam_errstatus.h>

#define AVP_HEADER_LEN(avp) (avp->avpCode == 0 ? 0 : (avp->flags & DIAM_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8))

DiamUINT32 getMinSize(CDictObjectAvp*);
/**************************************************************************
��    ��: CAvpObjectLib
�� �� ��: �����ֵ��ж��������AVP����
ʱ    ��: 2012��5��8��
**************************************************************************/
class CAvpObjectLib: public std::list<CDictObjectAvp*>
{
    friend class ACE_Singleton<CAvpObjectLib, ACE_Recursive_Thread_Mutex>;
public:
    /**************************************************************************
    �� �� ��: add
    ��������: //��ʵ�������AVP
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void add(CDictObjectAvp*);
    /**************************************************************************
    �� �� ��: search
    ��������: ��ʵ���в���AVP
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    CDictObjectAvp* search(const std::string& name);
    /**************************************************************************
    �� �� ��: search
    ��������: ��ʵ���в���AVP
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    CDictObjectAvp* search(DiamAVPCode code, DiamVendorId vendorid);
    /**************************************************************************
    �� �� ��: dump
    ��������: ������м��ص�AVP
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void dump();
private:
    //�������캯��
    CAvpObjectLib();
    //������������
    virtual ~CAvpObjectLib();
    //�߳���
    ACE_Thread_Mutex mutex;
};
//AVPLib��
typedef ACE_Singleton<CAvpObjectLib, ACE_Recursive_Thread_Mutex> CAvpLibInstance;

/**************************************************************************
��    ��: CCommandObjectLib
�� �� ��: �����ֵ��ж���������������
ʱ    ��: 2012��5��8��
**************************************************************************/
class CCommandObjectLib : public std::list<CDictObjectCommand*>
{
    friend class ACE_Singleton<CCommandObjectLib, ACE_Recursive_Thread_Mutex>;
public:
    /**************************************************************************
    �� �� ��: add
    ��������: ��ʵ��������������
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void add(CDictObjectCommand*);

    /**************************************************************************
    �� �� ��: search
    ��������: ��ʵ���в����������
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    CDictObjectCommand* search(DiamUINT32 appID, const char*name);

    /**************************************************************************
    �� �� ��: search
    ��������: ��ʵ���в����������
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    CDictObjectCommand* search(DiamUINT32 code, DiamUINT32 appId,	int request);

    /**************************************************************************
    �� �� ��: dump
    ��������: ������м��ص�������
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void dump();
private:
    //�������캯��
    CCommandObjectLib();
    //������������
    ~CCommandObjectLib();
    //�߳���
    ACE_Thread_Mutex mutex;
};

typedef ACE_Singleton<CCommandObjectLib, ACE_Recursive_Thread_Mutex> CCommandLibInstance;


/**************************************************************************
��    ��:CGroupAvpObjectLib
�� �� ��:�����ֵ������е�GroupAvp����
ʱ    ��: 2012��5��7��
**************************************************************************/
class CGroupAvpObjectLib : public std::list<CDictObjectGroupAvp*>
{
    friend class ACE_Singleton<CGroupAvpObjectLib, ACE_Recursive_Thread_Mutex>;
public:
    /**************************************************************************
    �� �� ��: add
    ��������: ��ʵ�������groupAvp����
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void add(CDictObjectGroupAvp*);
    /**************************************************************************
    �� �� ��: search
    ��������: ��ʵ���в���groupAvp����
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    CDictObjectGroupAvp* search(DiamUINT32 code, DiamUINT32 vendorId);

private:
    //�������캯��
    CGroupAvpObjectLib();
    //������������
    ~CGroupAvpObjectLib();
    //�߳���
    ACE_Thread_Mutex mutex;
};

typedef ACE_Singleton<CGroupAvpObjectLib, ACE_Recursive_Thread_Mutex> CGroupAvpLibInstance;

#endif



