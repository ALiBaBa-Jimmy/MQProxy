/*-----------------------------------------------------------
    sysHash.c -  Hash��c�ļ�

    ��Ȩ���� 2004 -2006 ������˾������BSC��Ŀ��.

    �޸���ʷ��¼
    --------------------
    20.00.01,       08-4-2004,      ���༽      ����.
-----------------------------------------------------------*/
#ifdef  __cplusplus
extern  "C"
{
#endif

/*
*�ļ�����
*/

#include "saap_def.h"
#include "syshash.h"

/*
*�궨��
*/

#define HASH_NODE HASH_NODE_XW

/*
*�ⲿ��������
*/

/*
*���غ�������
*/

/*---------------------------------------------------------------------
SYS_HashTblCreat    - ��ϣ����
����˵����
	ulBinNum        - ���룬��ϣ��Ĺ�ϣͰ��С
	ulNodeNum       - ���룬��ϣ��Ĺ�ϣ�ڵ���Ŀ
	fnFunc          - ���룬��ȡ��ϣͰ�����ĺ���ָ��
	pstHashTbl      - ��������صĹ�ϣ���ַ

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashTblCreat(XU32 ulBinNum, XU32 ulNodeNum, HASH_CNTN_GET_FUNC fnFunc, HASH_TABLE *pstHashTbl)
{
    XU32  i;

    //�Ϸ��Լ��
    if ((0 == ulBinNum) || (0 == ulNodeNum) || (XNULL == fnFunc) || (XNULL == pstHashTbl))
    {
        SYS_RETURN(XERROR);
    }

    //������ϣ�ڵ��
    pstHashTbl->pstNode = (HASH_NODE_XW*)XOS_MemMalloc(FID_SAAP, sizeof(HASH_NODE_XW) * ulNodeNum);
    if (XNULL == pstHashTbl->pstNode)
    {
        SYS_RETURN(XERROR);
    }
    //������ϣ����(��ϣͰ)��
    pstHashTbl->pstBin = (SYS_LISTENT_t*)XOS_MemMalloc(FID_SAAP, sizeof(SYS_LISTENT_t) * ulBinNum);
    if (XNULL == pstHashTbl->pstBin)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstNode);
        SYS_RETURN(XERROR);
    }

    //��¼��ϣͰ��С�͹�ϣ��Ľڵ��С�ͻ�ȡ��ϣͰ�ĺ���ָ��
    pstHashTbl->ulBinSize   = ulBinNum;
    pstHashTbl->ulNodeSize  = ulNodeNum;
    pstHashTbl->fnFunc      = fnFunc;

    //��ʼ����ϣͰ
    for (i = 0; i < ulBinNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstBin[i]);
    }
    //��ʼ����ϣ��Ŀ��нڵ�����
    SYS_ListIni(&pstHashTbl->stIdle);
    for (i = 0; i < ulNodeNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstNode[i].stLe);
        pstHashTbl->pstNode[i].ulIndex = i;
        SYS_ListAdd(pstHashTbl->stIdle.prev, &pstHashTbl->pstNode[i].stLe);
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_HashTblDel      - ��ϣ��ע��

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashTblDel(HASH_TABLE *pstHashTbl)
{
    //�Ϸ��Լ��
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }

    if(XNULL != pstHashTbl->pstNode)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstNode);
        pstHashTbl->pstNode = XNULL;
    }

    if(XNULL != pstHashTbl->pstBin)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstBin);
        pstHashTbl->pstBin = XNULL;
    }

    SYS_ListIni(&pstHashTbl->stIdle);
    pstHashTbl->ulNodeSize = 0;
    pstHashTbl->ulBinSize = 0;

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_HashTblClear      - ��ϣ�����

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashTblClear(HASH_TABLE *pstHashTbl)
{
    XU32  ulBinNum, ulNodeNum, i;

    //�Ϸ��Լ��
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }

    ulBinNum = pstHashTbl->ulBinSize;
    ulNodeNum = pstHashTbl->ulNodeSize;

    //��ʼ����ϣͰ
    for(i = 0; i < ulBinNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstBin[i]);
    }

    //��ʼ����ϣ��Ŀ��нڵ�����
    SYS_ListIni(&pstHashTbl->stIdle);
    for (i = 0; i < ulNodeNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstNode[i].stLe);
        SYS_ListAdd(pstHashTbl->stIdle.prev, &pstHashTbl->pstNode[i].stLe);
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_HashNodeInsert  - ����ϣ�����һ���ڵ�

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��
    ulHashKey       - ���룬�ؼ���
    ulHashValue     - ���룬����ʱ���ص�ֵ
    pulIndex        - ������½ڵ��ھ�̬��ϣ���е�λ��(����)

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashNodeInsert(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulHashValue, XU32 *pulIndex)
{
    XU32      ulBinIndex, ulHashValue1, ulIndex1;
    HASH_NODE_XW   *pstNode;

    //�Ϸ��Լ��
    if(XNULL == pstHashTbl || XNULL == pulIndex)
    {
        SYS_RETURN(XERROR);
    }

    if (pstHashTbl->stIdle.next == &pstHashTbl->stIdle)//û�п�����Դ
    {
        SYS_RETURN(XERROR);
    }

    //�ҳ���ϣͰ��������
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //�жϹ�ϣͰ�����ŵĺϷ���
    if (ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    ulHashValue1 = ulHashValue;
    ulIndex1 = BLANK_ULONG;
    if (XSUCC == SYS_HashNodeSearch(pstHashTbl, ulHashKey, &ulHashValue1, &ulIndex1))//�����������ͬ��
    {
        SYS_RETURN(XERROR);
    }//9999

    //���뵽Ͱ��
    pstNode = (HASH_NODE_XW*)pstHashTbl->stIdle.next;
    SYS_ListDel(&pstNode->stLe);
    SYS_ListAdd(pstHashTbl->pstBin[ulBinIndex].prev, &pstNode->stLe);
    pstNode->ulHashKey = ulHashKey;
    pstNode->ulHashValue = ulHashValue;

    //��д�½ڵ��ھ�̬��ϣ���е�λ��(����)
    *pulIndex = pstNode->ulIndex; //9999

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_HashNodeDel  - �ӹ�ϣ����ɾ��һ���ڵ�

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��
    ulHashKey       - ���룬�ؼ���
    pulIndex        - �������ɾ���Ĺ�ϣ�ڵ��������

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashNodeDel(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulIndex)
{
    XU32      ulBinIndex;
    HASH_NODE_XW   *pstNode;

    //�Ϸ��Լ��
    if((XNULL == pstHashTbl) || (XNULL == pulIndex))
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //�ҳ���ϣͰ��������
    if(XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //�жϹ�ϣͰ�����ŵĺϷ���
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    *pulIndex = BLANK_ULONG;

    pstNode = (HASH_NODE_XW*)pstHashTbl->pstBin[ulBinIndex].next;
    while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
    {
        if (pstNode->ulHashKey == ulHashKey)//�ҵ�
        {
            SYS_ListDel(&pstNode->stLe);
            SYS_ListAdd(pstHashTbl->stIdle.prev, &pstNode->stLe);
            *pulIndex = pstNode->ulIndex;
            SYS_RETURN(XSUCC);
        }
        pstNode = (HASH_NODE*)pstNode->stLe.next;
    }
    SYS_RETURN(XERROR);
}

/*---------------------------------------------------------------------
SYS_HashNodeSearch  - �ӹ�ϣ���в���һ���ڵ�

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��
    ulHashKey       - ���룬��ϣ�ؼ���
    pulHashValue    - �������ϣֵ
    pulIndex        - ������ڵ��ھ�̬��ϣ���е�λ��(����)

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashNodeSearch(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulHashValue, XU32 *pulIndex)
{
    XU32      ulBinIndex;
    HASH_NODE   *pstNode;

    //�Ϸ��Լ��
    if (XNULL == pstHashTbl || XNULL == pulHashValue || XNULL == pulIndex)
    {
        SYS_RETURN(XERROR);
    }
    if (XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //�ҳ���ϣͰ��������
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //�жϹ�ϣͰ�����ŵĺϷ���
    if (ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    pstNode = (HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
    while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
    {
        if (pstNode->ulHashKey == ulHashKey)//�ҵ�
        {
            *pulHashValue = pstNode->ulHashValue;
            *pulIndex = pstNode->ulIndex;
            SYS_RETURN(XSUCC);
        }
        pstNode = (HASH_NODE*)pstNode->stLe.next;
    }

    //ע: ���ڲ��ҹ�ϣ�ڵ㲻���������ģ���˲�ʹ��[SYS_RETURN]
    return(XERROR);
}

/*---------------------------------------------------------------------
SYS_HashTblWalk  - ������ϣ������нڵ�

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��
    fnVisit         - ���룬�����Թؼ���Ϊ�����Ĵ�����ָ��(������ʽΪ: (XS32 ulHashValue, XS32 ulIndex))
    ulHashKey       - fnVisit�Ĳ������ͣ��ؼ���
    ulHashValue     - fnVisit�Ĳ������ͣ���ϣֵ
    ulIndex         - fnVisit�Ĳ������ͣ���ϣ�ڵ��ڹ�ϣ���е�������

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_HashTblWalk(HASH_TABLE *pstHashTbl, XVOID (*fnVisit)(XU32 ulHashKey, XU32 ulHashValue, XU32 ulIndex))
{
    XU32      ulBinIndex;
    HASH_NODE   *pstNode;

    //�Ϸ��Լ��
    if (XNULL == pstHashTbl || XNULL == fnVisit)
    {
        SYS_RETURN(XERROR);
    }

    for (ulBinIndex = 0; ulBinIndex < pstHashTbl->ulBinSize; ulBinIndex++)
    {
        pstNode = (HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
        while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
        {
            fnVisit(pstNode->ulHashKey, pstNode->ulHashValue, pstNode->ulIndex);
            pstNode = (HASH_NODE*)pstNode->stLe.next;
        }
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashTblCreat    - ��չ��ϣ����

����˵����
    ulBinNum     - ���룬��չ��ϣ���С
    ulNodeNum     - ���룬��չ��ϣ��ڵ���Ŀ
    ulMaxSameKeyNode        - ���룬��չ��ϣ��������ͬ�ؼ��ֵ����ڵ���
    fnFunc - ���룬��ȡ��չ��ϣ��Ĺ�ϣͰ�ĺ���ָ��
    pstHashTbl              - ��������ص���չ��ϣ���ַ

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashTblCreat(XU32 ulBinNum, XU32 ulNodeNum, XU32 ulMaxSameKeyNode, HASH_CNTN_GET_FUNC fnFunc, EXTEND_HASH_TABLE *pstHashTbl)
{
    XU32  i;

    //�Ϸ��Լ��
    if ((0 == ulBinNum) || (0 == ulNodeNum) || (0 == ulMaxSameKeyNode) || (XNULL == fnFunc) || (XNULL == pstHashTbl))
    {
        SYS_RETURN(XERROR);
    }

    //������ϣ�ڵ��
    pstHashTbl->pstNode = (EXTEND_HASH_NODE *)XOS_MemMalloc(FID_SAAP, sizeof(EXTEND_HASH_NODE) * ulNodeNum);
    if (XNULL == pstHashTbl->pstNode)
    {
        SYS_RETURN(XERROR);
    }

    //������ϣ����(��ϣͰ)��
    pstHashTbl->pstBin = (SYS_LISTENT_t *)XOS_MemMalloc(FID_SAAP, sizeof(SYS_LISTENT_t) * ulBinNum);
    if(XNULL == pstHashTbl->pstBin)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstNode);
        SYS_RETURN(XERROR);
    }

    //��¼��ϣ���С�͹�ϣͰ��С,��ȡ��ϣͰ�ĺ���ָ��,������ͬ�ؼ��ֵ����ڵ���
    pstHashTbl->ulBinSize             = ulBinNum;
    pstHashTbl->ulNodeSize            = ulNodeNum;
    pstHashTbl->fnFunc                = fnFunc;
    pstHashTbl->ulMaxSameKeyNode      = ulMaxSameKeyNode;

    //��ʼ����ϣͰ
    for(i = 0; i < ulBinNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstBin[i]);
    }

    //��ʼ����ϣ��Ŀ��нڵ�����
    SYS_ListIni(&pstHashTbl->stIdle);
    for (i = 0; i < ulNodeNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstNode[i].stLe);
        pstHashTbl->pstNode[i].ulIndex = i;
        SYS_ListAdd(pstHashTbl->stIdle.prev, &pstHashTbl->pstNode[i].stLe);
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashTblDel      - ��չ��ϣ��ע��

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashTblDel(EXTEND_HASH_TABLE *pstHashTbl)
{
    //�Ϸ��Լ��
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }

    //�ͷŹ�ϣ�ڵ�����
    if(XNULL != pstHashTbl->pstNode)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstNode);
        pstHashTbl->pstNode = XNULL;
    }

    //�ͷŹ�ϣͰ����
    if(XNULL != pstHashTbl->pstBin)
    {
        XOS_MemFree(FID_SAAP,pstHashTbl->pstBin);
        pstHashTbl->pstBin = XNULL;
    }

    SYS_ListIni(&pstHashTbl->stIdle);
    pstHashTbl->ulNodeSize = 0;
    pstHashTbl->ulBinSize = 0;

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashTblClear      - ��չ��ϣ�����

����˵����
    pstHashTbl      - ���룬��չ��ϣ��ָ��

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashTblClear(EXTEND_HASH_TABLE *pstHashTbl)
{
    XU32 ulBinNum, ulNodeNum, i;

    //�Ϸ��Լ��
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }

    ulBinNum = pstHashTbl->ulBinSize;
    ulNodeNum = pstHashTbl->ulNodeSize;

    //��ʼ����ϣͰ
    for(i = 0; i < ulBinNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstBin[i]);
    }

    //��ʼ����ϣ��Ŀ��нڵ�����
    SYS_ListIni(&pstHashTbl->stIdle);
    for (i = 0; i < ulNodeNum; i++)
    {
        SYS_ListIni(&pstHashTbl->pstNode[i].stLe);
        SYS_ListAdd(pstHashTbl->stIdle.prev, &pstHashTbl->pstNode[i].stLe);
    }

    SYS_RETURN(XSUCC);
}
/*---------------------------------------------------------------------
SYS_ExtendHashNodeSearch  - ����չ��ϣ���в���һ������ڵ�

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��
    ulHashKey       - ���룬�ؼ���
    ulValueNum          - ���룬����Ľڵ���
    pulHashValues       - ���룬����ʱ���ص�ֵ����

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashNodeSearch1(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulValueNum, XU32 pulHashValues[])
{
    XU32              ulBinIndex, i;
    EXTEND_HASH_NODE    *pstNode;

    //�Ϸ��Լ��
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //�ҳ���ϣͰ��������
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //�жϹ�ϣͰ�����źϷ���
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    for (i = 0; i < ulValueNum; i++)
    {
        pstNode = (EXTEND_HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
        while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
        {
            if ((pstNode->ulHashKey == ulHashKey) && (pstNode->ulHashValue == pulHashValues[i]))
            {
                SYS_RETURN (XSUCC);
            }
            pstNode = (EXTEND_HASH_NODE*)pstNode->stLe.next;
        }
    }
    SYS_RETURN(XERROR);
}

/*---------------------------------------------------------------------
SYS_ExtendHashNodeInsert  - ����չ��ϣ�����һ�������ڵ�

����˵����
    pstHashTbl    - ���룬��չ��ϣ��ָ��
    ulHashKey           - ���룬�ؼ���
    ulValueNum          - ���룬����Ľڵ���
    pulHashValues       - ���룬����ʱ���ص�ֵ����
    pulIndexs           - ������½ڵ��ھ�̬��չ��ϣ���е�λ��(����)����

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashNodeInsert(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulValueNum, XU32 pulHashValues[], XU32 pulIndexs[])
{
    XU32      ulBinIndex, i;
    EXTEND_HASH_NODE   *pstNode;

    //�Ϸ��Լ��
    if(XNULL == pstHashTbl || XNULL == pulHashValues || XNULL == pulIndexs)
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //�������Ľڵ�������[�����ͬ�ؼ��ֽڵ���],��ʧ��
    if (ulValueNum > pstHashTbl->ulMaxSameKeyNode)
    {
        SYS_RETURN(XERROR);
    }
    //��������ֵ�ĸ���Ϊ 0,��ֱ�ӷ��سɹ�
    if (0 == ulValueNum)
    {
        SYS_RETURN(XSUCC);
    }

    //�����Ƿ�����ͬ�û�
    if (XSUCC == SYS_ExtendHashNodeSearch1(pstHashTbl, ulHashKey, ulValueNum, pulHashValues))
    {
        SYS_RETURN(XERROR);
    }

    //�鿴��û���㹻�Ŀ��нڵ�
    pstNode = (EXTEND_HASH_NODE*)pstHashTbl->stIdle.next;
    for (i = 0; i < ulValueNum; i++)
    {
        if (&pstNode->stLe == &pstHashTbl->stIdle)//û���㹻�Ŀ��нڵ�
        {
            SYS_RETURN(XERROR);
        }
        pstNode = (EXTEND_HASH_NODE*)pstNode->stLe.next;
    }

    //�ҳ���ϣͰ��������
    if(XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //�жϹ�ϣͰ�����źϷ���
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    for (i = 0; i < ulValueNum; i++)
    {
        pstNode = (EXTEND_HASH_NODE*)pstHashTbl->stIdle.next;
        SYS_ListDel(&pstNode->stLe);
        SYS_ListAdd(pstHashTbl->pstBin[ulBinIndex].prev, &pstNode->stLe);
        pstNode->ulHashKey = ulHashKey;
        pstNode->ulHashValue = pulHashValues[i];
        pulIndexs[i] = pstNode->ulIndex;
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashNodeDel  - ����չ��ϣ����ɾ��ָ���ؼ��ֵĲ��ֽڵ�������нڵ�

����˵����
    pstHashTbl        - ���룬��չ��ϣ��ָ��
    ulHashKey               - ���룬��Ҫɾ���Ľڵ�Ĺؼ���
    ulValueNum              - ���룬�����ֵ�ĸ��������� BLANK_ULONG(0xFFFFFFFF)ʱ��ʾҪɾ���ؼ�����ͬ�����нڵ�
    pulHashValues           - ���룬����ĸ��ڵ��ֵ(��ulValueNum==BLANK_ULONGʱ�����ֶβ�������)
    pulDelNum               - �������ɾ���Ľڵ����
    pulIndexs               - �������ɾ���Ľڵ����Ӧ������

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashNodeDel(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulValueNum, XU32 pulHashValues[], XU32 *pulDelNum, XU32 pulIndexs[])
{
    XU32      ulBinIndex, i;
    EXTEND_HASH_NODE   *pstNode;

    //�Ϸ��Լ��
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }
    if ((BLANK_ULONG != ulValueNum) && (ulValueNum > pstHashTbl->ulMaxSameKeyNode))
    {
        SYS_RETURN(XERROR);
    }

    //�ҳ���ϣͰ��������
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //�жϹ�ϣͰ�����źϷ���
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    pulDelNum[0] = 0;
    for (i = 0; i < ulValueNum; i++)
    {
        pstNode = (EXTEND_HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
        while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
        {
            if ((pstNode->ulHashKey == ulHashKey) && (pstNode->ulHashValue == pulHashValues[i]))
            {
                pulIndexs[i] = pstNode->ulIndex;
                pulDelNum[0]++;
                SYS_ListDel(&pstNode->stLe);
                SYS_ListAdd(pstHashTbl->stIdle.prev, &pstNode->stLe);
                break;
            }
            pstNode = (EXTEND_HASH_NODE*)pstNode->stLe.next;
        }
    }

    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ExtendHashNodeSearch  - ����չ��ϣ���в���һ���ڵ�

����˵����
    pstHashTbl      - ���룬��ϣ��ָ��
    ulHashKey       - ���룬�ؼ���
    pulValueNum     - ��������ҵ��Ĺ�ϣ�ڵ����
    ulHashValues    - �������ϣֵ�������
    ulIndexs        - ������ڵ��ھ�̬��ϣ���е�λ��(����)

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ExtendHashNodeSearch(EXTEND_HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 *pulValueNum, XU32 ulHashValues[], XU32 ulIndexs[])
{
    XU32      ulBinIndex;
    EXTEND_HASH_NODE   *pstNode;

    //�Ϸ��Լ��
    if(XNULL == pstHashTbl)
    {
        SYS_RETURN(XERROR);
    }
    if(XNULL == pstHashTbl->fnFunc)
    {
        SYS_RETURN(XERROR);
    }

    //�ҳ���ϣͰ��������
    if (XSUCC != pstHashTbl->fnFunc(ulHashKey, &ulBinIndex))
    {
        SYS_RETURN(XERROR);
    }
    //�жϹ�ϣͰ�����źϷ���
    if(ulBinIndex >= pstHashTbl->ulBinSize)
    {
        SYS_RETURN(XERROR);
    }

    *pulValueNum = 0;
    pstNode = (EXTEND_HASH_NODE*)pstHashTbl->pstBin[ulBinIndex].next;
    while (&pstNode->stLe != &pstHashTbl->pstBin[ulBinIndex])
    {
        if (pstNode->ulHashKey == ulHashKey)
        {
            if (*pulValueNum >= pstHashTbl->ulMaxSameKeyNode)
            {
                SYS_RETURN (XSUCC);
            }
            ulHashValues[*pulValueNum] = pstNode->ulHashValue;
            ulIndexs[*pulValueNum] = pstNode->ulIndex;
            pulValueNum[0]++;
        }
        pstNode = (EXTEND_HASH_NODE*)pstNode->stLe.next;
    }
    if (0 != pulValueNum[0])
    {
        SYS_RETURN(XSUCC);
    }
    else
    {
        SYS_RETURN(XERROR);
    }
}

XS32 SYS_HashNodeRebuild(HASH_TABLE *pstHashTbl, XU32 ulHashKey, XU32 ulHashValue, XU32 ulHashNodeIndex)
{
    SYS_RETURN(XSUCC);
}

XS32 SYS_ListIni(SYS_LISTENT_t *ent)
{
    ent->next = ent->prev = ent;
    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ListAdd    - ��һ�ڵ���ӵ�˫��������

����˵����
    pstTblHeader      - ���룬˫������ͷָ��
    pstNewNode        - ���룬˫�����½ڵ�

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ListAdd(SYS_LISTENT_t *old, SYS_LISTENT_t *add)
{
    add->next = old->next;
    add->prev = old;
    old->next = add;
    (add->next)->prev = add;
    SYS_RETURN(XSUCC);
}

/*---------------------------------------------------------------------
SYS_ListDel    - ��˫��������ɾ��һ���ڵ�

����˵����
    psDelNode      - ���룬��Ҫɾ���Ľڵ��ָ��

����ֵ: XSUCC, ��������ʧ�ܷ���FAIL
------------------------------------------------------------------------*/
XS32 SYS_ListDel(SYS_LISTENT_t *del)
{
    if(XNULL == del)
    {
        SYS_RETURN(XERROR);
    }

    (del->prev)->next = del->next;
    (del->next)->prev = del->prev;
    del->next = del->prev = del;
    SYS_RETURN(XSUCC);
}

#ifdef  __cplusplus
}
#endif


