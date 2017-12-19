/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xoshash.h
**
**  description:  
**
**  author: zengjiandong
**
**  date:   2006.3.8
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author                  date                 modification            
**   zengjiandong        2006.3.8              create  
**************************************************************/
#ifndef _xoshash_H_
#define _xoshash_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xosarray.h"

/*-------------------------------------------------------------------------
                �궨��
-------------------------------------------------------------------------*/
/*#define XOS_HASH_ERROR     (-1)*/
#define HASH_MAGIC_VALUE    0x0aaaff77
XOS_DECLARE_HANDLE(XOS_HHASH);             /*  hash������  */


/*-------------------------------------------------------------------------
                              �ṹ��ö������
-------------------------------------------------------------------------*/
/************************************************************************
* XOS_HashListElement struct
* ����ṹ��������û���Ԫ�أ�ͬʱ�ṩһ����ǰ
* ������ָ�룬������ͬһ��Keyֵ��Ԫ�ع���
* һ������
* 
* next           - ָ�������ͬkeyֵ����һ��Ԫ��
* prev           - ָ�������ͬkeyֵ����һ��Ԫ��
* entryNum    - keyֵ��hash�������ڵ�
************************************************************************/
typedef struct hashListElement_tag XOS_HashListElement;
struct hashListElement_tag
{
    XOS_HashListElement*    next;
    XOS_HashListElement*    prev;
    XS32                    entryNum;
};


/*-------------------------------------------------------------------------
API ����
-------------------------------------------------------------------------*/

/************************************************************************
* ���ڱȽ�hashԪ���е�ĳ�������Ƿ���ͬ
* hHash-->hash���
* ElemPtr-->Ԫ�ص�ָ��
* �Ƚ���ͬ����XSUCC,����XERROR
************************************************************************/
typedef   XU32  (*HashElemCmp)(  XOS_HHASH   hHash,
                                 XVOID*          ElemPtr );


/************************************************************************
* Hash�����Ķ���
* ���룺param                 - Ԫ��
*                  paramSize           - Ԫ�صĴ�С(��λ�ֽ�)
*                  hashSize              -  hash��Ĵ�С
* ���� : hash ���
************************************************************************/
typedef XU32 (*HashFunc) (
                          XVOID*   param, 
                          XS32     paramSize, 
                          XS32     hashSize);


/************************************************************************
* key �ȽϺ����Ķ���
* ����  : key1, key2              - ���ڱȽϵ�����Keyֵ
*                keySize                   - keyֵ�Ĵ�С
* ���� : ��ȷ���XTURE,���򷵻�XFALSE
************************************************************************/
typedef XBOOL (*HashKeyCmp) (
                             XVOID* key1, 
                             XVOID* key2, 
                             XU32 keySize );


/************************************************************************
* XOS_HASHFunc
* ����: �����ÿ��hashԪ��ͨ�õĺ���
* ����: hHash   - ������
*       elem    - Ԫ��
*       param   - ����
* ���:
* ����: ָ��һ�����������´ε���XOS_HASHFuncʹ��
************************************************************************/
typedef XVOID* (*XOS_HashFunc)(XOS_HHASH hHash, XVOID* elem, XVOID *param);


/************************************************************************
* t_XOSHASH structure
* ����hash����������Ϣ
* hash                   - Ҫʹ�õ�hash����
* compare             - �ȽϺ���
* numKeys            - hash��Ĵ�С(����key����������)
* userKeySize        - key������
* userElemSize       - Ԫ�صĴ�С
* alignedKeySize    - keyֵ�Ĵ�С(�ֽ�)
* numElements       - Ԫ�ص�����
* curSize                - ��ǰԪ�ص�����
* keys                    - �����ָ�룬������鱣��ָ��Ԫ�ص�ָ��
*                    
* elements              - ���飬���ڴ��Ԫ��
************************************************************************/
typedef struct
{
    HashFunc                    hash;
    XU32                        magicVal;
    HashKeyCmp                  compare;
    XU32                        numKeys;
    XU32                        userKeySize;
    XU32                        userElemSize;
    XU32                        alignedKeySize;
    XU32                        numElements;
    XU32                        curSize;
    XOS_ArrayElement*           keys;
    XOS_HARRAY                  elements;
} t_XOSHASH;

/************************************************************************
* ��������XOS_HashConstruct
* ���� : ����һ��hash����,ʹ��Ĭ�ϵ�hash�����ͱȽϺ���
* ���� : numOfKeys           - hash��Ĵ�С,ӦԶ���ڱ��е�Ԫ����
*        numOfElems          - hash����Ԫ�ص�����
*        keySize             - keys�Ĵ�С
*        elemSize            - Ԫ�صĴ�С
* ��� : ��
* ���� : �ɹ�����hash����,ʧ�ܷ���XNULL
* ˵�� : hash������hash�ȽϺ�������Ĭ�ϵĺ�����
*               ��Ҫ�����Լ��ĺ���ʱ������XOS_HashSetHashFunc()
*               ��XOS_HashSetCompareFunc()����
************************************************************************/
XPUBLIC  XOS_HHASH    XOS_HashConstruct( XU32             numOfKeys,
                                         XU32             numOfElems,
                                         XU32             keySize,
                                         XU32             elemSize,
                                         XCONST XCHAR*    name);

