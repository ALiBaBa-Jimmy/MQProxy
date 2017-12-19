
/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xosmmgt.h
**
**  description:  OS portable include file
**
**  author: zhouguishuang
**
**  date:   2006.8.21
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author              date              modification            
**   zhouguishuang       2006.8.21             create  
**************************************************************/

#ifndef __XOS_MMGT_H__
#define __XOS_MMGT_H__

#ifdef  XOS_MDLMGT
/***************************************************************
        VXWORKS PORTABLE SECTION
***************************************************************/

/*  OS portable definations for vxWorks  */
#ifdef XOS_VXWORKS
/*vxWorks h files used by MM*/
#include <vxworks.h>
#include <string.h>
#include <loadLib.h>
#include <symLib.h>
#include <semLib.h>
#include <assert.h>
#include <iosLib.h>
#include <errno.h>
#include <stdlib.h>
#include <logLib.h>
#include <stdio.h>        
#include <a_out.h>        /*for symbol operation*/
#include <elf.h>
#include <sysSymTbl.h>    /*for symbol operation*/
#include <rebootLib.h>    /*for reboot()*/
#include <errnoLib.h>  
#include <shellLib.h>     /* for execute()*/
#include <dbgLib.h>          /* for tt() */
/*  semaphore  */
#include <semLib.h>
#include <sysLib.h>        /* for sysReboot()*/
#include <wdLib.h>        /* for watchdog timer*/

#include <stat.h>
#include <bootLib.h>
#include <sockLib.h>
#endif


#ifdef XOS_WIN32
#include "windows.h"
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <Imagehlp.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#endif

#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
//#include <sys/file.h>
//#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <sys/select.h>
#include <elf.h>
#include <bfd.h>
#include <sys/mman.h>
#include "xostype.h"

#endif /* #if defined(XOS_LINUX) || defined(XOS_SOLARIS) */
#define MM_SEM_B_INIT_FULL            1
#define MM_SEM_B_INIT_EMPTY            0


#if (defined(XOS_SOLARIS) || defined(XOS_LINUX))
#define ERROR           (-1)
#endif


#ifdef XOS_VXWORKS
#define MMGT_PATH_MAX    512
#define XosMMtaskCreate(taskName,priority,stackSize,funcptr,para) \
        taskSpawn(taskName, priority, 0,  stackSize, (FUNCPTR)funcptr,(int)(para),0,0,0,0,0,0,0,0,0)
#define MM_INVALID_TASK    ERROR
#define MM_taskSuspendCurrent()   taskSuspend(taskIdSelf())
/*  Watch dog timer lib */
#define MM_WD_TMR_ID            WDOG_ID
#define MM_wdTmrCreate()        wdCreate()
#define MM_wdTmrStart(id,timlen,func,para)   wdStart(id,(timlen*sysClkRateGet()),func,para)
#define MM_wdTmrStop(id)        wdCancel(id)

/*reset the cpu*/
#ifdef CPU_ZT5550
#define MMRebootCPU()            reboot(0)
#elif defined CPU_ZT5541
#define MMRebootCPU()            sysReboot()
#else
#define MMRebootCPU()          reboot(0)
#endif


#define MM_MUTEX_ID            SEM_ID
#define MM_SEMB_ID                SEM_ID

#define XosMM_mutexCreate()   semMCreate(0)
#define XosMMmutexTake(a)        semTake(a,WAIT_FOREVER)
#define XosMMmutexGive(a)        semGive(a)
#define XosMMsemBCreate(a)    semBCreate(0,SEM_EMPTY)
#define XosMMsemBTake(a,b)        semTake(a,WAIT_FOREVER)
#define XosMMsemBGive(a)        semGive(a)
#define MM_INVALID_MUTEX        NULL
#define MM_INVALID_SEMB        NULL
#endif  /*  XOS_VXWORKS  */

