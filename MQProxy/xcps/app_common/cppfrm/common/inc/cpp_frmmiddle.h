/************************************************************************************
�ļ���  :cpp_frmmiddle.h

�ļ�����:

����    :zzt

��������:2005/10/20

�޸ļ�¼:

************************************************************************************/

#ifndef __CPP_FRMMIDDLE_H_
#define __CPP_FRMMIDDLE_H_

#include "cpp_adapter.h"
#include "cpp_common.h"
#include "cpp_tcn_vector.h"


enum EObjState
{
    OBJ_SLEEP,
    OBJ_ACTIVE,
    OBJ_ERROR = -1
};



/*********************************************************************
���� : class CChunkMgr
ְ�� :
       ������ͷ��ڴ��.�����ڴ�ʱ����Ĵ�С,Ϊ��
       �����ͷ�,����m_ChunkBySize�а�uiSize����������
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CChunkMgr
{
    typedef tcn::vector<CChunk*> TVctPCChunk;

public:
    //���ڽ�ֹ�˿��������Ա����ṩһ��Ĭ�Ϲ���
    CChunkMgr()
    {
    }

    ~CChunkMgr()
    {
        for(XU32 i = 0; i<m_ChunkBySize.size(); ++i)
        {
            delete m_ChunkBySize[i];
            m_ChunkBySize[i]   = XNULL;
            m_ChunkByStart[i]  = XNULL;
        }
    }
    //Ѱ�ҵ�һ�����������Ļ�������
    XVOID * Allocate(XU32 uiSize)
    {
        //Ϊ�˺�C++��operator new����Ϊһ��,��������СΪ0���ڴ�
        if(uiSize == 0)
        {
            ++uiSize;
        }

        //i������==-1
        for(XS32 i = findbySize(uiSize); i < (XS32)m_ChunkBySize.size(); ++i)
        {
            //static_cast<XU32>(i) д����Ϊ������PC-LINT�澯
            XVOID *ptmp = m_ChunkBySize[static_cast<XU32>(i)]->Allocate();
            if(ptmp != XNULL)
            {
                return ptmp;
            }
        }
        return XNULL;
    }
    EerrNo  Deallocate(XVOID *p)
    {
        XS32  i = findbyLeftAddr((XU8 *)p);
        if(i < 0)
        {
            return ERR_MEMORY;
        }
        //static_cast<XU32>(i) д����Ϊ������PC-LINT�澯
        m_ChunkByStart[static_cast<XU32>(i)]->Deallocate(p);
        return ERR_OK;
    }
    //�����Դ�С������˳���򻺳�������ӻ���,
    //������Ҫ�޸ģ���
    EerrNo AddChunk(XU32 uiSize, XU16 wCount)
    {
        if(GetMaxBlkSize() >= uiSize || wCount == 0 )
        {
            return ERR_INVALID_PARAM;
        }
        CChunk *ptmp = new  CChunk(uiSize,wCount);
        m_ChunkBySize.push_back(ptmp);
        //��ӵ����������������е�VECTOR��
        AddByStar(ptmp);
        return ERR_OK;
    }
    //�ж�p��ָ����ڴ��Ƿ�Ϊ���������е�
    bool InChunkMgr(XVOID *p )const
    {
        return (findbyLeftAddr((XU8 *)p) >= 0) ;
    }

public:
    //���Ŀ��С
    XU32 GetMaxBlkSize()const
    {
        return (m_ChunkBySize.size() == 0? 0:m_ChunkBySize.back()->GetBlockSize());
    }
    //��С�Ŀ��С
    XU32 GetMixBlkSize()const
    {
        return (m_ChunkBySize.size() == 0? 0:m_ChunkBySize.front()->GetBlockSize());
    }

private:
    FORBID_COPY_ASSIGN(CChunkMgr);

private:
    //m_ChunkByStart��Chunk������������������,���ò�������
    XVOID AddByStar(CChunk* pChunk);
    //Ѱ�ҵ�һ��GetBlockSize>=uiSize��CChunk����
    XS32  findbySize(XU32 uiSize)const;
    //Ѱ��ucp �����Ķ���chunk
    XS32  findbyLeftAddr(XU8 *ucp)const;

private:
    TVctPCChunk     m_ChunkBySize;   //Chunk���󰴿�Ĵ�С��������
    TVctPCChunk     m_ChunkByStart;  //Chunk���󰴻�����ʼ��ַ������������
};

/*********************************************************************
���� : class IProcesserId
ְ�� : �������,������
       �������ʾ(ID)ֵ �Ͷ�����������������ֵ��ת��
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class IProcesserId
{
public:
    //������ֵ����һ��ID
    virtual  XU32 SpawnNewId(XS32 index) = 0;
    //��ID������������
    virtual  XS32 RevertToIndex(XU32 Id)const = 0;
    virtual  XU32 GetRandomBitNum();
    virtual ~IProcesserId()
    {
    }
};

/*********************************************************************
���� : class CRandId
ְ�� :
       �������ʾ(ID)ֵ �Ͷ�����������������ֵ��ת��
       ������ת����IDʱ,ÿ��IDֵ��һ��
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CRandomId:public IProcesserId
{
#define MAX_RANDOM_BITS  16
#define MAX_BITS         32

public:
    //������ΧΪ[1, 0X7FFFFFFF],uiMaxNum ID�ĸ��������ֵ
    CRandomId(XU32 uiMaxNum)
    {
        //��Ȼ�����ID,���������λ�����BITλ
        CPP_ASSERT_RN((uiMaxNum != 0)&&(uiMaxNum <= (XU32)0X7FFFFFFF));

        m_uiMaxNum    = uiMaxNum;
        //���������Ŀ�������BIT��λ��,�����BITλ���16��
        m_uiRndbitNum = MAX_RANDOM_BITS;
        uiMaxNum   >>= MAX_RANDOM_BITS;
        while(uiMaxNum)
        {
            --m_uiRndbitNum;
            uiMaxNum >>= 1;
        }
        //������Ա�֤m_uiRndbitNum> 0;

        m_uiRan = 0;
        //m_uiRan = reinterpret_cast<XU32>(this);
    }
    //������ֵ����һ��ID
    XU32 SpawnNewId(XS32 index);
    //��ID������������
    XS32  RevertToIndex(XU32 Id)const;
    XU32 GetRandomBitNum();

private:
    //�������BITλ��
    XU32 NoRndBitNum()const
    {
        return MAX_BITS-m_uiRndbitNum;
    }
    //ȡ�õ�λ�����BIT������
    XU32 NoRndMask()const
    {
        //ע����λ��������ȼ�����,һ�� Ҫ�ǵü�����
        return ((1<<NoRndBitNum())-1);
    }

    //ȡ�ø�λ�����BIT������
    XU32   RndMask()const
    {
        return ~NoRndMask();//ȡ�ø�λ����,���ǽ���λ����ȡ��
    }

private:
    XU32   m_uiRan;           //�����ʼֵ
    XU32   m_uiRndbitNum;     //��һ��32BIT����,��� �ɶ��ٸ�BIT�������,���BIT�ڸ�λ
    XU32   m_uiMaxNum;
};

/*********************************************************************
���� : class CNoRandId
ְ�� :
       �������ʾ(ID)ֵ �Ͷ�����������������ֵ��ת��
       ������ת����IDʱ,ÿ��IDֵһ��
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CNoRandomId:public IProcesserId
{
public:
    CNoRandomId(XU32 )
    {
    }
    //������ֵ����һ��ID
    XU32 SpawnNewId(XS32 index);
    //��ID������������
    XS32 RevertToIndex(XU32 Id)const;
};


/*********************************************************************
���� : template<class T> class TResourcePool
ְ�� : ��Դ�ء��ó��е���Դֻ�ܴ�������״̬1 ACTIVE,
                2��SLEEP̬
Э�� : ��T���붨�����³�Ա����void  SetId(XU32 id),XU32 GetId()const
��ʷ :
       �޸���   ����          ����
**********************************************************************/


