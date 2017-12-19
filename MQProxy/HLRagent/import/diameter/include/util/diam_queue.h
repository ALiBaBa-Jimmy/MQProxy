#ifndef __DIAM_QUEUE_H__
#define __DIAM_QUEUE_H__

#include <util/diam_ace.h>
#include <util/diam_log.h>
#include <util/diam_sharedptr.h>
#include <util/diam_utillock.h>
#include <api/diam_datatype.h>
#include <api/diam_message.h>
#include <parser/diam_msgbuffer.h>
#include <map>

template <class TYPE>
class DiamQueue
{
public:
    DiamQueue(const char* name): name(name), condition_(mutex_), size(0) 
    {
    }

    ~DiamQueue() {}

public:
    int Enqueue(TYPE msg)
    {
        DiamScopeLock<ACE_Thread_Mutex> guard(mutex_);
        dataQueue.push_back(msg);
        size++;
        condition_.signal();
        return size;
    }

    int Dequeue(TYPE & msg)
    {
        DiamScopeLock<ACE_Thread_Mutex> guard(mutex_);
        if (dataQueue.empty())
        {
            ACE_Time_Value tm(1);
            tm += ACE_OS::time();
            if (-1 == condition_.wait(&tm))
            {
                return 0;
            }
            if (dataQueue.empty())
            {
                return 0;
            }
        }
        msg = dataQueue.front();
        dataQueue.pop_front();
        size--;
        return 1;
    }

    DiamUINT32  Size()
    {
        DiamScopeLock<ACE_Thread_Mutex> guard(mutex_);
        return size;
    }
    DiamBool    Empty()
    {
        DiamScopeLock<ACE_Thread_Mutex> guard(mutex_);
        return Size() == 0 ? true : false;
    }
private:
    std::list<TYPE>  dataQueue;
    //–≈∫≈¡ø
    ACE_Condition<ACE_Thread_Mutex> condition_;
    ACE_Thread_Mutex   mutex_;
    size_t size;
    std::string name;
};

class DiamMsg;
class DiamMsgBlock;
typedef DiamQueue<SharedPtr<DiamMsg> > DiamRecvQueue;
typedef DiamQueue<DiamMsgBlock*>       DiamSendQueue;


template <class ARG>
class DiamIterAction
{
public:
    // return TRUE to delete entry in iteration
    // return FALSE to sustain entry
    virtual bool operator()(ARG&)=0;

protected:
    virtual ~DiamIterAction()
    {
    }
    DiamIterAction()
    {
    }
};

template <class ARG>
class DiamIterActionDelete :
    public DiamIterAction<ARG>
{
public:
    // return TRUE to delete entry in iteration
    // return FALSE to sustain entry
    virtual bool operator()(ARG&) {
        return true;
    }
    virtual ~DiamIterActionDelete() {
    }
};

template <class ARG>
class DiamIterActionNone :
    public DiamIterAction<ARG>
{
public:
    // return TRUE to delete entry in iteration
    // return FALSE to sustain entry
    virtual bool operator()(ARG&) {
        return false;
    }
    virtual ~DiamIterActionNone() {
    }
};

template <class INDEX, class DATA>
class DiamProtectedMap : private std::map<INDEX, DATA>
{
public:
    virtual ~DiamProtectedMap()
    {
    }
    virtual void Add(INDEX ndx, DATA data)
    {
        ACE_Write_Guard<ACE_RW_Mutex> guard(rw_lock_);
        std::map<INDEX, DATA>::insert(std::pair<INDEX, DATA>(ndx, data));
    }
    virtual bool Lookup(INDEX ndx, DATA &data)
    {
        ACE_Read_Guard<ACE_RW_Mutex> guard(rw_lock_);
        typename std::map<INDEX, DATA>::iterator i;
        i = std::map<INDEX, DATA>::find(ndx);
        if (i != std::map<INDEX, DATA>::end())
        {
            data = i->second;
            return (true);
        }
        return (false);
    }
    virtual bool Remove(INDEX ndx, DiamIterAction<DATA> &e)
    {
        ACE_Write_Guard<ACE_RW_Mutex> guard(rw_lock_);
        typename std::map<INDEX, DATA>::iterator i;
        i = std::map<INDEX, DATA>::find(ndx);
        if (i != std::map<INDEX, DATA>::end())
        {
            e(i->second);
            std::map<INDEX, DATA>::erase(i);
            return (true);
        }
        return (false);
    }
    virtual bool IsEmpty()
    {
        ACE_Read_Guard<ACE_RW_Mutex> guard(rw_lock_);
        return std::map<INDEX, DATA>::empty() ? true : false;
    }

    virtual void Iterate(DiamIterAction<DATA> &e)
    {
        ACE_Write_Guard<ACE_RW_Mutex> guard(rw_lock_);
        typename std::map<INDEX, DATA>::iterator i = std::map<INDEX, DATA>::begin();
        while (i != std::map<INDEX, DATA>::end())
        {
            if (e(i->second))
            {
                typename std::map<INDEX, DATA>::iterator h = i;
                i ++;
                std::map<INDEX, DATA>::erase(h);
                continue;
            }
            i ++;
        }
    }
    unsigned int Size()
    {
        return std::map<INDEX, DATA>::size();
    }

private:
    ACE_RW_Mutex  rw_lock_;
};
#endif
