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
                  ����ͷ�ļ�
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
                 ģ���ڲ��궨��
-------------------------------------------------------------------------*/
#define  MAX_BLOCK_BITS           (32)           /*���֧�� 4G ���ڴ��*/
#define  MEM_MAGIC_VALUE          (0x00aaff77)  /*������֤�ڴ��ͷ*/
#define  MEM_MAGIC_TAIL           (0x00aaff88)  /*������֤�ڴ��β*/
#define  MEM_DBG_FILE_NAME_LEN    (16)          /*�����ļ�������*/
#define  MEM_CMD_SHOW_CTL(x)      ((x)%50 == 0)? (XOS_Sleep(5)) : XOS_UNUSED(x)

/*-------------------------------------------------------------------------
                 ģ���ڲ��ṹ��ö�ٶ���
-------------------------------------------------------------------------*/

/*BUCKECT ���ƿ�*/
typedef struct
{
    XS32 blockSize;
    XCHAR *headAddr;
    XCHAR *tailAddr;
    t_XOSMUTEXID bucketLock; /* ��ÿ��bucket ����*/
    XOS_HARRAY  blockArray;
}t_BUCKETCB;


/*�ڴ��ͷ�����ƽṹ*/

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
    XU32  fid;  /*���ܿ��*/
    t_XOSTT time;/*�ڴ�����ʱ��*/
    XCHAR fileName[MEM_DBG_FILE_NAME_LEN]; /*�����ڴ���ļ���*/
    XU32  lineNum;           /*�����ڴ��λ��*/

    /*��������ջ*/
    t_MEMSTACK memstack;/*�ڴ����ʱ�ĺ�������ջ*/
 #endif /*MEM_FID_DEBUG*/

    XS32 memLen;         /*������ڴ�ĳ���*/
    XS32 headCheck;    /*�ڴ��ͷ����֤��, ��ֹ�ڴ�ǰԽ��*/
}t_BLOCKHEAD;

/*�ڴ��β����֤�ṹ*/
typedef struct
{
    XS32 tailCheck; /*�ڴ��β����֤��,��ֹ�ڴ���Խ��*/
}t_BLOCKTAIL;

/*���ֲ��ұ�������*/
typedef struct
{
    XVOID *pLocation; /*bucket ��hash�ж�Ӧ��λ����Ϣ*/
}t_BUCKPTR;

/* �ڴ���ʾ��Ϣ���͵Ķ���*/
typedef struct
{
    XS32 index;
    CLI_ENV* pCliEnv;
    XU64 totalUse;
#ifdef MEM_FID_DEBUG
    XU32 fid;
#endif
}t_MEMSHOW;

/*�ڴ����Ľṹ*/
typedef struct
{
    XBOOL initialized;  /*�ڴ��ʼ���ı�־*/
    XOS_HHASH buckHash; /*���ٶ�λbucket ��*/
    XU32 buckTypes;                /* �����ļ������õ��ڴ����Ŀ��*/
    t_MEMBLOCK *pBlockPtr;     /*�����������ļ��е���Ϣ*/
    XU64 totalSize;                   /*�ܹ�������ڴ��С*/
    t_BUCKPTR *pElements;     /* hash ����Ԫ��λ����Ϣ,�����ڴ��ͷ�ʱ���ֲ���*/
    XS32 maxBuckBits;
}t_MEMMNT;

#ifdef MEM_FID_DEBUG
#include "xosqueue.h"

extern t_MODMNT  g_modMnt;
#define XOS_MAX_THREADID (1000) /*����߳��������߳�ID�ڲ���ϵͳ��Ψһ*/
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

t_ThreadIdCache g_ThreadIdCache; /*�߳�ID��ϣ�����*/
XBOOL bModifyFidFlg = XFALSE;    /*fid ������־*/

#endif



/*�����ڴ��ͷ�ʱ��Ҫ��ӡ��ջ��FID�ʹ�ӡ����*/
XSTATIC t_XOSMUTEXID g_fidMemMux;
XS32 g_fidMemCfg[FID_MAX];

/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
t_MEMMNT g_memMnt;
t_MEMEXCFG g_memExCfg;
RESET_PROC_FUNC pMemProcFunc = NULL;

/*-------------------------------------------------------------------------
                ģ���ڲ�����
-------------------------------------------------------------------------*/
XU32 MEM_CallTrace(XPOINT *pBuffer, XU32 nDepth);
XS16 MEM_Ptr_Show(CLI_ENV* pCliEnv, XPOINT pPtr);
/************************************************************************
������: XOS_MemProcFuncReg
���ܣ�  ע��ص�����������ϵͳ�����ڴ����ʱҵ�������ƺ���
���룺  ulFuncAddr ��ע��ĺ���
�����
���أ�
˵����
************************************************************************/
XS32 XOS_MemProcFuncReg(void * ulFuncAddr)
{
     pMemProcFunc = (RESET_PROC_FUNC)ulFuncAddr;
     return XSUCC;
}