#ifdef  XOS_WIN32
#define MMGT_PATH_MAX    512
#define MAX_FILENAME_LENGTH 1024
#define MMRebootCPU() ExitProcess(-1)
#define XosMMtaskCreate(taskName,priority,stackSize,funcptr,para) \
        CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)funcptr,NULL,0,&gMMTaskIdContent)
#define MM_taskSuspendCurrent()   SuspendThread(GetCurrentThread())


#define MM_INVALID_TASK         NULL

#define MM_MUTEX_ID               HANDLE
#define MM_SEMB_ID                HANDLE

#define MM_INVALID_MUTEX        NULL
#define MM_INVALID_SEMB            NULL

#define XosMM_mutexCreate()        CreateMutex(NULL,FALSE,NULL)
#define XosMMmutexTake(a)        WaitForSingleObject(a,INFINITE)
#define XosMMmutexGive(a)        ReleaseMutex(a)

#define XosMMsemBCreate(a)        CreateSemaphore(NULL, a, 1, NULL)
#define XosMMsemBTake(a,b)      WaitForSingleObject(a,INFINITE)
#define XosMMsemBGive(a)        (!ReleaseSemaphore(a, 1, NULL)) 
/*on windows, ReleaseSemaphore will return 0 when failed*/

#endif /* #ifdef  XOS_WIN32 */


#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
#define MMGT_PATH_MAX    512
#define MAX_FILENAME_LENGTH 1024
#define MMRebootCPU() exit(0);
#define MM_INVALID_TASK        ERROR

/* #define MM_taskSuspendCurrent()*/       /*pthread_kill(pthread_self(),SIGSTOP)*/
/* change by wulei 070115 */
#define MM_taskSuspendCurrent()\
while(1)\
{\
    pause();\
}\

  
#define MM_MUTEX_ID        int
#define MM_SEMB_ID        int

#define MM_INVALID_MUTEX    ERROR
#define MM_INVALID_SEMB        ERROR


struct bfd_mm_hash_entry
{
    struct bfd_hash_entry root;
    int    ArrayIndex;
};
struct bfd_hash_entry *bfd_mm_hash_newfunc (struct bfd_hash_entry *entry,
                                            struct bfd_hash_table *table,
                                            const char *string);


#endif /* #if defined(XOS_LINUX) || defined(XOS_SOLARIS) */

extern XU32 gMMDebugPrint;

#ifdef XOS_VXWORKS
#define MMInfo(format, arg...) printf("[MM_INFO]: " format "\n" , ## arg)
#define MMDbg(format, arg...)  if(gMMDebugPrint){printf("[MM_DBG ]: " format "\n" , ## arg);}
#define MMWarn(format, arg...) printf("[MM_WARN]: " format "\n" , ## arg)
#define MMErr(format, arg...)  VT_FORMAT_ERR printf("[MM_ERR ]: " format "\n" , ## arg); VT_FORMAT_RESET
#endif

#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
    #define MMInfo(format, arg...) printf("[MM_INFO]: " format "\n" , ## arg)
    #define MMDbg(format, arg...)  if(gMMDebugPrint){printf("[MM_DBG ]: " format "\n" , ## arg);}
    #define MMWarn(format, arg...) printf("[MM_WARN]: " format "\n" , ## arg)
    #define MMErr(format, arg...)  VT_FORMAT_ERR printf("[MM_ERR ]: " format "\n" , ## arg); VT_FORMAT_RESET
#endif

#ifdef XOS_WIN32
extern void MMInfo(char* format, ...);
extern void MMDbg(char* format, ...);
extern void MMWarn(char* format, ...);
extern void MMErr(char* format, ...);

#endif

struct wsmsg
{
    XU32                    dwMsgPoolType;        /* msg payload pool type*/
    XU32                    dwEvent;
    XU32                    dwParam1;
    XU32                    dwParam2;
    XU32                    dwParam3;
    XU32                    dwParam4;
    XU8                  *pData;
    XU32                    dwDataLen;
};


