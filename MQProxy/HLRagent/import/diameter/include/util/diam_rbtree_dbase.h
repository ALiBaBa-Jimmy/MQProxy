/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年6月1日
**************************************************************************/
#ifndef __OD_UTL_RBTREE_DBASE_H__
#define __OD_UTL_RBTREE_DBASE_H__

#include <util/diam_rbtree.h>

template<class TYPE, class ARG>
class OD_Utl_DbaseTree : public util_RbTreeTree
{
public:
    typedef enum {
        ENTRY_ALLOC_FAILURE,
        ENTRY_INSERTION_FAILURE,
        ENTRY_NOT_FOUND
    } DB_ERROR;

protected:
    class OD_Utl_DbEntry : public util_RbTreeData {
    public:
        OD_Utl_DbEntry(TYPE &index) : m_Index(index) {
        }
        int operator<(util_RbTreeData &cmp) {
            OD_Utl_DbEntry &compare = reinterpret_cast<OD_Utl_DbEntry&>(cmp);
            return (m_Index < compare.Index());
        }
        int operator==(util_RbTreeData &cmp) {
            OD_Utl_DbEntry &compare = reinterpret_cast<OD_Utl_DbEntry&>(cmp);
            return (m_Index == compare.Index());
        }
        void clear(void *userData = 0) {
        }
        TYPE &Index() {
            return m_Index;
        }
    protected:
        TYPE m_Index;
    };

public:
    OD_Utl_DbaseTree() {
    }
    virtual ~OD_Utl_DbaseTree() {
    }
    void Add(TYPE &index, ARG &arg) {
        OD_Utl_DbEntry *newEntry = new OD_Utl_DbEntry(index);
        if (newEntry) {
            newEntry->payload = reinterpret_cast<void*>(&arg);
            if (Insert(newEntry)) {
                return;
            }
            throw (ENTRY_INSERTION_FAILURE);
        }
        throw (ENTRY_ALLOC_FAILURE);
    }
    void Remove(TYPE &index, ARG **arg = NULL) {
        OD_Utl_DbEntry lookupEntry(index), *searchEntry;
        searchEntry = static_cast<OD_Utl_DbEntry*>
                      (util_RbTreeTree::Remove(&lookupEntry));
        if (searchEntry) {
            if (arg) {
                *arg = reinterpret_cast<ARG*>(searchEntry->payload);
            }
            return;
        }
        throw (ENTRY_NOT_FOUND);
    }
    ARG *Search(TYPE &index) {
        OD_Utl_DbEntry lookupEntry(index), *searchEntry;
        searchEntry = static_cast<OD_Utl_DbEntry*>
                      (util_RbTreeTree::Find(&lookupEntry));
        if (searchEntry) {
            return reinterpret_cast<ARG*>(searchEntry->payload);
        }
        throw (ENTRY_NOT_FOUND);
    }
};

#endif // __OD_UTL_RBTREE_DBASE_H__
