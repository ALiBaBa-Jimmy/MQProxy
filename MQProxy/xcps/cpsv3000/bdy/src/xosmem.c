/***************************************************************
 **
 **  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
 **
 **  Core Network Department  platform team
 **
 **  filename: xosmem.c
 **
 **  description:
 **
 **  author: wangzongyou
 **
 **  date:   2006.7.05
 **
 ***************************************************************
 **                          history
 **
 ***************************************************************
 **   author          date              modification
 **
 **************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/

#include "xosxml.h"

#include "xosarray.h"
#include "xoshash.h"
#include "clishell.h"
#include "xosencap.h"

#include "xosmem.h"

#include "xostrace.h"
#include "xosmodule.h"
#include "xosos.h"
#include "xoslog.h"
#include "xospub.h"

#ifdef XOS_LINUX
#include<execinfo.h>
#endif

XEXTERN XS32 TRC_formateloctime(XCHAR *datet);
XSTATIC XS32 MEM_findSymByAddr( XPOINT addr,XCHAR* symbolName );


/*-------------------------------------------------------------------------
                 模块内部宏定义
-------------------------------------------------------------------------*/
#define  MAX_BLOCK_BITS           (32)           /*最大支持 4G 的内存块*/
#define  MEM_MAGIC_VALUE          (0x00aaff77)  /*用来验证内存的头*/
#define  MEM_MAGIC_TAIL           (0x00aaff88)  /*用来验证内存的尾*/
#define  MEM_DBG_FILE_NAME_LEN    (16)          /*最大的文件名长度*/
#define  MEM_CMD_SHOW_CTL(x)      ((x)%50 == 0)? (XOS_Sleep(5)) : XOS_UNUSED(x)

/*-------------------------------------------------------------------------
                 模块内部结构和枚举定义
-------------------------------------------------------------------------*/

/*BUCKECT 控制块*/
typedef struct
{
    XS32 blockSize;
    XCHAR *headAddr;
    XCHAR *tailAddr;
    t_XOSMUTEXID bucketLock; /* 对每种bucket 上琐*/
    XOS_HARRAY  blockArray;
}t_BUCKETCB;


/*内存块头部控制结构*/

/*add by lixn 2007-1-30*/
#ifdef MEM_FID_DEBUG
typedef struct _MEM_Fun_Stack_S
{

     XPOINT AllocFunc_ST[MAX_MEM_STACK_DEPTH];
     XPOINT FreeFunc_ST[MAX_MEM_STACK_DEPTH];

}t_MEMSTACK;
#endif /*MEM_FID_DEBUG*/

typedef struct
{
#ifdef MEM_FID_DEBUG
    XU32  fid;  /*功能块号*/
    t_XOSTT time;/*内存分配的时间*/
    XCHAR fileName[MEM_DBG_FILE_NAME_LEN]; /*分配内存的文件名*/
    XU32  lineNum;           /*分配内存的位置*/

    /*函数调用栈*/
    t_MEMSTACK memstack;/*内存分配时的函数调用栈*/
 #endif /*MEM_FID_DEBUG*/

    XS32 memLen;         /*分配的内存的长度*/
    XS32 headCheck;    /*内存块头部验证字, 防止内存前越界*/
}t_BLOCKHEAD;

/*内存块尾部验证结构*/
typedef struct
{
    XS32 tailCheck; /*内存块尾部验证字,防止内存块后越界*/
}t_BLOCKTAIL;

/*二分查找保存数据*/
typedef struct
{
    XVOID *pLocation; /*bucket 在hash中对应的位置信息*/
}t_BUCKPTR;

/* 内存显示信息类型的定义*/
typedef struct
{
    XS32 index;
    CLI_ENV* pCliEnv;
    XU64 totalUse;
#ifdef MEM_FID_DEBUG
    XU32 fid;
#endif
}t_MEMSHOW;

/*内存管理的结构*/
typedef struct
{
    XBOOL initialized;  /*内存初始化的标志*/
    XOS_HHASH buckHash; /*快速定位bucket 用*/
    XU32 buckTypes;                /* 配置文件中配置的内存的条目数*/
    t_MEMBLOCK *pBlockPtr;     /*整理后的配置文件中的信息*/
    XU64 totalSize;                   /*总共分配的内存大小*/
    t_BUCKPTR *pElements;     /* hash 表中元素位置信息,用来内存释放时二分查找*/
    XS32 maxBuckBits;
}t_MEMMNT;

#ifdef MEM_FID_DEBUG
#include "xosqueue.h"

extern t_MODMNT  g_modMnt;
#define XOS_MAX_THREADID (1000) /*最大线程数量，线程ID在操作系统内唯一*/
typedef struct 
{
    XU32 uFid;
    XU64 dwThreadId;
}t_threadNode;

typedef struct 
{
    XU32 uMaxsize;
    XU32 uCurSize;
    t_threadNode threadId[XOS_MAX_THREADID];
}t_ThreadIdCache;

t_ThreadIdCache g_ThreadIdCache; /*线程ID哈希缓存表*/
XBOOL bModifyFidFlg = XFALSE;    /*fid 修正标志*/

#endif



/*定义内存释放时需要打印堆栈，FID和打印次数*/
XSTATIC t_XOSMUTEXID g_fidMemMux;
XS32 g_fidMemCfg[FID_MAX];

/*-------------------------------------------------------------------------
                模块内部全局变量
-------------------------------------------------------------------------*/
t_MEMMNT g_memMnt;
t_MEMEXCFG g_memExCfg;
RESET_PROC_FUNC pMemProcFunc = NULL;

/*-------------------------------------------------------------------------
                模块内部函数
-------------------------------------------------------------------------*/
XU32 MEM_CallTrace(XPOINT *pBuffer, XU32 nDepth);
XS16 MEM_Ptr_Show(CLI_ENV* pCliEnv, XPOINT pPtr);
/************************************************************************
函数名: XOS_MemProcFuncReg
功能：  注册回调函数，用于系统分配内存出错时业务层进行善后处理
输入：  ulFuncAddr 欲注册的函数
输出：
返回：
说明：
************************************************************************/
XS32 XOS_MemProcFuncReg(void * ulFuncAddr)
{
     pMemProcFunc = (RESET_PROC_FUNC)ulFuncAddr;
     return XSUCC;
}

/************************************************************************
 函数名 : MEM_getBitsNum
 功能   : 获取位数
 输入   :
 输出   : none
 返回   : 位数(长度为零返回-1)
 说明   :
************************************************************************/
XSTATIC XS16 MEM_getBitsNum(XU32 len)
{
    XS16 bits;
    XU32 tempLen;

    bits = 0;
    tempLen = len;
    while(tempLen)
    {
        tempLen = tempLen>>1;
        bits ++;
    }
    return bits;
}

/************************************************************************
 * MEM_hashFree
 * 功能: 定义对每个hash元素通用的函数
 * 输入: hHash   - 对象句柄
 *              elem    - 元素
 *              param   - 参数
 * 输出:
 * 返回: 指向一个参数，给下次调用XOS_HASHFunc使用
 ************************************************************************/
 XSTATIC XVOID* MEM_hashFree(XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
    t_BUCKETCB *pBuckCb;
    pBuckCb = (t_BUCKETCB*)XNULLP;
    pBuckCb = (t_BUCKETCB*)elem;
    if(pBuckCb != XNULLP)
    {
        XOS_ArrayMemDst(pBuckCb->blockArray);
        XOS_MutexDelete(&(pBuckCb->bucketLock));
    }
    return param;
}

/************************************************************************
 * MEM_hashShow
 * 功能: 定义对每个hash元素通用的函数
 * 输入: hHash   - 对象句柄
 *              elem    - 元素
 *              param   - 参数
 * 输出:
 * 返回: 指向一个参数，给下次调用XOS_HASHFunc使用
 ************************************************************************/
 XSTATIC XVOID* MEM_hashShow(XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
    t_BUCKETCB *pBuckCb;
    t_MEMSHOW  *pMemShow;
    XS32 curUseNum;

    pBuckCb = (t_BUCKETCB*)XNULLP;
    pBuckCb = (t_BUCKETCB*)elem;
    pMemShow = (t_MEMSHOW*)param;

    if(pBuckCb != XNULLP && pMemShow != XNULLP)
    {
         curUseNum = XOS_ArrayGetCurElemNum(pBuckCb->blockArray);
         XOS_CliExtPrintf( pMemShow->pCliEnv,
         "%-6d%-12d%-8d%-10d%-10d\r\n",
         pMemShow->index,
         pBuckCb->blockSize,
         XOS_ArrayGetMaxElemNum(pBuckCb->blockArray),
         curUseNum,
         XOS_ArrayGetMaxUsageElemNum(pBuckCb->blockArray)
         );

         pMemShow->index ++;
         pMemShow->totalUse = pMemShow->totalUse + curUseNum*pBuckCb->blockSize;
    }
    return param;
}

/**********************************
函数名称    : memHashShow
作者        : Liu.Da
设计日期    : 2007年12月6日
功能描述    : 定义对每个hash元素通用的函数
参数        : XOS_HHASH hHash
参数        : XVOID* elem  - 元素
参数        : XVOID *param - 参数
返回值        : XSTATIC XVOID*  指向一个参数，给下次调用XOS_HASHFunc使用
************************************/
XSTATIC XVOID* memHashShow(XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
    t_BUCKETCB *pBuckCb;
    t_MEMSHOW  *pMemShow;
    XS32 curUseNum;

    pBuckCb = (t_BUCKETCB*)XNULLP;
    pBuckCb = (t_BUCKETCB*)elem;
    pMemShow = (t_MEMSHOW*)param;

    if(pBuckCb != XNULLP && pMemShow != XNULLP)
    {
         curUseNum = XOS_ArrayGetCurElemNum(pBuckCb->blockArray);
         printf(
         "%-6d%-12d%-8d%-10d%-10d\r\n",
         pMemShow->index,
         pBuckCb->blockSize,
         XOS_ArrayGetMaxElemNum(pBuckCb->blockArray),
         curUseNum,
         XOS_ArrayGetMaxUsageElemNum(pBuckCb->blockArray)
         );

         pMemShow->index ++;
         pMemShow->totalUse = pMemShow->totalUse + curUseNum*pBuckCb->blockSize;
    }
    return param;
}
#ifdef MEM_FID_DEBUG
/************************************************************************
 * MEM_fidShow
 * 功能: 定义对每个hash元素通用的函数
 * 输入: hHash   - 对象句柄
 *              elem    - 元素
 *              param   - 参数
 * 输出:
 * 返回: 指向一个参数，给下次调用XOS_HASHFunc使用
 ************************************************************************/
XSTATIC XVOID* MEM_fidShow(XOS_HHASH hHash, XVOID* elem, XVOID *param)
{
    t_BUCKETCB *pBuckCb;
    t_MEMSHOW  *pMemShow;
    t_BLOCKHEAD *pBlockHead;
    XS32 fidUsage = 0;
    XS32 i;
    XU32 ublockSize = 0;
    XU32 uMaxElemNum = 0;
    XU32 uCurElemNum = 0; 

    pBuckCb = (t_BUCKETCB*)XNULLP;
    pBuckCb = (t_BUCKETCB*)elem;
    pMemShow = (t_MEMSHOW*)param;
    if(pBuckCb == XNULLP)
    {
        return param;
    }
    XOS_MutexLock(&(pBuckCb->bucketLock));
    for(i = XOS_ArrayGetFirstPos(pBuckCb->blockArray);
          i>=0;
          i= XOS_ArrayGetNextPos(pBuckCb->blockArray, i))
    {
         pBlockHead = (t_BLOCKHEAD*)XNULLP;
         pBlockHead = XOS_ArrayGetElemByPos(pBuckCb->blockArray, i);

         if(pBlockHead != XNULLP &&pMemShow->fid == pBlockHead->fid)
         {
             fidUsage ++;
         }
    }

    ublockSize = pBuckCb->blockSize;
    uMaxElemNum = XOS_ArrayGetMaxElemNum(pBuckCb->blockArray);
    uCurElemNum = XOS_ArrayGetCurElemNum(pBuckCb->blockArray);
         
    XOS_MutexUnlock(&(pBuckCb->bucketLock));

    if(pMemShow != XNULLP)
    {
         XOS_CliExtPrintf( pMemShow->pCliEnv,
         "%-6d%-12d%-8d%-10d%-10d\r\n",
         pMemShow->index,
         ublockSize,
         uMaxElemNum,
         uCurElemNum,
         fidUsage
         );

         pMemShow->index ++;
    }
    return param;
}