/************************************************************************
* XOS_HashMemCst
* ����: ����һ��hash����,(��û���ڴ��ʼ����ʱ����)
*                 
* ����  :        numOfKeys           - hash��Ĵ�С.ӦԶ���ڱ��е�Ԫ����
*                numOfElems          - hash����Ԫ�ص�����
*                hashFunc            - hash����
*                compareFunc         - keysֵ�ȽϺ���
*                keySize             - keys�Ĵ�С
*                elemSize            - Ԫ�صĴ�С
* ��� : ��
* ���� : �ɹ�����hash��������ʧ�ܷ���XNULL
************************************************************************/
XPUBLIC  XOS_HHASH    XOS_HashMemCst(
                                     XU32             numOfKeys,
                                     XU32             numOfElems,
                                     XU32             keySize,
                                     XU32             elemSize ,
                                     XCONST XCHAR*    name);


/************************************************************************
* ��������XOS_HashDestruct
* ���� : ɾ��hash�����ͷ��ڴ�
* ���� : hHash                   - HASH ������
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashDestruct(XOS_HHASH hHash);


/************************************************************************
* XOS_HashMemDst
* ����: ɾ��hash�����ͷ��ڴ� (�ڴ��ʼ��֮ǰ������)
* ����: hHash                     - HASH ������
* ���: ��
* ����: �ɹ�����XSUCC��ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashMemDst(   XOS_HHASH  hHash);


/************************************************************************
* ��������XOS_HashClear
* ���� : ���hash���մ���ʱ���Ǹ�״̬
* ���� : hHash                         - HASH������
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashClear(XOS_HHASH hHash);


/************************************************************************
* XOS_HashHandleIsValid
* ����: �ж�hash����Ƿ���Ч
* ����: hHash                           - hash���
* ���: ��
* ����: ��Ч����XTRUE����Ч����XFALSE
************************************************************************/
#define XOS_HashHandleIsValid(hash)  ((hash)? ((((t_XOSHASH*)(hash))->magicVal == HASH_MAGIC_VALUE)?XTRUE:XFALSE):XFALSE)
#if 0
XBOOL    XOS_HashHandleIsValid(    XOS_HHASH hHash  );
#endif


/************************************************************************
* ��������XOS_HashElemDel
* ���� : ��hash����ɾ��һ��Ԫ��
* ���� : hHash          - HASH������
*               pLocation      - hashԪ�ص�λ��ָ��,��add����ʱ����
* ��� : ��
* ���� : �ɹ����� XSUCC, ʧ�ܷ��� XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashElemDel(XOS_HHASH   hHash,  XVOID* pLocation);


/************************************************************************
* ��������XOS_HashDelByElem
* ����: ��hash����ɾ��һ��ָ��λ���ϵ�Ԫ��
* ����: hHash          - HASH������
*              pElem          - hashԪ�ص���ʵλ��ָ��
* ��� : ��
* ���� : �ɹ����� XSUCC, ʧ�ܷ��� XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashDelByElem( XOS_HHASH   hHash,  XVOID* pElem );


/************************************************************************
* ��������XOS_HashFindNext
* ����: �������ϴε���XOS_HashElemFind()����ͬ��keyֵ
����һ��Ԫ��
* ����  : hHash             - HASH������
*                pKey               - keyֵ
*                pLocation         - �ϴ���HashFindNext() ��HashFind()���ҵ���Ԫ��
* ��� : ��
* ���� : �ɹ������ҵ���Ԫ�ص�ָ�룬����XNULL
************************************************************************/
XPUBLIC  XVOID*    XOS_HashElemFindNext(
                                        XOS_HHASH hHash, 
                                        XVOID * pKey, 
                                        XVOID * pLocation);


/************************************************************************
* ��������XOS_HashGetElem
* ���� : ͨ��λ����Ϣȡ��Ԫ��
* ���� : hHash       - HASH������
*        pLocation   - hashԪ�ص�λ��ָ��,��add����ʱ����
* ��� : ��
* ���� : �ɹ�����Ԫ�ص��׵�ַ,ʧ�ܷ���XNULL
************************************************************************/
XPUBLIC  XVOID*   XOS_HashGetElem(XOS_HHASH  hHash, XVOID *pLocation );


