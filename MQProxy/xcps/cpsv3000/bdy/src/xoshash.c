/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xoshash.c
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
**   author                  date                modification            
**   zengjiandong        2006.3.8              create  
**   zengjiandong        2006.4.18             revise
**************************************************************/
#ifdef  __cplusplus
extern   "C"  {
#endif  /*  __cplusplus */


#include "xoshash.h"
#include "xoscfg.h"
#include "xostrace.h"
#include "xosmem.h"

/*-------------------------------------------------------------------------
              ģ���ڲ��궨��
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              ģ���ڲ����ݽṹ
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
              ģ���ڲ�����
-------------------------------------------------------------------------*/
#if 0
/************************************************************************
* XOS_HashHandleIsValid
* ����: �ж�hash����Ƿ���Ч
* ����: hHash                           - hash���
* ���: ��
* ����: ��Ч����XTRUE����Ч����XFALSE
************************************************************************/
XBOOL    XOS_HashHandleIsValid(    XOS_HHASH hHash  )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;

    if ( XNULL == hHash)
    {
        return( XFALSE );
    }

    if ( HASH_MAGIC_VALUE  == hash->magicVal )
    {
        return( XTRUE );
    }
    return ( XFALSE );
}
#endif


/************************************************************************
* XOS_HashHandleIsValid
* ����: �ж�n�Ƿ�Ϊ��������IntFirstPrime����
* ����: n       - ���жϵ�����
* ���: ��
* ����: ����������XTRUE������������XFALSE
************************************************************************/
XSTATIC  XBOOL  IsPrime(  XS32 n  )
{
    XS32  i;
    for (i = 2; i < n/i/*sqrt(n)*/+1; i++)
    {
        if (!(n%i)) 
        {
            return XFALSE;
        }
    }

    return XTRUE;
}


/************************************************************************
* IntFirstPrime
* ����: ���n ��ĵ�һ������
* ����: n              - ��ʼ������
* ���: ��
* ����: ��õ�����
************************************************************************/
XSTATIC  XS32  IntFirstPrime(  XS32  n  )
{
    XS32  i = n;

    if (0 == (i & 1))
    {
        i++;
    }
    while (!IsPrime(i))
    {
        i += 2;
    }    
    return i;
}


/************************************************************************
* HashFind
* ����: ͨ���ؼ��ֲ���Ԫ��
* ����:
* hHash                   - hash���
* key                       - �ؼ���
* ���: ��
* ����: �ɹ�����Ԫ�أ�ʧ���򷵻�XNULL
************************************************************************/
XSTATIC  XVOID*  HashFind(   XOS_HHASH hHash,  XVOID* key   )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XU32 keyValue;
    XOS_HashListElement* hashElem;

    /*����ľ�����߹ؼ���Ϊ�գ�ֱ�ӷ��ؿ�*/
    if (XNULL == hHash ||XNULL == key)
    {
        return XNULL;
    }

    /* ����hashͰ�� */
    keyValue = (XU32)(hash->hash(key, (XS32)(hash->userKeySize), (XS32)(hash->numKeys))) % (hash->numKeys);

    /* �ж�Ͱ���Ƿ����Ԫ�� */
    if (hash->keys[keyValue] == XNULL)
    {
        return XNULL;
    }

    /* ��Ϊ�գ���ʼ��Ͱ�в��� */
    hashElem = (XOS_HashListElement *)hash->keys[keyValue];

    /* ˳������Ƿ���key ֵ��ȵ�Ԫ�أ����򷵻� */
    while (XNULL != hashElem)
    {
        if (hash->compare(key, (XCHAR*)hashElem + sizeof(XOS_HashListElement), hash->userKeySize))
        {
            /* Found! */
            return hashElem;
        }
        hashElem = hashElem->next;
    }

    /* û���ҵ�������XNULL */
    return XNULL;
}


