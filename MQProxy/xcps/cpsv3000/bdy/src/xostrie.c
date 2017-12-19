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
                                包含头文件
-------------------------------------------------------------------------*/
#include "xostrie.h"
#include "xostrace.h"
#include "xoscfg.h"

/*-------------------------------------------------------------------------
                                 宏定义
-------------------------------------------------------------------------*/
#define minLongMask 16     /*长掩码的最小长度*/
#define maxLongMask 32    /*长掩码的最大长度*/
#define minShortMask 8      /*短掩码的最小长度*/
#define maxShortMask 15    /*短掩码的最大长度*/
#define nodeSize 16             /*16P节点的大小*/

/*-------------------------------------------------------------------------
                           结构和枚举声明
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                            模块内部全局变量
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                              模块内部函数
-------------------------------------------------------------------------*/

/************************************************************************
 * 函数名：FormString
 * 功能: 将ip 转换为数字串存储在ptr 中，并且返回串的长度
 * 输入: ip                                         - ip  地址
 *              masklen                                - 掩码长度
 * 输出: ptr                                        - 用于存储数字串
 * 返回: 数字串的长度
 ************************************************************************/
XSTATIC  XU32  FormString( XU32 ip, XU32 masklen, XU32 *ptr )
{
    XU32 i, length = 0;

    if (minShortMask > masklen || maxLongMask < masklen)
    {
        return length;
    }
    /*计算数组的长度*/
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

    /*将ip号转换成数字串*/
    if (minLongMask <= masklen && maxLongMask >= masklen)      /*长掩码的情况*/
    {
        *ptr = (ip>>16) & 0XFFFF;
        for(i = 1; i<length; i++)
        {
            ptr++;
            *ptr = (ip>>(16 - i*4))&0XF;
        }
    }
    
    if (minShortMask <= masklen && maxShortMask >= masklen)       /*短掩码的情况*/
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
 * 函数名：FindLocation
 * 功能: 在Trie 树中索引元素的位置
 * 输入: pRoot                            - 指向根的第一个元素
 *              ptr                                - 存放IP的数组
 *              length                           - 树的层次
 * 输出: pOutElem                      - 索引到的元素的地址
 * 返回: 成功且不在首层返回节点地址,
 *               失败或元素在首层返回XNULL
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

    /*只定位到根节点返回空,但输出为根节点的某个元素的地址*/
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
        return XNULL;             /*不能索引到指定的地方则返回XNULL*/
    }
    
    return pNode;               /*索引到指定的地方则返回节点的地址*/
}