#endif /*MEM_FID_DEBUG*/

/*确定是否为2 的整数次方*/
#define  IS_2N_NUM(n) (((n)==(XU32)(1<<MEM_getBitsNum((n)-1)))?XTRUE:XFALSE)

/************************************************************************
 * 输入 : param                - 元素
 *               paramSize          - 元素的大小(  单位字节)
 *               hashSize             - hash表的大小
 * 返回 : hash 结果
 ************************************************************************/
 XSTATIC  XU32 MEM_hashFunc(  XVOID*  param,  XS32    paramSize,  XS32    hashSize)
{
     XS32 hash = *((XS32*)param);

     return (hash % hashSize);
}

/************************************************************************
 * 函数名: MEM_tidyCfgBlocks
 * 输入 : pMemBlock                - 配置文件中读出的内存块配置数
 *               pBlocks          - 读出配置的内存块的个数
 *
   输出:  pBlocks       -整理后的内存块数量
 * 返回 :
 ************************************************************************/
 XSTATIC  XVOID MEM_tidyCfgBlocks(  t_MEMBLOCK* pMemBlock,  XS32 * pBlocks)
{
     XS32 i, j;
     XS32 blockRead;
     t_MEMBLOCK *pMemTemp;
     t_MEMBLOCK *pMemTemp2;

     /*整理一下，去掉block 数量是0 的*/
     blockRead = *pBlocks;
     for( i=0; i<blockRead; i++)
     {
          pMemTemp = pMemBlock+i;
          if(pMemTemp->blockNums == 0 || !IS_2N_NUM(pMemTemp->blockSize))
          {
              for(j=i+1; j<blockRead; j++)
              {
                   pMemTemp2 = pMemBlock+j;
                   if(pMemTemp2->blockNums > 0 && IS_2N_NUM(pMemTemp2->blockSize))
                   {
                        XOS_MemCpy(pMemTemp, pMemTemp2, sizeof(t_MEMBLOCK));
                        XOS_MemSet(pMemTemp2, 0, sizeof(t_MEMBLOCK));
                        break;
                   }
              }
          }
     }

     /* 获取其中实际的数目*/
     g_memMnt.totalSize = 0;
     j = 0;
     for(i=0; i < blockRead;  i++)
     {
         pMemTemp = pMemBlock+i;
         if(pMemTemp->blockNums > 0 && IS_2N_NUM(pMemTemp->blockSize))
         {
              j++;
              g_memMnt.totalSize = g_memMnt.totalSize + (pMemTemp->blockNums)*(pMemTemp->blockSize);
         }
     }
     *pBlocks = j;

}
/*-------------------------------------------------------------------------
                模块接口函数
-------------------------------------------------------------------------*/

/************************************************************************
 函数名 : MEM_Initlize
 功能   : 内存初始化
 输入   :
 输出   : none
 返回   :
 说明   :
************************************************************************/
XS32 MEM_Initlize(XVOID )
{
   t_MEMCFG memCfg;
   t_MEMBLOCK *pMemBlock;
   t_BUCKETCB  bucketCb;
   t_BUCKETCB*  pBucketCb;
   t_BUCKETCB*  pTempCb;
   XVOID *pLocation;
   t_BUCKPTR*pTemp;
   t_BUCKPTR *pTemp1;
   t_BUCKPTR temp2;
   XS32 ret;
   XU16 i;
   XU16 j;

   /*如果已经初始化*/
    if(g_memMnt.initialized)
    {
       XOS_CpsTrace(MD(FID_ROOT, PL_WARN), "MEM_init()-> reInit mem!");
       return XSUCC;
    }

   /*读内存配置文件*/
       XOS_MemSet(&memCfg, 0, sizeof(t_MEMCFG));
       XOS_MutexCreate(&g_fidMemMux);
    XOS_MemSet(g_fidMemCfg, 0, sizeof(g_fidMemCfg));
    
    //用下面的#IF 0替换该代码
#ifndef XOS_EW_START
    ret = XML_readMemCfg(&memCfg, "xos.xml");
#else
    ret = XML_readMemCfg(&memCfg, XOS_CliGetXmlName( ));
#endif
    if(ret != XSUCC || memCfg.memTypes == 0||memCfg.pMemBlock == XNULLP)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> read mem config file failed!");
        return XERROR;
    }
    /*读取内存异常处理配置信息,如果读取错误则按照默认值即可*/
    XML_readMemExCfg(&g_memExCfg, "xos.xml");
    
    /*整理，除掉配置文件中配置数据块个数为零的情况*/
    MEM_tidyCfgBlocks(memCfg.pMemBlock, (XS32 *)&(memCfg.memTypes));

    /*保存配置信息*/
    g_memMnt.buckTypes = memCfg.memTypes;
    g_memMnt.pBlockPtr = memCfg.pMemBlock;

    /*分配资源*/

    /*创建hash表*/
    g_memMnt.buckHash = XOS_HashMemCst(memCfg.memTypes+1, memCfg.memTypes, sizeof(XS32), sizeof(t_BUCKETCB), "memHash");
    if(!XOS_HashHandleIsValid(g_memMnt.buckHash) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> construct hash  failed!");
        if(memCfg.pMemBlock != XNULLP)
        {
            XOS_Free(memCfg.pMemBlock);
        }
        return XERROR;
    }
     /*设置hash 函数*/
    XOS_HashSetHashFunc(g_memMnt.buckHash, MEM_hashFunc);

    /*分配二分查找的内存空间*/
    g_memMnt.pElements = (t_BUCKPTR*)XNULLP ;
    g_memMnt.pElements = (t_BUCKPTR*)XOS_Malloc(sizeof(t_BUCKPTR)*memCfg.memTypes);
    if(g_memMnt.pElements == XNULLP)
    {
         XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> malloc the elements failed !");
         goto memInitErorr;
    }

    /*分配内存*/
    pMemBlock = (t_MEMBLOCK*)XNULLP;
    g_memMnt.maxBuckBits = 0;
    for(i=0; i<memCfg.memTypes; i++)
    {
        pMemBlock = memCfg.pMemBlock+i;
        XOS_MemSet(&bucketCb, 0, sizeof(t_BUCKETCB));
        bucketCb.blockSize = pMemBlock->blockSize;

        if(g_memMnt.maxBuckBits < bucketCb.blockSize)
         g_memMnt.maxBuckBits = bucketCb.blockSize;
        /*创建互斥量*/
        if( XSUCC != XOS_MutexCreate(&(bucketCb.bucketLock)))
        {
            XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> create mutex lock failed !");
            goto memInitErorr;
        }

        /*创建bucket 数组*/
        bucketCb.blockArray = XOS_ArrayMemCst(pMemBlock->blockSize+sizeof(t_BLOCKHEAD)+sizeof(t_BLOCKTAIL),
                                                                          pMemBlock->blockNums, "bucket");
        if(!XOS_ArrayHandleIsValid(bucketCb.blockArray))
        {
             XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> create array failed !");
             goto memInitErorr;
        }
        bucketCb.headAddr = (XCHAR*)XOS_ArrayGetHeadPtr(bucketCb.blockArray);
        bucketCb.tailAddr = (XCHAR*)XOS_ArrayGetTailPtr(bucketCb.blockArray);

        /*将申请的内存都初始化为0*/
        /*XOS_MemSet( bucketCb.headAddr, 0,
                               (pMemBlock->blockSize+sizeof(t_BLOCKHEAD)+sizeof(t_BLOCKTAIL) ) * pMemBlock->blockNums );
                               */
        /*添加到hash表中*/
        pLocation = XNULLP;
        pLocation = XOS_HashElemAdd(g_memMnt.buckHash, (XVOID*)&(pMemBlock->blockSize), (XVOID*)&bucketCb, XFALSE);
        if(pLocation == XNULLP)
        {
             XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> add bucket cb to hash faililed !");
             goto memInitErorr;
        }

        /* 保存信息做二分查找*/
        g_memMnt.pElements[i].pLocation = pLocation;

    }

    g_memMnt.maxBuckBits = MEM_getBitsNum(g_memMnt.maxBuckBits-1);

    /*对二分查找的部分进行排序*/
    /*按照内存地址增长的顺序排列*/
    for(i= 0; i<memCfg.memTypes; i++)
    {
        pTemp = g_memMnt.pElements + i;

        for(j=i+1; j<memCfg.memTypes; j++)
        {
            pBucketCb = (t_BUCKETCB*)XOS_HashGetElem(g_memMnt.buckHash, pTemp->pLocation);
            if(pBucketCb == XNULLP)
            {
                goto memInitErorr;
            }
            pTemp1 = g_memMnt.pElements + j;
            pTempCb = (t_BUCKETCB*)XOS_HashGetElem(g_memMnt.buckHash, pTemp1->pLocation);
            if(pTempCb == XNULLP)
            {
                goto memInitErorr;
            }
            if((XPOINT)(pBucketCb->headAddr) > (XPOINT)(pTempCb->headAddr) )
            {
                XOS_MemCpy(&temp2, pTemp1, sizeof(t_BUCKPTR));
                XOS_MemCpy(pTemp1, pTemp, sizeof(t_BUCKPTR));
                XOS_MemCpy(pTemp, &temp2, sizeof(t_BUCKPTR));
            }
        }

    }

    g_memMnt.initialized = XTRUE;
    return XSUCC;

    memInitErorr:

    /*释放读配置文件的空间*/
     if(memCfg.pMemBlock != XNULLP)
     {
         XOS_Free(memCfg.pMemBlock);
     }

     /*释放所有的bucket 内存*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_hashFree, XNULLP);

     /*释放hash内存*/
     XOS_HashMemDst(g_memMnt.buckHash);

     g_memMnt.initialized = XFALSE;
     return XERROR;

}

#ifdef MEM_FID_DEBUG
/************************************************************************
函数名: MEM_BKDRHash
功能：  字符串哈希函数
输入：  dwThreadId           - 当前运行的线程ID
输出： 
返回：  哈希索引
说明：
************************************************************************/
XU32  MEM_BKDRHash(XU64 dwThreadId)
{
    XU32 hashKey = 0;
    XU32 ulSeed = 131; //31、131、1313、13131
    XCHAR tmpBuf[8+2+1] = {0};
    XCHAR *pChar = NULL;

    /*将数字转化字符串*/
    XOS_Sprintf(tmpBuf, sizeof(tmpBuf)-1,"0x%08x", dwThreadId);    
    pChar = (XCHAR*)tmpBuf;
    
    while(*pChar)
    {
        hashKey = hashKey * ulSeed + (*pChar++);
    }

    hashKey = hashKey & 0X7FFFFFFF;    
    
    hashKey %= (((XU32)sizeof(g_ThreadIdCache.threadId)/sizeof(g_ThreadIdCache.threadId[0])));

    return hashKey;
}


/************************************************************************
函数名: MEM_QueryFidByThreadId
功能：  通过线程ID查询真实的FID
输入：  dwThreadId           - 当前运行的线程ID
输出：  upNewFid             - 当前模块的真正FID
返回：  
说明：
************************************************************************/
XU32 MEM_QueryFidByThreadId(XU64 dwThreadId, XU32 *upNewFid)
{
    XU32 hashKey = MEM_BKDRHash(dwThreadId);
    
    if(hashKey >=0 && hashKey < sizeof(g_ThreadIdCache.threadId)/sizeof(g_ThreadIdCache.threadId[0]) 
    && 0 != g_ThreadIdCache.threadId[hashKey].dwThreadId)
    {
        *upNewFid = g_ThreadIdCache.threadId[hashKey].uFid;
        return XSUCC;
    }
    else
    {
        return XERROR;
    }
}


/************************************************************************
函数名: MEM_AddFidByThreadId
功能：  通过线程ID将FID与线程ID的关系保存起来
输入：  dwThreadId     - 当前运行的线程ID
输出：  uFid           - 当前运行的线程ID所对应的模块的FID
返回：  
说明：  一个线程只会操作一个元素，不需要保护临界区
************************************************************************/
XVOID MEM_AddFidByThreadId(XU64 dwThreadId, XU32 uFid)
{
    XU32 hashKey = MEM_BKDRHash(dwThreadId);
    
    if(hashKey >=0 && hashKey < sizeof(g_ThreadIdCache.threadId)/sizeof(g_ThreadIdCache.threadId[0]) )
    {
      if(0 == g_ThreadIdCache.threadId[hashKey].dwThreadId)
      {
        g_ThreadIdCache.threadId[hashKey].dwThreadId = dwThreadId;
        g_ThreadIdCache.threadId[hashKey].uFid = uFid;
      }
      else
      {
      }
    }
}


