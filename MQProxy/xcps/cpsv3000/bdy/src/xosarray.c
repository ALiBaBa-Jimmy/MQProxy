/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosarray.c
**
**  description:  
**
**  author: zengjiandong
**
**  date:   2006.3.21
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author              date              modification            
**   zengjiandong    2006.3.21       create  
**   zengjiandong    2006.3.23       revise
**************************************************************/
#ifdef  __cplusplus
extern   "C"  {
#endif  /*  __cplusplus */
    
#include "xosarray.h"
#include "xostrace.h"
#include "xoscfg.h"
#include "xosmem.h"
/*-------------------------------------------------------------------------
                           ģ���ڲ��궨��
-------------------------------------------------------------------------*/
/*��ζ���*/
#define ALIGN sizeof(XVOID*)

/*�����־λռ�ÿռ�Ĵ�С*/
#define BIT_VECTOR_SIZE(n)   ( ((XU32)n+7)/8 + (ALIGN - (((XU32)n+7)/8)%ALIGN)%ALIGN)

/*ȡ��һ������Ԫ�ص�����*/
#define NEXT_VACANT(ra, i)  (((vacantNode *)ELEM_DATA(ra, i))->nextVacant)

/*-------------------------------------------------------------------------
                        ģ���ڲ����ݽṹ
-------------------------------------------------------------------------*/
typedef struct {
    XS32   nextVacant;                /* ָ����һ�����е�λ��*/
} vacantNode;

/*-------------------------------------------------------------------------
                        ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                          ģ���ڲ�����
-------------------------------------------------------------------------*/
/*********************************************************************
*                  ����ָ����־λ��ֵ�����Ƿ����
*                  value��ֵΪtrue��ʾռ��,false��ʾ����
*********************************************************************/
XSTATIC  XVOID  setBit(XCHAR *ptr, XU32 bit, XBOOL value)
{
    if (value)
    {
        ptr[bit>>3] |=  (0x80 >> (bit & 7));
    }
    else
    {
        ptr[bit>>3] &= ~(0x80 >> (bit & 7));
    }
}


XBOOL XOS_ArrayIsUesd(XOS_HARRAY raH, void* ptr)
{
    XS32 pos = 0;
    pos = XOS_ArrayGetByPtr(raH, ptr);
    if(pos != XERROR) {
        return (((getBit((BIT_VECTOR(raH)), (pos)))!= 0) ? (XTRUE):(XFALSE));
    }
    return XTRUE;
}

/**********************************************************************
*                               �ж��������Ƿ���Ч
**********************************************************************/
#if 0
XBOOL   XOS_ArrayHandleIsValid(  XOS_HARRAY arrayH )
{
    t_XOSARRAY  *ptr=(t_XOSARRAY *)XNULL;
    if( XNULL == arrayH)
    {
        return( XFALSE );
    }
    ptr = (t_XOSARRAY *)arrayH;
    if ( ARRAY_MAGIC_VALUE  == ptr->magicVal )
    {
        return( XTRUE );
    }
    return ( XFALSE );
}
#endif

/***********************************************************************
*                           ����Ҫ����Ŀռ�Ĵ�С
***********************************************************************/
XSTATIC  XU32  XOS_ArrayGetAllocationSize(XS32 elemSize, XS32 maxNumOfElements)
{
    return (sizeof(t_XOSARRAY) + 
        BIT_VECTOR_SIZE(maxNumOfElements) + 
        (maxNumOfElements * XOS_MAX((XU32)4, (XU32)elemSize)) );
}


/************************************************************************
* XOS_ArrayBuild
* ����: ����array �ĳ�ʼ��
* ����:         
*               buffer                                    - Ҫ��ʼ���Ŀռ�
*               elemSize                                  - ����Ԫ�صĴ�С
*               maxNumOfElements                          - ����Ԫ�ص�����
*               name                                      - ��������
* ���: ��
* ����: ��
************************************************************************/
XSTATIC  XVOID  XOS_ArrayBuild(
                                XCHAR*          buffer,
                                XS32            elemSize,
                                XS32            maxNumOfElements,
                                XCONST XCHAR*   name)
{
    t_XOSARRAY *ra;
    XU32 len;
    
    if ( XNULL == buffer)
    {
        return ;
    }
    
    /* ���ռ���0 */
    XOS_MemSet(buffer, 0, XOS_ArrayGetAllocationSize(elemSize, maxNumOfElements));
    
    /* ���ù���ͷ�ṹ����Ϣ */
    ra = (t_XOSARRAY *)buffer;
    
    ra->maxNumOfElements = maxNumOfElements;
    ra->arrayLocation    = (XCHAR*)ra + BIT_VECTOR_SIZE(maxNumOfElements) + sizeof(t_XOSARRAY);
    ra->curNumOfElements = 0;
    ra->sizeofElement    = elemSize;
    ra->maxUsage         = 0;
    ra->magicVal         = ARRAY_MAGIC_VALUE;
    
    if (XNULL != name)
    {
        len = (XU32)XOS_StrLen(name);
        XOS_StrNcpy(ra->name, name, XOS_MIN(len, ARRAY_NAME_SIZE - 1));
    }
    else
    {
        XOS_MemSet(ra->name, 0, ARRAY_NAME_SIZE);
    }
    
    XOS_ArrayInitClear((XOS_HARRAY)ra);   /* ��ʼ������Ϊ���� */
    
    /*return ra;*/
}


/***********************************************************************
*                           ��������ָ��λ��Ԫ�ص�ֵ
***********************************************************************/
XSTATIC  XVOID XOS_ArrayElemSet(  XOS_HARRAY raH,  XS32  location, XCONST  XVOID* src)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayElemSet , param invalid!\n");
        return ;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    if (src != XNULL)
    {
        XOS_MemCpy(ELEM_DATA(raH, location), (XVOID *)src, ((t_XOSARRAY *)raH)->sizeofElement);
    }
    else
    {
        XOS_MemSet(ELEM_DATA(raH, location), 0, (XU32)((t_XOSARRAY *)raH)->sizeofElement);
    }
}


