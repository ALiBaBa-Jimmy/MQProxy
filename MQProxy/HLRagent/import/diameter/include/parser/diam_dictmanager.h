/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年5月7日
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
