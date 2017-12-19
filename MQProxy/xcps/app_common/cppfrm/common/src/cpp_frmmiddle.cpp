/************************************************************************************
文件名  :cpp_frmmiddle.cpp

文件描述:

作者    :zzt

创建日期:2005/10/20

修改记录:

************************************************************************************/
#include "cpp_frmmiddle.h"


/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CChunk::CChunk(XU32 uiSize,size_t usCount)
    :m_rec(usCount)
{
    Init( uiSize,usCount);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CChunk::CChunk(XU32 uiSize) : m_rec(0)
{
    //CPP_ASSERT_RN(uiSize > 0);

#ifdef XOS_ARCH_64
    uiSize      = ((uiSize+7)>>3)<<3; //若uiSize不是8的倍数，则将之向上圆整为8的倍数
#else
    uiSize      = ((uiSize+3)>>2)<<2; //若uiSize不是4的倍数，则将之向上圆整为4的倍数
#endif

    m_uiSize    = uiSize;
    m_pchStart  = XNULL;
    m_usFrIdx   = 0;
    m_usLastIdx = m_usFrIdx;
}

/*********************************************************************
函数名称 : CChunk
功能描述 : 拷贝函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CChunk::CChunk(const CChunk& other)
    :m_rec(other.m_rec.GetMaxNum())
{
    Init(other.m_uiSize,m_rec.GetMaxNum());
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CChunk::reserve(size_t n)
{
    // 只允许在初始化阶段调用该函数
    CPP_ASSERT_RN(0 == GetRec().GetAlloced()); // 没有使用空闲块

    XU32 savSize = m_uiSize;
    Clear();
    m_rec.ResetMaxNum(n);
    Init( savSize, n);
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID CChunk::Init(XU32 uiSize,size_t usCount)
{
    CPP_ASSERT_RN(usCount>0 && uiSize >0);

#ifdef XOS_ARCH_64
    uiSize      = ((uiSize+7)>>3)<<3; //若uiSize不是8的倍数，则将之向上圆整为8的倍数
#else
    uiSize      = ((uiSize+3)>>2)<<2; //若uiSize不是4的倍数，则将之向上圆整为4的倍数
#endif

    m_uiSize    = uiSize;
    m_pchStart  = new XU8[m_uiSize * BlkCount()];

    //将空闲缓冲区成链,最后一个空闲链的NEXT指针域的值为m_usCount
    for(size_t i = 0; i < BlkCount(); ++i)
    {
        NextIndex(i)=i+1;
    }

    m_usFrIdx   = 0;
    m_usLastIdx = BlkCount() - 1;
}

/*********************************************************************
函数名称 :  CChunk
功能描述 : 赋值函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CChunk& CChunk::operator=(const CChunk& other)
{
    if(this != &other)
    {
        Clear();
        //重新
        m_rec.ResetMaxNum(other.m_rec.GetMaxNum());
        Init(other.m_uiSize,m_rec.GetMaxNum());
    }
    return *this;
}

/*********************************************************************
函数名称 :  CChunk
功能描述 : 赋值函数

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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
函数名称 :  AddByStar
功能描述 :  m_ChunkByStart中Chunk对象按左区间升序排列,采用插入排序

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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
函数名称 :  findbySize
功能描述 :   寻找第一个GetBlockSize>=uiSize的CChunk对象

参数输入 :  uiSize      XU32        内存块大小
参数输出 :
返回值   :  XS32         CChunk对象指针的索引,未找到则返回m_ChunkBySize.size()

修改历史 : Author        mm/dd/yy       Initial Writing
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
函数名称 : findbyLeftAddr
功能描述 :   寻找ucp 所属的对象chunk.折半查找

参数输入 :  ucp          XU8 *         内存块的起始地址
参数输出 :
返回值   :  XS32         CChunk对象指针的索引,未找到则返回-1

修改历史 : Author        mm/dd/yy       Initial Writing
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
    //在区间之外1
    if(iRight>=0 && m_ChunkByStart[(XU32)iRight]->GetBufEnd()<= ucp )
    {
        iRight = -1;
    }
    return iRight;
}

/*********************************************************************
函数名称 :GetRandomBitNum
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XU32 IProcesserId::GetRandomBitNum()
{
    return 0;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 : index      XU32      数组中的索引,索引必须从1开始
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XU32 CRandomId::SpawnNewId(XS32 index)
{
    CPP_ASSERT_RV((index > 0)&&(index <= static_cast<XS32>(m_uiMaxNum)), 0XFFFFFFFF);

    //保证每次分配的值不一样
    ++m_uiRan;

    // 将最高位置零，
    return ((m_uiRan << NoRndBitNum())|(static_cast<XU32>(index) & NoRndMask())) & 0X7FFFFFFF;
}

/*********************************************************************
函数名称 : RevertToIndex
功能描述 :   由ID 计算出索引

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS32 CRandomId::RevertToIndex(XU32 Id)const
{
    return static_cast<XS32>(Id & NoRndMask());
}

/*********************************************************************
函数名称 :GetRandomBitNum
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XU32 CRandomId::GetRandomBitNum()
{
    return m_uiRndbitNum;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XU32 CNoRandomId::SpawnNewId(XS32 index)
{
    return (XU32)index;
}

/*********************************************************************
函数名称 :
功能描述 : 由ID逆向计算出索引

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS32 CNoRandomId::RevertToIndex(XU32 Id)const
{
    return (XS32)Id;
}

/*********************************************************************
函数名称 :
功能描述 : 该函数返回一个值,该值的大小是2的n次方，并且是第一个>=i
           目前给hastable使用

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
size_t next_power_two(size_t i)
{
    if(i == 0)
    {
        return 1;
    }
    size_t bit1count = 0;   //对i中bit位为1的位进行计数
    size_t count = 0;       //最高不为0的bit位
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