/*-------------------------------------------------------------------------
                               ģ��ӿں���
-------------------------------------------------------------------------*/

/************************************************************************
* XOS_ArrayConstruct
* ����: ����һ������ ����
* ����: 
*       elemSize                         - Ԫ�صĴ�С(�ֽ�Ϊ��λ)
*       maxNumOfElements                 - ���Ԫ����
*       name                             - ��������
* ���: ��
* ����: �ɹ��������������
*       ʧ�ܷ���XNULL
************************************************************************/
XPUBLIC  XOS_HARRAY XOS_ArrayConstruct(  XS32 elemSize,  XS32  maxNumOfElements,  XCONST XCHAR*   name )
{
    t_XOSARRAY*   ra;
    XS32          size;
    XVOID*        ptr = XNULL;
    
    if (elemSize <= 0 || maxNumOfElements <= 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_ArrayConstruct()->Invalid param!\n");
        return XNULL;
    }
    
    /* ȷ��Ԫ�صĴ�С����Ϊ4 �ֽ�,���Ұ�4 �ֽڶ��� */
    if (elemSize < 4)
    {
        size = 4;
    }
    else
    {
        size = elemSize;
    }
    size = RV_ALIGN(size);
    
    /* ��������Ҫ�Ĵ洢�ռ� */
    if ((XS32)XOS_ArrayGetAllocationSize(size, maxNumOfElements) < 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_ArrayConstruct()->XOS_ArrayGetAllocationSize is too big!\n");
        return XNULL;    
    }
    if (XNULL ==  (ptr = (XVOID *)XOS_MemMalloc((XU32)FID_ROOT, XOS_ArrayGetAllocationSize(size, maxNumOfElements))))
    {
        return XNULL;
    }
    
    /* ��ʼ������ṹ������� */
    XOS_ArrayBuild((XCHAR *)ptr, size, maxNumOfElements, name);
    
    ra = (t_XOSARRAY*)ptr;
    return (XOS_HARRAY)ra;         /* ����������*/
}


