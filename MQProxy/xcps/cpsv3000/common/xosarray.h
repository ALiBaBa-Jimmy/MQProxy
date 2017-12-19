/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosarray.h
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
**   author          date              modification            
**   zengjiandong    2006.3.21       create  
**   zengjiandong    2006.3.23       revise
**************************************************************/
#ifndef _xosarray_H_
#define _xosarray_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xostype.h"

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
#define  ARRAY_MAGIC_VALUE        0x0aaaff77
#define  ARRAY_NAME_SIZE     32/*�������Ĵ�С*/
/*#define  XOS_ARRAY_ERROR     ( -1 )*/

/* �����±�Ϊi ��Ԫ�صĵ�ַ */
#define ELEM_DATA(ra, i) \
    ((XCHAR *) (((t_XOSARRAY*)(ra))->arrayLocation + (i)*(((t_XOSARRAY*)(ra))->sizeofElement)))

/*  ������  */
XOS_DECLARE_HANDLE(XOS_HARRAY);

/*�ĸ��ֽڶ���*/
#ifdef _ALPHA_
#define RV_ALIGN(numBytes) (((numBytes + 7) / 8) * 8)
#else
#define RV_ALIGN(numBytes) (((numBytes + 3) / 4) * 4)
#endif

#define BIT_VECTOR(ra)      ((XCHAR *)(ra) + sizeof(t_XOSARRAY) )


/* ȡ��ָ����־λ��ֵ�����Ƿ����*/
#define getBit(ptr, bit) \
    ((XU32)(((XCHAR *)(ptr))[((XU32)(bit)) >> 3] & (0x80 >> ((XU32)(bit) & 7))))


/*-------------------------------------------------------------------------
                �ṹ��ö������
-------------------------------------------------------------------------*/

/* ���� Ԫ������ */
typedef XVOID  *XOS_ArrayElement;

/*�ȽϺ���ָ��*/
typedef XBOOL (*XOS_ArrayCompare)(XOS_ArrayElement element1, XVOID *param);

typedef struct {
    XCHAR            name[ARRAY_NAME_SIZE];   /* ���������*/
    XS32             magicVal;                      /* �����ж������Ƿ���Ч*/
    XCHAR*           arrayLocation;              /* �������ʼ��ַ*/
    XS32             firstVacantElement;     /* ��һ�����е�λ��*/
    XS32             lastVacantElement;      /* ���һ�����е�λ��*/
    XS32             maxNumOfElements;   /* ����Ԫ�ص��������*/
    XS32             curNumOfElements;     /* ����Ԫ�صĵ�ǰ����*/
    XS32             sizeofElement;             /* Ԫ�صĴ�С*/
    XS32             maxUsage;                   /* ������������ʹ����*/
    XOS_ArrayCompare compare;       /* �ȽϺ���*/
    
    XS32             firstNodeLocation;       
    XS32             lastNodeLocation;
} t_XOSARRAY;

/*-------------------------------------------------------------------------
API ����
-------------------------------------------------------------------------*/


/****************************************************************
* XOS_ArrayConstruct
* ����: ����һ������ ����
* ����: elemSize                        - Ԫ�صĴ�С(�ֽ�Ϊ��λ)
*       maxNumOfElements                - ���Ԫ����
*       name                            -��������
* ���: ��
* ����: �ɹ��������������
*               ʧ�ܷ���XNULL
***************************************************************/
XPUBLIC  XOS_HARRAY XOS_ArrayConstruct(
                                       XS32 elemSize, 
                                       XS32  maxNumOfElements,
                                       XCONST XCHAR*   name  );


/************************************************************************
* XOS_ArrayConstruct
* ����: ����һ������ ����(�ڴ��ʼ��֮ǰ����)
* ����: elemSize                        - Ԫ�صĴ�С(�ֽ�Ϊ��λ)
*       maxNumOfElements                - ���Ԫ����
        name                            -��������
* ���: ��
* ����: �ɹ��������������
*               ʧ�ܷ���XNULL
************************************************************************/
XPUBLIC  XOS_HARRAY XOS_ArrayMemCst(
                                    XS32 elemSize,  
                                    XS32  maxNumOfElements,  
                                    XCONST XCHAR*   name  );

/****************************************************************
* XOS_ArrayDestruct
* ����  : �ͷ����飬�ͷ�ռ�õ��ڴ�
* ����  : arrayH               - Ҫ�ͷŵ����������
* ��� : ��
* ���� : ��
***************************************************************/
XPUBLIC  XVOID XOS_ArrayDestruct( XOS_HARRAY arrayH );

/************************************************************************
* XOS_ArrayMemDst
* ���� : �ͷ����飬�ͷ�ռ�õ��ڴ�(�ڴ��ʼ��֮ǰ��)
* ���� : arrayH                      - Ҫ�ͷŵ����������
* ��� : ��
* ���� : ��
************************************************************************/
XPUBLIC  XVOID XOS_ArrayMemDst( XOS_HARRAY arrayH);

