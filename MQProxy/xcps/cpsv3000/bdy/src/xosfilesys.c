/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosfilesys.c
**
**  description:  ��win2000/turbolinux/solaris/vxworks�µ��ļ���Ŀ¼�������з�װ
    ���ڲ�ͬ�Ĳ���ϵͳ���ò�ͬ�ĺ�����
    XOS_WIN32----window
    XOS_LINUX-----linux
    XOS_SOLARIS---solaris
    XOS_VXWORKS---vxworks
    XOS_VTA------vxworks�е�vtaϵͳ
    XOS_NVTA-----vxworks�еķ�vtaϵͳ
**
**  author: lixiangni
**
**  date:   2006.8.22
**
***************************************************************
**                          history
**
***************************************************************

**   author          date              modification
**   lxn             2006.8.22          create

**************************************************************/

#ifdef __cplusplus

extern "C" {

#endif /* __cplusplus */

/*---------------------------------------------------------*
                 ͷ�ļ�
*----------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include "xosfilesys.h"
#include "xostrace.h"
#include "xoscfg.h"

//#if ( XOS_LINUX || XOS_SOLARIS)
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )

#include <unistd.h>

#endif /*linux/solaris*/

#define WORK_ROOT_PATH "/XOS"

typedef struct _FILE_MUTEX_LOCK
{
  t_XOSMUTEXID changdir_lock;               /*Ŀ¼ת����*/
  t_XOSMUTEXID file_lock;                   /*�ļ����������ļ�д���ļ��ƶ���*/
}file_mutex_lock;

XSTATIC file_mutex_lock mutexlock_file_dir; /*�ļ�Ŀ¼�������ṹ*/

/*----------------------------------------------------------*
����ȫ�ֱ�������
*------------------------------------------------------------*/
XSTATIC XS32 XOS_FILE_ERRORNO;
XSTATIC XCHAR *OPENFILEMODE[]                          = { "r","w","a","rb","wb","ab","r+","w+","a+","rb+","wb+","ab+" };
XSTATIC XCHAR FILE_POSTFIXNAME[5]                      = ".txt";
XSTATIC XCHAR CURRENTWORK_PATH[MAX_DIRANDFILEPATH_LEN] = {0};

/*---------------------------------------------------------*
�ڲ���������
*----------------------------------------------------------*/

/**********************************************************

��  �ܣ��ж������ļ�·�����Ƿ���ȷ,���·����ɺϷ����ڲ�ͬ��ϵͳ��ת��Ϊ��ͬ�ĸ�ʽ
        ���봮���󷵻�XERROR������ת������������XSUCC

***********************************************************/

XS32 FS_formatpath(XCONST XCHAR *srcpathstr,XCHAR *formatpathstr);

/**********************************************************

��  �ܣ�����������ļ�·�����ļ��Ƿ������չ����
        ���������ļ���û����չ�����򷵻���ʾû����չ��

**********************************************************/
XS32 FS_isfilenameright(XCONST XCHAR *filename);

/**********************************************************

��  �ܣ�����ͬ����ϵͳ�µ��������Լ������ϲ�Ϊdesstr��ʽ���Ա���ϵͳ����

**********************************************************/
XS32 FS_CatDelorder(XCHAR *desstr,XCONST XCHAR *order,XCONST XCHAR *path);

/**********************************************************

��  �ܣ��жϴ����Ŀ¼�����Ŀ¼���Ǿ���Ŀ¼

**********************************************************/
XS32 FS_IsAbsolOrRelaDir(XCONST XCHAR *dir);

/*---------------------------------------------------------------*
�ļ�����ģ������ӿ�
*----------------------------------------------------------------*/

/************************************************************************

��������FS_Init
��  �ܣ����ļ�����ģ����г�ʼ��
��  �룺
��  ����
��  �أ�
˵  ����������Ҫ�Ƕ��ļ�����ģ���е�ȫ�ֱ������г�ʼ��
��  ����ϵͳĬ�Ϲ���Ŀ¼��

************************************************************************/
 XVOID FS_Init(XVOID)
{
//#if ( XOS_WIN32 )
#ifdef XOS_WIN32

     /*�õ�ϵͳ��ǰ����Ŀ¼(����·��)*/
     _getcwd(CURRENTWORK_PATH,MAX_DIRANDFILEPATH_LEN);

#endif  /*XOS_WIN32 || XOS_VTA*/

//#if ( XOS_LINUX || XOS_SOLARIS || XOS_VTA )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VTA) )

     getcwd(CURRENTWORK_PATH,MAX_DIRANDFILEPATH_LEN);

#endif  /*XOS_LINUX || XOS_SOLARIS*/

    return ;

}

/*---------------------------------------------------------*
�ļ���Ŀ¼��������ӿں���ʵ��
*----------------------------------------------------------*/

/************************************************************************
��������XOS_CreatDir
��  �ܣ������ļ�
��  �룺dirname--Ҫ������Ŀ¼·���������ܺ��Ѿ����ڵ�Ŀ¼·�����ظ����������
        ͳһ��ʽΪ��/path/dirname
��  ����
��  �أ��ɹ�����XSUCC
        ʧ�ܷ���XERROR,������XERRPARA(��Ч����)��XERRPATH(��ЧĿ¼·��)
˵  ����
************************************************************************/
XS32 XOS_CreatDir(XCONST XCHAR * dirname)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};
#ifdef XOS_VXWORKS
#ifdef XOS_VTA
    XCHAR vxorder[MAX_DIRANDFILEPATH_LEN]   = {0}; /*vxwork�µ��ñ�׼��dos����*/
#endif
#endif
    XS32  recordbackvalue                   = 1;

    /*��¼���øú���ʱ�ľ��Թ���Ŀ¼��������ɺ󷵻ظù���Ŀ¼*/
    XCHAR currentdir[MAX_DIRANDFILEPATH_LEN] = {0};

    XOS_GetCurrentWorkDir(currentdir,MAX_DIRANDFILEPATH_LEN);

    /* Ŀ¼����Ч��*/
    if ( XNULL == dirname )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input param null !");
        return XERRPARA;
    }

    /*�ж�·���Ƿ񳬳�*/
    if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(dirname) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input dirname too long !");
        return XERRPARA;
    }

    if( XSUCC != FS_formatpath(dirname,formatstr)  || '\0' == formatstr[0] )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input param dirname %s -> %s  error !",dirname,formatstr);
        return XERRPARA;
    }

    /*���� ����Ŀ¼��ת��*/
    if ( XSUCC != XOS_MutexCreate(&mutexlock_file_dir.changdir_lock) )
    {
        /*������ʧ��*/
        return XERROR;
    }

    /*��������*/
    if ( XSUCC != XOS_MutexLock(&mutexlock_file_dir.changdir_lock) )
    {
        return XERROR;
    }

    recordbackvalue = chdir(formatstr);

    /*��Ŀ¼�Ѿ�����*/
    if ( 0 == recordbackvalue )
    {
        chdir(currentdir);

        /*����*/
        if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
        {
            return XERROR ;
        }
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input dirpath is exist !");
        return XERRPATH;
    }
    else
    {
        /*����Ŀ¼*/
#ifdef  XOS_WIN32

        if ( _mkdir(formatstr) < 0 )
        {
            if ( EEXIST  == errno )
            {
                /*����*/
                if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
                {
                    return XERROR ;
                }

                XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input dirpath is exist !");
                return XF_EXIST;
            }

            if ( ENOENT == errno )
            {
                /*����*/
                if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
                {
                    return XERROR ;
                }

                XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input dirpath not find !");
                /*·��δ�ҵ�*/
                return XERRPATH;
            }
        }

#endif /*win32*/

#if ( defined (XOS_LINUX) || defined (XOS_SOLARIS) )

        /*�����û��ɶ�дִ�е��ļ���*/
        if( mkdir(formatstr,S_IRWXU) < 0 )
        {
            /*����*/
            if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
            {
                return XERROR ;
            }

            XOS_Trace(MD(FID_FILE,PL_ERR),
                "XOS_CreatDir ->creat dir %s error in linux or solaris !",
                formatstr);
            return XERROR;
        }

#endif /*linux/solaris*/

#if 0 /*modified lixn 2006.11.20*/
#ifdef XOS_VXWORKS

        if ( ERROR == mkdir(formatstr) )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->creat dir %s error in vxworks !",formatstr);
            return XERROR;
        }