/************************************************************************
* XOS_ArrayConstruct
* ����: ����һ������ ����(�ڴ��ʼ��֮ǰ����)
* ����: 
*       elemSize                         - Ԫ�صĴ�С(�ֽ�Ϊ��λ)
*       maxNumOfElements                 - ���Ԫ����
*       name                             - ��������
* ���: ��
* ����: �ɹ��������������
*               ʧ�ܷ���XNULL
************************************************************************/
XPUBLIC  XOS_HARRAY XOS_ArrayMemCst(  XS32 elemSize,  XS32  maxNumOfElements,  XCONST XCHAR*   name  )
{
    t_XOSARRAY*   ra;
    XS32          size;
    XVOID*        ptr = XNULL;
    
    if (elemSize <= 0 || maxNumOfElements <= 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_ArrayConstruct()->Invalid param!\n");
        return XNULL;
    }
    
    /* ȷ��Ԫ�صĴ�С����Ϊ4 �ֽ�,���Ұ�4 �ֽڶ��� */
    if (elemSize < 4)
    {
        size = 4;
    }
    else
    {
        size = elemSize;
    }
    size = RV_ALIGN(size);
    
    /* ��������Ҫ�Ĵ洢�ռ� */
    if ((XS32)XOS_ArrayGetAllocationSize(size, maxNumOfElements) < 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_ArrayConstruct()->XOS_ArrayGetAllocationSize is too big!\n");
        return XNULL;
    }
    if (XNULL ==  (ptr = (XVOID *)XOS_Malloc(XOS_ArrayGetAllocationSize(size, maxNumOfElements))))
    {
        return XNULL;
    }
    
    /* ��ʼ������ṹ������� */
    XOS_ArrayBuild((XCHAR *)ptr, size, maxNumOfElements, name);
    
    ra = (t_XOSARRAY*)ptr;
    return (XOS_HARRAY)ra;         /* ����������*/
}


/************************************************************************
* XOS_ArrayDestruct
* ���� : �ͷ����飬�ͷ�ռ�õ��ڴ�
* ���� : arrayH                      - Ҫ�ͷŵ����������
* ��� : ��
* ���� : ��
************************************************************************/
XPUBLIC  XVOID XOS_ArrayDestruct( XOS_HARRAY arrayH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayDestruct , param invalid!\n");
        return ;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    ra->magicVal = 0;/*���ƻ�������ͷ�*/
    if (XERROR == XOS_MemFree((XU32)FID_ROOT, (t_XOSARRAY*)arrayH))        /* �ͷ�����ռ�*/
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayDestruct , mem free fail!\n");
        return ;
    }
}


/************************************************************************
* XOS_ArrayMemDst
* ���� : �ͷ����飬�ͷ�ռ�õ��ڴ�(�ڴ��ʼ��֮ǰ��)
* ���� : arrayH                      - Ҫ�ͷŵ����������
* ��� : ��
* ���� : ��
************************************************************************/
XPUBLIC  XVOID XOS_ArrayMemDst( XOS_HARRAY arrayH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayDestruct , param invalid!\n");
        return ;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    XOS_Free(ra);        /* �ͷ�����ռ�*/
}


/************************************************************************
* XOS_ArrayInitClear
* ����: ���鴴��ʱ���ã���ʼ��һ���յ�Ԫ�ر�
* ����: raH                        - ���������
* ���: ��
* ����: ��
************************************************************************/
XPUBLIC  XVOID  XOS_ArrayInitClear(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    XS32 i;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayInitClear , param invalid!\n");
        return ;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    ra->curNumOfElements = 0;
    ra->firstNodeLocation = ra->lastNodeLocation = -1;
    ra->maxUsage = 0;
    
    /*  �������ʼ��Ϊ���� */
    if (ra->maxNumOfElements > 0)
    {
        ra->firstVacantElement = 0;
    }
    else
    {
        ra->firstVacantElement = -1;
    }
    ra->lastVacantElement = ra->maxNumOfElements-1;
    for (i=0; i<ra->maxNumOfElements; i++)
    {
        ((vacantNode *)ELEM_DATA(ra, i))->nextVacant = i+1;
    }
    
    ((vacantNode *)ELEM_DATA(ra, ra->maxNumOfElements-1))->nextVacant = XERROR;
    
    /* ��־λȫ����0���������Ԫ�ؾ����� */
    XOS_MemSet((XVOID*)BIT_VECTOR(ra), 0, BIT_VECTOR_SIZE(ra->maxNumOfElements)); /* make all free */
}