/************************************************************************
* HashFindNext
* ����: ��Ͱ�ĵ�ǰλ�ò�������ͬkey ֵ��Ԫ��
* ����:
* hHash                  - hash���
* key                    - �ؼ���
* location               - ��hash Ͱ�е�ǰλ��
* ���: ��
* ����: �ɹ�����Ԫ�أ�ʧ���򷵻�XNULL
************************************************************************/
XSTATIC  XVOID*   HashFindNext(
                               XOS_HHASH      hHash,
                               XVOID*         key,
                               XVOID*         location)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* hashElem;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    if (XNULL == location || XNULL == key)
    {
        return XNULL;
    }
    hashElem = (XOS_HashListElement *)location;

    /*�ӵ�ǰλ�����²���key ֵ��ȵ�Ԫ��*/
    hashElem = hashElem->next;
    while (XNULL != hashElem)
    {
        if (hash->compare(key, (XCHAR*)hashElem + sizeof(XOS_HashListElement), hash->userKeySize))
        {
            /* Found! */
            return hashElem;
        }
        hashElem = hashElem->next;
    }
    return XNULL;
}


/************************************************************************
* HashDoAll
* ����: hash ��������
* ����:
* hHash                 - hash ���
* func                  - ��������(  �����ȶ���)
* param                 - �ȽϵĲ���
* ���: ��
* ����: �ɹ�����XSUCC��ʧ���򷵻�XERROR
************************************************************************/
XSTATIC  XS32  HashDoAll(
                         XOS_HHASH           hHash,
                         XOS_HashFunc        func,
                         XVOID*              param)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XS32 cur;
    XCHAR  *ptr = XNULL;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == func)
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    cur = -1;

    /*��æ������˳�����Ԫ��*/
    while (0 <= (cur = XOS_ArrayGetNextPos(hash->elements, cur)))
    {
        ptr = (XCHAR*)XOS_ArrayGetElemByPos(hash->elements, cur);
        if ( XNULL != ptr )
        {
            param = func(hHash, ((XCHAR*)ptr + sizeof(XOS_HashListElement) + hash->alignedKeySize) , param);
        }
    }
    return XSUCC;
}


/************************************************************************
* Ĭ�ϵ�hash����
* ���� :
* param                - Ԫ��
* paramSize            - Ԫ�صĴ�С(  ��λ�ֽ�)
* hashSize             - hash��Ĵ�С
* ���� : hash ���
************************************************************************/
XSTATIC  XU32 XOS_Hashstr(  XVOID*  param,  XS32    paramSize,  XS32    hashSize)
{
    XS32 hash = 0;
    XU32 ulSeed = 31; //31��131��1313��13131
    XCHAR* ptr = (XCHAR *)param;

    if (0 >= paramSize)
    {
        /* string is null terminated */
        while (*ptr)
        {
            //hash = hash << 1 ^ *ptr++;
			hash = hash*ulSeed + *ptr++; /*���¹�ϣ�㷨*/
        }
    }
    else
    {
        while (0 < paramSize--  && *ptr)
        {
            //hash = hash << 1 ^ *ptr++;
            hash = hash*ulSeed + *ptr++; /*���¹�ϣ�㷨*/
        }
    }

    return (hash % hashSize);
}


/************************************************************************
* Ĭ�ϵıȽϺ���. Checks byte-by-byte����ȫƥ��Ƚ�.
* ����  :
* key1, key2            - ���ڱȽϵ�����Keyֵ
* keySize               - keyֵ�Ĵ�С
* ���� : ��ȷ���XTURE,���򷵻�XFALSE
************************************************************************/
XSTATIC  XBOOL  XOS_HashDefaultCompare( XVOID *key1,  XVOID* key2,  XU32 keySize)
{
    return (XBOOL)(XOS_MemCmp(key1, key2, keySize) == 0);
}