#endif /*_XOS_VXWORKS_*/
#endif /*0*/

        /*���vxworks��ʵ��dos��׼���ļ�Ŀ¼����������ñ�׼����*/
#ifdef XOS_VXWORKS
#ifdef XOS_VTA

        FS_CatDelorder(vxorder,"MD ",formatstr);
        system(vxorder);

#endif /*vta*/
#endif /*XOS_VXWORKS*/

        /*���򴴽�Ŀ¼��ɣ��ָ����ú����ֳ�*/
        chdir(currentdir);

        /*����*/
        if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
        {
            return XERROR ;
        }

        return XSUCC;
    }

}

/************************************************************************
��������XOS_ChangCurrentDir
��  �ܣ������ļ�
��  �룺dirpath -- Ҫ����Ĺ���Ŀ¼��������ڣ����򷵻ش���ͳһ��·����ʽΪ/path/dirname
��  ����
��  �أ��ɹ�����XSUCC
          ʧ�ܷ���XERROR
˵  ����
************************************************************************/
XS32 XOS_ChangeCurrentDir(XCONST XCHAR *dirpath)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};

    /* Ŀ¼����Ч��*/
    if ( XNULL == dirpath )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ChangeCurrentDir ->input dirpath null !");
        return XERRPARA;
    }

    if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(dirpath) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ChangeCurrentDir ->input dirpath too long !");
        return XERRPARA;
    }

    if( XSUCC != FS_formatpath(dirpath,formatstr)  || '\0' == formatstr[0]  )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ChangeCurrentDir ->input param dirpath %s -> %s  error !",dirpath,formatstr);
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( 0 == chdir(formatstr) )
    {
        return XSUCC;
    }
    else
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ChangeCurrentDir ->chdir(%s) error!",formatstr);
        /*���ô�����-��ЧĿ¼·��*/
        return XERRPATH;
    }

}

/************************************************************************
��������XOS_DeleteDir
��  �ܣ�ɾ��Ŀ¼
��  �룺dirname -- Ҫɾ����Ŀ¼·������������ڣ����򷵻ش���ͳһ��·����ʽΪ/path/dirname

��  ����
��  �أ��ɹ�����XSUCC
          ʧ�ܷ���XERROR
˵  ����
************************************************************************/
XS32 XOS_DeleteDir(XCONST XCHAR * dirname,e_DELDIRMODE delmode)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN]  = {0};
    XCHAR expdelorder[MAX_ORDERLEN]          = {0};         /*���������EXPDEL�Ļ�����¼ɾ��Ŀ¼�Լ��������������*/
#if ( (defined ( XOS_WIN32 )) || (defined ( XOS_VTA )) )
    XCHAR windeldir[]                        = "RD /S /Q "; /*windows�µľ���ɾ��Ŀ¼����*/
#endif

#if (defined ( XOS_LINUX )) || (defined ( XOS_SOLARIS ))
    XCHAR linuxdeldir[]                      = "rm -rf ";   /*linux�¾���ɾ��Ŀ¼��������г�������Ŀ¼�ݹ�ɾ��*/
#endif
#ifdef XOS_SOLARIS
    XCHAR solarisdeldir[]                    = "rmdir ";    /*solairs��һ��ɾ��Ŀ¼����*/
#endif

    /* Ŀ¼����Ч��*/
    if ( XNULL == dirname )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->input dirpath null !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(dirname) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->input dirpath too long !");
        return XERRPARA;
    }

    if( XSUCC != FS_formatpath(dirname,formatstr) || '\0' == formatstr[0]  )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),
                     "XOS_DeleteDir ->input input param dirname %s -> %s  error !",dirname,formatstr);
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( (EXPDEL != delmode) && (NORMALDEL != delmode) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),
            "XOS_DeleteDir ->input delmode %d error !",
            delmode);
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( NORMALDEL == delmode )
    {
#if ( defined (XOS_SOLARIS) )

        /*��Ҫɾ����Ŀ¼�Լ������кϲ�*/
        if ( XSUCC != FS_CatDelorder(expdelorder,solarisdeldir,dirname) )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->del dir in solaris wrong!");
            /*���ô�����*/
            return XERRPATH;
        }

//#ifndef XOS_VXWORKS

        system(expdelorder);

//#endif /*xos_vxworks*/

        return XSUCC;

#endif /*solaris*/

//#if ( XOS_LINUX || XOS_WIN32 || XOS_VTA )
#if ( defined(XOS_LINUX) || defined(XOS_WIN32) || defined(XOS_VTA) )

        if ( 0 == rmdir(formatstr) )
        {
            return XSUCC;
        }
        else
        {
            /*����ϵͳ���صĴ��������*/
            if ( ENOTEMPTY == errno )
            {
                XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->delete dirpath not empty !");
                /*���ô�����-Ŀ¼�ǿ�*/
                return XDIREMPTY;
            }

            if ( ENOENT == errno )
            {
                XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->delete dirpath not exist !");
                /*Ŀ¼������*/
                return XERRPATH;
            }

            XOS_Trace(MD(FID_FILE,PL_ERR),
                "XOS_DeleteDir ->delete dirpath %s error !",
                dirname);
            return XERROR;
        }

#endif /*defined (linux/win32/vta)*/
    }

    if ( EXPDEL == delmode )
    {
        /*���ղ�������ɾ����Ŀ¼�Լ���Ŀ¼�µ������ļ��к��ļ�*/
        /*vxworks�°��ձ�׼dos�ļ�ϵͳ����*/
#if ( (defined ( XOS_WIN32 )) || (defined ( XOS_VTA )) )

        if ( XSUCC != FS_CatDelorder(expdelorder,windeldir,formatstr) )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->expdel dir error !");
            /*���ô�����*/
            return XERRPATH;
        }

#endif
#if ( (defined ( XOS_LINUX )) || (defined ( XOS_SOLARIS )) )

        /*��Ҫɾ����Ŀ¼�Լ������кϲ�*/
        if ( XSUCC != FS_CatDelorder(expdelorder,linuxdeldir,dirname) )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->expdel dir error !");
            /*���ô�����*/
            return XERRPATH;
        }

