/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月8日
**************************************************************************/
#ifndef __DIAM_MALLOC_H__
#define __DIAM_MALLOC_H__

#include <util/diam_ace.h>
#include <util/diam_config.h>

typedef ACE_Malloc<ACE_LOCAL_MEMORY_POOL, ACE_SYNCH_MUTEX> CMalloc;
typedef ACE_Allocator_Adapter<CMalloc> CAllocator;

#define DIAM_MEMORY_MANAGER_NAME "Diam_Memory_Manager"

/**************************************************************************
类    名:
类 功 能: 内存管理器,提供基本的内存分配
时    间: 2012年5月8日
**************************************************************************/
class CMemoryManager : public CAllocator
{
    friend class ACE_Singleton<CMemoryManager, ACE_Recursive_Thread_Mutex>; /**< memory manager */

private:
    CMemoryManager() : CAllocator(DIAM_MEMORY_MANAGER_NAME) {}
};

typedef ACE_Singleton<CMemoryManager, ACE_Recursive_Thread_Mutex> MemoryManagerInstance;

//DIAMETER_SINGLETON_DECLARE(ACE_Singleton, CMemoryManager, ACE_Recursive_Thread_Mutex);



#endif /*__DIAM_MEMORYMANAGER_H__*/