/*-------------------------------------------------------------------------
                 ģ��ӿں���
-------------------------------------------------------------------------*/
/************************************************************************
* XOS_HashConstruct
* ����: ����һ��hash����, ʹ��Ĭ�ϵ�hash�����ͱȽϺ���
* ����  :
*        numOfKeys           - hash��Ĵ�С.ӦԶ���ڱ��е�Ԫ����
*        numOfElems          - hash����Ԫ�ص�����
*        hashFunc               - hash����
*        compareFunc         - keysֵ�ȽϺ���
*        keySize                 - keys�Ĵ�С
*        elemSize               - Ԫ�صĴ�С
* ��� : ��
* ���� : �ɹ�����hash������ʧ�ܷ���XNULL
************************************************************************/
XPUBLIC  XOS_HHASH    XOS_HashConstruct(
                                       XU32             numOfKeys,
                                       XU32             numOfElems,
                                       XU32             keySize,
                                       XU32             elemSize ,
                                       XCONST XCHAR*     name)
{
    t_XOSHASH* phash;
    XS32      actualKeySize;
    actualKeySize = IntFirstPrime((XS32)numOfKeys); /* ��hash Ͱ������*/
    if ((XS32)numOfKeys <= 0 || (XS32)keySize <= 0 || (XS32)elemSize <= 0 || (XS32)numOfElems <= 0 || actualKeySize < 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error!\n");
        return XNULL;
    }
    /* ����hash Ͱ�ռ�*/
    if ((XS32)(actualKeySize * sizeof(XOS_ArrayElement)) < 0 || (XS32)(sizeof(t_XOSHASH) + actualKeySize * sizeof(XOS_ArrayElement)) < 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error, memory  keySize*numOfKeys  is too big!\n");
        return XNULL;
    }
    phash = (t_XOSHASH *)XOS_MemMalloc( (XU32)FID_ROOT, 
        (XS32)(sizeof(t_XOSHASH) + actualKeySize * sizeof(XOS_ArrayElement)));
    if( XNULL == phash )
    {
        return  XNULL;
    }

    /*HASH Ͱָ���ʼ��*/
    XOS_MemSet(((XU8*)phash + sizeof(t_XOSHASH)), 0, actualKeySize * sizeof(XOS_ArrayElement));

    /*hash ����ͷ�ṹ�ĳ�ʼ��*/
    phash->hash = XOS_Hashstr;
    phash->compare = XOS_HashDefaultCompare;
    phash->numKeys = (XU32)actualKeySize;
    phash->userKeySize = keySize;
    phash->userElemSize = elemSize;
    phash->alignedKeySize = (XU32)RV_ALIGN(keySize);
    phash->numElements = numOfElems;
    phash->curSize = 0;
    phash->keys = (XOS_ArrayElement*)((XU8*)phash + sizeof(t_XOSHASH));
    phash->magicVal = HASH_MAGIC_VALUE;

    /*����æ����*/
    if ((XS32)(elemSize + phash->alignedKeySize + sizeof(XOS_HashListElement)) < 0)
    {
        XOS_PRINT(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error, elemSize+alignedKeySize+sizeof(XOS_HashListElement) is too big!\n");
        XOS_MemFree( (XU32)FID_ROOT, phash);
        return XNULL;
    }
    phash->elements =
        XOS_ArrayConstruct((XS32)(elemSize + phash->alignedKeySize + sizeof(XOS_HashListElement)), 
        (XS32)numOfElems, name);
    if (XNULL == phash->elements)
    {
        XOS_MemFree( (XU32)FID_ROOT, phash);
        return XNULL;
    }

    return (XOS_HHASH)phash;
}


/************************************************************************
* XOS_HashMemCst
* ����: ����һ��hash����,(��û���ڴ��ʼ������)
* ����  :
*                numOfKeys           - hash��Ĵ�С.ӦԶ���ڱ��е�Ԫ����
*                numOfElems          - hash����Ԫ�ص�����
*                hashFunc               - hash����
*                compareFunc         - keysֵ�ȽϺ���
*                keySize                 - keys�Ĵ�С
*                elemSize               - Ԫ�صĴ�С
* ��� : ��
* ���� : �ɹ�����hash��������ʧ�ܷ���XNULL
************************************************************************/
XPUBLIC  XOS_HHASH    XOS_HashMemCst(
                                    XU32             numOfKeys,
                                    XU32             numOfElems,
                                    XU32             keySize,
                                    XU32             elemSize ,
                                    XCONST XCHAR*     name)
{
    t_XOSHASH* phash;
    XS32      actualKeySize;

    actualKeySize = IntFirstPrime((XS32)numOfKeys); /* ��hash Ͱ������*/
    if ((XS32)numOfKeys <= 0 || (XS32)keySize <= 0 || (XS32)elemSize <= 0 || (XS32)numOfElems <= 0 || actualKeySize < 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error!\n");
        return XNULL;
    }
    /* ����hash Ͱ�ռ�*/
    if ((XS32)(actualKeySize * sizeof(XOS_ArrayElement)) < 0 || (XS32)(sizeof(t_XOSHASH) + actualKeySize * sizeof(XOS_ArrayElement)) < 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error, memory  keySize*numOfKeys  is too big!\n");
        return XNULL;
    }
    phash = (t_XOSHASH *)XOS_Malloc((XS32)(sizeof(t_XOSHASH) + actualKeySize * sizeof(XOS_ArrayElement)));
    if( XNULL == phash )
    {
        return  XNULL;
    }

    /*HASH Ͱָ���ʼ��*/
    XOS_MemSet(((XU8*)phash + sizeof(t_XOSHASH)), 0, actualKeySize * sizeof(XOS_ArrayElement));

    /* hash ����ͷ�ṹ�ĳ�ʼ��*/
    phash->hash = XOS_Hashstr;
    phash->compare = XOS_HashDefaultCompare;
    phash->numKeys = (XU32)actualKeySize;
    phash->userKeySize = keySize;
    phash->userElemSize = elemSize;
    phash->alignedKeySize = (XU32)RV_ALIGN(keySize);
    phash->numElements = numOfElems;
    phash->curSize = 0;
    phash->keys = (XOS_ArrayElement*)((XU8*)phash + sizeof(t_XOSHASH));
    phash->magicVal = HASH_MAGIC_VALUE;

    /* ����æ����*/
    if ((XS32)(elemSize + phash->alignedKeySize + sizeof(XOS_HashListElement)) < 0)
    {
        XOS_CpsTrace(MD(FID_ROOT, PL_ERR), "XOS_HashConstruct()->Param Error, elemSize+alignedKeySize+sizeof(XOS_HashListElement) is too big!\n");
        XOS_Free( phash);
        return XNULL;    
    }
    phash->elements =
        XOS_ArrayMemCst((XS32)(elemSize + phash->alignedKeySize + sizeof(XOS_HashListElement)), 
        (XS32)numOfElems, name);
    if (XNULL == phash->elements)
    {
        XOS_Free( phash);
        return XNULL;
    }

    return (XOS_HHASH)phash;
}


/************************************************************************
* XOS_HashDestruct
* ����: ɾ��hash�����ͷ��ڴ�
* ����: hHash                     - HASH ������
* ���: ��
* ����: �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashDestruct(   XOS_HHASH  hHash)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    XOS_ArrayDestruct(hash->elements);
    XOS_MemFree( (XU32)FID_ROOT, hash);

    return XSUCC;
}


/************************************************************************
* XOS_HashMemDst
* ����: ɾ��hash�����ͷ��ڴ� (�ڴ��ʼ��֮ǰ������)
* ����: hHash                     - HASH ������
* ���: ��
* ����: �ɹ�����XSUCC��ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashMemDst(   XOS_HHASH  hHash)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    XOS_ArrayMemDst(hash->elements);
    XOS_Free( hash);

    return XSUCC;
}


/************************************************************************
* ��������XOS_HashClear
* ���� : ���hash���մ���ʱ���Ǹ�״̬
* ���� : hHash                   - HASH������
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashClear( XOS_HHASH  hHash )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HARRAY ra = XNULL;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    /*HASH Ͱָ���ʼ��*/
    XOS_MemSet(((XU8*)hash + sizeof(t_XOSHASH)), 0, 
        (hash->numKeys) * sizeof(XOS_ArrayElement));

    hash->hash = XOS_Hashstr;
    hash->compare = XOS_HashDefaultCompare;
    hash->curSize = 0;
    hash->keys = (XOS_ArrayElement*)((XU8*)hash + sizeof(t_XOSHASH));
    if (XNULL == (ra = hash->elements))
    {
        XOS_MemFree( (XU32)FID_ROOT, hash);
        return XERROR;
    }

    XOS_ArrayInitClear((XOS_HARRAY)ra);   /* ��ʼ������ */
    return XSUCC;
}