#endif /*XOS_LINUX || XOS_SOLARIS*/

//#if ( XOS_LINUX || XOS_WIN32 || XOS_VTA || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_WIN32) || defined(XOS_VTA) || defined(XOS_SOLARIS) )

        /*����ϵͳ��ɾ��Ŀ¼����*/
           system(expdelorder);

#endif /* vxworks*/

        return XSUCC;

    }

    return XSUCC;
}

/************************************************************************
��������XOS_OpenFile
��  �ܣ������ļ�
��  �룺filename -- Ҫ�򿪵��ļ������������������ļ������򷵻ش���
        ͳһ��·����ʽΪ/path/filename+��չ��
        ���û����չ������ͳһ�ļ���չ��Ϊ.txt
��  ����
��  �أ��ɹ�����ָ����ļ����ļ�ָ��
          ʧ�ܷ���NULL
˵  �����Ը���ϵͳ�µĴ��ļ������ķ�װ��ͬʱ���ݲ������ش������.
        ���ļ�ģʽΪ�������ƣ��ɶ�д
************************************************************************/
FILE* XOS_OpenFile(XCONST XCHAR * filename,e_OPENFILEMODE mode)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};
    FILE  *fileptr                          = XNULLP;
    XS32  fnfix                             = 0;      /*��¼�ļ��Ƿ�����չ�������ķ���ֵ*/

    /* �ļ�·������Ч��*/
    if ( XNULL == filename )
    {
        /*���ô�����-��Ч����*/
        XOS_FILE_ERRORNO = XERRPARA;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_OpenFile ->input param null !");
        return XNULLP;
    }

    if ( mode < XF_RTMODE || mode > XF_APLUSBMODE )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_OpenFile ->input param mode is wrong!");
        return XNULLP;
    }

    if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(filename) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_OpenFile ->input filename too long !");
        return XNULLP;
    }

    /*����filename·���Ƿ��ǺϷ��ַ�
    '\0' == formatstr[0]����win�µ�""�ַ���
    */
    if( XSUCC != FS_formatpath(filename,formatstr)
        || '\0' == formatstr[0]  )
    {
        /*���ô�����-��Ч����*/
        XOS_FILE_ERRORNO = XERRPARA;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_OpenFile ->input param filename %s -> %s  error !",filename,formatstr);
        return XNULLP;
    }

    fnfix = FS_isfilenameright(filename);

    /*filename·������*/
    if ( XFPATHEXSS == fnfix )
    {
        /*���ô�����-�ļ�·������*/
        XOS_FILE_ERRORNO = XFPATHEXSS;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_OpenFile ->input filename too long !");
        return XNULLP;
    }

    /*û����չ��*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*Ϊformatstr������չ��*/
        /*��Ϊ��ǰ���FS_isfilenameright�������Ѿ����ε��ļ������������������:
        ����չ����filename ���ȴ���MAX_DIRANDFILEPATH_LEN - 5;
        ����չ����filename ���ȴ���MAX_DIRANDFILEPATH_LEN-5;
        ��������������������չ��ʱ��ֻ��������չ����filenameС��
        MAX_DIRANDFILEPATH_LEN-5�Ŀɿط�Χ�ڽ��м������ӣ����ÿ����������

        */
        /*
        XOS_StrNcpy(formatstr+(XOS_StrLen(formatstr)),
        FILE_POSTFIXNAME,
        sizeof(FILE_POSTFIXNAME));
        */
        XOS_StrNCat(formatstr,FILE_POSTFIXNAME, MAX_DIRANDFILEPATH_LEN);
    }

    if ( XNULLP == (fileptr = fopen((XCONST XCHAR*)formatstr,OPENFILEMODE[mode])) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),
            "XOS_OpenFile ->can not open the file %s !",
            filename);
        return XNULLP;
    }
    else
    {
        return fileptr;
    }

}

/************************************************************************
��������XOS_WriteFile
��  �ܣ�������д���ļ�
��  �룺ptrdata--Ҫд������ָ�룬�ǿա�
        datasize--Ҫд���������ÿ�����������ݽṹ�Ĵ�С
        datanum--���������ݽṹ�ܹ�Ҫд���ٴ�
        fileptr -- Ҫ��ȡ���ļ�ָ�룬���벻Ϊ��������ڵ��ļ���Ӧ�����򷵻ش���
��  ����
��  �أ��ɹ�����XSUCC
          ʧ�ܷ���XERROR
˵  ����datasize*datanum Ӧ�õ���ptrdata���ֽ���;Ҫ����дд����;
       Ҫ�Ľ�Ч�ʵĻ�������Ƚ�����д��������ڴ��У��ȵ�һ��ʱ����д�ļ�����û��ʵ�֣���
       �����ж�Ҫд���ļ��Ƿ��Ѿ��ﵽ�ļ�ϵͳ��Ҫ�������޶�
************************************************************************/
XS32  XOS_WriteFile(XCONST XVOID *ptrdata, XU32 datasize, XU32 datanum, FILE *fileptr)
{
    t_XOSMUTEXID wflock; /*д�ļ�����*/
    XU32         wnum;   /*��¼fwrite�ķ���ֵ*/

    /* �ļ�·������Ч��*/
    if ( XNULLP == fileptr )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile ->input fileptr null !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( XNULL == ptrdata )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile ->input dataptr  null !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( XSUCC != XOS_MutexCreate(&wflock) )
    {
        /*������ʧ��-������*/
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile ->creat lock failed !");
        return XF_LOGKFAILED;
    }

    /*��������*/
    if ( XSUCC != XOS_MutexLock(&wflock) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile -> lock failed !");
        return XF_LOGKFAILED;

    }

    wnum = (XU32)fwrite(ptrdata,datasize,datanum,fileptr);

    /*����*/
    if ( XSUCC != XOS_MutexUnlock(&wflock) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile ->unlock failed !");
        return XF_LOGKFAILED;
    }
    return wnum;
}

#if  1

/************************************************************************
��������XOS_FileLen
��  �ܣ�����ļ��Ĵ�С�ֽ���
��  �룺filename -- �ļ�����������ڣ����򷵻ش���
        ͳһ��·����ʽΪ/path/filename+��չ��
        ���û����չ������ͳһ�ļ���չ��Ϊ.txt
��  ����
��  �أ��ɹ������ļ��ֽ�����С
          ʧ�ܷ���XERROR
˵  ����
************************************************************************/
XS32 XOS_FileLen(XCONST XCHAR * filename, XUFILESIZE *filelength)
{
//#if ( XOS_WIN32 || XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_WIN32) || defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VXWORKS))
#ifdef XOS_WIN32

    struct _stat fbuf;

#endif /*XOS_WIN32*/

//#if ( XOS_LINUX || XOS_SOLARIS  )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VXWORKS))

    struct stat fbuf;

#endif /*XOS_LINUX || XOS_SOLARIS*/

    if ( XNULL == filelength )
    {
        printf("INFO:XOS_FileLen ->filelength is null !\r\n");
        return XERRPARA;
    }

    if ( XNULL == filename )
    {
        printf("INFO:XOS_FileLen ->filename is null !\r\n");
        return XERRPARA;
    }

    if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(filename) )
    {
        printf("INFO:XOS_FileLen ->input filename too long !\r\n");
        return XERRPARA;
    }

    *filelength = 0;

    /*����Getfileatt�����ĵ��ļ�������*/
    /*ȡ���ļ���С,���ֽڼ���*/

