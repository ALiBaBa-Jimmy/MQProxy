/***************************************************************
 **
 **  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
 **  
 **  Core Network Department  platform team  
 **
 **  filename: xostrie.c
 **
 **  description:  
 **
 **  author: zengjiandong
 **
 **  date:   2006.4.7
 **
 ***************************************************************
 **                          history                     
 **  
 ***************************************************************
 **   author                 date                modification            
 **   zengjiandong        2006.4.7              create  
 **************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                                ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xostrie.h"
#include "xostrace.h"
#include "xoscfg.h"

/*-------------------------------------------------------------------------
                                 �궨��
-------------------------------------------------------------------------*/
#define minLongMask 16     /*���������С����*/
#define maxLongMask 32    /*���������󳤶�*/
#define minShortMask 8      /*���������С����*/
#define maxShortMask 15    /*���������󳤶�*/
#define nodeSize 16             /*16P�ڵ�Ĵ�С*/

/*-------------------------------------------------------------------------
                           �ṹ��ö������
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                            ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                              ģ���ڲ�����
-------------------------------------------------------------------------*/

/************************************************************************
 * ��������FormString
 * ����: ��ip ת��Ϊ���ִ��洢��ptr �У����ҷ��ش��ĳ���
 * ����: ip                                         - ip  ��ַ
 *              masklen                                - ���볤��
 * ���: ptr                                        - ���ڴ洢���ִ�
 * ����: ���ִ��ĳ���
 ************************************************************************/
XSTATIC  XU32  FormString( XU32 ip, XU32 masklen, XU32 *ptr )
{
    XU32 i, length = 0;

    if (minShortMask > masklen || maxLongMask < masklen)
    {
        return length;
    }
    /*��������ĳ���*/
    if (minLongMask == masklen || minShortMask == masklen)
    {
        length = 1;
    }
    else if((minShortMask < masklen && 12 >= masklen) ||(minLongMask < masklen && 20 >= masklen))
    {
        length = 2;
    }
    else if((12 < masklen && 15 >= masklen) ||(20 < masklen && 24 >= masklen))
    {
        length = 3;
    }
    else if(24 < masklen && 28 >= masklen)
    {
        length = 4;
    }
    else if(28 < masklen && maxLongMask >= masklen)
    {
        length = 5;
    }

    /*��ip��ת�������ִ�*/
    if (minLongMask <= masklen && maxLongMask >= masklen)      /*����������*/
    {
        *ptr = (ip>>16) & 0XFFFF;
        for(i = 1; i<length; i++)
        {
            ptr++;
            *ptr = (ip>>(16 - i*4))&0XF;
        }
    }
    
    if (minShortMask <= masklen && maxShortMask >= masklen)       /*����������*/
    {
        *ptr = (ip>>24) & 0XFF;
        for(i = 1; i<length; i++)
        {
            ptr++;
            *ptr = (ip>>(24 - i*4))&0XF;
        }
    }
    return length;
}


/************************************************************************
 * ��������FindLocation
 * ����: ��Trie ��������Ԫ�ص�λ��
 * ����: pRoot                            - ָ����ĵ�һ��Ԫ��
 *              ptr                                - ���IP������
 *              length                           - ���Ĳ��
 * ���: pOutElem                      - ��������Ԫ�صĵ�ַ
 * ����: �ɹ��Ҳ����ײ㷵�ؽڵ��ַ,
 *               ʧ�ܻ�Ԫ�����ײ㷵��XNULL
 ************************************************************************/
XSTATIC  t_TRIENODE* FindLocation(t_TRIEELEM* pRoot, XU32* ptr, XU32 length,  XVOID* *pOutElem)
{
    XU32 i, index = 0;
    t_TRIENODE*  pNode = XNULL;
    if (XNULL == pRoot || XNULL == ptr || 0 == length)
    {
        return XNULL;
    }
    index = *ptr++;

    /*ֻ��λ�����ڵ㷵�ؿ�,�����Ϊ���ڵ��ĳ��Ԫ�صĵ�ַ*/
    *pOutElem = &(pRoot[index]);
    if (1 == length || XNULL == pRoot[index].pNext)
    {
        return  XNULL;
    }

    pNode = (pRoot[index]).pNext;
    index = *ptr;
    *pOutElem = &(pNode->node[index]);
    for (i = 2; i<length; i++)
    {
        if (XNULL == (pNode->node[index]).pNext)
        {
            return XNULL;
        }
        pNode = (pNode->node[index]).pNext;
        ptr++;
        index =*ptr;
        *pOutElem = &(pNode->node[index]);
    }
    if (i < length)
    {
        return XNULL;             /*����������ָ���ĵط��򷵻�XNULL*/
    }
    
    return pNode;               /*������ָ���ĵط��򷵻ؽڵ�ĵ�ַ*/
}


