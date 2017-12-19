/************************************************************************************
�ļ���  : cpp_tcn_list_operator.h

�ļ�����:

����    :zzt

��������:2006/05/09

�޸ļ�¼: ԭ���ϣ��û��಻�ܴӽڵ���������ֻ����Ϊ�ڵ�������ݳ�Ա
          ��Ϊ����Ĳ�������ǿ������ת�������漰���麯����ָ���λ��
          �ͻ�������������ڴ��еĲ������⡣����Ӧ�ò�Ҫ�ӽڵ�������
************************************************************************************/
#ifndef __CPP_TCN_LIST_OPERATOR_H_
#define __CPP_TCN_LIST_OPERATOR_H_

#include "cpp_adapter.h"

namespace tcn
{

#define GET_INDEX_BY_ADDR(minuend ,subtrahend ,size)  (((XS32)(minuend) - (XS32)(subtrahend))/(size))
#define GET_ELEMENT_AT(type,origin,offset,size)       *((type*)(((XS32)(origin))+(offset)*(size)))


/*********************************************************************
���� : template<class TP> struct CSDNode
ְ�� : ��������ڵ�,����������ָͨ������ڵ�,Ҳ��������������ڵ�
Э�� :
��ʷ :
       �޸���   ����          ����
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
���� :
ְ�� : ˫������ڵ�,����������ָͨ������ڵ�,Ҳ��������������ڵ�
Э�� :
��ʷ :
       �޸���   ����          ����
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
���� :
ְ�� : ֻ��ͷ�ڵ��ָ������
Э�� :
��ʷ :
       �޸���   ����          ����
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
���� :
ְ�� : ӵ��ͷβ�ڵ��ָ������
Э�� :
��ʷ :
       �޸���   ����          ����
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
���� :
ְ�� : ӵ��ͷ�ڵ����������
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
template<class TIndex,class CNT>
struct H_Idx_List
{
    typedef CNT      CNodeType;
    typedef TIndex  TPoint;
    //ndSize��С
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
���� :
ְ�� : ӵ��ͷβ�ڵ����������
Э�� :
��ʷ :
       �޸���   ����          ����
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

/*�������*/
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
    //�ֽ�β�ڵ㴦���
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
    //����βָ��
    refList.SetTail(refList.GetAddr(refNode));
    //����ͷָ��,��ͷָ��Ϊ�յĻ�
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
        //ɾ����ʣ�µ�һ���ڵ�
        refList.SetTail(refList.GetNull());
    }
    return tmp;
}

