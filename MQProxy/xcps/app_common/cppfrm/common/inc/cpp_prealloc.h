/****************************************************************************
文件名  :cpp_prealloc.h

文件描述:

作者    :yuxiao

创建日期:2006/2/21

修改记录:
*************************************************************************/

#ifndef __CPP_PREALLOC_H_
#define __CPP_PREALLOC_H_

#include "cpp_tcn_adapter.h"


/*lint -e1512*/
/*********************************************************************
名称 : class CHistoryCount
职责 : 记录预分配的历史信息
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
template<class T> class CHistoryCount
{
public:
    CHistoryCount(T tMaxNum)
        :m_tMaxNum(tMaxNum)
    {
        Clear();
    }
    bool operator == (const CHistoryCount& other)const
    {
        return (m_tMaxNum ==other.m_tMaxNum)&&(m_tAlloced ==other.m_tAlloced)&&(m_tPeakValue ==other.m_tAlloced);
    }

    XVOID IncrementCount()
    {
        ++m_tAlloced;
        if(m_tAlloced > m_tPeakValue)
        {
            m_tPeakValue = m_tAlloced;
        }
    }
    XVOID DecrementCount()
    {
        --m_tAlloced;
    }
    T GetMaxNum()const
    {
        return m_tMaxNum;
    }
    XVOID ResetMaxNum(T MaxNum)
    {
        m_tMaxNum = MaxNum;
    }

    T GetAlloced()const
    {
        return m_tAlloced;
    }
    T GetFreeCount()const
    {
        return m_tMaxNum - m_tAlloced;
    }

    T GetPeakValue()const
    {
        return m_tPeakValue;
    }
    XVOID Clear()
    {
        m_tAlloced    =0;
        m_tPeakValue  =0;
    }

    XVOID print()const
    {
        //PrintInfo("max num =%d,allocated num = %d,peakValue = %d \n",m_tMaxNum,m_tAlloced,m_tPeakValue);
    }

private:
    T   m_tMaxNum;     //最大个数
    T   m_tAlloced;    //当前已被分配的个数
    T   m_tPeakValue;  //分配的峰值
};

/*********************************************************************
名称 : class CChunk
职责 : 管理dwCount个大小为dwSize的内存块要求dwCount<=65535
       由于字节对齐的要求,若dwSize不是2的倍数,则将其增
       大一个字节，使其成为2的倍数
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
typedef CHistoryCount<size_t> CAllocRecord; //记录一个Chunk里内存分配的历史信息

class CChunk
{
    friend class CFastIndexPool;

public:
    CChunk(XU32 uiSize,size_t usCount);
    ~CChunk()
    {
        Clear();
    }
    //拷贝和赋值
    CChunk(const CChunk& other);

    // 用于内存预分配时,不提供个数
    // 必须保证reserve时, chunk中没有任何内存被占用
    // add by liusj
    CChunk(XU32 uiSize);
    XVOID reserve(size_t n);


    CChunk& operator =(const CChunk& other);
    //交换两个CChunk的内容，
    XVOID swap(CChunk& other);

    //是否已经没有空闲资源了
    bool IsEmpty()const
    {
        return m_usFrIdx == BlkCount();
    }
    //分配一个内存块
    XVOID * Allocate()
    {
        //已经没有空闲块了
        if(IsEmpty())
        {
            return XNULL;
        }
        //添加计数信息
        m_rec.IncrementCount();

        size_t curIndex = m_usFrIdx;
        m_usFrIdx = NextIndex(m_usFrIdx);

        if (IsEmpty())
        {
            // 分配完最后一个节点后,last等于first
            m_usLastIdx = m_usFrIdx;
        }

        return &NextIndex(curIndex);
    }
    //释放一个内存块必须是本缓冲区中非空闲的内存块的,
    XVOID Deallocate(XVOID *p)
    {
        CPP_ASSERT_RN(InChunk(p));
        AddToFree(GetIndexByAddr(p));
        m_rec.DecrementCount();
    }
    //获得内存块的起始地址
    XU8 * GetStartAddr()const
    {
        return m_pchStart;
    }
    XU32 GetBlockSize()const
    {
        return m_uiSize;
    }
    //判断地址是否为该缓冲区的
    bool InChunk(const XVOID *pbyAddr)const
    {
        CPP_ASSERT_RV(pbyAddr != XNULL, false);
        return ((XU8*)pbyAddr>= m_pchStart) && ((XU8*)pbyAddr < GetBufEnd());
    }
    //所有块
    size_t BlkCount()const
    {
        return m_rec.GetMaxNum();
    }
    //缓冲区的结束地址.是最后一个有效地址的下一地址
    XU8 *  GetBufEnd()const
    {
        return (m_pchStart+m_uiSize*BlkCount());
    }
    const CAllocRecord& GetRec()const
    {
        return m_rec;
    }

private:
    XVOID Init(XU32 uiSize,size_t usCount);
    XVOID Clear()
    {
        m_rec.Clear();
        delete [] m_pchStart;
        m_pchStart = XNULL;//屏蔽PC-LINT告警
    }

    XVOID AddToFree(size_t usIndex )
    {
        // NextIndex(usIndex) = m_usFrIdx;
        // m_usFrIdx = usIndex;

        if (IsEmpty())
        {
            m_usFrIdx = usIndex;
            NextIndex(usIndex) = m_usLastIdx;
            m_usLastIdx = usIndex;
        }
        else
        {
            NextIndex(usIndex) = NextIndex(m_usLastIdx);
            NextIndex(m_usLastIdx) = usIndex;
            m_usLastIdx = usIndex;
        }
    }
    //获得第I个节点所指向的下一个节点
    size_t&  NextIndex(size_t i)const
    {
        return *((size_t*)(m_pchStart+i*m_uiSize));
    }
    //获得p所指向的内存块在缓冲区中的索引,
    //不对p所指向的内存做是否为本缓冲区做校验
    size_t  GetIndexByAddr(XVOID *p)const
    {
        //SS_ASSERT(InChunk(p));
        return ((XU32)((XU8*)p-m_pchStart))/m_uiSize;
    }
    XVOID print()const
    {
        m_rec.print();
        PrintInfo(PA(CPPFRM_MODULE_ID,PL_ERR),"chunk free index=%d, size=%d, begin addr=%d\r\n",
                  m_usFrIdx, m_uiSize, m_pchStart);
    }

private:
    XU8*         m_pchStart;     //缓存的开始地址
    XU32         m_uiSize;       //内存块的大小
    size_t       m_usFrIdx;      //空闲链的索引
    size_t       m_usLastIdx;    //空闲链的尾索引

private:
    CAllocRecord m_rec;          //历史记录
};


/*********************************************************************
名称 : CPreAllocator
职责 : 内存预分配的配置器。
       按STL的标准,标准配置器承担的两个职责，一个是分配内存，一个是构造和析构对象
       我们这里的配置器只承担了内存配置的功能，将对象的构造和析构由容器自己完成
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
class CPreAllocator
{
public:
    //构造函数,个数和大小
    CPreAllocator(size_t count,size_t size)
        :m_Chunk(size,count)
    {

    }

    CPreAllocator(size_t size)
        :m_Chunk(size, 1)
    {

    }

    XVOID reserve(size_t n)
    {
        m_Chunk.reserve(n);
    }

    XVOID* allocate()
    {
        XVOID* ptr = m_Chunk.Allocate();

        //内存分配失败后，打印资源的大小和首地址，便于判断,2010.3.23
        if( XNULL == ptr )
        {
            PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"chunk size=%d, max_num=%d, begin addr=%d\r\n",
                      m_Chunk.GetBlockSize(), max_size(), m_Chunk.GetStartAddr());
        }

        //CPP_ASSERT(XNULL != ptr); // 超出预分配空间，空间预算有问题
        return ptr;
    }
    XVOID deallocate(XVOID  *p)
    {
        if(0 != p)
        {
            m_Chunk.Deallocate(p);
        }
    }

    size_t max_size() const
    {
        return m_Chunk.BlkCount();
    }
    XVOID swap(CPreAllocator& other)
    {
        m_Chunk.swap(other.m_Chunk);
    }

    bool operator == (const CPreAllocator& other)const
    {
        return false;
    }
    bool operator != (const CPreAllocator& other)const
    {
        return true;
    }

private:
    CChunk m_Chunk;
};


/*********************************************************************
名称 : CAllocator
职责 : 普通的内存配置器的接口
协作 :
历史 :
       修改者   日期          描述
*********************************************************************/
class CAllocator
{
public:
    CAllocator(size_t size)
        :m_blkSize(size)
    {
    }