/************************************************************************
函数名: MEM_fidModify
功能：  根据当前线程向平台注册的FID进行内存申请前的FID修正
输入：  pFid           - 功能块id
输出：  pFid           - 返回修正后的FID
返回：  
说明：  如果一个模块只有一个FID，但创建了多个线程，则只有在平台中注册的线程才能进行修正。如定时器模块的高、低定时线程没有注册到平台，则不能修正。
        目前平台仅支持一个FID注册一个线程。
************************************************************************/
XS32 MEM_fidModify(XU32 *pFid)
{
#ifdef MEM_FID_DEBUG
    XU32 i = 0;
    t_TIDCB *pTidCb = NULL;
    t_FIDCB *pFidCb = NULL;

    t_XOSTASKID tmpThreadHandle = 0; /*线程句柄*/
    XU64 tmpTaskId = 0;              /*线程ID*/

#ifdef XOS_WIN32    
    XU64 oldTaskId = 0;              /*线程ID*/
#endif
    if(NULL == pFid)
    {
        return XERROR;
    }
    
    if(100 == *pFid)/*只对new操作进行*/
    {
      
#ifdef XOS_VXWORKS
        tmpTaskId = (XU64)taskIdSelf(); 
        /*获取当前线程ID,vx下实际上返回的是控制块的地址, tmpTaskId与tmpThreadHandle表示同样的值*/
        tmpThreadHandle = (t_XOSTASKID)taskTcb(tmpTaskId);        

        /*搜索fid与hreadhandle缓存哈希表*/
        if(XSUCC == MEM_QueryFidByThreadId(tmpThreadHandle, pFid))
        {
            return XSUCC;
        }

        for(i = FID_XOSMIN; i<FID_MAX; i++)
        {
            pFidCb = (t_FIDCB*)XNULLP;
            pFidCb = MOD_getFidCb(i);
            if(XNULL != pFidCb)
            {
                pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
                if(pTidCb != (t_TIDCB*)XNULLP)
                {
                    if(pTidCb->tskId == tmpThreadHandle)/*比较句柄*/
                    {
                        *pFid = pFidCb->fidNum;
                        MEM_AddFidByThreadId(tmpThreadHandle, *pFid);
                        return XSUCC;
                    }
                }
            }
        }        
#endif

#ifdef XOS_LINUX
        tmpThreadHandle = (t_XOSTASKID)pthread_self();/*获取当前线程句柄*/
        tmpTaskId = (XU64)gettid();/*获取当前线程ID*/
        
        /*搜索fid与hreadhandle缓存哈希表*/
        if(XSUCC == MEM_QueryFidByThreadId(tmpTaskId, pFid))
        {
            return XSUCC;
        }

        /*完善缓存表，每个模块注册完成之后最多执行一次，在呼个模块注册完成之前，可能会执行多次重复执行*/
        for(i = FID_XOSMIN; i<FID_MAX; i++)
        {
            pFidCb = (t_FIDCB*)XNULLP;
            pFidCb = MOD_getFidCb(i);
            if(XNULL != pFidCb)
            {
                pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
                if(pTidCb != (t_TIDCB*)XNULLP)
                {
                    if(pTidCb->tskId == tmpThreadHandle)/*比较句柄*/
                    {
                        *pFid = pFidCb->fidNum;                        
                        MEM_AddFidByThreadId(tmpTaskId, *pFid);
                        return XSUCC;
                    }
                }
            }
        }
#endif

#ifdef XOS_WIN32 /*win7 vista win2008*/
#if 0
        tmpTaskId = GetCurrentThreadId(); /*获取当前线程ID*/
        if(XSUCC == MEM_QueryFidByThreadId(tmpTaskId, pFid))
        {
            return XSUCC;
        }

        for(i = FID_XOSMIN; i<FID_MAX; i++)
        {
            pFidCb = (t_FIDCB*)XNULLP;
            pFidCb = MOD_getFidCb(i);
            if(XNULL != pFidCb)
            {
                pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
                if(pTidCb != (t_TIDCB*)XNULLP)
                {                            
                    /*根据线程句柄获取线程ID*/
                    oldTaskId = GetThreadId(pTidCb->tskId);
                    if(oldTaskId == tmpTaskId)/*比较线程ID,因为windows下不能第二次获取相同句柄*/
                    {
                        *pFid = pFidCb->fidNum;
                        MEM_AddFidByThreadId(tmpTaskId, *pFid);
                        return XSUCC;
                    }
                }
            }
        }
#endif
#endif


     }
#endif
    return XERROR;    
}

#endif


/************************************************************************
函数名: XOS_MemMalloc
功能：  分配一个内存块
输入：  fid           - 功能块id
        nbytes        - 要分配内存的字节数
输出：  N/A
返回：  XVOID *       -分配的内存指针
说明：  返回值由用户强制转换
************************************************************************/
XVOID *XOS_MemMalloc1(XU32 fid, XU32 nbytes, const XCHAR* fileName, XU32 lineNo)

{

    XS16 bits = 0;
    XS32 key = 0;
    t_BUCKETCB *pBuckCb = (t_BUCKETCB *)XNULLP;
    t_BLOCKHEAD *pBlockHead = (t_BLOCKHEAD *)XNULLP;
    t_BLOCKTAIL  *pBlockTail = (t_BLOCKTAIL *)XNULLP;
    char szBuf[256] = {0};


    /* add by wulei 2006.12.08 */
    /* 为支持C++的内存使用方法，利用此编译宏，则必须在main函数第一行调用MEM函数 */
    #ifdef XOS_NOTC_MEM
    if (!g_memMnt.initialized)
    {
        if (XSUCC != MEM_Initlize())
        {
            /* 内存初始化失败,直接退出 */
            return XNULLP;
        }
    }
    #endif /* XOS_NOTC_MEM */
    #ifdef XOS_MEM_ERROR
    if(XSUCC != MemMallocFailTest( fid,  nbytes, fileName,lineNo))
    {
        return XNULLP ;
    }
    #endif

    /*入口安全性检查*/
    if(!XOS_isValidFid(fid) || nbytes == 0 ||!g_memMnt.initialized)
    {
        return XNULLP;
    }

    /*构造key*/
    bits = MEM_getBitsNum(nbytes-1);
    /*正常状况下,第一次应该可以找到*/

    if(bits > g_memMnt.maxBuckBits )
    {   
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR),"MEM_getBitsNum()->the maxbytes is [%d], the bits is[%d], the nbytes is [%d]. ",
                 g_memMnt.maxBuckBits, bits, nbytes);
        goto MEMERR;
    }

    for(; bits <= g_memMnt.maxBuckBits; bits++)
    {
        pBuckCb = (t_BUCKETCB*)XNULLP;
        key = (1<<bits);
        pBuckCb = (t_BUCKETCB*)XOS_HashElemFind(g_memMnt.buckHash, (XVOID *)&key);
        if(pBuckCb != XNULLP) /*找到*/
        {
            pBlockHead = (t_BLOCKHEAD*)XNULLP;
            XOS_MutexLock(&(pBuckCb->bucketLock));
            XOS_ArrayAddExt(pBuckCb->blockArray, (XOS_ArrayElement*)&pBlockHead);
            if(pBlockHead != XNULLP)
            {
                 /*填写内存的头部字段*/
           #ifdef MEM_FID_DEBUG
                /*fid修正,适应C++的new操作时，使用的不是模块真正的FID，需要进行修正*/
                if(XTRUE == bModifyFidFlg)
                {
                    MEM_fidModify(&fid);
                }

                pBlockHead->fid = fid;
                XOS_Time((t_XOSTT*)&(pBlockHead->time));
                Trace_abFileName(fileName, (XCHAR*)(pBlockHead->fileName), MEM_DBG_FILE_NAME_LEN-1);
                pBlockHead->lineNum = lineNo;

                /*处理函数调用栈信息*/
                /*首先将头部以前存在的信息清除掉*/
                XOS_MemSet(pBlockHead->memstack.AllocFunc_ST, 0, MAX_MEM_STACK_DEPTH * sizeof(XPOINT) );

                MEM_CallTrace(pBlockHead->memstack.AllocFunc_ST,MAX_MEM_STACK_DEPTH) ;

           #endif
                pBlockHead->memLen = RV_ALIGN(nbytes);
                pBlockHead->headCheck = MEM_MAGIC_VALUE;

                /*填写尾部字段*/
                pBlockTail = (t_BLOCKTAIL*)(((XCHAR*)pBlockHead)+(sizeof(t_BLOCKHEAD)+pBlockHead->memLen));
                pBlockTail->tailCheck = MEM_MAGIC_TAIL;
                XOS_MutexUnlock(&(pBuckCb->bucketLock));

                return (XVOID*)(((XCHAR*)pBlockHead)+sizeof(t_BLOCKHEAD));
             }

             /*当前内存块已经用尽的情况*/
             /*以下打印可能造成无限递归，导致调用栈溢出*/
            //XOS_CpsTrace(MD(FID_ROOT, PL_WARN),
            //"XOS_MemMalloc()-> the blocSize[%d] bucket exhaust when fid %d call %d byetes!", pBuckCb->blockSize,fid,nbytes);
            XOS_MutexUnlock(&(pBuckCb->bucketLock));
        }
    }

MEMERR:
    /*在所有的bucket中都没有找到*/
    /*to do 扩展成heap*/
    /*以下打印可能造成无限递归，导致调用栈溢出*/
    //XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemMalloc()-> the all buckets exhaust when fid %d call %d byetes!",
    //            fid,nbytes);
    //XOS_CpsTrace(MD(FID_ROOT, PL_ERR),"XOS_MemMalloc()->the maxbytes is [%d], the bits is[%d]. ",
    //             g_memMnt.maxBuckBits, bits);

    /*异常的内存申请写入日志*/
    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "XOS_MemMalloc()-> the all buckets exhaust when [FID] %d call %d byetes!\r\n" \
                                        "FID[%d] XOS_MemMalloc()->the maxbytes is [%d], the bits is[%d]\r\n",
                                         fid, nbytes, fid, g_memMnt.maxBuckBits, bits);
    write_to_syslog(szBuf);

    /*将堆栈写入日志*/
    write_stack_to_syslog(fid);    

    /*内存异常时重启*/
    if(NULL!= pMemProcFunc)
    {        
        pMemProcFunc();
    }
    else if(g_memExCfg._enableBoot) 
    {
#ifdef XOS_VXWORKS
        XOS_Reset(0);
#else
        _exit(0);
#endif
    }
    return XNULLP;
}

