/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosfilesys.c
**
**  description:  对win2000/turbolinux/solaris/vxworks下的文件和目录操作进行封装
    对于不同的操作系统利用不同的宏屏蔽
    XOS_WIN32----window
    XOS_LINUX-----linux
    XOS_SOLARIS---solaris
    XOS_VXWORKS---vxworks
    XOS_VTA------vxworks中的vta系统
    XOS_NVTA-----vxworks中的非vta系统
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
                 头文件
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
  t_XOSMUTEXID changdir_lock;               /*目录转移锁*/
  t_XOSMUTEXID file_lock;                   /*文件的锁包括文件写，文件移动等*/
}file_mutex_lock;

XSTATIC file_mutex_lock mutexlock_file_dir; /*文件目录互斥锁结构*/

/*----------------------------------------------------------*
本地全局变量定义
*------------------------------------------------------------*/
XSTATIC XS32 XOS_FILE_ERRORNO;
XSTATIC XCHAR *OPENFILEMODE[]                          = { "r","w","a","rb","wb","ab","r+","w+","a+","rb+","wb+","ab+" };
XSTATIC XCHAR FILE_POSTFIXNAME[5]                      = ".txt";
XSTATIC XCHAR CURRENTWORK_PATH[MAX_DIRANDFILEPATH_LEN] = {0};

/*---------------------------------------------------------*
内部函数声明
*----------------------------------------------------------*/

/**********************************************************

功  能：判断输入文件路径串是否正确,如果路径组成合法则在不同的系统下转化为不同的格式
        输入串错误返回XERROR，否则转化串，并返回XSUCC

***********************************************************/

XS32 FS_formatpath(XCONST XCHAR *srcpathstr,XCHAR *formatpathstr);

/**********************************************************

功  能：测试输入的文件路径中文件是否包含扩展名，
        如果输入的文件名没有扩展名，则返回提示没有扩展名

**********************************************************/
XS32 FS_isfilenameright(XCONST XCHAR *filename);

/**********************************************************

功  能：将不同操作系统下的命令行以及参数合并为desstr形式，以便于系统调用

**********************************************************/
XS32 FS_CatDelorder(XCHAR *desstr,XCONST XCHAR *order,XCONST XCHAR *path);

/**********************************************************

功  能：判断传入的目录是相对目录还是绝对目录

**********************************************************/
XS32 FS_IsAbsolOrRelaDir(XCONST XCHAR *dir);

/*---------------------------------------------------------------*
文件管理模块软件接口
*----------------------------------------------------------------*/

/************************************************************************

函数名：FS_Init
功  能：对文件管理模块进行初始化
输  入：
输  出：
返  回：
说  明：这里主要是对文件管理模块中的全局变量进行初始化
包  括：系统默认工作目录等

************************************************************************/
 XVOID FS_Init(XVOID)
{
//#if ( XOS_WIN32 )
#ifdef XOS_WIN32

     /*得到系统当前工作目录(绝对路径)*/
     _getcwd(CURRENTWORK_PATH,MAX_DIRANDFILEPATH_LEN);

#endif  /*XOS_WIN32 || XOS_VTA*/

//#if ( XOS_LINUX || XOS_SOLARIS || XOS_VTA )
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_VTA) )

     getcwd(CURRENTWORK_PATH,MAX_DIRANDFILEPATH_LEN);

#endif  /*XOS_LINUX || XOS_SOLARIS*/

    return ;

}

/*---------------------------------------------------------*
文件和目录操作对外接口函数实现
*----------------------------------------------------------*/

/************************************************************************
函数名：XOS_CreatDir
功  能：建立文件
输  入：dirname--要创建的目录路径名。不能和已经存在的目录路径名重复，否则出错。
        统一格式为：/path/dirname
输  出：
返  回：成功返回XSUCC
        失败返回XERROR,并返回XERRPARA(无效参数)和XERRPATH(无效目录路径)
说  明：
************************************************************************/
XS32 XOS_CreatDir(XCONST XCHAR * dirname)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};
#ifdef XOS_VXWORKS
#ifdef XOS_VTA
    XCHAR vxorder[MAX_DIRANDFILEPATH_LEN]   = {0}; /*vxwork下调用标准的dos命令*/