#define MM_SYMB_TYP_DATA    1
#define MM_SYMB_TYP_FUNC    2
typedef XU32 XOS_MM_MODULE_ID;

#ifndef _DEF_WS_STATUS_
#define _DEF_WS_STATUS_
typedef int XOS_MM_STATUS;
#endif

#ifdef XOS_WIN32
#ifndef XOS_SOLARIS 
#ifndef _DEF_STATUS_

#define _DEF_STATUS_
typedef int STATUS;

#endif
#endif
#endif

#ifndef XOS_WIN32
#ifndef HAVE_HANDLE
#define HAVE_HANDLE
typedef XPOINT HANDLE;
#endif
#endif


typedef struct wsmsg *XOS_MM_MSG_ID;

typedef XU32 XOS_MM_TASK_ID;
typedef XOS_MM_STATUS (* WS_TASK_ENTRY_FUNC) (XOS_MM_MSG_ID pMsg, void *);


/* cpu核数总数 */
#ifndef MAX_CPUCORE_NUM
#define MAX_CPUCORE_NUM   1024
#endif

/* cpu绑定配置 */
typedef struct
{
    XS32 cpunum;
    XU16 Cpus[MAX_CPUCORE_NUM];
}t_CpuBindCfg;



void XosMMerrNoSet(unsigned int errNo);
unsigned int XosMMerrNoGet();

/*return XSUCC or ERROR, type and value will be returned*/
int XosMM_findSymForDog(char* symName,int* type, XPOINT* value);
int XosMM_findSymbByName(char* symName,int* type, XPOINT* value);

void XosMMShellParser();
int  XosMMExecuteStrSingleFunc(char* str);
void XosMMExecuteShellStr(char* str);
int XosMMinitSymbTbl(char* execFileName);
XS32 XosModuleManagerInit (XS8 * bootRootPath, XS8 * cfgFileName);


/* ====== module loader error definations  ======= */
#define MODULE_MANAGER_CANNOTOPENOBJ      0xffff0001  /*can not open obj file*/

#define MODULE_MANAGER_CANNOTLOADOBJ      0xffff0002  /*can not load obj file*/

#define MODULE_MANAGER_CANNOTFINDFUNCADDR 0xffff0003  /*can not find entry 
                                                        function address     */
/* ====== SYS reg error definations ============== */
#define SYSREG_CANNOTOPENCFGFILE          0xfffe0001 /*can not open 
                                                        configuration file   */
#define SYSREG_CANNOTREADCFGFILE          0xfffe0002 /*can not read 
                                                        configuration file   */
#define SYSREG_CANNOTOPENDIR              0xfffe0003 /*can not open certain 
                                                        system registry table 
                                                        directory            */
#define SYSREG_CANNOTFINDKEY              0xfffe0004 /*can not find specified 
                                                        system registry table 
                                                        key                  */

/* ====== module manager root error definations  ======= */
#define MMROOT_ERROR_RUN_TIME_OUT         0xfffd0001 /*watch dog time expired*/


/*
*    return value of Module Init Entry
*/
#define MODULE_INIT_OK              0 /*  module is ok                         */
#define MODULE_INIT_FATAL_ERROR     1 /*  system need not to stop running      */
#define MODULE_INIT_COMMON_ERROR    2 /*  system must stop running and reset.  */
#define MODULE_INIT_NOT_START        3 /*  system is not inited yet*/
#define MODULE_INIT_RUNNING             4 /*  system is not initializing*/


/*
*     return value of XosModuleInitWaitTask() 
*/
#define MM_TASK_INIT_OK             1
#define MM_TASK_INIT_FAIL            2
#define MM_TASK_INIT_MM_Q_ERR         3

/*  configuration file name*/
#define WACOS_CONFIG_FILE           "xosmmcfg.dat" /*configuration file*/

#define MAX_MODULE_NAME             100
#define MAX_MODULE_NUMBER           100

/* when call APIs of System Registry Table, caller must pay attention to string length,
 * there shall be enough memory to store key name and value.
 */