/************************************************************************
函数名: XOS_MemCheck
功能：  判断内存块是否属于bucket的内存块,不释放内存
输入：  fid           - 功能块id
                ptr           - 内存块首地址
输出：  N/A
返回:   XSUCC  -    是
        XERROR -    否
说明：
************************************************************************/
XS32 XOS_MemCheck(XU32 fid, XVOID *ptr)
{
    XS32 i;
    XS32 j;
    t_BUCKPTR *pBuckPtr;
    t_BUCKETCB *pBuckCb;
    t_BLOCKHEAD *pBlockHead;
    t_BLOCKTAIL  *pBlockTail;

    /*安全性检查*/
    if(ptr == XNULLP || !g_memMnt.initialized)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->input ptr is null  !");
        #ifdef XOS_DEBUG
        /*调试用，如出现内存错误，则直接挂起*/
        XOS_SusPend();
        #endif
        return XERROR;
    }

    /*二分查找获取指针所在的array*/
    i =  0;
    j = g_memMnt.buckTypes-1;
    while(i <= j)
    {
        pBuckPtr = g_memMnt.pElements+((i+j)/2);
        pBuckCb = (t_BUCKETCB*)XNULLP;
        pBuckCb = XOS_HashGetElem(g_memMnt.buckHash, (XVOID *)(pBuckPtr->pLocation));
        if(pBuckCb == XNULLP)
        {
             XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->error get hash elem !");
             #ifdef XOS_DEBUG
             /*调试用，如出现内存错误，则直接挂起*/
             XOS_SusPend();
             #endif
             return XERROR;
        }
        /*查找成功*/
        if((XPOINT)ptr > (XPOINT)(pBuckCb->headAddr)
           &&(XPOINT)ptr < (XPOINT)(pBuckCb->tailAddr))
        {
             /*作安全性验证*/
             /*前越界验证*/
             pBlockHead = (t_BLOCKHEAD*)((XCHAR*)ptr-(sizeof(t_BLOCKHEAD)));
             if(pBlockHead->headCheck != MEM_MAGIC_VALUE
                 || pBlockHead->memLen > pBuckCb->blockSize)
             {
                 XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->mem destroy before addr[%p] !", ptr);
                 #ifdef XOS_DEBUG
                 /*调试用，如出现内存错误，则直接挂起*/
                 XOS_SusPend();
                 #endif
                 return XERROR;
             }
             /*后越界验证*/
             pBlockTail = (t_BLOCKTAIL*)((XCHAR*)ptr + pBlockHead->memLen);
             if(pBlockTail->tailCheck != MEM_MAGIC_TAIL)
             {
                  XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->mem destroy after addr[%p] !", ptr);
                  #ifdef XOS_DEBUG
                  /*调试用，如出现内存错误，则直接挂起*/
                  XOS_SusPend();
                  #endif
                  return XERROR;
             }

             /*判断到内存块属于bucket,但是不释放内存,返回成功*/
             return XSUCC;
        }

        /*在上半部*/
        if((XPOINT)ptr >(XPOINT)(pBuckCb->tailAddr))
        {
            i = (i+j)/2+1;
            continue;
        }

        /*在下半部分*/
        if((XPOINT)ptr < (XPOINT)pBuckCb->headAddr)
        {
            j = (i+j)/2 -1;
            continue;
        }

    }

    /*一直都没有找到, 应该是地址无效*/
    XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->input addr[%p]  is not in bucket !", ptr);
    #ifdef XOS_DEBUG
    /*调试用，如出现内存错误，则直接挂起*/
    XOS_SusPend();
    #endif

    return XERROR;
}

/************************************************************************
函数名: XOS_MemFree
功能：  释放一个内存块
输入：  fid           - 功能块id
        ptr           - 要释放的内存首地址
输出：  N/A
返回:   XSUCC  -    成功
        XERROR -    失败
说明：
************************************************************************/
XS32 XOS_MemFree(XU32 fid, XVOID *ptr)
{
    XS32 i = 0;
    XS32 j = 0;
    t_BUCKPTR *pBuckPtr;
    t_BUCKETCB *pBuckCb;
    t_BLOCKHEAD *pBlockHead;
    t_BLOCKTAIL  *pBlockTail;

    #ifdef XOS_MEM_ERROR
    MemFreeFailTest( fid, ptr);
    #endif

    /*安全性检查*/
    if(ptr == XNULLP || !g_memMnt.initialized)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_WARN), "XOS_MemFree()->input ptr is null  !");
        #ifdef XOS_DEBUG
        /*调试用，如出现内存错误，则直接挂起*/
        XOS_SusPend();
        #endif
        goto MEMERR;
    }

    /*二分查找获取指针所在的array*/
    i =  0;
    j = g_memMnt.buckTypes-1;
    while(i <= j)
    {
        pBuckPtr = g_memMnt.pElements+((i+j)/2);
        pBuckCb = (t_BUCKETCB*)XNULLP;
        pBuckCb = XOS_HashGetElem(g_memMnt.buckHash, (XVOID *)(pBuckPtr->pLocation));
        if(pBuckCb == XNULLP)
        {
             XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemFree()->error get hash elem !");
             #ifdef XOS_DEBUG
             /*调试用，如出现内存错误，则直接挂起*/
             XOS_SusPend();
             #endif
             goto MEMERR;
        }
        /*查找成功*/
        if((XPOINT)ptr > (XPOINT)(pBuckCb->headAddr)
           &&(XPOINT)ptr < (XPOINT)(pBuckCb->tailAddr))
        {
             /*作安全性验证*/
             /*前越界验证*/
             pBlockHead = (t_BLOCKHEAD*)((XCHAR*)ptr-(sizeof(t_BLOCKHEAD)));
             if(pBlockHead->headCheck != MEM_MAGIC_VALUE
                 || pBlockHead->memLen > pBuckCb->blockSize)
             {
                 XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemFree()->mem destroy before addr[%p] !", ptr);
                 #ifdef XOS_DEBUG
                 /*调试用，如出现内存错误，则直接挂起*/
                 XOS_SusPend();
                 #endif
                 goto MEMERR;
             }
             /*后越界验证*/
             pBlockTail = (t_BLOCKTAIL*)((XCHAR*)ptr + pBlockHead->memLen);
             if(pBlockTail->tailCheck != MEM_MAGIC_TAIL)
             {
                  XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemFree()->mem destroy after addr[%p] !", ptr);
                  #ifdef XOS_DEBUG
                  /*调试用，如出现内存错误，则直接挂起*/
                  XOS_SusPend();
                  #endif
                  goto MEMERR;
             }

        #ifdef MEM_FID_DEBUG
        XOS_MemSet(pBlockHead->memstack.FreeFunc_ST, 0, MAX_MEM_STACK_DEPTH * sizeof(XPOINT) );

        MEM_CallTrace(pBlockHead->memstack.FreeFunc_ST, MAX_MEM_STACK_DEPTH);
        #endif /*MEM_FID_DEBUG*/

             /*内存释放*/
             XOS_MutexLock(&(pBuckCb->bucketLock));
             XOS_ArrayDeleteByPos(pBuckCb->blockArray, XOS_ArrayGetByPtr(pBuckCb->blockArray, (XCHAR*)ptr-(sizeof(t_BLOCKHEAD))));
             XOS_MutexUnlock(&(pBuckCb->bucketLock));

             return XSUCC;
        }

        /*在上半部*/
        if((XPOINT)ptr >(XPOINT)(pBuckCb->tailAddr))
        {
            i = (i+j)/2+1;
            continue;
        }

        /*在下半部分*/
        if((XPOINT)ptr < (XPOINT)pBuckCb->headAddr)
        {
            j = (i+j)/2 -1;
            continue;
        }

    }

MEMERR:
    /*一直都没有找到, 应该是地址无效*/
    XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemFree()->error  fid is [%d] input addr[%p] bucktype is [%d]!",
                    fid, ptr, j);

     
    #ifdef XOS_DEBUG
    /*调试用，如出现内存错误，则直接挂起*/
    XOS_SusPend();
    #endif    

    //内存释放错误则记录堆栈
    write_stack_to_syslog(fid);
    
    return XERROR;
}

/*兼容SSI的内存管理接口*/
XU32 XOS_MEMCtrl(XU32 nbytes )
{
    XS32 key;
    t_BUCKETCB *pBuckCb= (t_BUCKETCB*)XNULLP;
    XU16 bits;
    XU32 avlnum,totnum,curUseNum;

    /*入口安全性检查*/
    if( nbytes == 0 )
    {
        return 0;
    }

    /*构造key*/
    bits = MEM_getBitsNum(nbytes-1);
    if(bits <= MAX_BLOCK_BITS)
    {

        key = (1<<bits);
        pBuckCb = (t_BUCKETCB*)XOS_HashElemFind(g_memMnt.buckHash, (XVOID *)&key);
        if(pBuckCb != XNULLP)
        {
         curUseNum = XOS_ArrayGetCurElemNum(pBuckCb->blockArray);
         totnum = XOS_ArrayGetMaxElemNum(pBuckCb->blockArray);
         avlnum = totnum-curUseNum;
         return (avlnum/(totnum/10));
        }
     }
    return 0;

}

/************************************************************************
 函数名:MEM_infoShow
 功能: 显示mem 中配置信息
 输入:
 输出:
 返回:
 说明: ntlthreedinfo命令的最终执行函数
************************************************************************/
XVOID MEM_infoShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
     t_MEMSHOW showInfo;

     XOS_CliExtPrintf(pCliEnv,
     "mem usage  list: \r\n--------------------------------------------------\r\n");
     XOS_CliExtPrintf(pCliEnv,
     "%-6s%-12s%-8s%-10s%-10s\r\n",
     "index",
     "blockSize",
     "maxCfg",
     "curUsage",
     "maxUsage"
     );
     XOS_MemSet(&showInfo, 0, sizeof(t_MEMSHOW));
     showInfo.index = 1;
     showInfo.pCliEnv = pCliEnv;
     showInfo.totalUse = 0;

     /*打印数据*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_hashShow, (XVOID*)&showInfo);

     /*end of udp list */
     XOS_CliExtPrintf(pCliEnv,
     "--------------------------------------------------\r\n total malloc: %llu  usage: %llu  leave: %llu \r\n",
     g_memMnt.totalSize, showInfo.totalUse, g_memMnt.totalSize-showInfo.totalUse);

}

/**********************************
函数名称    : memshow
作者        : Jeff.Zeng
设计日期    : 2007年12月6日
功能描述    : 显示mem 中配置信息
参数        : void
返回值        : XVOID
************************************/
XVOID memshow(void)
{
     t_MEMSHOW showInfo;

     printf(
     "mem usage  list: \r\n--------------------------------------------------\r\n");
     printf(
     "%-6s%-12s%-8s%-10s%-10s\r\n",
     "index",
     "blockSize",
     "maxCfg",
     "curUsage",
     "maxUsage"
     );
     XOS_MemSet(&showInfo, 0, sizeof(t_MEMSHOW));
     showInfo.index = 1;
     showInfo.totalUse = 0;

     /*打印数据*/
     XOS_HashWalk(g_memMnt.buckHash, memHashShow, (XVOID*)&showInfo);

     /*end of udp list */
     printf(
     "--------------------------------------------------\r\n total malloc: %llu  usage: %llu  leave: %llu \r\n",
     g_memMnt.totalSize, showInfo.totalUse, g_memMnt.totalSize-showInfo.totalUse);

}

/************************************************************************
函数名:XOS_GetMemContent
功能:  获取指定内存的指定长度数据到用户缓冲区
输入:title - 用户定义的标题
    mem －用户指定的内存地址
    len  －用户需要获取的内存数据的长度
    buf －用户缓冲区指针
    bufSize - 用户缓冲区大小
输出:
返回: 获取到的内存数据的长度
说明: 输出数据的16进制ASCII值到缓冲区
************************************************************************/
XU32 XOS_GetMemContent(
    XCONST XS8 *title,
    XCONST void *mem,
    XU32 len,
    XU8 *buf,
    XU32 bufSize
    )
{
    static XCONST XS8 *pn = "0123456789ABCDEF";

    XCONST XU8 *memAddr = (XCONST XU8*)(mem);
    XS8 *bufAddr = (XS8*)(buf);

    XS8 *bufCur = bufAddr;
    XS8 *bufEnd = bufCur + bufSize;

    XCONST XU8 *memCur = memAddr;
    XCONST XU8 *memEnd = memAddr + len;

    if (NULL == mem || 0 == bufSize)
    {
        return 0;
    }
    if (title)
    {
        XU32 titleLen = XOS_StrLen(title);
        if (titleLen + 1 >= bufSize)
        {
            return 0;
        }
        XOS_MemCpy(bufCur, title, titleLen);
        bufCur += titleLen;
    }

    bufCur += XOS_Sprintf(bufCur, (XU32)(bufEnd - bufCur)," len:%u\r\n", len);

    for (; bufCur + 3 <= bufEnd && memCur < memEnd; bufCur += 3, memCur++)
    {
        bufCur[0] = pn[*memCur >> 4];
        bufCur[1] = pn[*memCur & 0x0f];
        bufCur[2] = ' ';
    }

    if (bufCur >= bufEnd)
    {
        bufCur = bufEnd - 1;
    }

    *bufCur = '\0';

    return (XU32)(bufCur - bufAddr);
}