#ifdef XOS_WIN32

    if( XERROR == _stat((const char *)filename,&fbuf) )

#endif /*XOS_WIN32*/

//#if ( XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS)|| defined(XOS_VXWORKS))

        if( XERROR == stat((char *)filename,&fbuf) )

#endif /*XOS_LINUX || XOS_SOLARIS|| XOS_VXWORKS*/
        {
            printf("INFO:XOS_FileLen ->stat failed !\r\n");
            return XERROR;
        }

        *filelength = (XUFILESIZE)fbuf.st_size;
#else

        *filelength = XNULL;

#endif

        return XSUCC;

}

#endif /*1*/

/************************************************************************
��������XOS_ReadFile
��  �ܣ���ȡ�ļ��������ݵ�ָ�����ڴ���
��  �룺desdata--���ļ����ݶ�ȡ�������ڴ���ַ
        datasize--desdata��ָ���ݽṹ�Ĵ�С
        length-- desdata����ָ���ݽṹ���ܳ���
        fileptr --Ҫ��ȡ���ļ�ָ�룬���벻Ϊ��������ڵ��ļ���Ӧ�����򷵻ش���

��  ����
��  �أ��ɹ������ܹ���ȡ���ַ�����
        ʧ�ܷ���XERROR
˵  ����datasize*length����С�ڻ����desdata���ֽ������������desdata�����ֽ���
        ��desdata��ָ�ڴ�֮�������Ҳ���޸ģ���ɲ���֪����
        ���С��fileptr�ļ����ֽ���������н�ȡ
        �ú�������datasize*length�Ĵ�С������ȡ�ļ��е���������
************************************************************************/
XS32 XOS_ReadFile(XVOID *desdataptr, XU32 datasize,XU32 datanum, FILE *fileptr)
{
    /* �ļ�·������Ч��*/
    if ( XNULLP == fileptr )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ReadFile ->input para null!");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( XNULL == desdataptr )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ReadFile ->input para null!");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    /*-datasize��length�ܹ����ֽ����Ƿ�Ϸ�Ϊ0�������κβ�����ֱ�ӷ���0*/
    if ( 0 == datasize *datanum )
    {
        return datasize*datanum;
    }

    /*�����û��������������д*/
    return ( (XS32)fread(desdataptr,datasize,datanum,fileptr) );
}

/************************************************************************
��������XOS_CloseFile
��  �ܣ��ر��ļ�
��  �룺fileptr -- Ҫ�رյ��ļ�ָ�룬���벻Ϊ��������ڵ��ļ���Ӧ�����򷵻ش���
        Ӧ��Ϊ���ļ�����Ӧ���ļ�ָ��
��  ����
��  �أ��ɹ�����XSUCC
        ʧ�ܷ���XERROR
˵  ����
************************************************************************/
XS32 XOS_CloseFile(FILE **fileptr)
{
    if ( XNULLP == *fileptr )
    {
        return XERROR;
    }
    if ( 0 == fclose(*fileptr) )
    {
        *fileptr = XNULLP ;
        return XSUCC;

    }
    else
    {
        XOS_Trace(MD(FID_FILE,PL_WARN),"XOS_CloseFile ->close file error!");
        *fileptr = XNULLP;
        return XERROR;
    }
}

/************************************************************************
��������XOS_DeleteFile
��  �ܣ�ɾ���ļ�
��  �룺filename -- Ҫɾ�����ļ�������������Ҵ��ڹر�״̬�����򷵻ش���
        ͳһ��·����ʽΪ/path/filename+��չ��
        ���û����չ������ͳһ�ļ���չ��Ϊ.txt
��  ����
��  �أ��ɹ�����XSUCC
        ʧ�ܷ���XERROR
˵  ����
************************************************************************/
XS32 XOS_DeleteFile(XCONST XCHAR * filename)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};
    XS32  fnfix                             = 0;

    /* �ļ�·������Ч��*/
    if ( XNULL == filename )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteFile ->input param null !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(filename) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteFile ->input filename too long !");
        return XERRPARA;
    }

    if( XSUCC != FS_formatpath(filename,formatstr)
        || '\0' == formatstr[0] )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteFile ->input param filename %s -> %s  error !",filename,formatstr);
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    fnfix = FS_isfilenameright(filename);

    /*filename·������*/
    if ( XFPATHEXSS == fnfix )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteFile ->filename too long !");
        /*���ô�����-�ļ�·������*/
        return XFPATHEXSS;
    }

    /*û����չ��*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*Ϊformatstr������չ��*/
        /*
        XOS_StrNcpy(formatstr+(XOS_StrLen(formatstr)),FILE_POSTFIXNAME,sizeof(FILE_POSTFIXNAME));
        */
        XOS_StrNCat(formatstr,FILE_POSTFIXNAME, MAX_DIRANDFILEPATH_LEN);

    }

    if ( 0 == remove(formatstr) )
    {
        /* XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteFile ->delete file error !");*/
        return XSUCC;
    }
    else
    {
        if ( EACCES == errno )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteFile ->can not delete file  !");
            /*ֻ���ļ�*/
            return XERRMODE;
        }

        if ( ENOENT == errno )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),
                "XOS_DeleteFile ->file %s is not exist !",
                filename);
            /*�ļ�������*/
            return XERRPATH;
        }

        return XERROR;
    }
}
/************************************************************************
��������XOS_GetFileAtt
��  �ܣ�����ļ��ĸ�����
��  �룺filename -- �ļ�����������ڣ����򷵻ش���ͳһ��·����ʽΪ/path/filename+��չ��
        ���û����չ������ͳһ�ļ���չ��Ϊ.txt
        fileatt--t_FILEATT�ṹ��������¼�ļ�����������ֵ
��  ����
��  �أ��ɹ�����XSUCC
        ʧ�ܷ���XERROR,�Լ�������
˵  ����
************************************************************************/
XS32 XOS_GetFileAtt(XCONST XCHAR * filename, t_FILEATT *fileatt)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN + 1] = {0};

#if ( defined ( XOS_WIN32 ) )

    struct _stat buffer ;

#endif /*XOS_WIN32*/

//#if ( XOS_LINUX || XOS_SOLARIS || XOS_VTA )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VTA) )

    struct stat buffer ;

#endif /*XOS_LINUX || XOS_SOLARIS || XOS_VTA*/

    XS32 fnfix     =0; /*��¼�ļ��Ƿ�����չ���ķ���ֵ*/
    XS32 backvalue = 0;

#ifdef XOS_NVTA

    return XSUCC;

