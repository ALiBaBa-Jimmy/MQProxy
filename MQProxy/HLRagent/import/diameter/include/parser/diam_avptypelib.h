/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��: diamObjectFactroy.h
��    ��: ��ɽ������������������Ĵ���
ʱ    ��: 2012��5��8��
**************************************************************************/
#ifndef __DIAM_OBJECTFACTOR_H__
#define __DIAM_OBJECTFACTOR_H__

#include <parser/diam_datatype.h>
#include <parser/diam_avptype.h>
#include <parser/diam_parseravpvalue.h>
#include <util/diam_config.h>

/**************************************************************************
��    ��:
�� �� ��:
ʱ    ��: 2012��5��8��
**************************************************************************/
class CAvpTypeLib : public std::list<CAvpType*>
{
    friend class ACE_Singleton<CAvpTypeLib, ACE_Recursive_Thread_Mutex>; /**< type list */
public:
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    CAvpType* search(DiamUINT32 type) ;
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    CAvpType* search(const char* name);
    /**************************************************************************
    �� �� ��:
    ��������:
    ��    ��:
    �� �� ֵ:
    **************************************************************************/
    void add(CAvpType* avpType);

private:
    CAvpTypeLib(void);
    ~CAvpTypeLib(void);
    void registerDefaultTypes();

    ACE_Thread_Mutex mutex; /**< mutex protector */
};

/**************************************************************************
��    ��: AvpType����
�� �� ��:
ʱ    ��: 2012��5��8��
**************************************************************************/
typedef ACE_Singleton<CAvpTypeLib, ACE_Recursive_Thread_Mutex> AvpTypeInstance;

//DIAMETER_SINGLETON_DECLARE(ACE_Singleton, CAvpTypeLib, ACE_Recursive_Thread_Mutex);

#endif /**/

