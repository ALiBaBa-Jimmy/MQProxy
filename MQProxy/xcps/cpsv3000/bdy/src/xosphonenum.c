/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosphonenum.c
**
**  description: �����������Դ�ļ�
**
**  author: wentao
**
**  date:   2006.11.13
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "xosphonenum.h"
#include "xoscfg.h"
#include "xosmem.h"
#include "xosarray.h"

/*-------------------------------------------------------------------------
                ģ���ڲ��궨��
-------------------------------------------------------------------------*/

#define CHAR_NUM        12   /*�������ַ��ĸ���*/
#define MAX_INDEX_NUM    8192 /*������Ĵ�С����������������������8K*/



/*�ж��ַ�ch�Ƿ��ǺϷ��ĺ����ַ�*/
#define CHARACTER_ISVALID(ch) \
    ((('#'==(ch)) || ('*'==(ch)) || (((ch)>='0')&&((ch)<='9'))) ? XTRUE : XFALSE)

/*�ж��ַ�ch�Ƿ��ǺϷ��ĺ����ַ�*/
#define PHONENUM_ISVALID(pCh, info, i, actualLen) \
{\
    pCh = info;\
    for (i = 0; i<actualLen; i++)\
    {\
        if (XFALSE == CHARACTER_ISVALID(*pCh))\
        {\
            return XERROR;\
        }\
        pCh++;\
    }\
}

/*�ͷ��ڴ�*/
#define XX_FREE(pt)\
{\
    XOS_MemFree(FID_NUMTREE, (pt));\
    (pt) = XNULL;\
}

/*��ȡָ���ƫ����*/
#define POINTER_OFFSET(ch) \
    (('*' == (ch)) ? (CHAR_NUM-2) : (('#' == (ch)) ? (CHAR_NUM-1) : ((ch) - '0')))

/*-------------------------------------------------------------------------
                ģ���ڲ����ݽṹ
-------------------------------------------------------------------------*/
/*�����������ݽṹ*/
typedef struct Tree_Node t_PHONENUM;
struct Tree_Node
{
    XU8 info;
    t_CHARNET *property; /*��������Ҫ��ֵ*/
    t_PHONENUM* pChild[CHAR_NUM];   /*ָ���ӽڵ��ָ��*/
};

/*����������ݽṹ*/
typedef struct
{
    XU32    index;     /*����ֵ*/
    t_PHONENUM  *address;   /*����ַ*/
    t_CHARNET *property; /*���Ե�ַ*/
}t_INDEX;

/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
/*�����ȫ����*/
XSTATIC  t_PHONENUM *sys_pRoot = XNULL;

/*�������ڴ������Ǵ���*/
XSTATIC XOS_HARRAY HIndexTable = XNULL;

/*�ڰ�λ��������ʱʹ��*/
XSTATIC XS8 convenient[CHAR_NUM+1] = "1234567890*#";

/*-------------------------------------------------------------------------
                ģ���ڲ�����
-------------------------------------------------------------------------*/
/************************************************************
  �������ܣ���������ʼ��һ������������еĽڵ�
  ����    ��pTree ָ��ڵ��ָ��
			ch    �ڵ��д�ŵ��ַ�
  ����ֵ  ���ɹ�����    �ڵ��ַ
            ʧ�ܷ��أ�	XNULL
************************************************************/
XSTATIC t_PHONENUM* XOS_nodeConstruct(XU8 ch)
{
    t_PHONENUM* pTree;
    XS32 i;

    if (XNULL == (pTree = (t_PHONENUM*)XOS_MemMalloc(FID_NUMTREE,sizeof(t_PHONENUM))))/*�������ռ�*/
    {
        return XNULL;
    }

    /* ��ʼ����� */
    pTree->info = ch;
    pTree->property = XNULL;
    for (i=0; i<CHAR_NUM; i++)
    {
        pTree->pChild[i] = XNULL;
    }

    return pTree;
}