/************************************************************************
* ��������XOS_HashSetElem
* ���� : ��λ����ϢΪlocation��Ԫ�����ó�pElem
* ���� : hHash        - HASH������
*        pLocation    - hashԪ�ص�λ��ָ��,��add����ʱ����
*        pElem        - Ҫ����Ԫ�ص��׵�ַ
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32   XOS_HashSetElem(XOS_HHASH  hHash, XVOID *pLocation, XVOID* pElem);


/************************************************************************
* ��������XOS_HashGetKeyByElem
* ����: ȡ��λ����ϢΪlocation��Ԫ�صĹؼ���
* ����: hHash          - HASH������
*       pElem          - hashԪ�ص���ʵλ�õ�ָ��
* ���: ��
* ����: �ɹ�����key���׵�ַ, ʧ�ܷ��ؿ�
************************************************************************/
XPUBLIC  XVOID*  XOS_HashGetKeyByElem(XOS_HHASH hHash, XVOID* pElem );


/************************************************************************
* ��������XOS_HashWalk
* ���� : ����hash��(ע��Ҫ��д��������)
* ���� : hHash                        - hash����
*        fFunc                        - ��������
*        pParam                       - ���������Ĳ���
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32 XOS_HashWalk(XOS_HHASH hHash, XOS_HashFunc fFunc ,XVOID* pParam);


/************************************************************************
* ��������XOS_ hashSetHashFunc
* ���� : ����hash����
* ���� : hHash                - HASH������
*        fFunc                - hash����
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32 XOS_HashSetHashFunc(XOS_HHASH hHash, HashFunc fFunc) ;


/************************************************************************
* ��������XOS_HashSetKeyCompareFunc
* ���� : ����hash�ؼ��ֱȽϺ���(�ú�����hash�����ʵ������,����ʵ��ģ����ѯ)
* ���� : hHash               - HASH������
*        fFunc               - hash�ؼ��ֱȽϺ���
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32 XOS_HashSetKeyCompareFunc(XOS_HHASH hHash,HashKeyCmp fFunc);


/************************************************************************
* XOS_HashEbscSagFind
* ����: ͨ��keyֵ����Ԫ�ص�λ��
* ����: hHash                        - HASH������
*       key                          - keyֵ
* ���: pIndex                       - hash  Ԫ�ص�������
* ����: �ɹ�����ָ���һ������keyֵ��Ԫ�ص�ָ��
*                ����ʧ�ܷ���XNULLָ��
************************************************************************/
XPUBLIC  XVOID*    XOS_HashEbscSagFind(XOS_HHASH  hHash, XVOID* pKey, XS32* pIndex );


/************************************************************************
* XOS_HashEbscSagAdd
* ����: ����һ���µ�Ԫ�ص�hash��
*              
* ����: hHash                          - HASH ������
*       pKey                           - keyֵ
*       pElement                       - ��ӵ�Ԫ��
*       allowSameKey                   - �Ƿ���������ͬ��key
* ���: pIndex                         - hashԪ�ش洢��������
* ����: �ɹ�����ָ��Ԫ����hash���е�λ��
*               ʧ�ܷ���XNULL
* ˵��: �˽ӿ���ʱֻ��ebsc��sagʹ��
************************************************************************/
XPUBLIC  XVOID*  XOS_HashEbscSagAdd(
                                    XOS_HHASH  hHash,
                                    XVOID*        pKey,
                                    XVOID*        pElement,
                                    XBOOL         allowSameKey,
                                    XS32*          pIndex );


/************************************************************************
* ��������XOS_HashFind
* ���� : ͨ��Keyֵ����Ԫ�ص�λ��
* ���� : hHash                 - HASH������
*        pKey                  - keyֵ
* ��� : ��
* ���� : �ɹ�����ָ���һ������keyֵ��Ԫ�ص�ָ��
*                ����ʧ�ܷ���XNULLָ��
************************************************************************/
#define XOS_HashElemFind(hHash, pKey)  XOS_HashEbscSagFind(hHash, pKey, 0)


/************************************************************************
* ��������XOS_HashElemAdd
* ����: ����һ���µ�Ԫ�ص�hash��
*              �������allowSameKey����ΪFALSE
*              �����ж�hash�����Ƿ��Ѿ�����һ��Ԫ��keyֵ
*              ����Ԫ�ص�keyֵ��ȣ���������Ԫ�ز������
* ����: hHash                       - HASH ������
*       pKey                        - keyֵ
*       pElement                    - ��ӵ�Ԫ��
*       allowSameKey                - �Ƿ���������ͬ��keyֵ
* ���: ��
* ����: �ɹ�������ӵ�Ԫ�����ڵ�λ����Ϣ, ʧ�ܷ���NULL
* ˵��: Ҫ���û������key��element������Ϊ�ղ����
************************************************************************/
#define XOS_HashElemAdd(hHash, pKey, pElement, allowSameKey)  \
XOS_HashEbscSagAdd(hHash, pKey, pElement, allowSameKey, 0)


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xoshash.h*/