#define MAX_KEYNAME_LENGTH          30        
#define MAX_KEYVALUE_LENGTH         128

/* ----------------------------------mdl.h-----------------------------------------------------*/
#define VT_FORMAT_ERR               printf("\033[1;31m");
#define VT_FORMAT_RESET             printf("\033[0m");

#define KEYNAME_MEM72               "Mem72"
#define KEYNAME_MEM128                "Mem128"
#define KEYNAME_MEM256                "Mem256"
#define KEYNAME_MEM512                "Mem512"
#define KEYNAME_MEM1024                "Mem1024"
#define KEYNAME_MEM2048                "Mem2048"
#define KEYNAME_HEAPSIZE            "HeapSize"
#define KEYNAME_LINEAR_HEAP         "LinearHeap"
#define KEYNAME_STACKSIZE            "StackSize"
#define KEYNAME_TASKCOUNT            "TaskCount"
#define KEYNAME_MODULE_ID            "ModuleId"
#define KEYNAME_SSI_TOTAL_SIZE         "SsiTotalMem"
#define KEYNAME_CPU_BIND             "CpuBind"
#define KEYNAME_MSGQUE_NUM           "MsgQueNum"

#define KEYNAME_MEM_POOL_NAME        "PoolName"
#define KEYNAME_MEM_CACHE_SIZE        "CacheSize"
#define KEYNAME_MEM_HEAP_SIZE       "HeapSize"
#define KEYNAME_MEM_N_REGION           "NormalRegion"
#define KEYNAME_MEM_N_HEAP           "NormalHeap"
#define KEYNAME_MEM_M_REGION           "ModuleRegion"
#define KEYNAME_MEM_VIRT_BASE       "VirtualBase"
#define KEYNAME_MEM_VIRT_SIZE       "VirtualSize"
#define KEYNAME_MEM_SYS_MOD_SIZE    "ModuleMemSize"

#define OBJFILENAMEKEY                 "ObjectFileName"
#define OBJFILEPATHKEY                 "ObjectFilePath"
#define OBJENTRYFUNCKEY               "EntryFunc"
#define MODULENAMEKEY                  "Name"

#define NO_CFG_PATH                 "no_cfg_path"
#define NO_CFG_NAME                 "no_cfg_name"


#define DEFAULT_MODULE_INIT_TIME_OUT 60     /*  in seconds */


#define MMIntErr(format, a,b,c,d,e,f)  logMsg("[MM_ERR ]: " format "\n", a,b,c,d,e,f)
#define MMIntInfo(format, a,b,c,d,e,f) logMsg("[MM_INFO]: " format "\n", a,b,c,d,e,f)

/*
* module entry function type
*/
typedef XS32 (*MODULE_ENTRY)(HANDLE hParametersDir, XU32 argc, XS8 * argv[]);

#define MM_MAX_MODULENAME_LENGTH        30
#define MM_MAX_OBJFILE_NAME_LENGTH      30
#define MM_MAX_OBJFILE_PATH_LENGTH      30
#define MM_MAX_ENTRYFUNCTION_NAME       30

/*---------------------------------root.h------------------------------------------------------*/
#define MAX_MODMSGQ_LENGTH              40
#define MAX_MODMSG_SIZE                 64

#define TIME_TO_DELAY_WHILE_ERROR       5     /*  in seconds */

/*  action code, while error occures  */
#define ACTION_IGNORE                   0x01
#define ACTION_RELOAD                   0x02
#define ACTION_REBOOT                   0x03
#define ACTION_RESET                    0x04

/* board type */
#define BOARD_TYPE_RH3000                0x01
#define BOARD_TYPE_AP3000                0x02
#define BOARD_TYPE_OTHER                0x03
#define BOARD_TYPE_NONE                    0x04

