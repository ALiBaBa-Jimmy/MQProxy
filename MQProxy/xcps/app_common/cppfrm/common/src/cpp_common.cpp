/************************************************************************************
�ļ���  :cpp_common.cpp

�ļ�����:

����    :zzt

��������:2005/10/20

�޸ļ�¼:

************************************************************************************/
#include "cpp_common.h"



cspout::endlout cspout::endl;


extern "C" XS32 XOS_GetSysPath(XCHAR *szSysPath, XS32 nPathLen);


XS8 CPathSS::sRootPath[SS_PATH_MAX] = {0};
XS8* CPathSS::sDir[] = {".","./","./cfg","./log","./doc","./cache","./tmp", "./cdr"};


/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CPathSS::CPathSS(DIR_TYPE  type, const XS8* filename)
{
    InitPathRoot();
    sprintf(sFileName, "%s%s", (XS8*)CPathSS(type), filename);
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CPathSS::CPathSS(DIR_TYPE  type)
{
    InitPathRoot();

    XU32 dir_len = 0;
    dir_len = XOS_StrLen(sRootPath);

    if(dir_len < SS_PATH_MAX && dir_len > 0)
    {
        if ( !('\\' == sRootPath[dir_len-1] || '/' == sRootPath[dir_len-1] ))
        {
            XOS_StrCat(sRootPath, "/");
        }
    }
    if(dir_len <=2)
    {
        XOS_MemSet(sRootPath, 0x0,sizeof(sRootPath));
        XOS_StrCpy(sRootPath, "./");
    }
    switch(type)
    {
    case DIR_ROOT:
        sprintf(sFileName, "%s", sRootPath);
        break;
    case DIR_BIN:
        sprintf(sFileName, "%s", sRootPath);
        break;
    case DIR_CFG:
        sprintf(sFileName, "%s%s/", sRootPath,"cfg");
        break;
    case DIR_LOG:
        sprintf(sFileName, "%s%s/", sRootPath,"log");
        break;
    case DIR_DOC:
        sprintf(sFileName, "%s%s/", sRootPath,"doc");
        break;
    case DIR_CACHE:
        sprintf(sFileName, "%s%s/", sRootPath,"cache");
        break;
    case DIR_TMP:
        sprintf(sFileName, "%s%s/", sRootPath,"tmp");
        break;
    case DIR_CDR:
        sprintf(sFileName, "%s%s/", sRootPath,"cdr");
        break;
    default:
        sprintf(sFileName, "%s%s/", sRootPath, sDir[type]);
        break;
    }
}

/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CPathSS::CPathSS()
{
    InitPathRoot();

    XOS_MemSet(sFileName, 0, sizeof(sFileName));
}