/************************************************************************
 ������ : MEM_getBitsNum
 ����   : ��ȡλ��
 ����   :
 ���   : none
 ����   : λ��(����Ϊ�㷵��-1)
 ˵��   :
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
 * ����: �����ÿ��hashԪ��ͨ�õĺ���
 * ����: hHash   - ������
 *              elem    - Ԫ��
 *              param   - ����
 * ���:
 * ����: ָ��һ�����������´ε���XOS_HASHFuncʹ��
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
 * ����: �����ÿ��hashԪ��ͨ�õĺ���
 * ����: hHash   - ������
 *              elem    - Ԫ��
 *              param   - ����
 * ���:
 * ����: ָ��һ�����������´ε���XOS_HASHFuncʹ��
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
��������    : memHashShow
����        : Liu.Da
�������    : 2007��12��6��
��������    : �����ÿ��hashԪ��ͨ�õĺ���
����        : XOS_HHASH hHash
����        : XVOID* elem  - Ԫ��
����        : XVOID *param - ����
����ֵ        : XSTATIC XVOID*  ָ��һ�����������´ε���XOS_HASHFuncʹ��
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
 * ����: �����ÿ��hashԪ��ͨ�õĺ���
 * ����: hHash   - ������
 *              elem    - Ԫ��
 *              param   - ����
 * ���:
 * ����: ָ��һ�����������´ε���XOS_HASHFuncʹ��
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

/*ȷ���Ƿ�Ϊ2 �������η�*/
#define  IS_2N_NUM(n) (((n)==(XU32)(1<<MEM_getBitsNum((n)-1)))?XTRUE:XFALSE)

/************************************************************************
 * ���� : param                - Ԫ��
 *               paramSize          - Ԫ�صĴ�С(  ��λ�ֽ�)
 *               hashSize             - hash��Ĵ�С
 * ���� : hash ���
 ************************************************************************/
 XSTATIC  XU32 MEM_hashFunc(  XVOID*  param,  XS32    paramSize,  XS32    hashSize)
{
     XS32 hash = *((XS32*)param);

     return (hash % hashSize);
}

/************************************************************************
 * ������: MEM_tidyCfgBlocks
 * ���� : pMemBlock                - �����ļ��ж������ڴ��������
 *               pBlocks          - �������õ��ڴ��ĸ���
 *
   ���:  pBlocks       -�������ڴ������
 * ���� :
 ************************************************************************/
 XSTATIC  XVOID MEM_tidyCfgBlocks(  t_MEMBLOCK* pMemBlock,  XS32 * pBlocks)
{
     XS32 i, j;
     XS32 blockRead;
     t_MEMBLOCK *pMemTemp;
     t_MEMBLOCK *pMemTemp2;

     /*����һ�£�ȥ��block ������0 ��*/
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

     /* ��ȡ����ʵ�ʵ���Ŀ*/
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
                ģ��ӿں���
-------------------------------------------------------------------------*/

/************************************************************************
 ������ : MEM_Initlize
 ����   : �ڴ��ʼ��
 ����   :
 ���   : none
 ����   :
 ˵��   :
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

   /*����Ѿ���ʼ��*/
    if(g_memMnt.initialized)
    {
       XOS_CpsTrace(MD(FID_ROOT, PL_WARN), "MEM_init()-> reInit mem!");
       return XSUCC;
    }

   /*���ڴ������ļ�*/
       XOS_MemSet(&memCfg, 0, sizeof(t_MEMCFG));
       XOS_MutexCreate(&g_fidMemMux);
    XOS_MemSet(g_fidMemCfg, 0, sizeof(g_fidMemCfg));
    
    //�������#IF 0�滻�ô���
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
    /*��ȡ�ڴ��쳣����������Ϣ,�����ȡ��������Ĭ��ֵ����*/
    XML_readMemExCfg(&g_memExCfg, "xos.xml");
    
    /*�������������ļ����������ݿ����Ϊ������*/
    MEM_tidyCfgBlocks(memCfg.pMemBlock, (XS32 *)&(memCfg.memTypes));

    /*����������Ϣ*/
    g_memMnt.buckTypes = memCfg.memTypes;
    g_memMnt.pBlockPtr = memCfg.pMemBlock;

    /*������Դ*/

    /*����hash��*/
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
     /*����hash ����*/
    XOS_HashSetHashFunc(g_memMnt.buckHash, MEM_hashFunc);

    /*������ֲ��ҵ��ڴ�ռ�*/
    g_memMnt.pElements = (t_BUCKPTR*)XNULLP ;
    g_memMnt.pElements = (t_BUCKPTR*)XOS_Malloc(sizeof(t_BUCKPTR)*memCfg.memTypes);
    if(g_memMnt.pElements == XNULLP)
    {
         XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> malloc the elements failed !");
         goto memInitErorr;
    }

    /*�����ڴ�*/
    pMemBlock = (t_MEMBLOCK*)XNULLP;
    g_memMnt.maxBuckBits = 0;
    for(i=0; i<memCfg.memTypes; i++)
    {
        pMemBlock = memCfg.pMemBlock+i;
        XOS_MemSet(&bucketCb, 0, sizeof(t_BUCKETCB));
        bucketCb.blockSize = pMemBlock->blockSize;

        if(g_memMnt.maxBuckBits < bucketCb.blockSize)
         g_memMnt.maxBuckBits = bucketCb.blockSize;
        /*����������*/
        if( XSUCC != XOS_MutexCreate(&(bucketCb.bucketLock)))
        {
            XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> create mutex lock failed !");
            goto memInitErorr;
        }

        /*����bucket ����*/
        bucketCb.blockArray = XOS_ArrayMemCst(pMemBlock->blockSize+sizeof(t_BLOCKHEAD)+sizeof(t_BLOCKTAIL),
                                                                          pMemBlock->blockNums, "bucket");
        if(!XOS_ArrayHandleIsValid(bucketCb.blockArray))
        {
             XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> create array failed !");
             goto memInitErorr;
        }
        bucketCb.headAddr = (XCHAR*)XOS_ArrayGetHeadPtr(bucketCb.blockArray);
        bucketCb.tailAddr = (XCHAR*)XOS_ArrayGetTailPtr(bucketCb.blockArray);

        /*��������ڴ涼��ʼ��Ϊ0*/
        /*XOS_MemSet( bucketCb.headAddr, 0,
                               (pMemBlock->blockSize+sizeof(t_BLOCKHEAD)+sizeof(t_BLOCKTAIL) ) * pMemBlock->blockNums );
                               */
        /*��ӵ�hash����*/
        pLocation = XNULLP;
        pLocation = XOS_HashElemAdd(g_memMnt.buckHash, (XVOID*)&(pMemBlock->blockSize), (XVOID*)&bucketCb, XFALSE);
        if(pLocation == XNULLP)
        {
             XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "MEM_init()-> add bucket cb to hash faililed !");
             goto memInitErorr;
        }

        /* ������Ϣ�����ֲ���*/
        g_memMnt.pElements[i].pLocation = pLocation;

    }

    g_memMnt.maxBuckBits = MEM_getBitsNum(g_memMnt.maxBuckBits-1);

    /*�Զ��ֲ��ҵĲ��ֽ�������*/
    /*�����ڴ��ַ������˳������*/
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

    /*�ͷŶ������ļ��Ŀռ�*/
     if(memCfg.pMemBlock != XNULLP)
     {
         XOS_Free(memCfg.pMemBlock);
     }

     /*�ͷ����е�bucket �ڴ�*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_hashFree, XNULLP);

     /*�ͷ�hash�ڴ�*/
     XOS_HashMemDst(g_memMnt.buckHash);

     g_memMnt.initialized = XFALSE;
     return XERROR;

}