/************************************************************************
* XOS_HashElemAdd
* ����: ����һ���µ�Ԫ�ص�hash��
*              �������allowSameKey����ΪFALSE
               �����ж�hash�����Ƿ��Ѿ�����һ��Ԫ��keyֵ
               ����Ԫ�ص�keyֵ��ȣ���������Ԫ�ز������
* ����:
*              hHash                          - HASH ������
*              pKey                           - keyֵ
*              pElement                       - ��ӵ�Ԫ��
*              allowSameKey                   - �Ƿ���������ͬ��key
* ���: ��
* ����: �ɹ�����ָ��Ԫ����hash���е�λ��
*       ʧ�ܷ���XNULL
* ˵��: Ҫ���û������key��element������Ϊ�ղ����
************************************************************************/
#if 0
XPUBLIC  XVOID*  XOS_HashElemAdd(
                                XOS_HHASH  hHash,
                                XVOID*        pKey,
                                XVOID*        pElement,
                                XBOOL         allowSameKey)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* newElem;
    XS32 keyValue;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == pKey || XNULL == pElement)
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /* ���ҵ�����ͬkey ֵ��Ԫ���򷵻�Ԫ�أ����������Ԫ��*/
    if (!allowSameKey)
    {
        newElem = (XOS_HashListElement*)HashFind(hHash, pKey);
        if (XNULL != newElem)
        {
            return newElem;
        }
    }

    /* ��æ�����в����Ƿ��п���Ԫ�ؿռ�*/
    if (0 > XOS_ArrayAddExt(hash->elements, (XOS_ArrayElement*)&newElem))
    {
        /* Not found... */
        return XNULL;
    }
    /* ��hash ������ɢ��Ͱ*/
    keyValue = hash->hash(pKey, (XS32)(hash->userKeySize), 
        (XS32)(hash->numKeys)) % ((XU32)hash->numKeys);
    newElem->prev = XNULL;
    newElem->entryNum = keyValue;
    newElem->next = (XOS_HashListElement *)hash->keys[keyValue];
    if (XNULL != newElem->next)
    {
        newElem->next->prev = newElem;
    }

    /* ���Ԫ��*/
    XOS_MemCpy((XCHAR*)newElem + sizeof(XOS_HashListElement), pKey, hash->userKeySize);
    XOS_MemCpy((XCHAR*)newElem + sizeof(XOS_HashListElement) + hash->alignedKeySize, 
        pElement, hash->userElemSize);
    hash->keys[keyValue] = (XOS_ArrayElement)newElem;
    hash->curSize++;
    return newElem;
}
#endif


