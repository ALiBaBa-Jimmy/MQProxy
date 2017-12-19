/****************************************************************************
�ļ���  :cpp_prealloc.h

�ļ�����:

����    :yuxiao

��������:2006/2/21

�޸ļ�¼:
*************************************************************************/

#ifndef __CPP_PREALLOC_H_
#define __CPP_PREALLOC_H_

#include "cpp_tcn_adapter.h"


/*lint -e1512*/
/*********************************************************************
���� : class CHistoryCount
ְ�� : ��¼Ԥ�������ʷ��Ϣ
Э�� :
��ʷ :
       �޸���   ����          ����
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
    T   m_tMaxNum;     //������
    T   m_tAlloced;    //��ǰ�ѱ�����ĸ���
    T   m_tPeakValue;  //����ķ�ֵ
};

/*********************************************************************
���� : class CChunk
ְ�� : ����dwCount����СΪdwSize���ڴ��Ҫ��dwCount<=65535
       �����ֽڶ����Ҫ��,��dwSize����2�ı���,������
       ��һ���ֽڣ�ʹ���Ϊ2�ı���
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
typedef CHistoryCount<size_t> CAllocRecord; //��¼һ��Chunk���ڴ�������ʷ��Ϣ

class CChunk
{
    friend class CFastIndexPool;

public:
    CChunk(XU32 uiSize,size_t usCount);
    ~CChunk()
    {
        Clear();
    }
    //�����͸�ֵ
    CChunk(const CChunk& other);

    // �����ڴ�Ԥ����ʱ,���ṩ����
    // ���뱣֤reserveʱ, chunk��û���κ��ڴ汻ռ��
    // add by liusj
    CChunk(XU32 uiSize);
    XVOID reserve(size_t n);


    CChunk& operator =(const CChunk& other);
    //��������CChunk�����ݣ�
    XVOID swap(CChunk& other);

    //�Ƿ��Ѿ�û�п�����Դ��
    bool IsEmpty()const
    {
        return m_usFrIdx == BlkCount();
    }
    //����һ���ڴ��
    XVOID * Allocate()
    {
        //�Ѿ�û�п��п���
        if(IsEmpty())
        {
            return XNULL;
        }
        //��Ӽ�����Ϣ
        m_rec.IncrementCount();

        size_t curIndex = m_usFrIdx;
        m_usFrIdx = NextIndex(m_usFrIdx);

        if (IsEmpty())
        {
            // ���������һ���ڵ��,last����first
            m_usLastIdx = m_usFrIdx;
        }

        return &NextIndex(curIndex);
    }
    //�ͷ�һ���ڴ������Ǳ��������зǿ��е��ڴ���,
    XVOID Deallocate(XVOID *p)
    {
        CPP_ASSERT_RN(InChunk(p));
        AddToFree(GetIndexByAddr(p));
        m_rec.DecrementCount();
    }
    //����ڴ�����ʼ��ַ
    XU8 * GetStartAddr()const
    {
        return m_pchStart;
    }
    XU32 GetBlockSize()const
    {
        return m_uiSize;
    }
    //�жϵ�ַ�Ƿ�Ϊ�û�������
    bool InChunk(const XVOID *pbyAddr)const
    {
        CPP_ASSERT_RV(pbyAddr != XNULL, false);
        return ((XU8*)pbyAddr>= m_pchStart) && ((XU8*)pbyAddr < GetBufEnd());
    }
    //���п�
    size_t BlkCount()const
    {
        return m_rec.GetMaxNum();
    }
    //�������Ľ�����ַ.�����һ����Ч��ַ����һ��ַ
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
        m_pchStart = XNULL;//����PC-LINT�澯
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
    //��õ�I���ڵ���ָ�����һ���ڵ�
    size_t&  NextIndex(size_t i)const
    {
        return *((size_t*)(m_pchStart+i*m_uiSize));
    }
    //���p��ָ����ڴ���ڻ������е�����,
    //����p��ָ����ڴ����Ƿ�Ϊ����������У��
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
    XU8*         m_pchStart;     //����Ŀ�ʼ��ַ
    XU32         m_uiSize;       //�ڴ��Ĵ�С
    size_t       m_usFrIdx;      //������������
    size_t       m_usLastIdx;    //��������β����

private:
    CAllocRecord m_rec;          //��ʷ��¼
};


/*********************************************************************
���� : CPreAllocator
ְ�� : �ڴ�Ԥ�������������
       ��STL�ı�׼,��׼�������е�������ְ��һ���Ƿ����ڴ棬һ���ǹ������������
       ���������������ֻ�е����ڴ����õĹ��ܣ�������Ĺ���������������Լ����
Э�� :
��ʷ :
       �޸���   ����          ����
*********************************************************************/
class CPreAllocator
{
public:
    //���캯��,�����ʹ�С
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

        //�ڴ����ʧ�ܺ󣬴�ӡ��Դ�Ĵ�С���׵�ַ�������ж�,2010.3.23
        if( XNULL == ptr )
        {
            PrintInfo(PA(CPPFRM_MODULE_ID,PL_EXP),"chunk size=%d, max_num=%d, begin addr=%d\r\n",
                      m_Chunk.GetBlockSize(), max_size(), m_Chunk.GetStartAddr());
        }

        //CPP_ASSERT(XNULL != ptr); // ����Ԥ����ռ䣬�ռ�Ԥ��������
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
���� : CAllocator
ְ�� : ��ͨ���ڴ��������Ľӿ�
Э�� :
��ʷ :
       �޸���   ����          ����
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
    //�ýӿڿ��ܱ�������������N��T���ʹ�С���ڴ�ռ䣬�����ö���
    XVOID* allocate(size_t n = 1)
    {
        CPP_ASSERT_RV(n>0, XNULL);
        return Allocate(n*m_blkSize);
    }
    //�ͷ��ͷ��ڴ�ռ�
    XVOID deallocate(XVOID  *p)
    {
        Dealloctor(p);
    }
    //����ʲô������
    XVOID swap(CAllocator& other)
    {

    }

    XVOID reserve(size_t n)
    {
        //��Ԥ����,ֱ�ӷ���
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

//����hashTable,��Ҫ�����������͵��ڴ�
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