#endif
#endif
    XS32  recordbackvalue                   = 1;

    /*记录调用该函数时的绝对工作目录，动作完成后返回该工作目录*/
    XCHAR currentdir[MAX_DIRANDFILEPATH_LEN] = {0};

    XOS_GetCurrentWorkDir(currentdir,MAX_DIRANDFILEPATH_LEN);

    /* 目录的有效性*/
    if ( XNULL == dirname )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input param null !");
        return XERRPARA;
    }

    /*判断路径是否超长*/
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

    /*加锁 互斥目录的转移*/
    if ( XSUCC != XOS_MutexCreate(&mutexlock_file_dir.changdir_lock) )
    {
        /*创建锁失败*/
        return XERROR;
    }

    /*加锁互斥*/
    if ( XSUCC != XOS_MutexLock(&mutexlock_file_dir.changdir_lock) )
    {
        return XERROR;
    }

    recordbackvalue = chdir(formatstr);

    /*该目录已经存在*/
    if ( 0 == recordbackvalue )
    {
        chdir(currentdir);

        /*解锁*/
        if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
        {
            return XERROR ;
        }
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input dirpath is exist !");
        return XERRPATH;
    }
    else
    {
        /*创建目录*/
#ifdef  XOS_WIN32

        if ( _mkdir(formatstr) < 0 )
        {
            if ( EEXIST  == errno )
            {
                /*解锁*/
                if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
                {
                    return XERROR ;
                }

                XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input dirpath is exist !");
                return XF_EXIST;
            }

            if ( ENOENT == errno )
            {
                /*解锁*/
                if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
                {
                    return XERROR ;
                }

                XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CreatDir ->input dirpath not find !");
                /*路径未找到*/
                return XERRPATH;
            }
        }

#endif /*win32*/

#if ( defined (XOS_LINUX) || defined (XOS_SOLARIS) )

        /*创建用户可读写执行的文件夹*/
        if( mkdir(formatstr,S_IRWXU) < 0 )
        {
            /*解锁*/
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

        /*如果vxworks下实现dos标准的文件目录操作，则调用标准命令*/
#ifdef XOS_VXWORKS
#ifdef XOS_VTA

        FS_CatDelorder(vxorder,"MD ",formatstr);
        system(vxorder);

#endif /*vta*/
#endif /*XOS_VXWORKS*/

        /*程序创建目录完成，恢复调用函数现场*/
        chdir(currentdir);

        /*解锁*/
        if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
        {
            return XERROR ;
        }

        return XSUCC;
    }

}

/************************************************************************
函数名：XOS_ChangCurrentDir
功  能：建立文件
输  入：dirpath -- 要进入的工作目录，必须存在，否则返回错误，统一的路径格式为/path/dirname
输  出：
返  回：成功返回XSUCC
          失败返回XERROR
说  明：
************************************************************************/
XS32 XOS_ChangeCurrentDir(XCONST XCHAR *dirpath)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};

    /* 目录的有效性*/
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
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( 0 == chdir(formatstr) )
    {
        return XSUCC;
    }
    else
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ChangeCurrentDir ->chdir(%s) error!",formatstr);
        /*设置错误码-无效目录路径*/
        return XERRPATH;
    }

}

/************************************************************************
函数名：XOS_DeleteDir
功  能：删除目录
输  入：dirname -- 要删除的目录路径名，必须存在，否则返回错误，统一的路径格式为/path/dirname

输  出：
返  回：成功返回XSUCC
          失败返回XERROR
说  明：
************************************************************************/
XS32 XOS_DeleteDir(XCONST XCHAR * dirname,e_DELDIRMODE delmode)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN]  = {0};
    XCHAR expdelorder[MAX_ORDERLEN]          = {0};         /*如果参数是EXPDEL的话，记录删除目录以及其参数的命令行*/
#if ( (defined ( XOS_WIN32 )) || (defined ( XOS_VTA )) )
    XCHAR windeldir[]                        = "RD /S /Q "; /*windows下的绝对删除目录命令*/
#endif

#if (defined ( XOS_LINUX )) || (defined ( XOS_SOLARIS ))
    XCHAR linuxdeldir[]                      = "rm -rf ";   /*linux下绝对删除目录的命令对列出的所有目录递归删除*/