#ifdef MEM_FID_DEBUG
/************************************************************************
������: MEM_BKDRHash
���ܣ�  �ַ�����ϣ����
���룺  dwThreadId           - ��ǰ���е��߳�ID
����� 
���أ�  ��ϣ����
˵����
************************************************************************/
XU32  MEM_BKDRHash(XU64 dwThreadId)
{
    XU32 hashKey = 0;
    XU32 ulSeed = 131; //31��131��1313��13131
    XCHAR tmpBuf[8+2+1] = {0};
    XCHAR *pChar = NULL;

    /*������ת���ַ���*/
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
������: MEM_QueryFidByThreadId
���ܣ�  ͨ���߳�ID��ѯ��ʵ��FID
���룺  dwThreadId           - ��ǰ���е��߳�ID
�����  upNewFid             - ��ǰģ�������FID
���أ�  
˵����
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
������: MEM_AddFidByThreadId
���ܣ�  ͨ���߳�ID��FID���߳�ID�Ĺ�ϵ��������
���룺  dwThreadId     - ��ǰ���е��߳�ID
�����  uFid           - ��ǰ���е��߳�ID����Ӧ��ģ���FID
���أ�  
˵����  һ���߳�ֻ�����һ��Ԫ�أ�����Ҫ�����ٽ���
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
������: MEM_fidModify
���ܣ�  ���ݵ�ǰ�߳���ƽ̨ע���FID�����ڴ�����ǰ��FID����
���룺  pFid           - ���ܿ�id
�����  pFid           - �����������FID
���أ�  
˵����  ���һ��ģ��ֻ��һ��FID���������˶���̣߳���ֻ����ƽ̨��ע����̲߳��ܽ����������綨ʱ��ģ��ĸߡ��Ͷ�ʱ�߳�û��ע�ᵽƽ̨������������
        Ŀǰƽ̨��֧��һ��FIDע��һ���̡߳�
************************************************************************/
XS32 MEM_fidModify(XU32 *pFid)
{
#ifdef MEM_FID_DEBUG
    XU32 i = 0;
    t_TIDCB *pTidCb = NULL;
    t_FIDCB *pFidCb = NULL;

    t_XOSTASKID tmpThreadHandle = 0; /*�߳̾��*/
    XU64 tmpTaskId = 0;              /*�߳�ID*/

#ifdef XOS_WIN32    
    XU64 oldTaskId = 0;              /*�߳�ID*/
#endif
    if(NULL == pFid)
    {
        return XERROR;
    }
    
    if(100 == *pFid)/*ֻ��new��������*/
    {
      
#ifdef XOS_VXWORKS
        tmpTaskId = (XU64)taskIdSelf(); 
        /*��ȡ��ǰ�߳�ID,vx��ʵ���Ϸ��ص��ǿ��ƿ�ĵ�ַ, tmpTaskId��tmpThreadHandle��ʾͬ����ֵ*/
        tmpThreadHandle = (t_XOSTASKID)taskTcb(tmpTaskId);        

        /*����fid��hreadhandle�����ϣ��*/
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
                    if(pTidCb->tskId == tmpThreadHandle)/*�ȽϾ��*/
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
        tmpThreadHandle = (t_XOSTASKID)pthread_self();/*��ȡ��ǰ�߳̾��*/
        tmpTaskId = (XU64)gettid();/*��ȡ��ǰ�߳�ID*/
        
        /*����fid��hreadhandle�����ϣ��*/
        if(XSUCC == MEM_QueryFidByThreadId(tmpTaskId, pFid))
        {
            return XSUCC;
        }

        /*���ƻ����ÿ��ģ��ע�����֮�����ִ��һ�Σ��ں���ģ��ע�����֮ǰ�����ܻ�ִ�ж���ظ�ִ��*/
        for(i = FID_XOSMIN; i<FID_MAX; i++)
        {
            pFidCb = (t_FIDCB*)XNULLP;
            pFidCb = MOD_getFidCb(i);
            if(XNULL != pFidCb)
            {
                pTidCb = XOS_ArrayGetElemByPos(g_modMnt.tidArray, pFidCb->tidIndex);
                if(pTidCb != (t_TIDCB*)XNULLP)
                {
                    if(pTidCb->tskId == tmpThreadHandle)/*�ȽϾ��*/
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
        tmpTaskId = GetCurrentThreadId(); /*��ȡ��ǰ�߳�ID*/
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
                    /*�����߳̾����ȡ�߳�ID*/
                    oldTaskId = GetThreadId(pTidCb->tskId);
                    if(oldTaskId == tmpTaskId)/*�Ƚ��߳�ID,��Ϊwindows�²��ܵڶ��λ�ȡ��ͬ���*/
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
������: XOS_MemMalloc
���ܣ�  ����һ���ڴ��
���룺  fid           - ���ܿ�id
        nbytes        - Ҫ�����ڴ���ֽ���
�����  N/A
���أ�  XVOID *       -������ڴ�ָ��
˵����  ����ֵ���û�ǿ��ת��
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
    /* Ϊ֧��C++���ڴ�ʹ�÷��������ô˱���꣬�������main������һ�е���MEM���� */
    #ifdef XOS_NOTC_MEM
    if (!g_memMnt.initialized)
    {
        if (XSUCC != MEM_Initlize())
        {
            /* �ڴ��ʼ��ʧ��,ֱ���˳� */
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

    /*��ڰ�ȫ�Լ��*/
    if(!XOS_isValidFid(fid) || nbytes == 0 ||!g_memMnt.initialized)
    {
        return XNULLP;
    }

    /*����key*/
    bits = MEM_getBitsNum(nbytes-1);
    /*����״����,��һ��Ӧ�ÿ����ҵ�*/

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
        if(pBuckCb != XNULLP) /*�ҵ�*/
        {
            pBlockHead = (t_BLOCKHEAD*)XNULLP;
            XOS_MutexLock(&(pBuckCb->bucketLock));
            XOS_ArrayAddExt(pBuckCb->blockArray, (XOS_ArrayElement*)&pBlockHead);
            if(pBlockHead != XNULLP)
            {
                 /*��д�ڴ��ͷ���ֶ�*/
           #ifdef MEM_FID_DEBUG
                /*fid����,��ӦC++��new����ʱ��ʹ�õĲ���ģ��������FID����Ҫ��������*/
                if(XTRUE == bModifyFidFlg)
                {
                    MEM_fidModify(&fid);
                }

                pBlockHead->fid = fid;
                XOS_Time((t_XOSTT*)&(pBlockHead->time));
                Trace_abFileName(fileName, (XCHAR*)(pBlockHead->fileName), MEM_DBG_FILE_NAME_LEN-1);
                pBlockHead->lineNum = lineNo;

                /*����������ջ��Ϣ*/
                /*���Ƚ�ͷ����ǰ���ڵ���Ϣ�����*/
                XOS_MemSet(pBlockHead->memstack.AllocFunc_ST, 0, MAX_MEM_STACK_DEPTH * sizeof(XPOINT) );

                MEM_CallTrace(pBlockHead->memstack.AllocFunc_ST,MAX_MEM_STACK_DEPTH) ;

           #endif
                pBlockHead->memLen = RV_ALIGN(nbytes);
                pBlockHead->headCheck = MEM_MAGIC_VALUE;

                /*��дβ���ֶ�*/
                pBlockTail = (t_BLOCKTAIL*)(((XCHAR*)pBlockHead)+(sizeof(t_BLOCKHEAD)+pBlockHead->memLen));
                pBlockTail->tailCheck = MEM_MAGIC_TAIL;
                XOS_MutexUnlock(&(pBuckCb->bucketLock));

                return (XVOID*)(((XCHAR*)pBlockHead)+sizeof(t_BLOCKHEAD));
             }

             /*��ǰ�ڴ���Ѿ��þ������*/
             /*���´�ӡ����������޵ݹ飬���µ���ջ���*/
            //XOS_CpsTrace(MD(FID_ROOT, PL_WARN),
            //"XOS_MemMalloc()-> the blocSize[%d] bucket exhaust when fid %d call %d byetes!", pBuckCb->blockSize,fid,nbytes);
            XOS_MutexUnlock(&(pBuckCb->bucketLock));
        }
    }

MEMERR:
    /*�����е�bucket�ж�û���ҵ�*/
    /*to do ��չ��heap*/
    /*���´�ӡ����������޵ݹ飬���µ���ջ���*/
    //XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemMalloc()-> the all buckets exhaust when fid %d call %d byetes!",
    //            fid,nbytes);
    //XOS_CpsTrace(MD(FID_ROOT, PL_ERR),"XOS_MemMalloc()->the maxbytes is [%d], the bits is[%d]. ",
    //             g_memMnt.maxBuckBits, bits);

    /*�쳣���ڴ�����д����־*/
    XOS_Sprintf(szBuf, sizeof(szBuf)-1, "XOS_MemMalloc()-> the all buckets exhaust when [FID] %d call %d byetes!\r\n" \
                                        "FID[%d] XOS_MemMalloc()->the maxbytes is [%d], the bits is[%d]\r\n",
                                         fid, nbytes, fid, g_memMnt.maxBuckBits, bits);
    write_to_syslog(szBuf);

    /*����ջд����־*/
    write_stack_to_syslog(fid);    

    /*�ڴ��쳣ʱ����*/
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
������: XOS_MemCheck
���ܣ�  �ж��ڴ���Ƿ�����bucket���ڴ��,���ͷ��ڴ�
���룺  fid           - ���ܿ�id
                ptr           - �ڴ���׵�ַ
�����  N/A
����:   XSUCC  -    ��
        XERROR -    ��
˵����
************************************************************************/
XS32 XOS_MemCheck(XU32 fid, XVOID *ptr)
{
    XS32 i;
    XS32 j;
    t_BUCKPTR *pBuckPtr;
    t_BUCKETCB *pBuckCb;
    t_BLOCKHEAD *pBlockHead;
    t_BLOCKTAIL  *pBlockTail;

    /*��ȫ�Լ��*/
    if(ptr == XNULLP || !g_memMnt.initialized)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->input ptr is null  !");
        #ifdef XOS_DEBUG
        /*�����ã�������ڴ������ֱ�ӹ���*/
        XOS_SusPend();
        #endif
        return XERROR;
    }

    /*���ֲ��һ�ȡָ�����ڵ�array*/
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
             /*�����ã�������ڴ������ֱ�ӹ���*/
             XOS_SusPend();
             #endif
             return XERROR;
        }
        /*���ҳɹ�*/
        if((XPOINT)ptr > (XPOINT)(pBuckCb->headAddr)
           &&(XPOINT)ptr < (XPOINT)(pBuckCb->tailAddr))
        {
             /*����ȫ����֤*/
             /*ǰԽ����֤*/
             pBlockHead = (t_BLOCKHEAD*)((XCHAR*)ptr-(sizeof(t_BLOCKHEAD)));
             if(pBlockHead->headCheck != MEM_MAGIC_VALUE
                 || pBlockHead->memLen > pBuckCb->blockSize)
             {
                 XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->mem destroy before addr[%p] !", ptr);
                 #ifdef XOS_DEBUG
                 /*�����ã�������ڴ������ֱ�ӹ���*/
                 XOS_SusPend();
                 #endif
                 return XERROR;
             }
             /*��Խ����֤*/
             pBlockTail = (t_BLOCKTAIL*)((XCHAR*)ptr + pBlockHead->memLen);
             if(pBlockTail->tailCheck != MEM_MAGIC_TAIL)
             {
                  XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->mem destroy after addr[%p] !", ptr);
                  #ifdef XOS_DEBUG
                  /*�����ã�������ڴ������ֱ�ӹ���*/
                  XOS_SusPend();
                  #endif
                  return XERROR;
             }

             /*�жϵ��ڴ������bucket,���ǲ��ͷ��ڴ�,���سɹ�*/
             return XSUCC;
        }

        /*���ϰ벿*/
        if((XPOINT)ptr >(XPOINT)(pBuckCb->tailAddr))
        {
            i = (i+j)/2+1;
            continue;
        }

        /*���°벿��*/
        if((XPOINT)ptr < (XPOINT)pBuckCb->headAddr)
        {
            j = (i+j)/2 -1;
            continue;
        }

    }

    /*һֱ��û���ҵ�, Ӧ���ǵ�ַ��Ч*/
    XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemCheck()->input addr[%p]  is not in bucket !", ptr);
    #ifdef XOS_DEBUG
    /*�����ã�������ڴ������ֱ�ӹ���*/
    XOS_SusPend();
    #endif

    return XERROR;
}

/************************************************************************
������: XOS_MemFree
���ܣ�  �ͷ�һ���ڴ��
���룺  fid           - ���ܿ�id
        ptr           - Ҫ�ͷŵ��ڴ��׵�ַ
�����  N/A
����:   XSUCC  -    �ɹ�
        XERROR -    ʧ��
˵����
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

    /*��ȫ�Լ��*/
    if(ptr == XNULLP || !g_memMnt.initialized)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_WARN), "XOS_MemFree()->input ptr is null  !");
        #ifdef XOS_DEBUG
        /*�����ã�������ڴ������ֱ�ӹ���*/
        XOS_SusPend();
        #endif
        goto MEMERR;
    }

    /*���ֲ��һ�ȡָ�����ڵ�array*/
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
             /*�����ã�������ڴ������ֱ�ӹ���*/
             XOS_SusPend();
             #endif
             goto MEMERR;
        }
        /*���ҳɹ�*/
        if((XPOINT)ptr > (XPOINT)(pBuckCb->headAddr)
           &&(XPOINT)ptr < (XPOINT)(pBuckCb->tailAddr))
        {
             /*����ȫ����֤*/
             /*ǰԽ����֤*/
             pBlockHead = (t_BLOCKHEAD*)((XCHAR*)ptr-(sizeof(t_BLOCKHEAD)));
             if(pBlockHead->headCheck != MEM_MAGIC_VALUE
                 || pBlockHead->memLen > pBuckCb->blockSize)
             {
                 XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemFree()->mem destroy before addr[%p] !", ptr);
                 #ifdef XOS_DEBUG
                 /*�����ã�������ڴ������ֱ�ӹ���*/
                 XOS_SusPend();
                 #endif
                 goto MEMERR;
             }
             /*��Խ����֤*/
             pBlockTail = (t_BLOCKTAIL*)((XCHAR*)ptr + pBlockHead->memLen);
             if(pBlockTail->tailCheck != MEM_MAGIC_TAIL)
             {
                  XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemFree()->mem destroy after addr[%p] !", ptr);
                  #ifdef XOS_DEBUG
                  /*�����ã�������ڴ������ֱ�ӹ���*/
                  XOS_SusPend();
                  #endif
                  goto MEMERR;
             }

        #ifdef MEM_FID_DEBUG
        XOS_MemSet(pBlockHead->memstack.FreeFunc_ST, 0, MAX_MEM_STACK_DEPTH * sizeof(XPOINT) );

        MEM_CallTrace(pBlockHead->memstack.FreeFunc_ST, MAX_MEM_STACK_DEPTH);
        #endif /*MEM_FID_DEBUG*/

             /*�ڴ��ͷ�*/
             XOS_MutexLock(&(pBuckCb->bucketLock));
             XOS_ArrayDeleteByPos(pBuckCb->blockArray, XOS_ArrayGetByPtr(pBuckCb->blockArray, (XCHAR*)ptr-(sizeof(t_BLOCKHEAD))));
             XOS_MutexUnlock(&(pBuckCb->bucketLock));

             return XSUCC;
        }

        /*���ϰ벿*/
        if((XPOINT)ptr >(XPOINT)(pBuckCb->tailAddr))
        {
            i = (i+j)/2+1;
            continue;
        }

        /*���°벿��*/
        if((XPOINT)ptr < (XPOINT)pBuckCb->headAddr)
        {
            j = (i+j)/2 -1;
            continue;
        }

    }