/************************************************************
  �������ܣ��жϽڵ��Ƿ�Ϊ��
  ����    �� pNode       ��ǰ�ڵ�
  ����ֵ  ��Ϊ�շ��� XTRUE�����򷵻� XFALSE
************************************************************/
XSTATIC XBOOL XOS_nodeIsNULL(t_PHONENUM* pNode)
{
    XS32 i;
    
    if (XNULL == pNode || XNULL != pNode->property)
    {
        return XFALSE;
    }

    for (i=0; i<CHAR_NUM; i++)
    {
        if (XNULL != pNode->pChild[i])
            return XFALSE;
    }

    return XTRUE;
}

/************************************************************************
 * checkIndexElem
 * ����: ��ɾ���ڵ�ʱ�����������ͬ��
 * ����  : arrayH     - ���������
 * ��� : pNode      - Ҫɾ���Ľ��ĵ�ַ
 * ���� : �ɹ�����XSUCC�����򷵻�XERROR(-1)
 *************************************************************************/
XSTATIC XS32  XOS_checkIndexElem(t_PHONENUM *pNode)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)HIndexTable;
    t_INDEX *pIndex = XNULL;
    XS32 i, count;

    if ( XTRUE  != XOS_ArrayHandleIsValid(HIndexTable) || XNULL == pNode)
    {
        return XERROR;  /*SYS_ARRAY_ERROR*/
    }

    count = 0;
    for (i=0; i<ra->maxNumOfElements; i++)
    {
        if (!XOS_ArrayElemIsVacant(HIndexTable, i)) 
        {
            count++;
            pIndex = (t_INDEX*)XOS_ArrayGetElemByPos(HIndexTable, i);
            if (XNULL != pIndex && pIndex->address == pNode)
            {
                pIndex->address = XNULL;
            }
        }
        if (count == ra->curNumOfElements)
        {
            break;
        }
    }
    return XSUCC;
}

/************************************************************
  �������ܣ��ݹ�ɾ�����е����нڵ������
  ����    �� pNode       ����������еĽڵ�
  ����ֵ  ���ɹ����� SUCC�����򷵻� XERROR
************************************************************/
XSTATIC XS32 XOS_nodeDestruct (t_PHONENUM* pNode)
{
    XS32 i;
    
    if (XNULL == pNode)/*��������*/
    {
        return XSUCC;
    }

    for (i=0; i<CHAR_NUM; i++)
    {
        XOS_nodeDestruct (pNode->pChild[i]);
        pNode->pChild[i] = XNULL;
    }

    /*�����Բ�Ϊ�գ����ͷ����Կռ䣬ɾ������*/
    if (XNULL != pNode->property)
    {
        XX_FREE(pNode->property);
    }

    /*�ͷŽڵ�ռ䣬ɾ���ڵ�*/
    XX_FREE(pNode);

    return XSUCC;
}