/************************************************************************
 * 函数名：TravelNodes
 * 功能: 由XOS_TrieTravel调用，用于遍历16P 节点
 * 输入: pTrie                   - Trie树句柄
 *              ptr                       - 16P 节点
 * 输出: 无
 * 返回: 无
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
    for (i = 0; i<nodeSize; i++)                              /* 第一层*/
    {
        if (minShortMask < pElem1->maskLen)
        {
            pTrie->printElemFunc(pElem1);
        }
        if (XNULL != pElem1->pNext)
        {
            pElem2 = pElem1->pNext->node;

            for (j = 0; j<nodeSize; j++)                     /*第二层*/
            {
                if (minShortMask < pElem2->maskLen)
                {
                    pTrie->printElemFunc(pElem2);
                }
                if (XNULL != pElem2->pNext)
                {
                    pElem3 = pElem2->pNext->node;
                    
                    for (k = 0; k<nodeSize; k++)            /*第三层*/
                    {
                        if (minShortMask < pElem3->maskLen)
                        {
                            pTrie->printElemFunc(pElem3);
                        }
                        if (XNULL != pElem3->pNext)
                        {
                            pElem4 = pElem3->pNext->node;
                            
                            for (n = 0; n<nodeSize; n++)    /*第四层*/
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
                                       模块接口函数
-------------------------------------------------------------------------*/

/************************************************************************
 * 函数名：XOS_TrieHandleIsValid
 * 功能: 判断创建的trie对象是否有效
 * 输入: HTrie             - Trie树句柄
 * 输出: 无
 * 返回: 成功返回XSUCC,失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieHandleIsValid( XOS_HTRIE  HTrie)
{
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    
    if (XNULLP == pTrie)
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_TrieConstruct()-> initial  function error!\n");
        return XERROR;
    }

    /*判断注册的函数是否有效*/
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
 * 函数名：XOS_TrieElemAdd
 * 功能: 增加一个新的元素到trie树中
 * 输入: HTrie                          - Trie树句柄
 *              pKey                           - 关键字
 *              pElement                     - 要插入的trie树中的元素
 * 输出: 无
 * 返回: 添加成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieElemAdd(    XOS_HTRIE   HTrie,
                                                                 XVOID*        pKey,
                                                                 XVOID*        pElement)
{
    XU32 i, net, masklen, index;
    XU32 array[5] = {0, 0, 0, 0, 0}, layers;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP;
    t_TRIENODE *pNode = XNULLP;/*初始化为NULL*/

    /*判断Trie树句柄是否有效和key指针是否为XNULL*/
    if ( XERROR == XOS_TrieHandleIsValid( HTrie ) || XNULLP == pKey )
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieElemAdd()->Param Error!\n");
        return XERROR;
    }

    net = ((t_TRIEKEY*)pKey)->ip;
    masklen = ((t_TRIEKEY*)pKey)->masklen;
    if (masklen<minShortMask || masklen>maxLongMask)
    {
        return XERROR;/*掩码长度不在范围内则返回*/
    }

    /*将网络号转换为数字串，存储在a[5], layers为串的长度*/
    layers = FormString(net, masklen, &array[0]);
    if (layers<1 || layers>5)
    {
        return XERROR;
    }

    /*从根节点处开始索引*/
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
        {   /*节点不存在，且没有到达确定层时创建节点*/
            if (pTrie->numOfNode == pTrie->maxNumOfNode)/* 先判断16P 节点是否用完*/
            {
                XOS_PRINT(MD(FID_ROOT, PL_WARN), "XOS_TrieElemAdd()->16P nodes exhaust!\n");
                return XERROR;
            }
            pElem->pNext = pTrie->addNodeFunc();  /*增加一个16P节点*/
            if (XNULLP != pElem->pNext)
            {
                /*memset(pElem->pNext, 0, sizeof(t_TRIENODE));*/
                pTrie->numOfNode = pTrie->numOfNode + 1;
                if(0 == pElem->maskLen && XNULLP != pNode)
                {
                    pNode->usage = pNode->usage + 1;
                    pTrie->downloadFunc(pNode);               /*节点被修改*/
                }
                else if (XNULLP == pNode)
                {
                    pTrie->rootDownloadFunc(pElem);/*根节点的指针被修改*/
                }
                (pElem->pNext)->pParent = pNode;
                pTrie->downloadFunc(pElem->pNext);        /*节点被修改*/
            }
            else
            {
                return XERROR;
            }
        }
        
        pNode = pElem->pNext;    /*到达下一层*/
        index = array[i];
        pElem = &(pNode->node[index]);
    }

    /*当不是根节点,元素为空且没有指向下一层时,使用数加一*/
    if ((XNULLP !=pNode) && (XNULLP == pElem->pNext) && (0 == pElem->maskLen))
    {
        pNode->usage = pNode->usage + 1;
        pTrie->downloadFunc(pNode);               /*节点被修改*/
    }
    
    /* 原来位置没有存储元素*/
    if (0 == pElem->maskLen)
    {
        pTrie->numOfRt = pTrie->numOfRt + 1;       /*Rt数加1*/
    }
    
    /*增加的元素的掩码长度大于或等于原有元素的掩码长
       或原有元素的掩码长为0 时才覆盖原来的值       */
    if (masklen >= pElem->maskLen)
    {
        pElem->ip = net;
        pElem->maskLen = masklen;
        pElem->pRt = pElement;
        if (XNULLP !=pNode)/*判断是否是根节点被修改了*/
        {
            pTrie->downloadFunc(pNode);               /*节点被修改*/
        }
        else
        {
            pTrie->rootDownloadFunc(pElem);/*根节点被修改了*/
        }
    }

    return XSUCC;
}


/************************************************************************
 * 函数名：XOS_TrieElemDel
 * 功能: 在trie树中删除一个元素
 * 输入: HTrie                           - Trie树句柄
 *              pKey                            - 关键字
 *              pElement                      - 要删除的trie树中的元素
 * 输出: 无
 * 返回: 删除成功返回XSUCC, 失败返回XERROR
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

    /*判断Trie树句柄是否有效和key指针是否为XNULL*/
    if ( XERROR == XOS_TrieHandleIsValid( HTrie ) || XNULL == pKey )
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieElemDel()->Param Error!\n");
        return XERROR;
    }

    net = ((t_TRIEKEY*)pKey)->ip;
    masklen = ((t_TRIEKEY*)pKey)->masklen;
    if (masklen<minShortMask || masklen>maxLongMask)
    {
        return XERROR; /*掩码长度不在范围内则返回*/
    }

    /*将网络号转换为数字串，存储在a[5], layers为串的长度*/
    layers = FormString(net, masklen, &array[0]);

    /*当要删除的元素在根节点时*/
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
    
    /*在树中定位节点和元素的位置*/
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

    /* 没找到则返回XERROR */
    if (XNULLP == pNode ||XNULLP == pElem)
    {
        return XERROR;
    }

    if (pElem->ip == net && pElem->maskLen == masklen && pElem->pRt == pElement)
    {  /*  掩码和网络号是否相等*/
        /*  清空元素*/
        pElem->ip = 0;
        pElem->maskLen = 0;
        pElem->pRt = XNULLP;
        if (XNULLP == pElem->pNext)
        {
            pNode->usage = pNode->usage - 1;
        }
        pTrie->numOfRt = pTrie->numOfRt - 1;
        pTrie->downloadFunc(pNode);            /* 通知用户修改了16P  节点*/

        /*  回溯，看是否有需要删除的节点，有则删除*/
        for (i = layers-1; i > 0; i--)
        {
            if (0 == pNode->usage)
            {
                if (XNULLP == pNode->pParent)
                {
                    (pRoot[array[0]]).pNext = XNULLP;
                    pTrie->rootDownloadFunc(&(pRoot[array[0]]));
                    pTrie->numOfNode = pTrie->numOfNode - 1;
                    pTrie->delNodeFunc(pNode);    /*删除16P节点*/
                }
                else
                {
                    pTemp = pNode->pParent;
                    pElem = &(pTemp->node[array[i-1]]);
                    pElem->pNext = XNULLP;
                    pTrie->downloadFunc(pTemp);    /* 通知用户修改了16P  节点*/
                    pTrie->numOfNode = pTrie->numOfNode - 1;
                    pTrie->delNodeFunc(pNode);    /*删除16P节点*/
                    pNode = pTemp;
                    if (0 == pElem->maskLen)
                    {
                        pNode->usage = pNode->usage - 1;
                        pTrie->downloadFunc(pNode);  /* 通知用户修改了16P  节点*/
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
 * 函数名：XOS_TrieSearch
 * 功能: 在Trie树中查找一个元素
 * 输入: HTrie                                      - Trie树句柄
 *              pKey                                      - 关键字
 * 输出: pRt                                        - 查找到的Rt信息
                 失败或成功查找但Rt为NULL时输出为XNULL,
                 成功且Rt不为NULL则输出Rt的地址
 * 返回: 查找成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieSearch( XOS_HTRIE HTrie, XVOID*  pKey, XVOID** pRt)
{
    XU32 net, masklen;
    XU32 array[5] = {0, 0, 0, 0, 0}, layers;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP;
    t_TRIENODE *pNode = XNULLP;

    /*句柄为空则返回XERROR */
    if (XNULLP == pTrie || XNULLP == pKey || XNULLP == pRt)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieSearch()->Param Error!\n");
        if (XNULLP != pRt)
        {*pRt = XNULLP;}
        return XERROR;
    }

    *pRt = XNULLP;/*初始化输出值为NULL*/
    net = ((t_TRIEKEY*)pKey)->ip;
    masklen = ((t_TRIEKEY*)pKey)->masklen;
    if (masklen<minShortMask || masklen>maxLongMask)
    {
        return XERROR; /*掩码长度不在范围内则返回*/
    }

    /*将网络号转换为数字串，存储在a[5], layers为串的长度*/
    layers = FormString(net, masklen, &array[0]);

    /*在树中定位节点和元素*/
    if (masklen >= minLongMask)
    {
        pNode = FindLocation(pTrie->pLongMask, &array[0], layers, (XVOID* *)(&pElem));
    }
    else
    {
        pNode = FindLocation(pTrie->pShortMask, &array[0], layers, (XVOID* *)(&pElem));
    }

    /*没找到则返回空*/
    if (XNULLP == pNode && XNULLP == pElem)
    {
        return XERROR;
    }

    /*先比较是否满足条件，符合则返回Rt*/
    if (XNULLP != pElem && pElem->maskLen == masklen && pElem->ip == net && XNULLP != pElem->pRt)
    {
        *pRt = pElem->pRt;
        return XSUCC;
    }
    
    return XERROR;
}


/************************************************************************
 * 函数名：XOS_TrieALUSearch
 * 功能 : 在Trie树中查找一个元素，根据ip从高位到低位尽可能多地匹配
 * 输入 : HTrie                                     - Trie树句柄
 *               ip                                          - 网络号
 * 输出: pRt                                        - 查找到的Rt信息
                 失败或成功查找但Rt为NULL时输出为XNULL,
                 成功且Rt不为NULL则输出Rt的地址
 * 返回 : 查找成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieALUSearch( XOS_HTRIE HTrie, XU32  ip, XVOID** pRt)
{
    XU32 array[5] = {0, 0, 0, 0, 0}, index;
    XS32 i;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP, *ptr = XNULLP;
    t_TRIENODE *pNode = XNULLP;
    
    /*输入的句柄为空则返回XERROR */
    if (XNULLP == pTrie || XNULLP == pRt)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieALUSearch()->Param Error!\n");
        if (XNULLP != pRt)
        {*pRt = XNULLP;}
        return XERROR;
    }
    
    /*  先在长掩码树中查找*/
    /*  将网络号转换为字符串序列，存储在a[5]  中*/
    FormString(ip, maxLongMask, &array[0]);
    index = array[0];
    pElem = &(pTrie->pLongMask[index]);

    /*  在长掩码树中往下尽可能深地索引*/
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
        return XSUCC;   /*  找到有则返回Rt 的地址，但是Rt  可能为空*/
    }
    
    /*  在短掩码树中查找*/
    /*  将网络号转换为字符串序列，存储在a[5] 中*/
    FormString(ip, maxShortMask, &array[0]);            
    index = array[0];
    pElem = &(pTrie->pShortMask[index]);

    /*  在短掩码树中往下尽可能深地索引*/
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
        return XSUCC;         /*  找到有则返回Rt 的地址，Rt可能为空*/
    }

    *pRt = pTrie->defMask;/*没有匹配的元素,或者找到的Rt为NULL,则输出默认网关*/
    return XERROR;
}


/************************************************************************
 * 函数名：XOS_TrieTravel
 * 功能: 在Trie树中遍历
 * 输入: HTrie                           - Trie树句柄
 * 输出: 无
 * 返回: 遍历成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieTravel(XOS_HTRIE  HTrie)
{
    XS32 i;
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;
    t_TRIEELEM *pElem = XNULLP;

    /*判断句柄是否有效 */
    if (XERROR == XOS_TrieHandleIsValid( HTrie ))
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_TrieTravel()->Invalid Trie Tree Handle!\n");
        return XERROR;
    }

    /* 输出Trie 树统计信息*/
    XOS_PRINT(MD(FID_ROOT, PL_INFO), "Show trie tree massage:\n");
    XOS_PRINT(MD(FID_ROOT, PL_INFO), "count of  current 16P nodes : %d\n",pTrie->numOfNode);/*目前16P节点的个数*/
    XOS_PRINT(MD(FID_ROOT, PL_INFO), "count of current Rt : %d\n",pTrie->numOfRt);/*目前存储的Rt信息的个数*/
    XOS_PRINT(MD(FID_ROOT, PL_INFO), "maxnum of nodes : %d\n",pTrie->maxNumOfNode);/*16P节点的最大数量*/

    /* 先在长掩码树中遍历*/
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

    /*  在短掩码树中遍历*/
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
 * 函数名：XOS_TrieSetDefaultMask
 * 功能: 设置Trie树的默认网关(当查找无匹配时返回)
 * 输入: HTrie                                     - Trie树句柄
 *              pMask                                    - 默认网关
 * 输出: 无
 * 返回: 遍历成功返回XSUCC, 失败返回XERROR
 ************************************************************************/
XPUBLIC  XS32  XOS_TrieSetDefaultMask( XOS_HTRIE  HTrie, XVOID* pMask)
{
    t_XOSTRIE* pTrie = (t_XOSTRIE*)HTrie;

    /*判断句柄是否有效 */
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