MEMERR:
    /*һֱ��û���ҵ�, Ӧ���ǵ�ַ��Ч*/
    XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_MemFree()->error  fid is [%d] input addr[%p] bucktype is [%d]!",
                    fid, ptr, j);

     
    #ifdef XOS_DEBUG
    /*�����ã�������ڴ������ֱ�ӹ���*/
    XOS_SusPend();
    #endif    

    //�ڴ��ͷŴ������¼��ջ
    write_stack_to_syslog(fid);
    
    return XERROR;
}

/*����SSI���ڴ����ӿ�*/
XU32 XOS_MEMCtrl(XU32 nbytes )
{
    XS32 key;
    t_BUCKETCB *pBuckCb= (t_BUCKETCB*)XNULLP;
    XU16 bits;
    XU32 avlnum,totnum,curUseNum;

    /*��ڰ�ȫ�Լ��*/
    if( nbytes == 0 )
    {
        return 0;
    }

    /*����key*/
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
 ������:MEM_infoShow
 ����: ��ʾmem ��������Ϣ
 ����:
 ���:
 ����:
 ˵��: ntlthreedinfo���������ִ�к���
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

     /*��ӡ����*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_hashShow, (XVOID*)&showInfo);

     /*end of udp list */
     XOS_CliExtPrintf(pCliEnv,
     "--------------------------------------------------\r\n total malloc: %llu  usage: %llu  leave: %llu \r\n",
     g_memMnt.totalSize, showInfo.totalUse, g_memMnt.totalSize-showInfo.totalUse);

}