/************************************************************************
* XOS_HashEbscSagAdd
* ����: ����һ���µ�Ԫ�ص�hash��
* ����:
*              hHash                          - HASH ������
*              pKey                           - keyֵ
*              pElement                       - ��ӵ�Ԫ��
*              allowSameKey                   - �Ƿ���������ͬ��key                 
* ���: pIndex                        - hashԪ�ش洢��������
* ����: �ɹ�����ָ��Ԫ����hash���е�λ��
*       ʧ�ܷ���XNULL
* ˵��: �˽ӿ���ʱֻ��ebsc��sagʹ��
************************************************************************/
XPUBLIC  XVOID*  XOS_HashEbscSagAdd(
                                    XOS_HHASH     hHash,
                                    XVOID*        pKey,
                                    XVOID*        pElement,
                                    XBOOL         allowSameKey,
                                    XS32*         pIndex )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* newElem;
    XS32 keyValue;
    XS32 index = -1;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == pKey || XNULL == pElement)
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /* ���ҵ�����ͬkey ֵ��Ԫ���򷵻�Ԫ�أ����������Ԫ��*/
    if (!allowSameKey)
    {
        newElem = (XOS_HashListElement*)HashFind(hHash, pKey);
        if (XNULL != newElem)
        {
            return newElem;
        }
    }

    /* ��æ�����в����Ƿ��п���Ԫ�ؿռ�*/
    if (0 >(index=XOS_ArrayAddExt(hash->elements, (XOS_ArrayElement*)&newElem)))
    {
        /* Not found... */
        return XNULL;
    }
    /* ��hash ������ɢ��Ͱ*/
    keyValue = hash->hash(pKey, (XS32)(hash->userKeySize), 
        (XS32)(hash->numKeys)) % ((XU32)hash->numKeys);
    newElem->prev = XNULL;
    newElem->entryNum = keyValue;
    newElem->next = (XOS_HashListElement *)hash->keys[keyValue];
    if (XNULL != newElem->next)
    {
        newElem->next->prev = newElem;
    }

    /* ���Ԫ��*/
    XOS_MemCpy((XCHAR*)newElem + sizeof(XOS_HashListElement), pKey, hash->userKeySize);
    XOS_MemCpy((XCHAR*)newElem + sizeof(XOS_HashListElement) + hash->alignedKeySize, 
        pElement, hash->userElemSize);
    hash->keys[keyValue] = (XOS_ArrayElement)newElem;
    hash->curSize++;
    if(pIndex != XNULLP)
    {
        *pIndex = index;
    }
    return newElem;
}