/************************************************************************
函数名:XOS_MemPrint
功能:  打印指定内存的指定长度数据到trace
输入: fid - 模块fid
    title - 用户定义的标题
    level - 打印级别
    mem －用户指定的内存地址
    len  －用户需要获取的内存数据的长度
输出:
返回: 获取到的内存数据的长度
说明: 输出数据的16进制ASCII值到trace
************************************************************************/
void XOS_MemPrint(
    XCONST XU32 fid,
    XCONST XS8 *title,
    e_PRINTLEVEL level,
    XCONST void *mem,
    XU32 len
    )
{
    XU8 outPutBuf[1024];
    if (len > 1024)
    {
        XOS_PRINT(MD(fid, level), "Too Long:%d", len);
        return;
    }
    XOS_GetMemContent(title, mem, len, outPutBuf, 1024);
    XOS_PRINT(MD(fid, level), "%s", outPutBuf);
}
#ifdef MEM_FID_DEBUG

XVOID memblockswill( XS32 size)

{

     XU32 blockSize;
     XS16 bits;
     t_BUCKETCB *pBuckCb;
     t_BLOCKHEAD *pBlockHead;
     XS32 i, j;

     blockSize = size;

     bits = MEM_getBitsNum(blockSize-1);

     pBuckCb = (t_BUCKETCB*)XNULLP;
     while(pBuckCb == XNULLP && bits <= MAX_BLOCK_BITS)
     {
          blockSize = 1<<bits;
          pBuckCb = XOS_HashElemFind(g_memMnt.buckHash, (XVOID*)&blockSize);
          bits++;
     }

     if(pBuckCb == XNULLP)
     {
//        printf( "sorry , not conifg this size memblocks !\n\r");
        return ;
     }
     printf(
     "blocksize[ %d] mem usage  info: \r\n--------------------------------------------------\r\n", blockSize);
     printf(
     "%-6s%-12s%-16s%-6s%-15s%-10s\r\n",
     "index",
     "userFid",
     "fileName",
     "line",
     "time",
     "useSize"
     );

     pBlockHead = (t_BLOCKHEAD*)XNULLP;
/*     XOS_MutexLock(&(pBuckCb->bucketLock));*/
     i = XOS_ArrayGetFirstPos(pBuckCb->blockArray);
     j = 0;
    while(i>=0)
    {
        pBlockHead = (t_BLOCKHEAD*)XOS_ArrayGetElemByPos(pBuckCb->blockArray, i);
        if(NULL == pBlockHead)
        {
            break;
        }
#ifdef XOS_ARCH_64
        printf(
        "%-6d%-12s%-16s%-6d%-15llu%-10d\r\n",
        j+1,
        XOS_getFidName(pBlockHead->fid),
        pBlockHead->fileName,
        pBlockHead->lineNum,
        (XUTIME)(pBlockHead->time),
        pBlockHead->memLen
        );
#else
        printf(
         "%-6d%-12s%-16s%-6d%-15u%-10d\r\n",
        j+1,
        XOS_getFidName(pBlockHead->fid),
        pBlockHead->fileName,
        pBlockHead->lineNum,
        (XUTIME)(pBlockHead->time),
        pBlockHead->memLen
        );
#endif
          i = XOS_ArrayGetNextPos(pBuckCb->blockArray, i);
          j++;
     }
/*     XOS_MutexUnlock(&(pBuckCb->bucketLock));*/
     /*end of  list */
     printf(
     "--------------------------------------------------\r\n total lists: %d\n\r", j);

}


/************************************************************************
 函数名:MEM_fidUsage
 功能: 显示mem 中fid 使用信息
 输入:
 输出:
 返回:
 说明: memfidusage命令的最终执行函数
************************************************************************/
XVOID MEM_fidUsage(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
     t_MEMSHOW showInfo;
     XU32 fid;
     XCHAR *pFidName;
     
    if(siArgc != 2)
    {
      XOS_CliExtPrintf(pCliEnv,"parameter num is wrong\r\n");
      return ;
    }

     fid = atol(ppArgv[1]);
     if(!XOS_isValidFid(fid))
     {
         XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
         return ;
     }
     
     pFidName = (XCHAR*)XNULLP;
     pFidName = XOS_getFidName(fid);
	 /*new操作时使用的fid为100*/
     if((pFidName == XNULLP) && (100 != fid))
     {
         XOS_CliExtPrintf(pCliEnv,"FID[ %d] is not reg !\r\n", fid);
         return;
     }
     XOS_CliExtPrintf(pCliEnv,
     "FID[ %s] mem usage  info: \r\n--------------------------------------------------\r\n", pFidName);
     XOS_CliExtPrintf(pCliEnv,
     "%-6s%-12s%-8s%-10s%-10s\r\n",
     "index",
     "blockSize",
     "maxCfg",
     "curUsage",
     "fidUsage"
     );
     XOS_MemSet(&showInfo, 0, sizeof(t_MEMSHOW));
     showInfo.index = 1;
     showInfo.pCliEnv = pCliEnv;
     showInfo.fid = fid;

     /*打印数据*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_fidShow, (XVOID*)&showInfo);

     /*end of udp list */
     XOS_CliExtPrintf(pCliEnv,
     "--------------------------------------------------\r\n");

}

/************************************************************************
 函数名:MEM_fidBlockSwill
 功能: 显示mem 中指定fid 与指定block的使用信息
 输入:
 输出:
 返回:
 说明: 
************************************************************************/
XVOID MEM_fidBlockSwill(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XU32 uFid = 0;
    XU32 blockSize = 0;
    XS16 bits;
    t_BUCKETCB *pBuckCb = NULL;
    t_BLOCKHEAD *pBlockHead = NULL;
    XS32 i, j;
    
    if(siArgc != 3 )
    {
         XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
         return ;
    }

    uFid = atol(ppArgv[1]);
    if(!XOS_isValidFid(uFid))
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
        return ;
    }
    blockSize = atol(ppArgv[2]);

    bits = MEM_getBitsNum(blockSize-1);

    pBuckCb = (t_BUCKETCB*)XNULLP;
    while(pBuckCb == XNULLP && bits <= MAX_BLOCK_BITS)
    {
        blockSize = 1<<bits;
        pBuckCb = XOS_HashElemFind(g_memMnt.buckHash, (XVOID*)&blockSize);
        bits++;
    }

    if(pBuckCb == XNULLP)
    {
        XOS_CliExtPrintf(pCliEnv,"sorry , not conifg this size memblocks !\n\r");
        return ;
    }
    XOS_CliExtPrintf(pCliEnv,
    "blocksize[ %d] mem usage  info: \r\n--------------------------------------------------\r\n", blockSize);
    XOS_CliExtPrintf(pCliEnv,
    "%-6s%-18s%-22s%-6s%-11s%-10s     %-16s\r\n",
    "index",
    "userFid",
    "fileName",
    "line",
    "time",
    "useSize",
    "mem address"
    );

    pBlockHead = (t_BLOCKHEAD*)XNULLP;
    /*     XOS_MutexLock(&(pBuckCb->bucketLock));*/
    i = XOS_ArrayGetFirstPos(pBuckCb->blockArray);
    j = 0;
    while(i>=0)
    {
        pBlockHead = (t_BLOCKHEAD*)XOS_ArrayGetElemByPos(pBuckCb->blockArray, i);
        if(uFid == pBlockHead->fid)
        {
            XOS_CliExtPrintf(pCliEnv,
            "%-6d%-18s%-22s%-6d%-11d%-10d     0x%p\r\n",
            ++j,
            XOS_getFidName(pBlockHead->fid),
            pBlockHead->fileName,
            pBlockHead->lineNum,
            (XUTIME)(pBlockHead->time),
            pBlockHead->memLen,
            (XCHAR *)pBlockHead+sizeof(t_BLOCKHEAD)
            );
        }

        i = XOS_ArrayGetNextPos(pBuckCb->blockArray, i);
    }
    /*     XOS_MutexUnlock(&(pBuckCb->bucketLock));*/
    /*end of  list */
    XOS_CliExtPrintf(pCliEnv,
    "--------------------------------------------------\r\nfid=%d, bits=%d, total lists: %d\n\r", uFid, bits, j);
}

/************************************************************************
 函数名:MEM_swill
 功能: 显示mem 中内存块使用信息，用来调试内存泄漏
 输入:
 输出:
 返回:
 说明: memswill命令的最终执行函数
************************************************************************/
XVOID MEM_swill(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv, XS32 size)
{

     XU32 blockSize;
     XS16 bits;
     t_BUCKETCB *pBuckCb;
     t_BLOCKHEAD *pBlockHead;
     XS32 i, j;

     if(size == 0)
     {        
        if(siArgc != 2 )
        {
             XOS_CliExtPrintf(pCliEnv,"input parameter is wrong\r\n");
             return ;
        }
        blockSize = atol(ppArgv[1]);
     }
     else
     {
         blockSize = size;
     }

     bits = MEM_getBitsNum(blockSize-1);

     pBuckCb = (t_BUCKETCB*)XNULLP;
     while(pBuckCb == XNULLP && bits <= MAX_BLOCK_BITS)
     {
          blockSize = 1<<bits;
          pBuckCb = XOS_HashElemFind(g_memMnt.buckHash, (XVOID*)&blockSize);
          bits++;
     }

     if(pBuckCb == XNULLP)
     {
        XOS_CliExtPrintf(pCliEnv,"sorry , not conifg this size memblocks !\n\r");
        return ;
     }
     XOS_CliExtPrintf(pCliEnv,
     "blocksize[ %d] mem usage  info: \r\n--------------------------------------------------\r\n", blockSize);
     XOS_CliExtPrintf(pCliEnv,
     "%-6s%-18s%-22s%-6s%-11s%-10s     %-16s\r\n",
     "index",
     "userFid",
     "fileName",
     "line",
     "time",
     "useSize",
     "mem address"
     );

     pBlockHead = (t_BLOCKHEAD*)XNULLP;
/*     XOS_MutexLock(&(pBuckCb->bucketLock));*/
     i = XOS_ArrayGetFirstPos(pBuckCb->blockArray);
     j = 0;
    while(i>=0)
    {
        pBlockHead = (t_BLOCKHEAD*)XOS_ArrayGetElemByPos(pBuckCb->blockArray, i);
		/*遍历时可能被释放了*/
        if(NULL == pBlockHead)
        {
            break;
        }
        
        XOS_CliExtPrintf(pCliEnv,
                        "%-6d%-18s%-22s%-6d%-11d%-10d     0x%p\r\n",
                        j+1,
                        XOS_getFidName(pBlockHead->fid),
                        pBlockHead->fileName,
                        pBlockHead->lineNum,
                        (XUTIME)(pBlockHead->time),
                        pBlockHead->memLen,
                        (XCHAR *)pBlockHead+sizeof(t_BLOCKHEAD)
                        );

        i = XOS_ArrayGetNextPos(pBuckCb->blockArray, i);
        j++;

        MEM_CMD_SHOW_CTL(j);
    }
    /*     XOS_MutexUnlock(&(pBuckCb->bucketLock));*/
    /*end of  list */
    XOS_CliExtPrintf(pCliEnv,
    "--------------------------------------------------\r\n total lists: %d\n\r", j);

}


/************************************************************************
 函数名:MEM_blocksSwill
 功能: 显示mem 中内存块使用信息，用来调试内存泄漏
 输入:
 输出:
 返回:
 说明: memswill命令的最终执行函数
************************************************************************/
XVOID MEM_blocksSwill(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
      MEM_swill( pCliEnv,  siArgc,  ppArgv, 0);
}

/************************************************************************
 函数名:MEM_allSwill
 功能: 显示所有mem 中内存块使用信息，用来调试内存泄漏
 输入:
 输出:
 返回:
 说明: memswill命令的最终执行函数
************************************************************************/
XVOID MEM_allSwill(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
     XU32 i;
     t_MEMBLOCK *pMemBlock;

     for(i=0; i< g_memMnt.buckTypes; i++)
     {
         pMemBlock = (t_MEMBLOCK*)(g_memMnt.pBlockPtr)+i;
         MEM_swill( pCliEnv,  siArgc,  ppArgv, pMemBlock->blockSize);
         XOS_CliExtPrintf(pCliEnv,"\n\r\n\r ===================================================\n\r\r\n");
     }
}

/************************************************************************
 函数名:MEM_allSwill
 功能: 显示所有mem 中内存块使用信息，用来调试内存泄漏
 输入:
 输出:
 返回:
 说明: memswill命令的最终执行函数
************************************************************************/
XVOID memallswill(void)
{
     XU32 i;
     t_MEMBLOCK *pMemBlock;

     for(i=0; i< g_memMnt.buckTypes; i++)
     {
         pMemBlock = (t_MEMBLOCK*)(g_memMnt.pBlockPtr)+i;
         memblockswill(  pMemBlock->blockSize);
         printf( "\n\r\n\r ===================================================\n\r\r\n");
     }
}