/****************************************************************
* XOS_ArrayInitClear
* ����: ���鴴��ʱ���ã���ʼ��һ���յ�Ԫ�ر�
* ����: arrayH                       - ���������
* ���: ��
* ����: ��
***************************************************************/
XPUBLIC  XVOID  XOS_ArrayInitClear( XOS_HARRAY arrayH );


/****************************************************************
* XOS_ArrayAdd
* ����: ����һ��Ԫ�ص�����,Ԫ�ر����䵽�����λ���ɷ���ֵ����
* ����:  arrayH                   - ���������
*        elem                     - Ҫ�����Ԫ��
*                                   �����ֵΪXNULL,��Ԫ�ص�ֵΪ��
* ���: ��
* ����: �ɹ�����Ԫ���������е��±�ֵ(�Ǹ�ֵ)
*              ʧ�ܷ���XERROR
***************************************************************/
XPUBLIC  XS32   XOS_ArrayAdd( XOS_HARRAY arrayH, XOS_ArrayElement elem );


/****************************************************************
* XOS_ArrayAddExt
* ����: ������������һ��Ԫ�ص�λ��
* ���� : arrayH      - �������ľ��
* ��� : pOutElem     - ���뵽��Ԫ�ص�ָ�룬�������XNULL��û�����뵽.
* ���� : �ɹ����طǸ�λ��ֵ, ʧ�ܷ���XERROR 
***************************************************************/
XPUBLIC  XS32  XOS_ArrayAddExt(XOS_HARRAY arrayH, XOS_ArrayElement *pOutElem);


/****************************************************************
* XOS_ArrayGetElemByPos 
* ����: ��ȡָ��λ���ϵ�Ԫ��
* ���� : arrayH                - ���������
* ��� : location              - Ԫ���������е�λ��
* ���� : ����Ҫ��ȡ��Ԫ��,locationС���㷵��XNULL
***************************************************************/
XPUBLIC  XOS_ArrayElement XOS_ArrayGetElemByPos(XOS_HARRAY arrayH, XS32  location);


/****************************************************************
*XOS_ArrayDeleteByPos
* ����: ɾ��ָ��λ���ϵ�Ԫ��
* ���� : arrayH                  - ���������
* ��� : location                - Ҫɾ����Ԫ�ص�λ��
* ���� : �ɹ�����XSUCC��ʧ�ܷ���XERROR
***************************************************************/
XPUBLIC  XS32   XOS_ArrayDeleteByPos(XOS_HARRAY arrayH,XS32 location);

#if 0
/****************************************************************
* XOS_ArrayElemIsVacant
* ����: ���ָ��λ���ϵ�Ԫ���Ƿ�Ϊ��
* ����  : arrayH              - ���������
*         location            - Ҫ����λ��
* ��� : ��
* ���� : �����Ϊ�շ���XTURE, ��Ϊ�շ���XFALSE
*               ʧ�ܷ���XERROR
***************************************************************/
XPUBLIC  XBOOL   XOS_ArrayElemIsVacant( XOS_HARRAY arrayH, XS32 location);
#endif
#define XOS_ArrayElemIsVacant(ra, loc)  (((loc)<0 ||(loc)>=((t_XOSARRAY*)(ra))->maxNumOfElements)? XTRUE: (((getBit((BIT_VECTOR(ra)), (loc)))!= 0) ? (XFALSE):(XTRUE)))
/****************************************************************
* XOS_ArrayGetByPtr
* ����: ����Ԫ�ص�ֵ��ȡԪ���������е�λ��
* ���� : arrayH      - ���������
* ��� : ptr         - Ԫ�ص�ֵ
* ���� : �ɹ�����Ԫ�ص�λ��ֵ�����򷵻�XERROR
***************************************************************/
XPUBLIC  XS32  XOS_ArrayGetByPtr(XOS_HARRAY arrayH, XVOID *ptr);


/****************************************************************
* XOS_ArrayGetNextPos
* ����: ��ȡ��ǰλ�ÿ�ʼ����һ����Ԫ�ص�λ��
* ���� : arrayH               - ���������
* ��� : location             - ��ʼ��λ��
* ���� : �ɹ�������һ��Ԫ�ص�λ�ã����򷵻�XERROR
***************************************************************/
XPUBLIC  XS32  XOS_ArrayGetNextPos( XOS_HARRAY  arrayH, XS32  location);



/****************************************************************
* XOS_ArraySetCompareFunc
* ����: ���ñȽϺ�����raFind()ʹ��
* ����  : arrayH           - ���������
*         func             - �ȽϺ���
* ��� : ��
* ���� : �ɹ�����XSUCC,ʧ�ܷ���XERROR
***************************************************************/
XPUBLIC  XS32 XOS_ArraySetCompareFunc( XOS_HARRAY arrayH, XOS_ArrayCompare func);


