/************************************************************************************
文件名  : cpp_tcn_list_operator.h

文件描述:

作者    :zzt

创建日期:2006/05/09

修改记录: 原则上，用户类不能从节点类派生。只能作为节点类的数据成员
          因为链表的操作中有强制类型转换。这涉及到虚函数表指针的位置
          和基类对象数据在内存中的布局问题。所以应用不要从节点类派生
************************************************************************************/
#ifndef __CPP_TCN_LIST_OPERATOR_H_
#define __CPP_TCN_LIST_OPERATOR_H_

#include "cpp_adapter.h"

namespace tcn
{

#define GET_INDEX_BY_ADDR(minuend ,subtrahend ,size)  (((XS32)(minuend) - (XS32)(subtrahend))/(size))
#define GET_ELEMENT_AT(type,origin,offset,size)       *((type*)(((XS32)(origin))+(offset)*(size)))


/*********************************************************************
名称 : template<class TP> struct CSDNode
职责 : 单向链表节点,即可以是普通指针链表节点,也可以是索引链表节点
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
template<class TP> struct CSDNode
{
    typedef TP PointType;
    TP GetNext()
    {
        return m_pNext;
    }
    XVOID SetNext(TP pNode)
    {
        m_pNext = pNode;
    }

    TP m_pNext;
};

/*********************************************************************
名称 :
职责 : 双向链表节点,即可以是普通指针链表节点,也可以是索引链表节点
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
template<class TP> struct CBDNode:public CSDNode<TP>
{
    TP GetPrev()
    {
        return m_pPrev;
    }
    XVOID SetPrev(TP pPrevNode)
    {
        m_pPrev = pPrevNode;
    }

    TP  m_pPrev;
};

/*********************************************************************
名称 :
职责 : 只有头节点的指针链表
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
template<class CNT> struct H_Pt_List
{
    typedef CNT                      CNodeType;
    typedef typename CNT::PointType  TPoint;
    H_Pt_List()
    {
        m_pHead = GetNull();
    }
    TPoint GetHead()const
    {
        return m_pHead;
    }
    XVOID SetHead(TPoint tp)
    {
        m_pHead = tp;
    }

    CNodeType& GetNode(TPoint pt)const
    {
        //CPP_ASSERT(!IsNull(pt));
        return *(static_cast<CNodeType*>(pt));
    }
    TPoint  GetAddr(CNodeType& refNode)const
    {
        return &refNode;
    }
    bool IsNull(TPoint tp)const
    {
        return tp == GetNull();
    }
    static TPoint GetNull()
    {
        return XNULL;
    }
    TPoint GetEnd()
    {
        return GetNull();
    }
    bool AtEnd(TPoint tp)
    {
        return GetNull() == tp;
    }
    TPoint m_pHead;
};

/*********************************************************************
名称 :
职责 : 拥有头尾节点的指针链表
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
template<class CNT>
struct H_T_Pt_List:public H_Pt_List<CNT>
{
    typedef typename CNT::PointType TPoint;
    H_T_Pt_List()
    {
        m_pTail = H_Pt_List<CNT>::GetNull();
    }
    TPoint GetTail()const
    {
        return m_pTail;
    }
    XVOID SetTail(TPoint tp)
    {
        m_pTail =tp;
    }
    TPoint m_pTail;
};

typedef CSDNode<XVOID*> CSDBaseNode;
typedef CBDNode<XVOID*> CBDBaseNode;
//<SD,H,XNULL> pt_list
typedef H_Pt_List<CSDBaseNode > CSD_H_Pt_List;
//<SD,H,T>    pt_list
typedef H_T_Pt_List<CSDBaseNode > CSD_H_T_Pt_List;
//<BD,H,XNULL> pt_list
typedef H_Pt_List<CBDBaseNode > CBD_H_Pt_List;
//<BD,H,T   > pt_list
typedef H_T_Pt_List<CBDBaseNode> CBD_H_T_Pt_List;

/*********************************************************************
名称 :
职责 : 拥有头节点的索引链表
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
template<class TIndex,class CNT>
struct H_Idx_List
{
    typedef CNT      CNodeType;
    typedef TIndex  TPoint;
    //ndSize大小
    H_Idx_List(size_t ndSize,XVOID* pAddr,size_t ndCount)
    {
        m_NdSize  = ndSize;
        m_pBegin  = pAddr;
        m_NdCount = ndCount;
        m_pHead   = GetNull();
    }
    TPoint GetHead()const
    {
        return m_pHead;
    }
    XVOID SetHead(TPoint tp)
    {
        m_pHead = tp;
    }
    CNodeType& GetNode(TPoint pt)const
    {
        //CPP_ASSERT((pt>=0)&& !IsNull(pt));
        return GET_ELEMENT_AT(CNodeType,m_pBegin,pt,m_NdSize);
    }
    TPoint  GetAddr(CNodeType& refNode)const
    {
        return GET_INDEX_BY_ADDR(&refNode,m_pBegin,m_NdSize);
    }
    bool IsNull(TPoint tp)const
    {
        return tp >= GetNull();
    }
    bool Empty()const
    {
        return IsNull(GetHead());
    }

    TPoint GetNull()const
    {
        return m_NdCount;
    }
    TPoint GetEnd()
    {
        return GetNull();
    }
    bool AtEnd(TPoint tp)
    {
        return GetNull() <= tp;
    }
    size_t  m_NdSize;
    TPoint  m_pHead;
    XVOID*   m_pBegin;
    size_t  m_NdCount;

};

/*********************************************************************
名称 :
职责 : 拥有头尾节点的索引链表
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
template<class TIndex,class CNT>
struct H_T_Idx_List:public H_Idx_List<TIndex,CNT>
{
    typedef TIndex  TPoint;

    H_T_Idx_List(size_t ndSize,XVOID* pAddr,size_t ndCount)
        :H_Idx_List<TIndex,CNT>(ndSize,pAddr,ndCount)
    {
        SetTail(H_Idx_List<TIndex,CNT>::GetNull());
    }
    TPoint GetTail()const
    {
        return m_pTail;
    }
    XVOID SetTail(TPoint tp)
    {
        m_pTail = tp;
    }

    TPoint m_pTail;
};

//<SD,H,XNULL> Idx_list
template<class TIndex>
struct CSD_H_Idx_List:public H_Idx_List<TIndex,CSDNode<TIndex> >
{
    CSD_H_Idx_List( size_t ndSize,XVOID* pAddr,size_t ndCount)
        :H_Idx_List<TIndex,CSDNode<TIndex> >(ndSize,pAddr,ndCount)
    {
    }
};

//<SD,H,T> Idx_list
template<class TIndex>
struct CSD_H_T_Idx_List:public H_T_Idx_List<TIndex,CSDNode<TIndex> >
{
    CSD_H_T_Idx_List(size_t ndSize,XVOID* pAddr,size_t ndCount)
        :H_T_Idx_List<TIndex,CSDNode<TIndex> >(ndSize,pAddr,ndCount)
    {
    }
};

//<BD,H,XNULL> Idx_list
template<class TIndex>
struct CBD_H_Idx_List:public H_Idx_List<TIndex,CBDNode<TIndex> >
{
    CBD_H_Idx_List(size_t ndSize,XVOID* pAddr,size_t ndCount)
        :H_Idx_List<TIndex,CBDNode<TIndex> >(ndSize,pAddr,ndCount)
    {
    }

};

//<BD,H,T> Idx_list
template<class TIndex>
struct CBD_H_T_Idx_List:public H_T_Idx_List<TIndex,CBDNode<TIndex> >
{
    CBD_H_T_Idx_List(size_t ndSize,XVOID* pAddr,size_t ndCount)
        :H_T_Idx_List<TIndex,CBDNode<TIndex> >(ndSize,pAddr,ndCount)
    {
    }
};

/*链表操作*/
//<SD,H,XNULL> add before head
//<SD,H,XNULL> delete before tail
template<class TNode,class TList>
XVOID SD_H_AddBeforeHead(TNode& refNode,TList& refList)
{
    refNode.SetNext(refList.GetHead());
    refList.SetHead(refList.GetAddr(refNode));
    return;
}

template<class TList>
typename TList::CNodeType& SD_H_MoveHead(TList& refList)
{
    typename TList::TPoint point = refList.GetHead();
    //CPP_ASSERT(!refList.IsNull(point));
    refList.SetHead(refList.GetNode(point).GetNext());
    return refList.GetNode(point);
}

//<SD,H,T> delete at (head),add at(head,end)
template<class TNode,class TList>
XVOID SD_H_T_AddBeforeHead(TNode& refNode,TList& refList)
{
    SD_H_AddBeforeHead(refNode,refList);
    //现将尾节点处理好
    if(refList.IsNull(refList.GetTail()))
    {
        refList.SetTail(refList.GetAddr(refNode));
    }
}

template<class TNode,class TList>
XVOID SD_H_T_AddBeforeEnd(TNode& refNode,TList& refList)
{
    refNode.SetNext(refList.GetNull());
    if( !refList.IsNull(refList.GetTail()) )
    {
        refList.GetNode(refList.GetTail()).SetNext(refList.GetAddr(refNode));
    }
    //设置尾指针
    refList.SetTail(refList.GetAddr(refNode));
    //处理头指针,若头指针为空的话
    if(refList.IsNull(refList.GetHead()))
    {
        refList.SetHead(refList.GetAddr(refNode));
    }
}

template<class TList>
typename TList::CNodeType& SD_H_T_MoveHead(TList& refList)
{
    typename TList::CNodeType& tmp = SD_H_MoveHead(refList);
    if(refList.IsNull(refList.GetHead()))
    {
        //删除仅剩下的一个节点
        refList.SetTail(refList.GetNull());
    }
    return tmp;
}

/*********************************************************************
函数名称 : AddListNode(TNode& refNode,TNode& refAfter)
功能描述 : 环型双向链表的添加操作,该链表有一个头节点，

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
//通过位置添加
template<class TNode,class TLac,class TList>
inline XVOID AddBCNode(TList& refList,TNode& refNode,const TLac lct)
{
    CPP_ASSERT_RN(!refList.IsNull(lct));
    CPP_ASSERT_RN(!refList.IsNull(refList.GetHead()));
    refNode.SetNext(lct);
    refNode.SetPrev(refList.GetNode(lct).GetPrev());
    refList.GetNode(refList.GetNode(lct).GetPrev()).SetNext(refList.GetAddr(refNode));
    refList.GetNode(lct).SetPrev(refList.GetAddr(refNode));
}

template<class TLac,class TList>
typename TList::CNodeType& MoveBCNode(TList& refList,TLac Lct)
{
    //删除的是头节点
    //CPP_ASSERT(refList.GetHead() != Lct)
    refList.GetNode(refList.GetNode(Lct).GetNext()).SetPrev(refList.GetNode(Lct).GetPrev());
    refList.GetNode(refList.GetNode(Lct).GetPrev()).SetNext(refList.GetNode(Lct).GetNext());
    return refList.GetNode(Lct);
}

//<BD,H,XNULL> add before location. location not includes end
/*********************************************************************
函数名称 : AddListNode(TNode& refNode,TLac& refLct,TList& refList)
功能描述 : <BD,H,XNULL> delete at  location ,add at location .
           location includes  head and end
           双向链表，但不是环状的。只有头指针
           只有头指针双向链表的添加操作, 将refNode添加到refLct之前
           函数前置条件: refLct != END

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class TNode,class TLac,class TList>
XVOID BD_H_AddBeforelocation(TNode& refNode,TLac Lct,TList& refList)
{
    //若在头节点之前插入
    if(refList.GetHead() ==Lct)
    {
        if(!refList.IsNull(refList.GetHead()))
        {
            refList.GetNode(refList.GetHead()).SetPrev(refList.GetAddr(refNode));
        }
        refNode.SetPrev(refList.GetNull());
        SD_H_AddBeforeHead(refNode,refList);
        return;
    }
    CPP_ASSERT_RN(refList.GetNull() != Lct);
    //在中间节点前插入
    AddBCNode(refList,refNode,Lct);
}

template<class TLac,class TList>
typename TList::CNodeType& BD_H_MoveAtlocation(TLac Lct,TList& refList)
{
    //不是END点
    //CPP_ASSERT(!refList.AtEnd(Lct));
    //链表必须不为空
    //CPP_ASSERT(!refList.IsNull(refList.GetHead()));

    //取得被删节点的前后指针
    TLac next  = refList.GetNode(Lct).GetNext();
    TLac prev  = refList.GetNode(Lct).GetPrev();

    //若删除的不是尾节点
    if(!refList.IsNull(next))
    {
        refList.GetNode(next).SetPrev(prev);
    }

    //若删除的是头节点
    if(refList.IsNull(prev))
    {
        refList.SetHead(next);
    }
    else//删除的不是头节点
    {
        refList.GetNode(prev).SetNext(next);
    }
    return refList.GetNode(Lct);
}

/*********************************************************************
函数名称 : AddListNode(TNode& refNode,TLac& refLct,TList& refList)
功能描述 : <BD,H,T > delete at  location ,add at location .
           location includes  head and end
           双向链表，但不是环状的。有头、尾指针
           只有头指针双向链表的添加操作, 将refNode
           添加到refLct之前

参数输入 : refList    struct TList  必须定义
           SetHead(),SetTail(lct)
           GetNext(lct),GetPrev(lct),GetNode(lct)
           GetNullLink(),IsNullLink()
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class TNode,class TLac,class TList>
XVOID BD_H_T_AddBeforelocation(TNode& refNode,TLac Lct,TList& refList)
{
    //在END点前插入!refList.IsNull(refList.GetHead())&
    if(refList.AtEnd(Lct))
    {
        //将被插入节点的prev域设置好
        refNode.SetPrev(refList.GetTail());
        SD_H_T_AddBeforeEnd(refNode,refList);

        return ;
    }
    //在中间节点插入.
    BD_H_AddBeforelocation(refNode,Lct,refList);
}

template<class TLac,class TList>
typename TList::CNodeType& BD_H_T_MoveLocation(TLac& Lct,TList& refList)
{
    //不是END点
    //CPP_ASSERT(!refList.AtEnd(Lct));
    //链表必须不为空
    //CPP_ASSERT(!refList.IsNull(refList.GetHead()) && !refList.IsNull(refList.GetTail()));
    //若删除的是尾节点,则将尾指针处理好.
    if(refList.GetTail() == Lct)
    {
        refList.SetTail(refList.GetNode(Lct).GetPrev());
    }
    //当作仅有头指针的双向链表处理
    return BD_H_MoveAtlocation(Lct,refList);
}

//双向循环索引链表
//<BD,H,T> Idx_list
template<class TIndex>
struct CBD_R_Idx_List:public H_Idx_List<TIndex,CBDNode<TIndex> >
{
    //pAddr索引链表的所占的起始内存。size 元素个数,headIdx头节点的索引
    //双向环型链表的头节点的索引
    CBD_R_Idx_List(size_t ndSize,XVOID* pAddr,size_t size,TIndex headIdx)
        :H_Idx_List<TIndex,CBDNode<TIndex> >(ndSize,pAddr,size)
    {
        //CPP_ASSERT(headIdx >= 0);
        //CPP_ASSERT(headIdx < size);
        //头节点的索引
        SetHead(headIdx);
        //将头节点指针指向自己
        GetNode(headIdx).SetNext(headIdx);
        GetNode(headIdx).SetPrev(headIdx);
    }
    //是否为空
    bool Empty()const
    {
        return H_Idx_List<TIndex,CBDNode<TIndex> >::GetHead().GetNext() == H_Idx_List<TIndex,CBDNode<TIndex> >::GetHead();
    }

};


//双向循环指针链表
struct BD_Pt_list
{
    typedef CBDBaseNode             CNodeType;
    typedef CBDBaseNode::PointType  TPoint;
    BD_Pt_list()
    {
        Init();
    }
    //注意，拷贝函数的实现很特殊。
    BD_Pt_list(const BD_Pt_list& other)
    {
        Init();
    }
    TPoint GetHead()
    {
        return &m_head;
    }
    static CNodeType& GetNode(TPoint pt)
    {
        //CPP_ASSERT(!IsNull(pt));
        return *(static_cast<CNodeType*>(pt));
    }
    static TPoint  GetAddr(CNodeType& refNode)
    {
        return &refNode;
    }
    static bool IsNull(XVOID* tp)
    {
        return tp == GetNull();
    }
    static XVOID* GetNull()
    {
        return XNULL;
    }


    //在头节点之前插入一个节点
    XVOID PushFront(CBDBaseNode& Node)
    {
        AddBCNode(*this,Node,m_head.GetNext());
    }
    //调用者
    static XVOID Move(CBDBaseNode& node)
    {
        CPP_ASSERT_RN((node.GetPrev() !=&node)&& (node.GetNext() !=&node));
        GetNode(node.GetNext()).SetPrev(node.GetPrev());
        GetNode(node.GetPrev()).SetNext(node.GetNext());

    }

    XVOID Init()
    {
        m_head.SetNext(&m_head);
        m_head.SetPrev(&m_head);
    }
    CNodeType   m_head;
private://禁止赋值
    BD_Pt_list& operator =(const BD_Pt_list& other);
};
}

#endif





