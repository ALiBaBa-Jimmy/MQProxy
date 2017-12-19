/************************************************************************************
文件名  :cpp_mutex.h

文件描述:

作者    :zzt

创建日期:2006/4/28

修改记录:
************************************************************************************/
#ifndef __CPP_MUTEX_H_
#define __CPP_MUTEX_H_

#include "cpp_adapter.h"


/*********************************************************************
名称 : TMutexGuard
职责 : 哨兵锁。具有自动上锁和解锁的功能
协作:  class TMutexHand:互斥的信号量的句柄
       class FTake:     在互斥信号量上执行获取操作的仿函数
       class FGive:     在互斥信号量上执行释放操作的仿函数
历史 :
       修改者   日期          描述
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
        FTake functionTake;//是否已经被获取了,不考虑递归的情况
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
名称 : CMutex
职责 : 封装CPS平台提供的互斥锁
协作 :
历史 :
       修改者   日期          描述
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
名称 :
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
struct CFunTake
{
    XVOID operator()(CMutex& refMt)
    {
        refMt.Take();
    }
};

/*********************************************************************
名称 :
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
struct CFunGive
{
    XVOID operator()(CMutex& refMt)
    {
        refMt.Give();
    }
};

/*********************************************************************
名称 :
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
typedef TMutexGuard<CMutex,CFunTake,CFunGive> CMutexGuard;



#if 0
/*********************************************************************
名称 : CRWLock
职责 : 读写锁。由该锁保护的数据可以同时由一个线程写，或由多个线程读。
       若某一线程在写，不允许其它的线程读或写；若由某一线程在读，不允许
       其他的线程写，但允许其他的线程读.

       在单CPU的系统上，该锁的意义可能不是很大
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CRWLock
{
public:
    CRWLock()
    {
        m_uiRdCount = 0;//读线程计数清零
    }
    XVOID ReadTake()
    {
        CMutexGuard mtRd(m_mtRdCount);//对m_mtRdCount上锁，因为需要操作m_uiRdCount
        if(m_uiRdCount++ >0)//当前已有线程在读，就不需要获取公共锁
        {
            return ;
        }
        //本线程是第一个读的线程，要上公共锁
        m_mtCommon.Take();
    }

    XVOID ReadGive()
    {
        CMutexGuard mtRd(m_mtRdCount);//对m_mtRdCount上锁，因为需要操作m_uiRdCount
        if(--m_uiRdCount ==0)//已没有任何线程在读
        {
            m_mtCommon.Give();//开公共锁，允许写线程进入
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
    XU32    m_uiRdCount;//读计数
    CMutex  m_mtRdCount;//保护m_uiRdCount变量
    CMutex  m_mtCommon;//读线程和写线程的公共锁

};

/*********************************************************************
名称 :
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
struct CFunReadTake
{
    XVOID operator()(CRWLock& refMt)
    {
        refMt.ReadTake();
    }
};

/*********************************************************************
名称 :
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
struct CFunReadGive
{
    XVOID operator()(CRWLock& refMt)
    {
        refMt.ReadGive();
    }
};

/*********************************************************************
名称 : CReadMutexGuard
职责 : 自动读锁
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
typedef TMutexGuard<CRWLock,CFunReadTake,CFunReadGive> CReadMutexGuard;


/*********************************************************************
名称 :
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
struct CFunWriteTake
{
    XVOID operator()(CRWLock& refMt)
    {
        refMt.WriteTake();
    }
};

/*********************************************************************
名称 :
职责 :
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
struct CFunWriteGive
{
    XVOID operator()(CRWLock& refMt)
    {
        refMt.WriteGive();
    }
};

/*********************************************************************
名称 : CWriteMutexGuard
职责 :自动写锁
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
typedef TMutexGuard<CRWLock,CFunWriteTake,CFunWriteGive> CWriteMutexGuard;


/*********************************************************************
名称 :
职责 : 保护全局对象的锁。该全局对象可能在另一个全局对象的构造函数中访问
协作 :
       CMutex& GetAnLock()
       {
           static CMutex mx;
           return mx;
       }
历史 :
       修改者   日期          描述
**********************************************************************/
// 解决初始化时，全局锁和全局对象谁先初始化的问题
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