/****************************************************************
* XOS_ArrayFind
* ����: ���ݲ���������������ϵ�Ԫ�ص�λ��
* ���� : arrayH           - ���������
* ��� : param            - �ȽϺ����Ĳ���
* ���� : �ɹ�����Ԫ�ص�λ��ֵ�����򷵻�XERROR
***************************************************************/
XPUBLIC  XS32  XOS_ArrayFind(XOS_HARRAY arrayH, XVOID *param);


/****************************************************************
* XOS_ArrayGetFirstPos
* ����: �������±�0��ʼ��ȡ��Ԫ�صĵ�һ��λ��
* ���� : arrayH         - ���������
* ��� : ��
* ���� : �ɹ����ص�һ��Ԫ�ص�λ��ֵ�����򷵻�XERROR
***************************************************************/
XPUBLIC  XS32  XOS_ArrayGetFirstPos( XOS_HARRAY arrayH );


/****************************************************************
* XOS_ArrayElemCompare
* ����: ʹ��ָ���ȽϺ���������ĵ�һ��λ������ҷ��ϲ�����Ԫ��
* ����  : arrayH             - �������
*         compare            -�ȽϺ���
*         param              -�ȽϺ����Ĳ���
* ��� :  ��
* ���� : �ɹ����ط��ϲ����Ľڵ�λ�ã�����XERROR
***************************************************************/
XPUBLIC  XS32   XOS_ArrayElemCompare(XOS_HARRAY arrayH, XOS_ArrayCompare compare, XVOID *param);


/****************************************************************
* XOS_ArrayGetCompareFunc
* ����: ��ȡ�����Ԫ�رȽϺ���
* ���� : arrayH                        - ���������
* ��� : ��
* ���� : �ɹ����رȽϺ�����ʧ�ܷ���XNULL
***************************************************************/
XPUBLIC  XOS_ArrayCompare  XOS_ArrayGetCompareFunc(XOS_HARRAY arrayH);


/****************************************************************
* XOS_ArrayGetCurElemNum
* ����: ��ȡ���鵱ǰ��Ч��Ԫ�ص�����
* ���� : arrayH                      - ����ľ��
* ��� :  ��
* ���� : �ɹ�����Ԫ�ص�����������XERROR 
****************************************************************/
XPUBLIC  XS32  XOS_ArrayGetCurElemNum(XOS_HARRAY arrayH);


/****************************************************************
* XOS_ArrayGetName
* ����: ��ȡ��������
* ����: arrayH                      - ���������
* ���: ��
* ����: ���������ַ���ָ��
***************************************************************/
XPUBLIC  XCONST XCHAR* XOS_ArrayGetName(XOS_HARRAY arrayH);


/*wzy add the funs to inplement the list */
/************************************************************************
*                               ��ȡarray �ɴ��Ԫ�ص��������
************************************************************************/
XS32  XOS_ArrayGetMaxElemNum(XOS_HARRAY raH);


/************************************************************************
*                                    ��ȡarray �����ʹ����
************************************************************************/
XS32   XOS_ArrayGetMaxUsageElemNum(XOS_HARRAY raH);


/************************************************************************
*                                     ��ȡԪ�صĴ�С
************************************************************************/
XS32  XOS_ArrayGetElemSize(XOS_HARRAY raH);


/************************************************************************
*                                         ��ȡ��һ���ڵ�
************************************************************************/
XS32 XOS_ArrayGetFirstNode(XOS_HARRAY raH);


/************************************************************************
*                                         ��ȡ���һ���ڵ�
************************************************************************/
XS32 XOS_ArrayGetLastNode(XOS_HARRAY raH);


/************************************************************************
*                                         ���õ�һ���ڵ�
************************************************************************/

XS32  XOS_ArraySetFirstNode(XOS_HARRAY raH, XS32 location);


/************************************************************************
*                                        �������һ���ڵ�
************************************************************************/
XS32 XOS_ArraySetLastNode(XOS_HARRAY raH, XS32 location);


/*һ�������ӿڸ��ڴ������*/
/************************************************************************
*                                         ��ȡ��һ���ڵ��׵�ַ
************************************************************************/

XCHAR* XOS_ArrayGetHeadPtr(XOS_HARRAY raH);

/************************************************************************
*                                       ��ȡ���һ���ڵ���׵�ַ
************************************************************************/
XCHAR* XOS_ArrayGetTailPtr(XOS_HARRAY raH);

XBOOL XOS_ArrayIsUesd(XOS_HARRAY raH, void* ptr);

#define XOS_ArrayHandleIsValid(ra)  ((ra)? ((((t_XOSARRAY*)(ra))->magicVal == ARRAY_MAGIC_VALUE)?XTRUE:XFALSE):XFALSE)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xoshash.h*/