#endif
#ifdef XOS_SOLARIS
    XCHAR solarisdeldir[]                    = "rmdir ";    /*solairs下一般删除目录命令*/
#endif

    /* 目录的有效性*/
    if ( XNULL == dirname )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->input dirpath null !");
        /*设置错误码-无效参数*/
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
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( (EXPDEL != delmode) && (NORMALDEL != delmode) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),
            "XOS_DeleteDir ->input delmode %d error !",
            delmode);
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( NORMALDEL == delmode )
    {
#if ( defined (XOS_SOLARIS) )

        /*将要删除的目录以及命令行合并*/
        if ( XSUCC != FS_CatDelorder(expdelorder,solarisdeldir,dirname) )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->del dir in solaris wrong!");
            /*设置错误码*/
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
            /*根据系统返回的错误码输出*/
            if ( ENOTEMPTY == errno )
            {
                XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->delete dirpath not empty !");
                /*设置错误码-目录非空*/
                return XDIREMPTY;
            }

            if ( ENOENT == errno )
            {
                XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->delete dirpath not exist !");
                /*目录不存在*/
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
        /*按照参数绝对删除该目录以及该目录下的所有文件夹和文件*/
        /*vxworks下按照标准dos文件系统操作*/
#if ( (defined ( XOS_WIN32 )) || (defined ( XOS_VTA )) )

        if ( XSUCC != FS_CatDelorder(expdelorder,windeldir,formatstr) )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->expdel dir error !");
            /*设置错误码*/
            return XERRPATH;
        }

#endif
#if ( (defined ( XOS_LINUX )) || (defined ( XOS_SOLARIS )) )

        /*将要删除的目录以及命令行合并*/
        if ( XSUCC != FS_CatDelorder(expdelorder,linuxdeldir,dirname) )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteDir ->expdel dir error !");
            /*设置错误码*/
            return XERRPATH;
        }

#endif /*XOS_LINUX || XOS_SOLARIS*/

//#if ( XOS_LINUX || XOS_WIN32 || XOS_VTA || XOS_SOLARIS )
#if ( defined(XOS_LINUX) || defined(XOS_WIN32) || defined(XOS_VTA) || defined(XOS_SOLARIS) )

        /*调用系统的删除目录操作*/
           system(expdelorder);

#endif /* vxworks*/

        return XSUCC;

    }

    return XSUCC;
}