/**********************************
��������    : memshow
����        : Jeff.Zeng
�������    : 2007��12��6��
��������    : ��ʾmem ��������Ϣ
����        : void
����ֵ        : XVOID
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

     /*��ӡ����*/
     XOS_HashWalk(g_memMnt.buckHash, memHashShow, (XVOID*)&showInfo);

     /*end of udp list */
     printf(
     "--------------------------------------------------\r\n total malloc: %llu  usage: %llu  leave: %llu \r\n",
     g_memMnt.totalSize, showInfo.totalUse, g_memMnt.totalSize-showInfo.totalUse);

}

/************************************************************************
������:XOS_GetMemContent
����:  ��ȡָ���ڴ��ָ���������ݵ��û�������
����:title - �û�����ı���
    mem ���û�ָ�����ڴ��ַ
    len  ���û���Ҫ��ȡ���ڴ����ݵĳ���
    buf ���û�������ָ��
    bufSize - �û���������С
���:
����: ��ȡ�����ڴ����ݵĳ���
˵��: ������ݵ�16����ASCIIֵ��������
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
������:XOS_MemPrint
����:  ��ӡָ���ڴ��ָ���������ݵ�trace
����: fid - ģ��fid
    title - �û�����ı���
    level - ��ӡ����
    mem ���û�ָ�����ڴ��ַ
    len  ���û���Ҫ��ȡ���ڴ����ݵĳ���
���:
����: ��ȡ�����ڴ����ݵĳ���
˵��: ������ݵ�16����ASCIIֵ��trace
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
 ������:MEM_fidUsage
 ����: ��ʾmem ��fid ʹ����Ϣ
 ����:
 ���:
 ����:
 ˵��: memfidusage���������ִ�к���
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
	 /*new����ʱʹ�õ�fidΪ100*/
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

     /*��ӡ����*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_fidShow, (XVOID*)&showInfo);

     /*end of udp list */
     XOS_CliExtPrintf(pCliEnv,
     "--------------------------------------------------\r\n");

}