#else

    /* �ļ�·������Ч��*/
    if ( XNULL == filename )
    {
        XOS_CpsTrace(MD(FID_FILE,PL_ERR),"XOS_GetFileAtt ->input param null !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( XNULL == fileatt )
    {
        XOS_CpsTrace(MD(FID_FILE,PL_ERR),"XOS_GetFileAtt ->input param null !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(filename) )
    {
        XOS_CpsTrace(MD(FID_FILE,PL_ERR),"XOS_GetFileAtt ->input filename too long !");
        return XERRPARA;
    }

    if ( XSUCC != FS_formatpath(filename,formatstr)
        || '\0' == formatstr[0] )
    {
        XOS_CpsTrace(MD(FID_FILE,PL_ERR),"XOS_GetFileAtt ->input param filename %s -> %s  error !",filename,formatstr);
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    fnfix = FS_isfilenameright(filename);

    /*filename·������*/
    if ( XFPATHEXSS == fnfix )
    {
        XOS_CpsTrace(MD(FID_FILE,PL_ERR),"XOS_GetFileAtt ->input filename too long !");
        /*���ô�����-�ļ�·������*/
        return XFPATHEXSS;
    }

    /*û����չ��*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*Ϊformatstr������չ��*/
        /*
        XOS_StrNcpy(formatstr+(XOS_StrLen(formatstr)),FILE_POSTFIXNAME,sizeof(FILE_POSTFIXNAME));
        */
        XOS_StrNCat(formatstr,FILE_POSTFIXNAME, MAX_DIRANDFILEPATH_LEN);

    }

#ifdef XOS_WIN32

    backvalue = _stat(formatstr,&buffer);

#endif  /*XOS_WIN32*/

//#if ( XOS_LINUX || XOS_SOLARIS || XOS_VTA )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VTA) )

    backvalue = stat(formatstr,&buffer);

#endif  /*XOS_LINUX || XOS_SOLARIS*/

    if ( 0 == backvalue )
    {
        fileatt->f_dev    =buffer.st_dev;
        fileatt->f_gid    = buffer.st_gid;
        fileatt->f_ino    = buffer.st_ino;
        fileatt->f_latime = buffer.st_atime;
        fileatt->f_lctime = buffer.st_ctime;
        fileatt->f_mode   = buffer.st_mode;
        fileatt->f_mtime  = buffer.st_mtime;
        fileatt->f_nlink  = buffer.st_nlink;
        fileatt->f_rdev   = buffer.st_rdev;
        fileatt->f_size   = (XUFILESIZE)buffer.st_size;
        fileatt->f_uid    = buffer.st_uid;
        return XSUCC;
    }
    else
    {
        if ( ENOENT == errno )
        {
            XOS_CpsTrace(MD(FID_FILE,PL_WARN),"XOS_GetFileAtt ->get file %s att error !",filename);
            return XERRPATH;
        }

        XOS_CpsTrace(MD(FID_FILE,PL_WARN),"XOS_GetFileAtt ->get file %s  att error !",filename);
        return XERROR;
    }

#endif /* XOS_NVTA */

}

/************************************************************************
��������XOS_SetFilePtr
��  �ܣ������ļ�ָ��λ��
��  �룺fileptr -- �ļ�ָ��
        offset--ƫ����
        currentpos-��ʼλ�ã�ֵ������
        XF_SEEKCUR-�ӵ�ǰ�ļ�ָ��λ�ÿ�ʼ
        XF_SEEKSET-���ļ�ͷ��ʼ
        XF_SEEKEND---���ļ�ĩβ��ʼ

��  ����
��  �أ��ɹ�����XSUCC
        ʧ�ܷ���XERROR���Լ�������
˵  ����
************************************************************************/
 XS32 XOS_SetFilePtr(const FILE *fileptr, XS32 offset, XU32 currentpos)
 {

      if ( XNULLP == fileptr )
      {
          XOS_Trace(MD(FID_FILE,PL_WARN),"XOS_SetFilePtr ->input param null !");
          /*���ô�����-��Ч����*/
          return XERRPARA;
      }

      if ( (XF_SEEKCUR != currentpos)
          && (XF_SEEKEND != currentpos)
          && (XF_SEEKSET != currentpos) )
      {
          XOS_Trace(MD(FID_FILE,PL_WARN),"XOS_SetFilePtr ->input param error !");
           /*���ô�����-��Ч����*/
          return XERRPARA;
      }

      if ( 0 != fseek((FILE *)fileptr,offset,currentpos) )
      {
          XOS_Trace(MD(FID_FILE,PL_WARN),"XOS_SetFilePtr ->fseek operation error!");
          return XERROR;
      }

      return XSUCC;

 }

/************************************************************************
��������XOS_GetFilePtr
��  �ܣ������ļ�ָ��λ��
��  �룺fileptr -- �ļ�ָ��
��  ����
��  �أ��ɹ������ļ�ָ��λ��
        ʧ�ܷ���XERROR�Լ�������
˵  ����
 ************************************************************************/
 XS32 XOS_GetFilePtr(FILE *fileptr)
{

      if ( XNULLP == fileptr )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_GetFilePtr ->input param null !");
          /*���ô�����-��Ч����*/
          return XERRPARA;
      }

      return ( ftell(fileptr) );

}
/************************************************************************
��������XOS_MoveFile
��  �ܣ���Դ�ļ��ƶ���Ŀ���ļ���ָ��·����
��  �룺srcfilename -- Ҫ�ƶ���Դ�ļ���������ڡ����ļ��ڽ��в���֮ǰ���봦�ڹر�״̬
        desfilename--ҪĿ���ļ������ļ��Ѿ�����ʱ�����ر���������ļ������ڣ����������ļ�����
        Դ�ļ������ƶ������ļ�
��  ����
��  �أ��ɹ�����XSUCC
        ʧ�ܷ���XERROR
˵  �����ļ���ͳһ��ʽΪ:/path/filename+��չ�������û����չ��������.txt�ļ��Դ�
 ************************************************************************/
XS32 XOS_MoveFile(XCONST XCHAR * srcfilename, XCONST XCHAR * desfilename)
{
     XCHAR        MoveOrder[MAX_ORDERLEN] = {0}; /*ϵͳ������*/
     XCHAR        path[MAX_ORDERLEN]      = {0}; /*srcfile to desfile��������*/
     XCHAR        blank[] = " ";                 /**/

     /*�жϲ���*/
     FILE         *srcf                   = XNULLP;
     FILE         *desf                   = XNULLP;

//#if ( XOS_WIN32 || XOS_VTA )
#if ( defined(XOS_WIN32) || defined(XOS_VTA) )

     XCHAR MOrder[] = "MOVE ";

#endif  /*XOS_WIN32 || XOS_VTA */

//#if ( XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )

     XCHAR MOrder[] = "mv ";

#endif  /*XOS_LINUX || XOS_SOLARIS*/

     if ( XNULL == srcfilename )
     {
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->input srcfilename null !");
         /*���ô�����-��Ч����*/
         return XERRPARA;
     }

     if ( XNULL == desfilename )
     {
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->input desfilename null !");
         /*���ô�����-��Ч����*/
         return XERRPARA;
     }

     if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(srcfilename)
         || MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(desfilename) )
     {
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->input filename too long !");
         return XERRPARA;
     }

     /*�����´��һ���ո񣬿��ǵ�������������+2*/
     if(MAX_ORDERLEN < XOS_StrLen(srcfilename) + XOS_StrLen(desfilename) + 2)
     {
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->input filename too long !");
         return XERRPARA;
     }
        
     if ( XNULLP == (srcf = fopen(srcfilename,"r")) )
     {
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->srcfilename is not exist !");
         return XERRPATH;
     }
     else
     {
         fclose(srcf);
     }

     if ( XNULLP != (desf = fopen(desfilename,"r")) )
     {
         fclose(desf);
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->desile is already  exist !");
         return XERRPARA;
     }

     XOS_StrNcpy( path,srcfilename, MAX_ORDERLEN);
     XOS_StrNcpy( path + XOS_StrLen(srcfilename), blank, MAX_ORDERLEN - XOS_StrLen(srcfilename) -1);
     XOS_StrNcpy( path + XOS_StrLen(srcfilename) + XOS_StrLen(blank),
                  desfilename,
                  MAX_ORDERLEN - XOS_StrLen(srcfilename) - XOS_StrLen(blank) -1);

     if ( XSUCC != FS_CatDelorder(MoveOrder,MOrder,path) )
     {
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->order is wrong!");
         /*���ô�����*/
         return XERROR;
     }

//#if ( XOS_WIN32 || XOS_VTA || XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_WIN32) || defined(XOS_VTA) || defined(XOS_LINUX) || defined(XOS_SOLARIS) )

     system(MoveOrder);

#endif

     return XSUCC;

}

/************************************************************************
��������XOS_CopyFile
��  �ܣ���Դ�ļ�����һ�ݵ�Ŀ���ļ���Դ�ļ����ֲ���
��  �룺srcfilename --Ҫ���Ƶ�Դ�ļ���������ڡ����ļ��ڽ��в���֮ǰ���봦�ڹر�״̬
        desfilename--Ҫ���Ƶ�Ŀ���ļ������ļ��Ѿ�����ʱ�����봦�ڹر�״̬�������ݽ������ǣ�
        ������ļ������ڣ��������µ��ļ�
��  ����
��  �أ��ɹ�����XSUCC
        ʧ�ܷ���XERROR
˵  ������srcfilename�е���������д�뵽desfilename
        �У�����֮ǰ�����ļ����봦�ڹر�״̬��
        �ļ�·����ʽΪ��/path/filename+��չ����������.txt�ļ��Դ�
************************************************************************/
XS32 XOS_CopyFile(XCONST XCHAR * srcfilename, XCONST XCHAR * desfilename)
{
    XCHAR srcformatstr[MAX_DIRANDFILEPATH_LEN] = {0};
    XCHAR desformatstr[MAX_DIRANDFILEPATH_LEN] = {0};
    FILE  *srcf                                = XNULLP;
    FILE  *desf                                = XNULLP;
    XS32  fnfix                                = 0;      /*��¼�ж��ļ����Ƿ�����չ������ֵ*/
    XCHAR copyorder[MAX_ORDERLEN]              = {0};
    XCHAR copydirpath[MAX_ORDERLEN]            = {0};
    XCHAR blank[] = " ";                                 /**/
    XS32  order_len = 0;

//#if ( XOS_WIN32 || XOS_VTA  )
#if ( defined(XOS_WIN32) || defined(XOS_VTA) )

    XCHAR COrder[] = "COPY ";

#endif

//#if ( XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )

    XCHAR COrder[] = "cp ";

#endif

    if ( XNULL == srcfilename )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input param null !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( XNULL == desfilename )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input param null !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(srcfilename)
        || MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(desfilename) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input filename too long !");
        return XERRPARA;
    }

    if ( XSUCC != FS_formatpath(srcfilename,srcformatstr)
        || '\0' == srcformatstr[0]  )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input param srcfilename %s -> %s  error !",srcfilename,srcformatstr);
        return XERRPARA;
    }

    fnfix = FS_isfilenameright(srcfilename);

    /*filename·������*/
    if ( XFPATHEXSS == fnfix )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input filename too long !");
        /*���ô�����-�ļ�·������*/
        return XFPATHEXSS;
    }

