/****************************************************************************
�ļ���  :cpp_tree.h

�ļ�����:

����    :yuxiao

��������:2006/2/21

�޸ļ�¼:
*************************************************************************/
#ifndef __CPP_TREE_H_
#define __CPP_TREE_H_

#include"cpp_adapter.h"


#define INVALID_OBJ_ID      0

//����: 0:  successful; -1: error
#define MAX_OBJ_IN_RES      0xFFFFFFFFuL
#define MAX_RANDOM_BITS     16

#define MAX_BRANCH_NUM      16
#define MAX_TREE_DEPTH      64


enum EResState
{
    RES_IDLE,
    RES_BUSY,
    RES_ERROR = -1
};


enum ETreeResult
{
    TREE_FIND,
    TREE_NOT_FOUND,
    TREE_NOT_COMPLETE,
    TREE_FAIL = -1
};



/*************************************************
* ������
*************************************************/
class CObj
{
protected:
    XU32    m_uiId;
    EerrNo  m_eErr;

public:
    CObj(XU32 id);

    virtual ~CObj();

    XU32    GetId() const;
    EerrNo SetId(XU32 id);

    EerrNo GetErr() const;
    XVOID   SetErr(EerrNo err);

    virtual XVOID Print();
};


/*************************************************
* ����ڵ���
*************************************************/
class CLinkNode:public CObj
{
protected:
    XU32 m_uiLinkId;
    CLinkNode *m_pPrev;
    CLinkNode *m_pNext;

private:
    XVOID Init();

public:
    CLinkNode(XU32 id);
    virtual ~CLinkNode();

    XU32  GetLinkId() const;

    //��ȡǰһ��ָ��
    CLinkNode *GetPrev() const;

    //��ȡ��һ��ָ��
    CLinkNode *GetNext();

    //����һ���ڵ�
    EerrNo Insert(XU32 uiLinkId, CLinkNode *pObj);

    //׷��һ���ڵ�
    EerrNo Append(XU32 uiLinkId, CLinkNode *pObj);

    //�Ƴ�һ���ڵ�
    EerrNo Remove(XU32 uiLinkId);

};



/*************************************************
* ������
*************************************************/
class CLink : public CObj
{
protected:

    XU32 m_uiObjNum;      //�����еĶ�����Ŀ
    CLinkNode *m_pHead;   //����ͷָ��
    CLinkNode *m_pTail;   //����βָ��

public:
    CLink();

    virtual ~CLink();

    //��ȡ�����еĶ�����Ŀ
    XU32        GetNum() const;

    //��ȡͷ�ڵ����
    CLinkNode *GetHead() const;

    //��ȡβ�ڵ����
    CLinkNode *GetTail() const;

    //����Ƿ���������
    bool IsInLink(CLinkNode *pObj);
    CLinkNode *Search(XU32 uiId);

    //��ӵ�����ͷ��
    EerrNo AddToHead(CLinkNode *pObj);

    //��ӵ�����β��
    EerrNo AddToTail(CLinkNode *pObj);

    //�Ƴ�һ���ڵ�
    EerrNo Remove(CLinkNode *pObj);

    //�Ƴ�ͷ�ڵ�
    CLinkNode *RemoveHead();

    //�Ƴ�β�ڵ�
    CLinkNode *RemoveTail();

    //�Ƴ����нڵ�
    XVOID RemoveAll();

    virtual XVOID Print();
    XVOID PrintFromHead(XU32 cnt, bool bDetailFlag);
    XVOID PrintFromTail(XU32 cnt, bool bDetailFlag);
    XVOID PrintAll(bool bDetailFlag);
};




/*************************************************
* ��Դ�ڵ���
*************************************************/
class CResource;
class CResNode : public CLinkNode
{
protected:
    CResource *m_pRes;

public:
    CResNode(CResource *pRes);

    ~CResNode();

    CResource *GetRes() const;

    virtual XVOID Print();
};


/*************************************************
* ��Դ��
*************************************************/
class CResource : public CObj
{
private:
    //�ڷ����ʱ�������ͬ��ID
    XU32        m_uiRandom;
    //_randomBits�������BIT��Ŀ
    XU32        m_uiRandomBits;

protected:
    //���������������Ŀ
    XU32        m_uiMaxNum;
    //�������ʵ����ӵĶ�����Ŀ
    XU32        m_uiRealNum;
    //��ֵ������ʱ��
    XU32        m_uiPeakTimeId;
    //��Դ����ָ������
    CResNode  **m_pObjBuf;
    //��������
    CLink      *m_pIdleLink;

private:
    EerrNo Init(XU32 uiMaxNum, bool bNeedRandom);

public:
    CResource(XU32 uiId, XU32 uiMaxNum, bool bNeedRandom = false);

    virtual ~CResource();

    XU32 GetMaxNum() const;
    XU32 GetRealNum() const;
    XU32 GetIdleNum() const;

    EerrNo Add(CResNode *pObj);

    CResNode *AllocNode();
    EerrNo FreeById(XU32 uiId);

    CResNode *GetById(XU32 uiId);
    EResState GetStateById(XU32 uiId);


    virtual bool IsPrintNode(CLinkNode *pObj);

    virtual XVOID Print();

};




/*************************************************
* ���ڵ���
*************************************************/
class CTreeNode  : public CResNode
{
protected:
    XPOINT      m_uiFruitId;
    XU8         m_ucBranchCnt;
    CTreeNode **m_pBranch;

public:
    CTreeNode(CResource *pRes, XU8 ucMaxBranchNum);
    virtual ~CTreeNode();

    bool IsLeaf();

    bool IsRich();

    XPOINT GetFruit() const;
    XVOID SetFruit(XPOINT uiFruitID);

    //��ȡ��֧
    CTreeNode *GetBranch(XU8 ucBranchOffset) const;

    //��ӷ�֧
    EerrNo AddBranch(XU8 ucBranchOffset, CTreeNode *pBranch);

    //ɾ����֧
    EerrNo DelBranch(XU8 ucBranchOffset);

    virtual XVOID Print();
};




/*************************************************
* ��
*************************************************/
class CTree : public CResource
{
private:

protected:
    XU8 m_ucMaxBranchNum;
    XU8 m_ucMaxTreeDepth;
    CTreeNode *m_pRoot;

private:
    EerrNo Init(XU8 ucMaxBranchNum, XU8 ucMaxTreeDepth);
    XVOID PrintNode(CTreeNode *pNode, XU8 ucLevel, XU8 ucBranchOffset);


public:
    CTree(XU32 uiId, XU32 uiMaxNode, XU8 ucMaxBranchNum, XU8 ucMaxTreeDepth);
    ~CTree();

    //���Ҽ�¼
    ETreeResult SearchFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruidID);


    //���Ҽ�¼�����ع�ʵ����
    ETreeResult SearchMulFruit(XU8 ucBranchListCnt, XU8 *pucBranchList,
                               XPOINT * pFruidID,XU8 &ucFruitLen);

    //��Ӽ�¼
    EerrNo AddFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT uiFruidID);

    //�Ƴ���¼
    EerrNo RemoveFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruidID);

    EerrNo CheckRepeat(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruidID);

    EerrNo RemoveAllFruit(CTreeNode *pTreeNode);

    EerrNo Clear();


    XVOID PrintTree();

    //��ȡ���м�¼�ڵ�
    XVOID GetAllFruit(XPOINT * puiFruit,XU32 & uiFruitLen);
    XVOID GetTreeFruit(CTreeNode *pNode, XU8 ucLevel,XPOINT * puiFruit,XU32 & uiFruitNum);
};

#endif




