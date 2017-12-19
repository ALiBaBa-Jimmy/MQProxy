/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xoslist.h
**
**  description:  
**
**  author: wangzongyou
**
**  date:   2006.6.26
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author              date              modification            
**  
**************************************************************/
#ifdef  __cplusplus
extern   "C"  {
#endif  /*  __cplusplus */

#include "xosarray.h"
#include "xoslist.h"
#include "xostrace.h"
/*-------------------------------------------------------------------------
               ģ���ڲ��궨��
-------------------------------------------------------------------------*/
#define INVALID_LIST_LOCATION                                  (-1)


/*-------------------------------------------------------------------------
               ģ���ڲ����ݽṹ
-------------------------------------------------------------------------*/
typedef struct 
{
    XS32 prev;
    XS32 next;
    XCHAR *data;
} t_LISTELEM;


/*-------------------------------------------------------------------------
              ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              ģ���ڲ�����
-------------------------------------------------------------------------*/
/************************************************************************
* XOS_listAddNode
* ����: ��ӽڵ�
* ����:
*       hlist       - ������
*       elem        - Ԫ��
* ���: ��
* ����: �ɹ�����������λ����Ϣ
*       ʧ�ܷ���XERROR
************************************************************************/
XSTATIC XS32 XOS_listAddNode(XOS_HLIST hlist, nodeType elem)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    XS32 loc;
    t_LISTELEM *elemList;

    if ( (loc = XOS_ArrayAddExt(raH, (XVOID **)&elemList)) < 0)
    {
        return XERROR;
    }

    if (elem)
    {
        XOS_MemCpy((XVOID *)&(elemList->data), elem, XOS_ArrayGetElemSize(raH)-sizeof(t_LISTELEM)+sizeof(XCHAR*));
    }
    else
    {
        XOS_MemSet((XVOID *)&(elemList->data), 0, XOS_ArrayGetElemSize(raH)-sizeof(t_LISTELEM)+sizeof(XCHAR*));
    }

    elemList->prev = elemList->next = XERROR;
    return loc;
}


/*-------------------------------------------------------------------------
                      ģ��ӿں���
-------------------------------------------------------------------------*/
/************************************************************************
* XOS_listConstruct
* ����: ����һ������ ����
* ����:
*              elemSize                         - Ԫ�صĴ�С(�ֽ�Ϊ��λ)
*              maxNumOfElements                 - ���Ԫ����
*              name                             - ��������
* ���: ��
* ����: �ɹ��������������
*               ʧ�ܷ���XNULL
************************************************************************/
XOS_HLIST XOS_listConstruct(
                           XS32          elemSize,
                           XS32          maxNumOfElements,
                           const XCHAR*  name)
{
    XOS_HARRAY raH;

    if (elemSize <= 0 || maxNumOfElements <= 0)
    {
        return XNULL;
    }
    raH = XOS_ArrayConstruct((XS32)(elemSize + sizeof(t_LISTELEM) - sizeof(XCHAR*)),
                             maxNumOfElements,
                             name);

    if (raH == XNULL)
    {
        return XNULL;
    }

    XOS_ArraySetFirstNode(raH, INVALID_LIST_LOCATION);
    XOS_ArraySetLastNode(raH, INVALID_LIST_LOCATION);
    return (XOS_HLIST)raH;
}