/************************************************************************
* XOS_ArraySetCompareFunc
* ����: ���ñȽϺ�����XOS_ArrayFind()ʹ��
* ����: 
*                arrayH             - ���������
*                func               - �ȽϺ���
* ���: ��
* ����: �ɹ�����XSUCC,ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32 XOS_ArraySetCompareFunc( XOS_HARRAY raH,  XOS_ArrayCompare func)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
#ifdef XOS_ARRAY_DEBUG    
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArraySetCompareFunc , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    ra->compare = func;         /*  ���ñȽϺ���*/
    return XSUCC;
}


/************************************************************************
* XOS_ArrayAdd
* ����: ����һ��Ԫ�ص�����,Ԫ�ر����䵽�����λ��
�ɷ���ֵ����
* ����:  
*        arrayH             - ���������
*        elem               - Ҫ�����Ԫ��
*                             �����ֵΪXNULL,��Ԫ�ص�ֵΪ��
* ���: ��
* ����: �ɹ�����Ԫ���������е��±�ֵ(�Ǹ�ֵ)
*               ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32   XOS_ArrayAdd( XOS_HARRAY arrayH, XOS_ArrayElement elem)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    XS32  vLocation;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayAdd , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /* ���ж��������Ƿ��п��еĿռ� */
    if ( (vLocation = ra->firstVacantElement) == XERROR)
    {
        XOS_PRINT(MD(FID_ROOT, PL_WARN), "XOS_ArrayAdd (%s): Array full (%d elements)\n", ra->name, ra->maxNumOfElements);
        return XERROR;
    }
    
    /* ��ʣ��ռ䣬����������ȡ�ÿռ䣬�ñ�־λ */
    ra->firstVacantElement = ((vacantNode *)ELEM_DATA(ra, vLocation))->nextVacant;
    if (ra->firstVacantElement == XERROR)
    {
        ra->lastVacantElement = XERROR;
    }
    setBit(BIT_VECTOR(ra), (XU32)vLocation, XTRUE); /* make it occupied */
    
    /* �ڿ���λ��������Ԫ�ص�ֵ */
    XOS_ArrayElemSet(arrayH, vLocation, elem);
    
    /* ���������ͳ������ */
    ra->curNumOfElements++;
    if (ra->curNumOfElements > ra->maxUsage)
    {
        ra->maxUsage = ra->curNumOfElements;
    }
    
    /* �������ӵ�Ԫ�ص�λ�� */
    return vLocation;
}


/************************************************************************
* XOS_ArrayAddExt
* ����: ������������һ��Ԫ�ص�λ��
* ���� : arrayH            - �������ľ��
* ��� : pOutElem          - ���뵽��Ԫ�ص�ָ�룬�������XNULL��û�����뵽.
* ���� : �ɹ����طǸ�ֵ��λ�ã�ʧ���򷵻�XERROR
************************************************************************/
XPUBLIC  XS32   XOS_ArrayAddExt( XOS_HARRAY arrayH,  XOS_ArrayElement *pOutElem)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    XS32 vLocation;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayAddExt , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /*  ���ж��������Ƿ��п��пռ� */
    if ( (vLocation = ra->firstVacantElement) == XERROR)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_WARN), "XOS_ArrayAddExt (%s): Array full (%d elements)\n", ra->name, ra->maxNumOfElements);
        if (pOutElem != XNULL)
        {
            *pOutElem = XNULL;
        }
        return XERROR;
    }
    
    /* ��������ȡ��Ԫ�ؿռ䣬�������ñ�־λ */
    ra->firstVacantElement = ((vacantNode *)ELEM_DATA(ra, vLocation))->nextVacant;
    if (ra->firstVacantElement == XERROR)
    {
        ra->lastVacantElement = XERROR;
    }
    setBit(BIT_VECTOR(ra), vLocation, XTRUE); /* make it occupied */
    /* Set statistical information */
    ra->curNumOfElements++;
    
    if (ra->curNumOfElements > ra->maxUsage)
    {
        ra->maxUsage = ra->curNumOfElements;
    }
    
    /*  ������뵽��Ԫ�صĵ�ַ*/
    if (pOutElem != XNULL)
    {
        *pOutElem = (XOS_ArrayElement)ELEM_DATA(arrayH, vLocation);
    }
    
    return vLocation;     /* Return the location */
}