    CAllocator& operator = (const CAllocator& other)
    {
        if(this != &other)
        {
            //CPP_ASSERT(other.m_blkSize == m_blkSize);
            m_blkSize = other.m_blkSize;
        }
        return *this;
    }
    //该接口可能被用来分配连续N个T类型大小的内存空间，所以用断言
    XVOID* allocate(size_t n = 1)
    {
        CPP_ASSERT_RV(n>0, XNULL);
        return Allocate(n*m_blkSize);
    }
    //释放释放内存空间
    XVOID deallocate(XVOID  *p)
    {
        Dealloctor(p);
    }
    //交换什么都不作
    XVOID swap(CAllocator& other)
    {

    }

    XVOID reserve(size_t n)
    {
        //非预分配,直接返回
    }

    size_t max_size() const
    {
        size_t n = (size_t)(-1) / m_blkSize;
        return (0 < n ? n : 1);
    }
    bool operator == (const CAllocator& other)const
    {
        return true;
    }
    bool operator != (const CAllocator& other)const
    {
        return false;
    }

private:
    size_t m_blkSize;
};

//对于hashTable,需要配置两种类型的内存
class CByteAllocator
{
public:
    XVOID*ByteAlloc(size_t size)
    {
        return Allocate(size);
    }
    XVOID ByteDealloc(XVOID*pBuf)
    {
        Dealloctor(pBuf);
    }
};

class CPreBANAllocator:public CPreAllocator,public CByteAllocator
{
public:
    CPreBANAllocator(size_t count,size_t size)
        :CPreAllocator(count,size)
    {
    }

    CPreBANAllocator(size_t size)
        :CPreAllocator(size)
    {
    }

};

class CBANAllocator:public CAllocator,public CByteAllocator
{
public:
    CBANAllocator(size_t size)
        :CAllocator(size)
    {
    }
};


#endif

