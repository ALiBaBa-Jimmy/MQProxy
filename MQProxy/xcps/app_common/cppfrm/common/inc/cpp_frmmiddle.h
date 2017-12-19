/************************************************************************************
文件名  :cpp_frmmiddle.h

文件描述:

作者    :zzt

创建日期:2005/10/20

修改记录:

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
名称 : class CChunkMgr
职责 :
       分配和释放内存块.分配内存时按块的大小,为了
       快数释放,所以m_ChunkBySize中按uiSize的升序排列
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CChunkMgr
{
    typedef tcn::vector<CChunk*> TVctPCChunk;

public:
    //由于禁止了拷贝，所以必须提供一个默认构造
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
    //寻找第一个满足条件的缓冲区。
    XVOID * Allocate(XU32 uiSize)
    {
        //为了和C++的operator new的行为一致,允许分配大小为0的内存
        if(uiSize == 0)
        {
            ++uiSize;
        }

        //i不可能==-1
        for(XS32 i = findbySize(uiSize); i < (XS32)m_ChunkBySize.size(); ++i)
        {
            //static_cast<XU32>(i) 写法是为了屏蔽PC-LINT告警
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
        //static_cast<XU32>(i) 写法是为了屏蔽PC-LINT告警
        m_ChunkByStart[static_cast<XU32>(i)]->Deallocate(p);
        return ERR_OK;
    }
    //必须以大小递增的顺序向缓冲区中添加缓存,
    //这里需要修改，以
    EerrNo AddChunk(XU32 uiSize, XU16 wCount)
    {
        if(GetMaxBlkSize() >= uiSize || wCount == 0 )
        {
            return ERR_INVALID_PARAM;
        }
        CChunk *ptmp = new  CChunk(uiSize,wCount);
        m_ChunkBySize.push_back(ptmp);
        //添加到按左区间升序排列的VECTOR中
        AddByStar(ptmp);
        return ERR_OK;
    }
    //判断p所指向的内存是否为本缓冲区中的
    bool InChunkMgr(XVOID *p )const
    {
        return (findbyLeftAddr((XU8 *)p) >= 0) ;
    }

public:
    //最大的块大小
    XU32 GetMaxBlkSize()const
    {
        return (m_ChunkBySize.size() == 0? 0:m_ChunkBySize.back()->GetBlockSize());
    }
    //最小的块大小
    XU32 GetMixBlkSize()const
    {
        return (m_ChunkBySize.size() == 0? 0:m_ChunkBySize.front()->GetBlockSize());
    }

private:
    FORBID_COPY_ASSIGN(CChunkMgr);

private:
    //m_ChunkByStart中Chunk对象按左区间升序排列,采用插入排序
    XVOID AddByStar(CChunk* pChunk);
    //寻找第一个GetBlockSize>=uiSize的CChunk对象
    XS32  findbySize(XU32 uiSize)const;
    //寻找ucp 所属的对象chunk
    XS32  findbyLeftAddr(XU8 *ucp)const;

private:
    TVctPCChunk     m_ChunkBySize;   //Chunk对象按块的大小升序排列
    TVctPCChunk     m_ChunkByStart;  //Chunk对象按缓存起始地址升序升序排列
};

/*********************************************************************
名称 : class IProcesserId
职责 : 纯虚基类,派生类
       将对象标示(ID)值 和对象所在数组中索引值做转换
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class IProcesserId
{
public:
    //由索引值生成一个ID
    virtual  XU32 SpawnNewId(XS32 index) = 0;
    //由ID逆向计算出索引
    virtual  XS32 RevertToIndex(XU32 Id)const = 0;
    virtual  XU32 GetRandomBitNum();
    virtual ~IProcesserId()
    {
    }
};

/*********************************************************************
名称 : class CRandId
职责 :
       将对象标示(ID)值 和对象所在数组中索引值做转换
       将索引转换成ID时,每次ID值不一样
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CRandomId:public IProcesserId
{
#define MAX_RANDOM_BITS  16
#define MAX_BITS         32

public:
    //参数范围为[1, 0X7FFFFFFF],uiMaxNum ID的个数的最大值
    CRandomId(XU32 uiMaxNum)
    {
        //既然是随机ID,则至少最高位是随机BIT位
        CPP_ASSERT_RN((uiMaxNum != 0)&&(uiMaxNum <= (XU32)0X7FFFFFFF));

        m_uiMaxNum    = uiMaxNum;
        //根据最大数目决定随机BIT的位数,但随机BIT位最多16个
        m_uiRndbitNum = MAX_RANDOM_BITS;
        uiMaxNum   >>= MAX_RANDOM_BITS;
        while(uiMaxNum)
        {
            --m_uiRndbitNum;
            uiMaxNum >>= 1;
        }
        //这里可以保证m_uiRndbitNum> 0;

        m_uiRan = 0;
        //m_uiRan = reinterpret_cast<XU32>(this);
    }
    //由索引值生成一个ID
    XU32 SpawnNewId(XS32 index);
    //由ID逆向计算出索引
    XS32  RevertToIndex(XU32 Id)const;
    XU32 GetRandomBitNum();

private:
    //非随机的BIT位数
    XU32 NoRndBitNum()const
    {
        return MAX_BITS-m_uiRndbitNum;
    }
    //取得低位非随机BIT的掩码
    XU32 NoRndMask()const
    {
        //注意移位运算的优先级极低,一定 要记得加括号
        return ((1<<NoRndBitNum())-1);
    }

    //取得高位的随机BIT的掩码
    XU32   RndMask()const
    {
        return ~NoRndMask();//取得高位掩码,就是将地位掩码取反
    }

private:
    XU32   m_uiRan;           //随机起始值
    XU32   m_uiRndbitNum;     //在一个32BIT数中,随机 由多少个BIT是随机的,随机BIT在高位
    XU32   m_uiMaxNum;
};

/*********************************************************************
名称 : class CNoRandId
职责 :
       将对象标示(ID)值 和对象所在数组中索引值做转换
       将索引转换成ID时,每次ID值一样
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CNoRandomId:public IProcesserId
{
public:
    CNoRandomId(XU32 )
    {
    }
    //由索引值生成一个ID
    XU32 SpawnNewId(XS32 index);
    //由ID逆向计算出索引
    XS32 RevertToIndex(XU32 Id)const;
};


/*********************************************************************
名称 : template<class T> class TResourcePool
职责 : 资源池。该池中的资源只能处于两种状态1 ACTIVE,
                2是SLEEP态
协作 : 类T必须定义如下成员函数void  SetId(XU32 id),XU32 GetId()const
历史 :
       修改者   日期          描述
**********************************************************************/