/************************************************************************
函数名：XOS_OpenFile
功  能：建立文件
输  入：filename -- 要打开的文件名，不能有重名的文件，否则返回错误，
        统一的路径格式为/path/filename+扩展名
        如果没有扩展名，则统一文件扩展名为.txt
输  出：
返  回：成功返回指向该文件的文件指针
          失败返回NULL
说  明：对各个系统下的打开文件函数的封装，同时根据操作返回错误参数.
        打开文件模式为：二进制，可读写
************************************************************************/
FILE* XOS_OpenFile(XCONST XCHAR * filename,e_OPENFILEMODE mode)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};
    FILE  *fileptr                          = XNULLP;
    XS32  fnfix                             = 0;      /*记录文件是否有扩展名函数的返回值*/

    /* 文件路径的有效性*/
    if ( XNULL == filename )
    {
        /*设置错误码-无效参数*/
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

    /*输入filename路径是否是合法字符
    '\0' == formatstr[0]控制win下的""字符串
    */
    if( XSUCC != FS_formatpath(filename,formatstr)
        || '\0' == formatstr[0]  )
    {
        /*设置错误码-无效参数*/
        XOS_FILE_ERRORNO = XERRPARA;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_OpenFile ->input param filename %s -> %s  error !",filename,formatstr);
        return XNULLP;
    }

    fnfix = FS_isfilenameright(filename);

    /*filename路径过长*/
    if ( XFPATHEXSS == fnfix )
    {
        /*设置错误码-文件路径过长*/
        XOS_FILE_ERRORNO = XFPATHEXSS;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_OpenFile ->input filename too long !");
        return XNULLP;
    }

    /*没有扩展名*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*为formatstr加上扩展名*/
        /*因为在前面的FS_isfilenameright函数中已经屏蔽掉文件名过长的情况，包括:
        有扩展名，filename 长度大于MAX_DIRANDFILEPATH_LEN - 5;
        无扩展名，filename 长度大于MAX_DIRANDFILEPATH_LEN-5;
        两种情况，所以这里加扩展名时，只需在无扩展名且filename小于
        MAX_DIRANDFILEPATH_LEN-5的可控范围内进行加入连接，不用考虑溢出问题

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
函数名：XOS_WriteFile
功  能：将数据写入文件
输  入：ptrdata--要写得数据指针，非空。
        datasize--要写入的数据中每个单独的数据结构的大小
        datanum--对上述数据结构总共要写多少次
        fileptr -- 要读取的文件指针，必须不为空且与存在的文件对应，否则返回错误。
输  出：
返  回：成功返回XSUCC
          失败返回XERROR
说  明：datasize*datanum 应该等于ptrdata的字节数;要考虑写写互斥;
       要改进效率的话，最好先将内容写进申请的内存中，等到一定时间再写文件（还没有实现）；
       不能判断要写的文件是否已经达到文件系统所要求的最大限度
************************************************************************/
XS32  XOS_WriteFile(XCONST XVOID *ptrdata, XU32 datasize, XU32 datanum, FILE *fileptr)
{
    t_XOSMUTEXID wflock; /*写文件的锁*/
    XU32         wnum;   /*记录fwrite的返回值*/

    /* 文件路径的有效性*/
    if ( XNULLP == fileptr )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile ->input fileptr null !");
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( XNULL == ptrdata )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile ->input dataptr  null !");
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( XSUCC != XOS_MutexCreate(&wflock) )
    {
        /*创建锁失败-错误码*/
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile ->creat lock failed !");
        return XF_LOGKFAILED;
    }

    /*加锁互斥*/
    if ( XSUCC != XOS_MutexLock(&wflock) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile -> lock failed !");
        return XF_LOGKFAILED;

    }

    wnum = (XU32)fwrite(ptrdata,datasize,datanum,fileptr);

    /*解锁*/
    if ( XSUCC != XOS_MutexUnlock(&wflock) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_WriteFile ->unlock failed !");
        return XF_LOGKFAILED;
    }
    return wnum;
}

#if  1

/************************************************************************
函数名：XOS_FileLen
功  能：获得文件的大小字节数
输  入：filename -- 文件名，必须存在，否则返回错误，
        统一的路径格式为/path/filename+扩展名
        如果没有扩展名，则统一文件扩展名为.txt
输  出：
返  回：成功返回文件字节数大小
          失败返回XERROR
说  明：
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

    /*利用Getfileatt函数的到文件的属性*/
    /*取得文件大小,以字节计算*/

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
函数名：XOS_ReadFile
功  能：读取文件所有内容到指定的内存中
输  入：desdata--将文件内容读取后放入的内存块地址
        datasize--desdata所指数据结构的大小
        length-- desdata中所指数据结构的总长度
        fileptr --要读取的文件指针，必须不为空且与存在的文件对应，否则返回错误。

输  出：
返  回：成功返回总共读取的字符个数
        失败返回XERROR
说  明：datasize*length必须小于或等于desdata的字节数，如果大于desdata的总字节数
        则将desdata所指内存之后的内容也将修改，造成不可知错误。
        如果小于fileptr文件的字节数，则进行截取
        该函数根据datasize*length的大小决定读取文件中的数据内容
************************************************************************/
XS32 XOS_ReadFile(XVOID *desdataptr, XU32 datasize,XU32 datanum, FILE *fileptr)
{
    /* 文件路径的有效性*/
    if ( XNULLP == fileptr )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ReadFile ->input para null!");
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( XNULL == desdataptr )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_ReadFile ->input para null!");
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    /*-datasize和length总共的字节数是否合法为0不进行任何操作，直接返回0*/
    if ( 0 == datasize *datanum )
    {
        return datasize*datanum;
    }

    /*读不用互斥包括读读，读写*/
    return ( (XS32)fread(desdataptr,datasize,datanum,fileptr) );
}

/************************************************************************
函数名：XOS_CloseFile
功  能：关闭文件
输  入：fileptr -- 要关闭的文件指针，必须不为空且与存在的文件对应，否则返回错误。
        应该为打开文件所对应的文件指针
输  出：
返  回：成功返回XSUCC
        失败返回XERROR
说  明：
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
函数名：XOS_DeleteFile
功  能：删除文件
输  入：filename -- 要删除的文件名，必须存在且处于关闭状态，否则返回错误，
        统一的路径格式为/path/filename+扩展名
        如果没有扩展名，则统一文件扩展名为.txt
输  出：
返  回：成功返回XSUCC
        失败返回XERROR
说  明：
************************************************************************/
XS32 XOS_DeleteFile(XCONST XCHAR * filename)
{
    XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};
    XS32  fnfix                             = 0;

    /* 文件路径的有效性*/
    if ( XNULL == filename )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteFile ->input param null !");
        /*设置错误码-无效参数*/
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
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    fnfix = FS_isfilenameright(filename);

    /*filename路径过长*/
    if ( XFPATHEXSS == fnfix )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_DeleteFile ->filename too long !");
        /*设置错误码-文件路径过长*/
        return XFPATHEXSS;
    }

    /*没有扩展名*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*为formatstr加上扩展名*/
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
            /*只读文件*/
            return XERRMODE;
        }

        if ( ENOENT == errno )
        {
            XOS_Trace(MD(FID_FILE,PL_ERR),
                "XOS_DeleteFile ->file %s is not exist !",
                filename);
            /*文件不存在*/
            return XERRPATH;
        }

        return XERROR;
    }
}
/************************************************************************
函数名：XOS_GetFileAtt
功  能：获得文件的各属性
输  入：filename -- 文件名，必须存在，否则返回错误，统一的路径格式为/path/filename+扩展名
        如果没有扩展名，则统一文件扩展名为.txt
        fileatt--t_FILEATT结构，用来记录文件的所有属性值
输  出：
返  回：成功返回XSUCC
        失败返回XERROR,以及错误码
说  明：
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

    XS32 fnfix     =0; /*记录文件是否有扩展名的返回值*/
    XS32 backvalue = 0;