/************************************************************************
 ������:MEM_fidBlockSwill
 ����: ��ʾmem ��ָ��fid ��ָ��block��ʹ����Ϣ
 ����:
 ���:
 ����:
 ˵��: 
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
 ������:MEM_swill
 ����: ��ʾmem ���ڴ��ʹ����Ϣ�����������ڴ�й©
 ����:
 ���:
 ����:
 ˵��: memswill���������ִ�к���
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
		/*����ʱ���ܱ��ͷ���*/
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
 ������:MEM_blocksSwill
 ����: ��ʾmem ���ڴ��ʹ����Ϣ�����������ڴ�й©
 ����:
 ���:
 ����:
 ˵��: memswill���������ִ�к���
************************************************************************/
XVOID MEM_blocksSwill(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
      MEM_swill( pCliEnv,  siArgc,  ppArgv, 0);
}

/************************************************************************
 ������:MEM_allSwill
 ����: ��ʾ����mem ���ڴ��ʹ����Ϣ�����������ڴ�й©
 ����:
 ���:
 ����:
 ˵��: memswill���������ִ�к���
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
 ������:MEM_allSwill
 ����: ��ʾ����mem ���ڴ��ʹ����Ϣ�����������ڴ�й©
 ����:
 ���:
 ����:
 ˵��: memswill���������ִ�к���
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
 ������:MEM_StackShow
 ����: ��ʾ�ÿ��ڴ��ڷ�����ͷ�ʱ�ĺ�������ջ��Ϣ
 ����:
 ���:
 ����:
 ˵��: memstackshow ���������ִ�к���
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

    /*��ȫ�Լ��*/
    if(pPtr == XNULL || !g_memMnt.initialized)
    {
         XOS_CliExtPrintf(pCliEnv,"mem address is wrong\r\n");

          return XERROR;
    }
    /*����ڴ��׵�ַ��׼ȷ��*/
    /*���ֲ��һ�ȡָ�����ڵ�array*/
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
             /*����ȫ����֤*/
             /*ǰԽ����֤*/
             pBlockHead = (t_BLOCKHEAD*)(pPtr-(sizeof(t_BLOCKHEAD)));//(XCHAR*)
             //pBlockHead = (t_BLOCKHEAD*)((XCHAR*)pPtr-(sizeof(t_BLOCKHEAD)));
                 //pBlockHead = (t_BLOCKHEAD*)(XCHAR*)pPtr;
             if(pBlockHead->headCheck != MEM_MAGIC_VALUE
                 || pBlockHead->memLen > pBuckCb->blockSize)
             {
                 XOS_CliExtPrintf(pCliEnv,"the mem address [%p] is wrong!\r\n", pPtr);

                 return XERROR;
             }
             /*��Խ����֤*/
           //pBlockTail = (t_BLOCKTAIL*)((XCHAR*)pPtr + pBlockHead->memLen);
                    pBlockTail = (t_BLOCKTAIL*)((XCHAR*)pBlockHead +sizeof(t_BLOCKHEAD)+ pBlockHead->memLen);
             if(pBlockTail->tailCheck != MEM_MAGIC_TAIL)
             {
                  XOS_CliExtPrintf(pCliEnv,"the mem address [%p] is wrong!\r\n", pPtr);

                  return XERROR;
             }

            /*�׵�ַ��ȷ�������ջ*/
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

                /*���ϰ벿������*/
        if((XPOINT)pPtr >(XPOINT)(pBuckCb->tailAddr))
        {
            i = (i+j)/2+1;
            continue;
        }

        /*���°벿�ּ�����*/
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
������: xosMemCheck()
���ܣ�  memory check function for debug
˵����  ����ֵ���û�ǿ��ת��
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

    /*����״����,��һ��Ӧ�ÿ����ҵ�*/

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
        if(pBuckCb != XNULLP) /*�ҵ�*/
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
             /*  ��λ���Ƿ���Ԫ��ռ��,just continue */
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
 ������:XOS_CaptureStackBackTrace
 ����: win_64�»�ȡ�������ö�ջ
 ����:
 ���:
 ����:
 ˵��: 
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
 ������:MEM_CallTrace
 ����: ��ʾ�ڴ����ջ
 ����:
 ���:
 ����:
 ˵��: vxworks��ֻ������cpu = ppc860, OS = T22/T2;
      windows���ʺ�x86��x84_64;
      linux ֧��x86_32��x86_64
