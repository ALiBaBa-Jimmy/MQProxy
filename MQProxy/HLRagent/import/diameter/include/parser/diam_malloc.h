/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��8��
**************************************************************************/
#ifndef __DIAM_MALLOC_H__
#define __DIAM_MALLOC_H__

#include <util/diam_ace.h>
#include <util/diam_config.h>

typedef ACE_Malloc<ACE_LOCAL_MEMORY_POOL, ACE_SYNCH_MUTEX> CMalloc;
typedef ACE_Allocator_Adapter<CMalloc> CAllocator;

#define DIAM_MEMORY_MANAGER_NAME "Diam_Memory_Manager"

/**************************************************************************
��    ��:
�� �� ��: �ڴ������,�ṩ�������ڴ����
ʱ    ��: 2012��5��8��
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

