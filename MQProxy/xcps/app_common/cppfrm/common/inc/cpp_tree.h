/****************************************************************************
文件名  :cpp_tree.h

文件描述:

作者    :yuxiao

创建日期:2006/2/21

修改记录:
*************************************************************************/
#ifndef __CPP_TREE_H_
#define __CPP_TREE_H_

#include"cpp_adapter.h"


#define INVALID_OBJ_ID      0

//返回: 0:  successful; -1: error
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
* 对象类
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
* 链表节点类
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

    //获取前一个指针
    CLinkNode *GetPrev() const;

    //获取下一个指针
    CLinkNode *GetNext();

    //插入一个节点
    EerrNo Insert(XU32 uiLinkId, CLinkNode *pObj);

    //追加一个节点
    EerrNo Append(XU32 uiLinkId, CLinkNode *pObj);

    //移除一个节点
    EerrNo Remove(XU32 uiLinkId);

};



/*************************************************
* 链表类
*************************************************/
class CLink : public CObj
{
protected:

    XU32 m_uiObjNum;      //链表中的对象数目
    CLinkNode *m_pHead;   //链表头指针
    CLinkNode *m_pTail;   //链表尾指针

public:
    CLink();

    virtual ~CLink();

    //获取链表中的对象数目
    XU32        GetNum() const;

    //获取头节点对象
    CLinkNode *GetHead() const;

    //获取尾节点对象
    CLinkNode *GetTail() const;

    //检查是否在链表中
    bool IsInLink(CLinkNode *pObj);
    CLinkNode *Search(XU32 uiId);

    //添加到链表头部
    EerrNo AddToHead(CLinkNode *pObj);

    //添加到链表尾部
    EerrNo AddToTail(CLinkNode *pObj);

    //移除一个节点
    EerrNo Remove(CLinkNode *pObj);

    //移除头节点
    CLinkNode *RemoveHead();

    //移除尾节点
    CLinkNode *RemoveTail();

    //移除所有节点
    XVOID RemoveAll();

    virtual XVOID Print();
    XVOID PrintFromHead(XU32 cnt, bool bDetailFlag);
    XVOID PrintFromTail(XU32 cnt, bool bDetailFlag);
    XVOID PrintAll(bool bDetailFlag);
};




/*************************************************
* 资源节点类
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
* 资源类
*************************************************/
class CResource : public CObj
{
private:
    //在分配的时候给对象不同的ID
    XU32        m_uiRandom;
    //_randomBits决定随机BIT数目
    XU32        m_uiRandomBits;

protected:
    //对象池中最大对象数目
    XU32        m_uiMaxNum;
    //对象池中实际添加的对象数目
    XU32        m_uiRealNum;
    //峰值发生的时间
    XU32        m_uiPeakTimeId;
    //资源对象指针数组
    CResNode  **m_pObjBuf;
    //空闲链表
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
* 树节点类
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

    //获取分支
    CTreeNode *GetBranch(XU8 ucBranchOffset) const;

    //添加分支
    EerrNo AddBranch(XU8 ucBranchOffset, CTreeNode *pBranch);

    //删除分支
    EerrNo DelBranch(XU8 ucBranchOffset);

    virtual XVOID Print();
};




/*************************************************
* 树
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

    //查找记录
    ETreeResult SearchFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruidID);


    //查找记录，返回果实数组
    ETreeResult SearchMulFruit(XU8 ucBranchListCnt, XU8 *pucBranchList,
                               XPOINT * pFruidID,XU8 &ucFruitLen);

    //添加记录
    EerrNo AddFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT uiFruidID);

    //移除记录
    EerrNo RemoveFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruidID);

    EerrNo CheckRepeat(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruidID);

    EerrNo RemoveAllFruit(CTreeNode *pTreeNode);

    EerrNo Clear();


    XVOID PrintTree();

    //获取所有记录节点
    XVOID GetAllFruit(XPOINT * puiFruit,XU32 & uiFruitLen);
    XVOID GetTreeFruit(CTreeNode *pNode, XU8 ucLevel,XPOINT * puiFruit,XU32 & uiFruitNum);
};

#endif




