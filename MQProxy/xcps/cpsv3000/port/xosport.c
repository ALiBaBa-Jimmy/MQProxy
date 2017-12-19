/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosport.c
**
**  description:  ��ģ�����Ķ������ļ�����ʱ������ļ�
                          ����ʧ��
**
**  author: wangzongyou
**
**  date:   2006.7.24
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
#include "xosshell.h"

/*-------------------------------------------------------------------------
                 ģ���ڲ��ṹ��ö�ٶ���
-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------
                ģ���ڲ�ȫ�ֱ���
-------------------------------------------------------------------------*/
/* t_XOSLOGINLIST *LoginList;*/
/*change by wulei 2006.8.18 
  �˱���Ӧ����ע��ʱ����ڲ�ͬ���ļ��У���˷���.h�ļ��Ϻ�
  ���˱�����������xosport.h��
*/


/*-------------------------------------------------------------------------
                ģ���ڲ�����
-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
                ģ��ӿں���
-------------------------------------------------------------------------*/
#ifdef XOS_NEED_MAIN


#ifndef XOS_MDLMGT
#if 1/* ��B10���ṩ��ģ��������ʽ ��ҵ����Ҫ�����Լ����з�װ */
/************************************************************************
������:XOS_InfoReg
����: ҵ��ģ���ע��
����:
���:
����:
˵��: ��ʱΪ�˽ӿڲ���,д����.c�ļ�ע��. ��ģ�����
          ʱ,��ȡ����.c �ļ�,���е�ģ��ͨ���������ļ�����
************************************************************************/
XPUBLIC XS8 XOS_InfoReg(t_XOSLOGINLIST *LoginList)
{
    /*�ڴ����������ҵ���Ӧ��*/

    /*ע���±������FID_MAX-FID_USERMIN ��, ����Խ��*/
    return XSUCC;
}
#endif


#if defined (XOS_WIN32) || defined (XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VXWORKS)
/************************************************************************
������: main
����:
����:
���:
����:
˵��:
************************************************************************/
int main( int argc, char **argv )
{
    XOS_DaemonInit( argc, argv );
    /*  �ڴ˵���ƽ̨��ں���������ƽ̨ */
    if(XSUCC != XOS_Root())
    {
        return XERROR;
    }

    CLI_locConsoleEntry();

    return 0;
}
#endif

#endif /* XOS_MDLMGT */ 

/* ģ�����������ʽ */
#ifdef XOS_MDLMGT
/************************************************************************
������:    main        
���ܣ�          
������          
�����          
���أ�          
˵����          
************************************************************************/

#if defined (XOS_WIN32) || defined (XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VTA)
int main( int argc,char *argv[] )
{
    g_main_argc = argc;
    g_main_argv = argv;
    XOS_DaemonInit( argc, argv );

#ifndef XOS_EW_START    
    if (XSUCC != XosMMinitSymbTbl(argv[0]))
#else
    if (XSUCC != XosMMinitSymbTbl( XOS_CliGetSysName() ))
#endif
    {
        MMErr("Init Symbol Table Failed");
        return -1;
    }

    XosModuleManagerInit(argv[1], argv[2]);
    /*MM_taskSuspendCurrent();*/

    CLI_locConsoleEntry();

    return 0;
}
#endif /* #if defined (XOS_WIN32) || defined (XOS_LINUX) || defined(XOS_SOLARIS) */


#ifdef XOS_VXWORKS
#ifdef XOS_NVTA
XVOID CPSRoot(int argc,char **argv)
{
    if (XSUCC != XosMMinitSymbTbl(argv[0]))
    {
        MMErr("Init Symbol Table Failed");
        return;
    }

    XosModuleManagerInit(argv[1], argv[2]);
    MM_taskSuspendCurrent();
    return;
}
#endif /*XOS_NVTA*/
#endif/* XOS_VXWORKS */
#endif/* XOS_MDLMGT */
#endif /*XOS_NEED_MAIN*/


#ifdef __cplusplus
}
#endif /* __cplusplus */