/************************************************************************
* ��������XOS_HashElemDel
* ����: ��hash����ɾ��һ��ָ��λ���ϵ�Ԫ��
* ����:
*       hHash          - HASH������
*       pLocation      - hashԪ�ص�λ��ָ��,��add����ʱ����
* ��� : ��
* ���� : �ɹ����� XSUCC, ʧ�ܷ��� XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashElemDel( XOS_HHASH   hHash,  XVOID* pLocation )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* userElem;
    XS32 loc;

    if (XNULL == hash || XNULL == pLocation)
    {
        return XERROR;
    }

    userElem = (XOS_HashListElement *)pLocation;
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, userElem)))
    {
        return XERROR;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XERROR;
    }

    /* ��Ԫ�ش�hash Ͱ��ɾ��*/
    if (XNULL == userElem->prev)
    {
        /* First element - update keys table */
        hash->keys[userElem->entryNum] = userElem->next;
    }
    else
    {
        userElem->prev->next = userElem->next;
    }

    if (XNULL != userElem->next)
    {
        userElem->next->prev = userElem->prev;
    }
    hash->curSize = hash->curSize - 1;

    /* ���ռ���뵽������*/
    return XOS_ArrayDeleteByPos(hash->elements, XOS_ArrayGetByPtr(hash->elements, userElem));
}


/************************************************************************
* ��������XOS_HashDelByElem
* ����: ��hash����ɾ��һ��ָ��λ���ϵ�Ԫ��
* ����:
*       hHash          - HASH������
*       pElem          - hashԪ�ص���ʵλ��ָ��
* ��� : ��
* ���� : �ɹ����� XSUCC, ʧ�ܷ��� XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashDelByElem( XOS_HHASH   hHash,  XVOID* pElem )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XOS_HashListElement* userElem;
    XS32 loc;

    if (XNULL == hash || XNULL == pElem)
    {
        return XERROR;
    }
    userElem = (XOS_HashListElement *)((char*)pElem - hash->alignedKeySize - sizeof(XOS_HashListElement));
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, userElem)))
    {
        return XERROR;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XERROR;
    }

    /* ��Ԫ�ش�hash Ͱ��ɾ��*/
    if (XNULL == userElem->prev)
    {
        /* First element - update keys table */
        hash->keys[userElem->entryNum] = userElem->next;
    }
    else
    {
        userElem->prev->next = userElem->next;
    }

    if (XNULL != userElem->next)
    {
        userElem->next->prev = userElem->prev;
    }
    hash->curSize = hash->curSize - 1;

    /* ���ռ���뵽������*/
    return XOS_ArrayDeleteByPos(hash->elements, XOS_ArrayGetByPtr(hash->elements, userElem));
}


/************************************************************************
* XOS_HashElemFind
* ����: ͨ��keyֵ����Ԫ�ص�λ��
* ����:
*       hHash                        - HASH������
*       key                          - keyֵ
* ���: ��
* ����: �ɹ�����ָ���һ������keyֵ��Ԫ�ص�ָ��
*       ����ʧ�ܷ���XNULLָ��
************************************************************************/
#if 0
XPUBLIC  XVOID*    XOS_HashElemFind(XOS_HHASH  hHash, XVOID* pKey)
{
    XVOID *ptr = XNULL;

#ifdef XOS_HASH_DEBUG     
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /* ͨ��key ����Ԫ��*/
    ptr = HashFind( (XOS_HHASH)hHash, pKey);
    if ( XNULL != ptr )
    {
        /*  ����Ԫ��*/
        return ( XOS_HashGetElem( (XOS_HHASH)hHash, ptr ) );
    }
    return XNULL;    /* û���ҵ��򷵻�XNULL*/
}
#endif