/*------------------------------------base info define-----------------------------------------*/
#define XOS_MM_MOD_BASE                     256      /*Module Id Base value*/
#define XOS_MM_TSK_PER_MOD                  8
#define XOS_MM_MIN_TASKS                    256
#define XOS_MM_MAX_MODULES                  512  /*module id should not exceed or equal XOS_MM_MAX_MODULES*/
#define XOS_MM_MAX_TASKS                    ((XOS_MM_MAX_MODULES-XOS_MM_MOD_BASE+1)*XOS_MM_TSK_PER_MOD+XOS_MM_MIN_TASKS) 


/* -----------------------------------os reliable define---------------------------------------*/
#define XOS_MM_TASK_NAME_LEN                15
#define XOS_MM_MODULE_NAME_LEN                15
#define XOS_MM_FILE_NAME_LEN                64

#define XOS_MM_MEM_POOL_MAX                    20
#define XOS_MM_MEM_POOL_NAME_MAX            24

#define XOS_MM_INVALID_TASK_ID              0XFFFFFFFF
/*------------------------------------os reliable struct---------------------------------------*/
typedef struct ws_mem_init_para_s
{
    XU32 cacheSize;
    XU32 heapSize;
    char poolName[XOS_MM_MEM_POOL_NAME_MAX]; 
}ws_mem_init_para_t,XOS_MM_MEM_INIT_PARA;

typedef struct ws_mod_init_para
{
    XU32                    nModuleId;
    XU32                    dwMemSize;
    XU32                  nMaxStackSize;    /* Maximize stack size for specified module, it is 
                                        dependent on how many tasks will be created within 
                                        this module and how large of each task stack size is*/
    XU32                   nMsgQueNum;  /* 消息队列大小 */
                                        
    XU32                 segSizeText;
    XU32                 segSizeData;
    XU32                 segSizeBss;
    
    XU8                    aModuleName[XOS_MM_MODULE_NAME_LEN+1];/*0x4,4*/
    XU8                   aObjFileName[XOS_MM_FILE_NAME_LEN];      /*0x14,20*/
    XU8                    memPoolName[XOS_MM_MEM_POOL_NAME_MAX];
    
    XU8                    maxTaskNum;
    XU8                    bLinearHeap;
    XU16                    reserved;
}XOS_MM_MOD_INIT_PARA;


typedef struct ws_init_para
{
    XU32                 virtualBase;
    XU32                 virtualSize;
    XU32                 boardType;
    XU32                 moduleRegionSize;
    XU32                 stackSize;
    XU32                 linearHeapInitSize;
    XU32                 linearHeapGrowSize;
    XOS_MM_MEM_INIT_PARA    sysMemInitPara;
    XOS_MM_MEM_INIT_PARA    usrMemInitPara[XOS_MM_MEM_POOL_MAX];
    XU32                 ppbPciVaddr;
    XU32                 ppbPciVaddrSize;
    XU32                 pooCount;
    XU32                 totalTmrCount;
}XOS_MM_SSI_INIT_PARA;

/*-----------------------------------struct declare -------------------------------------------*/
typedef struct MM_MODULE_S
{
    XOS_MM_MOD_INIT_PARA    wsModInitPara;    
    XS8                  modName[MM_MAX_MODULENAME_LENGTH + 2];    
    MODULE_ENTRY        pEntryFunc;    
    XU32                 num_parameter;  /*parameter number*/    
    HANDLE              modHDir; /*contains the handle <MODULE>*/     
    XU32                 module_init_flag;
    XS32                 module_init_sys_err_no;    
    XU32                 modInitTime; /*the time that module initialization cost*/
    XOS_MM_MODULE_ID        wsModuleId;
    t_CpuBindCfg         module_cpu_bind; /* 供平台解析cfg.xml文件存放各模块module cpu绑定信息 */
#ifdef XOS_VXWORKS
    XU32                 segSizeText;
    XU32                 segSizeData;
    XU32                 segSizeBss;
#endif
    /*    struct MM_MODULE_S * pNextModule; next module*/
}XOS_MM_MODULE_T;