/************************************************************************
* XOS_ArrayGetElemByPos
* ����: ��ȡָ��λ���ϵ�Ԫ��
* ���� : arrayH             - ���������
* ��� : location           - Ԫ���������е�λ��
* ���� : ����Ҫ��ȡ��Ԫ��,locationС���㷵��XNULL
************************************************************************/
XPUBLIC  XOS_ArrayElement XOS_ArrayGetElemByPos( XOS_HARRAY  arrayH,  XS32  location)
{
    t_XOSARRAY* ra = (t_XOSARRAY *)arrayH;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetElemByPos , param invalid!\n");
        return XNULL;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /*  ��λ���Ƿ���Ч*/
    if ((location < 0) || (unsigned)location >= (unsigned)(ra->maxNumOfElements))
    {
        return XNULL;
    }
    
    /*  ��λ���Ƿ���Ԫ��ռ��*/
    if (XOS_ArrayElemIsVacant(arrayH, location) == XTRUE)
    {
        return XNULL;
    }
    
    return (XOS_ArrayElement)ELEM_DATA(arrayH, location);/* ����Ԫ��*/
}


/************************************************************************
* XOS_ArrayDeleteByPos
* ����: ɾ��ָ��λ���ϵ�Ԫ��
* ���� : arrayH                       - ���������
* ��� : location                     - Ҫɾ����Ԫ�ص�λ��
* ���� : �ɹ�����XSUCC��ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32   XOS_ArrayDeleteByPos( XOS_HARRAY arrayH,  XS32 location)
{
    t_XOSARRAY *pArrayH = XNULL;
    pArrayH = (t_XOSARRAY *)arrayH;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayDelByPos , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /* λ���Ƿ���Ч,�Ƿ�洢��Ԫ��*/
    if (location < 0 || location >= pArrayH->maxNumOfElements)
    {
        return XERROR;
    }
    if (XOS_ArrayElemIsVacant(arrayH, location) == XTRUE)
    {
        return XERROR;
    }
    
    /* �������뵽������ */
    pArrayH->curNumOfElements--;
    
    /*DEBUG��,�ͷŵ�����Ԫ�ط�������ͷ;��DEBUG�·ŵ�����β*/
#ifdef XOS_ARRAYMEM_DEBUG
    ((vacantNode *)ELEM_DATA(arrayH, location))->nextVacant = pArrayH->firstVacantElement;
    pArrayH->firstVacantElement = location;
    if (pArrayH->lastVacantElement == XERROR)
    {
        pArrayH->lastVacantElement = location;
    }
#else
    ((vacantNode *)ELEM_DATA(arrayH, location))->nextVacant = XERROR;
    if (pArrayH->lastVacantElement != XERROR)
    {
        ((vacantNode *)ELEM_DATA(arrayH, pArrayH->lastVacantElement))->nextVacant = location;
    }
    else
    {
        pArrayH->firstVacantElement = location;
    }
    
    pArrayH->lastVacantElement = location;
#endif
    setBit(BIT_VECTOR(arrayH), (XU32)location, XFALSE); /* ��־λ��0 */
    
    return XSUCC;
}