/************************************************************************
* XOS_HashEbscSagFind
* ����: ͨ��keyֵ����Ԫ�ص�λ��
* ����:
*       hHash                      - HASH������
*       key                        - keyֵ
* ���: pIndex                     - hash  Ԫ�ص�������
* ����: �ɹ�����ָ���һ������keyֵ��Ԫ�ص�ָ��
*       ����ʧ�ܷ���XNULLָ��
************************************************************************/
XPUBLIC  XVOID*    XOS_HashEbscSagFind(XOS_HHASH  hHash, XVOID* pKey, XS32* pIndex )
{
    XVOID *ptr = XNULL;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /* ͨ��key ����Ԫ��*/
    ptr = HashFind( (XOS_HHASH)hHash, pKey);
    if ( XNULL != ptr )
    {
        /*  ����Ԫ��*/
        if(pIndex != XNULLP)
        {
            *pIndex = XOS_ArrayGetByPtr(((t_XOSHASH*)hHash)->elements, ptr);
        }
        return ( XOS_HashGetElem( (XOS_HHASH)hHash, ptr ) );
    }
    return XNULL;    /* û���ҵ��򷵻�XNULL*/
}


/************************************************************************
* XOS_HashElemFindNext
* ����: �������ϴε���XOS_HashElemFind()����ͬ��keyֵ
        ����һ��Ԫ��
* ���� :
*        hHash             - HASH������
*        pKey              - keyֵ
*        pLocation         - �ϴ���HashFindNext() ��HashFind()���ҵ���Ԫ��
* ��� : ��
* ���� : �ɹ������ҵ���Ԫ�ص�ָ�룬����XNULL
************************************************************************/
XPUBLIC  XVOID*    XOS_HashElemFindNext(
                                       XOS_HHASH   hHash,
                                       XVOID*         pKey,
                                       XVOID*         pLocation )
{
    XVOID *ptr1 = XNULL, *ptr2 = XNULL;
    t_XOSHASH*  pHash = (t_XOSHASH *)XNULL;

    if ( ( !XOS_HashHandleIsValid(hHash) ) 
        || (XNULL == pKey) || (XNULL == pLocation) )
    {
        return( XNULL );
    }
    pHash = (t_XOSHASH *)hHash;
    ptr1 = ((XCHAR *)pLocation - (pHash->alignedKeySize)) - sizeof(XOS_HashListElement) ;

    /* �ӵ�ǰλ�����²�����ͬkey ֵ��Ԫ��*/
    ptr2 = HashFindNext( (XOS_HHASH)pHash, pKey, ptr1  );
    if ( XNULL != ptr2 )
    {
        return ( XOS_HashGetElem( (XOS_HHASH)pHash, ptr2 ) );
    }
    return XNULL;
}


/************************************************************************
* ��������XOS_HashGetElem
* ����: ͨ��λ����Ϣȡ��Ԫ��
* ����:
*       hHash       - HASH������
*       pLocation   - hashԪ�ص�λ��ָ��,��add����ʱ����
* ��� : ��
* ���� : �ɹ�����Ԫ�ص��׵�ַ,ʧ�ܷ���XNULL
************************************************************************/
XPUBLIC  XVOID*  XOS_HashGetElem( XOS_HHASH hHash,  XVOID* pLocation)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XS32 loc;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, pLocation)))
    {
        return XNULL;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XNULL;
    }

    if (XNULL == pLocation)
    {
        return XNULL;
    }

    /* ����Ԫ��ֵ*/
    return ((XCHAR*)pLocation + sizeof(XOS_HashListElement) + hash->alignedKeySize);
}