/*-----------------------------------------------------------------------------*/
typedef struct sys_reg_key_s
{
#define MAX_KEYTYPE_LEGNTH  30        /*type of key, it is a string, */
    XU32                     keyType;
#define VALUEKEY            1
#define SHELLKEY            2

#define MAX_KEYNAME_LENGTH  30
    XS8                      name[MAX_KEYNAME_LENGTH + 2]; /*name of key, if the key is not value key
                                      */
#define MAX_KEYVALUE_LENGTH 128
    XS8                      value[MAX_KEYVALUE_LENGTH + 4];
    struct sys_reg_key_s    *pFirstSonKey;
    struct sys_reg_key_s    *pNextBrotherKey;
}sys_reg_key_t;

typedef sys_reg_key_t       *sys_reg_key_ptr;
typedef sys_reg_key_t       sys_reg_dir_t;
/*-----------------------------------------------------------------------------*/
typedef struct sys_reg_tbl_s
{
#define MAX_VERSION_LENGTH      15
    XS8                          version[MAX_VERSION_LENGTH + 1];            /*system registry table version*/
#define MAX_CODE_LENGTH         15
    XS8                          encoding[MAX_CODE_LENGTH +1];                /*system registry code type*/
    
    sys_reg_dir_t               *rootDir; /*root directory*/
}sys_reg_tbl_t;
/*-----------------------------------------------------------------------------*/

#define SYSREG_ISKEY(handle) (((sys_reg_key_t*)(handle))->pFirstSonKey == NULL)
#define SYSREG_ISDIR(handle) (((sys_reg_key_t*)(handle))->pFirstSonKey != NULL)
#define SYSREG_SHELLKEY(handle) ((0 == strcmp(((sys_reg_key_t*)handle)->name, "SHELL"))\
                                   ||(0 == strcmp(((sys_reg_key_t*)handle)->name, "Shell"))\
                                   ||(0 == strcmp(((sys_reg_key_t*)handle)->name, "shell")))


/* ----------------------------------function declare------------------------------------------ */
XU32 XosModuleInitWaitTask(int timeOut);
void XosModuleInitTaskOK();
void XosModuleInitTaskFail();


/*
*    Functions that tell the status of the init procedure
*/

typedef XU32 (* MM_INIT_OVER_CBF)(void);
XS32 XosMMRegisterInitOverCallback( MM_INIT_OVER_CBF pFunc );

/*  Function prototypes  */

XS32 XosMMCreateModule(HANDLE hDir, XOS_MM_MODULE_T *pModule);
int XosMMStrTrimSpace(char * strToTrim);
int XosMMStrTrim(char * strToTrim, char charPattern);
int XosMMStrFindChar(char* strToLkup,char charPattern,int outInvComma);
int MMStrFindCharOutsideInvComma(char* strToLkup,char charPattern);
int XosMMStrFindOtherChar(char* strToLkup,char charPattern);
int XosMMStrFindNoSpace(char* strToLkup);
int XosMMStrFindSpace(char* strToLkup);
int XosMMStrFindLastChar(char* strToLkup,char charPattern,int outInvComma);

HANDLE XosMMGetCurrentLoadModule( void );


XS32 XosMMRunModule(XOS_MM_MODULE_T * pModule);
XU32 XosMMGetModuleInitTime(XOS_MM_MODULE_T * pModule);
XOS_MM_STATUS XosMMGetGlobalConfig();
XS32 XosMMShowAllModule();
XU32 XosMMShowInitError();
void XosMMShowModuleInfo(XU32 modIndex);