template<class T> class TResourcePool
{
#define ACTIVE_INDEX -1//已经被激活的节点的m_iNextFreeIndex值
    struct  Node
    {
        XS32 m_iNextFreeIndex;//m_buffer中的所有的空闲的NOde连在一起,忙的Node的该字段填ACTIVE_INDEX
        XS32 m_iPrevFreeIndex;//忙的Node的该字段填ACTIVE_INDEX
        T * m_resource_data;
        //构造和析构
        Node(T* pdata = XNULL)
        {
            m_resource_data = pdata;
            SetActive();
        }
        //注意,不要定义Node的析构函数,若定义了析构函数
        //则一定不能在析构函数中调用delete m_resource_data;因为
        //我们的代码中会产生Node型的临时对象,导致m_data被释放两次
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
    #define FREE_HEAD_INDEX 0 //m_buffer中空闲链表的头节点的索引
    typedef tcn::vector<Node> TVctNode;//所用容器定义

    TResourcePool(XU32 uiMax,bool isRandId);
    ~TResourcePool();
    //考虑提供一个遍历活动对象的迭代器(暂时不实现)

    //将pObj加入资源池中的空闲链表中,这些对象刚添加进去的时候是处于OBJ_SLEEP态.
    EerrNo Input(T *pObj) ;

    //激活资源池中的一个对象.就是找到资源池中的一个处于OBJ_SLEEP
    //态的对象,重新设置其ID,并将其状态改成OBJ_ACTIVE.
    T *Active();
    //通过ID的操作
    T *Active(XU32 id);  //通过ID激活一个资源,不改变被激活资源的ID

    EObjState GetStateById(XU32 id)const;
    EerrNo DeActive(XU32 id);  //通过ID 将资源从ACTIVE态改变成SLEEP态
    T *GetById(XU32 id)const;  //通过ID获得T的指针，注意，T指向的资源状态有可能是Sleep的
    //ID是否合法,只有当它返回TRUE时才能使用输出参数index
    bool IsValidId(XU32 id,XS32&  index)const;
    XS32 GetIndexFormId(XU32 id)const
    {
        return  m_pSpawnId->RevertToIndex(id);
    }
    //资源的开始处的索引
    XS32 GetBeginIndex()const
    {
        return FREE_HEAD_INDEX+1;
    }
    //获得索引的最大值
    XS32 GetEndIndex()const
    {
        return (XS32)GetCurSourceNum();
    }

    T * GetByIndex(XS32 index)const //通过索引获得T的指针，不判断索引的合法性
    {
        return m_buffer[static_cast<XU32>(index)].GetData();
    }
    EObjState GetStateByIndex(XS32 index)const  //不判断索引的合法性
    {
        return (m_buffer[static_cast<XU32>(index)].IsActive() ? OBJ_ACTIVE:OBJ_SLEEP);
    }
    //该索引处的资源是否处于ACTIVE态
    bool IsActiveByIndex(XS32 index)const
    {
        CPP_ASSERT_RV(IsValidIndex(index), false);
        return m_buffer[index].IsActive();
    }
    bool IsActive(XS32 index)const   //索引的范围是否合法
    {
        return m_buffer[index].IsActive();
    }
    bool IsValidIndex(XS32 index)const   //索引的范围是否合法
    {
        return (index >= GetBeginIndex())&&(index <= GetEndIndex());
    }

    //最大资源的数目
    XU32 GetCurSourceNum()const
    {
        return m_buffer.size()-1;//注意，索引0处的资源未用
    }
    XU32 GetSourceCapacity()const
    {
        return m_buffer.capacity()-1;//注意，索引0处的资源未用
    }
    /*链表基本操作*/
    //将节点I加入到空闲链表中
    XVOID AddToFree(XS32 iIndex)
    {
        //双向环型链表
        m_buffer[iIndex].SetNext( m_buffer[m_lastFreeIndex].GetNext());
        m_buffer[iIndex].SetPrev( m_lastFreeIndex);

        m_buffer[m_lastFreeIndex].SetNext(iIndex);

        m_lastFreeIndex = iIndex;
        // m_buffer[m_buffer[iIndex].GetNext()].SetPrev(iIndex);
    }
    XVOID RemoveFromFree(XS32 index)
    {
        //必须是空闲链表中的
        CPP_ASSERT_RN(!IsActiveByIndex(index));
        //这里删除操作
        if ((XU32)index == m_lastFreeIndex) // 移除最后一个节点,需要修改最后一个节点
        {
            m_lastFreeIndex = m_buffer[m_lastFreeIndex].GetPrev();
        }

        XS32 iprev = m_buffer[index].GetPrev();
        XS32 inext = m_buffer[index].GetNext();
        m_buffer[iprev].SetNext(inext);
        m_buffer[inext].SetPrev(iprev);
        //设置为活动态
        m_buffer[index].SetActive();
    }

public:
    XVOID ClrCounter()
    {
        m_oprCounter.Clear();
    }
    //打印该资源池的信息
    XVOID Print(CLI_ENV* pCliEnv);
    //打印资源池中指定资源ID的信息
    XVOID PrintItem(XU32 id, CLI_ENV* pCliEnv);
    //打印空闲节点的信息
    XVOID PrintLastFree(XU32 cnt, CLI_ENV* pCliEnv);
    //Print last free resource node's information
    XVOID PrintLastFree(XU32 id, XU32 cnt, CLI_ENV* pCliEnv);
    XVOID PrintItems(XU32 startItem, XU32 cnt, CLI_ENV* pCliEnv);       //Print several resource node's information

private:
    FORBID_COPY_ASSIGN(TResourcePool);//禁止拷贝和赋值

private:
    TVctNode          m_buffer;        //所有空闲节点组成一个环型索引链表,链表的头节点为m_buffer[0]
    IProcesserId*     m_pSpawnId;      //计算输出节点ID的策略对象,
    XU32              m_lastFreeIndex; //最后释放的一个节点

private:
    CHistoryCount<XU32> m_oprCounter;
};

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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

    //为m_buffer保留uiMax+1大小的内存,+1是因为将m_buffer[0]作为了空闲链表的辅助头节点
    m_buffer.reserve(uiMax+1);
    //将m_buffer[0]作为空闲链表的头节点
    m_buffer.push_back(typename TResourcePool::Node());
    m_buffer[FREE_HEAD_INDEX].SetNext(FREE_HEAD_INDEX);
    m_buffer[FREE_HEAD_INDEX].SetPrev(FREE_HEAD_INDEX);

    m_lastFreeIndex = FREE_HEAD_INDEX; // 初始化后,没有最后释放
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> EerrNo TResourcePool<T>::Input(T *pObj)
{
    CPP_ASSERT_RV(pObj != XNULL, ERR_INITIAL_FAIL);
    if(m_buffer.size()  == m_buffer.capacity())
    {
        return ERR_MEMORY;
    }
    m_buffer.push_back(typename TResourcePool::Node(pObj));
    XU32 index = m_buffer.size()-1;//刚被添加的节点的索引,索引从1开始
    AddToFree(index);
    pObj->SetId(index);//重新设置ID
    return ERR_OK;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> T* TResourcePool<T>::Active()
{
    XS32 index = m_buffer[FREE_HEAD_INDEX].GetNext();
    //不合法的索引，直接返回
    if( !IsValidIndex(index))
    {
        return XNULL;
    }
    RemoveFromFree(index);
    //增加已分配的计数
    m_oprCounter.IncrementCount();
    //根据索引,重新设置对象的ID
    m_buffer[index].GetData()->SetId(m_pSpawnId->SpawnNewId(index));
    return m_buffer[index].GetData();
}
/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T>  T * TResourcePool<T>::Active(XU32 id)
{
    if(GetStateById(id) == OBJ_SLEEP)
    {
        m_oprCounter.IncrementCount();//将已分配的个数加1
        XS32 index = GetIndexFormId(id);
        RemoveFromFree(index);
        return m_buffer[index].GetData();
    }
    return XNULL;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> EerrNo  TResourcePool<T>::DeActive(XU32 id)
{
    if(GetStateById(id) == OBJ_ACTIVE)
    {
        m_oprCounter.DecrementCount();//释放了一个
        AddToFree(GetIndexFormId(id));
        return ERR_OK;
    }
    return ERR_INVALID_OPR;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> bool TResourcePool<T>::IsValidId(XU32 id,XS32& index)const
{
    //通过ID获得索引
    index = GetIndexFormId(id);
    //判断索引是否是否合法
    if(IsValidIndex(index))
    {
        //判断对应位置上的ID与参数中的ID一样
        return  m_buffer[index].GetData()->GetId() == id;
    }
    return false;
}

/*********************************************************************
函数名称 :
功能描述 : 通过ID获得T的指针，注意，T指向的资源状态有可能是Sleep的

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> T *TResourcePool<T>::GetById(XU32 id)const
{
    XS32 index ;//这里不初始化
    //是合法的索引
    if(IsValidId(id,index))
    {
        return m_buffer[index].GetData();
    }
    return XNULL;
}

/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> EObjState TResourcePool<T>::GetStateById(XU32 id)const
{
    XS32 index ;//这里不初始化
    //是合法的索引
    if(IsValidId(id,index))
    {
        //是不可用的ID
        return (m_buffer[index].IsActive() ? OBJ_ACTIVE:OBJ_SLEEP);
    }
    return OBJ_ERROR;
}

/*********************************************************************
函数名称 :  TResourcePool的析构函数
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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
函数名称 :
功能描述 : 调试打印信息的接口

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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
函数名称 :
功能描述 : 打印某个资源节点的信息

参数输入 : id         XU32          将要打印的状态机的ID
           mid        MDID          该资源此所属的模块ID的
           ePL        EPrintLevel   打印级别
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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
函数名称 :
功能描述 : 打印空闲节点的信息

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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
函数名称 :
功能描述 : //Print last free resource node's information

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
template<class T> XVOID TResourcePool<T>::PrintLastFree(XU32 id, XU32 uiCnt, CLI_ENV* pCliEnv)
{
    XS32 index ; //这里不初始化
    if( (uiCnt == 0)||(!IsValidId(id,index)) )
    {
        return ; //该ID非法，直接返回
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
函数名称 :
功能描述 :  Print several resource node's information

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
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
名称 : class CFastIndexPool
职责 :
       将[0,n-1]的正整数集合作为一个资源池,池中的正整数
       可以处于两种状态:已分配态和空闲态.
       这种索引池分配和释放一个资源的速度较快，但是消耗较多的
       内存.
       CCmprIndexPool资源池较节省内存，但是分配和释放一索引的
       的速度较慢
协作 :
历史 :
       修改者   日期          描述
**********************************************************************/
class CFastIndexPool
{
#define INVILIDE_INDEX  0xffff
#define BLOCKSIZE       sizeof(XU16)

public:
    CFastIndexPool(XU16 usMaxIndex)
        :m_pIndexPool(XNULL)
    {
        //元素的个数比CCHUNK_INVILIDE_INDEX少一个,即最多65534
        CPP_ASSERT_RN( usMaxIndex < (XU16)INVILIDE_INDEX);
        m_pIndexPool = new  CChunk (BLOCKSIZE,usMaxIndex);
    }
    ~CFastIndexPool()
    {
        delete m_pIndexPool;
        m_pIndexPool = XNULL;
    }
    //分配下一个空闲的索引
    XU16 AllocIndex()
    {
        if(m_pIndexPool->IsEmpty())
        {
            return INVILIDE_INDEX;//空的就直接返回一个非法值
        }
        XU16 curIndex =  static_cast<XU16>(m_pIndexPool->m_usFrIdx);
        //已分配出去的内存块写上一个非法值,在释放ID时做检查用
        *((XU16*) m_pIndexPool->Allocate()) = INVILIDE_INDEX;
        return curIndex;
    }
    //释放成功返回TRUE,释放失败返回FALSE
    bool  DeallocIndex(XU16 index)
    {
        //若索引范围越界，则返回FALSE，或者，释放了一个还未分配的索引，也返回FALSE
        if((index >= m_pIndexPool->BlkCount()) || (m_pIndexPool->NextIndex(index) != INVILIDE_INDEX) )
        {
            return false;
        }
        m_pIndexPool->AddToFree(index);
        return true;
    }
    //是否为合法的索引,合法
    bool IsValidIndex(XU16 index)const
    {
        return index < m_pIndexPool->BlkCount();
    }
    //资源池是否已经为空.为空就是没有空闲的索引了
    bool IsEmpty()const
    {
        return m_pIndexPool->IsEmpty();
    }

private:
    //禁止拷贝和赋值
    FORBID_COPY_ASSIGN(CFastIndexPool);

private:
    CChunk * m_pIndexPool;
};



#endif