#ifdef XOS_NVTA

    return XSUCC;

#else

    /* 文件路径的有效性*/
    if ( XNULL == filename )
    {
        XOS_CpsTrace(MD(FID_FILE,PL_ERR),"XOS_GetFileAtt ->input param null !");
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( XNULL == fileatt )
    {
        XOS_CpsTrace(MD(FID_FILE,PL_ERR),"XOS_GetFileAtt ->input param null !");
        /*设置错误码-无效参数*/
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
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    fnfix = FS_isfilenameright(filename);

    /*filename路径过长*/
    if ( XFPATHEXSS == fnfix )
    {
        XOS_CpsTrace(MD(FID_FILE,PL_ERR),"XOS_GetFileAtt ->input filename too long !");
        /*设置错误码-文件路径过长*/
        return XFPATHEXSS;
    }

    /*没有扩展名*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*为formatstr加上扩展名*/
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
函数名：XOS_SetFilePtr
功  能：设置文件指针位置
输  入：fileptr -- 文件指针
        offset--偏移量
        currentpos-开始位置，值包括：
        XF_SEEKCUR-从当前文件指针位置开始
        XF_SEEKSET-从文件头开始
        XF_SEEKEND---从文件末尾开始

输  出：
返  回：成功返回XSUCC
        失败返回XERROR，以及错误码
说  明：
************************************************************************/
 XS32 XOS_SetFilePtr(const FILE *fileptr, XS32 offset, XU32 currentpos)
 {

      if ( XNULLP == fileptr )
      {
          XOS_Trace(MD(FID_FILE,PL_WARN),"XOS_SetFilePtr ->input param null !");
          /*设置错误码-无效参数*/
          return XERRPARA;
      }

      if ( (XF_SEEKCUR != currentpos)
          && (XF_SEEKEND != currentpos)
          && (XF_SEEKSET != currentpos) )
      {
          XOS_Trace(MD(FID_FILE,PL_WARN),"XOS_SetFilePtr ->input param error !");
           /*设置错误码-无效参数*/
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
函数名：XOS_GetFilePtr
功  能：设置文件指针位置
输  入：fileptr -- 文件指针
输  出：
返  回：成功返回文件指针位置
        失败返回XERROR以及错误码
说  明：
 ************************************************************************/
 XS32 XOS_GetFilePtr(FILE *fileptr)
{

      if ( XNULLP == fileptr )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_GetFilePtr ->input param null !");
          /*设置错误码-无效参数*/
          return XERRPARA;
      }

      return ( ftell(fileptr) );

}
/************************************************************************
函数名：XOS_MoveFile
功  能：将源文件移动到目标文件所指的路径下
输  入：srcfilename -- 要移动的源文件，必须存在。该文件在进行操作之前必须处于关闭状态
        desfilename--要目的文件，该文件已经存在时，返回报错。如果该文件不存在，则将生成新文件并将
        源文件内容移动到新文件
输  出：
返  回：成功返回XSUCC
        失败返回XERROR
说  明：文件名统一格式为:/path/filename+扩展名。如果没有扩展名，则以.txt文件对待
 ************************************************************************/
XS32 XOS_MoveFile(XCONST XCHAR * srcfilename, XCONST XCHAR * desfilename)
{
     XCHAR        MoveOrder[MAX_ORDERLEN] = {0}; /*系统命令行*/
     XCHAR        path[MAX_ORDERLEN]      = {0}; /*srcfile to desfile的命令行*/
     XCHAR        blank[] = " ";                 /**/

     /*判断参数*/
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
         /*设置错误码-无效参数*/
         return XERRPARA;
     }

     if ( XNULL == desfilename )
     {
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->input desfilename null !");
         /*设置错误码-无效参数*/
         return XERRPARA;
     }

     if ( MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(srcfilename)
         || MAX_DIRANDFILEPATH_LEN <= XOS_StrLen(desfilename) )
     {
         XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_MoveFile ->input filename too long !");
         return XERRPARA;
     }

     /*后续会拷贝一个空格，考虑到结束符，所以+2*/
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
         /*设置错误码*/
         return XERROR;
     }

//#if ( XOS_WIN32 || XOS_VTA || XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_WIN32) || defined(XOS_VTA) || defined(XOS_LINUX) || defined(XOS_SOLARIS) )

     system(MoveOrder);

#endif

     return XSUCC;

}

/************************************************************************
函数名：XOS_CopyFile
功  能：将源文件复制一份到目标文件，源文件保持不变
输  入：srcfilename --要复制的源文件，必须存在。该文件在进行操作之前必须处于关闭状态
        desfilename--要复制的目的文件，该文件已经存在时，必须处于关闭状态，且内容将被覆盖；
        如果该文件不存在，则将生成新的文件
输  出：
返  回：成功返回XSUCC
        失败返回XERROR
说  明：将srcfilename中的所有内容写入到desfilename
        中，操作之前两个文件必须处于关闭状态；
        文件路径格式为：/path/filename+扩展名，否则都以.txt文件对待
************************************************************************/
XS32 XOS_CopyFile(XCONST XCHAR * srcfilename, XCONST XCHAR * desfilename)
{
    XCHAR srcformatstr[MAX_DIRANDFILEPATH_LEN] = {0};
    XCHAR desformatstr[MAX_DIRANDFILEPATH_LEN] = {0};
    FILE  *srcf                                = XNULLP;
    FILE  *desf                                = XNULLP;
    XS32  fnfix                                = 0;      /*记录判断文件名是否有扩展名返回值*/
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
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( XNULL == desfilename )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input param null !");
        /*设置错误码-无效参数*/
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

    /*filename路径过长*/
    if ( XFPATHEXSS == fnfix )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input filename too long !");
        /*设置错误码-文件路径过长*/
        return XFPATHEXSS;
    }

#if 0

    /*没有扩展名*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*为formatstr加上扩展名*/
        XOS_StrNcpy(srcformatstr+(XOS_StrLen(srcformatstr)),FILE_POSTFIXNAME,sizeof(FILE_POSTFIXNAME));
    }

