/****************************************************************************
�ļ�    :cpp_mm_hash.h

�ļ�����:VDB��VLR��HASH�����ģ��

����	:wh

��������:2006/03/10

�޸ļ�¼:

*************************************************************************/
#ifndef __SS_MM_HASH_H_
#define __SS_MM_HASH_H_

#include <functional>
#include "cpp_adapter.h"
#include "cpp_tcn_hashtable.h"
#include "cpp_tcn_map.h"

//using namespace std;
using namespace tcn;

const XU32 g_HashInitSize    = 200;        //hash��ĳ�ʼ��С


// HASH�����ݶ���
enum  ETblOptResult
{
    TBL_OPT_COMPLETED = 1,    //�����ɹ�
    REC_EXIST,                //��¼�Ѵ���
    REC_NOTEXIST              //��¼������
};



//HASH����ģ��
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

    //��ȡ��ļ�¼��Ŀ
    inline XU32 Size()
    {
        return m_HashTbl.size();
    };

    //ɾ����¼
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

    ETblOptResult Add(Hash_Node_Type & node);            //����һ����¼
    XVOID Add(Hash_Node_Type & node, bool tmp);
    ETblOptResult Get(Hash_Key_Type KeyWord, Hash_Node_Type &node); //�鿴һ����¼
    ETblOptResult Modify(Hash_Node_Type  &node);         //�޸�Ψһһ����¼���豣֤�ؼ���Ωһ
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
        return REC_EXIST;                               //��¼�Ѵ���
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

//�鿴һ����¼
template< class HashTbl, class Key, class Node, class HashIter, class Pair >
ETblOptResult mapuHash< HashTbl, Key, Node, HashIter, Pair >::Get(Hash_Key_Type KeyWord, Hash_Node_Type &node)
{
    HashIter tmpIte(XNULL,XNULL);
    m_ite = m_HashTbl.find(KeyWord);
    if(tmpIte == m_ite)
    {
        return REC_NOTEXIST;                          //��¼������
    }
    else
    {
        node = *m_ite;
        return TBL_OPT_COMPLETED;                     //���ҳɹ�
    }
}


//�޸�Ψһһ����¼���豣֤�ؼ���Ωһ
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
        return REC_NOTEXIST;                          //��¼������
    }
    else
    {
        //�����ڵ����ݵ�hash���У���Ϊ������ָ�����
        //����bit�������弴��
        XOS_MemMove(&(*m_ite), &node, sizeof(Hash_Node_Type));

        return TBL_OPT_COMPLETED;                     //�޸ĳɹ�
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