/*-------------------------------------------------------------------------
                ģ��ӿں���
-------------------------------------------------------------------------*/
/************************************************************
  �������ܣ���Ӻ��루�������ھ��ȴ�������
  ����    ��info	-	�绰�����ַ���(��'\0'�������ַ���������Ϊ0~ MAXPHONELEN)
            	    �绰���룬������"13800138000"
	        len	-	�绰����ĳ��ȣ���"13800138000"�ĳ���Ϊ 11
                    len ȡֵ��Χ��0 ~ MAXPHONELEN
	        property	-	������Ϣ������Ϊ��ʼ���˵Ľṹָ�룬��ӦΪXNULL��

  ����ֵ  ���ɹ�����    XSUCC
            ʧ�ܷ��أ�	1. һ��ʧ��XERROR(-1)(���磺��������,�ڴ����ʧ��)
                        2.�����Ѿ�����NUM_EXIST(2)

************************************************************/
XEXTERN XS32 XOS_PhoneAddInfo(XU8 *info, XU32 len, t_CHARNET *property)
{
    XS32 i, actualLen;/* ʵ����ӵĺ��볤�� */
    t_PHONENUM *pNode = XNULL, *pTemp = XNULL;
    XU8 *pCh = XNULL;

    /* ������֤ */
    if ((XNULL == info)||(0 == len)||(XNULL == property)||(MAXPHONELEN < (actualLen = (XU32)XOS_MIN(XOS_StrLen(info), len))))
    {
        return XERROR;
    }

    PHONENUM_ISVALID(pCh, info, i, actualLen);/* �жϺ����Ƿ���ȷ */

    if (XNULL == sys_pRoot)/* ��������δ���� */
    {
        if (XNULL == (sys_pRoot = XOS_nodeConstruct('T')))/* ��������ʼ��һ�����ڵ� */
        {
            return XERROR;
        }
        if (XNULL == (HIndexTable = XOS_ArrayConstruct( sizeof(t_INDEX), MAX_INDEX_NUM, "Index_Table")))/*����������*/
        {
            XX_FREE(sys_pRoot);
            return XERROR;
        }
    }

    if( 0 >= actualLen)
    {
        return XERROR;  /**/
    }
    
    pNode = sys_pRoot;
    pCh = info;
    for (i = 0; i<actualLen; i++)
    {
        if (XNULL == (pTemp = pNode->pChild[POINTER_OFFSET(*pCh)]))/* Ѱ����һ��ڵ� */
        {
            if (XNULL == (pTemp = XOS_nodeConstruct(*pCh)))/*�²�ڵ�ΪNULL�򴴽��½ڵ�*/
            {
                return XERROR;
            }
            pNode->pChild[POINTER_OFFSET(*pCh)] = pTemp;
        }
        pNode = pTemp;
        pCh++;
    }

    /*��û��Ԫ����ֵ�����򷵻�NUM_EXIST*/
    if (XNULL != pNode->property)
    {
        return NUM_EXIST;
    }

    if(XNULL == (pNode->property = (t_CHARNET *)XOS_MemMalloc(FID_NUMTREE,sizeof(t_CHARNET))))
    {
        return XERROR;
    }
    XOS_MemCpy(pNode->property, property, sizeof(t_CHARNET));

    return XSUCC;
}

/************************************************************
  �������ܣ�ɾ���绰����
  ����    ��info	-	�绰�����ַ���(��'\0'�������ַ���������Ϊ0~ MAXPHONELEN)
            	        �绰���룬������"13800138000"
	        len	-	�绰����ĳ��ȣ���"13800138000"�ĳ���Ϊ 11
            	        len ȡֵ��Χ��0 ~ MAXPHONELEN

  ����ֵ  ���ɹ�����	XSUCC
		    ʧ�ܷ��أ�	1. һ��ʧ��XERROR(-1)
		                3.���벻����NUM_NON(3)

************************************************************/
XEXTERN XS32 XOS_PhoneDelInfo(XU8 *info, XU32 len)
{
    XU8 *pCh = XNULL;
    XS32 i, actualLen;
    t_PHONENUM  *pNode = XNULL, *pTemp = XNULL;
    t_PHONENUM* pNodesList[MAXPHONELEN+1];/*���ڱ�������ɾ��·���ϵĽڵ��ַ*/

    if (XNULL == sys_pRoot)/* ��������δ���� */
    {
        return NUM_NON;
    }

    /* ������֤ */
    if ((XNULL == info) || (0 == len)||(MAXPHONELEN < (actualLen = (XU32)XOS_MIN(XOS_StrLen(info), len))))
    {
        return XERROR;
    }

    PHONENUM_ISVALID(pCh, info, i, actualLen);/* �жϺ����Ƿ���ȷ */

    if( 0 >= actualLen)
    {
        return XERROR;  /**/
    }
    
    for (i=0; i<=MAXPHONELEN; i++)/*��ʼ��ָ��ΪNULL*/
    {
        pNodesList[i] = XNULL;
    }
    pNodesList[0] = sys_pRoot;

    pNode = sys_pRoot;
    pCh = info;
    for (i = 1; i<=actualLen; i++)
    {
        if (XNULL == (pTemp = pNode->pChild[POINTER_OFFSET(*pCh)]))/* Ѱ����һ��ڵ� */
        {
            return NUM_NON;
        }
        pNode = pTemp;
        pNodesList[i] = pTemp;
        pCh++;
    }

    if (XNULL == pNode->property)/*�ж�ָ��λ�õ�����ֵ�Ƿ�Ϊ��*/
    {
        return NUM_NON;
    }

    XX_FREE(pNode->property);

    for (i = actualLen; i>0; i--)/*���ϻ��ݣ�������ɾ���Ľڵ�ɾ��*/
    {
        if (XNULL != pNodesList[i])
        {
            if (XFALSE == XOS_nodeIsNULL(pNodesList[i]))
            {
                return XSUCC;
            }
            if(XERROR == XOS_checkIndexElem(pNodesList[i])) /* */
            {
                return XERROR;
            }
            pNodesList[i-1]->pChild[POINTER_OFFSET(pNodesList[i]->info)] = XNULL;
            XX_FREE(pNodesList[i]);
        }
    }

    return XSUCC;
}