template<class T> class TResourcePool
{
#define ACTIVE_INDEX -1//�Ѿ�������Ľڵ��m_iNextFreeIndexֵ
    struct  Node
    {
        XS32 m_iNextFreeIndex;//m_buffer�е����еĿ��е�NOde����һ��,æ��Node�ĸ��ֶ���ACTIVE_INDEX
        XS32 m_iPrevFreeIndex;//æ��Node�ĸ��ֶ���ACTIVE_INDEX
        T * m_resource_data;
        //���������
        Node(T* pdata = XNULL)
        {
            m_resource_data = pdata;
            SetActive();
        }
        //ע��,��Ҫ����Node����������,����������������
        //��һ�����������������е���delete m_resource_data;��Ϊ
        //���ǵĴ����л����Node�͵���ʱ����,����m_data���ͷ�����
        bool IsActive()const
        {
            return m_iNextFreeIndex == ACTIVE_INDEX;
        }
        XVOID SetActive()
        {
            m_iNextFreeIndex = ACTIVE_INDEX;
            m_iPrevFreeIndex = ACTIVE_INDEX;
        }

        XVOID SetNext(XS32 index)
        {
            m_iNextFreeIndex = index;
        }
        XS32 GetNext()const
        {
            return m_iNextFreeIndex;
        }
        XVOID SetPrev(XS32 index)
        {
            m_iPrevFreeIndex = index;
        }
        XS32 GetPrev()const
        {
            return m_iPrevFreeIndex;
        }
        T* GetData()const
        {
            return m_resource_data;
        }
    };

public:
    #define FREE_HEAD_INDEX 0 //m_buffer�п��������ͷ�ڵ������
    typedef tcn::vector<Node> TVctNode;//������������

    TResourcePool(XU32 uiMax,bool isRandId);
    ~TResourcePool();
    //�����ṩһ�����������ĵ�����(��ʱ��ʵ��)

    //��pObj������Դ���еĿ���������,��Щ�������ӽ�ȥ��ʱ���Ǵ���OBJ_SLEEP̬.
    EerrNo Input(T *pObj) ;

    //������Դ���е�һ������.�����ҵ���Դ���е�һ������OBJ_SLEEP
    //̬�Ķ���,����������ID,������״̬�ĳ�OBJ_ACTIVE.
    T *Active();
    //ͨ��ID�Ĳ���
    T *Active(XU32 id);  //ͨ��ID����һ����Դ,���ı䱻������Դ��ID

    EObjState GetStateById(XU32 id)const;
    EerrNo DeActive(XU32 id);  //ͨ��ID ����Դ��ACTIVE̬�ı��SLEEP̬
    T *GetById(XU32 id)const;  //ͨ��ID���T��ָ�룬ע�⣬Tָ�����Դ״̬�п�����Sleep��
    //ID�Ƿ�Ϸ�,ֻ�е�������TRUEʱ����ʹ���������index
    bool IsValidId(XU32 id,XS32&  index)const;
    XS32 GetIndexFormId(XU32 id)const
    {
        return  m_pSpawnId->RevertToIndex(id);
    }
    //��Դ�Ŀ�ʼ��������
    XS32 GetBeginIndex()const
    {
        return FREE_HEAD_INDEX+1;
    }
    //������������ֵ
    XS32 GetEndIndex()const
    {
        return (XS32)GetCurSourceNum();
    }

    T * GetByIndex(XS32 index)const //ͨ���������T��ָ�룬���ж������ĺϷ���
    {
        return m_buffer[static_cast<XU32>(index)].GetData();
    }
    EObjState GetStateByIndex(XS32 index)const  //���ж������ĺϷ���
    {
        return (m_buffer[static_cast<XU32>(index)].IsActive() ? OBJ_ACTIVE:OBJ_SLEEP);
    }
    //������������Դ�Ƿ���ACTIVE̬
    bool IsActiveByIndex(XS32 index)const
    {
        CPP_ASSERT_RV(IsValidIndex(index), false);
        return m_buffer[index].IsActive();
    }
    bool IsActive(XS32 index)const   //�����ķ�Χ�Ƿ�Ϸ�
    {
        return m_buffer[index].IsActive();
    }
    bool IsValidIndex(XS32 index)const   //�����ķ�Χ�Ƿ�Ϸ�
    {
        return (index >= GetBeginIndex())&&(index <= GetEndIndex());
    }

    //�����Դ����Ŀ
    XU32 GetCurSourceNum()const
    {
        return m_buffer.size()-1;//ע�⣬����0������Դδ��
    }
    XU32 GetSourceCapacity()const
    {
        return m_buffer.capacity()-1;//ע�⣬����0������Դδ��
    }
    /*�����������*/
    //���ڵ�I���뵽����������
    XVOID AddToFree(XS32 iIndex)
    {
        //˫��������
        m_buffer[iIndex].SetNext( m_buffer[m_lastFreeIndex].GetNext());
        m_buffer[iIndex].SetPrev( m_lastFreeIndex);

        m_buffer[m_lastFreeIndex].SetNext(iIndex);

        m_lastFreeIndex = iIndex;
        // m_buffer[m_buffer[iIndex].GetNext()].SetPrev(iIndex);
    }
    XVOID RemoveFromFree(XS32 index)
    {
        //�����ǿ��������е�
        CPP_ASSERT_RN(!IsActiveByIndex(index));
        //����ɾ������
        if ((XU32)index == m_lastFreeIndex) // �Ƴ����һ���ڵ�,��Ҫ�޸����һ���ڵ�
        {
            m_lastFreeIndex = m_buffer[m_lastFreeIndex].GetPrev();
        }

        XS32 iprev = m_buffer[index].GetPrev();
        XS32 inext = m_buffer[index].GetNext();
        m_buffer[iprev].SetNext(inext);
        m_buffer[inext].SetPrev(iprev);
        //����Ϊ�̬
        m_buffer[index].SetActive();
    }

public:
    XVOID ClrCounter()
    {
        m_oprCounter.Clear();
    }
    //��ӡ����Դ�ص���Ϣ
    XVOID Print(CLI_ENV* pCliEnv);
    //��ӡ��Դ����ָ����ԴID����Ϣ
    XVOID PrintItem(XU32 id, CLI_ENV* pCliEnv);
    //��ӡ���нڵ����Ϣ
    XVOID PrintLastFree(XU32 cnt, CLI_ENV* pCliEnv);
    //Print last free resource node's information
    XVOID PrintLastFree(XU32 id, XU32 cnt, CLI_ENV* pCliEnv);
    XVOID PrintItems(XU32 startItem, XU32 cnt, CLI_ENV* pCliEnv);       //Print several resource node's information

private:
    FORBID_COPY_ASSIGN(TResourcePool);//��ֹ�����͸�ֵ

private:
    TVctNode          m_buffer;        //���п��нڵ����һ��������������,�����ͷ�ڵ�Ϊm_buffer[0]
    IProcesserId*     m_pSpawnId;      //��������ڵ�ID�Ĳ��Զ���,
    XU32              m_lastFreeIndex; //����ͷŵ�һ���ڵ�

private:
    CHistoryCount<XU32> m_oprCounter;
};

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T>  TResourcePool<T>::TResourcePool(XU32 uiMax,bool isRandId)
    :m_pSpawnId(XNULL),m_oprCounter(uiMax)
{
    CPP_ASSERT_RN(uiMax>0);
    if(isRandId)
    {
        m_pSpawnId = new CRandomId(uiMax);
    }
    else
    {
        m_pSpawnId = new CNoRandomId(uiMax);
    }

    //Ϊm_buffer����uiMax+1��С���ڴ�,+1����Ϊ��m_buffer[0]��Ϊ�˿�������ĸ���ͷ�ڵ�
    m_buffer.reserve(uiMax+1);
    //��m_buffer[0]��Ϊ���������ͷ�ڵ�
    m_buffer.push_back(typename TResourcePool::Node());
    m_buffer[FREE_HEAD_INDEX].SetNext(FREE_HEAD_INDEX);
    m_buffer[FREE_HEAD_INDEX].SetPrev(FREE_HEAD_INDEX);

    m_lastFreeIndex = FREE_HEAD_INDEX; // ��ʼ����,û������ͷ�
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> EerrNo TResourcePool<T>::Input(T *pObj)
{
    CPP_ASSERT_RV(pObj != XNULL, ERR_INITIAL_FAIL);
    if(m_buffer.size()  == m_buffer.capacity())
    {
        return ERR_MEMORY;
    }
    m_buffer.push_back(typename TResourcePool::Node(pObj));
    XU32 index = m_buffer.size()-1;//�ձ���ӵĽڵ������,������1��ʼ
    AddToFree(index);
    pObj->SetId(index);//��������ID
    return ERR_OK;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> T* TResourcePool<T>::Active()
{
    XS32 index = m_buffer[FREE_HEAD_INDEX].GetNext();
    //���Ϸ���������ֱ�ӷ���
    if( !IsValidIndex(index))
    {
        return XNULL;
    }
    RemoveFromFree(index);
    //�����ѷ���ļ���
    m_oprCounter.IncrementCount();
    //��������,�������ö����ID
    m_buffer[index].GetData()->SetId(m_pSpawnId->SpawnNewId(index));
    return m_buffer[index].GetData();
}
/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T>  T * TResourcePool<T>::Active(XU32 id)
{
    if(GetStateById(id) == OBJ_SLEEP)
    {
        m_oprCounter.IncrementCount();//���ѷ���ĸ�����1
        XS32 index = GetIndexFormId(id);
        RemoveFromFree(index);
        return m_buffer[index].GetData();
    }
    return XNULL;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> EerrNo  TResourcePool<T>::DeActive(XU32 id)
{
    if(GetStateById(id) == OBJ_ACTIVE)
    {
        m_oprCounter.DecrementCount();//�ͷ���һ��
        AddToFree(GetIndexFormId(id));
        return ERR_OK;
    }
    return ERR_INVALID_OPR;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> bool TResourcePool<T>::IsValidId(XU32 id,XS32& index)const
{
    //ͨ��ID�������
    index = GetIndexFormId(id);
    //�ж������Ƿ��Ƿ�Ϸ�
    if(IsValidIndex(index))
    {
        //�ж϶�Ӧλ���ϵ�ID������е�IDһ��
        return  m_buffer[index].GetData()->GetId() == id;
    }
    return false;
}

/*********************************************************************
�������� :
�������� : ͨ��ID���T��ָ�룬ע�⣬Tָ�����Դ״̬�п�����Sleep��

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> T *TResourcePool<T>::GetById(XU32 id)const
{
    XS32 index ;//���ﲻ��ʼ��
    //�ǺϷ�������
    if(IsValidId(id,index))
    {
        return m_buffer[index].GetData();
    }
    return XNULL;
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> EObjState TResourcePool<T>::GetStateById(XU32 id)const
{
    XS32 index ;//���ﲻ��ʼ��
    //�ǺϷ�������
    if(IsValidId(id,index))
    {
        //�ǲ����õ�ID
        return (m_buffer[index].IsActive() ? OBJ_ACTIVE:OBJ_SLEEP);
    }
    return OBJ_ERROR;
}

/*********************************************************************
�������� :  TResourcePool����������
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> TResourcePool<T>::~TResourcePool()
{
    delete m_pSpawnId;
    m_pSpawnId = XNULL;
    for(XU32 i = 0; i < m_buffer.size(); ++i)
    {
        delete m_buffer[i].m_resource_data;
        m_buffer[i].m_resource_data = XNULL;
    }
}

/*********************************************************************
�������� :
�������� : ���Դ�ӡ��Ϣ�Ľӿ�

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> XVOID TResourcePool<T>::Print(CLI_ENV* pCliEnv)
{
    XOS_CliExtPrintf(pCliEnv, "Resource: MaxNum=%u, ActiveNum=%u, FreeNum=%u, PeakNum=%u\r\n",
                     GetCurSourceNum(),
                     m_oprCounter.GetAlloced(),
                     m_oprCounter.GetFreeCount(),
                     m_oprCounter.GetPeakValue());

    //XOS_CliExtPrintf(pCliEnv, "Resource: RandomBits=%d\n",
    //                   m_pSpawnId->GetRandomBitNum());
}

/*********************************************************************
�������� :
�������� : ��ӡĳ����Դ�ڵ����Ϣ

�������� : id         XU32          ��Ҫ��ӡ��״̬����ID
           mid        MDID          ����Դ��������ģ��ID��
           ePL        EPrintLevel   ��ӡ����
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> XVOID TResourcePool<T>::PrintItem(XU32 id, CLI_ENV* pCliEnv)
{
    T* p = GetById(id);
    if(XNULL == p )
    {
        XOS_CliExtPrintf(pCliEnv,"fsmId=%d fsm not found \r\n",id);
    }
    else
    {
        p->Print(pCliEnv);
    }
}

/*********************************************************************
�������� :
�������� : ��ӡ���нڵ����Ϣ

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> XVOID TResourcePool<T>::PrintLastFree(XU32 uCount, CLI_ENV* pCliEnv)
{
    CPP_ASSERT_RN(pCliEnv);
    XS32 index = m_lastFreeIndex;

    for(XU32 i = 0; IsValidIndex(index)&&i < uCount; index = m_buffer[index].GetPrev(), i++)
    {
        if (!m_buffer[index].IsActive())
        {
            m_buffer[index].GetData()->Print(pCliEnv);
        }
        else
        {
            XOS_CliExtPrintf(pCliEnv,"Index = %d fsm is Active\r\n", index);
        }
    }
}

/*********************************************************************
�������� :
�������� : //Print last free resource node's information

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> XVOID TResourcePool<T>::PrintLastFree(XU32 id, XU32 uiCnt, CLI_ENV* pCliEnv)
{
    XS32 index ; //���ﲻ��ʼ��
    if( (uiCnt == 0)||(!IsValidId(id,index)) )
    {
        return ; //��ID�Ƿ���ֱ�ӷ���
    }
    XU32 i = 0;
    do
    {
        m_buffer[index].GetData()->Print(pCliEnv);
        index = m_buffer[index].GetNext();
    }
    while(IsValidIndex(index) && (++i < uiCnt));
}

/*********************************************************************
�������� :
�������� :  Print several resource node's information

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> XVOID TResourcePool<T>::PrintItems(XU32 startItem, XU32 cnt, CLI_ENV* pCliEnv)
{
    for (XU32 i = 0; i < cnt; i++)
    {
        T* p = GetById(startItem + i);
        if(XNULL != p)
        {
            p->Print(pCliEnv);
        }
    }
}


/*********************************************************************
���� : class CFastIndexPool
ְ�� :
       ��[0,n-1]��������������Ϊһ����Դ��,���е�������
       ���Դ�������״̬:�ѷ���̬�Ϳ���̬.
       ���������ط�����ͷ�һ����Դ���ٶȽϿ죬�������Ľ϶��
       �ڴ�.
       CCmprIndexPool��Դ�ؽϽ�ʡ�ڴ棬���Ƿ�����ͷ�һ������
       ���ٶȽ���
Э�� :
��ʷ :
       �޸���   ����          ����
**********************************************************************/
class CFastIndexPool
{
#define INVILIDE_INDEX  0xffff
#define BLOCKSIZE       sizeof(XU16)

public:
    CFastIndexPool(XU16 usMaxIndex)
        :m_pIndexPool(XNULL)
    {
        //Ԫ�صĸ�����CCHUNK_INVILIDE_INDEX��һ��,�����65534
        CPP_ASSERT_RN( usMaxIndex < (XU16)INVILIDE_INDEX);
        m_pIndexPool = new  CChunk (BLOCKSIZE,usMaxIndex);
    }
    ~CFastIndexPool()
    {
        delete m_pIndexPool;
        m_pIndexPool = XNULL;
    }
    //������һ�����е�����
    XU16 AllocIndex()
    {
        if(m_pIndexPool->IsEmpty())
        {
            return INVILIDE_INDEX;//�յľ�ֱ�ӷ���һ���Ƿ�ֵ
        }
        XU16 curIndex =  static_cast<XU16>(m_pIndexPool->m_usFrIdx);
        //�ѷ����ȥ���ڴ��д��һ���Ƿ�ֵ,���ͷ�IDʱ�������
        *((XU16*) m_pIndexPool->Allocate()) = INVILIDE_INDEX;
        return curIndex;
    }
    //�ͷųɹ�����TRUE,�ͷ�ʧ�ܷ���FALSE
    bool  DeallocIndex(XU16 index)
    {
        //��������ΧԽ�磬�򷵻�FALSE�����ߣ��ͷ���һ����δ�����������Ҳ����FALSE
        if((index >= m_pIndexPool->BlkCount()) || (m_pIndexPool->NextIndex(index) != INVILIDE_INDEX) )
        {
            return false;
        }
        m_pIndexPool->AddToFree(index);
        return true;
    }
    //�Ƿ�Ϊ�Ϸ�������,�Ϸ�
    bool IsValidIndex(XU16 index)const
    {
        return index < m_pIndexPool->BlkCount();
    }
    //��Դ���Ƿ��Ѿ�Ϊ��.Ϊ�վ���û�п��е�������
    bool IsEmpty()const
    {
        return m_pIndexPool->IsEmpty();
    }

private:
    //��ֹ�����͸�ֵ
    FORBID_COPY_ASSIGN(CFastIndexPool);

private:
    CChunk * m_pIndexPool;
};



#endif