/************************************************************************
* ��������XOS_HashSetElem
* ����: ��λ����ϢΪlocation��Ԫ�����ó�pElem
* ����:
*       hHash           - HASH������
*       pLocation       - hashԪ�ص�λ��ָ��,��add����ʱ����
*       pElem           - Ҫ����Ԫ�ص��׵�ַ
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC XS32 XOS_HashSetElem(XOS_HHASH  hHash, XVOID *pLocation, XVOID* pElem)
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XS32 loc;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, pLocation)))
    {
        return XERROR;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XERROR;
    }

    if ( XNULL == pLocation || XNULL == pElem)
    {
        return XERROR;
    }

    /* ����Ԫ�ص�ֵ*/
    XOS_MemCpy((XCHAR*)pLocation+ 
        sizeof(XOS_HashListElement) + hash->alignedKeySize, pElem, hash->userElemSize);
    return XSUCC;
}


/************************************************************************
* ��������XOS_HashGetKeyByElem
* ����: ȡ��λ����ϢΪlocation��Ԫ�صĹؼ���
* ����:
*       hHash          - HASH������
*       pElem          - hashԪ�ص�λ�õ�ָ��
* ���: ��
* ����: �ɹ�����key���׵�ַ, ʧ�ܷ��ؿ�
************************************************************************/
XPUBLIC  XVOID*  XOS_HashGetKeyByElem(XOS_HHASH hHash, XVOID* pElem )
{
    t_XOSHASH* hash = (t_XOSHASH *)hHash;
    XS32 loc;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == pElem )
    {
        return XNULL;
    }
#endif /*XOS_HASH_DEBUG*/

    /*�ж�pElem�Ƿ���Ч*/
    if (XERROR == (loc=XOS_ArrayGetByPtr(hash->elements, (XCHAR*)pElem - hash->alignedKeySize - sizeof(XOS_HashListElement))))
    {
        return XNULL;
    }
    else if (XOS_ArrayElemIsVacant( hash->elements,  loc))
    {
        return XNULL;
    }

    return (XVOID*)((XCHAR*)pElem - hash->alignedKeySize);/*���عؼ���*/
}


/************************************************************************
* ��������XOS_HashWalk
* ����: ����hash��(ע��Ҫ��д��������)
* ����:
*       hHash                - hash����
*       fFunc                - ��������
*       pParam               - ���������Ĳ���
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashWalk( XOS_HHASH  hHash, XOS_HashFunc  fFunc, XVOID*   pParam)
{
#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash ) || XNULL == fFunc )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    return ( HashDoAll( (XOS_HHASH)hHash, (XOS_HashFunc)fFunc, pParam ) );
}


/************************************************************************
* XOS_HashSetHashFunc
* ����: ����hash���hash����
* ����:
*       hHash         - HASH������
*       fFunc         - hash����
* ���: ��
* ����: �ɹ�����XSUCC,����XERROR
************************************************************************/
XPUBLIC  XS32  XOS_HashSetHashFunc( XOS_HHASH hHash,  HashFunc fFunc )
{
    t_XOSHASH *pHash = (t_XOSHASH *)XNULL;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    pHash = (t_XOSHASH *)hHash;
    if (XNULL != fFunc)
    {
        pHash->hash = fFunc ;                               /* ����hash ����*/
        return ( XSUCC );
    }
    else
    {
        return ( XERROR );
    }
}


/************************************************************************
* ��������XOS_HashSetKeyCompareFunc
* ����: ����hash�ؼ��ֱȽϺ���
(�ú�����hash�����ʵ������,����ʵ��ģ����ѯ)
* ����:
*       hHash              - HASH������
*       fFunc              - hash�ؼ��ֱȽϺ���
* ��� : ��
* ���� : �ɹ�����XSUCC, ʧ�ܷ���XERROR
************************************************************************/
XPUBLIC XS32  XOS_HashSetKeyCompareFunc( XOS_HHASH hHash,  HashKeyCmp fFunc )
{
    t_XOSHASH *pHash = XNULL;

    pHash = (t_XOSHASH *)hHash;

#ifdef XOS_HASH_DEBUG
    if ( XTRUE != XOS_HashHandleIsValid( hHash )  )
    {
        return XERROR;
    }
#endif /*XOS_HASH_DEBUG*/

    if (XNULL != fFunc)
    {
        pHash->compare = fFunc ;                           /* ���ñȽϺ���*/
        return ( XSUCC );
    }
    else
    {
        return ( XERROR );
    }
}


#ifdef   __cplusplus
}
#endif   /*  __cplusplus  */