/************************************************************************
* XOS_listDestruct
* ����: �ͷ�һ������ ����
* ����: hlist                        - ������
* ���: ��
* ����: 
************************************************************************/
XVOID XOS_listDestruct(XOS_HLIST hlist)
{
    XOS_ArrayDestruct((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listClear
* ����:���������ʼ��״̬
* ����: hlist                        - ������
* ���: ��
* ����: 
************************************************************************/
XVOID XOS_listClear(XOS_HLIST hlist)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;

    XOS_ArrayInitClear(raH);
    XOS_ArraySetFirstNode(raH, INVALID_LIST_LOCATION);
    XOS_ArraySetLastNode(raH, INVALID_LIST_LOCATION);
}


/************************************************************************
* XOS_listSetCompareFunc
* ����: ���ñȽϺ���
* ����:
*       hlist                        - ������
*       func                         - �ȽϺ���ָ��
* ���: ��
* ����: �ɹ�����XSUCC,ʧ�ܷ���XERROR
************************************************************************/
XS32 XOS_listSetCompareFunc( XOS_HLIST hlist,  nodeCmpFunc func)
{
    return XOS_ArraySetCompareFunc((XOS_HARRAY)hlist, func);
}


/************************************************************************
* XOS_listElemIsVacant
* ����: �жϽڵ��Ƿ�Ϊ��
* ����: hlist                        - ������
* ���: ��
* ����:�����Ϊ�շ���XTURE, ��Ϊ�շ���XFALSE
*      ʧ�ܷ���XERROR 
************************************************************************/
XS32 XOS_listElemIsVacant(XOS_HLIST hlist, XS32 location)                
{
    if (XNULL == hlist)
    {
        return XERROR;
    }
    return (XS32)XOS_ArrayElemIsVacant((XOS_HARRAY)hlist, location);
}


/************************************************************************
* XOS_listCurSize
* ����: ��ǰ������Ԫ�صĸ���
* ����: hlist                        - ������
* ���: ��
* ����: ��ǰԪ�صĸ���,���󷵻�XERROR
************************************************************************/
XS32 XOS_listCurSize(XOS_HLIST hlist)
{
    return XOS_ArrayGetCurElemNum((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listMaxSize
* ����:��ȡ�����ܴ洢Ԫ�ص�������
* ����: hlist                        - ������
* ���: ��
* ����: �����Դ洢Ԫ�صĸ���,���󷵻�XERROR
************************************************************************/
XS32     XOS_listMaxSize    (XOS_HLIST hlist)
{
    return XOS_ArrayGetMaxElemNum((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listHead
* ����: ��ȡ����ͷ��Ԫ�ص�����
* ����: hlist                        - ������
* ���: ��
* ����: ��ȡͷ�ڵ������,���󷵻�XERROR
************************************************************************/
XS32 XOS_listHead(XOS_HLIST hlist)
{
    return XOS_ArrayGetFirstNode((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listTail
* ����: ��ȡ����β��Ԫ�ص�����
* ����: hlist                        - ������
* ���: ��
* ����: ��ȡβ�ڵ������,���󷵻�XERROR
************************************************************************/
XS32 XOS_listTail(XOS_HLIST hlist)
{
    return XOS_ArrayGetLastNode((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listNext
* ����: ��ȡ��һ���ڵ������
* ����:
*       hlist                        - ������
*       location                     - ��ǰԪ�ص�����
* ���: ��
* ����: ��һ���ڵ������,���󷵻�XERROR
************************************************************************/
XS32 XOS_listNext(XOS_HLIST hlist, XS32 location)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    t_LISTELEM *elem;

    if ( !(elem = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, location)))
    {
        return XERROR;
    }
    else
    {
        return elem->next;
    }
}


/************************************************************************
* XOS_listPrev
* ����: ��ȡǰһ���ڵ������
* ����:
*       hlist                        - ������
*       location                     - ��ǰԪ�ص�����
* ���: ��
* ����: ǰһ���ڵ������,���󷵻�XERROR
************************************************************************/
XS32 XOS_listPrev(XOS_HLIST hlist, XS32 location)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    t_LISTELEM *elem;

    if (!(elem = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, location)))
    {
        return XERROR;
    }
    else
    {
        return elem->prev;
    }
}


/************************************************************************
* XOS_listMaxUsage
* ����: ��ȡ���ʹ�ù�Ԫ��,����ͳ��
* ����: hlist                        - ������
* ���: ��
* ����: ���ʹ�ù�������
************************************************************************/
XS32 XOS_listMaxUsage(XOS_HLIST hlist)
{
    return XOS_ArrayGetMaxUsageElemNum((XOS_HARRAY)hlist);
}


/************************************************************************
* XOS_listMaxUsage
* ����: ͨ��������ȡԪ��
* ����:
*       hlist                        - ������
*       location                     - Ԫ���������е�����
* ���: ��
* ����: �ڵ�Ԫ�ص��������е��׵�ַ
************************************************************************/
nodeType XOS_listGetElem(XOS_HLIST hlist, XS32 location)
{
    t_LISTELEM *elem;

    if (!(elem = (t_LISTELEM *)XOS_ArrayGetElemByPos((XOS_HARRAY)hlist, location)))
    {
        return NULL;
    }
    else
    {
        return &(elem->data);
    }
}


/************************************************************************
* XOS_listGetByPtr
* ����: ͨ��Ԫ��ָ���ȡԪ������
* ����:
*       hlist                        - ������
*       ptr                          - Ԫ���������е��׵�ַ
* ���: ��
* ����: ����ֵ,���󷵻�XERROR
************************************************************************/
XS32 XOS_listGetByPtr(XOS_HLIST hlist, XVOID *ptr)
{
    return XOS_ArrayGetByPtr((XOS_HARRAY)hlist, (XCHAR *)ptr - 2*sizeof(XS32));
}


/************************************************************************
* XOS_listCompare
* ����: ֱ��ͨ���ȽϺ�������ƥ���Ԫ��
* ����:
*       hlist                     - ������
*       location                  - ���ҽڵ����ʼ����
*       param                     - �ȽϺ�������
*       compare                   - �ȽϺ���
* ���: ��
* ����: ƥ��Ԫ�ص�����ֵ,���󷵻�XERROR
************************************************************************/
XS32 XOS_listCompare(XOS_HLIST hlist, XS32 location, XVOID *param, nodeCmpFunc compare)
{
    nodeType *elem;
    XS32 cur;

    for (cur=location; cur != INVALID_LIST_LOCATION; cur=XOS_listNext(hlist, cur)) 
    {
        if (!(elem = (nodeType*)XOS_listGetElem(hlist, cur)))
        {
            return XERROR; /* invalid element */
        }
        if ( (compare && compare(elem, param)) || elem == param)
        {
            return cur;
        }
    }

    return XERROR;
}


/************************************************************************
* XOS_listFind
* ����:�ҳ�ƥ���Ԫ��
* ����:
*       hlist                        - ������
*       location                     - ���ҽڵ����ʼ����
*       param                        - �ȽϺ�������
* ���: �� 
* ����: ƥ��Ԫ�ص�����ֵ,���󷵻�XERROR
************************************************************************/
XS32 XOS_listFind(XOS_HLIST hlist, XS32 location, XVOID *param)
{
    return XOS_listCompare(hlist, location, param, (nodeCmpFunc)XOS_ArrayGetCompareFunc((XOS_HARRAY)hlist));
}


/************************************************************************
* XOS_listAdd
* ����: ��ڵ�����Ϊlocation���һ���ڵ�
* ����:
*       hlist                   - ������
*       location                - �ڵ�����
*       elem                    - �ڵ��Ԫ��(��ӽڵ�ǰ����һ����ʱ����
*                                 �洢Ҫ��ӵ�Ԫ�أ���ӽڵ�ʱ�ڽӿڴ���
*                                 ������ַ��Ԫ�������봴��ʱ��ͬ)
* ���: ��
* ����: ���Ԫ�������е�����,���󷵻�XERROR
************************************************************************/
XS32 XOS_listAdd(XOS_HLIST hlist, XS32 location, nodeType elem)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    XS32 Apos, Bpos, Cpos=INVALID_LIST_LOCATION;
    t_LISTELEM *A, *B, *C;

    if ( (Bpos = XOS_listAddNode(hlist, elem)) < 0)
    {
        return XERROR;
    }
    if (Bpos == location)
    {
        location = XERROR;/*���ڵ���뵽����ͷ*/
    }
    (location<0)?(Apos=INVALID_LIST_LOCATION):(Apos=location);

    B = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Bpos);
    A = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Apos);
    (A)?(Cpos=A->next):(Cpos=XOS_ArrayGetFirstNode(raH), Apos=INVALID_LIST_LOCATION);
    C = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Cpos);

    B->prev = Apos;
    B->next = Cpos;
    if (C)
    {
        C->prev = Bpos;
    }
    else
    {
        XOS_ArraySetLastNode(raH, Bpos);
    }
    if (A)
    {
        A->next = Bpos;
    }
    else
    {
        XOS_ArraySetFirstNode(raH, Bpos);
    }

    return Bpos;
}

/************************************************************************
* XOS_listAddHead
* ����:������ͷ����һ���ڵ�
* ����:
*       hlist       - ������
*       elem        - �ڵ��Ԫ��(��ӽڵ�ǰ����һ����ʱ����
*                              �洢Ҫ��ӵ�Ԫ�أ���ӽڵ�ʱ�ڽӿڴ���
*                              ������ַ��Ԫ�������봴��ʱ��ͬ)
* ���: ��
* ����: ���Ԫ�������е�����ֵ,���󷵻�XERROR
************************************************************************/
XS32 XOS_listAddHead(XOS_HLIST hlist, nodeType elem)
{
    return XOS_listAdd(hlist, INVALID_LIST_LOCATION, elem);
}


/************************************************************************
* XOS_listAddTail
* ����: ��β����һ���ڵ�
* ����:
*       hlist       - ������
*       elem        - �ڵ��Ԫ��(��ӽڵ�ǰ����һ����ʱ����
*                              �洢Ҫ��ӵ�Ԫ�أ���ӽڵ�ʱ�ڽӿڴ���
*                              ������ַ��Ԫ�������봴��ʱ��ͬ)
* ���: ��
* ����: ���Ԫ�������е�����ֵ,���󷵻�XERROR
************************************************************************/
XS32 XOS_listAddTail(XOS_HLIST hlist, nodeType elem)
{
    return XOS_listAdd(hlist, XOS_listTail(hlist), elem);
}


/************************************************************************
* XOS_listDelete
* ����:ɾ��ָ��λ�õĽڵ�
* ����:
*       hlist                        - ������
*       location                     - Ҫɾ���Ľڵ������
* ���: ��
* ����: �ɹ�XTRUE, ʧ��XFALSE
************************************************************************/
XBOOL XOS_listDelete(XOS_HLIST hlist, XS32 location)
{
    XOS_HARRAY raH = (XOS_HARRAY)hlist;
    XS32 Apos, Cpos;
    t_LISTELEM *A, *B, *C;

    if (XNULLP == hlist)
    {
        return XFALSE;
    }
    if (XOS_ArrayElemIsVacant(raH, location) == XTRUE)
    {
        return XFALSE;
    }
    B = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, location);
    if (!B)
    {
        return XFALSE;
    }

    Apos = B->prev;
    Cpos = B->next;
    A = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Apos);
    C = (t_LISTELEM *)XOS_ArrayGetElemByPos(raH, Cpos);

    if (C)
    {
        C->prev = Apos;
    }
    else
    {
        XOS_ArraySetLastNode(raH, Apos);
    }
    if (A)
    {
        A->next = Cpos;
    }
    else
    {
        XOS_ArraySetFirstNode(raH, Cpos);
    }

    XOS_ArrayDeleteByPos(raH, location);
    return XTRUE;
}


/************************************************************************
* XOS_listDeleteHead
* ����: ɾ��ͷ�ڵ�
* ����: hlist                        - ������
* ���: ��
* ����: �ɹ�XTRUE, ʧ��XFALSE
************************************************************************/
XBOOL XOS_listDeleteHead(XOS_HLIST hlist)
{
    return XOS_listDelete(hlist, XOS_listHead(hlist));
}


/************************************************************************
* XOS_listDeleteTail
* ����:ɾ��β���Ľڵ�
* ����: hlist                        - ������
* ���: ��
* ����: �ɹ�XTRUE, ʧ��XFALSE
************************************************************************/
XBOOL XOS_listDeleteTail(XOS_HLIST hlist)
{
    return XOS_listDelete(hlist, XOS_listTail(hlist));
}


/************************************************************************
* XOS_listDeleteAll
* ����: ɾ�����еĽڵ�
* ����: hlist                        - ������
* ���: ��
* ����: 
************************************************************************/
XBOOL XOS_listDeleteAll(XOS_HLIST hlist)
{
    while (XOS_listDeleteHead(hlist));
    return XTRUE;
}


#ifdef  __cplusplus
}
#endif  /*  __cplusplus */