/************************************************************
  �������ܣ�ɾ�����в��żƻ�
  ����    ��N/A
  ����ֵ  ���ɹ�����    XSUCC
            ʧ�ܷ��أ�	XERROR
************************************************************/
XEXTERN XS32 XOS_DelAllPNum( XVOID )
{
    XS32 i, count;
    t_INDEX *pIndex;
    t_XOSARRAY *ra = (t_XOSARRAY *)HIndexTable;
    
    if (XNULL == sys_pRoot)
    {
        return XSUCC;
    }

    for (i=0; i<CHAR_NUM; i++)
    {
        XOS_nodeDestruct (sys_pRoot->pChild[i]);/*�ݹ�ɾ���ڵ㺯��*/
        sys_pRoot->pChild[i] = XNULL;
    }
/*
    if (XNULL != sys_pRoot->property)
    {
        XX_FREE(sys_pRoot->property);
    }
*/    
    count = 0;
    for (i=0; i<ra->maxNumOfElements; i++)
    {
        if (!XOS_ArrayElemIsVacant(HIndexTable, i)) 
        {
            count++;
            pIndex = (t_INDEX*)XOS_ArrayGetElemByPos(HIndexTable, i);
            if (XNULL != pIndex)
            {
                pIndex->address = XNULL;
            }
        }
        if (count == ra->curNumOfElements)
        {
            break;
        }
    }
    return XSUCC;
}

/************************************************************
  �������ܣ���������
  ����    ��info	-	�绰�����ַ���(��'\0'�������ַ���������Ϊ0~ MAXPHONELEN)
            	        �绰���룬������"13800138000"
	        len	-	�绰����ĳ��ȣ���" 13800138000"�ĳ���Ϊ 11
            	        len ȡֵ��Χ��0 ~ MAXPHONELEN
            property	-	���ز��ҵ��ĺ�����Ϣ

  ����ֵ  ���ɹ�����    XSUCC
	        ʧ�ܷ��� ��	XERROR
************************************************************/
XEXTERN XS32 XOS_PhoneSearchInfo(XU8 *info, XU32 len, t_CHARNET *property)
{
    XS32 i, actualLen;/* ʵ����ӵĺ��볤�� */
    t_PHONENUM* pNode = XNULL, *pTemp = XNULL;
    XU8 *pCh = XNULL;

    /* ������֤ */
    if ((XNULL == info) || (0 == len) || (XNULL == property) || (XNULL == sys_pRoot)||(MAXPHONELEN < (actualLen = (XU32)XOS_MIN(XOS_StrLen(info), len))))
    {
        return XERROR;
    }

    PHONENUM_ISVALID(pCh, info, i, actualLen);/* �жϺ����Ƿ���ȷ */
/*
    if( 0 >= actualLen)
    {
        return XERROR;  
    }
*/    
    pNode = sys_pRoot;
    pCh = info;
    for (i = 0; i<actualLen; i++)
    {
        if (XNULL == (pTemp = pNode->pChild[POINTER_OFFSET(*pCh)]))/* Ѱ����һ��ڵ� */
        {
            return XERROR;
        }
        pNode = pTemp;
        pCh++;
    }

    if (XNULL == pNode->property)/*�ж�ָ��λ�õ�����ֵ�Ƿ�Ϊ��*/
    {
        return XERROR;
    }
    XOS_MemCpy(property, pNode->property, sizeof(t_CHARNET));
    return XSUCC;
}

