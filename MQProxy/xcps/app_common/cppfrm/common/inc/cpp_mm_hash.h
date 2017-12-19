/****************************************************************************
文件    :cpp_mm_hash.h

文件描述:VDB和VLR的HASH表操作模板

作者	:wh

创建日期:2006/03/10

修改记录:

*************************************************************************/
#ifndef __SS_MM_HASH_H_
#define __SS_MM_HASH_H_

#include <functional>
#include "cpp_adapter.h"
#include "cpp_tcn_hashtable.h"
#include "cpp_tcn_map.h"

//using namespace std;
using namespace tcn;

const XU32 g_HashInitSize    = 200;        //hash表的初始大小


// HASH表数据定义
enum  ETblOptResult
{
    TBL_OPT_COMPLETED = 1,    //操作成功
    REC_EXIST,                //记录已存在
    REC_NOTEXIST              //记录不存在
};



//HASH操作模板
template< class HashTbl, class Key, class Node, class HashIter, class Pair >
class mapuHash
{
public:
    typedef HashTbl    HashTbl_Type;
    typedef HashIter   Hash_Iter_Type;
    typedef Pair       Hash_Pair_Type;
    typedef Node       Hash_Node_Type;
    typedef Key        Hash_Key_Type;
    typedef typename   HashTbl_Type::E_K   Extractor;

public:
    mapuHash(size_t iBucketSize = g_HashInitSize, size_t PreNodeNum = g_HashInitSize)
    :m_HashTbl(iBucketSize, PreNodeNum, hash<XU32>(), equal_to<XU32>())
    {
        m_pos = m_HashTbl.begin();
    };
    ~mapuHash(){};

public:

    HashTbl &GetHashTbl(){return m_HashTbl;};

    //获取表的记录数目
    inline XU32 Size()
    {
        return m_HashTbl.size();
    };

    //删除记录
    /*XVOID Delete(XU32 uiUid, bool delAll = false)
    {
        if(delAll == true)
        {
            m_HashTbl.clear();
        }
        else
        {
            m_HashTbl.erase(uiUid);
        }
    };*/

    ETblOptResult Add(Hash_Node_Type & node);            //增加一条记录
    XVOID Add(Hash_Node_Type & node, bool tmp);
    ETblOptResult Get(Hash_Key_Type KeyWord, Hash_Node_Type &node); //查看一条记录
    ETblOptResult Modify(Hash_Node_Type  &node);         //修改唯一一条记录，需保证关键吗惟一
    XVOID  Delete(Hash_Key_Type KeyWord);
    Hash_Iter_Type &GetHashNode(){return m_pos;}
    XVOID SetHashNode(Hash_Iter_Type  node){ m_pos = node;}

private:
    HashTbl_Type      m_HashTbl;
    Hash_Iter_Type    m_ite;
    Hash_Pair_Type    m_pair;
    Hash_Iter_Type    m_pos;
};


template< class HashTbl, class Key, class Node, class HashIter, class Pair >
ETblOptResult mapuHash<HashTbl, Key, Node, HashIter, Pair>::Add(Hash_Node_Type& node)
{
    //Hash_Node_Type *pNode = &node;
    //XU32            uiKey = (XU32&)node;
    //Modified by guanjinlong 2006-09-05
    Hash_Key_Type KeyWord = Extractor()(node);
    m_ite = m_HashTbl.find(KeyWord);

    if (m_ite == m_HashTbl.insert_unique_noresize(node).first)
    {
        return REC_EXIST;                               //记录已存在
    }
    else
    {
        return TBL_OPT_COMPLETED;
    }
}

template< class HashTbl, class Key, class Node, class HashIter, class Pair >
XVOID mapuHash< HashTbl, Key, Node, HashIter, Pair >::Add(Hash_Node_Type & node, bool tmp)
{
    m_ite = m_HashTbl.insert_equal(node);

    (XVOID)tmp;
}

//查看一条记录
template< class HashTbl, class Key, class Node, class HashIter, class Pair >
ETblOptResult mapuHash< HashTbl, Key, Node, HashIter, Pair >::Get(Hash_Key_Type KeyWord, Hash_Node_Type &node)
{
    HashIter tmpIte(XNULL,XNULL);
    m_ite = m_HashTbl.find(KeyWord);
    if(tmpIte == m_ite)
    {
        return REC_NOTEXIST;                          //记录不存在
    }
    else
    {
        node = *m_ite;
        return TBL_OPT_COMPLETED;                     //查找成功
    }
}


//修改唯一一条记录，需保证关键吗惟一
template<class HashTbl, class Key, class Node, class HashIter, class Pair>
ETblOptResult mapuHash<HashTbl, Key, Node,HashIter, Pair>::Modify(Hash_Node_Type  &node)
{
    //XU32 uiKey = (XU32&)node;
    //Modified by guanjinlong 2006-09-05
    Hash_Key_Type KeyWord = Extractor()(node);

    HashIter tmpIte(XNULL,XNULL);
    m_ite = m_HashTbl.find(KeyWord);
    if(tmpIte == m_ite)
    {
        return REC_NOTEXIST;                          //记录不存在
    }
    else
    {
        //拷贝节点内容到hash表中，因为不存在指针因此
        //按照bit拷贝语义即可
        XOS_MemMove(&(*m_ite), &node, sizeof(Hash_Node_Type));

        return TBL_OPT_COMPLETED;                     //修改成功
    }
}


template< class HashTbl, class Key, class Node, class HashIter, class Pair >
XVOID mapuHash<HashTbl, Key, Node, HashIter, Pair>::Delete(Hash_Key_Type KeyWord)
{
    HashIter tmpIte(XNULL,XNULL);

    m_ite = m_HashTbl.find(KeyWord);

    if(tmpIte != m_ite)
    {
        if(m_ite == m_pos)
        {
            m_pos++;
        }
        m_HashTbl.erase(KeyWord);
    }
}


#endif  //__SS_MM_HASH_H_