/************************************************************************
 * ��������TravelNodes
 * ����: ��XOS_TrieTravel���ã����ڱ���16P �ڵ�
 * ����: pTrie                   - Trie�����
 *              ptr                       - 16P �ڵ�
 * ���: ��
 * ����: ��
 ************************************************************************/
XSTATIC  XVOID  TravelNodes(XOS_HTRIE HTrie, t_TRIENODE*  ptr )
{
    XS32 i, j, k, n;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIENODE* pNode = ptr;
    t_TRIEELEM *pElem1 = XNULL, *pElem2 = XNULL, *pElem3 = XNULL, *pElem4 = XNULL;
    if (XNULL == pNode || XNULL == pTrie->printElemFunc)
    {
        return;
    }
    pElem1 = pNode->node;
    for (i = 0; i<nodeSize; i++)                              /* ��һ��*/
    {
        if (minShortMask < pElem1->maskLen)
        {
            pTrie->printElemFunc(pElem1);
        }
        if (XNULL != pElem1->pNext)
        {
            pElem2 = pElem1->pNext->node;

            for (j = 0; j<nodeSize; j++)                     /*�ڶ���*/
            {
                if (minShortMask < pElem2->maskLen)
                {
                    pTrie->printElemFunc(pElem2);
                }
                if (XNULL != pElem2->pNext)
                {
                    pElem3 = pElem2->pNext->node;
                    
                    for (k = 0; k<nodeSize; k++)            /*������*/
                    {
                        if (minShortMask < pElem3->maskLen)
                        {
                            pTrie->printElemFunc(pElem3);
                        }
                        if (XNULL != pElem3->pNext)
                        {
                            pElem4 = pElem3->pNext->node;
                            
                            for (n = 0; n<nodeSize; n++)    /*���Ĳ�*/
                            {
                                if (minShortMask < pElem4->maskLen)
                                {
                                    pTrie->printElemFunc(pElem4);
                                }
                                pElem4++;
                            }
                        }
                        pElem3++;
                    }
                }
                pElem2++;
            }
        }
        pElem1++;
    }
}

/*-------------------------------------------------------------------------
                                       ģ��ӿں���
-------------------------------------------------------------------------*/

/************************************************************************
 * ��������XOS_TrieHandleIsValid
 * ����: �жϴ�����trie�����Ƿ���Ч
 * ����: HTrie             - Trie�����
 * ���: ��
 * ����: �ɹ�����XSUCC,ʧ�ܷ���XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieHandleIsValid( XOS_HTRIE  HTrie)
{
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    
    if (XNULLP == pTrie)
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_TrieConstruct()-> initial  function error!\n");
        return XERROR;
    }

    /*�ж�ע��ĺ����Ƿ���Ч*/
    if (XNULLP == pTrie->addNodeFunc ||XNULLP == pTrie->delNodeFunc||XNULLP == pTrie->downloadFunc 
         || XNULLP == pTrie->printElemFunc || XNULLP == pTrie->rootDownloadFunc)
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_TrieConstruct()-> register  function error!\n");
        return XERROR;
    }
    
    if (XNULLP == pTrie->pLongMask || XNULLP == pTrie->pShortMask)
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_TrieConstruct()-> no root of trie tree!\n");
        return XERROR;
    }

    return XSUCC;
}