#if 0
/************************************************************************
* XOS_ArrayElemIsVacant
* ����: ���ָ��λ���ϵ�Ԫ���Ƿ�Ϊ��
* ���� :  
*         arrayH                   - ���������
*         location                 - Ҫ����λ��
* ��� : ��
* ���� : �����Ϊ�շ���XTURE, ��Ϊ�շ���XFALSE
*                ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XBOOL XOS_ArrayElemIsVacant(  XOS_HARRAY arrayH,  XS32 location)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayElemIsVacant , param invalid!\n");
        return XTRUE;
    }
    if (!ra || location<0 || location>=ra->maxNumOfElements)
    {
        return XTRUE;
    }
    
    /*  ͨ����־λ�жϸ�λ�õ�Ԫ���Ƿ�Ϊ��*/
    return ((getBit((BIT_VECTOR(ra)), (location))) != 0) ? (XFALSE):(XTRUE);
}

#endif

/************************************************************************
* XOS_ArrayGetFirstPos
* ����: �������±�0��ʼ��ȡ��Ԫ�صĵ�һ��λ��
* ���� : arrayH                - ���������
* ��� : ��
* ���� : �ɹ����ص�һ��Ԫ�ص�λ��ֵ�����򷵻�ERROR
*************************************************************************/
XPUBLIC  XS32  XOS_ArrayGetFirstPos(   XOS_HARRAY arrayH )
{
    XS32   i = 0; 
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetFirstPos , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    for ( i=0; i < ( (t_XOSARRAY*)arrayH)->maxNumOfElements; i++ )
    {
        if ( !XOS_ArrayElemIsVacant(arrayH, i ) )/* Ԫ���Ƿ�Ϊ��*/
        {
            return ( i );
        }
    }
    return XERROR;
}


/************************************************************************
* XOS_ArrayGetNextPos
* ����: ��ȡ��ǰλ�ÿ�ʼ����һ����Ԫ�ص�λ��
* ���� : arrayH                - ���������
* ��� : location              - ��ʼ��λ��
* ���� : �ɹ�������һ��Ԫ�ص�λ�ã����򷵻�XERROR
************************************************************************/
XPUBLIC XS32  XOS_ArrayGetNextPos(  XOS_HARRAY  arrayH,  XS32  location)
{
    t_XOSARRAY* ra = (t_XOSARRAY *)arrayH;
    XS32 i;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetNextPos , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    if (location < 0)
    {
        i = 0;
    }
    else
    {
        i = location + 1;
    }
    if (i >= ra->maxNumOfElements)
    {
        return XERROR; /* out of bounds */
    }
    
    while ((i < ra->maxNumOfElements) && XOS_ArrayElemIsVacant(arrayH, i))
    {
        i++;
    }
    
    if (i < ra->maxNumOfElements)
    {
        return i;
    }
    else
    {
        return XERROR;
    }
}


/************************************************************************
* XOS_ArrayGetByPtr
* ����: ����Ԫ�ص�ֵ��ȡԪ���������е�λ��
* ���� : arrayH                      - ���������
* ��� : ptr                         - Ԫ�ص�ָ��
* ���� : �ɹ�����Ԫ�ص�λ��ֵ�����򷵻�XERROR
************************************************************************/
XPUBLIC  XS32  XOS_ArrayGetByPtr( XOS_HARRAY arrayH, XVOID *ptr)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    XS32  location;
    XS32  position;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )/* ����Ƿ���Ч*/
    {
    /* �˴�����xos_trace�����µݹ飬����ȡ��
    XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetByPtr , param invalid!\n");
        */
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    /*  Ԫ�صĵ�ַ�Ƿ��ڷ�Χ��*/
    if (((XCHAR *)ptr < (XCHAR *)(ra->arrayLocation)) ||
        ((XCHAR *)ptr >= (XCHAR *)(ra->arrayLocation) + ra->maxNumOfElements*(ra->sizeofElement)))
    {
        return XERROR;
    }
    
    /* ͨ����ַ����Ԫ�ص������±� */
    location = (XS32)((XCHAR *)ptr - ra->arrayLocation);
    
    /* Make sure the pointer is aligned properly */
    if (location % ra->sizeofElement != 0)
    {   /* alignment */
        return XERROR;
    }
    position = location/(ra->sizeofElement);
    
    return position;
}