#endif

    if( XSUCC != FS_formatpath(desfilename,desformatstr)
        || '\0' == desformatstr[0]  )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),
            "XOS_CopyFile ->input param desfilename %s -> %s  error !",desfilename,desformatstr);
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    fnfix = FS_isfilenameright(desfilename);

    /*filename路径过长*/
    if ( XFPATHEXSS == fnfix )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->input filename too long !");
        /*设置错误码-文件路径过长*/
        return XFPATHEXSS;
    }

#if 0

    /*没有扩展名*/
    if ( XF_NOFIXNAME == fnfix )
    {
        /*为formatstr加上扩展名*/
        XOS_StrNcpy(desformatstr+(XOS_StrLen(desformatstr)),FILE_POSTFIXNAME,sizeof(FILE_POSTFIXNAME));
    }

#endif

    if (XNULLP != (desf = (fopen((XCONST XCHAR *)desformatstr,OPENFILEMODE[ XF_RTMODE ]))) )
    {
        /*目标新文件是否已经存在*/
        fclose(desf);
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->desfile already exist !");
        /*设置错误码-无效参数*/
        return XERRPARA;
    }

    if ( XNULLP == (srcf = (fopen((XCONST XCHAR *)srcformatstr,OPENFILEMODE[ XF_RBMODE ]))) )
    {
        /*源文件不存在*/
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->srcfile not exist !");
        /*设置错误码-无效参数*/
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

    /*这里已经判定了要拷贝的字符串的长度，所以不用考虑会溢出的问题*/
    XOS_StrNcpy( copydirpath,srcformatstr, XOS_StrLen(srcformatstr) );
    XOS_StrNcpy( copydirpath+XOS_StrLen(srcformatstr),blank,XOS_StrLen(blank) );
    XOS_StrNcpy( copydirpath+XOS_StrLen(srcformatstr)+XOS_StrLen(blank),
        desformatstr,
        XOS_StrLen(desformatstr) );

    if ( XSUCC != FS_CatDelorder(copyorder,COrder,copydirpath) )
    {
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->order is wrong. order is too long or order is null!");
        /*设置错误码*/
        return XERROR;
    }

//#if ( XOS_WIN32 || XOS_VTA || XOS_LINUX || XOS_SOLARIS )
#if ( defined(XOS_WIN32) || defined(XOS_VTA) || defined(XOS_LINUX) || defined(XOS_SOLARIS) )

    system(copyorder);

#endif

#if 0

    /*准备读写的操作-打开文件*/
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
        /*创建锁失败-错误码*/
        XOS_FILE_ERRORNO = XF_LOGKFAILED;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->creat lock failed!");
        return XERROR;
    }

    /*加锁互斥*/
    if ( XSUCC != XOS_MutexLock(&cflock) )
    {
        /*加锁失败*/
        XOS_FILE_ERRORNO = XF_LOGKFAILED;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->lock failed!");
        return XERROR;

    }

    /*循环读文件知道结束，并写进另一个目标文件中*/
    while ( EOF != (tmpfilebuffer = fgetc(srcf)) )
    {
        fputc(tmpfilebuffer,desf);
    }

    fclose(srcf);
    fclose(desf);

    /*解锁互斥*/
    if ( XSUCC != XOS_MutexUnlock(&cflock) )
    {
        /*解锁失败*/
        XOS_FILE_ERRORNO = XF_LOGKFAILED;
        XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_CopyFile ->unlock failed!");
        return XERROR;

    }