/************************************************************************
 * ��������XOS_TrieElemAdd
 * ����: ����һ���µ�Ԫ�ص�trie����
 * ����: HTrie                          - Trie�����
 *              pKey                           - �ؼ���
 *              pElement                     - Ҫ�����trie���е�Ԫ��
 * ���: ��
 * ����: ��ӳɹ�����XSUCC, ʧ�ܷ���XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieElemAdd(    XOS_HTRIE   HTrie,
                                                                 XVOID*        pKey,
                                                                 XVOID*        pElement)
{
    XU32 i, net, masklen, index;
    XU32 array[5] = {0, 0, 0, 0, 0}, layers;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP;
    t_TRIENODE *pNode = XNULLP;/*��ʼ��ΪNULL*/

    /*�ж�Trie������Ƿ���Ч��keyָ���Ƿ�ΪXNULL*/
    if ( XERROR == XOS_TrieHandleIsValid( HTrie ) || XNULLP == pKey )
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieElemAdd()->Param Error!\n");
        return XERROR;
    }

    net = ((t_TRIEKEY*)pKey)->ip;
    masklen = ((t_TRIEKEY*)pKey)->masklen;
    if (masklen<minShortMask || masklen>maxLongMask)
    {
        return XERROR;/*���볤�Ȳ��ڷ�Χ���򷵻�*/
    }

    /*�������ת��Ϊ���ִ����洢��a[5], layersΪ���ĳ���*/
    layers = FormString(net, masklen, &array[0]);
    if (layers<1 || layers>5)
    {
        return XERROR;
    }

    /*�Ӹ��ڵ㴦��ʼ����*/
    index = array[0];
    if (masklen<minLongMask)
    {
        pElem = &(pTrie->pShortMask[index]);
    }
    else
    {
        pElem = &(pTrie->pLongMask[index]);
    }
    
    for (i=1; i<layers; i++)
    {
        if (XNULLP == pElem->pNext)
        {   /*�ڵ㲻���ڣ���û�е���ȷ����ʱ�����ڵ�*/
            if (pTrie->numOfNode == pTrie->maxNumOfNode)/* ���ж�16P �ڵ��Ƿ�����*/
            {
                XOS_PRINT(MD(FID_ROOT, PL_WARN), "XOS_TrieElemAdd()->16P nodes exhaust!\n");
                return XERROR;
            }
            pElem->pNext = pTrie->addNodeFunc();  /*����һ��16P�ڵ�*/
            if (XNULLP != pElem->pNext)
            {
                /*memset(pElem->pNext, 0, sizeof(t_TRIENODE));*/
                pTrie->numOfNode = pTrie->numOfNode + 1;
                if(0 == pElem->maskLen && XNULLP != pNode)
                {
                    pNode->usage = pNode->usage + 1;
                    pTrie->downloadFunc(pNode);               /*�ڵ㱻�޸�*/
                }
                else if (XNULLP == pNode)
                {
                    pTrie->rootDownloadFunc(pElem);/*���ڵ��ָ�뱻�޸�*/
                }
                (pElem->pNext)->pParent = pNode;
                pTrie->downloadFunc(pElem->pNext);        /*�ڵ㱻�޸�*/
            }
            else
            {
                return XERROR;
            }
        }
        
        pNode = pElem->pNext;    /*������һ��*/
        index = array[i];
        pElem = &(pNode->node[index]);
    }

    /*�����Ǹ��ڵ�,Ԫ��Ϊ����û��ָ����һ��ʱ,ʹ������һ*/
    if ((XNULLP !=pNode) && (XNULLP == pElem->pNext) && (0 == pElem->maskLen))
    {
        pNode->usage = pNode->usage + 1;
        pTrie->downloadFunc(pNode);               /*�ڵ㱻�޸�*/
    }
    
    /* ԭ��λ��û�д洢Ԫ��*/
    if (0 == pElem->maskLen)
    {
        pTrie->numOfRt = pTrie->numOfRt + 1;       /*Rt����1*/
    }
    
    /*���ӵ�Ԫ�ص����볤�ȴ��ڻ����ԭ��Ԫ�ص����볤
       ��ԭ��Ԫ�ص����볤Ϊ0 ʱ�Ÿ���ԭ����ֵ       */
    if (masklen >= pElem->maskLen)
    {
        pElem->ip = net;
        pElem->maskLen = masklen;
        pElem->pRt = pElement;
        if (XNULLP !=pNode)/*�ж��Ƿ��Ǹ��ڵ㱻�޸���*/
        {
            pTrie->downloadFunc(pNode);               /*�ڵ㱻�޸�*/
        }
        else
        {
            pTrie->rootDownloadFunc(pElem);/*���ڵ㱻�޸���*/
        }
    }

    return XSUCC;
}


