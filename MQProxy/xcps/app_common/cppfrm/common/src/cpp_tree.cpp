/****************************************************************************
文件名	:cpp_tree.cpp

文件描述:

作者    :yuxiao

创建日期:2006/2/21

修改记录:
*************************************************************************/
#include "cpp_adapter.h"
#include "cpp_tree.h"


/*lint -e429 */
/*lint -e1551*/


#define RESOURCE_INDEX_BITS_MASK ((m_uiRandomBits) ? ((1<<(32 - m_uiRandomBits))-1) : 0xFFFFFFFF)

XU32 g_RandomBits = 0;

XU32 g_LinkId = 1;

/*********************************************************************
函数名称 : CObj
功能描述 :

参数输入 : XU32 uiId
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CObj::CObj(XU32 uiId)
{

    m_uiId = uiId;
    m_eErr = ERR_OK;
}


/*********************************************************************
函数名称 : ~CObj
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CObj::~CObj()
{
    m_uiId = INVALID_OBJ_ID;
}

//获取对象ID
inline XU32 CObj::GetId() const
{
    return m_uiId;
}

//获取错误值
inline EerrNo CObj::GetErr() const
{
    return m_eErr;
}



/*********************************************************************
函数名称 : SetId
功能描述 :

参数输入 : XU32  uiId
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CObj::SetId(XU32 uiId)
{
    CPP_ASSERT_RV(INVALID_OBJ_ID != uiId, ERR_INVALID_PARAM);

    m_uiId = uiId;

    return ERR_OK;
}

/*********************************************************************
函数名称 : SetErr
功能描述 :

参数输入 : EerrNo eErr
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CObj::SetErr(EerrNo eErr)
{
    m_eErr = eErr;
}


/*********************************************************************
函数名称 : Print
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CObj::Print()
{

}


/*********************************************************************
函数名称 : CLinkNode
功能描述 :

参数输入 : XU32 uiId
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLinkNode::CLinkNode(XU32 uiId) : CObj(uiId)
{
    Init();
}

/*********************************************************************
函数名称 : CLinkNode
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLinkNode::~CLinkNode()
{
    m_uiLinkId = INVALID_OBJ_ID;
}


/*********************************************************************
函数名称 : Init
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLinkNode::Init()
{
    m_uiLinkId = INVALID_OBJ_ID;
    m_pPrev    = XNULL;
    m_pNext    = XNULL;
}

//获取节点ID
inline XU32 CLinkNode::GetLinkId() const
{
    return m_uiLinkId;
}

inline CLinkNode* CLinkNode::GetPrev() const
{
    return m_pPrev;
}

inline CLinkNode* CLinkNode::GetNext()
{
    return m_pNext;
}



/*********************************************************************
函数名称 : Insert
功能描述 : 把当前对象放在pobj的前面

参数输入 : uiLinkId      XU32
           pObj          CLinkNode *
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLinkNode::Insert(XU32 uiLinkId, CLinkNode *pObj)
{
    CPP_ASSERT_RV(INVALID_OBJ_ID != uiLinkId, ERR_INVALID_PARAM);

    if(pObj == this)
    {
        //不能在自身前插入
        return ERR_INVALID_PARAM;
    }

    if((INVALID_OBJ_ID != m_uiLinkId) ||
            (XNULL != m_pPrev) ||
            (XNULL != m_pNext))
    {
        return ERR_INVALID_OPR;    //Before insert to link, it must be insulated
    }

    if(XNULL != pObj)
    {
        if(XNULL != pObj->m_pPrev)
        {
            pObj->m_pPrev->m_pNext = this;
            m_pPrev = pObj->m_pPrev;
        }

        m_pNext = pObj;
        pObj->m_pPrev = this;
    }

    m_uiLinkId = uiLinkId;

    return ERR_OK;
}

/*********************************************************************
函数名称 : Append
功能描述 : 把当前对象放在pobj后面

参数输入 : uiLinkId      XU32           pObj节点对象所在的链表的对象ID
           pObj          CLinkNode*
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLinkNode::Append(XU32 uiLinkId, CLinkNode *pObj)
{
    CPP_ASSERT_RV(INVALID_OBJ_ID != uiLinkId, ERR_INVALID_PARAM);

    if(pObj == this)
    {
        //不能添加自身
        return ERR_INVALID_PARAM;
    }

    if((INVALID_OBJ_ID != m_uiLinkId) ||
            (XNULL != m_pPrev) ||
            (XNULL != m_pNext))
    {
        return ERR_INVALID_OPR;    //Before append to link, it must be insulated
    }

    if(XNULL != pObj)
    {
        if(XNULL != pObj->m_pNext)
        {
            pObj->m_pNext->m_pPrev = this;
            m_pNext = pObj->m_pNext;
        }

        m_pPrev = pObj;
        pObj->m_pNext = this;
    }

    m_uiLinkId = uiLinkId;

    return ERR_OK;
}

/*********************************************************************
函数名称 : Remove
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLinkNode::Remove(XU32 uiLinkId)
{
    CPP_ASSERT_RV(INVALID_OBJ_ID != uiLinkId, ERR_INVALID_PARAM);

    if(m_uiLinkId != uiLinkId)
    {
        return ERR_INVALID_PARAM;    //it must be deleted by its owner
    }

    if(XNULL != m_pPrev)
    {
        m_pPrev->m_pNext = m_pNext;
    }

    if(XNULL != m_pNext)
    {
        m_pNext->m_pPrev = m_pPrev;
    }

    m_uiLinkId = INVALID_OBJ_ID;
    m_pPrev    = XNULL;
    m_pNext    = XNULL;

    return ERR_OK;
}


/*********************************************************************
函数名称 : CLink
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLink::CLink() : CObj(1)    //give a initial ID first
{

    //modify by panjian for 64
    m_uiId     = g_LinkId++;
    m_uiObjNum = 0;
    m_pHead    = XNULL;
    m_pTail    = XNULL;
}

/*********************************************************************
函数名称 : ~CLink
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLink::~CLink()
{
    (XVOID)RemoveAll();
    m_pHead = XNULL;
    m_pTail = XNULL;
    m_uiObjNum = 0;
}

inline XU32 CLink::GetNum() const
{
    return m_uiObjNum;
}

inline CLinkNode* CLink::GetHead() const
{
    return m_pHead;
}

inline CLinkNode* CLink::GetTail() const
{
    return m_pTail;
}

/*********************************************************************
函数名称 : IsInLink
功能描述 :

参数输入 : pObj     CLinkNode*      链表节点
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
bool CLink::IsInLink(CLinkNode *pObj)
{
    CPP_ASSERT_RV(XNULL != pObj,false);

    if(m_uiId != pObj->GetLinkId())
    {
        return false;
    }
    else
    {
        return true;
    }
}


/*********************************************************************
函数名称 : Search
功能描述 :

参数输入 : uiId      XU32
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLinkNode* CLink::Search(XU32 uiId)
{
    CPP_ASSERT_RV(XNULL != m_pHead,XNULL);
    CLinkNode *pSearch = m_pHead;
    XU32 i = 0;

    if(INVALID_OBJ_ID == uiId)
    {
        return XNULL;
    }

    for(i = 0; i < m_uiObjNum; ++i)
    {
        if(uiId == pSearch->GetId())
            return pSearch;

        pSearch = pSearch->GetNext();
        CPP_ASSERT_RV(XNULL != pSearch,XNULL);
    }

    return XNULL;
}

/*********************************************************************
函数名称 : AddToHead
功能描述 :

参数输入 : pObj      CLinkNode*       链表节点
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLink::AddToHead(CLinkNode *pObj)
{
    EerrNo result = ERR_OK;

    CPP_ASSERT_RV(XNULL != pObj, ERR_INVALID_PARAM);

    CPP_ASSERT_RV(INVALID_OBJ_ID != pObj->GetId(), ERR_INVALID_PARAM);        //Can't insert uninitial object

    result = pObj->Insert(m_uiId, m_pHead);
    if(ERR_OK != result)
        return result;

    ++m_uiObjNum;

    m_pHead = pObj;

    if(1 == m_uiObjNum)
    {
        m_pTail = pObj;
    }

    return ERR_OK;
}

/*********************************************************************
函数名称 : AddToTail
功能描述 :

参数输入 : pObj      CLinkNode*       链表节点
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLink::AddToTail(CLinkNode *pObj)
{
    EerrNo result = ERR_OK;

    CPP_ASSERT_RV(XNULL != pObj, ERR_INVALID_PARAM);

    CPP_ASSERT_RV(INVALID_OBJ_ID != pObj->GetId(), ERR_INVALID_PARAM);  //Can't insert uninitial object

    //把obj添加到m_pTail后面
    result = pObj->Append(m_uiId, m_pTail);
    if(ERR_OK != result)
        return result;

    ++m_uiObjNum;
    m_pTail = pObj;
    if(1 == m_uiObjNum)
    {
        m_pHead = pObj;
    }

    return ERR_OK;
}

/*********************************************************************
函数名称 : Remove
功能描述 :

参数输入 : pObj      CLinkNode*       链表节点
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLink::Remove(CLinkNode *pObj)
{
    EerrNo result = ERR_OK;
    CLinkNode *pTmp1 = XNULL;
    CLinkNode *pTmp2 = XNULL;

    CPP_ASSERT_RV(XNULL != pObj, ERR_INVALID_PARAM);

    if(0 == m_uiObjNum)
    {
        return ERR_INVALID_OPR;
    }

    pTmp1 = pObj->GetPrev();
    pTmp2 = pObj->GetNext();

    result = pObj->Remove(m_uiId);
    if(ERR_OK != result)
        return result;

    if(m_pHead == pObj)
    {
        m_pHead = pTmp2;
    }

    if(m_pTail == pObj)
    {
        m_pTail = pTmp1;
    }

    m_uiObjNum--;

    return ERR_OK;
}

/*********************************************************************
函数名称 : RemoveHead
功能描述 :

参数输入 : pObj      CLinkNode*       链表节点
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLinkNode* CLink::RemoveHead()
{
    if(XNULL == m_pHead)
    {
        return XNULL;
    }

    CLinkNode *pTmp1, *pTmp2;

    if(0 == m_uiObjNum)
    {
        return XNULL;
    }

    //如果对象数为0,m_pHead必须不为空
    //CPP_ASSERT(XNULL != m_pHead);

    pTmp1 = m_pHead;
    pTmp2 = m_pHead->GetNext();

    if(ERR_OK != m_pHead->Remove(m_uiId))
    {
        //060419加 yuxiao
        return XNULL;
    }

    --m_uiObjNum;
    m_pHead = pTmp2;

    if(0 == m_uiObjNum)
    {
        m_pTail = XNULL;
    }

    return pTmp1;
}

/*********************************************************************
函数名称 : RemoveTail
功能描述 :

参数输入 :
参数输出 :
返回值   : CLinkNode*

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLinkNode* CLink::RemoveTail()
{
    CLinkNode *pTmp1, *pTmp2;

    if((m_pTail == XNULL) || (0 == m_uiObjNum))
    {
        return XNULL;
    }

    //CPP_ASSERT(XNULL != m_pTail);    //if m_uiObjNum isn't 0, m_pTail must be not XNULL

    pTmp1 = m_pTail;
    pTmp2 = m_pTail->GetPrev();

    if(ERR_OK != m_pTail->Remove(m_uiId))
    {
        return XNULL;
    }

    m_uiObjNum--;
    m_pTail = pTmp2;

    if(0 == m_uiObjNum)
    {
        m_pHead = XNULL;
    }

    return pTmp1;
}

/*********************************************************************
函数名称 : RemoveAll
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::RemoveAll()
{
    while(m_uiObjNum)
    {
        if (RemoveHead() == XNULL)
        {
            break;
        }
    }
}

/*********************************************************************
函数名称 : Print
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::Print()
{
    CObj::Print();

}

/*********************************************************************
函数名称 : PrintFromHead
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::PrintFromHead(XU32 uiCnt, bool bDetailFlag)
{
    return;
}

/*********************************************************************
函数名称 : PrintFromTail
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::PrintFromTail(XU32 uiCnt, bool bDetailFlag)
{
    return;
}

/*********************************************************************
函数名称 : PrintAll
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::PrintAll(bool bDetailFlag)
{
    return;
}



/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResNode::CResNode(CResource *pRes) : CLinkNode(1)        //give a initial ID first
{

    m_pRes = pRes;

}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResNode::~CResNode()
{
    m_pRes = XNULL;
}


/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CResNode::Print()
{
    CLinkNode::Print();
}


/*********************************************************************
函数名称 :  CResource
功能描述 :

参数输入 : uiId            XU32          对象ID
           uiMaxNum        XU32          最大对象数
           bNeedRandom     bool
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResource::CResource(XU32 uiId, XU32 uiMaxNum, bool bNeedRandom) : CObj(uiId)
{

    m_eErr = Init(uiMaxNum, bNeedRandom);

}


/*********************************************************************
函数名称 :  ~CResource
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResource::~CResource()
{
    if (XNULL != m_pIdleLink)
    {
        delete m_pIdleLink;
        m_pIdleLink = XNULL;
    }

    for(XU32 i = 0; i < m_uiMaxNum; ++i)
    {
        if (XNULL != m_pObjBuf[i])
        {
            delete m_pObjBuf[i];
            m_pObjBuf[i] = XNULL;
        }
    }

    if (XNULL != m_pObjBuf)
    {
        delete[] m_pObjBuf;
        m_pObjBuf = XNULL;
    }

}


/*********************************************************************
函数名称 :  Init
功能描述 :

参数输入 : uiMaxNum        XU32          最大对象数
           bNeedRandom     bool
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CResource::Init(XU32 uiMaxNum, bool bNeedRandom)
{
    CPP_ASSERT_RV((0 < uiMaxNum), ERR_INVALID_PARAM);

    m_uiRandomBits = g_RandomBits++;
    m_uiMaxNum     = uiMaxNum;
    m_uiRealNum    = 0;
    //m_uiPeakTimeId = 0;

    if(false == bNeedRandom)
    {
        m_uiRandomBits = 0;
    }
    else
    {
        //根据最大数目(决定随机BIT的位数),但随机BIT位最多16个
        m_uiRandomBits = MAX_RANDOM_BITS;
        uiMaxNum >>= MAX_RANDOM_BITS;
        while(uiMaxNum)
        {
            m_uiRandomBits--;
            uiMaxNum >>= 1;
        }

        //CPP_ASSERT_RV(0 <= m_uiRandomBits, ERR_INVALID_PARAM);
    }

    m_pObjBuf = new CResNode *[m_uiMaxNum];
    if(XNULL == m_pObjBuf)
    {
        return ERR_MEMORY;
    }

    m_pIdleLink = new CLink;
    if((XNULL == m_pIdleLink) ||
            (ERR_OK != m_pIdleLink->GetErr()))
    {
        return ERR_MEMORY;
    }

    for(XU32 i = 0; i < m_uiMaxNum; ++i)
    {
        m_pObjBuf[i] = XNULL;
    }

    return ERR_OK;
}


inline XU32 CResource::GetMaxNum() const
{
    return m_uiMaxNum;
}

inline XU32 CResource::GetRealNum() const
{
    return m_uiRealNum;
}

inline XU32 CResource::GetIdleNum() const
{
    return m_pIdleLink->GetNum();
}


/*********************************************************************
函数名称 : Add
功能描述 : 将节点添加到空闲列表表尾?

参数输入 : pObj        CResNode *
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CResource::Add(CResNode *pObj)
{
    EerrNo result = ERR_OK;

    CPP_ASSERT_RV((XNULL != pObj), ERR_INVALID_PARAM);
    if(m_uiRealNum >= m_uiMaxNum)
    {
        return ERR_MEMORY;
    }
    result = m_pIdleLink->AddToTail(pObj);
    if(ERR_OK != result)
    {
        return result;
    }
    m_pObjBuf[m_uiRealNum] = pObj;
    ++m_uiRealNum;
    (XVOID)pObj->SetId(m_uiRealNum);
    return ERR_OK;
}


/*********************************************************************
函数名称 : AllocNode
功能描述 : 添加到资源池中所分配的资源ID与从资源池中分配出去的不一样

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResNode* CResource::AllocNode()
{
    XU32 Tmp = 0;
    CLinkNode *pTmpObj = XNULL;
    //XU32 usednum = 0;

    //从空闲链表中移出来
    pTmpObj = m_pIdleLink->RemoveHead();
    if(XNULL == pTmpObj)
    {
        return XNULL;
    }
    //随机BIT数不为0
    if(m_uiRandomBits)
    {
        Tmp = m_uiRandom;
        Tmp <<= (32 - m_uiRandomBits);
        Tmp |= pTmpObj->GetId() & RESOURCE_INDEX_BITS_MASK;
        (XVOID)pTmpObj->SetId(Tmp);

        ++m_uiRandom;
    }

    //Determine if peak number
    //usednum = m_uiRealNum - m_pIdleLink->GetNum();
    //日志

    return (CResNode *)pTmpObj;
}


/*********************************************************************
函数名称 : FreeById
功能描述 : 若在忙链表中,则添加到空闲链中

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CResource::FreeById(XU32 uiId)
{
    CPP_ASSERT_RV((INVALID_OBJ_ID != uiId), ERR_INVALID_PARAM);

    if(RES_BUSY != GetStateById(uiId))
    {
        return ERR_INVALID_OPR;
    }

    return m_pIdleLink->AddToTail(GetById(uiId));
}


/*********************************************************************
函数名称 :GetById
功能描述 :id-->索引-->CResNode *pTmpObj-->id == pTmpObj?

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResNode* CResource::GetById(XU32 uiId)
{
    XU32 Tmp = 0;
    CResNode *pTmpObj = XNULL;

    CPP_ASSERT_RV((INVALID_OBJ_ID != uiId), XNULL);

    Tmp = uiId & RESOURCE_INDEX_BITS_MASK;
    if(Tmp > m_uiRealNum || Tmp==0)
    {
        return XNULL;
    }

    pTmpObj = m_pObjBuf[Tmp-1];
    if(uiId == pTmpObj->GetId())
    {
        return pTmpObj;
    }
    else
    {
        return XNULL;
    }
}


/*********************************************************************
函数名称 :GetStateById
功能描述 :id-->CResNode-->是否在空闲链表中

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EResState CResource::GetStateById(XU32 uiId)
{
    CResNode *pTmp = XNULL;

    CPP_ASSERT_RV((INVALID_OBJ_ID != uiId), RES_ERROR);

    pTmp = GetById(uiId);
    if(XNULL == pTmp)
    {
        return RES_ERROR;
    }

    if(true == m_pIdleLink->IsInLink(pTmp))
    {
        return RES_IDLE;
    }
    else
    {
        return RES_BUSY;
    }
}



/*********************************************************************
函数名称 : IsPrintNode
功能描述

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
bool CResource::IsPrintNode(CLinkNode *pObj)
{
    return false;
}


/*********************************************************************
函数名称 : Print
功能描述

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CResource::Print()
{
    CObj::Print();
}





/*********************************************************************
函数名称 :  CTreeNode
功能描述 :

参数输入 : pRes            CResource *    资源
           ucMaxBranchNum  XU8            最大分支数
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CTreeNode::CTreeNode(CResource *pRes, XU8 ucMaxBranchNum) : CResNode(pRes)
{
    XU8 i = 0;

    CPP_ASSERT_RN((0 != ucMaxBranchNum) && (MAX_BRANCH_NUM >= ucMaxBranchNum));

    m_pBranch = new CTreeNode *[ucMaxBranchNum];
    if(XNULL == m_pBranch)
    {
        m_eErr = ERR_MEMORY;
    }
    else
    {
        for(i = 0; i < ucMaxBranchNum; ++i)
        {
            m_pBranch[i] = XNULL;
        }
    }

    m_uiFruitId   = INVALID_OBJ_ID;
    m_ucBranchCnt = 0;

}


/*********************************************************************
函数名称 :  ~CTreeNode
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CTreeNode::~CTreeNode()
{
    if (XNULL != m_pBranch)
    {
        delete[] m_pBranch;
        m_pBranch = XNULL;
    }
}

//获取节点记录值
inline XPOINT CTreeNode::GetFruit() const
{
    return m_uiFruitId;
}

//设置节点记录值
inline XVOID CTreeNode::SetFruit(XPOINT uiFruitID)
{
    m_uiFruitId = uiFruitID;
}

inline CTreeNode* CTreeNode::GetBranch(XU8 ucBranchOffset) const
{
    if (XNULL == m_pBranch)
    {
        return XNULL;
    }

    return m_pBranch[ucBranchOffset];
}


/*********************************************************************
函数名称 : IsLeaf
功能描述 : 分支为0即为叶子节点

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
bool CTreeNode::IsLeaf()
{
    if(0 == m_ucBranchCnt)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*********************************************************************
函数名称 : IsRich
功能描述 : 有超过一个分枝或果实

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
bool CTreeNode::IsRich()
{
    if((1 < m_ucBranchCnt) || (INVALID_OBJ_ID != m_uiFruitId))
    {
        return true;
    }
    else
    {
        return false;
    }
}


/*********************************************************************
函数名称 : AddBranch
功能描述 : 添加分支

参数输入 : BranchOffset  XU8             分支偏移量
           pBranch       CTreeNode *     节点
参数输出 :
返回值   : EerrNo

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTreeNode::AddBranch(XU8 ucBranchOffset, CTreeNode *pBranch)
{
    CPP_ASSERT_RV(XNULL != pBranch, ERR_INVALID_OPR);
    CPP_ASSERT_RV(XNULL != m_pBranch, ERR_INVALID_OPR);

    //判断该分支是否存在，如果存在，则返回错误
    if(XNULL != m_pBranch[ucBranchOffset])
    {
        return ERR_INVALID_OPR;
    }
    else
    {
        m_pBranch[ucBranchOffset] = pBranch;

        m_ucBranchCnt++;
        return ERR_OK;
    }
}


/*********************************************************************
函数名称 :  DelBranch
功能描述 : 删除分支

参数输入 : BranchOffset  XU8        分支偏移量
参数输出 :
返回值   : EerrNo

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTreeNode::DelBranch(XU8 BranchOffset)
{
    CPP_ASSERT_RV(XNULL != m_pBranch, ERR_INVALID_OPR);
    if(XNULL == m_pBranch[BranchOffset])
    {
        return ERR_INVALID_OPR;
    }
    else
    {
        m_pBranch[BranchOffset] = XNULL;
        m_ucBranchCnt--;
        return ERR_OK;
    }
}

/*********************************************************************
函数名称 : Print
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTreeNode::Print()
{
    //CResNode::Print();

    //printf("TreeNode: BranchCnt=%u, Fruit=0x%x\n",m_ucBranchCnt, m_uiFruitId);
}


/*********************************************************************
函数名称 :  CTree
功能描述 :

参数输入 : uiId                XU32        对象ID
           uiMaxNode           XU32        最大节点数
           ucMaxBranchNum      XU8         最大分支数
           ucMaxTreeDepth      XU8         最大深度
参数输出 :
返回值   : EerrNo

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CTree::CTree(XU32 uiId, XU32 uiMaxNode, XU8 ucMaxBranchNum, XU8 ucMaxTreeDepth) : CResource(uiId, uiMaxNode)
{
    //if(ERR_OK == m_eErr)
    //{
    m_eErr = Init(ucMaxBranchNum, ucMaxTreeDepth);
    //}
}


/*********************************************************************
函数名称 :  ~CTree
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CTree::~CTree()
{
    if (XNULL != m_pRoot)
    {
        delete m_pRoot;
        m_pRoot = XNULL;
    }
}


/*********************************************************************
函数名称 :  Init
功能描述 : 初始化函数

参数输入 : ucMaxBranchNum  XU8       最大树分支数
           max_tree_depth  XU8       最大树深度
参数输出 :
返回值   : EerrNo

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTree::Init(XU8 ucMaxBranchNum, XU8 ucMaxTreeDepth)
{
    XU32 i = 0;
    EerrNo eErr = ERR_OK;
    CTreeNode *pNode = XNULL;

    CPP_ASSERT_RV((0 != ucMaxBranchNum) && (MAX_BRANCH_NUM >= ucMaxBranchNum), ERR_INVALID_OPR);
    CPP_ASSERT_RV((0 < ucMaxTreeDepth) && (MAX_TREE_DEPTH >= ucMaxTreeDepth), ERR_INVALID_OPR);

    m_ucMaxBranchNum = ucMaxBranchNum;
    m_ucMaxTreeDepth = ucMaxTreeDepth;

    m_pRoot = new CTreeNode(this, ucMaxBranchNum);
    if(XNULL == m_pRoot)
    {
        return ERR_MEMORY;
    }

    eErr = m_pRoot->GetErr();
    if(ERR_OK != eErr)
    {
        return eErr;
    }

    for(i = 0; i < m_uiMaxNum; ++i)
    {
        pNode = (CTreeNode *)new CTreeNode(this, ucMaxBranchNum);
        if(XNULL == pNode)
        {
            return ERR_MEMORY;
        }
        eErr = pNode->GetErr();
        if(ERR_OK != eErr)
        {
            delete pNode;
            return eErr;
        }

        // 调用资源类的ADD，把资源节点放到空闲链表表尾
        eErr = Add(pNode);
        if(ERR_OK != eErr)
        {
            delete pNode;
            return eErr;
        }
    }

    return ERR_OK;
}


/*********************************************************************
函数名称 :  AddFruit
功能描述 : 添加节点

参数输入 : ucBranchListCnt  XU8          搜索长度
           pucBranchList    XU8 *        号码串
           fruidID          XU32         果实值
参数输出 :
返回值   : EerrNo

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTree::AddFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT fruitID)
{
    XU8 i = 0 ;
    CTreeNode *pTmpNode = m_pRoot;
    CTreeNode *pBranch = XNULL;

    CPP_ASSERT_RV((0 != ucBranchListCnt) && (XNULL != pucBranchList) && (INVALID_OBJ_ID != fruitID), ERR_INVALID_OPR);

    if(ucBranchListCnt > m_ucMaxTreeDepth)
    {
        return ERR_INVALID_PARAM;
    }

    //输入的是数字，检查允许的数字是否大于最大分支数
    for(i = 0; i < ucBranchListCnt; ++i)
    {
        if(*(pucBranchList+i) >= m_ucMaxBranchNum)
        {
            return ERR_INVALID_PARAM;
        }
    }

    // 获取链表对象数
    if(ucBranchListCnt > GetIdleNum())
    {
        //可能没有空闲树节点用来分配
        return ERR_MEMORY;
    }

    for(i = 0; i < ucBranchListCnt; ++i)
    {
        //获取分支节点
        pBranch = pTmpNode->GetBranch((*pucBranchList));

        if(XNULL == pBranch)
        {
            //从资源池中分配一个节点
            pBranch = (CTreeNode *)AllocNode();
            CPP_ASSERT_RV(XNULL != pBranch, ERR_INVALID_OPR);

            //把pbranch  节点添加到树中
            (XVOID)pTmpNode->AddBranch((*pucBranchList), pBranch);
        }

        pTmpNode = pBranch;
        pucBranchList++;
    }

    CPP_ASSERT_RV(XNULL != pBranch, ERR_INVALID_OPR);
    if(INVALID_OBJ_ID == pBranch->GetFruit())
    {
        //设置果实ID
        pBranch->SetFruit(fruitID);
        return ERR_OK;
    }
    else
    {
        return ERR_OBJ_EXISTED;
    }
}

/*********************************************************************
函数名称 :  CheckRepeat
功能描述 : 通过分支检查是否存在该记录，并返回该记录
           不包括回朔功能

参数输入 : ucBranchListCnt  XU8          搜索长度
           pucBranchList    XU8 *        号码串
参数输出 : fruidID          XU32 &       输出果实值
返回值   : EerrNo

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/

EerrNo CTree::CheckRepeat(XU8 BranchListCnt, XU8 *BranchList, XPOINT &fruidID)
{
    XU8          i        = 0;
    CTreeNode   *pTmpNode = m_pRoot;
    CTreeNode   *pBranch  = XNULL;

    CPP_ASSERT_RV((0 != BranchListCnt) && (XNULL != BranchList), ERR_INVALID_OPR);

    if(BranchListCnt > m_ucMaxTreeDepth)
    {
        return ERR_INVALID_PARAM;
    }

    //验证参数的有效性
    for(i = 0; i < BranchListCnt; ++i )
    {
        if(*(BranchList+i) >= m_ucMaxBranchNum)
        {
            return ERR_INVALID_PARAM;
        }
    }

    fruidID = INVALID_OBJ_ID;
    for(i = 0; i < BranchListCnt; ++i)
    {
        pBranch = pTmpNode->GetBranch(*BranchList);
        if(XNULL == pBranch)
        {
            fruidID = INVALID_OBJ_ID;
            break;
        }
        else
        {
            fruidID = pBranch->GetFruit();
        }

        pTmpNode = pBranch;
        BranchList++;
    }

    if((i == BranchListCnt) && (INVALID_OBJ_ID != fruidID))
    {
        return ERR_OBJ_EXISTED;
    }
    else
    {
        return ERR_OBJ_NOT_FOUND;
    }
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTree::Clear()
{
    return RemoveAllFruit(m_pRoot);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTree::RemoveAllFruit(CTreeNode *pTreeNode)
{
    if (XNULL == pTreeNode)
    {
        return ERR_OK;
    }

    XU32 i;
    for (i = 0; i < m_ucMaxBranchNum; i++)
    {
        if (XNULL != pTreeNode->GetBranch(i))
        {
            (XVOID)RemoveAllFruit(pTreeNode->GetBranch(i));
            (XVOID)pTreeNode->DelBranch(i);
        }
    }
    pTreeNode->SetFruit(INVALID_OBJ_ID);

    if (m_pRoot != pTreeNode)
    {
        CPP_ASSERT_RV(ERR_OK == FreeById(pTreeNode->GetId()),ERR_INVALID_OPR);
    }

    return ERR_OK;
}

/*********************************************************************
函数名称 : RemoveFruit
功能描述 : 删除节点

参数输入 : ucBranchListCnt  XU8          删除的号码串长度
           pucBranchList    XU8 *        号码串
参数输出 : fruidID          XU32 &       输出果实值，移除成功的话，释放空间用
返回值   : EerrNo

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTree::RemoveFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruitID)
{
    XU8 i = 0;
    CTreeNode *pTmpNode  = m_pRoot;
    CTreeNode *pBranch   = XNULL;
    XU8 ucDeadCnt        = 0;
    XU8 DeadWood[MAX_TREE_DEPTH] = {0};
    CTreeNode *pCutBegin = m_pRoot; // pCutBegin 记录需要删除的分支父节点，此节点下的分支需要删除
    EerrNo eErr;

    CPP_ASSERT_RV((0 != ucBranchListCnt) && (XNULL != pucBranchList), ERR_INVALID_OPR);

    //输入的是数字，检查允许的数字是否大于最大分支数
    for(i=0; i<ucBranchListCnt; ++i)
    {
        if(*(pucBranchList+i) >= m_ucMaxBranchNum)
        {
            return ERR_INVALID_PARAM;
        }
    }

    //根据关键码，针对每一分支点顺序查找
    for(i = 0; i < ucBranchListCnt; ++i)
    {
        pBranch = pTmpNode->GetBranch((*pucBranchList));

        if(XNULL == pBranch)
        {
            return ERR_OBJ_NOT_FOUND;
        }

        // 删除一条记录时可能需要相应删除结点，因此需要
        // 记录下需要删除的号码节点索引，当满足以下条件时需
        // 要跳过包括: 1) 分支数为1 并且有果实;2) 分支数大于1(不管有无果实)
        //(分支数大于1或果实ID为有效)和分支节点不是叶子(分支节点数为0)
        if((true == pBranch->IsRich()) && (false == pBranch->IsLeaf()))
        {
            pCutBegin = pBranch;
            ucDeadCnt = 0;
        }
        else
        {
            DeadWood[ucDeadCnt++] =(*pucBranchList);
        }

        pTmpNode = pBranch;
        pucBranchList++;
    }

    CPP_ASSERT_RV(XNULL != pBranch, ERR_INVALID_OPR);
    uiFruitID = pBranch->GetFruit();
    if(INVALID_OBJ_ID != uiFruitID)
    {
        pBranch->SetFruit(INVALID_OBJ_ID);
    }
    else
    {
        return ERR_OBJ_NOT_FOUND;
    }

    // 当为叶子节点时才需要删除父节点，并且循环次数为ucDeadCnt +1,
    // 原因时再下一个循环中才删除节点
    if(true == pBranch->IsLeaf())
    {
        for(i = 0; i <= ucDeadCnt; ++i)
        {
            //不获取叶分支
            if(ucDeadCnt != i)
            {
                pBranch = pCutBegin->GetBranch(DeadWood[i]);
                (XVOID)pCutBegin->DelBranch(DeadWood[i]);
            }

            //不能删除头节点
            if(0 != i)
            {
                //若在忙链表中,则添加到空闲链中
                eErr = FreeById(pCutBegin->GetId());
                CPP_ASSERT_RV(ERR_OK == eErr,ERR_INVALID_OPR);
            }

            pCutBegin = pBranch;
        }
    }

    return ERR_OK;
}



/*********************************************************************
函数名称 :  SearchFruit
功能描述 : 通过分支索引搜索果实

参数输入 : ucBranchListCnt  XU8          搜索长度
           pucBranchList    XU8 *        号码串
参数输出 : fruidID          XU32 &       输出果实值
返回值   : ETreeResult

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
ETreeResult CTree::SearchFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruidID)
{
    XU8 i;
    CTreeNode *pTmpNode = m_pRoot;
    CTreeNode *pBranch;
    XPOINT tmpFruit;

    CPP_ASSERT_RV(XNULL != pucBranchList, TREE_FAIL);

    //搜索长度必须大于0
    //允许传空号码，返回找不到，  060622
    //CPP_ASSERT( 0 < ucBranchListCnt );

    if(ucBranchListCnt == 0)
    {
        return TREE_FAIL;
    }

    //输入的是数字，检查允许的数字是否大于最大分支数
    for(i=0; i<ucBranchListCnt; ++i)
    {
        if(*(pucBranchList+i) >= m_ucMaxBranchNum)
        {
            return TREE_FAIL;
        }
    }

    uiFruidID = INVALID_OBJ_ID;

    for(i = 0; i < ucBranchListCnt; ++i)
    {
        //获取分支节点
        pBranch = pTmpNode->GetBranch((*pucBranchList));

        if(XNULL == pBranch)
        {
            break;
        }

        //获取果实ID
        tmpFruit = pBranch->GetFruit();
        if(INVALID_OBJ_ID != tmpFruit)
        {
            uiFruidID = tmpFruit;
        }

        pTmpNode = pBranch;
        pucBranchList++;
    }

    if((i == ucBranchListCnt) && (false == pBranch->IsLeaf()))
    {
        return TREE_NOT_COMPLETE;
    }
    else
    {
        if(INVALID_OBJ_ID == uiFruidID)
        {
            return TREE_NOT_FOUND;
        }
        else
        {
            return TREE_FIND;
        }
    }
}


/*********************************************************************
函数名称 :  SearchMulFruit
功能描述 : 通过分支索引搜索果实

参数输入 : ucBranchListCnt  XU8          搜索长度
           pucBranchList    XU8 *        号码串
参数输出 : fruidID          XU32 &       输出果实值
返回值   : ETreeResult

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
ETreeResult CTree::SearchMulFruit(XU8 ucBranchListCnt, XU8 *pucBranchList,
                                  XPOINT * puiFruidID,XU8 &ucFruitLen)
{
    CPP_ASSERT_RV(XNULL != pucBranchList, TREE_FAIL);
    CPP_ASSERT_RV(XNULL != puiFruidID, TREE_FAIL);
    CPP_ASSERT_RV(XNULL != m_pRoot, TREE_FAIL);

    XU8 i = 0;
    CTreeNode *pTmpNode = m_pRoot;
    CTreeNode *pBranch = XNULL;
    XPOINT uiTmpFruit = 0;

    //搜索长度必须大于0,
    //允许传空号码, 返回找不到,  060622
    //CPP_ASSERT( 0 < ucBranchListCnt );

    if(ucBranchListCnt == 0)
    {
        return TREE_FAIL;
    }

    //输入的是数字，检查允许的数字是否大于最大分支数
    for(i = 0; i < ucBranchListCnt; ++i)
    {
        if(*(pucBranchList+i) >= m_ucMaxBranchNum)
        {
            return TREE_FAIL;
        }
    }

    XOS_MemSet(puiFruidID, INVALID_OBJ_ID, ucBranchListCnt*sizeof(XPOINT));

    //果实节点数
    ucFruitLen = 0;
    for(i = 0; i < ucBranchListCnt; ++i)
    {
        //获取分支节点
        pBranch = pTmpNode->GetBranch((*pucBranchList));

        if(XNULL == pBranch)
        {
            break;
        }

        //获取果实ID
        uiTmpFruit = pBranch->GetFruit();
        if(INVALID_OBJ_ID != uiTmpFruit)
        {
            puiFruidID[ucFruitLen] = uiTmpFruit;
            ucFruitLen ++;
        }

        pTmpNode = pBranch;
        pucBranchList++;
    }

    //找到最后一个节点，而且该节点不是叶子节点,注意:在这种情况下，可能找到保存果实节点记录
    if((i == ucBranchListCnt) && (false == pBranch->IsLeaf()))
    {
        return TREE_NOT_COMPLETE;
    }
    else
    {
        //防止访问越界
        if(0 == ucFruitLen)
        {
            ucFruitLen ++;
        }

        //判断是否无效节点
        if(INVALID_OBJ_ID == puiFruidID[ucFruitLen-1])
        {
            ucFruitLen = 0;
            return TREE_NOT_FOUND;
        }
        else
        {
            return TREE_FIND;
        }
    }
}


/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTree::PrintNode(CTreeNode *pNode, XU8 ucLevel, XU8 ucBranchOffset)
{
    CPP_ASSERT_RN(XNULL != pNode);
    XU8 i = 0;
    CTreeNode *pBranch;

    /*
    for(i=0; i<ucLevel; ++i)
    {
        printf("level:%u " , Level);
    }
    printf("level:%u \n" , Level);
    printf("Tree: Offset %u ", BranchOffset);
    pNode->Print();
    */

    XPOINT uiFruit = 0;
    uiFruit = pNode->GetFruit();
    if(0 != uiFruit)
    {
        //printf("%d\n",uiFruit);
    }

    for(i = 0; i < m_ucMaxBranchNum; ++i)
    {
        pBranch = pNode->GetBranch(i);

        if(XNULL != pBranch)
        {
            PrintNode(pBranch, ucLevel+1, i);
        }
    }
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTree::GetAllFruit(XPOINT * puiFruit,XU32 & uiFruitLen)
{
    XU32 uiTemp = 0;
    GetTreeFruit(m_pRoot,0,puiFruit,uiTemp);
    uiFruitLen = uiTemp;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTree::GetTreeFruit(CTreeNode *pNode, XU8 ucLevel, XPOINT * puiFruit, XU32 & uiFruitNum)
{
    CPP_ASSERT_RN(XNULL != pNode);
    CPP_ASSERT_RN(XNULL != puiFruit);
    XPOINT uiFruit = 0;
    uiFruit = pNode->GetFruit();
    //XU32 uiFruitCnt = uiFruitNum;
    if(0 != uiFruit)
    {
        *( puiFruit + uiFruitNum)  = uiFruit;
        // uiFruitCnt = uiFruitCnt + 1;
        ++uiFruitNum;
    }

    CTreeNode *pBranch;
    for(XU32 i = 0; i < m_ucMaxBranchNum; ++i)
    {
        pBranch = pNode->GetBranch(i);

        if(XNULL != pBranch)
        {
            //GetTreeFruit(pBranch, ucLevel+1, puiFruit,uiFruitCnt);
            GetTreeFruit(pBranch, ucLevel+1, puiFruit,uiFruitNum);
        }
    }
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTree::PrintTree()
{
    //printf("Tree: Following are items in tree\n\n");
    PrintNode(m_pRoot, 0, 0);
}



