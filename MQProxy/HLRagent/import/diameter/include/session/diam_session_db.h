/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年6月5日
**************************************************************************/
#ifndef __DIAM_SESSION_DB_H__
#define __DIAM_SESSION_DB_H__

#include <deque>
#include <util/diam_ace.h>
#include <util/diam_rbtree.h>
#include <session/diam_session.h>
#include <session/diam_session_attrs.h>

/**************************************************************************
类    名: CDiamSessionEntity
类 功 能: 数据包裹类
时    间: 2012年8月31日
**************************************************************************/
class CDiamSessionEntity : public CDiamSessionCounter
{
public:
    CDiamSessionEntity(DiamJobData &data) : m_Data(data)
    {
    }
    DiamJobData &Data()
    {
        return m_Data;
    }
    int operator=(CDiamSessionEntity &cntr)
    {
        m_Data = cntr.Data();
        (CDiamSessionCounter&)(*this) = (CDiamSessionCounter&)cntr;
        return (true);
    }
private:
    DiamJobData &m_Data;
};

/**************************************************************************
类    名: CDiamSessionEntityList
类 功 能: 红黑树
时    间: 2012年8月31日
**************************************************************************/
class CDiamSessionEntityList : public std::list<CDiamSessionEntity*>
{
public:
    CDiamSessionEntityList();
    bool Get(DiameterSessionId &id);
    bool Add(DiameterSessionId &id, DiamJobData &data);
    bool Lookup(DiameterSessionId &id, DiamJobData *&data);
    bool Remove(DiameterSessionId &id);
    void Flush();

protected:
    typedef std::list<CDiamSessionEntity*>::iterator
    EntryIterator;

private:
    //记录list中
    CDiamSessionCounter m_LastKnownCounter;
};

class CDiamSessionEntityNode : public util_RbTreeData
{
public:
    CDiamSessionEntityNode(std::string &id, std::string &opt)
    {
        m_DiameterId = id;
        m_OptionalVal = opt;
    }
    std::string &DiameterId()
    {
        return m_DiameterId;
    }
    std::string &OptionalVal()
    {
        return m_OptionalVal;
    }
    CDiamSessionEntityList &EntityList()
    {
        return m_Entries;
    }
protected:
    int operator==(util_RbTreeData &cmp)
    {
        CDiamSessionEntityNode *id = (CDiamSessionEntityNode*)cmp.payload;
        if (m_DiameterId == id->DiameterId())
        {
            if (m_OptionalVal.length() > 0)
            {
                return (m_OptionalVal == id->OptionalVal()) ? true : false;
            }
            return true;
        }
        return false;
    }
    int operator<(util_RbTreeData &cmp)
    {
        CDiamSessionEntityNode *id = (CDiamSessionEntityNode*)cmp.payload;
        if (m_DiameterId < id->DiameterId())
        {
            if (m_OptionalVal.length() > 0)
            {
                return (m_OptionalVal < id->OptionalVal()) ?
                       true : false;
            }
            return true;
        }
        return false;
    }
    void clear(void *pload)
    {
        m_Entries.Flush();
    }
private:
    std::string m_DiameterId;
    std::string m_OptionalVal;
    CDiamSessionEntityList m_Entries;
};

class CDiamSessionDb : private util_RbTreeTree
{
public:
    bool GetSessionID(DiameterSessionId &id);
    bool Add(DiameterSessionId &id, DiamJobData &data);
    bool Lookup(DiameterSessionId &id, DiamJobData *&data);
    bool Remove(DiameterSessionId &id);

private:
    ACE_RW_Mutex m_rwMutex;
};

typedef ACE_Singleton<CDiamSessionDb, ACE_Recursive_Thread_Mutex> CDiam_SessionDb_S;
#define DiamSessionDB() (CDiam_SessionDb_S::instance())

#endif /* __DIAM_SESSION_DB_H__ */

