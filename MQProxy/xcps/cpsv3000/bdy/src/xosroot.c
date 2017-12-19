/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosroot.c
**
**  description:  ����ƽ̨����
**
**  author: wangzongyou
**
**  date:   2006.07.20
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
 
**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include "xosroot.h"
#include "xosmodule.h"
#include "xoscfg.h"
#include "xostrace.h"
#include "clishell.h"
#include "xosmem.h"
#include "xosencap.h"
#include "xosmmgt.h"
#include "xosnetinf.h"

/*-------------------------------------------------------------------------
                 ȫ�ֶ���
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                 ģ���ڲ��궨��
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
XU32 g_startSec = 0;

/*-------------------------------------------------------------------------
                 ģ���ڲ��ṹ��ö�ٶ���
-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
                ģ���ڲ�����
-------------------------------------------------------------------------*/
#ifdef XOS_MDLMGT
XPUBLIC XS32 XOS_Init(HANDLE hDir,XS32 argc, XS8** argv);
XPUBLIC XS32 XOS_ModNotice(HANDLE hDir,XS32 argc, XS8** argv);
#endif

#ifdef XOS_IPC_MGNT
XEXTERN XU32 XOS_IPCGetLocalSlot();
#endif

#ifdef XOS_LINUX
XEXTERN XVOID XOS_RegisterAllSignal();
#endif

/*-------------------------------------------------------------------------
                ģ��ӿں���
-------------------------------------------------------------------------*/
#ifndef XOS_MDLMGT
/************************************************************************
������: XOS_Root
���ܣ�  ƽ̨�����ں���
���룺  XVOID
�����  N/A
���أ�  XSUCC OR XERROR
˵����  ����ƽ̨����ĳ�ʼ����������ҵ�����д��ע��ṹ��������
        �Ĵ��������ȵ�
************************************************************************/
XPUBLIC XS8 XOS_Root(XVOID)
{
    XS32 ret;

	//#ifdef XOS_LINUX
#if 0
    /*���LINUX�����ź�ע��*/
    XOS_RegisterAllSignal();
#endif

	//#ifdef XOS_LINUX
#if 0
    /* ipmi������� */
    ret = XOS_CheckIpmiEnv();
    if (XSUCC == ret)
    {
        XOS_SetIpmiFlag(XTRUE);
    }
    else
    {
        XOS_SetIpmiFlag(XFALSE);
        printf("XOS_Root()->ipmi env check failed");
    }
#else
    XOS_SetIpmiFlag(XFALSE);
#endif

    /*�ڴ��ʼ�� */
    ret = MEM_Initlize();
    if(XSUCC != ret)
    {
        /*trace ��û�г�ʼ��*/
        printf( "XOS_Root()->mem init failed\n");
        goto rootErrorProc;
    }

    /*ģ������ʼ��*/
    ret = MOD_init();
    if(XSUCC != ret)
    {
        /*trace ��û�г�ʼ��*/
        printf("XOS_Root()->module management init failed\n");
        goto rootErrorProc;
    }
#ifdef XOS_IPC_MGNT
    XOS_SetLocalPID(XOS_IPCGetLocalSlot());
#endif
    /* ����ƽ̨*/
    ret = MOD_startXosFids();
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_Root()->start xos fids  failed");
        goto rootErrorProc;
    }

    /*����ҵ��ģ��*/
    ret = MOD_startUserFids();
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_Root()->start user fids  failed");
        goto rootErrorProc;
    }

    /*����֪ͨ*/
    ret = MOD_StartNotice();
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_Root()->start notice  failed");
        goto rootErrorProc;
    }

    return XSUCC;

rootErrorProc:

#ifdef XOS_DEBUG    
    /*������, ϵͳ��û���������������ֳ���Ϣ*/        
    XOS_SusPend();
#endif

    /*��ʱ�����ͷ���Դ�Ĵ���ϵͳ�������϶�û�а취�ܵ�*/
    return XERROR;
}
#endif


#ifdef XOS_MDLMGT
extern XS32  XOS_FIDROOT(HANDLE hDir,XS32 argc, XS8** argv);
/************************************************************************
������: XOS_Root
���ܣ�  
���룺  XVOID
�����  N/A
���أ�  XSUCC OR XERROR
˵����  
************************************************************************/
XPUBLIC XS32 XOS_Init(HANDLE hDir,XS32 argc, XS8** argv)
{
    XS32 ret;

	//#ifdef XOS_LINUX
#if 0
    /*���LINUX�����ź�ע��*/
    XOS_RegisterAllSignal();
#endif

#if 0
	#ifdef XOS_LINUX

    /* ipmi������� */
    ret = XOS_CheckIpmiEnv();
    if (XSUCC == ret)
    {
        XOS_SetIpmiFlag(XTRUE);
    }
    else
    {
        XOS_SetIpmiFlag(XFALSE);
        printf("XOS_Init()->ipmi env check failed");
    }
#else
    XOS_SetIpmiFlag(XFALSE);
#endif
#endif


    /*�ڴ��ʼ�� */
    ret = MEM_Initlize();
    if(XSUCC != ret)
    {
        /*trace ��û�г�ʼ��*/
        printf( "XOS_Init()->MEM_Initlize failed\n");
        return XERROR;
    }

    ret = MOD_init();
    if(XSUCC != ret)
    {
        /*trace ��û�г�ʼ��*/
        printf( "XOS_Init()->MOD_init failed\n");
        return XERROR;
    }

    /* ��ʼ���������û��� */
    ret = XOS_NetInfInit();
    if (XSUCC != ret)
    {
        printf("XOS_Init()->net interface module init failed\n");
        return XERROR;
    }

    g_startSec = XOS_TicksToSec(XOS_GetSysTicks());
#ifdef XOS_IPC_MGNT
    XOS_SetLocalPID(XOS_IPCGetLocalSlot());
#endif

    XOS_FIDROOT(hDir, argc, argv);


    return XSUCC;
}


/************************************************************************
������: XOS_ModNotice
���ܣ�  
���룺  XVOID
�����  N/A
���أ�  XSUCC OR XERROR
˵����  
************************************************************************/
XPUBLIC XS32 XOS_ModNotice(HANDLE hDir,XS32 argc, XS8** argv)
{
    XS32 ret;

    /*����֪ͨ*/
    ret = MOD_StartNotice();
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_ROOT, PL_ERR), "XOS_Root()->start notice  failed");
        return XERROR;
    }
    return XSUCC;
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
