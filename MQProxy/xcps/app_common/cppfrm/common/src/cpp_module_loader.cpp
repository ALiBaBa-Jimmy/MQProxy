/************************************************************************************
�ļ���  :cpp_module_loader.cpp

�ļ�����:

����    :zzt

��������:2006/05/09

�޸ļ�¼:
         �����XOSƽ̨����
************************************************************************************/
#include "cpp_module_loader.h"



/*********************************************************************
�������� :
�������� :

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
**********************************************************************/
CModuleMap& GetModuleMap()
{
    static CModuleMap s_lms;
    return s_lms;
}

/*********************************************************************
�������� : FindFct
�������� : ͨ��ģ��ID����һ��ģ��,��ǰʵ���Ǳ�������
           ����Ч���нϸߵ�Ҫ�󣬿��Ը���

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
�������� : GetModuleName
�������� : ��ȡģ������

�������� :
������� :
����ֵ   :

�޸���ʷ : Author        mm/dd/yy       Initial Writing
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
        // ��ҵ��ģ�飬ֱ�Ӵ�ӡid
        sprintf(pOutBuf, "%d", mid);
    }

    return pOutBuf;
}




