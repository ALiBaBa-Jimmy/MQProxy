/************************************************************************************
�ļ���  :cpp_mutex.h

�ļ�����:

����    :zzt

��������:2006/4/28

�޸ļ�¼:
************************************************************************************/
#ifndef __CPP_MUTEX_H_
#define __CPP_MUTEX_H_

#include "cpp_adapter.h"


/*********************************************************************
���� : TMutexGuard
ְ�� : �ڱ����������Զ������ͽ����Ĺ���
Э��:  class TMutexHand:������ź����ľ��
       class FTake:     �ڻ����ź�����ִ�л�ȡ�����ķº���
       class FGive:     �ڻ����ź�����ִ���ͷŲ����ķº���
��ʷ :
       �޸���   ����          ����
**********************************************************************/
template<class TMutexHand,class FTake,class FGive> class TMutexGuard
{
public:
    TMutexGuard(TMutexHand& refMHand)
        :m_refMHand(refMHand),m_bOwner(false)
    {
        Take();
    }
    ~TMutexGuard()
    {
        Give();
    }
    XVOID Take()
    {
        FTake functionTake;//�Ƿ��Ѿ�����ȡ��,�����ǵݹ�����
        functionTake(m_refMHand);
        m_bOwner = true;
    }
    XVOID Give()
    {
        if(m_bOwner)
        {
            FGive functionGive;
            functionGive(m_refMHand);
            m_bOwner = false;
        }
    }

private:
    TMutexHand& m_refMHand;
    bool        m_bOwner;
};

/*********************************************************************
���� : CMutex
ְ�� : ��װCPSƽ̨�ṩ�Ļ�����
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CMutex
{
public:
    CMutex()
    {
        CPP_ASSERT_RN(XSUCC == XOS_MutexCreate(&m_xosHand));
    }
    ~CMutex()
    {
        CPP_ASSERT_RN(XSUCC == XOS_MutexDelete(&m_xosHand));
    }
    XVOID Take()
    {
        CPP_ASSERT_RN(XSUCC == XOS_MutexLock(&m_xosHand));
    }
    XVOID Give()
    {
        CPP_ASSERT_RN(XSUCC == XOS_MutexUnlock(&m_xosHand));
    }

private:
    t_XOSMUTEXID m_xosHand;
};

/*********************************************************************
���� :
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
struct CFunTake
{
    XVOID operator()(CMutex& refMt)
    {
        refMt.Take();
    }
};

/*********************************************************************
���� :
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
struct CFunGive
{
    XVOID operator()(CMutex& refMt)
    {
        refMt.Give();
    }
};

/*********************************************************************
���� :
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
typedef TMutexGuard<CMutex,CFunTake,CFunGive> CMutexGuard;



#if 0
/*********************************************************************
���� : CRWLock
ְ�� : ��д�����ɸ������������ݿ���ͬʱ��һ���߳�д�����ɶ���̶߳���
       ��ĳһ�߳���д���������������̶߳���д������ĳһ�߳��ڶ���������
       �������߳�д���������������̶߳�.

       �ڵ�CPU��ϵͳ�ϣ�������������ܲ��Ǻܴ�
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CRWLock
{
public:
    CRWLock()
    {
        m_uiRdCount = 0;//���̼߳�������
    }
    XVOID ReadTake()
    {
        CMutexGuard mtRd(m_mtRdCount);//��m_mtRdCount��������Ϊ��Ҫ����m_uiRdCount
        if(m_uiRdCount++ >0)//��ǰ�����߳��ڶ����Ͳ���Ҫ��ȡ������
        {
            return ;
        }
        //���߳��ǵ�һ�������̣߳�Ҫ�Ϲ�����
        m_mtCommon.Take();
    }

    XVOID ReadGive()
    {
        CMutexGuard mtRd(m_mtRdCount);//��m_mtRdCount��������Ϊ��Ҫ����m_uiRdCount
        if(--m_uiRdCount ==0)//��û���κ��߳��ڶ�
        {
            m_mtCommon.Give();//��������������д�߳̽���
        }

    }

    XVOID WriteTake()
    {
        m_mtCommon.Take();
    }

    XVOID WriteGive()
    {
        m_mtCommon.Give();
    }

private:
    XU32    m_uiRdCount;//������
    CMutex  m_mtRdCount;//����m_uiRdCount����
    CMutex  m_mtCommon;//���̺߳�д�̵߳Ĺ�����

};

/*********************************************************************
���� :
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
struct CFunReadTake
{
    XVOID operator()(CRWLock& refMt)
    {
        refMt.ReadTake();
    }
};

/*********************************************************************
���� :
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
struct CFunReadGive
{
    XVOID operator()(CRWLock& refMt)
    {
        refMt.ReadGive();
    }
};

/*********************************************************************
���� : CReadMutexGuard
ְ�� : �Զ�����
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
typedef TMutexGuard<CRWLock,CFunReadTake,CFunReadGive> CReadMutexGuard;


/*********************************************************************
���� :
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
struct CFunWriteTake
{
    XVOID operator()(CRWLock& refMt)
    {
        refMt.WriteTake();
    }
};

/*********************************************************************
���� :
ְ�� :
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
struct CFunWriteGive
{
    XVOID operator()(CRWLock& refMt)
    {
        refMt.WriteGive();
    }
};

/*********************************************************************
���� : CWriteMutexGuard
ְ�� :�Զ�д��
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
typedef TMutexGuard<CRWLock,CFunWriteTake,CFunWriteGive> CWriteMutexGuard;


/*********************************************************************
���� :
ְ�� : ����ȫ�ֶ����������ȫ�ֶ����������һ��ȫ�ֶ���Ĺ��캯���з���
Э�� :
       CMutex& GetAnLock()
       {
           static CMutex mx;
           return mx;
       }
��ʷ :
       �޸���   ����          ����
**********************************************************************/
// �����ʼ��ʱ��ȫ������ȫ�ֶ���˭�ȳ�ʼ��������
typedef CMutex& (*PFunStaticLock)();

template<PFunStaticLock x> class CInitSalfLock
{
public:
    CInitSalfLock()
    {
        x();
    }
    XVOID Take()
    {
        GetAnLock().Take();
    }
    XVOID Give()
    {
        GetAnLock().Give();
    }

};
#endif


#endif /*_FRM_MUTEX_H_ */


