/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月9日
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