/************************************************************************
 * ��������XOS_TrieElemDel
 * ����: ��trie����ɾ��һ��Ԫ��
 * ����: HTrie                           - Trie�����
 *              pKey                            - �ؼ���
 *              pElement                      - Ҫɾ����trie���е�Ԫ��
 * ���: ��
 * ����: ɾ���ɹ�����XSUCC, ʧ�ܷ���XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieElemDel(   XOS_HTRIE      HTrie,
                                                               XVOID*          pKey,
                                                               XVOID*          pElement  )
{
    XU32 i, net, masklen;
    XU32 array[5] = {0, 0, 0, 0, 0}, layers;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP, *pRoot = XNULLP;
    t_TRIENODE *pNode = XNULLP, *pTemp = XNULLP;

    /*�ж�Trie������Ƿ���Ч��keyָ���Ƿ�ΪXNULL*/
    if ( XERROR == XOS_TrieHandleIsValid( HTrie ) || XNULL == pKey )
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieElemDel()->Param Error!\n");
        return XERROR;
    }

    net = ((t_TRIEKEY*)pKey)->ip;
    masklen = ((t_TRIEKEY*)pKey)->masklen;
    if (masklen<minShortMask || masklen>maxLongMask)
    {
        return XERROR; /*���볤�Ȳ��ڷ�Χ���򷵻�*/
    }

    /*�������ת��Ϊ���ִ����洢��a[5], layersΪ���ĳ���*/
    layers = FormString(net, masklen, &array[0]);

    /*��Ҫɾ����Ԫ���ڸ��ڵ�ʱ*/
    if (1 == layers && minLongMask == masklen)
    {
        pElem = &(pTrie->pLongMask[array[0]]);
        if (pElem->ip == net && pElem->pRt == pElement)
        {
            pElem->ip = 0;
            pElem->maskLen = 0;
            pElem->pRt =XNULLP;
            pTrie->rootDownloadFunc(pElem);
            pTrie->numOfRt = pTrie->numOfRt - 1;
            return XSUCC;
        }
        return XERROR;
    }
    if (1 == layers && minShortMask == masklen)
    {
        pElem = &(pTrie->pShortMask[array[0]]);
        if (pElem->ip == net && pElem->pRt == pElement)
        {
            pElem->ip = 0;
            pElem->maskLen = 0;
            pElem->pRt =XNULLP;
            pTrie->rootDownloadFunc(pElem);
            pTrie->numOfRt = pTrie->numOfRt - 1;
            return XSUCC;
        }
        return XERROR;
    }
    
    /*�����ж�λ�ڵ��Ԫ�ص�λ��*/
    if (masklen >= minLongMask)
    {
        pRoot = pTrie->pLongMask;
        pNode = FindLocation(pTrie->pLongMask, &array[0], layers, (XVOID* *)(&pElem));
    }
    else
    {
        pRoot = pTrie->pShortMask;
        pNode = FindLocation(pTrie->pShortMask, &array[0], layers, (XVOID* *)(&pElem));
    }

    /* û�ҵ��򷵻�XERROR */
    if (XNULLP == pNode ||XNULLP == pElem)
    {
        return XERROR;
    }

    if (pElem->ip == net && pElem->maskLen == masklen && pElem->pRt == pElement)
    {  /*  �����������Ƿ����*/
        /*  ���Ԫ��*/
        pElem->ip = 0;
        pElem->maskLen = 0;
        pElem->pRt = XNULLP;
        if (XNULLP == pElem->pNext)
        {
            pNode->usage = pNode->usage - 1;
        }
        pTrie->numOfRt = pTrie->numOfRt - 1;
        pTrie->downloadFunc(pNode);            /* ֪ͨ�û��޸���16P  �ڵ�*/

        /*  ���ݣ����Ƿ�����Ҫɾ���Ľڵ㣬����ɾ��*/
        for (i = layers-1; i > 0; i--)
        {
            if (0 == pNode->usage)
            {
                if (XNULLP == pNode->pParent)
                {
                    (pRoot[array[0]]).pNext = XNULLP;
                    pTrie->rootDownloadFunc(&(pRoot[array[0]]));
                    pTrie->numOfNode = pTrie->numOfNode - 1;
                    pTrie->delNodeFunc(pNode);    /*ɾ��16P�ڵ�*/
                }
                else
                {
                    pTemp = pNode->pParent;
                    pElem = &(pTemp->node[array[i-1]]);
                    pElem->pNext = XNULLP;
                    pTrie->downloadFunc(pTemp);    /* ֪ͨ�û��޸���16P  �ڵ�*/
                    pTrie->numOfNode = pTrie->numOfNode - 1;
                    pTrie->delNodeFunc(pNode);    /*ɾ��16P�ڵ�*/
                    pNode = pTemp;
                    if (0 == pElem->maskLen)
                    {
                        pNode->usage = pNode->usage - 1;
                        pTrie->downloadFunc(pNode);  /* ֪ͨ�û��޸���16P  �ڵ�*/
                    }
                }
            }
            else
            {
                return XSUCC;
            }
        }
        return XSUCC;
    }
    return XERROR;
}