************************************************************************/

XU32 MEM_CallTrace(XPOINT *pBuffer, XU32 nDepth)
{
/*ʹ�õ��ǻ�����ԣ�����Ҫ����cpu���ͣ�ϵͳ�ͺŵ�
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
//���º���ΪVXWORKS����23�˿��ڴ���ض�λ���ֶ�
/************************************************************************
 ������:meminfo
 ����: ��ʾmem ��������Ϣ
 ����:
 ���:
 ����:
 ˵��: 
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

     /*��ӡ����*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_hashShow, (XVOID*)&showInfo);

     /*end of udp list */
     printf("--------------------------------------------------\r\n total malloc: %llu  usage: %llu  leave: %llu \r\n",
     g_memMnt.totalSize, showInfo.totalUse, g_memMnt.totalSize-showInfo.totalUse);

     return;
}

#ifdef MEM_FID_DEBUG
/************************************************************************
 ������:memfid
 ����: ��ʾmem ��fid ʹ����Ϣ
 ����:
 ���:
 ����:
 ˵��: memfidusage���������ִ�к���
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

     /*��ӡ����*/
     XOS_HashWalk(g_memMnt.buckHash, MEM_fidShow, (XVOID*)&showInfo);

     /*end of udp list */
     printf("--------------------------------------------------\r\n");
     return;
}