/************************************************************************
 函数名:MEM_StackShow
 功能: 显示该块内存在分配和释放时的函数调用栈信息
 输入:
 输出:
 返回:
 说明: memstackshow 命令的最终执行函数
************************************************************************/

XVOID MEM_StackShow(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    XPOINT uPtr = 0;

    if (siArgc > 2)
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter number is wrong!\r\n");
        return ;
    }

    if (atol(ppArgv[1]) < 0)
    {
        XOS_CliExtPrintf(pCliEnv,"input parameter is wrong!\r\n");
        return ;
    }

    uPtr = (XPOINT)strtopointer(ppArgv[1],NULL,0);
    if(uPtr == 0)
    {
        XOS_CliExtPrintf(pCliEnv,"strtopointer failed.input parameter is wrong!\r\n");
        return;
    }

        MEM_Ptr_Show(pCliEnv,uPtr);

    return;

}

XS16 MEM_Ptr_Show(CLI_ENV* pCliEnv, XPOINT pPtr)
{
    XS32 k = 0;
    XCHAR pFuncName[MAX_STACK_FUN_NAME + 1]  = {0};
    XS32 i = 0;
    XS32 j = 0;
    t_BUCKPTR *pBuckPtr = XNULLP;
    t_BUCKETCB *pBuckCb = XNULLP;
    t_BLOCKHEAD *pBlockHead = XNULLP;
    t_BLOCKTAIL  *pBlockTail = XNULLP;

    /*安全性检查*/
    if(pPtr == XNULL || !g_memMnt.initialized)
    {
         XOS_CliExtPrintf(pCliEnv,"mem address is wrong\r\n");

          return XERROR;
    }
    /*检查内存首地址的准确性*/
    /*二分查找获取指针所在的array*/
    i =  0;
    j = g_memMnt.buckTypes-1;
    while(i <= j)
    {
        pBuckPtr = g_memMnt.pElements+((i+j)/2);
        pBuckCb = (t_BUCKETCB*)XNULLP;
        pBuckCb = XOS_HashGetElem(g_memMnt.buckHash, (XVOID *)(pBuckPtr->pLocation));
        if(pBuckCb == XNULLP)
        {
             XOS_CliExtPrintf(pCliEnv,"can not find mem in hash!\r\n");
              return XERROR;
        }

        if((XPOINT)pPtr >= (XPOINT)(pBuckCb->headAddr)
           &&(XPOINT)pPtr <= (XPOINT)(pBuckCb->tailAddr))
        {
             /*作安全性验证*/
             /*前越界验证*/
             pBlockHead = (t_BLOCKHEAD*)(pPtr-(sizeof(t_BLOCKHEAD)));//(XCHAR*)
             //pBlockHead = (t_BLOCKHEAD*)((XCHAR*)pPtr-(sizeof(t_BLOCKHEAD)));
                 //pBlockHead = (t_BLOCKHEAD*)(XCHAR*)pPtr;
             if(pBlockHead->headCheck != MEM_MAGIC_VALUE
                 || pBlockHead->memLen > pBuckCb->blockSize)
             {
                 XOS_CliExtPrintf(pCliEnv,"the mem address [%p] is wrong!\r\n", pPtr);

                 return XERROR;
             }
             /*后越界验证*/
           //pBlockTail = (t_BLOCKTAIL*)((XCHAR*)pPtr + pBlockHead->memLen);
                    pBlockTail = (t_BLOCKTAIL*)((XCHAR*)pBlockHead +sizeof(t_BLOCKHEAD)+ pBlockHead->memLen);
             if(pBlockTail->tailCheck != MEM_MAGIC_TAIL)
             {
                  XOS_CliExtPrintf(pCliEnv,"the mem address [%p] is wrong!\r\n", pPtr);

                  return XERROR;
             }

            /*首地址正确输出调用栈*/
            XOS_CliExtPrintf(pCliEnv,"Allocation Call trace: \r\n");
            while ( 0 != pBlockHead->memstack.AllocFunc_ST[k] && k < MAX_MEM_STACK_DEPTH )

            {
                /*if ( ERROR != symFindByValue(sysSymTbl, (U32)pMemInfo->adwAllocFunc[k], pFuncName, &iSymValue, &iSymType) )*/

                if ( XSUCC == MEM_findSymByAddr((XPOINT)pBlockHead->memstack.AllocFunc_ST[k], pFuncName))
                {
                      XOS_CliExtPrintf(pCliEnv,"^%s(0x%x)\r\n", pFuncName, (XPOINT)pBlockHead->memstack.AllocFunc_ST[k]);
                }

                k++;
            }
            XOS_CliExtPrintf(pCliEnv,"\r\n");

            k = 0;

            XOS_CliExtPrintf(pCliEnv,"Free call trace: \r\n");

            while (0 != pBlockHead->memstack.FreeFunc_ST[k] && k < MAX_MEM_STACK_DEPTH)
            {
                /*if ( ERROR != symFindByValue(sysSymTbl, (U32)pMemInfo->adwFreeFunc[k], pFuncName, &iSymValue, &iSymType) )*/

                if ( XSUCC == MEM_findSymByAddr((XPOINT)pBlockHead->memstack.FreeFunc_ST[k], pFuncName))
                {
                      XOS_CliExtPrintf(pCliEnv,"^%s(0x%x)\r\n", pFuncName, (XPOINT)pBlockHead->memstack.FreeFunc_ST[k]);
                }

                k++;
            }
            XOS_CliExtPrintf(pCliEnv,"\r\n");
            return XSUCC ;

        }

                /*在上半部继续找*/
        if((XPOINT)pPtr >(XPOINT)(pBuckCb->tailAddr))
        {
            i = (i+j)/2+1;
            continue;
        }

        /*在下半部分继续找*/
        if((XPOINT)pPtr < (XPOINT)pBuckCb->headAddr)
        {
            j = (i+j)/2 -1;
            continue;
        }

    }
    XOS_CliExtPrintf(pCliEnv,"the address is wrong ,can not find !\r\n");
    return XERROR ;
}

/*************************************************************\
modified by lixn 2007 - 6 -11
for global mem check

**************************************************************/

void mem_print(unsigned char* read_mem, int len)
{
     int i = 0;
     unsigned char lhex, rhex;

     len=len+160;//print more 160 char
     printf("mem print begin len=%d\n",len);
     printf("\n address %p: ",&read_mem[i]);

     while(i<len)
     {
       lhex =  (read_mem[i] & 0xf0) >> 4;
       rhex =   read_mem[i] & 0x0f ;
       printf("%x%x ",lhex,rhex);
       i++;
       if(i % 16 == 0)
           printf("\n address %p: ",&read_mem[i]);

     }
    printf("\n");
    printf("mem print end\n");
}

/************************************************************************
函数名: xosMemCheck()
功能：  memory check function for debug
说明：  返回值由用户强制转换
************************************************************************/
XVOID xosMemCheck()
{

    XS16 bits = 0;
    XS32 key = 0;
    t_BUCKETCB *pBuckCb = XNULLP;
    t_BLOCKHEAD *pBlockHead = XNULLP;
    t_BLOCKHEAD *pBlockWrtHead = XNULLP;
    t_BLOCKTAIL  *pBlockTail = XNULLP;
    t_XOSARRAY *pblkArray = XNULLP;
    int location=0;
    int locationWrt=0;
    int emptyNum=0;
    int checkOkNum=0;

    /* add by wulei 2006.12.08 */
    if (!g_memMnt.initialized)
    {
        printf("******xos memMnt not initialized,just return!\r\n");
        return;
    }

    /*正常状况下,第一次应该可以找到*/

    if(bits > g_memMnt.maxBuckBits )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR),"MEM_getBitsNum()->the maxbytes is [%d], the bits is[%d]. ",
                 g_memMnt.maxBuckBits, bits);

    }

    printf("******the blocks memCheck is beginning!\r\n");

    for(; bits <= g_memMnt.maxBuckBits; bits++)
    {
        pBuckCb = (t_BUCKETCB*)XNULLP;
        key = (1<<bits);
        pBuckCb = (t_BUCKETCB*)XOS_HashElemFind(g_memMnt.buckHash, (XVOID *)&key);
        if(pBuckCb != XNULLP) /*找到*/
        {
             pBlockHead = (t_BLOCKHEAD*)XNULLP;
             //XOS_MutexLock(&(pBuckCb->bucketLock));
             //ret = XOS_ArrayAddExt(pBuckCb->blockArray, (XOS_ArrayElement*)&pBlockHead);

             pblkArray =(t_XOSARRAY *) pBuckCb->blockArray;
             if(XNULLP == pblkArray)
             {
                 printf("pBuckCb[blockSize=%d,key=%d]->blockArray is NULL\r\n",pBuckCb->blockSize,key);
                 continue ;
             }

            location = 0;

            printf("******pblkArray[blockSize=%d]->pBlockHead[location=%d] is check beginning!\r\n",
                     pBuckCb->blockSize,location);

           for(location = 0;location < (pblkArray->maxNumOfElements);location++)
           {
             /*  该位置是否有元素占用,just continue */
            if (XOS_ArrayElemIsVacant(pblkArray, location) == XTRUE)
            {
                emptyNum++;
                continue ;
            }

             pBlockHead= (XOS_ArrayElement)ELEM_DATA(pblkArray, location);
             if(pBlockHead == XNULLP)
             {
                 printf("pblkArray[blockSize=%d]->pBlockHead[location=%d] is NULL\r\n",
                     pBuckCb->blockSize,location);
             }
             else if((MEM_MAGIC_VALUE != pBlockHead->headCheck)
                 ||( pBlockHead->memLen > pBuckCb->blockSize))
             {
                     printf("pblkArray[fid=%d,blockSize=%d]->pBlockHead[location=%d,memLen=%d]->headCheck is writed as 0x%x\r\n",
                         pBlockHead->fid,pBuckCb->blockSize,location,pBlockHead->memLen,pBlockHead->headCheck);
                     if(location>0)
                     {
                         locationWrt=location-1;
                         pBlockWrtHead =(XOS_ArrayElement)ELEM_DATA(pblkArray, locationWrt);
                         printf("***pBlockWrtHead info ***\r\n");
                         printf("***pBlockWrtHead alloc_info fileName=%s;lineNum=%d***\r\n",
                             pBlockWrtHead->fileName,pBlockWrtHead->lineNum);
                         mem_print((unsigned char*)pBlockWrtHead, pBuckCb->blockSize+sizeof(t_BLOCKHEAD)+sizeof(t_BLOCKTAIL));

                     }/*
                     else
                     (
                         printf("***pBlockWrtHead info but location=0!****\r\n");
                     )*/

             }
             else
             {
                 pBlockTail = (t_BLOCKTAIL*)(((XCHAR*)pBlockHead)+(sizeof(t_BLOCKHEAD)+pBlockHead->memLen));
                 if(pBlockTail->tailCheck != MEM_MAGIC_TAIL)
                 {
                     printf("pblkArray[fid=%d,blockSize=%d]->pBlockHead[location=%d]->tailCheck is writed as 0x%x\r\n",
                         pBlockHead->fid,pBuckCb->blockSize,location,pBlockTail->tailCheck);
                     if(location>0)
                     {
                         pBlockWrtHead =pBlockHead;
                         printf("***pBlockWrtHead info ***\r\n");
                         printf("pblkArray[fid=%d,blockSize=%d]->pBlockWrtHead[locationWrt=%d,memLen=%d]->headCheck is  0x%x\r\n",
                         pBlockWrtHead->fid,pBuckCb->blockSize,locationWrt,pBlockWrtHead->memLen,pBlockWrtHead->headCheck);

                             printf("***pBlockWrtHead alloc_info fileName=%s;lineNum=%d***\r\n",
                             pBlockWrtHead->fileName,pBlockWrtHead->lineNum);
                         mem_print((unsigned char*)pBlockWrtHead, pBuckCb->blockSize+sizeof(t_BLOCKHEAD)+sizeof(t_BLOCKTAIL));

                     }
                 }

                 checkOkNum++;
             }

          }

          printf("******pblkArray[blockSize=%d]->pBlockHead[location=%d] is check finished! and checkOkNum=%d,emptyNum=%d\r\n",
                     pBuckCb->blockSize,location,checkOkNum,emptyNum);
          emptyNum=0;
          checkOkNum=0;
        }
    }

    printf("******all the blocks are check finished!\r\n");
}
#endif /*MEM_FID_DEBUG*/

