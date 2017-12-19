/************************************************************************************
文件名  :cpp_module_loader.cpp

文件描述:

作者    :zzt

创建日期:2006/05/09

修改记录:
         框架与XOS平台联调
************************************************************************************/
#include "cpp_module_loader.h"



/*********************************************************************
函数名称 :
功能描述 :

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CModuleMap& GetModuleMap()
{
    static CModuleMap s_lms;
    return s_lms;
}

/*********************************************************************
函数名称 : FindFct
功能描述 : 通过模块ID查找一个模块,当前实现是遍历查找
           若对效率有较高的要求，可以更改

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XVOID* FindFct(MDID mid)
{
    CModuleMap::iterator it = GetModuleMap().find(mid);
    if(GetModuleMap().end() != it)
    {
        return static_cast<XVOID*>(it->second->m_pFct);
    }
    return XNULL;
}

/*********************************************************************
函数名称 : GetModuleName
功能描述 : 获取模块名称

参数输入 :
参数输出 :
返回值   :

修改历史 : Author        mm/dd/yy       Initial Writing
**********************************************************************/
XS8* GetModuleName(MDID mid, XS8* pOutBuf)
{
    if (pOutBuf == XNULL)
    {
        return pOutBuf;
    }

    CModuleMap::iterator it = GetModuleMap().find(mid);
    if(GetModuleMap().end() != it)
    {
        if (XNULL != it->second)
        {
            XOS_StrCpy(pOutBuf, it->second->m_fData.head.FIDName);
        }
        else
        {
            sprintf(pOutBuf, "%d", mid);
            //XOS_Sprintf();
        }
    }
    else
    {
        // 非业务模块，直接打印id
        sprintf(pOutBuf, "%d", mid);
    }

    return pOutBuf;
}




