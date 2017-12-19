/****************************************************************************
�ļ���	:cpp_tree.cpp

�ļ�����:

����    :yuxiao

��������:2006/2/21

�޸ļ�¼:
*************************************************************************/
#include "cpp_adapter.h"
#include "cpp_tree.h"


/*lint -e429 */
/*lint -e1551*/


#define RESOURCE_INDEX_BITS_MASK ((m_uiRandomBits) ? ((1<<(32 - m_uiRandomBits))-1) : 0xFFFFFFFF)

XU32 g_RandomBits = 0;

XU32 g_LinkId = 1;

/*********************************************************************
�������� : CObj
�������� :

�������� : XU32 uiId
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CObj::CObj(XU32 uiId)
{

    m_uiId = uiId;
    m_eErr = ERR_OK;
}


/*********************************************************************
�������� : ~CObj
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CObj::~CObj()
{
    m_uiId = INVALID_OBJ_ID;
}

//��ȡ����ID
inline XU32 CObj::GetId() const
{
    return m_uiId;
}

//��ȡ����ֵ
inline EerrNo CObj::GetErr() const
{
    return m_eErr;
}



/*********************************************************************
�������� : SetId
�������� :

�������� : XU32  uiId
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CObj::SetId(XU32 uiId)
{
    CPP_ASSERT_RV(INVALID_OBJ_ID != uiId, ERR_INVALID_PARAM);

    m_uiId = uiId;

    return ERR_OK;
}

/*********************************************************************
�������� : SetErr
�������� :

�������� : EerrNo eErr
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CObj::SetErr(EerrNo eErr)
{
    m_eErr = eErr;
}


/*********************************************************************
�������� : Print
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CObj::Print()
{

}


/*********************************************************************
�������� : CLinkNode
�������� :

�������� : XU32 uiId
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLinkNode::CLinkNode(XU32 uiId) : CObj(uiId)
{
    Init();
}

/*********************************************************************
�������� : CLinkNode
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CLinkNode::~CLinkNode()
{
    m_uiLinkId = INVALID_OBJ_ID;
}


/*********************************************************************
�������� : Init
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLinkNode::Init()
{
    m_uiLinkId = INVALID_OBJ_ID;
    m_pPrev    = XNULL;
    m_pNext    = XNULL;
}

//��ȡ�ڵ�ID
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
�������� : Insert
�������� : �ѵ�ǰ�������pobj��ǰ��

�������� : uiLinkId      XU32
           pObj          CLinkNode *
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLinkNode::Insert(XU32 uiLinkId, CLinkNode *pObj)
{
    CPP_ASSERT_RV(INVALID_OBJ_ID != uiLinkId, ERR_INVALID_PARAM);

    if(pObj == this)
    {
        //����������ǰ����
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
�������� : Append
�������� : �ѵ�ǰ�������pobj����

�������� : uiLinkId      XU32           pObj�ڵ�������ڵ�����Ķ���ID
           pObj          CLinkNode*
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLinkNode::Append(XU32 uiLinkId, CLinkNode *pObj)
{
    CPP_ASSERT_RV(INVALID_OBJ_ID != uiLinkId, ERR_INVALID_PARAM);

    if(pObj == this)
    {
        //�����������
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
�������� : Remove
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : CLink
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : ~CLink
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : IsInLink
�������� :

�������� : pObj     CLinkNode*      ����ڵ�
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : Search
�������� :

�������� : uiId      XU32
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : AddToHead
�������� :

�������� : pObj      CLinkNode*       ����ڵ�
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : AddToTail
�������� :

�������� : pObj      CLinkNode*       ����ڵ�
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CLink::AddToTail(CLinkNode *pObj)
{
    EerrNo result = ERR_OK;

    CPP_ASSERT_RV(XNULL != pObj, ERR_INVALID_PARAM);

    CPP_ASSERT_RV(INVALID_OBJ_ID != pObj->GetId(), ERR_INVALID_PARAM);  //Can't insert uninitial object

    //��obj��ӵ�m_pTail����
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
�������� : Remove
�������� :

�������� : pObj      CLinkNode*       ����ڵ�
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : RemoveHead
�������� :

�������� : pObj      CLinkNode*       ����ڵ�
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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

    //���������Ϊ0,m_pHead���벻Ϊ��
    //CPP_ASSERT(XNULL != m_pHead);

    pTmp1 = m_pHead;
    pTmp2 = m_pHead->GetNext();

    if(ERR_OK != m_pHead->Remove(m_uiId))
    {
        //060419�� yuxiao
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
�������� : RemoveTail
�������� :

�������� :
������� :
����ֵ   : CLinkNode*

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : RemoveAll
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : Print
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::Print()
{
    CObj::Print();

}

/*********************************************************************
�������� : PrintFromHead
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::PrintFromHead(XU32 uiCnt, bool bDetailFlag)
{
    return;
}

/*********************************************************************
�������� : PrintFromTail
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::PrintFromTail(XU32 uiCnt, bool bDetailFlag)
{
    return;
}

/*********************************************************************
�������� : PrintAll
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CLink::PrintAll(bool bDetailFlag)
{
    return;
}



/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResNode::CResNode(CResource *pRes) : CLinkNode(1)        //give a initial ID first
{

    m_pRes = pRes;

}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResNode::~CResNode()
{
    m_pRes = XNULL;
}


/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CResNode::Print()
{
    CLinkNode::Print();
}


/*********************************************************************
�������� :  CResource
�������� :

�������� : uiId            XU32          ����ID
           uiMaxNum        XU32          ��������
           bNeedRandom     bool
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResource::CResource(XU32 uiId, XU32 uiMaxNum, bool bNeedRandom) : CObj(uiId)
{

    m_eErr = Init(uiMaxNum, bNeedRandom);

}


/*********************************************************************
�������� :  ~CResource
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :  Init
�������� :

�������� : uiMaxNum        XU32          ��������
           bNeedRandom     bool
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
        //���������Ŀ(�������BIT��λ��),�����BITλ���16��
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
�������� : Add
�������� : ���ڵ���ӵ������б��β?

�������� : pObj        CResNode *
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : AllocNode
�������� : ��ӵ���Դ�������������ԴID�����Դ���з����ȥ�Ĳ�һ��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CResNode* CResource::AllocNode()
{
    XU32 Tmp = 0;
    CLinkNode *pTmpObj = XNULL;
    //XU32 usednum = 0;

    //�ӿ����������Ƴ���
    pTmpObj = m_pIdleLink->RemoveHead();
    if(XNULL == pTmpObj)
    {
        return XNULL;
    }
    //���BIT����Ϊ0
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
    //��־

    return (CResNode *)pTmpObj;
}


/*********************************************************************
�������� : FreeById
�������� : ����æ������,����ӵ���������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :GetById
�������� :id-->����-->CResNode *pTmpObj-->id == pTmpObj?

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :GetStateById
�������� :id-->CResNode-->�Ƿ��ڿ���������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : IsPrintNode
��������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
bool CResource::IsPrintNode(CLinkNode *pObj)
{
    return false;
}


/*********************************************************************
�������� : Print
��������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CResource::Print()
{
    CObj::Print();
}





/*********************************************************************
�������� :  CTreeNode
�������� :

�������� : pRes            CResource *    ��Դ
           ucMaxBranchNum  XU8            ����֧��
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :  ~CTreeNode
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CTreeNode::~CTreeNode()
{
    if (XNULL != m_pBranch)
    {
        delete[] m_pBranch;
        m_pBranch = XNULL;
    }
}

//��ȡ�ڵ��¼ֵ
inline XPOINT CTreeNode::GetFruit() const
{
    return m_uiFruitId;
}

//���ýڵ��¼ֵ
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
�������� : IsLeaf
�������� : ��֧Ϊ0��ΪҶ�ӽڵ�

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : IsRich
�������� : �г���һ����֦���ʵ

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : AddBranch
�������� : ��ӷ�֧

�������� : BranchOffset  XU8             ��֧ƫ����
           pBranch       CTreeNode *     �ڵ�
������� :
����ֵ   : EerrNo

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTreeNode::AddBranch(XU8 ucBranchOffset, CTreeNode *pBranch)
{
    CPP_ASSERT_RV(XNULL != pBranch, ERR_INVALID_OPR);
    CPP_ASSERT_RV(XNULL != m_pBranch, ERR_INVALID_OPR);

    //�жϸ÷�֧�Ƿ���ڣ�������ڣ��򷵻ش���
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
�������� :  DelBranch
�������� : ɾ����֧

�������� : BranchOffset  XU8        ��֧ƫ����
������� :
����ֵ   : EerrNo

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : Print
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTreeNode::Print()
{
    //CResNode::Print();

    //printf("TreeNode: BranchCnt=%u, Fruit=0x%x\n",m_ucBranchCnt, m_uiFruitId);
}


/*********************************************************************
�������� :  CTree
�������� :

�������� : uiId                XU32        ����ID
           uiMaxNode           XU32        ���ڵ���
           ucMaxBranchNum      XU8         ����֧��
           ucMaxTreeDepth      XU8         ������
������� :
����ֵ   : EerrNo

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CTree::CTree(XU32 uiId, XU32 uiMaxNode, XU8 ucMaxBranchNum, XU8 ucMaxTreeDepth) : CResource(uiId, uiMaxNode)
{
    //if(ERR_OK == m_eErr)
    //{
    m_eErr = Init(ucMaxBranchNum, ucMaxTreeDepth);
    //}
}


/*********************************************************************
�������� :  ~CTree
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :  Init
�������� : ��ʼ������

�������� : ucMaxBranchNum  XU8       �������֧��
           max_tree_depth  XU8       ��������
������� :
����ֵ   : EerrNo

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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

        // ������Դ���ADD������Դ�ڵ�ŵ����������β
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
�������� :  AddFruit
�������� : ��ӽڵ�

�������� : ucBranchListCnt  XU8          ��������
           pucBranchList    XU8 *        ���봮
           fruidID          XU32         ��ʵֵ
������� :
����ֵ   : EerrNo

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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

    //����������֣��������������Ƿ��������֧��
    for(i = 0; i < ucBranchListCnt; ++i)
    {
        if(*(pucBranchList+i) >= m_ucMaxBranchNum)
        {
            return ERR_INVALID_PARAM;
        }
    }

    // ��ȡ���������
    if(ucBranchListCnt > GetIdleNum())
    {
        //����û�п������ڵ���������
        return ERR_MEMORY;
    }

    for(i = 0; i < ucBranchListCnt; ++i)
    {
        //��ȡ��֧�ڵ�
        pBranch = pTmpNode->GetBranch((*pucBranchList));

        if(XNULL == pBranch)
        {
            //����Դ���з���һ���ڵ�
            pBranch = (CTreeNode *)AllocNode();
            CPP_ASSERT_RV(XNULL != pBranch, ERR_INVALID_OPR);

            //��pbranch  �ڵ���ӵ�����
            (XVOID)pTmpNode->AddBranch((*pucBranchList), pBranch);
        }

        pTmpNode = pBranch;
        pucBranchList++;
    }

    CPP_ASSERT_RV(XNULL != pBranch, ERR_INVALID_OPR);
    if(INVALID_OBJ_ID == pBranch->GetFruit())
    {
        //���ù�ʵID
        pBranch->SetFruit(fruitID);
        return ERR_OK;
    }
    else
    {
        return ERR_OBJ_EXISTED;
    }
}

/*********************************************************************
�������� :  CheckRepeat
�������� : ͨ����֧����Ƿ���ڸü�¼�������ظü�¼
           ��������˷����

�������� : ucBranchListCnt  XU8          ��������
           pucBranchList    XU8 *        ���봮
������� : fruidID          XU32 &       �����ʵֵ
����ֵ   : EerrNo

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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

    //��֤��������Ч��
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTree::Clear()
{
    return RemoveAllFruit(m_pRoot);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : RemoveFruit
�������� : ɾ���ڵ�

�������� : ucBranchListCnt  XU8          ɾ���ĺ��봮����
           pucBranchList    XU8 *        ���봮
������� : fruidID          XU32 &       �����ʵֵ���Ƴ��ɹ��Ļ����ͷſռ���
����ֵ   : EerrNo

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
EerrNo CTree::RemoveFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruitID)
{
    XU8 i = 0;
    CTreeNode *pTmpNode  = m_pRoot;
    CTreeNode *pBranch   = XNULL;
    XU8 ucDeadCnt        = 0;
    XU8 DeadWood[MAX_TREE_DEPTH] = {0};
    CTreeNode *pCutBegin = m_pRoot; // pCutBegin ��¼��Ҫɾ���ķ�֧���ڵ㣬�˽ڵ��µķ�֧��Ҫɾ��
    EerrNo eErr;

    CPP_ASSERT_RV((0 != ucBranchListCnt) && (XNULL != pucBranchList), ERR_INVALID_OPR);

    //����������֣��������������Ƿ��������֧��
    for(i=0; i<ucBranchListCnt; ++i)
    {
        if(*(pucBranchList+i) >= m_ucMaxBranchNum)
        {
            return ERR_INVALID_PARAM;
        }
    }

    //���ݹؼ��룬���ÿһ��֧��˳�����
    for(i = 0; i < ucBranchListCnt; ++i)
    {
        pBranch = pTmpNode->GetBranch((*pucBranchList));

        if(XNULL == pBranch)
        {
            return ERR_OBJ_NOT_FOUND;
        }

        // ɾ��һ����¼ʱ������Ҫ��Ӧɾ����㣬�����Ҫ
        // ��¼����Ҫɾ���ĺ���ڵ���������������������ʱ��
        // Ҫ��������: 1) ��֧��Ϊ1 �����й�ʵ;2) ��֧������1(�������޹�ʵ)
        //(��֧������1���ʵIDΪ��Ч)�ͷ�֧�ڵ㲻��Ҷ��(��֧�ڵ���Ϊ0)
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

    // ��ΪҶ�ӽڵ�ʱ����Ҫɾ�����ڵ㣬����ѭ������ΪucDeadCnt +1,
    // ԭ��ʱ����һ��ѭ���в�ɾ���ڵ�
    if(true == pBranch->IsLeaf())
    {
        for(i = 0; i <= ucDeadCnt; ++i)
        {
            //����ȡҶ��֧
            if(ucDeadCnt != i)
            {
                pBranch = pCutBegin->GetBranch(DeadWood[i]);
                (XVOID)pCutBegin->DelBranch(DeadWood[i]);
            }

            //����ɾ��ͷ�ڵ�
            if(0 != i)
            {
                //����æ������,����ӵ���������
                eErr = FreeById(pCutBegin->GetId());
                CPP_ASSERT_RV(ERR_OK == eErr,ERR_INVALID_OPR);
            }

            pCutBegin = pBranch;
        }
    }

    return ERR_OK;
}



/*********************************************************************
�������� :  SearchFruit
�������� : ͨ����֧����������ʵ

�������� : ucBranchListCnt  XU8          ��������
           pucBranchList    XU8 *        ���봮
������� : fruidID          XU32 &       �����ʵֵ
����ֵ   : ETreeResult

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
ETreeResult CTree::SearchFruit(XU8 ucBranchListCnt, XU8 *pucBranchList, XPOINT &uiFruidID)
{
    XU8 i;
    CTreeNode *pTmpNode = m_pRoot;
    CTreeNode *pBranch;
    XPOINT tmpFruit;

    CPP_ASSERT_RV(XNULL != pucBranchList, TREE_FAIL);

    //�������ȱ������0
    //�����պ��룬�����Ҳ�����  060622
    //CPP_ASSERT( 0 < ucBranchListCnt );

    if(ucBranchListCnt == 0)
    {
        return TREE_FAIL;
    }

    //����������֣��������������Ƿ��������֧��
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
        //��ȡ��֧�ڵ�
        pBranch = pTmpNode->GetBranch((*pucBranchList));

        if(XNULL == pBranch)
        {
            break;
        }

        //��ȡ��ʵID
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
�������� :  SearchMulFruit
�������� : ͨ����֧����������ʵ

�������� : ucBranchListCnt  XU8          ��������
           pucBranchList    XU8 *        ���봮
������� : fruidID          XU32 &       �����ʵֵ
����ֵ   : ETreeResult

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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

    //�������ȱ������0,
    //�����պ���, �����Ҳ���,  060622
    //CPP_ASSERT( 0 < ucBranchListCnt );

    if(ucBranchListCnt == 0)
    {
        return TREE_FAIL;
    }

    //����������֣��������������Ƿ��������֧��
    for(i = 0; i < ucBranchListCnt; ++i)
    {
        if(*(pucBranchList+i) >= m_ucMaxBranchNum)
        {
            return TREE_FAIL;
        }
    }

    XOS_MemSet(puiFruidID, INVALID_OBJ_ID, ucBranchListCnt*sizeof(XPOINT));

    //��ʵ�ڵ���
    ucFruitLen = 0;
    for(i = 0; i < ucBranchListCnt; ++i)
    {
        //��ȡ��֧�ڵ�
        pBranch = pTmpNode->GetBranch((*pucBranchList));

        if(XNULL == pBranch)
        {
            break;
        }

        //��ȡ��ʵID
        uiTmpFruit = pBranch->GetFruit();
        if(INVALID_OBJ_ID != uiTmpFruit)
        {
            puiFruidID[ucFruitLen] = uiTmpFruit;
            ucFruitLen ++;
        }

        pTmpNode = pBranch;
        pucBranchList++;
    }

    //�ҵ����һ���ڵ㣬���Ҹýڵ㲻��Ҷ�ӽڵ�,ע��:����������£������ҵ������ʵ�ڵ��¼
    if((i == ucBranchListCnt) && (false == pBranch->IsLeaf()))
    {
        return TREE_NOT_COMPLETE;
    }
    else
    {
        //��ֹ����Խ��
        if(0 == ucFruitLen)
        {
            ucFruitLen ++;
        }

        //�ж��Ƿ���Ч�ڵ�
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTree::GetAllFruit(XPOINT * puiFruit,XU32 & uiFruitLen)
{
    XU32 uiTemp = 0;
    GetTreeFruit(m_pRoot,0,puiFruit,uiTemp);
    uiFruitLen = uiTemp;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CTree::PrintTree()
{
    //printf("Tree: Following are items in tree\n\n");
    PrintNode(m_pRoot, 0, 0);
}



