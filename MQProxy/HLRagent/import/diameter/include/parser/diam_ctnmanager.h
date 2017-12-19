/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��9��
**************************************************************************/
#ifndef __DIAM_CTNMANAGER_H__
#define __DIAM_CTNMANAGER_H__

#include <parser/diam_avpcontainer.h>
#include <util/diam_config.h>

class DIAMETER_EXPORT CAvpContainerEntryManager
{
public:
    CAvpContainerEntry *acquire(AvpDataType type);

    void release(CAvpContainerEntry* entry);
};

class DIAMETER_EXPORT CAvpContainerManager
{
public:
    CAvpContainer *acquire(const char* name);

    CAvpContainer* acquire(const DiamUINT32 avpCode);

    void release(CAvpContainer* entry);
};

#endif /*__DIAM_CTNMANAGER_H__*/


