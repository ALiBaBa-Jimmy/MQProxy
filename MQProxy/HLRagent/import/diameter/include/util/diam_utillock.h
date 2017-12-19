/***************************************************************
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  filename:       diam_utillock.h
**  description:
**  author:         panjian
**  data:           2014.07.16
***************************************************************
**                          history
***************************************************************
**   author          date              modification
**   panjian         2014.07.16        create
**************************************************************/
#ifndef __DIAM_UTILLOCK_H__
#define __DIAM_UTILLOCK_H__
#include <util/diam_ace.h>

template <class LOCK>
class DiamScopeLock
{
public:
    DiamScopeLock(LOCK &l) : lock(l)
    {
        lock.acquire();
    }
    ~DiamScopeLock()
    {
        lock.release();
    }
    LOCK &Lock()
    {
        return lock;
    }

private:
    LOCK &lock;
};

typedef DiamScopeLock<ACE_Mutex> MutexScopeLock;

typedef DiamScopeLock<ACE_Thread_Mutex> ThreadMutexScopeLock;

#endif
