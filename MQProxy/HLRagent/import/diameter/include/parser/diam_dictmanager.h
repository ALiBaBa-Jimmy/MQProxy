/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��5��7��
**************************************************************************/
#ifndef __DIAM_DICTMANAGER_H__
#define __DIAM_DICTMANAGER_H__

#include <parser/diam_datatype.h>
#include <parser/diam_dictobject.h>

class CDiamDictManager
{
public:
    static void LoadBaseDictionary(char* xmlFile);
    static void LoadProductDictionary(char* xmlFile);
    static void Dump();
};

#endif /*__DIAM_DICTMANAGER_H__*/