/************************************************************************
 * ��������XOS_TrieSearch
 * ����: ��Trie���в���һ��Ԫ��
 * ����: HTrie                                      - Trie�����
 *              pKey                                      - �ؼ���
 * ���: pRt                                        - ���ҵ���Rt��Ϣ
                 ʧ�ܻ�ɹ����ҵ�RtΪNULLʱ���ΪXNULL,
                 �ɹ���Rt��ΪNULL�����Rt�ĵ�ַ
 * ����: ���ҳɹ�����XSUCC, ʧ�ܷ���XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieSearch( XOS_HTRIE HTrie, XVOID*  pKey, XVOID** pRt)
{
    XU32 net, masklen;
    XU32 array[5] = {0, 0, 0, 0, 0}, layers;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP;
    t_TRIENODE *pNode = XNULLP;

    /*���Ϊ���򷵻�XERROR */
    if (XNULLP == pTrie || XNULLP == pKey || XNULLP == pRt)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieSearch()->Param Error!\n");
        if (XNULLP != pRt)
        {*pRt = XNULLP;}
        return XERROR;
    }

    *pRt = XNULLP;/*��ʼ�����ֵΪNULL*/
    net = ((t_TRIEKEY*)pKey)->ip;
    masklen = ((t_TRIEKEY*)pKey)->masklen;
    if (masklen<minShortMask || masklen>maxLongMask)
    {
        return XERROR; /*���볤�Ȳ��ڷ�Χ���򷵻�*/
    }

    /*�������ת��Ϊ���ִ����洢��a[5], layersΪ���ĳ���*/
    layers = FormString(net, masklen, &array[0]);

    /*�����ж�λ�ڵ��Ԫ��*/
    if (masklen >= minLongMask)
    {
        pNode = FindLocation(pTrie->pLongMask, &array[0], layers, (XVOID* *)(&pElem));
    }
    else
    {
        pNode = FindLocation(pTrie->pShortMask, &array[0], layers, (XVOID* *)(&pElem));
    }

    /*û�ҵ��򷵻ؿ�*/
    if (XNULLP == pNode && XNULLP == pElem)
    {
        return XERROR;
    }

    /*�ȱȽ��Ƿ����������������򷵻�Rt*/
    if (XNULLP != pElem && pElem->maskLen == masklen && pElem->ip == net && XNULLP != pElem->pRt)
    {
        *pRt = pElem->pRt;
        return XSUCC;
    }
    
    return XERROR;
}


/************************************************************************
 * ��������XOS_TrieALUSearch
 * ���� : ��Trie���в���һ��Ԫ�أ�����ip�Ӹ�λ����λ�����ܶ��ƥ��
 * ���� : HTrie                                     - Trie�����
 *               ip                                          - �����
 * ���: pRt                                        - ���ҵ���Rt��Ϣ
                 ʧ�ܻ�ɹ����ҵ�RtΪNULLʱ���ΪXNULL,
                 �ɹ���Rt��ΪNULL�����Rt�ĵ�ַ
 * ���� : ���ҳɹ�����XSUCC, ʧ�ܷ���XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieALUSearch( XOS_HTRIE HTrie, XU32  ip, XVOID** pRt)
{
    XU32 array[5] = {0, 0, 0, 0, 0}, index;
    XS32 i;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP, *ptr = XNULLP;
    t_TRIENODE *pNode = XNULLP;
    
    /*����ľ��Ϊ���򷵻�XERROR */
    if (XNULLP == pTrie || XNULLP == pRt)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieALUSearch()->Param Error!\n");
        if (XNULLP != pRt)
        {*pRt = XNULLP;}
        return XERROR;
    }
    
    /*  ���ڳ��������в���*/
    /*  �������ת��Ϊ�ַ������У��洢��a[5]  ��*/
    FormString(ip, maxLongMask, &array[0]);
    index = array[0];
    pElem = &(pTrie->pLongMask[index]);

    /*  �ڳ������������¾������������*/
    for (i=1; i<=5; i++)
    {
        if (pElem->maskLen >= minLongMask)
        {
            ptr = pElem;
        }
        
        if (XNULLP == pElem->pNext)
        {
            break;
        }
        
        pNode = pElem->pNext;
        index = array[i];
        pElem = &(pNode->node[index]);
    }

    if (XNULLP != ptr && minLongMask <= ptr->maskLen && XNULLP != ptr->pRt)
    {
        *pRt = ptr->pRt;
        return XSUCC;   /*  �ҵ����򷵻�Rt �ĵ�ַ������Rt  ����Ϊ��*/
    }
    
    /*  �ڶ��������в���*/
    /*  �������ת��Ϊ�ַ������У��洢��a[5] ��*/
    FormString(ip, maxShortMask, &array[0]);            
    index = array[0];
    pElem = &(pTrie->pShortMask[index]);

    /*  �ڶ������������¾������������*/
    for (i=1; i<=3; i++)
    {
        if (pElem->maskLen >= minShortMask)
        {
            ptr = pElem;
        }
        
        if (XNULLP == pElem->pNext)
        {
            break;
        }
        
        pNode = pElem->pNext;
        index = array[i];
        pElem = &(pNode->node[index]);
    }

    if (XNULLP != ptr && minShortMask <= ptr->maskLen && XNULLP != ptr->pRt)
    {
        *pRt = ptr->pRt;
        return XSUCC;         /*  �ҵ����򷵻�Rt �ĵ�ַ��Rt����Ϊ��*/
    }

    *pRt = pTrie->defMask;/*û��ƥ���Ԫ��,�����ҵ���RtΪNULL,�����Ĭ������*/
    return XERROR;
}