#if 0

    /*û����չ��*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*Ϊformatstr������չ��*/
        XOS_StrNcpy(srcformatstr+(XOS_StrLen(srcformatstr)),FILE_POSTFIXNAME,sizeof(FILE_POSTFIXNAME));
    }

#endif

    if( XSUCC != FS_formatpath(desfilename,desformatstr)
        || '\0' == desformatstr[0]  )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),
            "XOS_CopyFile ->input param desfilename %s -> %s  error !",desfilename,desformatstr);
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    fnfix = FS_isfilenameright(desfilename);

    /*filename·������*/
    if ( XFPATHEXSS == fnfix )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input filename too long !");
        /*���ô�����-�ļ�·������*/
        return XFPATHEXSS;
    }

#if 0

    /*û����չ��*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*Ϊformatstr������չ��*/
        XOS_StrNcpy(desformatstr+(XOS_StrLen(desformatstr)),FILE_POSTFIXNAME,sizeof(FILE_POSTFIXNAME));
    }

#endif

    if (XNULLP != (desf = (fopen((XCONST XCHAR *)desformatstr,OPENFILEMODE[ XF_RTMODE ]))) )
    {
        /*Ŀ�����ļ��Ƿ��Ѿ�����*/
        fclose(desf);
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->desfile already exist !");
        /*���ô�����-��Ч����*/
        return XERRPARA;
    }

    if ( XNULLP == (srcf = (fopen((XCONST XCHAR *)srcformatstr,OPENFILEMODE[ XF_RBMODE ]))) )
    {
        /*Դ�ļ�������*/
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->srcfile not exist !");
        /*���ô�����-��Ч����*/
        return XERRPATH;
    }

    fclose(srcf);
    order_len = (XS32)XOS_StrLen(copydirpath)
        + (XS32)XOS_StrLen(srcformatstr)
        + (XS32)XOS_StrLen(blank)
        + (XS32)XOS_StrLen(desformatstr);

    if ( MAX_ORDERLEN -1 <= order_len )
    {
        return XERROR;
    }

    /*�����Ѿ��ж���Ҫ�������ַ����ĳ��ȣ����Բ��ÿ��ǻ����������*/
    XOS_StrNcpy( copydirpath,srcformatstr, XOS_StrLen(srcformatstr) );
    XOS_StrNcpy( copydirpath+XOS_StrLen(srcformatstr),blank,XOS_StrLen(blank) );
    XOS_StrNcpy( copydirpath+XOS_StrLen(srcformatstr)+XOS_StrLen(blank),
        desformatstr,
        XOS_StrLen(desformatstr) );

    if ( XSUCC != FS_CatDelorder(copyorder,COrder,copydirpath) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->order is wrong. order is too long or order is null!");
        /*���ô�����*/
        return XERROR;
    }

//#if ( XOS_WIN32 || XOS_VTA || XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_WIN32) || defined(XOS_VTA) || defined(XOS_LINUX) || defined(XOS_SOLARIS) )

    system(copyorder);

#endif