/************************************************************
  �������ܣ��������봮(��������)
  ����    ��info	-	�绰�����ַ���(��'\0'�������ַ���������Ϊ0~ MAXPHONELEN)
            	        �绰���룬������"13800138000"
	        len	-	�绰����ĳ��ȣ���" 13800138000"�ĳ���Ϊ 11
            	        len ȡֵ��Χ��0 ~ MAXPHONELEN
            property	-	���ط������ĺ�����Ϣ
            
  ����ֵ  ���ɹ���XSUCC �����غ������ԣ����ҳɹ�ʱ��Ч����
            ʧ�ܣ�XERROR
************************************************************/
XEXTERN XS32 XOS_AnaStrInfo(XU8 *info, XU32 len, t_CHARNET *property)
{
    XU32 i, actualLen, flag = 0;/* actualLen��ʵ�ʷ����ĺ��볤�� */
    t_PHONENUM* pNode = XNULL, *pTemp = XNULL;
    XU8 *pCh = XNULL;

    /* ������֤ */
    if ((XNULL == info) || (0 == len) || (XNULL == property) || (XNULL == sys_pRoot)||(MAXPHONELEN < (actualLen = (XU32)XOS_MIN(XOS_StrLen(info), len))))
    {
        return XERROR;
    }

    PHONENUM_ISVALID(pCh, info, i, actualLen);/* �жϺ����Ƿ���ȷ */

    pNode = sys_pRoot;
    pCh = info;
    for (i = 0; i<actualLen; i++)
    {
        if (XNULL == (pTemp = pNode->pChild[POINTER_OFFSET(*pCh)]))/* Ѱ����һ��ڵ� */
        {
            return((0 == flag) ? XERROR : XSUCC);
        }
        pNode = pTemp;
        if (XNULL != pNode->property)
        {
            XOS_MemCpy(property, pNode->property, sizeof(t_CHARNET));
            flag = 1;/*�ñ�־Ϊ1��˵��ƥ��ɹ�*/
        }
        pCh++;
    }

    return((1 == flag) ? XSUCC : XERROR);
}