#endif

    return XSUCC;

}

/************************************************************************
函数名：XOS_IsExistDir
功  能：查看dirname目录是否已经建立， Dirname格式为：/exm/dirname。
输  入：dirname-要查看的目录名，统一的路径格式为/path/dirname

输  出：
返  回：存在返回XSUCC
        不存在返回XERROR
说  明：
  ************************************************************************/
  XS32 XOS_IsExistDir(XCONST XCHAR * dirname)
  {

      XCHAR formatstr[MAX_DIRANDFILEPATH_LEN] = {0};
      XCHAR currentdir[MAX_DIRANDFILEPATH_LEN] = {0}; /*记录调用该函数时的工作目录，动作完成后返回该工作目录*/

      XOS_GetCurrentWorkDir(currentdir,MAX_DIRANDFILEPATH_LEN);

      /* 目录的有效性*/
      if ( XNULL == dirname )
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_IsExistDir ->input param null!");
          /*设置错误码-无效参数*/
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
          /*设置错误码-无效参数*/
          return XERRPARA;
      }

      /*加锁 互斥目录的转移*/
      if ( XSUCC != XOS_MutexCreate(&mutexlock_file_dir.changdir_lock) )
      {
          /*创建锁失败*/
          return XERROR;
      }

      /*加锁互斥*/
      if ( XSUCC != XOS_MutexLock(&mutexlock_file_dir.changdir_lock) )
      {
          return XERROR;
      }

      /*该目录已经存在*/
      if ( 0 == chdir(formatstr) )
      {
          /*返回以前的工作目录*/
          chdir(currentdir);

          /*解锁*/
          if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
          {
              return XERROR ;
          }

          return XSUCC;

      }
      else
      {
          XOS_Trace(MD(FID_FILE,PL_ERR),"XOS_IsExistDir ->check exist dirpath failed!");

          /*解锁*/
          if ( XSUCC != XOS_MutexUnlock(&mutexlock_file_dir.changdir_lock) )
          {
              return XERROR ;
          }

          return XERROR;

      }

}