#ifdef XOS_WIN32
#ifdef XOS_ARCH_64
/************************************************************************
 函数名:XOS_CaptureStackBackTrace
 功能: win_64下获取函数调用堆栈
 输入:
 输出:
 返回:
 说明: 
************************************************************************/
void XOS_CaptureStackBackTrace( XPOINT *pBuffer, XU32 nDepth )
{
     unsigned int   i = 0;
     void         * stack[ 100 ] = {0x0};
     unsigned short frames = 0;
     SYMBOL_INFO  * symbol = NULL;
     HANDLE         process = NULL;

     process = GetCurrentProcess();

     SymInitialize( process, NULL, TRUE );

     frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
     symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
     symbol->MaxNameLen   = 255;
     symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

     for( i = 0; i < frames && i < nDepth; i++ )
     {
         SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );

         //printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
         pBuffer[i] = (XPOINT)symbol->Address;
     }
     if(i < nDepth)
     {
        pBuffer[i] = 0;
     }

     free( symbol );
}
#endif
#endif

/************************************************************************
 函数名:MEM_CallTrace
 功能: 显示内存调用栈
 输入:
 输出:
 返回:
 说明: vxworks下只适用于cpu = ppc860, OS = T22/T2;
      windows下适合x86、x84_64;
      linux 支持x86_32、x86_64
************************************************************************/

XU32 MEM_CallTrace(XPOINT *pBuffer, XU32 nDepth)
{
/*使用的是汇编语言，所以要考虑cpu类型，系统型号等
*/

    XPOINT         iBp = 0;
    XPOINT         iPc = 0;
    XU32         i=0;

#ifdef XOS_VXWORKS

    WIND_TCB     *pTaskTcb;

    pTaskTcb = taskTcb(taskIdSelf());
    if ( NULL == pTaskTcb)
    {
        pBuffer[i] = 0;
        return XERROR ;
    }

#ifdef XOS_VX_PPC860
    /* Use following inline assembly to get current bp */
  #ifdef XOS_T22
    __asm__("stw  %%r31, %0" : "=m" (iBp));
 #elif defined(XOS_T2)
    __asm__("stw  31, %0" : "=m" (iBp));

 #endif

    //__asm__("stw  %%r31, %0" : "=m" (iBp));

    /* Adjust to next frame */
    iPc = *(XPOINT *)(iBp+4);
    iBp = *(XPOINT *)iBp;
    iPc = *(XPOINT *)(iBp+4);
    iBp = *(XPOINT *)iBp;
    i = 0;
    while ( i < nDepth )
    {
        if ( iBp >= (XPOINT)pTaskTcb->pStackBase )
        {
            break;
        }

        pBuffer[i] = iPc;

        iPc = *(XPOINT *)(iBp+4);
        iBp = *(XPOINT *)iBp;
        i++;

        if ( 0 == iBp )
            break;
    }
    if ( i < nDepth )
    {
        pBuffer[i] = 0;
    }

#else
   return XERROR ;
#endif/*_XOS_VX_PPC860_*/
#endif /*_XOS_VXWORKS_*/
#ifdef XOS_WIN32
#ifndef XOS_ARCH_64

/*    CONTEXT        stContext; */

    /* Use following inline assembly to get current pc and bp */
    __asm
    {
        mov     iBp, ebp ;
        mov        i, eax ;
        call    getcurip ;
getcurip:
        pop        eax ;
        mov        iPc, eax ;
        mov        eax, i ;
    }

    i = 0;
    /* Adjust to next frame */
    iPc = *(XPOINT *)(iBp+4);
    iBp = *(XPOINT *)iBp;

    while ( i < nDepth )
    {

        pBuffer[i] = iPc;

        /* Adjust to next frame */
        iPc = *(XPOINT *)(iBp+4);
        iBp = *(XPOINT *)iBp;
        i++;

        if ( 0 == iBp )
            break;

    }
    if ( i < nDepth )
    {
        pBuffer[i] = 0;
    }

#else
   XOS_UNUSED(iBp);
   XOS_UNUSED(iPc);
   XOS_UNUSED(i);
   XOS_CaptureStackBackTrace(pBuffer, nDepth);
#endif
#endif /*_XOS_WIN32_*/

#ifdef XOS_LINUX
    XOS_UNUSED(iBp);
    XOS_UNUSED(iPc);
    XOS_UNUSED(i);
    backtrace((XVOID**)pBuffer, nDepth);
#if 0
#ifdef XOS_ARCH_64
    __asm__ __volatile__("mov %%rbp, %0":"=g"(iBp)::"memory");

    i=0;
    iPc = *(XPOINT*)(iBp+8);
    iBp = *(XPOINT*)iBp;

    while(i<nDepth)
    {
        pBuffer[i] = iPc;
        iPc = *(XPOINT*)(iBp+8);
        iBp = *(XPOINT*)iBp;
        i++;
        
        if(0 == iBp)
            break;
    }
    
    if ( i < nDepth )
    {
        pBuffer[i] = 0;
    }
#else
     __asm__ __volatile__("movl %%ebp, %0":"=g"(iBp)::"memory");

    i=0;
    iPc = *(int*)(iBp+4);
    iBp = *(int*)iBp;

    while(i<nDepth)
    {
        pBuffer[i] = iPc;
        iPc = *(int*)(iBp+4);
        iBp = *(int*)iBp;
        i++;
        
        if(0 == iBp)
            break;
    }
    
    if ( i < nDepth )
    {
        pBuffer[i] = 0;
    }
#endif

#endif
#endif
    return XSUCC;
}



XSTATIC XS32 MEM_findSymByAddr( XPOINT addr,XCHAR* symbolName )
{
#ifdef XOS_VXWORKS
    XS32 result;
    XS32 value;
    SYM_TYPE symTyp;
 #ifdef XOS_T22
    XCHAR  *pName;
 #endif

    if( !addr || !symbolName )
    {
        return XERROR ;
    }

 #ifdef XOS_T22
    result = symByValueFind(sysSymTbl,addr,&pName,&value, &symTyp);
 #elif defined(T2)
    result = symFindByValue(sysSymTbl,addr,symbolName,&value, &symTyp);
 #endif
    if(XERROR == result)
    {
        return XERROR;
    }

 #ifdef XOS_T22
 XOS_StrNcpy(symbolName, pName, MAX_STACK_FUN_NAME);
     free(pName);
 #endif

    return XSUCC;
#endif
#ifdef XOS_WIN32
    XS8 Image[256];
#ifdef XOS_ARCH_64
    IMAGEHLP_SYMBOL64* pImage=(IMAGEHLP_SYMBOL64*)Image;

    if( !addr || !symbolName )
    {
        return XERROR;
    }
    XOS_MemSet(Image, 0, 256);
    pImage->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    pImage->Address = addr;
    pImage->Size = 0;
    pImage->Flags = SYMF_OMAP_GENERATED;
    pImage->MaxNameLength = MAX_STACK_FUN_NAME;

    if(!SymGetSymFromAddr64(GetCurrentProcess(),addr,0,pImage))
    {
        /*printf( "\nSymGetSymFromAddr Failed!\n" );*/
        return XERROR;
    }
     XOS_StrNcpy(symbolName, pImage->Name, MAX_STACK_FUN_NAME);
#else
    IMAGEHLP_SYMBOL* pImage=(IMAGEHLP_SYMBOL*)Image;

    if( !addr || !symbolName )
    {
        return XERROR;
    }
    XOS_MemSet(Image, 0, 256);
    pImage->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    pImage->Address = addr;
    pImage->Size = 0;
    pImage->Flags = SYMF_OMAP_GENERATED;
    pImage->MaxNameLength = MAX_STACK_FUN_NAME;

    if(!SymGetSymFromAddr(GetCurrentProcess(),addr,0,pImage))
    {
        /*printf( "\nSymGetSymFromAddr Failed!\n" );*/
        return XERROR;
    }
     XOS_StrNcpy(symbolName, pImage->Name, MAX_STACK_FUN_NAME);
#endif

     return XSUCC ;
#endif /*WIN32*/

#ifdef XOS_LINUX
    XVOID *array[1];
    XS8 **string = NULL;
    XS8 name[256] = {0};
    XS8 str1[256] = {0};
    XS8 str2[256] = {0};

    XOS_MemSet(name, 0, 256);

    array[0] = (void *)addr;
    string = backtrace_symbols(array,1);
    
    if(string != NULL)
    {
        //XOS_StrCpy(name,string[0]);
        sscanf(string[0],"%[^(](%[^+]%s",str1,name,str2);
    }
    free(string);
    
    XOS_StrNcpy(symbolName, name, MAX_STACK_FUN_NAME);
    return XSUCC ;
#endif
}


#if 1  
//以下函数为VXWORKS下在23端口内存相关定位的手段
/************************************************************************
 函数名:meminfo
 功能: 显示mem 中配置信息
 输入:
 输出:
 返回:
 说明: 
************************************************************************/
XVOID meminfo(XVOID)
{
     t_MEMSHOW showInfo;

     printf("mem usage  list: \r\n--------------------------------------------------\r\n");
     printf("%-6s%-12s%-8s%-10s%-10s\r\n",
     "index",
     "blockSize",
     "maxCfg",
     "curUsage",
     "maxUsage"
     );
     XOS_MemSet(&showInfo, 0, sizeof(t_MEMSHOW));
     showInfo.index = 1;
     showInfo.pCliEnv = XNULL;
     showInfo.totalUse = 0;

     /*打印数据*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_hashShow, (XVOID*)&showInfo);

     /*end of udp list */
     printf("--------------------------------------------------\r\n total malloc: %llu  usage: %llu  leave: %llu \r\n",
     g_memMnt.totalSize, showInfo.totalUse, g_memMnt.totalSize-showInfo.totalUse);

     return;
}

#ifdef MEM_FID_DEBUG
/************************************************************************
 函数名:memfid
 功能: 显示mem 中fid 使用信息
 输入:
 输出:
 返回:
 说明: memfidusage命令的最终执行函数
************************************************************************/
XVOID memfid(XU32 fid)
{
     t_MEMSHOW showInfo;
     XCHAR *pFidName;
     
     if(!XOS_isValidFid(fid))
     {
         printf("input parameter is wrong\r\n");
         return ;
     }
     
     pFidName = (XCHAR*)XNULLP;
     pFidName = XOS_getFidName(fid);
     if(pFidName == XNULLP)
     {
         printf("FID[ %d] is not reg !\r\n", fid);
         return;
     }
     printf("FID[ %s] mem usage  info: \r\n--------------------------------------------------\r\n", pFidName);
     printf("%-6s%-12s%-8s%-10s%-10s\r\n",
     "index",
     "blockSize",
     "maxCfg",
     "curUsage",
     "fidUsage"
     );
     XOS_MemSet(&showInfo, 0, sizeof(t_MEMSHOW));
     showInfo.index = 1;
     showInfo.pCliEnv = XNULLP;
     showInfo.fid = fid;

     /*打印数据*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_fidShow, (XVOID*)&showInfo);

     /*end of udp list */
     printf("--------------------------------------------------\r\n");
     return;
}

