/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月8日
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
类    名: CAvpObjectLib
类 功 能: 管理字典中定义的所有AVP对象
时    间: 2012年5月8日
**************************************************************************/
class CAvpObjectLib: public std::list<CDictObjectAvp*>
{
    friend class ACE_Singleton<CAvpObjectLib, ACE_Recursive_Thread_Mutex>;
public:
    /**************************************************************************
    函 数 名: add
    函数功能: //向实例中添加AVP
    参    数:
    返 回 值:
    **************************************************************************/
    void add(CDictObjectAvp*);
    /**************************************************************************
    函 数 名: search
    函数功能: 在实例中查找AVP
    参    数:
    返 回 值:
    **************************************************************************/
    CDictObjectAvp* search(const std::string& name);
    /**************************************************************************
    函 数 名: search
    函数功能: 在实例中查找AVP
    参    数:
    返 回 值:
    **************************************************************************/
    CDictObjectAvp* search(DiamAVPCode code, DiamVendorId vendorid);
    /**************************************************************************
    函 数 名: dump
    函数功能: 输出所有加载的AVP
    参    数:
    返 回 值:
    **************************************************************************/
    void dump();
private:
    //单例构造函数
    CAvpObjectLib();
    //单例析构函数
    virtual ~CAvpObjectLib();
    //线程锁
    ACE_Thread_Mutex mutex;
};
//AVPLib库
typedef ACE_Singleton<CAvpObjectLib, ACE_Recursive_Thread_Mutex> CAvpLibInstance;

/**************************************************************************
类    名: CCommandObjectLib
类 功 能: 管理字典中定义的所有命令对象
时    间: 2012年5月8日
**************************************************************************/
class CCommandObjectLib : public std::list<CDictObjectCommand*>
{
    friend class ACE_Singleton<CCommandObjectLib, ACE_Recursive_Thread_Mutex>;
public:
    /**************************************************************************
    函 数 名: add
    函数功能: 向实例中添加命令对象
    参    数:
    返 回 值:
    **************************************************************************/
    void add(CDictObjectCommand*);

    /**************************************************************************
    函 数 名: search
    函数功能: 在实例中查找命令对象
    参    数:
    返 回 值:
    **************************************************************************/
    CDictObjectCommand* search(DiamUINT32 appID, const char*name);

    /**************************************************************************
    函 数 名: search
    函数功能: 在实例中查找命令对象
    参    数:
    返 回 值:
    **************************************************************************/
    CDictObjectCommand* search(DiamUINT32 code, DiamUINT32 appId,	int request);

    /**************************************************************************
    函 数 名: dump
    函数功能: 输出所有加载的命令码
    参    数:
    返 回 值:
    **************************************************************************/
    void dump();
private:
    //单例构造函数
    CCommandObjectLib();
    //单例析构函数
    ~CCommandObjectLib();
    //线程锁
    ACE_Thread_Mutex mutex;
};

typedef ACE_Singleton<CCommandObjectLib, ACE_Recursive_Thread_Mutex> CCommandLibInstance;


/**************************************************************************
类    名:CGroupAvpObjectLib
类 功 能:管理字典中所有的GroupAvp对象
时    间: 2012年5月7日
**************************************************************************/
class CGroupAvpObjectLib : public std::list<CDictObjectGroupAvp*>
{
    friend class ACE_Singleton<CGroupAvpObjectLib, ACE_Recursive_Thread_Mutex>;
public:
    /**************************************************************************
    函 数 名: add
    函数功能: 向实例中添加groupAvp对象
    参    数:
    返 回 值:
    **************************************************************************/
    void add(CDictObjectGroupAvp*);
    /**************************************************************************
    函 数 名: search
    函数功能: 在实例中查找groupAvp对象
    参    数:
    返 回 值:
    **************************************************************************/
    CDictObjectGroupAvp* search(DiamUINT32 code, DiamUINT32 vendorId);

private:
    //单例构造函数
    CGroupAvpObjectLib();
    //单例析构函数
    ~CGroupAvpObjectLib();
    //线程锁
    ACE_Thread_Mutex mutex;
};

typedef ACE_Singleton<CGroupAvpObjectLib, ACE_Recursive_Thread_Mutex> CGroupAvpLibInstance;

#endif