/*********************************************************************
�������� : AddListNode(TNode& refNode,TNode& refAfter)
�������� : ����˫���������Ӳ���,��������һ��ͷ�ڵ㣬

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
//ͨ��λ�����
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
    //ɾ������ͷ�ڵ�
    //CPP_ASSERT(refList.GetHead() != Lct)
    refList.GetNode(refList.GetNode(Lct).GetNext()).SetPrev(refList.GetNode(Lct).GetPrev());
    refList.GetNode(refList.GetNode(Lct).GetPrev()).SetNext(refList.GetNode(Lct).GetNext());
    return refList.GetNode(Lct);
}

//<BD,H,XNULL> add before location. location not includes end
/*********************************************************************
�������� : AddListNode(TNode& refNode,TLac& refLct,TList& refList)
�������� : <BD,H,XNULL> delete at  location ,add at location .
           location includes  head and end
           ˫�����������ǻ�״�ġ�ֻ��ͷָ��
           ֻ��ͷָ��˫���������Ӳ���, ��refNode��ӵ�refLct֮ǰ
           ����ǰ������: refLct != END

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class TNode,class TLac,class TList>
XVOID BD_H_AddBeforelocation(TNode& refNode,TLac Lct,TList& refList)
{
    //����ͷ�ڵ�֮ǰ����
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
    //���м�ڵ�ǰ����
    AddBCNode(refList,refNode,Lct);
}

template<class TLac,class TList>
typename TList::CNodeType& BD_H_MoveAtlocation(TLac Lct,TList& refList)
{
    //����END��
    //CPP_ASSERT(!refList.AtEnd(Lct));
    //������벻Ϊ��
    //CPP_ASSERT(!refList.IsNull(refList.GetHead()));

    //ȡ�ñ�ɾ�ڵ��ǰ��ָ��
    TLac next  = refList.GetNode(Lct).GetNext();
    TLac prev  = refList.GetNode(Lct).GetPrev();

    //��ɾ���Ĳ���β�ڵ�
    if(!refList.IsNull(next))
    {
        refList.GetNode(next).SetPrev(prev);
    }

    //��ɾ������ͷ�ڵ�
    if(refList.IsNull(prev))
    {
        refList.SetHead(next);
    }
    else//ɾ���Ĳ���ͷ�ڵ�
    {
        refList.GetNode(prev).SetNext(next);
    }
    return refList.GetNode(Lct);
}

/*********************************************************************
�������� : AddListNode(TNode& refNode,TLac& refLct,TList& refList)
�������� : <BD,H,T > delete at  location ,add at location .
           location includes  head and end
           ˫�����������ǻ�״�ġ���ͷ��βָ��
           ֻ��ͷָ��˫���������Ӳ���, ��refNode
           ��ӵ�refLct֮ǰ

�������� : refList    struct TList  ���붨��
           SetHead(),SetTail(lct)
           GetNext(lct),GetPrev(lct),GetNode(lct)
           GetNullLink(),IsNullLink()
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class TNode,class TLac,class TList>
XVOID BD_H_T_AddBeforelocation(TNode& refNode,TLac Lct,TList& refList)
{
    //��END��ǰ����!refList.IsNull(refList.GetHead())&
    if(refList.AtEnd(Lct))
    {
        //��������ڵ��prev�����ú�
        refNode.SetPrev(refList.GetTail());
        SD_H_T_AddBeforeEnd(refNode,refList);

        return ;
    }
    //���м�ڵ����.
    BD_H_AddBeforelocation(refNode,Lct,refList);
}

template<class TLac,class TList>
typename TList::CNodeType& BD_H_T_MoveLocation(TLac& Lct,TList& refList)
{
    //����END��
    //CPP_ASSERT(!refList.AtEnd(Lct));
    //������벻Ϊ��
    //CPP_ASSERT(!refList.IsNull(refList.GetHead()) && !refList.IsNull(refList.GetTail()));
    //��ɾ������β�ڵ�,��βָ�봦���.
    if(refList.GetTail() == Lct)
    {
        refList.SetTail(refList.GetNode(Lct).GetPrev());
    }
    //��������ͷָ���˫��������
    return BD_H_MoveAtlocation(Lct,refList);
}

//˫��ѭ����������
//<BD,H,T> Idx_list
template<class TIndex>
struct CBD_R_Idx_List:public H_Idx_List<TIndex,CBDNode<TIndex> >
{
    //pAddr�����������ռ����ʼ�ڴ档size Ԫ�ظ���,headIdxͷ�ڵ������
    //˫���������ͷ�ڵ������
    CBD_R_Idx_List(size_t ndSize,XVOID* pAddr,size_t size,TIndex headIdx)
        :H_Idx_List<TIndex,CBDNode<TIndex> >(ndSize,pAddr,size)
    {
        //CPP_ASSERT(headIdx >= 0);
        //CPP_ASSERT(headIdx < size);
        //ͷ�ڵ������
        SetHead(headIdx);
        //��ͷ�ڵ�ָ��ָ���Լ�
        GetNode(headIdx).SetNext(headIdx);
        GetNode(headIdx).SetPrev(headIdx);
    }
    //�Ƿ�Ϊ��
    bool Empty()const
    {
        return H_Idx_List<TIndex,CBDNode<TIndex> >::GetHead().GetNext() == H_Idx_List<TIndex,CBDNode<TIndex> >::GetHead();
    }

};


//˫��ѭ��ָ������
struct BD_Pt_list
{
    typedef CBDBaseNode             CNodeType;
    typedef CBDBaseNode::PointType  TPoint;
    BD_Pt_list()
    {
        Init();
    }
    //ע�⣬����������ʵ�ֺ����⡣
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


    //��ͷ�ڵ�֮ǰ����һ���ڵ�
    XVOID PushFront(CBDBaseNode& Node)
    {
        AddBCNode(*this,Node,m_head.GetNext());
    }
    //������
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
private://��ֹ��ֵ
    BD_Pt_list& operator =(const BD_Pt_list& other);
};
}

#endif