/************************************************************************
 * ��������XOS_TrieTravel
 * ����: ��Trie���б���
 * ����: HTrie                           - Trie�����
 * ���: ��
 * ����: �����ɹ�����XSUCC, ʧ�ܷ���XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieTravel(XOS_HTRIE  HTrie)
{
    XS32 i;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP;

    /*�жϾ���Ƿ���Ч */
    if (XERROR == XOS_TrieHandleIsValid( HTrie ))
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieTravel()->Invalid Trie Tree Handle!\n");
        return XERROR;
    }

    /* ���Trie ��ͳ����Ϣ*/
    XOS_PRINT(MD(FID_ROOT, PL_INFO), "Show trie tree massage:\n");
    XOS_PRINT(MD(FID_ROOT, PL_INFO), "count of  current 16P nodes : %d\n",pTrie->numOfNode);/*Ŀǰ16P�ڵ�ĸ���*/
    XOS_PRINT(MD(FID_ROOT, PL_INFO), "count of current Rt : %d\n",pTrie->numOfRt);/*Ŀǰ�洢��Rt��Ϣ�ĸ���*/
    XOS_PRINT(MD(FID_ROOT, PL_INFO), "maxnum of nodes : %d\n",pTrie->maxNumOfNode);/*16P�ڵ���������*/

    /* ���ڳ��������б���*/
    pElem = pTrie->pLongMask;
    for (i = 0; i<0xFFFF; i++)
    {
        if (minLongMask == pElem->maskLen)
        {
            pTrie->printElemFunc(pElem);
        }
        if (XNULL != pElem->pNext)
        {
            TravelNodes(HTrie, pElem->pNext);
        }
        pElem++;
    }

    /*  �ڶ��������б���*/
    pElem = pTrie->pShortMask;
    for (i = 0; i<0xFF; i++)
    {
        if (minShortMask == pElem->maskLen)
        {
            pTrie->printElemFunc(pElem);
        }
        if (XNULLP != pElem->pNext)
        {
            TravelNodes(HTrie, pElem->pNext);
        }
        pElem++;
    }
    return XSUCC;
}


/************************************************************************
 * ��������XOS_TrieSetDefaultMask
 * ����: ����Trie����Ĭ������(��������ƥ��ʱ����)
 * ����: HTrie                                     - Trie�����
 *              pMask                                    - Ĭ������
 * ���: ��
 * ����: �����ɹ�����XSUCC, ʧ�ܷ���XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieSetDefaultMask( XOS_HTRIE  HTrie, XVOID* pMask)
{
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;

    /*�жϾ���Ƿ���Ч */
    if (XERROR == XOS_TrieHandleIsValid( HTrie ))
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieSetDefaultMask()->Invalid Trie Tree Handle!\n");
        return XERROR;
    }

    pTrie->defMask = pMask;
    return XSUCC;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