/************************************************************
  �������ܣ���λ�������룬���鵽�ĺ�����Ϣ�����û������pWord_p��������Ӧ��index��Ϣ��
  ����    ��bcd  -   Ϊ�����1 λ��bcd�룬f1-f9 ,fa,fb,fc���ζ�Ӧ1��9��'0'��'*','#'��
            index    -     Ϊ��������������ƽ̨������Ҫ���壩
            pWord_p  -   ���صĽṹ��property�е�pWord���ṹ�嶨��������ϵͳ���ݽṹ��
            pLen	-	Ϊ�������ԵĽṹ����(����ȫ���������ֹ�)

  ����ֵ  �����ҽ����0�ɹ� -1 ʧ�� 2 �����Ѻţ�
            �������Լ�����(property�ṹ���е�pWord)��ȫ���ȣ����ҳɹ�ʱ��Ч��
            ��������������������ʱ��Ч��
            
  ע�⣺���ڷ���������index��
        ���������ǵ�һλ����ʱӦ����Ҫ������ֵ��ʼ��ΪINITANA����0xffffffff��
        ƽ̨�����ݴ�ֵ���ж��Ƿ�����������Ҫ�����ĺ��룩�ٽ����ַ������������
        ���ǲ��ҵ���Ӧ�ĺ���ͽ�����Ϣ�������������������أ�
        ֮���û���Ҫ��������ʱ�Ϳ��Խ��ϴη��ص������������ˡ�
************************************************************/
XEXTERN XS32 XOS_AnaSchInfo(XU32 bcd, XU8 *pWord_p, XU32 *pLen, XU32 *index)
{
    XU32 i;
    t_PHONENUM  *pNode = XNULL, *pTemp = XNULL;
    t_INDEX *pIndex = XNULL, *pIndexElem = XNULL;
    XU8 ch;

    /* ������� */
    if ((XNULL == pWord_p)||(XNULL == pLen)||(XNULL == index)||(XNULL == sys_pRoot)||(XFALSE == XOS_ArrayHandleIsValid(HIndexTable) ))
    {
        return XERROR;
    }

    /*��BCD��ת�����ַ�*/
    i = bcd & 0xf;
    if (i>0 && i<=CHAR_NUM)
    {
        (ch = convenient[i-1]);
    }
    else
    {
        return XERROR;
    }


    /*�������*indexȷ���Ǹտ�ʼ������һλ���룬���Ǽ�����������
      -----������ֵ�����ݽ����ж�*/
    if (*index<MAX_INDEX_NUM)
    {
        if (XNULL == (pIndex = (t_INDEX*)XOS_ArrayGetElemByPos(HIndexTable, *index)))
        {
            return XERROR;
        }
        pNode = pIndex->address;    /*** ***/
        if (XNULL == pNode)
        {
            if(XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�������*/
            {
                return XERROR;
            }
            return XERROR;
        }
    }
    else if (INITANA == *index)
    {
        pNode = sys_pRoot;
    }
    else
    {
        return XERROR;
    }


    /*-----------------------
      �Խ��pNodeΪ��ڷ�������*/
    pTemp = pNode->pChild[POINTER_OFFSET(ch)]; /*------ pTemp�ǵ�ǰ�����нڵ���ӽڵ�*/
    

    /*------�ӽڵ�ΪNULL*/
    if (XNULL == pTemp)/*�ӽڵ�ΪNULL*/
    {
        if (pNode == sys_pRoot)/*�տ�ʼ������һλ����*/
        {
            return XERROR;
        }
        if (XNULL == pIndex->property)/*���ǵ�һλ���룬�����������propertyΪNULL*/
        {
            if(XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�����*/
            {
                return XERROR;
            }    
            return XERROR;
        }

        /*���ǵ�һλ���룬�����������property��ΪNULL*/
        *pLen = ((t_CHARNET*)(pIndex->property))->len;
        *pWord_p = ((t_CHARNET*)(pIndex->property))->pWord;
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�����*/
        {
            return XERROR;
        }    
        return XSUCC;
    }


    /*�����Ƿ����õ���------�ӽڵ�pTemp��ΪNULL�����*/
    if (pNode == sys_pRoot)/*�տ�ʼ������һλ����*/
    {
    
        if (XNULL != pTemp->property && MEET_NUM == pTemp->property->pWord)
        {
            *pLen = pTemp->property->len;
            *pWord_p = MEET_NUM;
            *index = INITANA;/*���ҵ�����ţ����²���*/
            return NUM_EXIST;
        }
        
        /*��һ���ձ������ε���Ϣ*/
        if ((*index = XOS_ArrayAddExt(HIndexTable, (XVOID**)&pIndexElem)) >= MAX_INDEX_NUM)
        {
            return XERROR;
        }
        
        if (pIndexElem)
        {
            pIndexElem->index = *index;
            pIndexElem->address = pTemp;
            pIndexElem->property = XNULL;
            if (XNULL != pTemp->property) /*****/
            {
                pIndexElem->property = pTemp->property;
            }
        }
        
        for (i=0; i<CHAR_NUM; i++)
        {
            if (XNULL != pTemp->pChild[POINTER_OFFSET(convenient[i])])/*pTemp���ӽڵ�*/
            {
                return NUM_EXIST;
            }
        }
        
        if (XNULL == pTemp->property)/*pTemp������ΪNULL�������ӽڵ�; ###���߳�ʱ�ſ���ִ�е�*/
        {
            if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�����*/
            {
                return XERROR;
            }    
            return XERROR;
        }
        
        *pLen = pTemp->property->len;/*���Բ�ΪNULL�������ӽڵ�*/
        *pWord_p = pTemp->property->pWord;
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�����*/
        {
            return XERROR;
        }    
        return XSUCC;
    }


    /*������pTemp��ΪNULL,��------pNode���Ǹ��ڵ�����*/
    if (XNULL == pTemp->property)/*���ǵ�һλ���룬���ӽڵ�� propertyΪNULL NULL*/
    {/*ֻ�޸��������еĽ���ַ�����޸�index��property*/

        pIndex->address = pTemp;
        
        for (i=0; i<CHAR_NUM; i++)
        {
            if (XNULL != pTemp->pChild[POINTER_OFFSET(convenient[i])])/*pTemp���ӽڵ�*/
            {
                return NUM_EXIST;
            }
        }

        /*ͬ��###��ֻ��һ���û�ʱ���ᷢ��������������߳�ʱ�ſ��ܷ���*/
        if (XNULL == pIndex->property)/*�������������ΪNULL����pTemp���ӽڵ�*/
        {
            if(XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�����*/
            {
                return XERROR;
            }    
            return XERROR;
        }
        
        *pLen = pIndex->property->len;/*������������Բ�ΪNULL�������ӽڵ�*/
        *pWord_p = pIndex->property->pWord;
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�����*/
        {
            return XERROR;
        }    
        return XSUCC;
    }


/*���´���pNode���Ǹ��ڵ㣬pTemp��ΪNULL����property��ΪNULL�����*/
    pIndex->address = pTemp;
    pIndex->property = pTemp->property;
    if (MEET_NUM == pTemp->property->pWord)/*���ҵ�����ţ����²���*/
    {
        *pLen = pTemp->property->len;
        *pWord_p = MEET_NUM;
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�����*/
        {
            return XERROR;
        }    
        *index = INITANA;
        return NUM_EXIST;
    }

    
    for (i=0; i<CHAR_NUM; i++)
    {
        if (XNULL != pTemp->pChild[POINTER_OFFSET(convenient[i])])/*��������ӺŲ�������*/
        {
            return NUM_EXIST;
        }
    }

    
    *pLen = pTemp->property->len;
    *pWord_p = pTemp->property->pWord;
    if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, *index)) /*�ͷ�����*/
    {
        return XERROR;
    }    
    return XSUCC;
}

/************************************************************
  �������ܣ�Index�Ŀռ��ͷ�
  ����    ��indexnum	-	Ϊ׼���ͷŵ�index�ţ���Χ��0 ~ MAX_INDEX_NUM
  
  ����ֵ  ���ɹ�����   XSUCC
	        ʧ�ܷ���   XERROR
***********************************************************/
XEXTERN XS32 XOS_IndexFree(XU32 indexnum)
{
    if((indexnum>=0)&&(indexnum < MAX_INDEX_NUM)&&(XTRUE == XOS_ArrayHandleIsValid(HIndexTable)))
    {
        if( XERROR == XOS_ArrayDeleteByPos(HIndexTable, indexnum)) /*�ͷ�����*/
        {
            return XERROR;
        }    
        return XSUCC;
    }

    return XERROR;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