XS32 XosCreateSystemRegistryDatabase(XS8 * filename, XS8 * filepath);
HANDLE XosSysRegGetRootDir();
XS32 XosSysRegOpenDir(HANDLE hDir, HANDLE * phSubDir, XS8 * subDirName);
XU32 XosSysRegQueryU32KeyValue(HANDLE hDir, XS8 * keyName, XU32 * value);
XU32 XosSysRegQueryS32KeyValue(HANDLE hDir, XS8 * keyName, XS32 * value);
XU32 XosSysRegQueryStrKeyValue(HANDLE hDir, XS8 * keyName, XS8 * value);
XU32 XosSysRegSetU32KeyValue(HANDLE hDir, XS8 * keyName, XU32 value);
XU32 XosSysRegSetS32KeyValue(HANDLE hDir, XS8 * keyName, XS32 value);
XS32 XosSysRegSetStrKeyValue(HANDLE hDir, XS8 * keyName, XS8 * value);
XU32 XosSysRegEnumDir(HANDLE hDir, HANDLE hPreDir, XS8 * subDirName, HANDLE * phSubDir);
XU32 XosSysRegEnumKey(HANDLE hDir, HANDLE hPreKey, XS8 * value, XS8 * name, HANDLE * phSubKey);
XU32 XosSysRegGetKeyName(HANDLE hKey, XS8 *keyName);
#define SysReg_GetDirName(hDir, dirName) XosSysRegGetKeyName(hDir, dirName)
XS32 XosSysRegGetKeyValue(HANDLE hKey, XS8 *keyValue);
XS32 XosMM_XmlReadCfg(XVOID);


/*private functions*/
sys_reg_dir_t * XosSysRegQueryDir(HANDLE hDir, XS8 * dirName);
sys_reg_key_t * XosSysRegQueryKey(HANDLE hDir, XS8 * keyName);
XS32 XosFirstLevelDirName(XS8 * keyName, XS8 * dirName, XS8 ** restName);
XU32 XosBuildSystemRegistryTable(XS8 *filename, XS8 *filepath);


XOS_MM_TASK_ID XosMMTaskIdSelf(void);
XOS_MM_STATUS XosModuleInit(XOS_MM_MOD_INIT_PARA *pModInfo);
XBOOL XosModuleIdCheck(XOS_MM_MODULE_ID nModuleId);

XS32 XosMM_XMLreadSrvCfg( sys_reg_tbl_t * parsedDoc,XS8 *filename);
XS32 XosMMCreateSrvModules(XU32 moduleidx);
XS32 XosModuleStartSrv(XCONST XS8 *filename);

/************************************************************************
函数名: XOS_SetCpuBind
功能：  设置平台cpu绑定
输入：  ptCpuBind cpu绑定的结构体信息
输出：
返回： 成功返回 0， 失败返回 -1
说明： 绑定针对平台下所有线程有效，绑定核数在cfg.xml global设置
       如果各module中有设置，在模块内，模块的设置会覆盖此全局设置
************************************************************************/
XS32 XOS_SetCpuBind(t_CpuBindCfg *ptCpuBind);
/************************************************************************
函数名: XOS_ParseCpuBindCfg
功能：  解析从配置文件中读取的绑定cpu的配置字符串，字符串以，分割
输入： value  : 从配置文件中解析的字符串
输出： pCpuBindCfg : 将解析到字符串根据分隔符','，解析到此结构体中
返回： 无
说明：
************************************************************************/
XVOID XOS_ParseCpuBindCfg(t_CpuBindCfg* pCpuBindCfg, XS8 *value);

XS32 XosModuleGetCfgSize(XVOID);
XS32 XosModuleGetCfgInfo(XCONST XS8 *filename, XU8 *modInfo);
XS32 XosModuleStartCfg(XCONST XU8 *modInfo);

#define MAX_TASK_PER_MODULE                8

/* Error number define */
#define XOS_MM_ERR_MODULE_MODULEID            0xFFF80001        /* Invalid Module Id */
#define    XOS_MM_ERR_MODULE_EXIST                0xFFF80002        /* Module already initialized */
#define XOS_MM_ERR_MODULE_NAME                0xFFF80003        /* Invalid moudle name */
#define XOS_MM_ERR_MODULE_MEM_INIT            0xFFF80004        /* Initial memory region error */
#define XOS_MM_ERR_MODULE_TASKOVER            0xFFF80005        /* Too many tasks in module */
#define XOS_MM_ERR_MODULE_NOMEM                0xFFF80006         /* Not enough memory */
#define XOS_MM_ERR_MODULE_STACKSIZE            0xFFF80007        /* Not enough stack size */
#define XOS_MM_ERR_MODULE_PARAMETER            0xFFF80008        /* Invalid input parameters */
/* ---------------------------------------------------------------------------------- */