/************************************************************************
 ������:memblock
 ����: ��ʾmem ���ڴ��ʹ����Ϣ�����������ڴ�й©
 ����:
 ���:
 ����:
 ˵��: 
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
 ������:memallblock
 ����: ��ʾ����mem ���ڴ��ʹ����Ϣ�����������ڴ�й©
 ����:
 ���:
 ����:
 ˵��: 
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
 ������:memstack
 ����: ��ʾ�ÿ��ڴ��ڷ�����ͷ�ʱ�ĺ�������ջ��Ϣ
 ����:
 ���:
 ����:
 ˵��: 
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
    printf("\n meminfo     para:no          -��ʾ�ڴ���Ϣ");
 #ifdef MEM_FID_DEBUG
    printf("\n memfid      para:fid         -��ʾָ��FIDʹ���ڴ���Ϣ");
    printf("\n memblock    para:blocksize   -��ʾָ��BLOCKSIZE�ڴ��ʹ����Ϣ");   
    printf("\n memallblock para:no          -��ʾ�����ڴ��ʹ����Ϣ");
    printf("\n memstack    para:addr        -��ʾָ����ַ�ĵ���ջ��Ϣ");
 #endif
    return ;
}

/************************************************************************
������: write_to_syslog
���ܣ�  д��¼��syslog
���룺  fid           - ���ܿ�id
�����  N/A
���أ�  XVOID 
˵���� 
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
������: XosStackDumpToSyslog
���ܣ�  ����������ջ��Ϣ��¼��syslog
���룺  pszBuf - ��ʾ��Ϣ
�����  N/A
���أ�  XVOID 
˵���� 
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
    
    /*��ȡ������ַ*/
    MEM_CallTrace(funcAddr, MAX_MEM_STACK_DEPTH);
    
    for(idx = 0; idx < MAX_MEM_STACK_DEPTH && funcAddr[idx] != 0; idx++) 
    {
        memset(pFuncName, 0, sizeof(pFuncName));
        /*���ݺ�����ַ��ȡ������*/
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
������: write_stack_to_syslog
���ܣ�  ����������ջ��Ϣ��¼��syslog
���룺  fid           - ���ܿ�id
�����  N/A
���أ�  XVOID 
˵���� 
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
    
    /*��ȡ������ַ*/
    MEM_CallTrace(funcAddr, MAX_MEM_STACK_DEPTH);
    //backtrace((XVOID**)funcAddr, MAX_MEM_STACK_DEPTH);
    //string = backtrace_symbols((void**)funcAddr, MAX_MEM_STACK_DEPTH);


    
    for(idx = 0; idx < MAX_MEM_STACK_DEPTH && funcAddr[idx] != 0; idx++) 
    {
        memset(pFuncName, 0, sizeof(pFuncName));
        /*���ݺ�����ַ��ȡ������*/
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
������: XOS_WriteToDump
���ܣ�  д��¼��dump
���룺  
�����  N/A
���أ�  XVOID 
˵���� 
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
������: XOS_StackDump
���ܣ�  ����������ջ��Ϣ��¼��dump�ļ�
���룺  fid           - ���ܿ�id
�����  N/A
���أ�  XVOID 
˵���� 
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
    
    /*��ȡ������ַ*/
    MEM_CallTrace(funcAddr, MAX_MEM_STACK_DEPTH);
    
    for(idx = 2; idx < MAX_MEM_STACK_DEPTH && funcAddr[idx] != 0; idx++) 
    {
        memset(pFuncName, 0, sizeof(pFuncName));
        /*���ݺ�����ַ��ȡ������*/
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