/************************************************************************
* XOS_ArrayFind
* ����: ���ݲ���������������ϵ�Ԫ�ص�λ��
* ���� : arrayH                - ���������
* ��� : param                 - �ȽϺ����Ĳ���
* ���� : �ɹ�����Ԫ�ص�λ��ֵ�����򷵻�ERROR
*************************************************************************/
XPUBLIC  XS32  XOS_ArrayFind( XOS_HARRAY arrayH,  XVOID *param)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    XS32 i;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayFind , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    for (i=0; i < ra->maxNumOfElements; i++)
    {
        if (!XOS_ArrayElemIsVacant(arrayH, i))
        {
            if (ra->compare)
            {
                if (ra->compare(ELEM_DATA(ra, i), param))   /*�û��ıȽϺ���*/
                {
                    return i;
                }
            }
            else if (ELEM_DATA(ra, i) == param)      /* Ĭ�ϵ���ȫƥ��Ƚ� */
            {
                return i;
            }
        }
    }
    return XERROR;
}


/************************************************************************
* XOS_ArrayElemCompare
* ����: ʹ��ָ���ȽϺ���������ĵ�һ��λ����
���ҷ��ϲ�����Ԫ��
* ���� : 
*        raH                      - �������
*        compare                  - �ȽϺ���
*        param                    - �ȽϺ����Ĳ���
* ��� :  ��
* ���� : �ɹ����ط��ϲ����Ľڵ�λ�ã����򷵻�XERROR
************************************************************************/
XPUBLIC  XS32   XOS_ArrayElemCompare( XOS_HARRAY raH,   XOS_ArrayCompare compare,  XVOID *param)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    XS32 i;
    
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayElemCompare , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    for (i=0; i < ra->maxNumOfElements; i++)
    {
        if (!XOS_ArrayElemIsVacant(raH, i)) 
        {
            if (compare)
            {
                if(compare(ELEM_DATA(ra, i), param)) /* �û��ıȽϺ���*/
                {
                    return i;
                }
            }
            else if (ELEM_DATA(ra, i) == param)    /*Ĭ�ϵıȽ�*/
            {
                return i;
            }
        }
    }
    return XERROR;
}


/************************************************************************
* xos_ArrayGetName
* ����: ��ȡ��������
* ���� : arrayH                    - ���������
* ��� : ��
* ���� : ���������ַ���ָ��
************************************************************************/
XPUBLIC  XCONST XCHAR *XOS_ArrayGetName( XOS_HARRAY arrayH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)arrayH;
    