/************************************************************************
函数名：XOS_GetCurrentWorkDir
功  能：查看当前用户所在的工作目录路径

输  出：
返  回：成功返回工作路径
        失败返回NULL
说  明：
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
函数名：XOS_GetCurrDiskNO
功  能：获取当前工作硬盘的序号。
输  入：
输  出：
返  回：成功返回硬盘序号
        失败返回XERROR以及错误码
说  明：
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
函数名：XOS_CurrDiskFreeCap
功  能：获取当前工作硬盘的剩余可用空间容量。
输  入：driveno-当前工作硬盘序号
        Cap -要返回的剩余空间K
输  出：
返  回：成功返回XSUCC
        失败返回XERROR
说  明：
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
功  能：1. 判断输入文件路径串的组成格式是否正确，路径为数字，字符和下划线组成。
        2.转化为不同系统下转化的路径格式(这里可以不转化，win下兼容/../格式)
        3.考虑win和vxworks下的绝对路径判断,格式为盘符:(/)文件夹(文件)/...
        4:linux和solairs下的格式得继续考证
返  回：输入串错误返回XERROR，否则转化串，并返回XSUCC
***********************************************************/
XS32 FS_formatpath(XCONST XCHAR *srcpathstr,XCHAR *formatpathstr)
{
    XU32   cyc    = 0;                                 /*循环变量*/
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
        /*如果文件名或目录名为空字符串，则作为错误返回防止出现异常*/
        if ( '\0' == file_ab_name[0] || 0 == file_ab_name[0] )
        {
            return XERROR ;
        }
    }

#endif
    ulLen = (XU32)strlen(srcpathstr);/*防止缓存连续造成越界*/
    while (*(srcpathstr+cyc) != '\0'
           && *(srcpathstr+cyc) != 0 && cyc < ulLen)
    {
        if ( despos >= MAX_DIRANDFILEPATH_LEN -2 )
        {
            break;
        }

        /*支持中文路径, bruce add 2012.1.31*/
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
        /*win和vxworks下处理绝对路径*/
#if ( (defined ( XOS_WIN32 )) || (defined( XOS_VTA )) )

#if 0
        /*判断是否为绝对路径*/
        if ( 1 == cyc )
        {
            /*增加一个对盘符后:号的判定*/
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

            /*将原字符串中的/转化为\*/
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

         /*是字符,下划线,或者数字以及"/" */
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
        /*目标字符串结尾*/
        *(formatpathstr+despos) ='\0';

        /*如果文件名或目录名为空字符串，则作为错误返回防止出现异常*/

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

功  能：判断文件路径有没有文件扩展名，
        如果没有则返回没有扩展名标志，
        有后缀名返回值为XSUCC

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
          /*路径过长*/
          return XFPATHEXSS;

      }

      if ( *(filename+cyc) == '\0' )
      {
          /*没有扩展名*/
          return XF_NOFIXNAME;

      }

#endif

      return XSUCC;

}

/**********************************************************

功  能：将不同操作系统下的命令行以及参数合并为desstr形式，以便于系统调用

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

      /*利用XOS_StrNCpy拷贝串*/

      XOS_StrNCat(desstr, path, XOS_StrLen(path));

      return XSUCC;
}

/**********************************************************

功  能：判断传入的目录是相对目录还是绝对目录
返回值：0--不是绝对路径
        1--是绝对路径
        -1--错误参数

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

      /*该路径长度大于2，察看是否是绝对路径*/
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

      /*暂时判断的不准确*/
      if ( '/' == dir[0] )
          return 1;
      else
          return 0;

#endif /*linux/solaris*/

  }

#ifdef __cplusplus

}

#endif /* __cplusplus */