/************************************************************************
 函数名:memblock
 功能: 显示mem 中内存块使用信息，用来调试内存泄漏
 输入:
 输出:
 返回:
 说明: 
************************************************************************/
XVOID memblock(XS32 size)
{

     XU32 blockSize;
     XS16 bits;
     t_BUCKETCB *pBuckCb;
     t_BLOCKHEAD *pBlockHead;
     XS32 i, j;

     if(size <= 0)
     {
           printf("input parameter is wrong\r\n");
           return ;
     }

     bits = MEM_getBitsNum(size-1);

     pBuckCb = (t_BUCKETCB*)XNULLP;
     while(pBuckCb == XNULLP && bits <= MAX_BLOCK_BITS)
     {
          blockSize = 1<<bits;
          pBuckCb = XOS_HashElemFind(g_memMnt.buckHash, (XVOID*)&blockSize);
          bits++;
     }

     if(pBuckCb == XNULLP)
     {
        printf("sorry , not conifg this size memblocks !\n\r");
        return ;
     }
     printf("blocksize[ %d] mem usage  info: \r\n--------------------------------------------------\r\n", blockSize);
     printf("%-6s%-18s%-22s%-6s%-11s%-10s     %-16s\r\n",
     "index",
     "userFid",
     "fileName",
     "line",
     "time",
     "useSize",
     "mem address"
     );

     pBlockHead = (t_BLOCKHEAD*)XNULLP;
     i = XOS_ArrayGetFirstPos(pBuckCb->blockArray);
     j = 0;
     while(i>=0)
     {
          pBlockHead = (t_BLOCKHEAD*)XOS_ArrayGetElemByPos(pBuckCb->blockArray, i);

#ifdef XOS_ARCH_64
          printf("%-6d%-18s%-22s%-6d%-11llu%-10d     %p\r\n",
          j+1,
          XOS_getFidName(pBlockHead->fid),
          pBlockHead->fileName,
          pBlockHead->lineNum,
          (XUTIME)(pBlockHead->time),
          pBlockHead->memLen,
          (XCHAR *)pBlockHead+sizeof(t_BLOCKHEAD)
          );
#else
        printf("%-6d%-18s%-22s%-6d%-11u%-10d     %p\r\n",
        j+1,
        XOS_getFidName(pBlockHead->fid),
        pBlockHead->fileName,
        pBlockHead->lineNum,
        (XUTIME)(pBlockHead->time),
        pBlockHead->memLen,
        (XCHAR *)pBlockHead+sizeof(t_BLOCKHEAD)
        );
#endif
          i = XOS_ArrayGetNextPos(pBuckCb->blockArray, i);
          j++;
     }

     printf("--------------------------------------------------\r\n total lists: %d\n\r", j);
     return;
}

/************************************************************************
 函数名:memallblock
 功能: 显示所有mem 中内存块使用信息，用来调试内存泄漏
 输入:
 输出:
 返回:
 说明: 
************************************************************************/
XVOID memallblock(XVOID)
{
     XU32 i;
     t_MEMBLOCK *pMemBlock;

     for(i=0; i< g_memMnt.buckTypes; i++)
     {
         pMemBlock = (t_MEMBLOCK*)(g_memMnt.pBlockPtr)+i;
         memblock(pMemBlock->blockSize);
         printf("\n\r\n\r ===================================================\n\r\r\n");
     }
     return;
}

/************************************************************************
 函数名:memstack
 功能: 显示该块内存在分配和释放时的函数调用栈信息
 输入:
 输出:
 返回:
 说明: 
************************************************************************/

XVOID memstack(XPOINT uPtr)
{
    if(uPtr == 0)
    {
        printf("parameter is wrong!\r\n");
        return;
    }

    MEM_Ptr_Show(XNULLP,uPtr);
    return;
}

#endif

XVOID memhelp()
{
    printf("\n command List:");
    printf("\n meminfo     para:no          -显示内存信息");
 #ifdef MEM_FID_DEBUG
    printf("\n memfid      para:fid         -显示指定FID使用内存信息");
    printf("\n memblock    para:blocksize   -显示指定BLOCKSIZE内存块使用信息");   
    printf("\n memallblock para:no          -显示所有内存块使用信息");
    printf("\n memstack    para:addr        -显示指定地址的调用栈信息");
 #endif
    return ;
}

/************************************************************************
函数名: write_to_syslog
功能：  写记录到syslog
输入：  fid           - 功能块id
输出：  N/A
返回：  XVOID 
说明： 
************************************************************************/
XS32 write_to_syslog(const XS8* msg,...)
{
    XCHAR msg_format[4096]  = {0}; 
    XS8*  ptr = NULL;
    XU32  msg_curlen  = 0;
    XS32  len = 0;
    FILE * fpw = NULL;
    va_list ap;

    if((len = TRC_formateloctime(msg_format)) <= 0)
    {
        return XERROR;
    }
 
    ptr = msg_format + len;
    *ptr++ = ' ';
    len++;
    
    va_start(ap, msg);
    msg_curlen = XOS_VsPrintf(ptr, sizeof(msg_format) - len - 1, msg, ap);
    va_end(ap);
    
    if (msg_curlen <= 0) 
    {
        return XERROR;
    }
    
    msg_curlen += len;
    msg_format[msg_curlen++] = '\n';
    
#ifdef XOS_VXWORKS
    fpw = fopen( "/ata0a/syslog.txt", "a+" );
#elif defined(XOS_WIN32)
    mkdir("log");
    fpw = fopen( "log/syslog.txt", "a+" );
#elif defined(XOS_LINUX)
    mkdir("log",S_IRWXU);
    fpw = fopen( "log/syslog.txt", "a+" );
#endif
    if(fpw != NULL) {
        ptr = msg_format;
        while(msg_curlen > 0) {
            len = (XS32)fwrite(ptr, 1, msg_curlen, fpw);
            msg_curlen -= len;
            ptr += len;
        }
        fclose(fpw);
        return XSUCC;
    }
    return XERROR;
}

/************************************************************************
函数名: XosStackDumpToSyslog
功能：  将函数调用栈信息记录到syslog
输入：  pszBuf - 提示信息
输出：  N/A
返回：  XVOID 
说明： 
************************************************************************/
XVOID XosStackDumpToSyslog(const char *pszBuf)
{
    XCHAR pFuncName[MAX_STACK_FUN_NAME+1]  = {0};
    XPOINT funcAddr[MAX_MEM_STACK_DEPTH+1] ={0};
    XS32 idx = 0;
    XCHAR stackLog[2048] = {0};
    XCHAR buf[255] = {0};

    if(NULL == pszBuf)
    {
        return ;
    }
    
    XOS_Sprintf(stackLog, sizeof(stackLog), 
                "vxworks api vsprintf stack overflow:\r\n%s\r\n", pszBuf);
    
    /*获取函数地址*/
    MEM_CallTrace(funcAddr, MAX_MEM_STACK_DEPTH);
    
    for(idx = 0; idx < MAX_MEM_STACK_DEPTH && funcAddr[idx] != 0; idx++) 
    {
        memset(pFuncName, 0, sizeof(pFuncName));
        /*根据函数地址获取函数名*/
        MEM_findSymByAddr(funcAddr[idx], pFuncName);
        if(XOS_StrLen(pFuncName) > 0) 
        {
            XOS_Sprintf(buf, sizeof(buf) - 1, "%p: ", funcAddr[idx]);
            XOS_StrNCat(stackLog, buf, sizeof(stackLog) - 1);
            XOS_Sprintf(buf, sizeof(buf) - 1, "%s\n", pFuncName);
            XOS_StrNCat(stackLog, buf, sizeof(stackLog) - 1);
        }
    }
    if(XOS_StrLen(stackLog) > 0) 
    {
        write_to_syslog(stackLog);
    }
}


/************************************************************************
函数名: write_stack_to_syslog
功能：  将函数调用栈信息记录到syslog
输入：  fid           - 功能块id
输出：  N/A
返回：  XVOID 
说明： 
************************************************************************/
XVOID write_stack_to_syslog(XU32 fid)
{
    XCHAR pFuncName[MAX_STACK_FUN_NAME+1]  = {0};
    XPOINT funcAddr[MAX_MEM_STACK_DEPTH+1] ={0};
    XS32 idx = 0;
    XCHAR stackLog[2048] = {0};
    XCHAR buf[255] = {0};

    XOS_MutexLock(&g_fidMemMux);
    if(fid < FID_MAX) {
        if(g_fidMemCfg[fid] >= g_memExCfg._logStackNum) {
            XOS_MutexUnlock(&g_fidMemMux);
            return;
        }
        g_fidMemCfg[fid]++;
    }
    XOS_MutexUnlock(&g_fidMemMux);

    XOS_Sprintf(stackLog, sizeof(stackLog), "fid=%d, memstacks list show as below:\n", fid);
    
    /*获取函数地址*/
    MEM_CallTrace(funcAddr, MAX_MEM_STACK_DEPTH);
    //backtrace((XVOID**)funcAddr, MAX_MEM_STACK_DEPTH);
    //string = backtrace_symbols((void**)funcAddr, MAX_MEM_STACK_DEPTH);


    
    for(idx = 0; idx < MAX_MEM_STACK_DEPTH && funcAddr[idx] != 0; idx++) 
    {
        memset(pFuncName, 0, sizeof(pFuncName));
        /*根据函数地址获取函数名*/
        MEM_findSymByAddr(funcAddr[idx], pFuncName);
        if(XOS_StrLen(pFuncName) > 0) {
            XOS_Sprintf(buf, sizeof(buf) - 1, "%p: ", funcAddr[idx]);
            XOS_StrNCat(stackLog, buf, sizeof(stackLog) - 1);
            XOS_Sprintf(buf, sizeof(buf) - 1, "%s\n", pFuncName);
            XOS_StrNCat(stackLog, buf, sizeof(stackLog) - 1);
        }
    }
    if(XOS_StrLen(stackLog) > 0) {
        write_to_syslog(stackLog);
    }
}

#endif


/************************************************************************
函数名: XOS_WriteToDump
功能：  写记录到dump
输入：  
输出：  N/A
返回：  XVOID 
说明： 
************************************************************************/
XS32 XOS_WriteToDump(const XS8* msg,...)
{
    XCHAR msg_format[1024]  = {0}; 
    XS8*  ptr = NULL;
    XU32  msg_curlen  = 0;
    XS32  len = 0;
    FILE * fpw = NULL;
    va_list ap;

    if((len = TRC_formateloctime(msg_format)) <= 0)
    {
        return XERROR;
    }
 
    ptr = msg_format + len;
    *ptr++ = ' ';
    len++;
    
    va_start(ap, msg);
    msg_curlen = XOS_VsPrintf(ptr, sizeof(msg_format) - len - 1, msg, ap);
    va_end(ap);
    
    if (msg_curlen <= 0) 
    {
        return XERROR;
    }
    
    msg_curlen += len;
    msg_format[msg_curlen++] = '\n';
    
#ifdef XOS_VXWORKS
    fpw = fopen( "/ata0a/stackdump.txt", "a+" );
#elif defined(XOS_WIN32)
    mkdir("log");
    fpw = fopen( "log/stackdump.txt", "a+" );
#elif defined(XOS_LINUX)
    mkdir("log",S_IRWXU);
    fpw = fopen( "log/stackdump.txt", "a+" );
#endif
    if(fpw != NULL) {
        ptr = msg_format;
        while(msg_curlen > 0) {
            len = (XS32)fwrite(ptr, 1, msg_curlen, fpw);
            msg_curlen -= len;
            ptr += len;
        }
        fclose(fpw);
        return XSUCC;
    }
    return XERROR;
}

/************************************************************************
函数名: XOS_StackDump
功能：  将函数调用栈信息记录到dump文件
输入：  fid           - 功能块id
输出：  N/A
返回：  XVOID 
说明： 
************************************************************************/
//extern XVOID XOS_StackDump(void);
XVOID XOS_StackDump(void)
{
    XCHAR pFuncName[MAX_STACK_FUN_NAME+1]  = {0};
    XPOINT funcAddr[MAX_MEM_STACK_DEPTH+1] ={0};
    XS32 idx = 0;
    XCHAR stackLog[2048] = {0};
    XCHAR buf[255] = {0};

    XOS_Sprintf(stackLog, sizeof(stackLog), "stack list show as below:\n");
    
    /*获取函数地址*/
    MEM_CallTrace(funcAddr, MAX_MEM_STACK_DEPTH);
    
    for(idx = 2; idx < MAX_MEM_STACK_DEPTH && funcAddr[idx] != 0; idx++) 
    {
        memset(pFuncName, 0, sizeof(pFuncName));
        /*根据函数地址获取函数名*/
        MEM_findSymByAddr(funcAddr[idx], pFuncName);
        if(XOS_StrLen(pFuncName) > 0) {
            XOS_Sprintf(buf, sizeof(buf) - 1, "%p: ", funcAddr[idx]);
            XOS_StrNCat(stackLog, buf, sizeof(stackLog) - 1);
            XOS_Sprintf(buf, sizeof(buf) - 1, "%s\n", pFuncName);
            XOS_StrNCat(stackLog, buf, sizeof(stackLog) - 1);
        }
    }
    
    if(XOS_StrLen(stackLog) > 0) 
    {
        XOS_WriteToDump(stackLog);
    }
}


#ifdef __cplusplus
}
#endif /* __cplusplus */