#if 0

    /*׼����д�Ĳ���-���ļ�*/
    if ( XNULL == (srcf = fopen(srcformatstr,OPENFILEMODE[XF_RTMODE])) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->can not open srcfile !");
        return XERROR;
    }

    if ( XNULL == (desf = fopen(desformatstr,OPENFILEMODE[XF_ATMODE])) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->can not open decfile !");
        return XERROR;
    }

    if ( XSUCC != XOS_MutexCreate(&cflock) )
    {
        /*������ʧ��-������*/
        XOS_FILE_ERRORNO = XF_LOGKFAILED;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->creat lock failed!");
        return XERROR;
    }

    /*��������*/
    if ( XSUCC != XOS_MutexLock(&cflock) )
    {
        /*����ʧ��*/
        XOS_FILE_ERRORNO = XF_LOGKFAILED;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->lock failed!");
        return XERROR;

    }

    /*ѭ�����ļ�֪����������д����һ��Ŀ���ļ���*/
    while ( EOF != (tmpfilebuffer = fgetc(srcf)) )
    {
        fputc(tmpfilebuffer,desf);
    }

    fclose(srcf);
    fclose(desf);

    /*��������*/
    if ( XSUCC != XOS_MutexUnlock(&cflock) )
    {
        /*����ʧ��*/
        XOS_FILE_ERRORNO = XF_LOGKFAILED;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->unlock failed!");
        return XERROR;

    }

#endif

    return XSUCC;

}

/************************************************************************
��������XOS_IsExistDir
��  �ܣ��鿴dirnameĿ¼�Ƿ��Ѿ������� Dirname��ʽΪ��/exm/dirname��
��  �룺dirname-Ҫ�鿴��Ŀ¼����ͳһ��·����ʽΪ/path/dirname

��  ����
��  �أ����ڷ���XSUCC
        �����ڷ���XERROR
˵  ����
  ************************************************************************/
  XS32 XOS_IsExistDir(XCONST XCHAR * dirname)
  {

      XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};
      XCHAR currentdir[MAX_DIRANDFILEPATH_LEN] = {0}; /*��¼���øú���ʱ�Ĺ���Ŀ¼��������ɺ󷵻ظù���Ŀ¼*/

      XOS_GetCurrentWorkDir(currentdir,MAX_DIRANDFILEPATH_LEN);

      /* Ŀ¼����Ч��*/
      if ( XNULL == dirname )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_IsExistDir ->input param null!");
          /*���ô�����-��Ч����*/
          return XERRPARA;
      }

      if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(dirname) )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_IsExistDir ->input dirname too long !");
          return XERRPARA;
      }

      if( XSUCC != FS_formatpath(dirname,formatstr)
          || '\0' == formatstr[0]   )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_IsExistDir ->input param dirname %s -> %s  error !",dirname,formatstr);
          /*���ô�����-��Ч����*/
          return XERRPARA;
      }

      /*���� ����Ŀ¼��ת��*/
      if ( XSUCC != XOS_MutexCreate(&mutexlock_file_dir.changdir_lock) )
      {
          /*������ʧ��*/
          return XERROR;
      }

      /*��������*/
      if ( XSUCC != XOS_MutexLock(&mutexlock_file_dir.changdir_lock) )
      {
          return XERROR;
      }

      /*��Ŀ¼�Ѿ�����*/
      if ( 0 == chdir(formatstr) )
      {
          /*������ǰ�Ĺ���Ŀ¼*/
          chdir(currentdir);

          /*����*/
          if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
          {
              return XERROR ;
          }

          return XSUCC;

      }
      else
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_IsExistDir ->check exist dirpath failed!");

          /*����*/
          if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
          {
              return XERROR ;
          }

          return XERROR;

      }

}

/************************************************************************
��������XOS_GetCurrentWorkDir
��  �ܣ��鿴��ǰ�û����ڵĹ���Ŀ¼·��

��  ����
��  �أ��ɹ����ع���·��
        ʧ�ܷ���NULL
˵  ����
************************************************************************/
XVOID XOS_GetCurrentWorkDir (XCHAR *currentdir,XU32 maxlen)
{
      if ( XNULL == currentdir )
      {
          XOS_Trace(MD(FID_FILE,PL_WARN),"XOS_GetCurrentWorkDir  ->input para is null!");
          return;
      }

      if ( MAX_DIRANDFILEPATH_LEN <= maxlen)
      {
          getcwd(currentdir,MAX_DIRANDFILEPATH_LEN);
          return;
      }
      else
      {
          getcwd(currentdir,maxlen);
          return;
      }

}

/************************************************************************
��������XOS_GetCurrDiskNO
��  �ܣ���ȡ��ǰ����Ӳ�̵���š�
��  �룺
��  ����
��  �أ��ɹ�����Ӳ�����
        ʧ�ܷ���XERROR�Լ�������
˵  ����
************************************************************************/
XS8 XOS_GetCurrDiskNO(XVOID)
{
#if ( (defined ( XOS_WIN32 )) )

      return ( _getdrive() );

#endif

#if ( defined ( XOS_LINUX ) || defined ( XOS_SOLARIS ) || ( defined ( XOS_VTA )) )

      return XSUCC;

#else

      XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_GetCurrDiskNO ->not defined system macro!");
      return XF_PLATEXPERROR;

#endif

}

/************************************************************************
��������XOS_CurrDiskFreeCap
��  �ܣ���ȡ��ǰ����Ӳ�̵�ʣ����ÿռ�������
��  �룺driveno-��ǰ����Ӳ�����
        Cap -Ҫ���ص�ʣ��ռ�K
��  ����
��  �أ��ɹ�����XSUCC
        ʧ�ܷ���XERROR
˵  ����
************************************************************************/
XS32 XOS_CurrDiskFreeCap(XCONST XU8 driveno,XU32 *cap)
{

#if ( (defined ( XOS_WIN32 )) )

      struct _diskfree_t  diskfree;

#endif

      if ( XNULL == cap )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CurrDiskFreeCap ->input capptr null!");
          return XERROR;
      }

      if ( 0 == driveno )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CurrDiskFreeCap ->input drive num error!");
          return XERROR;
      }

#if ( (defined ( XOS_WIN32 ))  )

      if ( 0 != _getdiskfree((XU32)_getdrive(),&diskfree) )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CurrDiskFreeCap ->getdrive function  error!");
          return XERROR;
      }
      else
      {
          *cap = diskfree.avail_clusters
                * diskfree.sectors_per_cluster / 1024
                * diskfree.bytes_per_sector;

          return XSUCC;
      }

#endif

#if ( (defined ( XOS_LINUX )) || (defined ( XOS_SOLARIS )) || (defined ( XOS_VTA ))  )

      return XSUCC;

#else

      XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CurrDiskFreeCap ->not defined systme macro!");
      return XF_PLATEXPERROR;

#endif

}