#ifdef XOS_ARRAY_DEBUG    
    if ( XTRUE  != XOS_ArrayHandleIsValid( arrayH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetName , param invalid!\n");
        return XNULL;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return (XCONST XCHAR *)(ra->name);
}


/************************************************************************
* XOS_ArrayGetCurElemNum
* ����: ��ȡ���鵱ǰ��Ч��Ԫ�ص�����
* ���� : raH                      - ����ľ��
* ��� : ��
* ���� : �ɹ�����Ԫ�ص�����������XERROR
************************************************************************/
XPUBLIC  XS32  XOS_ArrayGetCurElemNum(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
#ifdef XOS_ARRAY_DEBUG  
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetCurElemNum , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ra->curNumOfElements;
}


/************************************************************************
*                    ��ȡarray �ĵ�ǰ����Ԫ������
************************************************************************/
XPUBLIC XS32  XOS_ArrayGetVacantElemNum(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetVacantElemNum , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->maxNumOfElements - ((t_XOSARRAY *)raH)->curNumOfElements;
}


/************************************************************************
*                    ��ȡarray �ɴ��Ԫ�ص��������
************************************************************************/
XPUBLIC XS32  XOS_ArrayGetMaxElemNum(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetMaxElemNum , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->maxNumOfElements;
}


/************************************************************************
*                        ��ȡarray �����ʹ����
************************************************************************/
XPUBLIC XS32   XOS_ArrayGetMaxUsageElemNum(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetMaxUsageElemNum , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->maxUsage;
}


/************************************************************************
*                         ��ȡԪ�صĴ�С
************************************************************************/
XPUBLIC XS32  XOS_ArrayGetElemSize(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetElemSize , param invalid!\n");
        return XERROR;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->sizeofElement;
}


/************************************************************************
*                          ��ȡ�ȽϺ���
************************************************************************/
XPUBLIC XOS_ArrayCompare  XOS_ArrayGetCompareFunc(XOS_HARRAY raH)
{
#ifdef XOS_ARRAY_DEBUG
    if ( XTRUE  != XOS_ArrayHandleIsValid( raH ) )
    {
        XOS_PRINT(MD(FID_ROOT, PL_EXP), "XOS_ArrayGetCompareFunc , param invalid!\n");
        return XNULL;
    }
#endif /*XOS_ARRAY_DEBUG*/
    
    return ((t_XOSARRAY *)raH)->compare;
}

/*
Desc:wzy add the follow funs to implement the list 
*/

/************************************************************************
*                            ��ȡ��һ���ڵ�
************************************************************************/
XPUBLIC XS32 XOS_ArrayGetFirstNode(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
#ifdef XOS_ARRAY_DEBUG 
    if (!XOS_ArrayHandleIsValid(raH)) return XERROR;
#endif /*XOS_ARRAY_DEBUG*/
    
    return ra->firstNodeLocation;
}


/************************************************************************
*                           ��ȡ���һ���ڵ�
************************************************************************/
XPUBLIC XS32 XOS_ArrayGetLastNode(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
#ifdef XOS_ARRAY_DEBUG
    if (!XOS_ArrayHandleIsValid(raH)) return XERROR;
#endif /*XOS_ARRAY_DEBUG*/
    
    return ra->lastNodeLocation;
}


/************************************************************************
*                            ���õ�һ���ڵ�
************************************************************************/

XPUBLIC XS32 XOS_ArraySetFirstNode(XOS_HARRAY raH, XS32 location)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
#ifdef XOS_ARRAY_DEBUG
    if (!XOS_ArrayHandleIsValid(raH)) return XERROR;
#endif /*XOS_ARRAY_DEBUG*/
    
    ra->firstNodeLocation = location;
    return location;
}


/************************************************************************
*                         �������һ���ڵ�
************************************************************************/
XPUBLIC XS32 XOS_ArraySetLastNode(XOS_HARRAY raH, XS32 location)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    
    if (!XOS_ArrayHandleIsValid(raH)) return XERROR;
    
    ra->lastNodeLocation = location;
    return location;
}


/************************************************************************
*                            ��ȡ��һ���ڵ��׵�ַ
************************************************************************/
XPUBLIC XCHAR* XOS_ArrayGetHeadPtr(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    XS32 maxElements;
    
#ifdef XOS_ARRAY_DEBUG
    if (!XOS_ArrayHandleIsValid(raH)) return XNULLP;
#endif /*XOS_ARRAY_DEBUG*/
    
    maxElements = ((t_XOSARRAY*)ra)->maxNumOfElements;
    return (XCHAR*)ra + sizeof(t_XOSARRAY) + BIT_VECTOR_SIZE(maxElements);
}


/************************************************************************
*                        ��ȡ���һ���ڵ���׵�ַ
************************************************************************/
XPUBLIC XCHAR* XOS_ArrayGetTailPtr(XOS_HARRAY raH)
{
    t_XOSARRAY *ra = (t_XOSARRAY *)raH;
    XS32 maxElements;
    XS32 elemSize;
    
#ifdef XOS_ARRAY_DEBUG  
    if (!XOS_ArrayHandleIsValid(raH)) return XNULLP;
#endif /*XOS_ARRAY_DEBUG*/
    
    maxElements = ((t_XOSARRAY*)ra)->maxNumOfElements;
    elemSize = ra->sizeofElement;
    
    return  (XCHAR*)ra + sizeof(t_XOSARRAY) 
            + BIT_VECTOR_SIZE(maxElements) + (maxElements * XOS_MAX((XU32)4, (XU32)elemSize));
    
}


#ifdef  __cplusplus
}
#endif  /*  __cplusplus */


