/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: diamObjectFactroy.h
功    能: 完成解析器对象和容器对象的创建
时    间: 2012年5月8日
**************************************************************************/
#ifndef __DIAM_OBJECTFACTOR_H__
#define __DIAM_OBJECTFACTOR_H__

#include <parser/diam_datatype.h>
#include <parser/diam_avptype.h>
#include <parser/diam_parseravpvalue.h>
#include <util/diam_config.h>

/**************************************************************************
类    名:
类 功 能:
时    间: 2012年5月8日
**************************************************************************/
class CAvpTypeLib : public std::list<CAvpType*>
{
    friend class ACE_Singleton<CAvpTypeLib, ACE_Recursive_Thread_Mutex>; /**< type list */
public:
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    CAvpType* search(DiamUINT32 type) ;
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    CAvpType* search(const char* name);
    /**************************************************************************
    函 数 名:
    函数功能:
    参    数:
    返 回 值:
    **************************************************************************/
    void add(CAvpType* avpType);

private:
    CAvpTypeLib(void);
    ~CAvpTypeLib(void);
    void registerDefaultTypes();

    ACE_Thread_Mutex mutex; /**< mutex protector */
};

/**************************************************************************
类    名: AvpType单例
类 功 能:
时    间: 2012年5月8日
**************************************************************************/
typedef ACE_Singleton<CAvpTypeLib, ACE_Recursive_Thread_Mutex> AvpTypeInstance;

//DIAMETER_SINGLETON_DECLARE(ACE_Singleton, CAvpTypeLib, ACE_Recursive_Thread_Mutex);

#endif /**/