/**********************************************************
��  �ܣ�1. �ж������ļ�·��������ɸ�ʽ�Ƿ���ȷ��·��Ϊ���֣��ַ����»�����ɡ�
        2.ת��Ϊ��ͬϵͳ��ת����·����ʽ(������Բ�ת����win�¼���/../��ʽ)
        3.����win��vxworks�µľ���·���ж�,��ʽΪ�̷�:(/)�ļ���(�ļ�)/...
        4:linux��solairs�µĸ�ʽ�ü�����֤
��  �أ����봮���󷵻�XERROR������ת������������XSUCC
***********************************************************/
XS32 FS_formatpath(XCONST XCHAR *srcpathstr,XCHAR *formatpathstr)
{
    XU32   cyc    = 0;                                 /*ѭ������*/
    XU32   despos = 0;                                 /**/
    XU32 ulLen = 0;
#if 0  /*del by lixiangni 2007.1.6 */
    XCHAR  file_ab_name[MAX_DIRANDFILEPATH_LEN] = {0};

    if ( XSUCC !=  Trace_abFileName(srcpathstr, file_ab_name, MAX_DIRANDFILEPATH_LEN-1) )
    {
        return XERROR ;
    }
    else
    {
        /*����ļ�����Ŀ¼��Ϊ���ַ���������Ϊ���󷵻ط�ֹ�����쳣*/
        if ( '\0' == file_ab_name[0] || 0 == file_ab_name[0] )
        {
            return XERROR ;
        }
    }

#endif
    ulLen = (XU32)strlen(srcpathstr);/*��ֹ�����������Խ��*/
    while (*(srcpathstr+cyc) != '\0'
           && *(srcpathstr+cyc) != 0 && cyc < ulLen)
    {
        if ( despos >= MAX_DIRANDFILEPATH_LEN -2 )
        {
            break;
        }

        /*֧������·��, bruce add 2012.1.31*/
        if(*(srcpathstr+cyc) < 0)
        {
            if(*(srcpathstr+cyc+1) != '\0'
               && *(srcpathstr+cyc+1) != 0 && cyc+1 < ulLen)
            {
                if(*(srcpathstr+cyc+1) < 0)
                {
                    *(formatpathstr+despos) = *(srcpathstr+cyc) ;
                    *(formatpathstr+despos+1) = *(srcpathstr+cyc+1) ;    
                    cyc +=2;
                    despos +=2;
                }
                else
                {
                    return XERROR;
                }
            }
            else
            {
                return XERROR;
            }           
            
        }
        else
        {
        /*win��vxworks�´������·��*/
#if ( (defined ( XOS_WIN32 )) || (defined( XOS_VTA )) )

#if 0
        /*�ж��Ƿ�Ϊ����·��*/
        if ( 1 == cyc )
        {
            /*����һ�����̷���:�ŵ��ж�*/
            if (( ':' != *(srcpathstr+cyc) )
                && (!isalnum(*(srcpathstr+cyc)) )
                && ('_' != *(srcpathstr+cyc))
                &&  ('/' != *(srcpathstr+cyc))
                &&      ('\\' != *(srcpathstr+cyc))
                && ('.' != *(srcpathstr+cyc)) )
            {

                return XERROR;
            }
        }
#endif

        if ( '/' == *(srcpathstr+cyc) )
        {

            /*��ԭ�ַ����е�/ת��Ϊ\*/
            *(formatpathstr+despos) = '\\';
        }
    
        else
        {
            if (isalnum((int )(*(srcpathstr+cyc)))
                || '_'==*(srcpathstr+cyc)
                ||('.' == *(srcpathstr+cyc))
                || ( ':' == *(srcpathstr+cyc) )
                || ( '\\' == *(srcpathstr+cyc) )  ) /**/
            {
                *(formatpathstr+despos) = *(srcpathstr+cyc) ;
            }
            else
            {
                break;
            }
        }

#endif /*defined (WIN32))||(defined(WIN2K)) ||(defined(,XOS_VTA)*/

#if ( (defined ( XOS_LINUX )) || (defined ( XOS_SOLARIS )) )

         /*���ַ�,�»���,���������Լ�"/" */
        if ((!isalnum((int )(*(srcpathstr+cyc))))
            &&( '_' !=*(srcpathstr+cyc))
            && ('/' != *(srcpathstr+cyc))
            && ('.' != *(srcpathstr+cyc)) )
        {
            return XERROR;
        }

        *(formatpathstr+despos) = *(srcpathstr+cyc) ;

#endif /*defined (LINUX))|| (defined (SOLARIS*/
    
        despos++;
        cyc++;
        }

    }

    if ( '\0' == *(srcpathstr+cyc)
        ||  0 == *(srcpathstr+cyc) )
    {
        /*Ŀ���ַ�����β*/
        *(formatpathstr+despos) ='\0';

        /*����ļ�����Ŀ¼��Ϊ���ַ���������Ϊ���󷵻ط�ֹ�����쳣*/

        if ( '\0' == formatpathstr[0] || 0 == formatpathstr[0] )
        {
            return XERROR ;

        }

        return XSUCC;

    }
    else
    {
        return XERROR;

    }

}

/**********************************************************

��  �ܣ��ж��ļ�·����û���ļ���չ����
        ���û���򷵻�û����չ����־��
        �к�׺������ֵΪXSUCC

***********************************************************/
XS32 FS_isfilenameright(XCONST XCHAR *filename)
{
#if 0
      XU32 cyc = 0;

      while(  (*(filename+cyc) != '\0') )
      {
          if ( (*(filename+cyc) == '.') && isalpha(*(filename+cyc+1)) )
          {
              break;
          }

          cyc++;

      }

      if ( cyc >= MAX_DIRANDFILEPATH_LEN - sizeof(FILE_POSTFIXNAME) )
      {
          /*·������*/
          return XFPATHEXSS;

      }

      if ( *(filename+cyc) == '\0' )
      {
          /*û����չ��*/
          return XF_NOFIXNAME;

      }

#endif

      return XSUCC;

}

/**********************************************************

��  �ܣ�����ͬ����ϵͳ�µ��������Լ������ϲ�Ϊdesstr��ʽ���Ա���ϵͳ����

**********************************************************/
XS32 FS_CatDelorder(XCHAR *desstr,XCONST XCHAR *order,XCONST XCHAR *path)
{

      if ( XNULL == desstr )
      {
          return XERROR;
      }

      if ( XNULL == order )
      {
          return XERROR;
      }

      if ( XNULL == path )
      {
          return XERROR;
      }

      if ( MAX_ORDERLEN-2 <= XOS_StrLen(order) + XOS_StrLen(path) )
      {
          return XERROR;
      }
      XOS_StrNcpy(desstr,order,XOS_StrLen(order));

      /*����XOS_StrNCpy������*/

      XOS_StrNCat(desstr, path, XOS_StrLen(path));

      return XSUCC;
}

/**********************************************************

��  �ܣ��жϴ����Ŀ¼�����Ŀ¼���Ǿ���Ŀ¼
����ֵ��0--���Ǿ���·��
        1--�Ǿ���·��
        -1--�������

**********************************************************/

XS32 FS_IsAbsolOrRelaDir(XCONST XCHAR *dir)
{
      if ( XNULL == dir )
      {
          return XERROR;
      }

//#if ( XOS_WIN32 || XOS_VTA )
#if ( defined(XOS_WIN32) || defined(XOS_VTA) )

      if ( 2 >= XOS_StrLen(dir) )
      {
          return 0;
      }

      /*��·�����ȴ���2���쿴�Ƿ��Ǿ���·��*/
      if ( ':' == dir[1] &&
          ( ('a' <= dir[0] && dir[0] <= 'z' ) ||('A' <= dir[0] && dir[0] <= 'Z') ) )
      {
          return 1;
      }
      else
      {
          return 0;
      }

#endif /*win32/vta*/

//#if ( XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )

      /*��ʱ�жϵĲ�׼ȷ*/
      if ( '/' == dir[0] )
          return 1;
      else
          return 0;

#endif /*linux/solaris*/

  }

#ifdef __cplusplus

}

#endif /* __cplusplus */


