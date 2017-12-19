/************************************************************************************
�ļ���  :cpp_frmmiddle.cpp

�ļ�����:

����    :zzt

��������:2005/10/20

�޸ļ�¼:

************************************************************************************/
#include "cpp_frmmiddle.h"


/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CChunk::CChunk(XU32 uiSize,size_t usCount)
    :m_rec(usCount)
{
    Init( uiSize,usCount);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CChunk::CChunk(XU32 uiSize) : m_rec(0)
{
    //CPP_ASSERT_RN(uiSize > 0);

#ifdef XOS_ARCH_64
    uiSize      = ((uiSize+7)>>3)<<3; //��uiSize����8�ı�������֮����Բ��Ϊ8�ı���
#else
    uiSize      = ((uiSize+3)>>2)<<2; //��uiSize����4�ı�������֮����Բ��Ϊ4�ı���
#endif

    m_uiSize    = uiSize;
    m_pchStart  = XNULL;
    m_usFrIdx   = 0;
    m_usLastIdx = m_usFrIdx;
}

/*********************************************************************
�������� : CChunk
�������� : ��������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CChunk::CChunk(const CChunk& other)
    :m_rec(other.m_rec.GetMaxNum())
{
    Init(other.m_uiSize,m_rec.GetMaxNum());
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CChunk::reserve(size_t n)
{
    // ֻ�����ڳ�ʼ���׶ε��øú���
    CPP_ASSERT_RN(0 == GetRec().GetAlloced()); // û��ʹ�ÿ��п�

    XU32 savSize = m_uiSize;
    Clear();
    m_rec.ResetMaxNum(n);
    Init( savSize, n);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CChunk::Init(XU32 uiSize,size_t usCount)
{
    CPP_ASSERT_RN(usCount>0 && uiSize >0);

#ifdef XOS_ARCH_64
    uiSize      = ((uiSize+7)>>3)<<3; //��uiSize����8�ı�������֮����Բ��Ϊ8�ı���
#else
    uiSize      = ((uiSize+3)>>2)<<2; //��uiSize����4�ı�������֮����Բ��Ϊ4�ı���
#endif

    m_uiSize    = uiSize;
    m_pchStart  = new XU8[m_uiSize * BlkCount()];

    //�����л���������,���һ����������NEXTָ�����ֵΪm_usCount
    for(size_t i = 0; i < BlkCount(); ++i)
    {
        NextIndex(i)=i+1;
    }

    m_usFrIdx   = 0;
    m_usLastIdx = BlkCount() - 1;
}

/*********************************************************************
�������� :  CChunk
�������� : ��ֵ����

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CChunk& CChunk::operator=(const CChunk& other)
{
    if(this != &other)
    {
        Clear();
        //����
        m_rec.ResetMaxNum(other.m_rec.GetMaxNum());
        Init(other.m_uiSize,m_rec.GetMaxNum());
    }
    return *this;
}

/*********************************************************************
�������� :  CChunk
�������� : ��ֵ����

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CChunk::swap(CChunk& other)
{
    std::swap(m_rec,other.m_rec);
    std::swap(m_pchStart,other.m_pchStart);
    std::swap(m_uiSize,other.m_uiSize);
    std::swap(m_usFrIdx,other.m_usFrIdx);
    std::swap(m_usLastIdx,other.m_usLastIdx);
}

/*********************************************************************
�������� :  AddByStar
�������� :  m_ChunkByStart��Chunk������������������,���ò�������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CChunkMgr::AddByStar(CChunk* pChunk)
{
    CPP_ASSERT_RN(pChunk != XNULL);

    m_ChunkByStart.push_back(XNULL);
    for(XS32 i = (XS32)m_ChunkByStart.size() -2; i>= 0; --i)
    {
        if(m_ChunkByStart[(XU32)i]->GetStartAddr()  > pChunk->GetStartAddr())
        {
            m_ChunkByStart[(XU32)(i+1)] = m_ChunkByStart[(XU32)i];
        }
        else
        {
            m_ChunkByStart[(XU32)(i+1)] = pChunk;
            return ;
        }
    }
    m_ChunkByStart[0] = pChunk;
}

/*********************************************************************
�������� :  findbySize
�������� :   Ѱ�ҵ�һ��GetBlockSize>=uiSize��CChunk����

�������� :  uiSize      XU32        �ڴ���С
������� :
����ֵ   :  XS32         CChunk����ָ�������,δ�ҵ��򷵻�m_ChunkBySize.size()

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS32 CChunkMgr::findbySize(XU32 uiSize)const
{
    XS32 iLeft  = 0;
    XS32 iRight = static_cast<XS32>(m_ChunkBySize.size())-1;

    while(iLeft <=iRight)
    {
        XS32 iMiddle = (iLeft +iRight)/2;
        if(m_ChunkBySize[(XU32)iMiddle] ->GetBlockSize() ==uiSize)
        {
            return iMiddle;
        }
        else if (m_ChunkBySize[(XU32)iMiddle]->GetBlockSize() < uiSize)
        {
            iLeft = iMiddle +1;
        }
        else
        {
            iRight = iMiddle-1;
        }
    }
    return iLeft;
}

/*********************************************************************
�������� : findbyLeftAddr
�������� :   Ѱ��ucp �����Ķ���chunk.�۰����

�������� :  ucp          XU8 *         �ڴ�����ʼ��ַ
������� :
����ֵ   :  XS32         CChunk����ָ�������,δ�ҵ��򷵻�-1

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS32 CChunkMgr::findbyLeftAddr(XU8 *ucp)const
{
    CPP_ASSERT_RV(ucp !=XNULL, -1);

    XS32 iLeft  = 0;
    XS32 iRight = static_cast<XS32>(m_ChunkByStart.size())-1;

    while(iLeft <= iRight)
    {
        XS32 iMiddle = (iLeft +iRight)/2;
        if(m_ChunkByStart[(XU32)iMiddle]->GetStartAddr() ==ucp)
        {
            return iMiddle;
        }
        else if (m_ChunkByStart[(XU32)iMiddle]->GetStartAddr() <ucp)
        {
            iLeft = iMiddle +1;
        }
        else
        {
            iRight = iMiddle-1;
        }
    }
    //������֮��1
    if(iRight>=0 && m_ChunkByStart[(XU32)iRight]->GetBufEnd()<= ucp )
    {
        iRight = -1;
    }
    return iRight;
}

/*********************************************************************
�������� :GetRandomBitNum
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XU32 IProcesserId::GetRandomBitNum()
{
    return 0;
}

/*********************************************************************
�������� :
�������� :

�������� : index      XU32      �����е�����,���������1��ʼ
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XU32 CRandomId::SpawnNewId(XS32 index)
{
    CPP_ASSERT_RV((index > 0)&&(index <= static_cast<XS32>(m_uiMaxNum)), 0XFFFFFFFF);

    //��֤ÿ�η����ֵ��һ��
    ++m_uiRan;

    // �����λ���㣬
    return ((m_uiRan << NoRndBitNum())|(static_cast<XU32>(index) & NoRndMask())) & 0X7FFFFFFF;
}

/*********************************************************************
�������� : RevertToIndex
�������� :   ��ID ���������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS32 CRandomId::RevertToIndex(XU32 Id)const
{
    return static_cast<XS32>(Id & NoRndMask());
}

/*********************************************************************
�������� :GetRandomBitNum
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XU32 CRandomId::GetRandomBitNum()
{
    return m_uiRndbitNum;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XU32 CNoRandomId::SpawnNewId(XS32 index)
{
    return (XU32)index;
}

/*********************************************************************
�������� :
�������� : ��ID������������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS32 CNoRandomId::RevertToIndex(XU32 Id)const
{
    return (XS32)Id;
}

/*********************************************************************
�������� :
�������� : �ú�������һ��ֵ,��ֵ�Ĵ�С��2��n�η��������ǵ�һ��>=i
           Ŀǰ��hastableʹ��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
size_t next_power_two(size_t i)
{
    if(i == 0)
    {
        return 1;
    }
    size_t bit1count = 0;   //��i��bitλΪ1��λ���м���
    size_t count = 0;       //��߲�Ϊ0��bitλ
    while(i != 0)
    {
        if( (i&1) !=0)
        {
            ++bit1count;
        }
        ++count;
        i >>=1;
    }
    return (bit1count == 1)? (1<<(count-1)):(1<<count);
}