#ifndef XOS_MM_WSSI_SYS_TASK_ID
#define XOS_MM_WSSI_SYS_TASK_ID             1
#endif


/*------------------------------ Const definitions (Configuartion)-------------------*/
#define XOS_MM_COTAINER_MODULE_ID            0
#define XOS_MM_NON_SSI_TASK_ID                0x19790423        
/*------------------------------ Operation definitions (Macro)-----------------------*/
#define XOS_MM_APP_TASK_ID_CHECK(dTaskId)    XOS_MM_TASK_ID_CHECK(dTaskId)
#define XOS_MM_TASK_ID_CHECK(dTaskId)    \
                WS_ASSERT( !( (XOS_MM_WSSI_SYS_TASK_ID == (dTaskId)) || ( XOS_MM_MIN_TASKS<=(dTaskId)&&(dTaskId)<=XOS_MM_MAX_TASKS ) ), WS_ERR_TASK_TASKID, XERROR)


/*--------------------------------- module struct-----------------------------------*/
/*------------------------------ Type definitions  ----------------------------------*/

/* Memory region data structure */
typedef struct
{
    char*                    pMemBase;
    XU32                    dwMemSize;
    XU8*                    pStackBase;
    XU32                    dwStackFreeSize;
    XU32                      nMaxStackSize;    /* Maximize stack size for specified module, it is 
                                        dependent on how many tasks will be created within 
                                        this module and how large of each task stack size is*/
#if (defined XOS_VXWORKS)
    char*                    textAddr;                    /*0x11c,188*/
    char*                    bssAddr;                    /*0x120,192*/
    char*                    dataAddr;                    /*0x124,196*/
    XU32                    textSize;
    XU32                    bssSize;
    XU32                    dataSize;
    char*                    vmemRealTxtAddr;                    /*0x11c,188*/
    char*                    vmemRealBssAddr;                    /*0x120,192*/
    char*                    vmemRealDataAddr;                    /*0x124,196*/
#endif
    char*                    vmemRealMemBase;                    /*0x128,200*/
} XOS_MM_MOD_MEM_INFO;


typedef struct
{
    XOS_MM_MODULE_ID        nModuleId;
    XU8                        pTaskName[XOS_MM_TASK_NAME_LEN+1];
}XOS_MM_ENT_MOD_MAP;


typedef struct 
{
    XOS_MM_MODULE_ID         nModuleId;                    /*0x0,0*/
    XU8                        pModuleName[XOS_MM_MODULE_NAME_LEN+1];/*0x4,4*/
    XU8                       aObjFileName[XOS_MM_FILE_NAME_LEN];      /*0x14,20*/
    XOS_MM_TASK_ID            aTasks[MAX_TASK_PER_MODULE];       /*0x54,84*/
    XU8                        nNumTasks;                           /*0x74,116*/
    XU8                        nMaxTasks;                           
    XU8                        fUsed;                               
    XU8                        platMode;
    XU32                    platHeapUsage;
    XOS_MM_MOD_MEM_INFO        stMemInfo;                            /*0x8c,140*/
}XOS_MM_MODULE_TAB;


extern XOS_MM_MODULE_T  gMMModuleList[MAX_MODULE_NUMBER+2];

XS32 XOS_CheckIpmiEnv(XVOID);
XBOOL XOS_IpmiIsOk(XVOID);
XVOID XOS_SetIpmiFlag(XBOOL val);

#endif /* XOS_MDLMGT */

#endif  /* __XOS_MMGT_H__ */

