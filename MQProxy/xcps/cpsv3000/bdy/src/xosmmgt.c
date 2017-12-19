/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xosmdlmgt.c
**
**  description:  The task body of Module Manager Loader
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

#ifdef  XOS_MDLMGT

#ifdef __cplusplus
    extern "C" {
#endif

/*-----------------------Head file----------------------------------------*/
#include "xosos.h"
#include "xosencap.h"
#include "clishell.h"

#include "xmlparser.h"
#include "xosxml.h"
#include "xosmmgt.h"
#include "xostrace.h"
#include "xospub.h"



XEXTERN XS32 write_to_syslog(const XS8* msg,...);




/*-------------------end Head file----------------------------------------*/


/*------------------- global declare -------------------------------------*/
#define MM_INIT_CBF_MAX         10
MM_INIT_OVER_CBF                gMMInitOverCBF[MM_INIT_CBF_MAX];
/*ָʾ��ǰװ�ص�ģ�� */
HANDLE                          gdCurrentLoadModuleID;
/* ָʾģ��������״̬�����ھ����֣�0 ->û����� 1 ->�Ѿ���ɡ� */
static      XU32                dModuleCreateStatus = 0;
char                            gstrMMRootPath[MMGT_PATH_MAX+0x10];
char                            gstrMMCfgFileName[MAX_FILENAME_LENGTH+0x10];
MM_SEMB_ID                      gMMTaskHelpSemB = 0;
#define                         MM_TASK_INIT_RESPONSE_FAIL  2
#define                         MM_TASK_INIT_RESPONSE_OK    3
#define                         MM_TASK_INIT_RESPONSE_NULL  4
XU32                            gdMMTaskWaitResponse = MM_TASK_INIT_RESPONSE_NULL;
MM_MUTEX_ID                     gMMTaskWaitMutex = 0;
XOS_MM_TASK_ID                  gMMTaskWaitTaskId = 0;
XU32                            gMMInitTimeout;
XU32                            gMMFailReset = TRUE;
XU32                            gMMDebugPrint = FALSE;
XU8                             gMMSsiProcIdFileName[MAX_KEYVALUE_LENGTH+10];
XU32                            gMMSsiTotalSize;
XOS_MM_MODULE_ID                gMMCurModId;
static unsigned long            MMErrNo = 0;
XU32                            gMMFidTraceLel = 0;
t_CpuBindCfg                    gtCpuBindCfg;   /* ��ƽ̨����cfg.xml�ļ����global cpu����Ϣ */

/*--------------------------------------------------------------------------------------------*/

#if defined(XOS_WIN32) || defined(XOS_VXWORKS) || (defined(XOS_SOLARIS) || defined(XOS_LINUX))
#ifdef XOS_VXWORKS
/*
XS32                        gMMRootTaskId =0;
XS32                        gMMWdHandleTaskId =0;
*/
XS32                        gMMAlarmTaskId = 0;
char                        * gWS_taskLibStackProtRealAddr = NULL;
XU32                        gstWS_osTaskId[XOS_MM_MAX_TASKS];
extern char                 *sysBootLine; /* address of boot line */
extern STATUS               usrBootLineCrack (char *bootString, BOOT_PARAMS *pParams);
#endif /*XOS_VXWORKS*/

#ifdef  XOS_WIN32
/*
HANDLE                      gMMRootTaskId =NULL;
HANDLE                      gMMWdHandleTaskId =NULL;
*/
DWORD                       gMMTaskIdContent = 0;
HANDLE                      gMMAlarmTaskId = 0;
static XU32                 gWSTaskIdKey;
XU32                        gstWS_osTaskId[XOS_MM_MAX_TASKS];
#endif /*XOS_WIN32*/

#if defined (XOS_SOLARIS) || defined (XOS_LINUX)
/*
XS32                        gMMRootTaskId =NULL;
XS32                        gMMWdHandleTaskId =NULL;
*/
XS32                        gMMAlarmTaskId = 0;
asymbol                     **gMM_sysSymbolTable;
long                        gMM_sysSymbolCount;
XPOINT                         gMM_sysSymbolStartAddr;
struct bfd_hash_table       gMM_sysSymbolHashTbl;
static pthread_key_t        gWSTaskIdKey;
pthread_t                   gstWS_osTaskId[XOS_MM_MAX_TASKS];
#endif /*XOS_LINUX*/

#else
    #error Module Manager only suppoert XOS_VXWORKS, XOS_LINUX, XOS_SOLARIS and XOS_WIN32
#endif

#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
int XosMMsemBTake(MM_SEMB_ID semId,int timeout);
int XosMMmutexTake(MM_MUTEX_ID mutexId);
int XosMMmutexGive(MM_MUTEX_ID mutexId);
int XosMMsemBGive(MM_SEMB_ID semId);
MM_SEMB_ID XosMMsemBCreate(int initState);
MM_MUTEX_ID XosMM_mutexCreate();
#endif

XOS_MM_MODULE_T                 gMMModuleList[MAX_MODULE_NUMBER+2];

typedef struct MM_SYSTEM_INFO_S
{
    XU32 moduleCount;
    XU32 taskCount;
    XU32 stackTotal;
    XU32 objTotal;
    t_XOSSEMID sem;
}MM_SYSTEM_INFO_T;

MM_SYSTEM_INFO_T                gMMSystemInfo;
XOS_MM_SSI_INIT_PARA            gMMWsSsiInitPara;
XOS_MM_STATUS XosMMCalObjInfo(char* filename, XU32* pRetModSize, XOS_MM_MODULE_T *pModule);
XOS_MM_STATUS XosMMInitModuleList();
#define XOS_NON_SSI_TASK_ID            0x20060825
XU32                        gWSTaskIdMap[XOS_MM_MAX_TASKS];
#if defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_WIN32)
static XU32                 gWSTaskIdCom = XOS_NON_SSI_TASK_ID;
#endif

/* ȫ���ź���������*/
#define SYSREGSEMNAME       "SYSREGSEM"

/*ȫ���ź���������ϵͳע���Ĳ�����*/
MM_MUTEX_ID                 semProtectSysReg = 0;

/*ϵͳע���*/
static sys_reg_tbl_t        * gSysRegTable = NULL;

#define MM_PARSE_ST_LEVEL_MAX            10
sys_reg_tbl_t               *gptrSysRegTblRet;
sys_reg_key_ptr             gptraMMCurNode[MM_PARSE_ST_LEVEL_MAX];
XS32                        giMMSaxParseCurLevel;

#define MM_PARSE_STATE_NULL            0
#define MM_PARSE_STATE_ERROR        1
#define MM_PARSE_STATE_START        2
#define MM_PARSE_STATE_END            3

XS32                        gdMMSaxParseState;

#define NEXT_NODE_SON                1
#define NEXT_NODE_BRO                2

XU32                        gdNextNodeType;
int                         gdMMXmlParseDebug = 0;

#define MAX_PATH_NAME                240
#define WS_VMEM_MODULE_GAP_SIZE    (PAGE_SIZE*4)

XU8                            gWS_moduleRootPath[MAX_PATH_NAME];
XOS_MM_ENT_MOD_MAP              gaTriEntModMap[256];
XOS_MM_MODULE_TAB               gstWS_moduleTab[XOS_MM_MAX_MODULES];
XOS_MM_STATUS       XosModuleAdd(XOS_MM_MODULE_ID nModuleId, XS8 *pName,XOS_MM_MOD_INIT_PARA *pModInfo);
XOS_MM_STATUS       WS_moduleRemove(XOS_MM_MODULE_ID nModuleId );
XOS_MM_STATUS       XosModuleLibInit(XU8* pRootPath,XU32 nStackSize);
XOS_MM_STATUS        XosModuleTriMapInit();
XOS_MM_STATUS       XosMMTaskLibInit();
void                XosMSMMtaskSleep(int msec);
XS32                XosMMCreateAllModules(XCHAR* errMsg, XS32 msgSize);

static char gstrModuleInitFlag[5][15]={
                {"INIT_OK"},
                {"FATAL_ERR"},
                {"COMMON_ERR"},
                {"NOT_INIT"},
                {"INITING"}};
/*------------------- end global declare ----------------------------------*/


/*------------------- basic function --------------------------------------*/
/******************************************************************************
                    FUNCTIONS
******************************************************************************/
#ifdef VXWORKS   
//�Ȳ�����س�ʼ��
extern int PatInit(int argc,char *argv1, char *argv2, char *argv3);
extern int xos_hainit();
#endif

/************************************************************************
������: Patch_init
����:  �Ȳ����ĳ�ʼ��
����:
���:
����:�ɹ�����XSUCC , ʧ�ܷ���XERROR
˵��:
************************************************************************/
XS32 Patch_init(void)
{
#ifdef VXWORKS 
    XS32 ret;
    t_PATCHCFG  patchCfg;     
    ret = XML_readPatchCfg(&patchCfg, "xos.xml");
    if( XTRUE != ret )
    {
        return XERROR;//��ȡxmlʧ��
    }

    if( 0 == (XU32)atol(patchCfg.patchLoadFlag) )
    {
        //����ʱ�����ز���,��ָ��·��
        PatInit(2, patchCfg.maxPatchNum, patchCfg.patchPath, patchCfg.patchLoadFlag); 
    }
    else
    {
        //����ʱ���ز���,ָ��·��
        PatInit(3, patchCfg.maxPatchNum, patchCfg.patchPath, patchCfg.patchLoadFlag); 
    }
#endif    
    return XSUCC;
}


/************************************************************************
������:    XosMMerrNoSet
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
void XosMMerrNoSet(unsigned int errNo)
{
    MMErrNo = errNo;
}


/************************************************************************
������:    XosMMerrNoGet
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
unsigned int XosMMerrNoGet()
{
    return MMErrNo;
}


/************************************************************************
������:    XosMMGetCurLdModId
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_MODULE_ID XosMMGetCurLdModId()
{
    return gMMCurModId;
}


/************************************************************************
������:    XosMMSetCurLdModId
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
void XosMMSetCurLdModId(XOS_MM_MODULE_ID dModId)
{
    gMMCurModId = dModId;
}

/************************************************************************
������:    XosMMRegisterInitOverCallback
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XS32 XosMMRegisterInitOverCallback( MM_INIT_OVER_CBF pFunc )
{
    int looper;
    if( NULL == pFunc )
        return XERROR;
    else
    {
        for(looper=0;looper<MM_INIT_CBF_MAX;looper++)
        {
            if(gMMInitOverCBF[looper] == NULL)
            {
                gMMInitOverCBF[looper] = pFunc;
                return XSUCC;
            }
        }
        return XERROR;
    }
}


/************************************************************************
������:    XosMMGetCurrentLoadModule
���ܣ�  ��ȡ��ǰװ�ص�ģ��ID��
������
�����
���أ�  ��ǰģ��ID
˵����
************************************************************************/
HANDLE XosMMGetCurrentLoadModule( void )
{
    return gdCurrentLoadModuleID;
}


/************************************************************************
������:    XosMMSetCurrentLoadModule
���ܣ�  ���õ�ǰװ�ص�ģ��ID��
������
�����
���أ�  XSUCC -successful
˵����
************************************************************************/
XS32 XosMMSetCurrentLoadModule(HANDLE dModuleID)
{
    gdCurrentLoadModuleID = dModuleID;
    return XSUCC;
}


/************************************************************************
������:    XosMMCalModuleInfo
���ܣ�  ͳ�Ƶ�ǰģ����Ϣ
������
�����
���أ�  XSUCC -successful
˵����
************************************************************************/
XOS_MM_STATUS XosMMCalModuleInfo()
{
    HANDLE hRootDir = (HANDLE)NULL;
    HANDLE hModuleDir = (HANDLE)NULL;
    XS8 dirName[MAX_KEYNAME_LENGTH];
    XU32 tempU32Value=0;
    XOS_MM_MODULE_T* pModule=NULL;
    XU32 moduleCount=0;
    XU32 taskCount=0;
    XU32 stackTotalSize = 0;
    XU32 objTotal=0;
    XU32 result = XSUCC;
    XS8 objFileName[MAX_KEYVALUE_LENGTH+12] = "";
    XS8 objFilePath[MAX_KEYVALUE_LENGTH+12] = "";
    XS8 nameBuffer[MAX_KEYVALUE_LENGTH+MAX_KEYVALUE_LENGTH+10] = "";

    memset(dirName, 0, MAX_KEYNAME_LENGTH);

    gMMSystemInfo.moduleCount = 0;
    gMMSystemInfo.objTotal = 0;
    gMMSystemInfo.taskCount = 0;
    gMMSystemInfo.stackTotal = 0;

    MMInfo("Checking OBJ file");
    hRootDir = XosSysRegGetRootDir();

    XosMMInitModuleList();

    XosSysRegEnumDir(hRootDir, (HANDLE)NULL, dirName, &hModuleDir);
    for(moduleCount = 0; (moduleCount < MAX_MODULE_NUMBER) && (hModuleDir!=(HANDLE)NULL);)
    {
    /*decide whether system registry table directory is MODULE directory,
        if yes, create module*/
        if(0 == strncmp(dirName, "Module",6))
        {
            pModule = &(gMMModuleList[moduleCount]);
            moduleCount++;

            pModule->modHDir = hModuleDir;
            /* ģ�������� */
            if(XSUCC==XosSysRegQueryU32KeyValue(hModuleDir, KEYNAME_TASKCOUNT,&tempU32Value))
            {
                taskCount+=tempU32Value;
            }
            /* ��ջ��С */
            if(XSUCC==XosSysRegQueryU32KeyValue(hModuleDir, KEYNAME_STACKSIZE,&tempU32Value))
            {
                stackTotalSize += tempU32Value;
            }
            strcpy(nameBuffer,"");
            /* Ŀ���ļ�·�� */
            result = XosSysRegQueryStrKeyValue(hModuleDir, OBJFILEPATHKEY, objFilePath);
            if(result==XSUCC)
            {
                strcat(nameBuffer,objFilePath);
            }
            /* Ŀ���ļ����� */
            result = XosSysRegQueryStrKeyValue(hModuleDir, OBJFILENAMEKEY, objFileName);
            if(result==XSUCC)
            {
                strcat(nameBuffer,objFileName);
            }
            result = (XU32)strlen(nameBuffer);
            if(result>=XOS_MM_FILE_NAME_LEN)
            {
                MMErr("OBJName Too Long: Max is %d, current is %d, %s",
                    XOS_MM_FILE_NAME_LEN,result,nameBuffer);
                return XERROR;
            }
            if(XSUCC!=XosMMCalObjInfo(nameBuffer, &tempU32Value,pModule))
            {
                return XERROR;
            }
#ifdef XOS_VXWORKS
#if 0
            printf("Module BssSize:%-10d,TxtSize:%-10d,DataSize:%-10d\n",
                pModule->segSizeBss,pModule->segSizeText,
                pModule->segSizeData);
#endif
#endif
            objTotal += tempU32Value;
        }
        else if(0!=strncmp(dirName, "Global",6))
        {
            MMErr("Find an invalid Module Tag:%s",dirName);
            return XERROR;
        }
        /*��һ��ģ��*/
        XosSysRegEnumDir(hRootDir,hModuleDir, dirName, &hModuleDir);
        continue;
    }
    //printf("Done\n");
    gMMSystemInfo.moduleCount = moduleCount;
    gMMSystemInfo.taskCount   = taskCount;
    gMMSystemInfo.stackTotal  = stackTotalSize;
    gMMSystemInfo.objTotal    = objTotal;

    MMInfo("Total Module Count: %d",gMMSystemInfo.moduleCount);
    MMInfo("Total Task   Count: %d",gMMSystemInfo.taskCount);
    MMInfo("Total Stack  Size : %08x(%d)",gMMSystemInfo.stackTotal,gMMSystemInfo.stackTotal);
    MMInfo("Total Obj   Size  : %08x(%d)",gMMSystemInfo.objTotal,gMMSystemInfo.objTotal);
    if(gMMSystemInfo.moduleCount == 0)
    {
        MMErr("NO module Need to Start.\n");
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
������:    XosMMCalObjInfo
���ܣ�  ͳ��Ŀ���ļ���Ϣ
������
�����
���أ�
XSUCC      - successful
XERROR     - failed
˵����  ��Ҫ��VXWORKS����ϵͳ��ʹ��
************************************************************************/
XOS_MM_STATUS XosMMCalObjInfo(char* filename, XU32* pRetModSize, XOS_MM_MODULE_T *pModule)
{
    return XSUCC;
}


/************************************************************************
������:    XosMMGetModuleSegInfo
���ܣ�  ��ȡĿ���ļ�
������
�����
���أ�
XSUCC      - successful
XERROR     - failed
˵����  ��Ҫ��VXWORKS����ϵͳ��ʹ��
************************************************************************/
XOS_MM_STATUS XosMMGetModuleSegInfo(int fd, XU32 isFtpRead, XOS_MM_MODULE_T* pModule,XU32* totalSize)
{
#ifdef XOS_VXWORKS
    XU8        hdrBuf[sizeof(struct exec)+sizeof(Elf32_Ehdr)];
    struct exec*        pAoutExec;
    int ret;

#ifdef SHDRSZ /* SHDRSZ(ֻ֧�� tornado 2.2) */
    Elf32_Ehdr*            pElfHdr;
    Elf32_Shdr*            pElfSecHdr;
    XU8*                    pFilebuf=NULL;
    XU8*                 pFilebuf1=NULL;
    int sec_hdr_buf_size;
    int nextReadSize;
    int objTxtSize,objBssSize,objDataSize;
    int txtFound,bssFound,dataFound;
    int loop;
    char*   pSecHdrStr=NULL;
#endif
    /* Read segment info from a.out file */
    if(isFtpRead);
    /*    ret=XosftpLibRead(fd,hdrBuf,sizeof(struct exec)+sizeof(Elf32_Ehdr),5000);*/
    else
        ret=read(fd,hdrBuf,sizeof(struct exec)+sizeof(Elf32_Ehdr));
    if(ret<4) /*at least 4 bytes to decide whether the file is elf or a.out*/
    {
        MMErr("Read Ftp File Err:%d",ret);
        return XERROR;
    }
    if(strncmp(hdrBuf,ELFMAG,4)==0)
    {
        /*It is an elf file*/
        /*first get the all the section headers*/
#ifdef SHDRSZ /* SHDRSZ(only defined in tornado 2.2) */
        printf("File is ELF Format\n");
        if(ret<sizeof(Elf32_Ehdr))
        {
            MMErr("Read Ftp File Err:%d",ret);
            return XERROR;
        }
        pElfHdr = (Elf32_Ehdr*)hdrBuf;

        /*get total buffer size needed to read section header and allocate the buffer*/
        sec_hdr_buf_size = pElfHdr->e_shoff+pElfHdr->e_shnum*SHDRSZ;
        if(sec_hdr_buf_size<=ret)
        {
            /*already read all contents, this file is too small?*/
            MMErr("XosMMGetModuleSegInfo: OBJ File too small");
            return XERROR;
        }
        pFilebuf = malloc(sec_hdr_buf_size+100);
        if(pFilebuf==NULL)
        {
            MMErr("XosMMGetModuleSegInfo: malloc space to read elf content failed:size:%d",sec_hdr_buf_size);
            return XERROR;
        }

        /*Now buffer is allocated,
        copy the already read content into the buffer,
        read the rest of the file until the section headers are all read*/
        nextReadSize =sec_hdr_buf_size - ret;
        memcpy(pFilebuf,hdrBuf,ret);
#if 0
        if(isFtpRead)
            ret=WS_ftpLibRead(fd,pFilebuf+ret,nextReadSize,5000);
        else
            ret=read(fd,pFilebuf+ret,nextReadSize);
#endif
        if(ret<nextReadSize)
        {/*read error?????*/
            MMErr("Read Ftp File Err:%d",ret);
            return XERROR;
        }
        pElfSecHdr = (Elf32_Shdr*)(pFilebuf+pElfHdr->e_shoff);
        MMDbg("elf:string segment idx   :%d",pElfHdr->e_shstrndx);
        MMDbg("elf:string segment offset:0x%08x",
            (XU32)(pElfSecHdr[pElfHdr->e_shstrndx].sh_offset));

            /*now get the secHdrString section, we do not know wether the hdrStr section
        has already been read, if it hasn't we have to allocate memory to read it*/
        if(pElfSecHdr[pElfHdr->e_shstrndx].sh_offset<sec_hdr_buf_size)
        {
            pSecHdrStr = pFilebuf+pElfSecHdr[pElfHdr->e_shstrndx].sh_offset;
        }
        else
        {
            MMDbg("segHdr string table section is after sec_hdr:shstrndx:%08x,segHdr:%08x\n",
                (XU32)pElfSecHdr[pElfHdr->e_shstrndx].sh_offset,
                (XU32)pElfHdr->e_shoff);
            nextReadSize = pElfSecHdr[pElfHdr->e_shstrndx].sh_offset+
                pElfSecHdr[pElfHdr->e_shstrndx].sh_size- sec_hdr_buf_size;
            pFilebuf1 = malloc(nextReadSize+100);
            if(pFilebuf1==NULL)
            {
                MMErr("XosMMGetModuleSegInfo: malloc space to read elf content failed! size:%d",nextReadSize);
                return XERROR;
            }
#if 0
            if(isFtpRead)
                ret=WS_ftpLibRead(fd,pFilebuf1,nextReadSize,5000);
            else
                ret=read(fd,pFilebuf1,nextReadSize);
#endif
            if(ret<nextReadSize)
            {/*read error?????*/
                MMErr("Read Ftp File Err:%d",ret);
                return XERROR;
            }
            pSecHdrStr = pFilebuf1;
        }

        objTxtSize = objBssSize = objDataSize = 0;
        txtFound = 0;
        bssFound = 0;
        dataFound = 0;

        for(loop=0;loop<pElfHdr->e_shnum;loop++)
        {
            MMDbg("segment:%-10s;type:%04x;flag:%04x,addr:%08x;"
                "offset:%08x;size:%08x",
                pSecHdrStr+pElfSecHdr[loop].sh_name,
                (XU32)pElfSecHdr[loop].sh_type,
                (XU32)pElfSecHdr[loop].sh_flags,
                (XU32)pElfSecHdr[loop].sh_addr,
                (XU32)pElfSecHdr[loop].sh_offset,
                (XU32)pElfSecHdr[loop].sh_size);
            if(strncmp(pSecHdrStr+pElfSecHdr[loop].sh_name,".text",5)==0)
            {
                objTxtSize+=pElfSecHdr[loop].sh_size;
            }
            else if(strncmp(pSecHdrStr+pElfSecHdr[loop].sh_name,".rodata",7)==0)
            {
                objTxtSize+=pElfSecHdr[loop].sh_size;
            }
            else if(strncmp(pSecHdrStr+pElfSecHdr[loop].sh_name,".bss",4)==0)
            {
                objBssSize+=pElfSecHdr[loop].sh_size;
            }
            else if(strncmp(pSecHdrStr+pElfSecHdr[loop].sh_name,".data",5)==0)
            {
                objDataSize+=pElfSecHdr[loop].sh_size;
            }
        }
#if 0
        pModule->segSizeText = (objTxtSize);
        pModule->segSizeBss  = (objBssSize);
        pModule->segSizeData = (objDataSize);
        *totalSize = PAGE_ALIGN(pModule->segSizeText) +
            PAGE_ALIGN(pModule->segSizeBss) +
            PAGE_ALIGN(pModule->segSizeData);
#endif
        if(pFilebuf)
        {
            free(pFilebuf);
        }
        if(pFilebuf1)
        {
            free(pFilebuf1);
        }
        return XSUCC;
#else
        MMErr("Read Ftp File Err:%d",ret);
        return XERROR;
#endif /* SHDRSZ(only defined in tornado 2.2) */
    }
    else
    {
        /*It is an a.out file*/
        if(ret<sizeof(struct exec))
        {
            MMErr("Read hdr size less than elf header\n");
            return XERROR;
        }
        pAoutExec = (struct exec*)hdrBuf;
#if 0
        pModule->segSizeText = PAGE_ALIGN(pAoutExec->a_text);
        pModule->segSizeData = PAGE_ALIGN(pAoutExec->a_data);
        pModule->segSizeBss  = PAGE_ALIGN(pAoutExec->a_bss);
        *totalSize = pModule->segSizeText +
            pModule->segSizeData +
            pModule->segSizeBss;
#endif
        return XSUCC;
    }
#else
    return XSUCC;
#endif
}


/************************************************************************
������:    XosMMGetGlobalConfig
���ܣ�  ��ȡȫ�����ñ�������Ϣ
������
�����
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XOS_MM_STATUS XosMMGetGlobalConfig()
{
    HANDLE hModuleDir = (HANDLE)NULL;
    XS8 dirName[MAX_KEYNAME_LENGTH+2];    
    XS8 keyvalue[MAX_KEYVALUE_LENGTH] = {0};
    XU32 tempValue=0;

    MMInfo("Get global configuration,");

    memset(&gMMWsSsiInitPara,0,sizeof(gMMWsSsiInitPara));
    memset(dirName, 0, MAX_KEYNAME_LENGTH);

    XosSysRegEnumDir(XosSysRegGetRootDir(), (HANDLE)NULL, dirName, &hModuleDir);
    while(hModuleDir!=(HANDLE)NULL)
    {
        if(0 == strcmp(dirName, "Global"))
        {
            if(XERROR!=XosSysRegQueryU32KeyValue(hModuleDir,"SsiTotalTimer",&tempValue))
                gMMWsSsiInitPara.totalTmrCount= tempValue;
            else
                gMMWsSsiInitPara.totalTmrCount= 10000;

            if(XERROR!=XosSysRegQueryU32KeyValue(hModuleDir,"ModuleInitTimeout",&tempValue))
                gMMInitTimeout = tempValue;
            else
                gMMInitTimeout = DEFAULT_MODULE_INIT_TIME_OUT;
            if(XERROR!=XosSysRegQueryU32KeyValue(hModuleDir,"ModuleFailReset",&tempValue))
                gMMFailReset = tempValue;
            else
                gMMFailReset = TRUE;
            if(XERROR!=XosSysRegQueryU32KeyValue(hModuleDir,"ModuleManagerDebug",&tempValue))
                gMMDebugPrint = tempValue;
            else
                gMMDebugPrint = FALSE;

            if(XERROR!=XosSysRegQueryStrKeyValue(hModuleDir, "ModuleCpuBind", keyvalue))
                XOS_ParseCpuBindCfg(&gtCpuBindCfg, keyvalue);
            else
                gtCpuBindCfg.cpunum = 0;
            
        }
        XosSysRegEnumDir(XosSysRegGetRootDir(), hModuleDir, dirName, &hModuleDir);
    }
    MMInfo("Get global configuration done.");
    return XSUCC;
}


/************************************************************************
������:    XosMMModuleManagerRoot
���ܣ�  ģ�����װ�ص������������������װ�����е�ģ�顣
������
�����
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosMMModuleManagerRoot(void)
{
    XS32 retValue=0;
    XCHAR errMsg[255] = {0};

    XosModuleLibInit(NULL,0);
    XosMMSetCurrentLoadModule(0);

    /* ����ϵͳע��� */
    if (XSUCC != XosCreateSystemRegistryDatabase(gstrMMCfgFileName, gstrMMRootPath))
    {
        MMErr("Error creating system register database");
        XOS_Sprintf(errMsg, sizeof(errMsg) - 1, "XosMMModuleManagerRoot::Error creating system register database");
        goto FatalError;
    }

    /* ��ȡȫ������ */
    if(XSUCC != XosMMGetGlobalConfig())
    {
        MMErr("Error getting SSI global configuration");
        XOS_Sprintf(errMsg, sizeof(errMsg) - 1, "XosMMModuleManagerRoot::Error getting SSI global configuration");
        goto FatalError;
    }


    /* ���ý���cpu�� */
    if (XOS_SetCpuBind(&gtCpuBindCfg))
    {
        MMErr("Error set cpu bind\n");
    }

    /* ����ģ����Ϣ */
    if(XSUCC != XosMMCalModuleInfo())
    {
        MMErr("Error getting module info\n");
        XOS_Sprintf(errMsg, sizeof(errMsg) - 1, "XosMMModuleManagerRoot::Error getting module info");
        goto FatalError;
    }

    gMMWsSsiInitPara.moduleRegionSize = gMMSystemInfo.stackTotal+gMMSystemInfo.objTotal;

    /*�����ź���*/
    if (XERROR == XOS_SemCreate(&(gMMSystemInfo.sem), 1))
    {
        MMErr("Error create sem.\n");
        XOS_Sprintf(errMsg, sizeof(errMsg) - 1, "XosMMModuleManagerRoot::Error create sem");
        goto FatalError;
    }

    /*��ȡ�ź���*/
    XOS_SemGet(&(gMMSystemInfo.sem));
    
    /*  ��������ģ�� */
    retValue = XosMMCreateAllModules(errMsg, sizeof(errMsg));
    
     /*�ͷ��ź���*/
    if ( XSUCC != XOS_SemPut(&(gMMSystemInfo.sem) ))
    {
         XOS_SemDelete(&(gMMSystemInfo.sem));
         MMErr("Error put sem.\n");
         XOS_Sprintf(errMsg, sizeof(errMsg) - 1, "XosMMModuleManagerRoot::Error put sem");
         goto FatalError;
    }
    
    if(retValue != XSUCC)
    {
        goto FatalError;
    }
    goto NormalExit;

FatalError:
    dModuleCreateStatus = 0;
    write_to_syslog(errMsg);
    MMErr("System init has error,and will reboot!!!");
    //MM_taskSuspendCurrent();
    //reset();
    MMRebootCPU();
    return XERROR;

NormalExit:
    dModuleCreateStatus = 1;
    MMInfo("Module Manager Finished System Init!");
#ifdef XOS_VXWORKS
    MMInfo("|----------------------------------|");
    MMInfo("|-----start...........finished-----|");
    MMInfo("|-----------^(o'.'o)^--------------|");
    MMInfo("|--------O:)~~~~~~~~~(:O-----------|");
    MMInfo("|__________________________________|");
    XOS_CloseVxPrint();
    XOS_Sleep(2000);
    xos_hainit();
#endif

    return XSUCC;
}

/************************************************************************
������:    XosModuleInitWaitTask
���ܣ�  ���������������������ʵ��һ��ȷ��ģ�������ʼ���ɹ���ʧ�ܵĻ��ƣ�
��Ҫ��ģ����ں���������á���������ȴ�����ں�������������������Ϣ��
������  timeOut���ȴ���ʱ�䡣
�����
���أ�  ����������ش�������յ���Ϣ��
        MM_TASK_INIT_OK  --�ɹ�
        MM_TASK_INIT_FAIL--ʧ��
˵����
************************************************************************/
XU32 XosModuleInitWaitTask(int timeOut)
{
    XU32 ret = MM_TASK_INIT_OK;
#if 0     /* zhougs closed this 2006-8-24 */
    XU32 err;
#endif
    MMDbg("XosModuleInitWaitTask Wait For Task");
    if(gMMTaskHelpSemB==0||gMMTaskWaitMutex==0)
        return MM_TASK_INIT_OK;
    if(XSUCC!=XosMMsemBTake(gMMTaskHelpSemB,(long)timeOut))
    {
#if 0  /* zhougs closed this 2006-8-24 */
        err = WS_ErrorNoGet();
        if(err==WS_ERR_SEM_TIMEOUT)
        {
            MMErr("    Timeout when waiting task to init");
        }
#endif
        return MM_TASK_INIT_FAIL;
    }
    /*see task init result, use a mutex to protect the operation*/
    XosMMmutexTake(gMMTaskWaitMutex);
    if(gdMMTaskWaitResponse==MM_TASK_INIT_RESPONSE_FAIL)
    {
        /*task failed*/
        gdMMTaskWaitResponse = MM_TASK_INIT_RESPONSE_NULL;
        MMDbg("    XosModuleInitWaitTask Return FAIL from task %d",gMMTaskWaitTaskId);
        ret = MM_TASK_INIT_FAIL;
    }
    if(gdMMTaskWaitResponse==MM_TASK_INIT_RESPONSE_OK)
    {
        /*task inited ok*/
        gdMMTaskWaitResponse = MM_TASK_INIT_RESPONSE_NULL;
        MMDbg("    XosModuleInitWaitTask Return XSUCC from task%d",gMMTaskWaitTaskId);
        ret = MM_TASK_INIT_OK;
    }
    XosMMmutexGive(gMMTaskWaitMutex);
    return ret;
}


/************************************************************************
������:    XosModuleInitTaskOK
���ܣ�  �������ͨ���ź�����XosModuleInitWaitTask��������ͬ������Ϊ���ʱ��
XosModuleInitWaitTask�������ڵȴ��ź����׶Ρ����������񴴽��������������ǰ
��һ���ź����������ͻ���ģ���������XosModuleInitWaitTask�����Ϳ��Լ������У�
���ҷ��������ģ���������
������
�����
���أ� ��ʱ�򷵻�MM_TASK_INIT_RESPONSE_OK
˵���� �������ͨ��ȫ�ֱ���gdMMTaskWaitResponse���������
       ����ͨ��ȫ��gMMTaskWaitMutex�����Ĳ������б�����
************************************************************************/
void XosModuleInitTaskOK()
{
    char taskname[XOS_MM_TASK_NAME_LEN+0x10];
#ifdef XOS_VXWORKS
    char * ptrName = NULL;
#endif
    XOS_MM_TASK_ID  taskId;
    strcpy(taskname,"");
    taskId = XosMMTaskIdSelf();
    if(taskId==XOS_MM_INVALID_TASK_ID)
    {    /*not ssi 2.0 task*/
#ifdef XOS_VXWORKS
        /*get vxworks task name if in vxWorks env*/
        ptrName = taskName(taskIdSelf());
        if(NULL!=ptrName)
        {
            strncpy(taskname,ptrName,XOS_MM_TASK_NAME_LEN);
            taskname[XOS_MM_TASK_NAME_LEN-1]=0;
        }
#else
        /* on other operating system give a NOT-SSI-TSK*/
        strcpy(taskname,"NON-SSI-TSK");
#endif
    }

    if(gMMTaskHelpSemB==MM_INVALID_SEMB||gMMTaskWaitMutex==MM_INVALID_MUTEX)
        return;
    XosMMmutexTake(gMMTaskWaitMutex);
    if(gdMMTaskWaitResponse!=MM_TASK_INIT_RESPONSE_NULL)
    {
        MMErr("    Call task ok a second time, Suspend current task    ");
        MM_taskSuspendCurrent();
    }
    gdMMTaskWaitResponse = MM_TASK_INIT_RESPONSE_OK;
    gMMTaskWaitTaskId = XosMMTaskIdSelf();
    XosMMmutexGive(gMMTaskWaitMutex);

    if(XSUCC!=XosMMsemBGive(gMMTaskHelpSemB))
    {
        MMErr("    XosModuleInitTaskOK give semphore failed");
    }
}


/************************************************************************
������:    XosModuleInitTaskFail
���ܣ�  �������ͨ���ź�����XosModuleInitWaitTask��������ͬ������Ϊ���ʱ��
XosModuleInitWaitTask�������ڵȴ��ź����׶Ρ����������񴴽��������������ǰ
��һ���ź����������ͻ���ģ���������XosModuleInitWaitTask�����Ϳ��Լ������У�
���ҷ��������ģ���������
������
�����
���أ� ��ʱ�򷵻�MM_TASK_INIT_RESPONSE_FAIL
˵���� �������ͨ��ȫ�ֱ���gdMMTaskWaitResponse���������
       ����ͨ��ȫ��gMMTaskWaitMutex�����Ĳ������б�����
************************************************************************/
void XosModuleInitTaskFail()
{
    char taskname[XOS_MM_TASK_NAME_LEN+0x10];
#ifdef XOS_VXWORKS
    char * ptrName = NULL;
#endif
    XOS_MM_TASK_ID  taskId;
    strcpy(taskname,"");
    taskId = XosMMTaskIdSelf();
    if(taskId==XOS_MM_INVALID_TASK_ID)
    {    /*not ssi 2.0 task*/
#ifdef XOS_VXWORKS
        /*get vxworks task name if in vxWorks env*/
        ptrName = taskName(taskIdSelf());
        if(NULL!=ptrName)
        {
            strncpy(taskname,ptrName,XOS_MM_TASK_NAME_LEN);
            taskname[XOS_MM_TASK_NAME_LEN-1]=0;
        }
#else
        /* on other operating system give a NOT-SSI-TSK*/
        strcpy(taskname,"NON-SSI-TSK");
#endif
    }

    if(gMMTaskHelpSemB==0||gMMTaskWaitMutex==0)
        return;
    XosMMmutexTake(gMMTaskWaitMutex);
    if(gdMMTaskWaitResponse!=MM_TASK_INIT_RESPONSE_NULL)
    {
        MMErr("    Call taskfail a second time, Suspend current task");
        MM_taskSuspendCurrent();
    }
    gdMMTaskWaitResponse = MM_TASK_INIT_RESPONSE_FAIL;
    gMMTaskWaitTaskId = XosMMTaskIdSelf();

    XosMMmutexGive(gMMTaskWaitMutex);
    if(XSUCC!=XosMMsemBGive(gMMTaskHelpSemB))
    {
        MMErr("    XosModuleInitTaskFail give semphore failed");
    }
}


/************************************************************************
������:    XosModuleManagerInit
���ܣ�  ��ʼ��ģ�����װ������ϵͳ����Ҫ������¹��ܣ���ʼ��ȫ���ź�����
��ʼ��ȫ�ֻ����ź���������ģ�������������
������
bootRootPath���ַ���boot�ļ�·��
cfgFileName�� �����ļ�
�����
���أ�
˵����
************************************************************************/
XS32 XosModuleManagerInit (XS8 * bootRootPath, XS8 * cfgFileName)
{
    XS32  ret; 
#if  0
    HANDLE tempTaskId=0;
#endif

    //�Ȳ�����ʼ��
#ifdef VXWORKS           
    if(XSUCC != Patch_init())
    {
        MMInfo("Patch_init() failed,ignore!\n");
    }
#endif

    /* ������Ҫģ����������ź���������ź������û�ͨ��APIʹ�� */
    gMMTaskHelpSemB = XosMMsemBCreate(MM_SEM_B_INIT_EMPTY);
    if(MM_INVALID_SEMB == gMMTaskHelpSemB)
    {
        MMErr("Create Semphore for TaskInitWait failed");
        goto MM_MAIN_INIT_ERROR;
    }

    gMMTaskWaitMutex = XosMM_mutexCreate();
    if(MM_INVALID_MUTEX== gMMTaskWaitMutex)
    {
        MMErr("Create Mutex for sysRegTable failed");
        goto MM_MAIN_INIT_ERROR;
    }

    /* ģ�������������  */
    ret = XosMMModuleManagerRoot();
    if(ret != XSUCC)
    {
        goto MM_MAIN_INIT_ERROR;
    }

#if 0
    /*  ģ�������������,ͨ�����Բ�ͬ�Ľ��̿ռ������Է��ű�������⣬�����һ���ģ�
    �������Ҫ���ҵķ��ű�.o���������������һ�顣  */
    ret = XOS_TaskCreate("tMMan", 100, 1024*1024*2,
        XosMMModuleManagerRoot, (XVOID*)NULL, &(tempTaskId));
    if(ret != XSUCC)
    {
        MMErr("Create XOS_TaskCreate->create task failed !  tskName:tMMan");
        goto MM_MAIN_INIT_ERROR;
    }
#endif

    return XSUCC;

MM_MAIN_INIT_ERROR:
    if(gMMFailReset==1)
    {
        MMErr("There is error while initializing ModuleManager.");
        MM_taskSuspendCurrent();
    }
    return XERROR;
}


/************************************************************************
������:    XosMMInitModuleList
���ܣ����ģ���б��ȫ���ڴ档
���룺��
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XOS_MM_STATUS XosMMInitModuleList()
{
    memset(gMMModuleList, 0, sizeof(XOS_MM_MODULE_T)*(MAX_MODULE_NUMBER+2));
    return XSUCC;
}


/************************************************************************
������:    XosMMReadModuleInitPara
���ܣ�  ��ȡģ���ʼ��������
������
hDir:ϵͳע���ĸ�Ŀ¼
pModInitPara:����ģ���ʼ�����ݽṹ
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XOS_MM_STATUS XosMMReadModuleInitPara(HANDLE hDir, XOS_MM_MOD_INIT_PARA* pModInitPara)
{
    XS8 nameBuffer[MAX_KEYVALUE_LENGTH+MAX_KEYVALUE_LENGTH+10] = "";
    XS8 objFileName[MAX_KEYVALUE_LENGTH+12] = "";
    XS8 objFilePath[MAX_KEYVALUE_LENGTH+12] = "";
    XS8 memPoolNameBuf[MAX_KEYVALUE_LENGTH+12];
    int result;
    XU32 tempU32Value;
    memset(pModInitPara,0,sizeof(XOS_MM_MOD_INIT_PARA));

    /*��ȡģ������*/
    result = XosSysRegQueryStrKeyValue(hDir, MODULENAMEKEY,nameBuffer);
    if(result==XSUCC)
    {
        strncpy((char*)pModInitPara->aModuleName,nameBuffer,XOS_MM_MODULE_NAME_LEN);
        pModInitPara->aModuleName[XOS_MM_MODULE_NAME_LEN-1]=0;
    }
    else
    {
        strncpy((char*)pModInitPara->aModuleName,"ModuleNoName",XOS_MM_MODULE_NAME_LEN);
        pModInitPara->aModuleName[XOS_MM_MODULE_NAME_LEN-1]=0;
    }
    MMInfo("Begin creating %s, please wait...", pModInitPara->aModuleName);

    strcpy(nameBuffer,"");
    /* ��ȡĿ���ļ�����Ŀ���ļ�·�� */
    result = XosSysRegQueryStrKeyValue(hDir, OBJFILEPATHKEY, objFilePath);
    if(result==XSUCC)
    {
        strcat(nameBuffer,objFilePath);
    }
    result = XosSysRegQueryStrKeyValue(hDir, OBJFILENAMEKEY, objFileName);
    if(result==XSUCC)
    {
        strcat(nameBuffer,objFileName);
    }

    result = (XU32)strlen(nameBuffer);
    if(result>=XOS_MM_FILE_NAME_LEN)
    {
        MMErr("OBJName Too Long: Max is %d, current is %d, %s",
            XOS_MM_FILE_NAME_LEN,result,nameBuffer);
        MMErr("Create %s failed.",pModInitPara->aModuleName);
        return XERROR;
    }
    strcpy((char*)pModInitPara->aObjFileName,nameBuffer);
    MMInfo("ObjFileName: %s", (char*)pModInitPara->aObjFileName);


    /* ��ȡģ��ID */
    if(XSUCC==XosSysRegQueryU32KeyValue(hDir, KEYNAME_MODULE_ID, &tempU32Value))
    {
        pModInitPara->nModuleId     = tempU32Value;
        MMInfo("ModuleId: %d", pModInitPara->nModuleId);
    }
    else
    {
        MMErr("Must Define Module Id for SSI2.0 use!!");
        MMErr("Create %s failed.",pModInitPara->aModuleName);
        return XERROR;
    }
    /* ��ȡ������ */
    if(XSUCC==XosSysRegQueryU32KeyValue(hDir, KEYNAME_TASKCOUNT,&tempU32Value))
    {
        pModInitPara->maxTaskNum     = tempU32Value;
        pModInitPara->nMaxStackSize =tempU32Value*pModInitPara->nMaxStackSize;
        MMInfo("TaskNum: %d, StackSize:%d", pModInitPara->maxTaskNum, pModInitPara->nMaxStackSize );
    }
    /* ��ȡ�ڴ������ */
    if(XSUCC == XosSysRegQueryStrKeyValue(hDir, KEYNAME_MEM_POOL_NAME,memPoolNameBuf))
    {
        strncpy((char*)pModInitPara->memPoolName,memPoolNameBuf,XOS_MM_MEM_POOL_NAME_MAX);
        pModInitPara->memPoolName[XOS_MM_MEM_POOL_NAME_MAX-1]=0;

        MMInfo("memPoolName: %s",pModInitPara->memPoolName); 
    }
    /* ��ȡliner heap */
    if(XSUCC==XosSysRegQueryU32KeyValue(hDir, KEYNAME_LINEAR_HEAP,&tempU32Value))
        pModInitPara->bLinearHeap     = tempU32Value;
    else
        pModInitPara->bLinearHeap     = 0;

    MMInfo("LinearHeap: %d", pModInitPara->bLinearHeap);

    /* ��ȡstack size */
    if(XSUCC==XosSysRegQueryU32KeyValue(hDir, KEYNAME_STACKSIZE,&tempU32Value)) {
        pModInitPara->nMaxStackSize = tempU32Value;
    }

    MMInfo("pModInitPara->maxstackSize: %u", pModInitPara->nMaxStackSize);

    /* ��ȡ MsgQueNum */
    if(XSUCC == XosSysRegQueryU32KeyValue(hDir, KEYNAME_MSGQUE_NUM, &tempU32Value))
        pModInitPara->nMsgQueNum = tempU32Value;
    else
        pModInitPara->nMsgQueNum = 0;

    MMInfo("MsgQueNum: %u", pModInitPara->nMsgQueNum);

    return XSUCC;
}


/************************************************************************
������:    XosMMCreateModule
���ܣ�  ��ȡģ���ʼ������������ģ�����ݽṹ��
������
hDir:ϵͳע���ĸ�Ŀ¼
pModule:����ģ�����ݵĵ�ַ
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosMMCreateModule(HANDLE hDir, XOS_MM_MODULE_T *pModule)
{
    XS8 entryFuncName[MAX_KEYVALUE_LENGTH+12] = "_";    
    XS8 cpubindvalue[MAX_KEYVALUE_LENGTH] = {0};
    XS32 paraNum = 0;
    MODULE_ENTRY pFunc = NULL;
    XU32 initTime = 60;
    XS32 result = XSUCC;
    XOS_MM_MOD_INIT_PARA modInitPara;
    int symType;
    XPOINT symAddr;

    if(pModule==(HANDLE)NULL||hDir==(HANDLE)NULL)
        return XERROR;

    /*��ȡģ���ʼ������*/
    if(XosMMReadModuleInitPara(hDir,&modInitPara)!=XSUCC)
    {
        return XERROR;
    }

    XOS_MemCpy(&pModule->wsModInitPara, &modInitPara,sizeof(XOS_MM_MOD_INIT_PARA));
    
    strncpy((char*)pModule->modName,(char*)modInitPara.aModuleName,XOS_MM_MODULE_NAME_LEN);
    pModule->modName[XOS_MM_MODULE_NAME_LEN-1]=0;
    pModule->wsModuleId   = modInitPara.nModuleId;

    if(XSUCC!=XosModuleInit(&modInitPara))
    {
        MMErr("Create %s failed.",modInitPara.aModuleName);
        return XERROR;
    }

    /* ��ȡ��ں�������*/
    memset(entryFuncName, 0, MM_MAX_ENTRYFUNCTION_NAME+1);
    result = XosSysRegQueryStrKeyValue(hDir, OBJENTRYFUNCKEY, entryFuncName);
    if(result!=XSUCC)
    {
        MMErr("A Module Must have a Entry Function Tag");
        MMErr("Create %s failed.",modInitPara.aModuleName);
        return XERROR;
    }
    /*������ں�����ַ*/
    if(XSUCC!=XosMM_findSymbByName(entryFuncName,&symType,&symAddr))
    {
        MMErr("Create %s failed.",modInitPara.aModuleName);
        return XERROR;
    }
    else
    {
        MMInfo("Find  Entry Function %s OK!",entryFuncName);
    }
    if(symType!=MM_SYMB_TYP_FUNC)
    {
        MMErr("Entry Tag %s is not a function",entryFuncName);
        MMErr("Create %s failed.",modInitPara.aModuleName);
        return XERROR;
    }
    pFunc = (MODULE_ENTRY)symAddr;
    /*��ȡģ���������*/
    {
        sys_reg_key_t * pKey = NULL;
        HANDLE hParameterDir = (HANDLE)NULL;
        XS8 value[MAX_KEYVALUE_LENGTH];
        XS8 paraName[MAX_KEYNAME_LENGTH];

        /*get parameters num from system registry table*/
        result = XosSysRegOpenDir(hDir, &hParameterDir, "Parameter");
        if(XSUCC == result)
        {
            XosSysRegEnumKey(hParameterDir, (HANDLE)NULL, value, paraName, (HANDLE *)&pKey);
            paraNum = 0;
            /*enum parameter and store parameter value into argv*/
            while(NULL != pKey)
            {
                paraNum++;
                XosSysRegEnumKey(hParameterDir, (HANDLE)pKey, value, paraName, (HANDLE *)&pKey);
            }
        }
        else
        {
            paraNum = 0;
        }
    }

    /*��ȡģ���ʼ����ʱʱ��*/
    result = XosSysRegQueryU32KeyValue(hDir, "InitTimeout", &initTime);
    if(XERROR == result)
    {
        /*module does not have a manual timeout value, use the default*/
        initTime = gMMInitTimeout;
    }

    
    /* ģ���cpu����Ϣ */
    if(XERROR!=XosSysRegQueryStrKeyValue(hDir, KEYNAME_CPU_BIND, cpubindvalue))
        XOS_ParseCpuBindCfg(&pModule->module_cpu_bind, cpubindvalue);
    else
        pModule->module_cpu_bind.cpunum = 0;


    /*����ģ�������Ϣ */
    pModule->pEntryFunc = pFunc;
    pModule->modHDir = hDir;
    pModule->num_parameter = paraNum;
    pModule->modInitTime = initTime;
    pModule->module_init_flag = MODULE_INIT_NOT_START;

    MMInfo("Create %s succeed.",modInitPara.aModuleName);
    return XSUCC;
}


/************************************************************************
������:    XosMMStrTrim
���ܣ�
������
�������
���أ�
˵����
************************************************************************/
int XosMMStrTrim(char * strToTrim, char charPattern)
{
    int strLenth = (XS32)strlen(strToTrim);
    strLenth--;
    while(strLenth>=0&&strToTrim[strLenth]==charPattern)
    {
        strToTrim[strLenth]=0;
        strLenth--;
    }
    return strLenth+1;
}


/************************************************************************
������:    XosMMStrTrimSpace
���ܣ�
������
�������
���أ�
˵����
************************************************************************/
int XosMMStrTrimSpace(char * strToTrim)
{
    int strLenth = (XS32)strlen(strToTrim);
    strLenth--;
    while(strLenth>=0&&(strToTrim[strLenth]==' '||strToTrim[strLenth]=='\t'))
    {
        strToTrim[strLenth]=0;
        strLenth--;
    }
    return strLenth+1;
}


/************************************************************************
������:    XosMMStrFindChar
���ܣ�
������
�������
���أ�
˵����
************************************************************************/
int XosMMStrFindChar(char* strToLkup,char charPattern,int outInvComma)
{
    int strIdx=0;
    int loopMax = (XS32)strlen(strToLkup);
    int outsideComma;

    if(outInvComma)
    {
        outsideComma = TRUE;
        while(strIdx<loopMax)
        {
            if(outsideComma&&(charPattern == strToLkup[strIdx]))
                return strIdx;
            if('"'==strToLkup[strIdx])
                outsideComma = !outsideComma;
            strIdx++;
        }
    }
    else
    {
        while(strIdx<loopMax)
        {
            if(charPattern == strToLkup[strIdx])
                return strIdx;
            strIdx++;
        }
    }
    return -1;
}


/************************************************************************
������:    XosMMStrFindOtherChar
���ܣ�
������
�������
���أ�
˵����
************************************************************************/
int XosMMStrFindOtherChar(char* strToLkup,char charPattern)
{
    int strIdx=0;
    int loopMax = (XS32)strlen(strToLkup);
    while(strIdx<loopMax)
    {
        if(charPattern != strToLkup[strIdx])
            return strIdx;
        strIdx++;
    }
    return -1;
}


/************************************************************************
������:    XosMMStrFindNoSpace
���ܣ�
������
�������
���أ�
˵����
************************************************************************/
int XosMMStrFindNoSpace(char* strToLkup)
{
    int strIdx=0;
    int loopMax = (XS32)strlen(strToLkup);
    while(strIdx<loopMax)
    {
        if(' ' != strToLkup[strIdx]&&'\t'!=strToLkup[strIdx])
            return strIdx;
        strIdx++;
    }
    return -1;
}


/************************************************************************
������:    XosMMStrFindSpace
���ܣ�
������
�������
���أ�
˵����
************************************************************************/
int XosMMStrFindSpace(char* strToLkup)
{
    int strIdx=0;
    int loopMax = (XS32)strlen(strToLkup);
    while(strIdx<loopMax)
    {
        if(' '  == strToLkup[strIdx]||'\t'==strToLkup[strIdx])
            return strIdx;
        strIdx++;
    }
    return -1;
}


#if 0
int XosMMStrFindChar(char* strToLkup,char charPattern)
{
    int strIdx=0;
    int loopMax = strlen(strToLkup);
    while(strIdx<loopMax)
    {
        if(charPattern == strToLkup[strIdx])
            return strIdx;
        strIdx++;
    }
    return -1;
}
#endif


/************************************************************************
������:    XosMMStrFindLastChar
���ܣ�
������
�������
���أ�
˵����
************************************************************************/
int XosMMStrFindLastChar(char* strToLkup,char charPattern,int outInvComma)
{
    int strIdx;
    int loopMax;
    int outsideComma;
    int lastAppearIdx=-1;
    if(outInvComma)
    {
        strIdx = 0;
        loopMax = (XS32)strlen(strToLkup);
        outsideComma = TRUE;
        while(strIdx<loopMax)
        {
            if(outsideComma&&charPattern==strToLkup[strIdx])
                lastAppearIdx = strIdx;
            if('"'==strToLkup[strIdx])
                outsideComma = !outsideComma;
            strIdx++;
        }
    }
    else
    {
        strIdx = (XS32)strlen(strToLkup)-1;
        while(strIdx>=0)
        {
            if(charPattern == strToLkup[strIdx])
                return strIdx;
            strIdx--;
        }
    }
    return lastAppearIdx;
}


/************************************************************************
������:    XosMMRunModule
���ܣ�  ����ģ��
������  pModule��ģ����Ϣ�ṹ
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosMMRunModule(XOS_MM_MODULE_T * pModule)
{
    XS8 **argv = NULL;
    XS8 paraName[MAX_KEYNAME_LENGTH + 1];
    HANDLE hParameterDir = (HANDLE)NULL;
    sys_reg_key_t *pKey = NULL;
    XU32 i = 0;
    XU32 result = MODULE_INIT_OK;
    XS32 ret_Value = XSUCC;
    XU32 level = 0;

    if(NULL == pModule)
        return XERROR;

    if(NULL == pModule->pEntryFunc)
        return XERROR;
    pModule->module_init_flag = MODULE_INIT_RUNNING;

    MMInfo("Begin to run %s...", pModule->modName);

    /*�����ڴ����洢����*/
    if(0 < pModule->num_parameter)
    {
        argv = (XS8**)malloc((sizeof(XS8*))*pModule->num_parameter);
        for(i = 0; i < pModule->num_parameter; i++)
        {
            argv[i] = (XS8*)malloc(MAX_KEYVALUE_LENGTH + 1);
            memset(argv[i], 0, MAX_KEYVALUE_LENGTH);
        }
    }
    /*��ϵͳע����л�ȡ������Ϣ*/
    ret_Value = XosSysRegOpenDir(pModule->modHDir, &hParameterDir, "Parameter");
    if(ret_Value != XSUCC)
    {
        MMWarn("This module has no Parameter!");
    }
    if((HANDLE)NULL != hParameterDir)
    {
        /*ö�ٲ������Ұ�ֵ�洢��argv������*/
        XosSysRegEnumKey(hParameterDir, (HANDLE)NULL, argv[0], paraName, (HANDLE*)&pKey);
        for(i=1; (NULL != pKey) && (i < pModule->num_parameter); i++)
        {
            XosSysRegEnumKey(hParameterDir, (HANDLE)pKey, argv[i], paraName, (HANDLE *)&pKey);
        }
    }
    ret_Value = XosSysRegQueryU32KeyValue(pModule->modHDir, "DefaultTraceLel", &level);
    if(XSUCC == ret_Value)
    {
        gMMFidTraceLel = level;
    }
    else
    {
        gMMFidTraceLel = 0xffffffff;
    }
    {
        sys_reg_key_t *pShCmd = NULL;
        XS8 shellCmd[MAX_KEYVALUE_LENGTH + 1];
        XS8 *cmdName = paraName;
        /*����ں���֮ǰ����shell����*/
        XosSysRegEnumKey(pModule->modHDir, (HANDLE)NULL, shellCmd, cmdName, (HANDLE *)&pShCmd);
        while(NULL != pShCmd)
        {
            if(0 == strcmp(cmdName, OBJENTRYFUNCKEY))
                break;
            if(SYSREG_SHELLKEY((HANDLE)pShCmd))
            {
                /*ȷ��shell�����һ���ո�" " �ַ�������Ϊ��ᵼ���������*/
                XosMMStrTrim(shellCmd, ' ');
                if(strlen(shellCmd)!=0)
                {
                    MMInfo("Shell Cmd:\"%s!\"\n",shellCmd);
                    XosMMExecuteShellStr(shellCmd);
                }
            }
            XosSysRegEnumKey(pModule->modHDir, (HANDLE)pShCmd, shellCmd, cmdName, (HANDLE *)&pShCmd);
        }

        if(pModule->pEntryFunc==NULL)
        {
            if(0 < pModule->num_parameter)
            {
                for(i = 0; i < pModule->num_parameter; i++)
                {
                    free(argv[i]);
                }
                free(argv);
            }
            MMErr("Null EntryFunc");
            return XERROR;
        }
        else
        {
            /*������ں���*/
            result = (*(pModule->pEntryFunc))(hParameterDir, pModule->num_parameter, argv);
        }
        /*����ں���֮������shell����*/
        while(NULL != pShCmd)
        {
            if(SYSREG_SHELLKEY((HANDLE)pShCmd))
            {
                XosMMStrTrim(shellCmd, ' ');
                if(strlen(shellCmd)!=0)
                {
                    XosMMExecuteShellStr(shellCmd);
                }
            }
            XosSysRegEnumKey(pModule->modHDir, (HANDLE)pShCmd, shellCmd, cmdName, (HANDLE*)&pShCmd);
        }
    }
    if(0 < pModule->num_parameter)
    {
        for(i = 0; i < pModule->num_parameter; i++)
        {
            free(argv[i]);
        }
        free(argv);
    }

    if((result>=0)&&(result<5))
        MMInfo("%s run over. State %s",pModule->modName,gstrModuleInitFlag[result]);
    return result;
}


/************************************************************************
������:    XosMMCreateAllModules
���ܣ�  ��������ģ��
������  pModule��ģ����Ϣ�ṹ
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosMMCreateAllModules(XCHAR* errMsg, XS32 msgSize)
{
    XS32 i = 0;
    XS32 result = XSUCC;
    XOS_MM_MODULE_T* pModule;

    MMInfo("Begin creating modules, please wait...");
    printf("\n-----------------------------------------------------------\n");
    for(i = 0; i <= MAX_MODULE_NUMBER;i++)
    {
        pModule = &gMMModuleList[i];
        /*�鿴ϵͳע���Ŀ¼�Ƿ���ģ��Ŀ¼������ǣ��ʹ���ģ�顣*/
        if(pModule->modHDir)
        {
            result = XosMMCreateModule(pModule->modHDir, &(gMMModuleList[i]));
            if(XSUCC != result)
            {
                if(NULL != errMsg && msgSize > 0) {
                    XOS_Sprintf(errMsg, msgSize - 1, "Create Module %s fail!!", pModule->modName);
                }
                return result;
            }
            if(gMMModuleList[i].pEntryFunc==NULL)
                break;
            /* set gdCurrentLoadModuleID; modified by jeff.zheng 2005.5*/
            XosMMSetCurrentLoadModule((HANDLE)i);

            /* Start up this module */
            XosMMSetCurLdModId(gMMModuleList[i].wsModuleId);
            result = XosMMRunModule( &gMMModuleList[i]);

            gMMModuleList[i].module_init_flag = result;
            if( MODULE_INIT_OK != result )
            {
                if(errMsg && msgSize > 0) {
                    XOS_Sprintf(errMsg, msgSize - 1, "Run Module %s fail!!", pModule->modName);
                }
                goto FatalError;
            }
#ifdef XOS_LONG_SLEEP
            XosMSMMtaskSleep(200);
#endif
            printf("\r\n----------------Continue with Next Module------------------\n");
        }
    }
#ifdef XOS_LONG_SLEEP
    XosMSMMtaskSleep(500);
#endif
    goto NormalExit;
FatalError:
    MMErr("There are fatal errors while running module \"%s\"",
        gMMModuleList[i].modName);
    return XERROR;

NormalExit:
    return XSUCC;
}


/************************************************************************
������:    XosMMGetModuleInitTime
���ܣ�  ��ȡģ���ʼ��ʱ��
������  pModule��ģ����Ϣ�ṹ
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosMMGetModuleInitTime(XOS_MM_MODULE_T * pModule)
{
    return pModule->modInitTime;
}


/************************************************************************
������:    XosMMShowAllModule
���ܣ�  ��ʾ����ģ����Ϣ
������
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosMMShowAllModule()
{
    int i = 0;

    for(i = 0; i < MAX_MODULE_NUMBER; i++)
    {
        XosMMShowModuleInfo(i);
    }
    return XSUCC;
}


/************************************************************************
������:    XosMMShowInitError
���ܣ�  ��ʾ��ʼ������ģ����Ϣ
������
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosMMShowInitError()
{
    int i = 0;
    for(i = 0; i < MAX_MODULE_NUMBER; i++)
    {
        if((gMMModuleList[i].module_init_flag>MODULE_INIT_OK&&
            gMMModuleList[i].module_init_flag<MODULE_INIT_NOT_START)||
            gMMModuleList[i].module_init_sys_err_no!=0)
        {
            XosMMShowModuleInfo(i);
        }
    }
    return XSUCC;
}


/************************************************************************
������:    XosMMShowModuleInfo
���ܣ�  ��ʾ����ģ����Ϣ
������
�������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
void XosMMShowModuleInfo(XU32 modIndex)
{
    if(modIndex>MAX_MODULE_NUMBER)
        printf("Index Exceed Limit\n");
    if(NULL == gMMModuleList[modIndex].pEntryFunc)
    {
        return;
    }
    printf("Module Index            : %d\n", modIndex);
    /*  printf("Module Name             : %s\n", gMMModuleList[modIndex].name);
    printf("Module ID               : 0x%08x\n", gMMModuleList[modIndex].module_id);
    printf("Module Entry            : %s\n", gMMModuleList[modIndex].entry_function_name);
    printf("Module Entry Address    : 0x%08x\n", (XU32)gMMModuleList[modIndex].pEntryFunc);
    printf("Module OBJ File Name    : %s\n", gMMModuleList[modIndex].module_objfile_name);
    printf("Module OBJ File Path    : %s\n", gMMModuleList[modIndex].module_objfile_path);
    printf("Module Parameters Number: %d\n", gMMModuleList[modIndex].num_parameter);
    */
    printf("Module Init State: %s\n", gstrModuleInitFlag[gMMModuleList[modIndex].module_init_flag]);
    printf("Module Init Errno: 0x%x\n", gMMModuleList[modIndex].module_init_sys_err_no);
}


#ifdef XOS_WIN32
void MMInfo(char* format,...)
{
    char buffer[1024];
    va_list arglist;
    va_start( arglist, format );
    XOS_VsPrintf(buffer, sizeof(buffer), format,arglist);
    buffer[sizeof(buffer)-1] = 0;
    va_end( arglist );

#if 0
    _vsnprintf( buffer, sizeof(buffer), format, arglist );
    buffer[sizeof(buffer)-1] = 0;
#endif

    printf("[MMInfo]:%s\n",buffer);
}


void MMDbg(char* format,...)
{
    if(gMMDebugPrint)
    {
        char buffer[384];
        va_list arglist;
        va_start( arglist, format );
        XOS_VsPrintf(buffer, sizeof(buffer), format,arglist);
        buffer[sizeof(buffer)-1] = 0;
        va_end( arglist );

#if 0
        _vsnprintf( buffer, sizeof(buffer), format, arglist );
        buffer[sizeof(buffer)-1] = 0;
#endif

        printf("[MMDbg ]:%s\n",buffer);
    }
}


void MMWarn(char* format,...)
{
    char buffer[384];
    va_list arglist;
    va_start( arglist, format );
    
    XOS_VsPrintf(buffer, sizeof(buffer), format,arglist);
    buffer[sizeof(buffer)-1] = 0;
    va_end( arglist );

#if 0
    va_list arglist;
    va_start( arglist, format );
    _vsnprintf( buffer, sizeof(buffer), format, arglist );
    buffer[sizeof(buffer)-1] = 0;
    va_end( arglist );
#endif

    printf("[MMWarn]:%s\n",buffer);
}


void MMErr(char* format,...)
{
    char buffer[384];
    va_list arglist;
    va_start( arglist, format );
    XOS_VsPrintf(buffer, sizeof(buffer), format,arglist);
    buffer[sizeof(buffer)-1] = 0;
    va_end( arglist );

#if 0
    _vsnprintf( buffer, sizeof(buffer), format, arglist );
    buffer[sizeof(buffer)-1] = 0;
#endif

#ifdef XOS_VXWORKS
    printf("\033[1;31m[MMErr ]:%s\033[0m\n",buffer);
#else
    printf("[MMErr ]:%s\n",buffer);
#endif

}

#endif      /* #ifdef XOS_WIN32 */


/************************************************************************
������:    XosMMinitSymbTbl
���ܣ�  ����ִ���ļ�����ʼ��ϵͳ���ű���Ϣ������ļ���linux��windows
        ����ϵͳ������Ҫ��Vxworks���治��Ҫ��
������  execFileName��ִ���ļ�����
�����  ��
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
#if defined(_MSC_VER) && (_MSC_VER > 1200)
extern void XOS_CfgForSym(void);
extern void XOS_NtpcForSym(void);
#endif
int XosMMinitSymbTbl(char* execFileName)
{
#ifdef XOS_WIN32
    int err;
#if defined(_MSC_VER) && (_MSC_VER > 1200)
    //SymSetOptions(SYMOPT_UNDNAME);// | SYMOPT_DEFERRED_LOADS);
    // �����������һ�£�����xoscfg.c������ؽ���
    XOS_CfgForSym();
    XOS_NtpcForSym();
#endif

    /*��ʼ����ǰ���̵�WINDOWSϵͳ���ű�*/
#if defined(_MSC_VER) && (_MSC_VER > 1200)
    if(!SymInitialize(GetCurrentProcess(),NULL,FALSE))
#else
    if(!SymInitialize(GetCurrentProcess(),"",FALSE))
#endif
    {
        err=GetLastError();
        MMErr("Failed Initailize Process Symbol table,XOS_WIN32 Errcd:%d",err);
        return XERROR;
    }
    /*װ�ص�ǰ���̵ķ��ű�WINDOWSϵͳ*/
#if defined(_MSC_VER) && (_MSC_VER > 1200)
    if(!SymLoadModule64(GetCurrentProcess(),NULL,execFileName,NULL,0,0))
#else
    if(!SymLoadModule(GetCurrentProcess(),NULL,execFileName,NULL,0,0))
#endif
    {
        err=GetLastError();
        MMErr("Failed load symbol filename:%s,XOS_WIN32 Errcd:%d",execFileName,err);
        return XERROR;
    }
#endif

#if (defined(XOS_LINUX) || defined(XOS_SOLARIS))
    bfd *abfd;
    const char **targets, **pp;
    char **matching;
    long symbol_mem_size;
    long i;
    struct bfd_mm_hash_entry* pNewHashEntry;
    /*call bfd_init only once here*/
    bfd_init();
    MMInfo("bfd supported targets: ");
    targets = bfd_target_list();
    for (pp = targets; *pp != NULL; pp++)
    {
        MMInfo("%s ", *pp);
    }
    free(targets);
     
    abfd = NULL;
    abfd = bfd_openr (execFileName, NULL);

    if (abfd == NULL)
    {
        const char *errmsg = bfd_errmsg( bfd_get_error() );
        MMErr("after bfd_openr: abfd == NULL; "
            "bfd error = %s,filename:%s\n", errmsg,execFileName);
        exit(1);
    }
    MMInfo("bfd_openr successfully! filename:%s ; start address:0x%08x", execFileName, (unsigned int)abfd->start_address);
    gMM_sysSymbolStartAddr = abfd->start_address;
    if (bfd_check_format_matches (abfd, bfd_object, &matching))
    {
        MMInfo("This is an object file.");
    }
    else
    {
        MMErr("File Format not matching elf-i386");
    }

    symbol_mem_size = 0;
    symbol_mem_size = bfd_get_symtab_upper_bound (abfd);
    MMInfo("%d bytes is needed fot the symbol table",(int)symbol_mem_size);
    MMInfo("bfd_openr successfully! filename:%s ; start address:0x%08x",execFileName,(unsigned int)abfd->start_address);
    gMM_sysSymbolStartAddr = abfd->start_address;
    if (symbol_mem_size < 0)
    {
        return XERROR;
    }
    if (symbol_mem_size == 0)
    {
        return XERROR;
    }

    gMM_sysSymbolTable = NULL;
    gMM_sysSymbolTable = (asymbol **) malloc (symbol_mem_size);
    if(gMM_sysSymbolTable==NULL)
    {
        MMErr("xmalloc symbol memory %d bytes failed", (int)symbol_mem_size);
        return XERROR;
    }
    gMM_sysSymbolCount = bfd_canonicalize_symtab (abfd, gMM_sysSymbolTable);

    MMInfo("Reading %d Symbols into memory, occupy %d bytes",
        (int)gMM_sysSymbolCount, (int)symbol_mem_size);

    MMInfo("Reading %d Symbols into memory, occupy %d bytes",
        (int)gMM_sysSymbolCount, (int)symbol_mem_size);

    MMInfo("bfd_openr successfully! filename:%s ; start address:0x%08x",execFileName, (unsigned int)abfd->start_address);

    gMM_sysSymbolStartAddr = abfd->start_address;
    if (gMM_sysSymbolCount < 0)
    {
        MMErr("Reading Symbol failed");
        return XERROR;
    }

#ifdef XOS_BFD_215
    if(FALSE==bfd_hash_table_init_n(&gMM_sysSymbolHashTbl,
        bfd_mm_hash_newfunc,
        gMM_sysSymbolCount))
    {
        MMErr("Init symbol hash table failed");
        return XERROR;
    }
#else
    if(FALSE==bfd_hash_table_init_n(&gMM_sysSymbolHashTbl,
        bfd_mm_hash_newfunc,
        gMM_sysSymbolCount,gMM_sysSymbolCount))
    {
        MMErr("Init symbol hash table failed");
        return XERROR;
    }
#endif

    for (i = 0; i < gMM_sysSymbolCount; i++)
    {
        if(strlen(gMM_sysSymbolTable[i]->name)!=0)
        {
            pNewHashEntry = (struct bfd_mm_hash_entry*)bfd_hash_lookup(
                &gMM_sysSymbolHashTbl,
                gMM_sysSymbolTable[i]->name, TRUE,  FALSE);
            if(pNewHashEntry==NULL)
            {
                MMErr("Insert new symbol into hash failed %s",
                    gMM_sysSymbolTable[i]->name);
                continue;
            }
            pNewHashEntry->ArrayIndex = i;
        }
    }
#endif

    MMInfo("Insert new symbol into hash OK" );

    return XSUCC;
}


#if (defined(XOS_LINUX) || defined(XOS_SOLARIS))
/************************************************************************
������:    bfd_mm_hash_newfunc
���ܣ�  linux���ű���Ұ�������
������
�����  ��
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
struct bfd_hash_entry *bfd_mm_hash_newfunc (struct bfd_hash_entry *entry,
                                            struct bfd_hash_table *table,
                                            const char *string)
{
    struct bfd_mm_hash_entry *ret = (struct bfd_mm_hash_entry *) entry;

    /* Allocate the structure if it has not already been allocated by a
    derived class.  */
    if (ret == (struct bfd_mm_hash_entry *) NULL)
    {
        ret = ((struct bfd_mm_hash_entry *)
            bfd_hash_allocate (table, sizeof (struct bfd_mm_hash_entry)));
        if (ret == (struct bfd_mm_hash_entry *) NULL)
            return NULL;
    }

    /* Call the allocation method of the base class.  */
    ret = ((struct bfd_mm_hash_entry *)
        bfd_hash_newfunc ((struct bfd_hash_entry *) ret, table, string));

    /* Initialize the local fields here.  */

    return (struct bfd_hash_entry *) ret;
}
#endif


#ifdef XOS_WIN321
int gMM_symbLkupCount =0;
/************************************************************************
������:    XosSymEnumSymbolsCallback
���ܣ�
������
�����  ��
���أ�
TRUE      - successful
FALSE     - failed
˵����
************************************************************************/
BOOL CALLBACK XosSymEnumSymbolsCallback(
                                         LPCSTR SymbolName,
                                         ULONG SymbolAddress,
                                         ULONG SymbolSize,
                                         PVOID UserContext
                                         )
{
    char *p=(char*)UserContext;
    int readChar;

    if(p==NULL)
        return TRUE;

    if(strstr(SymbolName,p))
    {
        gMM_symbLkupCount++;
        printf("%s\t\t0x%x\n",SymbolName,SymbolAddress);
        if(gMM_symbLkupCount%20==0)
        {
            printf("Press return to contine, Press Q to exit Xoslkup");
            while(1)
            {
                readChar = getchar();
                switch(readChar)
                {
                case 'Q':
                case 'q':
                    return FALSE;
                case 13:
                case 10:
                    return TRUE;
                default:
                    return TRUE;
                }
            }
        }
    }

    return TRUE;
}


/************************************************************************
������:    Xoslkup
���ܣ�
������
�����  ��
���أ�  ��
˵����
************************************************************************/
void Xoslkup(char *strToFnd)
{
    gMM_symbLkupCount=0;
    if(strToFnd==NULL)
    {
        printf("Please Give a Name to Xoslkup!\n");
        return;
    }
    SymEnumerateSymbols( GetCurrentProcess(),
        SymGetModuleBase(GetCurrentProcess(),(DWORD)&gMM_symbLkupCount),
        XosSymEnumSymbolsCallback,(PVOID)strToFnd);
}
#endif  /* #ifdef XOS_WIN32 */


#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
int  XosMM_symbTblTraverse(struct bfd_hash_entry *symb, void* ptr);
int  gMM_symbLkupCount = 0;
/************************************************************************
������:    Xoslkup
���ܣ�
������
�����  ��
���أ�  ��
˵����
************************************************************************/
void Xoslkup(char * str)
{
    char localStr[30];
    gMM_symbLkupCount = 0;
    if(str==NULL)
    {
        printf("Total %d symbols in the symbol table\n",(int)gMM_sysSymbolCount);
        return;
    }
    if(strlen(str)>29)
    {
        strncpy(localStr,str,29);
        localStr[29]=0;
    }
    else
    {
        strcpy(localStr,str);
    }
    bfd_hash_traverse(&gMM_sysSymbolHashTbl, XosMM_symbTblTraverse, (void*) localStr);
}


/************************************************************************
������:    XosMM_symbTblTraverse
���ܣ�
������
�����  ��
���أ�  ��
˵����
************************************************************************/
int  XosMM_symbTblTraverse(struct bfd_hash_entry *symb, void* ptr)
{
    struct bfd_mm_hash_entry * ptrMmHash;
    char*    strToFnd = (char*) ptr;
    char*   str1 = NULL;
    asymbol*  ptrSymbol;
    int readChar;

    if(strToFnd==NULL)
        return TRUE;

    ptrMmHash = (struct bfd_mm_hash_entry*) symb;
    str1 = (char*)ptrMmHash->root.string;
    if(strstr(str1,strToFnd))
    {
        ptrSymbol = gMM_sysSymbolTable[ptrMmHash->ArrayIndex];
        gMM_symbLkupCount++;
        if(ptrMmHash->ArrayIndex<gMM_sysSymbolCount)
        {
            printf("%-30s    0x%08x (%-5s) 0x%08x\n",
                ptrSymbol->name,
                (unsigned int)(ptrSymbol->value+gMM_sysSymbolStartAddr),
                ptrSymbol->section->name,
                ptrSymbol->flags);
        }
        if(gMM_symbLkupCount%20==0)
        {
            printf("Press return to contine, Press Q to exit Xoslkup");
            while(1)
            {
                readChar = getchar();
                switch(readChar)
                {
                case 'Q':
                case 'q':
                    return FALSE;
                case 13:
                case 10:
                    return TRUE;
                default:
                    return TRUE;
                }
            }
        }
    }
    return TRUE;
}
#endif  /*#if defined(XOS_LINUX) || defined(XOS_SOLARIS)*/


/************************************************************************
������:    XosMM_findSymForDog
���ܣ�
������
�����  ��
���أ�  ��
˵����
************************************************************************/
int XosMM_findSymForDog(char* symName,int* type, XPOINT* value)
{
#ifdef XOS_VXWORKS
    int result;
    SYM_TYPE symTyp = N_TEXT;
    char symNameUnderline[256];
    int retAddr;
    /* convert the entry func name to the system symbol format "_funcName",
    and find the address of it*/
    strcpy( symNameUnderline, "_");
    strcat( symNameUnderline, symName );

    result = symFindByName(sysSymTbl, symName, (XS8**)(&retAddr), &symTyp);

    if(ERROR == result)
    {
        symTyp  = N_TEXT;
        result = symFindByName(sysSymTbl, symNameUnderline,(XS8**)(&retAddr), &symTyp);
        if(ERROR == result)
        {
            return ERROR;
        }
    }
    *value = retAddr;
    *type = MM_SYMB_TYP_FUNC;
    return XSUCC;
#else
    return XSUCC;
#endif  /* #ifdef XOS_VXWORKS */
}


/************************************************************************
������:    XosMM_findSymbByName
���ܣ�  ͨ�����ֲ��ҷ��ű��ַ
������
�����  ��
���أ�  ��
˵����
************************************************************************/
int XosMM_findSymbByName(char* symName,int* type, XPOINT* value)
{
#ifdef XOS_VXWORKS
    /* ���ҷ��ű���ڴ��ַ��VXWORKS���� */
    int result;
    SYM_TYPE symTyp = N_TEXT;
    char symNameUnderline[256];
    int retAddr;

    /* convert the entry func name to the system symbol format "_funcName",
    and find the address of it*/
    strcpy( symNameUnderline, "_");
    strcat( symNameUnderline, symName );
    result = symFindByName(sysSymTbl, symName, (XS8**)(&retAddr), &symTyp);

    if(ERROR == result)
    {
        symTyp  = N_TEXT;
        result = symFindByName(sysSymTbl, symNameUnderline,(XS8**)(&retAddr), &symTyp);
        if(ERROR == result)
        {
            MMErr("Can not find entry function %s, EC(%x), create module failed.",
                symNameUnderline,errnoGet());
            return ERROR;
        }
    }
    *value = retAddr;
    *type = MM_SYMB_TYP_FUNC;
    return XSUCC;
#endif /* XOS_VXWORKS */

#ifdef XOS_WIN32
#if defined(_MSC_VER) && (_MSC_VER > 1200)
    // ���ҷ��ű���ڴ��ַ��WINDOWS���� 
    int err;
    int length = sizeof(SYMBOL_INFO) + MAX_SYM_NAME*sizeof(TCHAR) + sizeof(ULONG64) - 1;
    static PSYMBOL_INFO pSymbol = NULL;
    
    if (pSymbol == NULL)
    {
        pSymbol = (PSYMBOL_INFO)malloc(length);  // ����ڴ���ͷţ���ʱ���������˳�ʱ��ϵͳ�Զ�����
    }

    memset(pSymbol, 0, length);
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    if(FALSE == SymFromName(GetCurrentProcess(), symName, pSymbol))
    {
        err = GetLastError();
        MMErr("Can't find entry function:%s errNum=%d",symName,err);
        return XERROR;
    }
    *type = MM_SYMB_TYP_FUNC;
    *value = (XPOINT)(pSymbol->Address);
    return XSUCC;
#else
    /* ���ҷ��ű���ڴ��ַ��WINDOWS���� */
    IMAGEHLP_SYMBOL symbolData;
    int err;

    memset(&symbolData,0,sizeof(symbolData));
    if(FALSE == SymGetSymFromName(GetCurrentProcess(),symName,&symbolData))
    {
        err = GetLastError();
        MMErr("Can't find entry function:%s errNum=%d",symName,err);
        return XERROR;
    }
    *type = MM_SYMB_TYP_FUNC;
    *value = symbolData.Address;
    return XSUCC;
#endif

#endif /*XOS_WIN32*/

#if (defined(XOS_LINUX) || defined(XOS_SOLARIS))
    /* ���ҷ��ű���ڴ��ַ��LINUX���� */
    int i=0;
    asymbol* ptrAsym=NULL;

    while(i<gMM_sysSymbolCount)
    {
        if(gMM_sysSymbolTable[i] != NULL)
        {
            if(strcmp(gMM_sysSymbolTable[i]->name,symName)==0)
            {
                ptrAsym = gMM_sysSymbolTable[i];
                break;
            }
        }
        i++;
    }
    if(ptrAsym==NULL)
    {
        printf("Symbol not found\n");
        return ERROR;
    }
    if(ptrAsym->flags&BSF_FUNCTION)
    {
        *type = MM_SYMB_TYP_FUNC;
    }
    else if(ptrAsym->flags&BSF_OBJECT)
    {
        *type = MM_SYMB_TYP_DATA;
    }
    else
        return XERROR;
    *value = ptrAsym->value + gMM_sysSymbolStartAddr;
    return XSUCC;
#endif /* defined(XOS_LINUX) || defined(XOS_SOLARIS) */
}


/************************************************************************
������:    XosMMShellParser
���ܣ�  Shell �������
������
�����  ��
���أ�  ��
˵����
************************************************************************/
void XosMMShellParser()
{
#ifndef XOS_VXWORKS
    int readChar;
    char readStr[280];
    int cur=0;
    MMInfo("A Simple Shell is started for debug\n");
    memset(readStr,0,sizeof(readStr));
#if 0
    initscr(); /*otherwise getch() will cause segment fault*/
#endif
    printf("\n->");
    while(1)
    {
        /* read lines of input*/
        readChar = getchar();
        {
            switch(readChar)
            {
            case 3:
                exit(0);
                break;
            case 13:
            case 10:
                if(cur!=0)
                {
                    printf("\n");
                    XosMMExecuteShellStr(readStr);
                }
                printf("->");
                cur=0;
                readStr[cur]=0;
                break;
            default :
                if(cur>=255)
                {
                    printf("Line too long\n");
                    break;
                }
                readStr[cur]=readChar;
                cur++;
                readStr[cur]=0;
                printf("%c",(char)readChar);
                break;
            }
        }
    }
#endif /* XOS_VXWORKS */
}


/************************************************************************
������:    XosMMExecuteShellStr
���ܣ�  Shell����ִ��
������
�����  ��
���أ�  ��
˵����
************************************************************************/
void XosMMExecuteShellStr(char* str)
{
#ifdef XOS_VXWORKS
    execute(str);
    return;
#else
    char execStr[512];
    int strTempIdx1;
    int strCurIdx;

    strcpy(execStr,str);
    /*
    * Parlay project require 'exit' to shutdown ssi,
    * but this can cause a segmentation coredump.
    * [BUG] This is a temporary resolvent to exit ssi.
    */
    if (strcmp(str, "exit") == 0)
    {
        printf("exiting ssi\n");
        exit(0);
    }
    strCurIdx = 0;
    while(1)
    {
        /*find first ';' */
        strTempIdx1 = XosMMStrFindChar(execStr+strCurIdx,';',1);
        if(strTempIdx1==-1)
        {
            XosMMExecuteStrSingleFunc(execStr+strCurIdx);
            break;
        }
        else
        {
            execStr[strCurIdx+strTempIdx1]=0;
            XosMMExecuteStrSingleFunc(execStr+strCurIdx);
            strCurIdx+=(strTempIdx1+1);
        }
    }
#endif /* XOS_VXWORKS */
}

typedef XU32 (*MM_SHELL_EXEC_FUNC) (XPOINT a, XPOINT b ,XPOINT c, XPOINT d, XPOINT e, XPOINT f, XPOINT g, XPOINT h, XPOINT i , XPOINT j);


/************************************************************************
������:    XosMMExecuteStrSingleFunc
���ܣ�  ִ��
������
�����  ��
���أ�  ��
˵����
************************************************************************/
XOS_MM_STATUS XosMMExecuteStrSingleFunc(char* strToExec)
{
#ifndef XOS_VXWORKS
    /* find the init function           */
    /*    strcpy(symName,entryFuncName);  */
    MM_SHELL_EXEC_FUNC pFunc;
    int result;
    char  strSymBolName[64];
    char  strPara[10][128];
    XPOINT pointPara[10];

    int execStrLen;
    int curStrIdx,tempStrIdxReal,tempStrIdx,paraCount;
    char* strToLongReturn;
    int symType = 0;
    XPOINT symAddr = (XPOINT)XNULLP;

#define MM_EXEC_TYPE_NULL           0
#define MM_EXEC_TYPE_FUNC_SPACE    1
#define MM_EXEC_TYPE_FUNC_BRACKET  2
#define MM_EXEC_TYPE_VAR_SET       3

    int execType = MM_EXEC_TYPE_FUNC_SPACE;

    execStrLen = XosMMStrTrimSpace(strToExec);
    /*clear sym and parameter list*/
    memset(strSymBolName,0,sizeof(strSymBolName));
    memset(strPara,0,sizeof(strPara));
    memset(pointPara,0,sizeof(pointPara));

    curStrIdx = 0;
    /*find the first not space char*/
    tempStrIdxReal = XosMMStrFindNoSpace(strToExec+curStrIdx);
    if(tempStrIdxReal==-1)
    {/*all white space, return*/
        return XERROR;
    }
    curStrIdx+=tempStrIdxReal;

    tempStrIdxReal = (XS32)strlen(strToExec+curStrIdx);
    /*get first string,should be function viriable name*/
    tempStrIdx = XosMMStrFindSpace(strToExec+curStrIdx);
    if(tempStrIdx!=-1)
    {
        execType = MM_EXEC_TYPE_FUNC_SPACE;
        tempStrIdxReal = tempStrIdx;
    }
    tempStrIdx = XosMMStrFindChar(strToExec+curStrIdx,'(',1);
    if(tempStrIdx!=-1&&tempStrIdx<tempStrIdxReal)
    {
        execType = MM_EXEC_TYPE_FUNC_SPACE;
        tempStrIdxReal = tempStrIdx;
    }
    tempStrIdx = XosMMStrFindChar(strToExec+curStrIdx,'=',1);
    if(tempStrIdx!=-1&&tempStrIdx<tempStrIdxReal)
    {
        execType = MM_EXEC_TYPE_FUNC_SPACE;
        tempStrIdxReal = tempStrIdx;
    }

    if(tempStrIdxReal>=64)
    {
        printf("Err:symbol too long, max is 64,current is %d",tempStrIdxReal);
        return XERROR;
    }
    strncpy(strSymBolName,strToExec+curStrIdx,tempStrIdxReal);
    strSymBolName[tempStrIdxReal]=0;
    curStrIdx +=tempStrIdxReal;

    if(execType==MM_EXEC_TYPE_FUNC_SPACE)
    {
        tempStrIdx = XosMMStrFindNoSpace(strToExec+curStrIdx);
        if(tempStrIdx!=-1)
        {
            /*decide expression type*/
            if(strToExec[curStrIdx+tempStrIdx]=='(')
            {
                execType = MM_EXEC_TYPE_FUNC_BRACKET;
                tempStrIdxReal = tempStrIdx;
                curStrIdx+=tempStrIdx+1;
            }
            else if(strToExec[curStrIdx+tempStrIdx]=='=')
            {
                execType = MM_EXEC_TYPE_VAR_SET;
                tempStrIdxReal = tempStrIdx;
                curStrIdx+=tempStrIdx+1;
            }
        }
    }

    /*find the last ')'*/
    if(execType == MM_EXEC_TYPE_FUNC_BRACKET)
    {
        tempStrIdx = XosMMStrFindLastChar(strToExec+curStrIdx,')',1);
        if(tempStrIdx==-1)
        {
            printf("Err:Synctax error, bracket not closed\n");
            return XERROR;
        }
        strToExec[curStrIdx+tempStrIdx]= 0;
    }

    /* now parse all parameters*/
    paraCount = 0;
    while(1)
    {
        if(curStrIdx>=execStrLen)
            break;
        tempStrIdxReal = XosMMStrFindOtherChar(strToExec+curStrIdx, ' ');
        if(tempStrIdxReal==-1)
            break;
        curStrIdx +=tempStrIdxReal;
        tempStrIdxReal = XosMMStrFindChar(strToExec+curStrIdx,',',1);
        if(tempStrIdxReal==-1)
            tempStrIdxReal = (XS32)strlen(strToExec+curStrIdx);
        strToExec[curStrIdx+tempStrIdxReal]=0;
        tempStrIdx = XosMMStrTrimSpace(strToExec+curStrIdx);
        if(strToExec[curStrIdx]=='"')
        {/*parameter should be a string*/
            if(strToExec[curStrIdx+tempStrIdx-1]=='"')
                strncpy(strPara[paraCount],strToExec+curStrIdx+1,tempStrIdxReal-2);
            else
            {
                printf("Err:Syntax error at parameter :%d,%s\n",paraCount+1,strToExec+curStrIdx);
                return XERROR;
            }
            pointPara[paraCount] = (XPOINT)(strPara+paraCount);
        }
        else
        {/* parameter should be int value*/
            if(strToExec[curStrIdx]=='0'&&
                (strToExec[curStrIdx+1]=='x'||strToExec[curStrIdx+1]=='X'))
            {
                pointPara[paraCount] = (XPOINT)strtopointer(strToExec+curStrIdx,&strToLongReturn,16);
                if(strToLongReturn!=strToExec+curStrIdx+tempStrIdx)
                {
                    printf("Err:Syntax error at para %d,%s\n",paraCount+1,strToExec+curStrIdx);
                    return XERROR;
                }
            }
            else
            {
                pointPara[paraCount] = (XPOINT)strtopointer(strToExec+curStrIdx,&strToLongReturn,10);
                if(strToLongReturn!=strToExec+curStrIdx+tempStrIdx)
                {
                    printf("Err:Syntax error at para %d,%s\n",paraCount+1,strToExec+curStrIdx);
                    return XERROR;
                }
            }
        }
        curStrIdx+=tempStrIdxReal+1;
        paraCount++;
    }

    if(execType==MM_EXEC_TYPE_VAR_SET&&paraCount!=1)
    {
        printf("Err:Synctax error for set argument\n");
        return XERROR;
    }

#if 1
    if(XSUCC!=XosMM_findSymbByName(strSymBolName,&symType,&symAddr))
    {
        printf("Err:Symbol Not Found %s\n",strSymBolName);
        return 0;
    }
    if(symType==MM_SYMB_TYP_FUNC)
    {
        if(execType == MM_EXEC_TYPE_VAR_SET)
        {
#ifdef XOS_WIN32
            /* we could not tell whether the symbol is a function or a variable*/
            *((XPOINT*)symAddr)=pointPara[0];
            return XSUCC;
#else
            printf("Err:Symbol is not function, %s\n",strSymBolName);
            return XERROR;
#endif
        }
        pFunc = (MM_SHELL_EXEC_FUNC)symAddr;
        if(pFunc!=0)
        {
            result = pFunc(pointPara[0],pointPara[1],pointPara[2],pointPara[3],pointPara[4],pointPara[5],
                pointPara[6],pointPara[7],pointPara[8],pointPara[9]);
            printf("\nreturn=0x%08x\n",result);
        }
    }
    else if(symType==MM_SYMB_TYP_DATA)
    {
        if(execType != MM_EXEC_TYPE_VAR_SET)
        {
            printf("Err:Symbol is a Function, %s\n",strSymBolName);
            return XERROR;
        }
        *((XPOINT*)symAddr)=pointPara[0];
    }
    else
    {
        printf("Unkown symbol type\n");
    }
#endif
#endif /* #ifndef XOS_VXWORKS */
    return XSUCC;
}


#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
#define MM_TASK_COUNT       3
typedef struct mm_thread_t
{
    pthread_t thread;
    int used;
}MM_THREAD;

MM_THREAD gMMPThreadArray[MM_TASK_COUNT];


/************************************************************************
������:    XosMMtaskCreate
���ܣ�  LINUX�´���һ������
������
�����
���أ�
˵����
************************************************************************/
int XosMMtaskCreate(char* taskName,int priority,int stackSize,void* funcptr,void* para)
{
    int i=1;
    pthread_attr_t attr;
    while(i<MM_TASK_COUNT)
    {
        if(gMMPThreadArray[i].used!=TRUE)
        {
            pthread_attr_init(&attr);
            pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            if (pthread_create(&gMMPThreadArray[i].thread, &attr, funcptr, para) != 0)
            {
                return MM_INVALID_TASK;
            }
            gMMPThreadArray[i].used=TRUE;
            return i;
        }
        i++;
    }
    return MM_INVALID_TASK;
}


#define MM_MUTEX_COUNT       3
typedef struct mm_mutex_t
{
    pthread_mutex_t mutex;
    int used;
}MM_MUTEX;

MM_MUTEX gMMMutexArray[MM_MUTEX_COUNT];


/************************************************************************
������:    XosMM_mutexCreate
���ܣ�  LINUX�´���һ�������ź���
������
�����
���أ�
˵����
************************************************************************/
MM_MUTEX_ID XosMM_mutexCreate()
{
    int i=1;
    while(i<MM_MUTEX_COUNT)
    {
        if(gMMMutexArray[i].used!=TRUE)
        {
            if(pthread_mutex_init(&gMMMutexArray[i].mutex, NULL)!=0)
            {
                MMErr("mutex init failed,%08x", errno);
                return MM_INVALID_MUTEX;
            }
            gMMMutexArray[i].used=TRUE;
            return i;
        }
        i++;
    }
    return MM_INVALID_MUTEX;
}


/************************************************************************
������:    XosMMmutexTake
���ܣ�  LINUX������һ�������ź���
������
�����
���أ�
˵����
************************************************************************/
int XosMMmutexTake(MM_MUTEX_ID mutexId)
{
    if(mutexId>=MM_MUTEX_COUNT||mutexId==0)
        return XERROR;
    if(pthread_mutex_lock(&gMMMutexArray[mutexId].mutex)!=0)
    {
        MMErr("mutex lock failed,%08x", errno);
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
������:    XosMMmutexGive
���ܣ�  LINUX���ͷ�һ�������ź���
������
�����
���أ�
˵����
************************************************************************/
int XosMMmutexGive(MM_MUTEX_ID mutexId)
{
    if(mutexId>=MM_MUTEX_COUNT||mutexId==0)
        return XERROR;
    if(pthread_mutex_unlock(&gMMMutexArray[mutexId].mutex)!=0)
    {
        MMErr("mutex unlock failed,%08x", errno);
        return XERROR;
    }
    return XSUCC;
}


#define MM_SEMB_COUNT       3
typedef struct mm_semb_t
{
    sem_t semb;
    int used;
}MM_SEMB;

MM_SEMB gMMSemBArray[MM_SEMB_COUNT+1];


/************************************************************************
������:    XosMMsemBCreate
���ܣ�  LINUX�´���һ���������ź���
������
�����
���أ�
˵����
************************************************************************/
MM_SEMB_ID XosMMsemBCreate(int initState)
{
    int i=1;
    while(i <=  MM_MUTEX_COUNT )
    {
        if(gMMSemBArray[i].used!=TRUE)
        {
            if (sem_init(&gMMSemBArray[i].semb, 0, 1)!=0)
            {
                MMErr("sem init failed,%08x", errno);
                return MM_INVALID_SEMB;
            }

            if(initState==MM_SEM_B_INIT_EMPTY)
            {
                {
                    int iRet;
                    do
                    {
                        iRet = sem_wait(&gMMSemBArray[i].semb);
                    }
                    while((iRet != 0) && (errno == EINTR));

                    if(iRet != 0)
                    {
                        printf("XosMMsemBCreate: errno=%d\r\n",errno);
                        sem_destroy(&gMMSemBArray[i].semb);
                        MMErr("sem init failed,%08x", errno);
                        return MM_INVALID_SEMB;
                    }
                }

                /*
                if(sem_wait(&gMMSemBArray[i].semb)!=0)
                {
                sem_destroy(&gMMSemBArray[i].semb);
                MMErr("sem init failed,%08x", errno);
                return MM_INVALID_SEMB;
                }
                */
            }
            else if(initState!=MM_SEM_B_INIT_FULL)
            {
                sem_destroy(&gMMSemBArray[i].semb);
                return MM_INVALID_SEMB;
            }
            gMMSemBArray[i].used=TRUE;
            /*            if(initState==MM_SEM_B_INIT_FULL)
            {
            if(sem_post(&gMMSemBArray[i].semb)!=0)
            {
            sem_destroy(&gMMSemBArray[i].semb);
            MMErr("sem init failed,%08x", errno);
            return MM_INVALID_SEMB;
            }
            }
            else if(initState!=MM_SEM_B_INIT_EMPTY)
            {
            sem_destroy(&gMMSemBArray[i].semb);
            return MM_INVALID_SEMB;
            }
            gMMSemBArray[i].used=TRUE;
            */
            return i;
        }
        i++;
    }
    return MM_INVALID_SEMB;
}


int XosMMsemBTake(MM_SEMB_ID semId,int timeout)
{
    if(semId>=MM_MUTEX_COUNT||semId==0)
        return XERROR;
        /*
        if(sem_wait(&gMMSemBArray[semId].semb)!=0)
        {
        MMErr("sem wait failed,%08x", errno);
        return XERROR;
        }
    */
    {
        int iRet;

        do
        {
            iRet = sem_wait(&gMMSemBArray[semId].semb);
        }
        while((iRet != 0) && (errno == EINTR));

        if(iRet != 0)
        {
            printf("XosMMsemBTake errno=%d\r\n",errno);
            MMErr("sem wait failed,%08x", errno);
            return XERROR;
        }
    }

    return XSUCC;
}


int XosMMsemBGive(MM_SEMB_ID semId)
{
    if(semId>=MM_MUTEX_COUNT||semId==0)
        return XERROR;
    if(sem_post(&gMMSemBArray[semId].semb)!=0)
    {
        MMErr("sem_post failed,%08x", errno);
        return XERROR;
    }
    return XSUCC;
}
#endif  /*#if defined(XOS_LINUX) || defined(XOS_SOLARIS)*/

extern int sysClkRateGet(void);
/************************************************************************
������:    XosMSMMtaskSleep
���ܣ�  ������ʱ����
������
�����
���أ�
˵����
************************************************************************/
void XosMSMMtaskSleep(int msec)
{
#ifdef XOS_WIN32
    Sleep(msec);
    return;
#endif /* XOS_WIN32 */

#ifdef SS_VW
    XU32 wTicks=0;
    wTicks = (msec * sysClkRateGet())/1000;
    if( 0 == wTicks )
        wTicks = 1;
    taskDelay( wTicks);
    return;
#endif /*SS_VW*/

#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
    struct timespec ts;
    ts.tv_sec = msec/1000;
    ts.tv_nsec = (msec%1000)*1000000;
    nanosleep(&ts, NULL);
    return;
#endif /* XOS_LINUX*/
}


/************************************************************************
������:    XosCreateSystemRegistryDatabase
���ܣ�  ���������ļ�������ϵͳע���
������
filename�������ļ���
filepath�������ļ�·��
�����  ��
���أ�  result
˵����
************************************************************************/
XS32 XosCreateSystemRegistryDatabase(XS8 *filename, XS8 *filepath)
{
    XS32 result;
#ifdef XOS_WIN32
    semProtectSysReg = XosMM_mutexCreate();
    if(0 == semProtectSysReg)
    {
        MMErr("Create Semphore For Registry Failed");
    }
#endif
    /*
    * create system registry table user output,
    * because now we have no our own cli, so we use logMsg,
    * sometime it should be replaced
    */
    MMInfo( "Building Sytem Registry Table, please wait...");
    result = XosMM_XmlReadCfg();
    if (XSUCC == result)
    {
        MMInfo( "Building System Registry Table succeed.");
    }
    else
    {
        MMInfo( "Building System Registry Table failed.");
    }

    return result;
}


/************************************************************************
������:    XosSysRegGetRootDir
���ܣ�  ��ȡϵͳע���ĸ�Ŀ¼��
������
�����
���أ�  ϵͳע����Ŀ¼
˵����
************************************************************************/
HANDLE XosSysRegGetRootDir()
{
    return (HANDLE)gSysRegTable->rootDir;
}


/************************************************************************
������:    XosSysRegOpenDir
���ܣ�  ����Ŀ¼��������Ŀ¼���ֲ�����ϵͳע����е�λ�á�
������
hDir��      ϵͳע����Ŀ¼
subDirName����Ŀ¼����
�����  phSubDir��  ��Ŀ¼��ַ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosSysRegOpenDir(HANDLE hDir, HANDLE *phSubDir,  XS8 *subDirName)
{
    sys_reg_dir_t * pDir = NULL;

    if((HANDLE)NULL==hDir||NULL==phSubDir||NULL==subDirName)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);
    pDir = ((sys_reg_dir_t*)hDir)->pFirstSonKey;
    while(NULL != pDir)
    {
        if(SYSREG_ISDIR((HANDLE)pDir))
        {
            if(0 == strcmp(pDir->name, subDirName))
            {
                *phSubDir = (HANDLE)pDir;
                XosMMmutexGive(semProtectSysReg);
                return XSUCC;
            }
        }
        pDir = pDir->pNextBrotherKey;
    }
    XosMMerrNoSet(SYSREG_CANNOTOPENDIR);
    XosMMmutexGive(semProtectSysReg);

    return XERROR;
}


/************************************************************************
������:    XosSysRegQueryU32KeyValue
���ܣ�  ���ݱ�������ֻ�ȡ�޷���U32����ֵ
������
hDir��      ϵͳע����Ŀ¼
keyName��   ���������
�����  value��     ���U32ֵ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosSysRegQueryU32KeyValue(HANDLE hDir, XS8 *keyName, XU32 *value)
{
    sys_reg_key_t *pKey = NULL;

    if((HANDLE)NULL==hDir||NULL==keyName||NULL==value)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);
    pKey = XosSysRegQueryKey(hDir, keyName);
    if(NULL == pKey)
    {
        XosMMerrNoSet(SYSREG_CANNOTFINDKEY);
        XosMMmutexGive(semProtectSysReg);
        return XERROR;
    }

    /*  ���ݲ�ͬ���͵�����ֵ  */
    if ( ( (pKey->value[0] == '0') && (pKey->value[1] == 'x') ) ||
        ( (pKey->value[0] == '0') && (pKey->value[1] == 'X') )
        )
        /*  input is formatted as 0x***  */
        sscanf( pKey->value, "%x", value );
    else
        sscanf( pKey->value, "%u", value );

    XosMMmutexGive(semProtectSysReg);

    return XSUCC;
}


/************************************************************************
������:    XosSysRegQueryS32KeyValue
���ܣ�  ���ݱ�������ֻ�ȡ�з���S32����ֵ
������
hDir��      ϵͳע����Ŀ¼
keyName��   ���������
�����  value��     ���S32ֵ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosSysRegQueryS32KeyValue(HANDLE hDir, XS8 *keyName, XS32 *value)
{
    sys_reg_key_t *pKey = NULL;
    XS8 * stop = NULL;

    if((HANDLE)NULL==hDir||NULL==keyName||NULL==value)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);
    pKey = XosSysRegQueryKey(hDir, keyName);
    if(NULL == pKey)
    {
        XosMMerrNoSet(SYSREG_CANNOTFINDKEY);
        XosMMmutexGive(semProtectSysReg);
        return XERROR;
    }
    *value = strtol((XS8*)(pKey->value), &stop, 10);
    XosMMmutexGive(semProtectSysReg);
    return XSUCC;
}


/************************************************************************
������:    XosSysRegQueryStrKeyValue
���ܣ�  ���ݱ�������ֻ�ȡ�ַ�������ֵ
������
hDir��      ϵͳע����Ŀ¼
keyName��   ���������
�����  value��     �ַ�������ֵ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosSysRegQueryStrKeyValue(HANDLE hDir, XS8 *keyName, XS8* value)
{
    sys_reg_key_t *pKey = NULL;

    if((HANDLE)NULL==hDir||NULL==keyName||NULL==value)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);
    pKey = XosSysRegQueryKey(hDir, keyName);
    if(NULL == pKey)
    {
        XosMMerrNoSet(SYSREG_CANNOTFINDKEY);
        XosMMmutexGive(semProtectSysReg);
        strcpy(value,"");
        return XERROR;
    }
    strcpy(value, pKey->value);
    XosMMmutexGive(semProtectSysReg);
    return XSUCC;
}


/************************************************************************
������:    XosSysRegSetU32KeyValue
���ܣ�  ����ϵͳע���������ΪkeyName����ֵΪvalue������ΪU32��
������
hDir��      ϵͳע����Ŀ¼
keyName��   ���������
�����  value��     ����ֵ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosSysRegSetU32KeyValue(HANDLE hDir, XS8 *keyName, XU32 value )
{
    sys_reg_key_t *pKey = NULL;

    if((HANDLE)NULL==hDir||NULL==keyName)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);
    pKey = XosSysRegQueryKey(hDir, keyName);
    if(NULL == pKey)
    {
        XosMMerrNoSet(SYSREG_CANNOTFINDKEY);
        XosMMmutexGive(semProtectSysReg);
        return XERROR;
    }
    sprintf(pKey->value, "%u", value);
    XosMMmutexGive(semProtectSysReg);
    return XSUCC;
}


/************************************************************************
������:    XosSysRegSetS32KeyValue
���ܣ�  ����ϵͳע���������ΪkeyName����ֵΪvalue������ΪS32��
������
hDir��      ϵͳע����Ŀ¼
keyName��   ���������
�����  value��     �ַ�������ֵ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosSysRegSetS32KeyValue(HANDLE hDir, XS8 *keyName, XS32 value )
{
    sys_reg_key_t *pKey = NULL;
    if((HANDLE)NULL==hDir||NULL==keyName)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);
    pKey = XosSysRegQueryKey(hDir, keyName);
    if(NULL == pKey)
    {
        XosMMerrNoSet(SYSREG_CANNOTFINDKEY);
        XosMMmutexGive(semProtectSysReg);
        return XERROR;
    }
    sprintf(pKey->value, "%d", value);
    XosMMmutexGive(semProtectSysReg);
    return XSUCC;
}


/************************************************************************
������:    XosSysRegSetStrKeyValue
���ܣ�  ����ϵͳע���������ΪkeyName����ֵΪvalue������Ϊ�ַ�����
������
hDir��      ϵͳע����Ŀ¼
keyName��   ���������
�����  value��     �ַ�������ֵ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosSysRegSetStrKeyValue(HANDLE hDir, XS8 *keyName, XS8 *value )
{
    sys_reg_key_t *pKey = NULL;
    if((HANDLE)NULL==hDir||NULL==keyName||NULL==value)
        return XERROR;
    if(strlen(value) > MAX_KEYVALUE_LENGTH)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);
    pKey = XosSysRegQueryKey(hDir, keyName);
    if(NULL == pKey)
    {
        XosMMerrNoSet(SYSREG_CANNOTFINDKEY);
        XosMMmutexGive(semProtectSysReg);
        return XERROR;
    }
    strcpy(pKey->value, value);
    XosMMmutexGive(semProtectSysReg);
    return XSUCC;
}


/************************************************************************
������:    XosSysRegEnumKey
���ܣ�  ����KEYֵ��������ȡ��ǰĿ¼hDir�£�����hPreKey����һ����������ֺ�ֵ��
        ���hPreKey��ֵΪ�գ��ͻ�ȡ��ǰĿ¼�ĵ�һ������ֵ��
������
hDir��      Ҫ���ҵ�Ŀ¼
hPreKey��   ǰһ������ĵ�ַ
�����
name��      ��ȡ�ı�����
value��     ��ȡ�ı���ֵ
phSubKey��  ��ȡ�ı����ַ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosSysRegEnumKey(HANDLE hDir, HANDLE hPreKey, XS8 *value, XS8 *name, HANDLE *phSubKey)
{
    sys_reg_key_t *pKey = NULL;

    if((HANDLE)NULL==hDir||NULL==value||NULL==phSubKey||NULL==name)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);

    if((HANDLE)NULL == hPreKey)
    {
        pKey = ((sys_reg_key_t*)hDir)->pFirstSonKey;
    }
    else
    {
        pKey = ((sys_reg_key_t*)hPreKey)->pNextBrotherKey;
    }

    while(NULL != pKey)
    {
        if((SYSREG_ISKEY((HANDLE)pKey)))
        {
            strcpy(value, pKey->value);
            strcpy(name, pKey->name);
            *phSubKey = (HANDLE)pKey;
            XosMMmutexGive(semProtectSysReg);
            return XSUCC;
        }
        pKey = pKey->pNextBrotherKey;
    }

    *phSubKey = (HANDLE)pKey; /* ��ʱ��ֵpKeyΪNULL */
    XosMMmutexGive(semProtectSysReg);
    return XSUCC;
}


/************************************************************************
������:    XosSysRegEnumKey
���ܣ�  ����DIR��������ȡ��ǰĿ¼hPreDir����һ��Ŀ¼��Ϣ����Ҫ��������Ŀ¼��
������
hDir��      һ��Ϊϵͳע���ĸ�Ŀ¼
hPreKey��   ǰһ��Ŀ¼��ַ��
�����
subDirName����õ���һ��Ŀ¼�����֡�
phSubDir��  ��õ�Ŀ¼��ַ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosSysRegEnumDir(HANDLE hDir, HANDLE hPreDir, XS8 *subDirName, HANDLE *phSubDir)
{
    sys_reg_dir_t *pDir = NULL;

    if((HANDLE)NULL==hDir||NULL==subDirName||NULL==phSubDir)
        return XERROR;

    XosMMmutexTake(semProtectSysReg);

    if((HANDLE)NULL == hPreDir)
    {
        pDir = ((sys_reg_key_t*)hDir)->pFirstSonKey;
    }
    else
    {
        pDir = ((sys_reg_key_t*)hPreDir)->pNextBrotherKey;
    }

    while(NULL != pDir)
    {
        if(SYSREG_ISDIR((HANDLE)pDir))
        {
            strcpy(subDirName, pDir->name);
            *phSubDir = (HANDLE)pDir;
            XosMMmutexGive(semProtectSysReg);
            return XSUCC;
        }
        pDir = pDir->pNextBrotherKey;
    }

    *phSubDir = (HANDLE)pDir;/* ��ʱ��Ŀ¼pDirΪNULL */
    XosMMmutexGive(semProtectSysReg);
    return XSUCC;
}


/************************************************************************
������:    XosSysRegGetKeyName
���ܣ�  ����hKey��ַ��ȡ���֡�
������  hKey��      �����ַ
�����  keyName��   ��������
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XU32 XosSysRegGetKeyName(HANDLE hKey, XS8 *keyName)
{
    if((HANDLE)NULL==hKey||NULL==keyName||
        (strlen(((sys_reg_key_t*)hKey)->name) >= MAX_KEYNAME_LENGTH+1))
        return XERROR;

    strcpy(keyName, ((sys_reg_key_t*)hKey)->name);

    return XSUCC;
}


/************************************************************************
������:    XosSysRegGetKeyValue
���ܣ�  ����hKey��ַ��ȡֵ��
������  hKey��      �����ַ
�����  keyValue��  ����ֵ
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosSysRegGetKeyValue(HANDLE hKey, XS8 *keyValue)
{
    if((HANDLE)NULL==hKey||NULL==keyValue||
        (strlen(((sys_reg_key_t*)hKey)->value) >= MAX_KEYVALUE_LENGTH+1))
        return XERROR;

    strcpy(keyValue, ((sys_reg_key_t*)hKey)->value);

    return XSUCC;
}


/************************************************************************
������:    XosSysRegQueryKey
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
sys_reg_key_t * XosSysRegQueryKey(HANDLE hDir, XS8 *keyName)
{
    sys_reg_dir_t * pDir = NULL;
    sys_reg_key_t * pKey = NULL;
    XS8  dirName[MAX_KEYNAME_LENGTH+2];
    XS8 * keyNameStart = NULL;

    if((HANDLE)NULL==hDir||NULL==keyName)
        return NULL;

    /*it needn't operate protect semaphore,
    because this function is a internal function*/
    memset(dirName, 0, MAX_KEYNAME_LENGTH);

    if(XSUCC == XosFirstLevelDirName(keyName, dirName, &keyNameStart))
    {
        pDir = XosSysRegQueryDir(hDir, dirName);
        if(NULL == pDir)
        {
            return NULL;
        }
        return XosSysRegQueryKey((HANDLE)pDir, keyNameStart);
    }
    else
    {
        pKey = ((sys_reg_dir_t*)hDir)->pFirstSonKey;
        while(NULL != pKey)
        {
            if(SYSREG_ISKEY((HANDLE)pKey))
            {
                if(0 == strcmp(keyName, pKey->name))
                {
                    return pKey;
                }
            }
            pKey = pKey->pNextBrotherKey;
        }
        return NULL;
    }
}


/************************************************************************
������:    XosSysRegQueryDir
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
sys_reg_dir_t * XosSysRegQueryDir(HANDLE hDir, XS8 *dirName)
{
    sys_reg_dir_t *pDir = NULL;

    if((HANDLE)NULL==hDir||NULL==dirName)
        return NULL;

    /*it needn't operate protect semaphore,
    * because this function is a internal function*/

    pDir = ((sys_reg_dir_t*)hDir)->pFirstSonKey;
    while(NULL != pDir)
    {
        if(SYSREG_ISDIR((HANDLE)pDir))
        {
            if(0 == strcmp(dirName, pDir->name))
            {
                return pDir;
            }
        }
        pDir = pDir->pNextBrotherKey;
    }
    return NULL;
}


/************************************************************************
������:    XosFirstLevelDirName
���ܣ�
������
�����
���أ�
XSUCC      - successful
XERROR     - failed
˵����
************************************************************************/
XS32 XosFirstLevelDirName(XS8 *keyName, XS8 *dirName, XS8 **restName)
{
    XS8 * cntLetter;
    XS8 * srcName;
    XS8 * dstName;

    if(NULL==keyName||NULL==dirName||NULL==restName)
        return XERROR;

    srcName = keyName;
    dstName = dirName;
    for(cntLetter = keyName; (*cntLetter != '\0') && (*cntLetter != '\\'); cntLetter ++)
    {
        *dstName = *srcName;
        dstName ++;
        srcName ++;
    }
    if(*cntLetter == '\0')
    {
        return XERROR;
    }
    else
    {
        *restName = cntLetter + 1;
        return XSUCC;
    }
}


/************************************************************************
������:    XosMMParseStrCpy
���ܣ�  �ڴ濽������
������
dest��Ŀ�ĵ�ַ
src�� Դ��ַ
maxLength������
�����
���أ�
˵����
************************************************************************/
void XosMMParseStrCpy(XU8* dest, const XU8* src, int maxLength)
{
    int strLength=0;

    while((*(src+strLength))!=0)
        strLength++;
    if(strLength>=maxLength)
        strLength=maxLength;
    memcpy(dest,src,strLength*sizeof(XU8));
    dest[strLength]=0;
}


/************************************************************************
������:    XosMMFreeRegTable
���ܣ�  �ͷ�ϵͳע����ڴ档
������
�����
���أ�
˵����
************************************************************************/
void XosMMFreeRegTable(sys_reg_tbl_t * parsedDoc)
{
    sys_reg_key_ptr tmpRegPtr=NULL,PretmpRegPtr=NULL;
    XU32  is_son=0;
    while(1)
    {
    /*�����ܳ��ڣ�����Ѷ�������ӽڵ�ȫ��ɾ���Ժ�����˳�
    ɾ��ԭ���ӽڵ���ֵܽ�㶼Ϊ�գ��Ϳ���ɾ����ǰ�ڵ�*/
        if(parsedDoc== NULL)
        {
            return;
        }
        if(parsedDoc->rootDir == NULL)
        {
            free(parsedDoc);
            parsedDoc = NULL;
            continue;
        }
        if((parsedDoc->rootDir->pFirstSonKey == NULL)&& \
            (parsedDoc->rootDir->pNextBrotherKey == NULL))
        {
            free(parsedDoc->rootDir);
            parsedDoc->rootDir = NULL;
            continue;
        }
        PretmpRegPtr = tmpRegPtr = parsedDoc->rootDir;
        while(tmpRegPtr)
        {
            /* Ҷ�ӽڵ� */
            if((!tmpRegPtr->pFirstSonKey)&&(!tmpRegPtr->pNextBrotherKey))
            {
                if(is_son)
                    PretmpRegPtr->pFirstSonKey = NULL;
                else
                    PretmpRegPtr->pNextBrotherKey = NULL;
                free(tmpRegPtr);
                tmpRegPtr = NULL;
                continue;
            }
            /* ���ӽڵ� */
            if(tmpRegPtr->pFirstSonKey)
            {
                PretmpRegPtr = tmpRegPtr;
                tmpRegPtr = tmpRegPtr->pFirstSonKey;
                is_son = 1;
                continue;
            }
            /* �ֵܽڵ� */
            if(tmpRegPtr->pNextBrotherKey)
            {
                PretmpRegPtr = tmpRegPtr;
                tmpRegPtr = tmpRegPtr->pNextBrotherKey;
                is_son = 0;
                continue;
            }
        } /* while(tmpRegPtr) */
    } /* while(1) */
}


#if defined(XOS_VXWORKS) && !defined(XOS_VTA)
XS16 XosMM_XMLreadMemCfg( sys_reg_tbl_t * parsedDoc);
#else
XS16 XosMM_XMLreadMemCfgNew( sys_reg_tbl_t * parsedDoc);
#endif


/************************************************************************
������:    XosMM_XmlReadCfg
���ܣ�  ��XML�����ȡģ�����������Ϣ��
������
�����
���أ�
˵����
************************************************************************/
XS32 XosMM_XmlReadCfg(XVOID)
{
    XS32 ret;
#if 0
    sys_reg_key_ptr reg1,reg2,reg3;
#endif

    gSysRegTable = NULL;
    

    gSysRegTable= (sys_reg_tbl_t*)malloc(sizeof(sys_reg_tbl_t));
    if(gSysRegTable == NULL)
    {
        return XERROR;
    }
    memset(gSysRegTable,0,sizeof(sys_reg_tbl_t));
#if defined(XOS_VXWORKS) && !defined(XOS_VTA)
    ret = XosMM_XMLreadMemCfg( gSysRegTable);
#else
    ret = XosMM_XMLreadMemCfgNew( gSysRegTable);
#endif



    return ret;
}


/*------------------------new xml function---------------------------------*/
/************************************************************************
������:    XosMM_XMLreadMemCfg
���ܣ�  ͨ��XML�ļ�����ϵͳע�����ģ������á��������ר��ΪVXWORKSд��
        ��ǰ��XML��֧��VXWORKS�汾��������Լ�����һ����
������
�����
���أ�
************************************************************************/
XS16 XosMM_XMLreadMemCfg( sys_reg_tbl_t * parsedDoc)
{
#ifdef XOS_VXWORKS
    XU32       i        = 0,j=0;

    sys_reg_key_ptr tmpRegPtr=NULL,curRegPtr=NULL,curReg1Ptr=NULL,curReg2Ptr=NULL,curReg3Ptr=NULL;

    /* ���system���ڵ� */
    tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
    if(NULL==tmpRegPtr)
    {
        printf("XosMMStartDocument not enough element memory\n");
    }
    memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
    XosMMParseStrCpy(tmpRegPtr->name,"system",MAX_KEYNAME_LENGTH);
    parsedDoc->rootDir=tmpRegPtr;
    curRegPtr = parsedDoc->rootDir;

    /*��� Global ���ڵ�*/
    tmpRegPtr = NULL;
    tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
    if(NULL==tmpRegPtr)
    {
        printf("XosMMStartDocument not enough element memory\n");
        goto ERR_OUT_LABLE;
    }
    memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
    XosMMParseStrCpy(tmpRegPtr->name,"Global",MAX_KEYNAME_LENGTH);
    curRegPtr->pFirstSonKey = tmpRegPtr;
    curReg1Ptr = curRegPtr->pFirstSonKey;

    /*��� Global �ӽڵ� ModuleInitTimeout*/
    tmpRegPtr = NULL;
    tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
    if(NULL==tmpRegPtr)
    {
        printf("XosMMStartDocument not enough element memory\n");
        goto ERR_OUT_LABLE;
    }
    memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
    XosMMParseStrCpy(tmpRegPtr->name,"ModuleInitTimeout",MAX_KEYNAME_LENGTH);
    XosMMParseStrCpy(tmpRegPtr->value,"300",MAX_KEYVALUE_LENGTH);

    curReg1Ptr->pFirstSonKey = tmpRegPtr;
    curReg2Ptr = curReg1Ptr->pFirstSonKey;

    /*ModuleFailReset �ڵ�*/
    tmpRegPtr = NULL;
    tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
    if(NULL==tmpRegPtr)
    {
        printf("XosMMStartDocument not enough element memory\n");
        goto ERR_OUT_LABLE;
    }
    memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
    XosMMParseStrCpy(tmpRegPtr->name,"ModuleFailReset",MAX_KEYNAME_LENGTH);
    XosMMParseStrCpy(tmpRegPtr->value,"1",MAX_KEYVALUE_LENGTH);

    curReg1Ptr->pFirstSonKey = tmpRegPtr;
    curReg2Ptr = curReg1Ptr->pFirstSonKey;

    /*�ҵ� Module ���ڵ�*/
    for(i=0;i<2;i++)
    {
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));

        if(i == 0)/* ��һ���ֵܽڵ� */
        {
            XosMMParseStrCpy(tmpRegPtr->name,"Module_A",MAX_KEYNAME_LENGTH);
            curReg1Ptr->pNextBrotherKey = tmpRegPtr;
            curReg1Ptr = curReg1Ptr->pNextBrotherKey;
        }
        else  /* ��һ���Ժ���ֵܽڵ� */
        {
            if(i==1)
            {
                XosMMParseStrCpy(tmpRegPtr->name,"Module_B",MAX_KEYNAME_LENGTH);
            }

            curReg1Ptr->pNextBrotherKey = tmpRegPtr;
            curReg1Ptr = curReg1Ptr->pNextBrotherKey;
        }

        /*��� Module �ӽڵ�*/

        /*Name �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"Name",MAX_KEYNAME_LENGTH);
        if(i==0)
        {
            XosMMParseStrCpy(tmpRegPtr->value,"Test Module A",MAX_KEYVALUE_LENGTH);
        }
        else
        {
            XosMMParseStrCpy(tmpRegPtr->value,"Test Module B",MAX_KEYVALUE_LENGTH);
        }

        curReg1Ptr->pFirstSonKey = tmpRegPtr;
        curReg2Ptr = curReg1Ptr->pFirstSonKey;

        /*ModuleId �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"ModuleId",MAX_KEYNAME_LENGTH);

        if(i==0)
        {
            XosMMParseStrCpy(tmpRegPtr->value,"20",MAX_KEYVALUE_LENGTH);
        }
        else
        {
            XosMMParseStrCpy(tmpRegPtr->value,"21",MAX_KEYVALUE_LENGTH);
        }
        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
        curReg2Ptr = curReg2Ptr->pNextBrotherKey;

        /*TaskCount �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"TaskCount",MAX_KEYNAME_LENGTH);
        XosMMParseStrCpy(tmpRegPtr->value,"2",MAX_KEYVALUE_LENGTH);

        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
        curReg2Ptr = curReg2Ptr->pNextBrotherKey;

        /*HeapSize �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"HeapSize",MAX_KEYNAME_LENGTH);
        XosMMParseStrCpy(tmpRegPtr->value,"2048",MAX_KEYVALUE_LENGTH);
        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
        curReg2Ptr = curReg2Ptr->pNextBrotherKey;

        /*StackSize �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"StackSize",MAX_KEYNAME_LENGTH);
        XosMMParseStrCpy(tmpRegPtr->value,"65535",MAX_KEYVALUE_LENGTH);

        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
        curReg2Ptr = curReg2Ptr->pNextBrotherKey;

        /*InitTimeout �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"InitTimeout",MAX_KEYNAME_LENGTH);
        XosMMParseStrCpy(tmpRegPtr->value,"300",MAX_KEYVALUE_LENGTH);

        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
        curReg2Ptr = curReg2Ptr->pNextBrotherKey;

        /*ObjectFileName �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"ObjectFileName",MAX_KEYNAME_LENGTH);
        XosMMParseStrCpy(tmpRegPtr->value,"wintest.o",MAX_KEYVALUE_LENGTH);

        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
        curReg2Ptr = curReg2Ptr->pNextBrotherKey;

        /*EntryFunc �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"EntryFunc",MAX_KEYNAME_LENGTH);

        if(i==0)
        {
            XosMMParseStrCpy(tmpRegPtr->value,"testmgtmain",MAX_KEYVALUE_LENGTH);
        }
        else
        {
            XosMMParseStrCpy(tmpRegPtr->value,"testmgtmain1",MAX_KEYVALUE_LENGTH);
        }
        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
        curReg2Ptr = curReg2Ptr->pNextBrotherKey;

        /*Parameter �ڵ�*/
        tmpRegPtr = NULL;
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
            goto ERR_OUT_LABLE;
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy(tmpRegPtr->name,"Parameter",MAX_KEYNAME_LENGTH);

        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
        curReg2Ptr = curReg2Ptr->pNextBrotherKey;

        /*�鿴 Parameter �Ƿ����ӽڵ�*/

        /*���� Parameter �ӽڵ�*/
        for(j=0;j<10;j++)
        {
            tmpRegPtr = NULL;
            tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
            if(NULL==tmpRegPtr)
            {
                printf("XosMMStartDocument not enough element memory\n");
                goto ERR_OUT_LABLE;
            }
            memset(tmpRegPtr,0,sizeof(sys_reg_key_t));

            if(j==0)
            {
                XosMMParseStrCpy(tmpRegPtr->name,"Parameter1",MAX_KEYNAME_LENGTH);
                XosMMParseStrCpy(tmpRegPtr->value,"111",MAX_KEYVALUE_LENGTH);
                curReg2Ptr->pFirstSonKey = tmpRegPtr;
                curReg3Ptr = curReg2Ptr->pFirstSonKey;
            }
            else
            {
                XosMMParseStrCpy(tmpRegPtr->name,"Parameter2",MAX_KEYNAME_LENGTH);
                XosMMParseStrCpy(tmpRegPtr->value,"222",MAX_KEYVALUE_LENGTH);
                curReg3Ptr->pNextBrotherKey = tmpRegPtr;
                curReg3Ptr = curReg3Ptr->pNextBrotherKey;
            }
        }
    }

    return XSUCC;

ERR_OUT_LABLE:
    XosMMFreeRegTable(parsedDoc);
    return( XERROR);

#endif  /*  XOS_VXWORKS  end*/

    return XSUCC;
}


XS16 XosMM_XMLreadMemCfgNew( sys_reg_tbl_t * parsedDoc)
{
    xmlDocPtr  doc        = NULL;
    xmlNodePtr cur      = NULL;
    xmlNodePtr level1cur  = NULL;
    xmlNodePtr level2cur = NULL;
    xmlNodePtr level3cur = NULL;
    xmlChar*   pTempStr = XNULL;
    sys_reg_key_ptr tmpRegPtr=NULL,curRegPtr=NULL,curReg1Ptr=NULL,curReg2Ptr=NULL,curReg3Ptr=NULL;
    XU8        level1firstnode,level2firstnode,level3firstnode;

#ifdef XOS_EW_START
    XCHAR szCfgFName[XOS_MAX_PATHLEN]={0};
    if( XSUCC != XOS_GetSysPath(szCfgFName, XOS_MAX_PATHLEN))
    {
        return XERROR;
    }
    XOS_StrCat(szCfgFName, "cfg.xml");
#endif

    if ( XNULL == parsedDoc )
    {
        return XERROR;
    }

#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
#ifndef XOS_EW_START
    doc = xmlParseFile("./cfg.xml");
#else
    doc = xmlParseFile(szCfgFName);
#endif
#endif

#if defined(XOS_WIN32) || defined(XOS_VTA)
#ifndef XOS_EW_START
    doc = xmlParseFile(".\\cfg.xml");
#else
    doc = xmlParseFile(szCfgFName);
#endif
#endif
    if (doc == XNULL)
    {
        return( XERROR );
    }
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        goto ERR_OUT_LABLE;
    }
    /* �ҵ�system���ڵ� */
    if ( !XOS_StrCmp( cur->name, "System") )
    {
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy((XU8*)tmpRegPtr->name,(XU8*)cur->name,MAX_KEYNAME_LENGTH);
        parsedDoc->rootDir=tmpRegPtr;
    }
    else
    {
        goto ERR_OUT_LABLE;
    }
    level1cur = cur->xmlChildrenNode;
    curRegPtr = parsedDoc->rootDir;

    while ( level1cur && xmlIsBlankNode ( level1cur ) )
    {
        level1cur = level1cur -> next;
    }
    if ( level1cur == XNULL )
    {
        goto ERR_OUT_LABLE;
    }

    level1firstnode = 1;
    /* Ŀǰģ�����ֻ֧������Ƕ�״��� */
    while ( level1cur != XNULL )
    {
        if(XOS_StrLen(level1cur->name) > 0)/* ��һ������ */
        {
            /* ��һ������ */
            tmpRegPtr = NULL;
            tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
            if(NULL==tmpRegPtr)
            {
                printf("XosMMStartDocument not enough element memory\n");
                goto ERR_OUT_LABLE;
            }
            memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
            XosMMParseStrCpy((XU8*)tmpRegPtr->name,(XU8*)level1cur->name,MAX_KEYNAME_LENGTH);
            if(level1firstnode)
            {
                curRegPtr->pFirstSonKey = tmpRegPtr;
                curReg1Ptr = curRegPtr->pFirstSonKey;
                level1firstnode = 0;
            }
            else
            {
                curReg1Ptr->pNextBrotherKey = tmpRegPtr;
                curReg1Ptr = curReg1Ptr->pNextBrotherKey;
            }

            level2cur = level1cur->xmlChildrenNode;
            while ( level2cur && xmlIsBlankNode ( level2cur ) )
            {
                level2cur = level2cur -> next;
            }
            level2firstnode = 1;
            while ( level2cur != XNULL)    /* �ڶ������� */
            {
                if(XOS_StrLen(level2cur->name) > 0)
                {
                    tmpRegPtr = NULL;
                    tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
                    if(NULL==tmpRegPtr)
                    {
                        printf("XosMMStartDocument not enough element memory\n");
                        goto ERR_OUT_LABLE;
                    }
                    memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
                    XosMMParseStrCpy((XU8*)tmpRegPtr->name,(XU8*)level2cur->name,MAX_KEYNAME_LENGTH);
                    pTempStr = xmlNodeListGetString( doc, level2cur->xmlChildrenNode, 1);
                    if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 && pTempStr[0])
                    {
                        XosMMParseStrCpy((XU8*)tmpRegPtr->value,(XU8*)pTempStr,MAX_KEYVALUE_LENGTH);
                    }
                    if(pTempStr) {
                        xmlFree(pTempStr);
                    }

                    if(level2firstnode)
                    {
                        curReg1Ptr->pFirstSonKey = tmpRegPtr;
                        curReg2Ptr = curReg1Ptr->pFirstSonKey;
                        level2firstnode = 0;
                    }
                    else
                    {
                        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
                        curReg2Ptr = curReg2Ptr->pNextBrotherKey;
                    }
                }

                level3cur = level2cur->xmlChildrenNode;
                while ( level3cur && xmlIsBlankNode ( level3cur ) )
                {
                    level3cur = level3cur -> next;
                }
                level3firstnode = 1;
                while ( level3cur != XNULL)    /* ���������� */
                {
                    pTempStr = xmlNodeListGetString( doc, level3cur->xmlChildrenNode, 1);
                    if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
                    {
                        tmpRegPtr = NULL;
                        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
                        if(NULL==tmpRegPtr)
                        {
                            printf("XosMMStartDocument not enough element memory\n");
                            goto ERR_OUT_LABLE;
                        }
                        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
                        XosMMParseStrCpy((XU8*)tmpRegPtr->name,(XU8*)level3cur->name,MAX_KEYNAME_LENGTH);
                        XosMMParseStrCpy((XU8*)tmpRegPtr->value,(XU8*)pTempStr,MAX_KEYVALUE_LENGTH);

                        if(level3firstnode)
                        {
                            curReg2Ptr->pFirstSonKey = tmpRegPtr;
                            curReg3Ptr = curReg2Ptr->pFirstSonKey;
                            level3firstnode = 0;
                        }
                        else
                        {
                            curReg3Ptr->pNextBrotherKey = tmpRegPtr;
                            curReg3Ptr = curReg3Ptr->pNextBrotherKey;
                        }
                    }
                    if(pTempStr) {
                        xmlFree(pTempStr);
                    }
                    /* ���������� */
                    level3cur = level3cur->next;
                    
                    while ( level3cur && xmlIsBlankNode ( level3cur ) )
                    {
                        level3cur = level3cur -> next;
                    }
                }
                level2cur = level2cur->next;
                
                while ( level2cur && xmlIsBlankNode ( level2cur ) )
                {
                    level2cur = level2cur -> next;
                }
            }
        }
        level1cur = level1cur->next;
        
        while ( level1cur && xmlIsBlankNode ( level1cur ) )
        {
            level1cur = level1cur -> next;
        }
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XSUCC;

ERR_OUT_LABLE:
    XosMMFreeRegTable(parsedDoc);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return( XERROR);
    /*  XOS Vxworks begin  */
}

/*------------------- end basic function ----------------------------------*/


#if defined(XOS_LINUX) || defined(XOS_SOLARIS) || defined(XOS_WIN32)
/************************************************************************
������:    XosMMTaskInitIdKey
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
void XosMMTaskInitIdKey()
{
    return;
}


/************************************************************************
������:    XosMMTaskSetId
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosMMTaskSetId(XU32 nWsId)
{
    if (nWsId > 0 && nWsId < XOS_MM_MAX_TASKS)
    {
    /*
    * Even if user has setted the WS task id,
    * we just reset it again without rechecking.
    */
        gWSTaskIdMap[nWsId] = nWsId;
#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
        if (pthread_setspecific(gWSTaskIdKey, &gWSTaskIdMap[nWsId]) == 0)
            return XSUCC;
#elif defined(XOS_WIN32)
        if (TlsSetValue(gWSTaskIdKey, &gWSTaskIdMap[nWsId]) != 0)
            return XSUCC;
#endif
    }
    else
    {
#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
        if(pthread_setspecific(gWSTaskIdKey, &gWSTaskIdCom) == 0)
            return XSUCC;
#elif defined(XOS_WIN32)
        if (TlsSetValue(gWSTaskIdKey, &gWSTaskIdCom) != 0)
            return XSUCC;
#endif
    }

    printf("[SSI] XosMMTaskSetId failed: task %d, %s\n",nWsId,strerror(errno));
    return XERROR;
}


/************************************************************************
������:    XosMMTaskGetId
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XU32 XosMMTaskGetId()
{
    void * p = 0;

#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
    p = pthread_getspecific(gWSTaskIdKey);
#elif defined(XOS_WIN32)
    p = TlsGetValue(gWSTaskIdKey);
#endif

    if (p)
        return *(XU32*)p;
    else
    {
        XosMMTaskSetId(XOS_MM_NON_SSI_TASK_ID);
        return XOS_MM_NON_SSI_TASK_ID;
    }
}
#endif /* XOS_LINUX || XOS_SOLARIS || XOS_WIN32 */


/************************************************************************
������:    XosModuleLibInit
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosModuleLibInit(XU8* pRootPath,XU32 nStackSize)
{
    memset(gstWS_moduleTab,0,sizeof(XOS_MM_MODULE_TAB)*XOS_MM_MAX_MODULES);
    /*first initialize task id facility*/
#if 0
    if(XSUCC!=XosMMTaskLibInit())
    {
        printf("XosMMTaskLibInit Failed\n");
        return XERROR;
    }
#endif
    return XSUCC;
}


/************************************************************************
������:    XosModuleInit
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosModuleInit(XOS_MM_MOD_INIT_PARA *pModInfo)
{
    XU32                nModuleId;
    XOS_MM_MODULE_TAB* pModuleEntry;

    if(pModInfo==NULL)
        return XERROR;
    nModuleId = pModInfo->nModuleId;
    if(nModuleId==0 || nModuleId>=XOS_MM_MAX_MODULES)
    {
        printf("[WS_SSI]XosModuleInit Failed, ModuleId too Large:%d,Max Id is %d\n",
            nModuleId,XOS_MM_MAX_MODULES);
        return XERROR;
    }
    pModuleEntry = &(gstWS_moduleTab[nModuleId]);
    if(pModuleEntry->fUsed)
    {
        printf("[WS_SSI]XosModuleInit Failed, ModuleId:%d already used\n",
            nModuleId);
        return XERROR;
    }

    pModuleEntry->fUsed = TRUE;
    pModuleEntry->nModuleId = nModuleId;
    pModuleEntry->nNumTasks = 0;
    pModuleEntry->platMode = pModInfo->bLinearHeap;

    strncpy((char*)pModuleEntry->pModuleName,
        (const char*)pModInfo->aModuleName, XOS_MM_MODULE_NAME_LEN);

    pModuleEntry->stMemInfo.nMaxStackSize = pModInfo->nMaxStackSize;
    if( XSUCC != XosModuleAdd(nModuleId, (XS8 *)(pModInfo->aObjFileName),pModInfo))
    {
        return XERROR;
    }
    return XSUCC;
}


/************************************************************************
������:    XosModuleDestroy
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosModuleDestroy( XOS_MM_MODULE_ID nModuleId )
{
    return XSUCC;
}


/************************************************************************
������:    XosModuleNameFromId
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosModuleNameFromId( XOS_MM_MODULE_ID nModuleId,XU8 * pModuleName )
{
    if(!pModuleName)
        return XERROR;
    if(XosModuleIdCheck(nModuleId))
    {
        strncpy( (char*)pModuleName,(const char*)&( gstWS_moduleTab[nModuleId].pModuleName ),
            XOS_MM_MODULE_NAME_LEN );
        pModuleName[XOS_MM_MODULE_NAME_LEN-1]=0;
        return XSUCC;
    }
    return XERROR;
}


/************************************************************************
������:    XosModuleAddTask
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosModuleAddTask( XOS_MM_MODULE_ID nModuleId, XOS_MM_TASK_ID nTaskId )
{
    XS32 i=0;

    if(XosModuleIdCheck(nModuleId))
    {
        if(gstWS_moduleTab[nModuleId].nNumTasks >= MAX_TASK_PER_MODULE)
            return XERROR;

        for( i=0;i<MAX_TASK_PER_MODULE;i++ )
        {
            if( 0 == gstWS_moduleTab[nModuleId].aTasks[i] )
            {
                gstWS_moduleTab[nModuleId].aTasks[i] = nTaskId;
                gstWS_moduleTab[nModuleId].nNumTasks++;
                break;
            }
        }
        return XSUCC;
    }
    return XERROR;
}


/************************************************************************
������:    XosModuleAdd
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosModuleAdd(XOS_MM_MODULE_ID nModuleId, XS8 *pName,XOS_MM_MOD_INIT_PARA *pModInfo)
{
    XOS_MM_MODULE_TAB    *pModule = NULL;
    XOS_MM_MOD_MEM_INFO* pModMemInfo = NULL;

    XOS_UNUSED(pModule);
    XOS_UNUSED(pModMemInfo);
    /* Allocate system common region */
    pModule = &gstWS_moduleTab[nModuleId];
    pModMemInfo = &(pModule->stMemInfo);

    return XSUCC;
}


/************************************************************************
������:    XosModuleIdCheck
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XBOOL XosModuleIdCheck(XOS_MM_MODULE_ID nModuleId)
{
    return ( nModuleId < XOS_MM_MAX_MODULES && gstWS_moduleTab[nModuleId].fUsed );
}


/************************************************************************
������:    XosModuleShow
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosModuleShow( XOS_MM_MODULE_ID nModuleId )
{
    XU32                dwIndex = 0;
    XOS_MM_MODULE_TAB *pstModule;
    XBOOL bRtn;

    bRtn = XosModuleIdCheck(nModuleId);
    if( !bRtn )
    {
        printf( "WS SSI : Invalid Module ID!\n" );
        return XERROR;
    }

    pstModule = gstWS_moduleTab + nModuleId;

    if( FALSE == pstModule->fUsed )
    {
        printf( "\nThe Module %d is a unused Module,No useful "
            "information is avalid!\n",nModuleId );
        return XSUCC;
    }

    printf( "\n-----------------------SSI MODULE INFORMATION"
        "-------------------------\n" );
    printf( "Module ID %d\tModule Name %s\t Number-of-Tasks %d\n",
        pstModule->nModuleId,pstModule->pModuleName,pstModule->nNumTasks );
    for( dwIndex=0;dwIndex<MAX_TASK_PER_MODULE;dwIndex++ )
    {
        printf( "Task ID[%d] %#x\t",dwIndex,pstModule->aTasks[dwIndex] );
        if( dwIndex == 3 || dwIndex == 7 )
            printf("\n");
    }
    printf( "\n=================================================="
        "====================\n ");
    printf( "\n--------------------------------------------------"
        "--------------------\n" );
    return XSUCC;
}


/************************************************************************
������:    XosModuleListShow
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosModuleListShow(void)
{
    XOS_MM_MODULE_TAB *pstModule;
    XOS_MM_MODULE_ID  nModuleId;

    printf("Id |Name            |Tsks|MemPool |MemBase   |MemSize   |StackBase|\n");
    for(nModuleId=0;nModuleId<XOS_MM_MAX_MODULES;nModuleId++)
    {
        pstModule = gstWS_moduleTab + nModuleId;

        if( FALSE == pstModule->fUsed )
            continue;
#ifdef XOS_ARCH_64
        printf("%3d|%-16s|%-2d  |0x%llx|%-10d|0x%llx|\n",
#else
        printf("%3d|%-16s|%-2d  |0x%08x|%-10d|0x%08x|\n",
#endif
            pstModule->nModuleId,
            pstModule->pModuleName,
            pstModule->nNumTasks,
            (XPOINT)pstModule->stMemInfo.pMemBase,
            pstModule->stMemInfo.dwMemSize,
            (XPOINT)pstModule->stMemInfo.pStackBase);
    }
    return XSUCC;
}


/************************************************************************
������:    XosModuleObjListShow
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
#ifdef XOS_VXWORKS
XOS_MM_STATUS XosModuleObjListShow(void)
{
    XOS_MM_MODULE_TAB *pstModule;
    XOS_MM_MODULE_ID  nModuleId;

    printf("Id |Name            |BssAddr |BssSize  |DataAddr|DataSize  |TextAddr|TextSize  |OBJName\n");
    for(nModuleId=0;nModuleId<XOS_MM_MAX_MODULES;nModuleId++)
    {
        pstModule = gstWS_moduleTab + nModuleId;

        if( FALSE == pstModule->fUsed )
            continue;
    }
    return XSUCC;
}
#endif


/************************************************************************
������:    XosModuleGetMdlEntry
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XVOID* XosModuleGetMdlEntry( XOS_MM_MODULE_ID dModuleID )
{
    if(dModuleID>=XOS_MM_MAX_MODULES)
        return NULL;
    return (XVOID*)&( gstWS_moduleTab[dModuleID] );
}


/************************************************************************
������:    XosModuleCheckTaskFull
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XBOOL XosModuleCheckTaskFull( XOS_MM_MODULE_ID dModuleId )
{
    if ( gstWS_moduleTab[dModuleId].nNumTasks >= MAX_TASK_PER_MODULE )
        return TRUE;
    return FALSE;
}


XOS_MM_STATUS XosModuleTriMapInit()
{
#ifdef WS_SUPPORT_TRI_SSI
    XS32 i;
    XS32    iTriModuleCount;

    /*Modified by wallace wu 2002-12-18*/
    /*iTriModuleCount = ENTLAST&0xFF;    original line*/
    iTriModuleCount = ENTLAST;
    /*#if 0*/
    for ( i = 0; i <= iTriModuleCount; i++ )
    {
        /*Modified by wallace wu 2002-12-18*/
        /*switch (i|0x0100) the original line*/
        switch ( i )
        {
        case ENTSS: /* ENTSS */
            gaTriEntModMap[i].nModuleId = WS_MOD_CM;/*WS_MOD_SS;*/
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSS", XOS_MM_TASK_NAME_LEN);
            break;
        case ENTHI: /* Tucl main task */
        case ENTHI1: /* Tucl receive task 1*/
        case ENTHI2: /* Tucl receive task 2*/
        case ENTHI3: /* Tucl receive task 3*/
            gaTriEntModMap[i].nModuleId = WS_MOD_HI;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTHI", XOS_MM_TASK_NAME_LEN);
            if ( i != ENTHI )
            {
                char pExtName[2]= {0};
                pExtName[0] = '1' + i - ENTHI1; /*ENTHI is not continuing with ENTHX.*/
                strncat(gaTriEntModMap[i].pTaskName,pExtName, 1);
            }
            break;

        case ENTSB: /* SCTP */
            gaTriEntModMap[i].nModuleId = WS_MOD_SB;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSB", XOS_MM_TASK_NAME_LEN);
            break;
        case ENTSX: /* SCTP_ADAPTOR */
            gaTriEntModMap[i].nModuleId = WS_MOD_SX;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSX", XOS_MM_TASK_NAME_LEN);
            break;
        case ENTDS: /* MSG Distr */
            gaTriEntModMap[i].nModuleId = WS_MOD_SS;/*WS_MOD_SS;*/
            strncpy(gaTriEntModMap[i].pTaskName, "ENTDS", XOS_MM_TASK_NAME_LEN);
            break;
        case ENTIT: /* M3UA */
            gaTriEntModMap[i].nModuleId = WS_MOD_IT;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIT", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTNF: /* M3UA NIF */
            gaTriEntModMap[i].nModuleId = WS_MOD_NF;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTNF", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSE: /* MTP1 */
            gaTriEntModMap[i].nModuleId = WS_MOD_SE;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSE", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSD: /* MTP2 */
            gaTriEntModMap[i].nModuleId = WS_MOD_SD;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSD", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSN: /* MTP3 */
            gaTriEntModMap[i].nModuleId = WS_MOD_SN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTDN: /* LDF-MTP3 */
            gaTriEntModMap[i].nModuleId = WS_MOD_DN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTDN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSP: /* SCCP */
            gaTriEntModMap[i].nModuleId = WS_MOD_SP;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSP", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTST: /* TCAP */
            gaTriEntModMap[i].nModuleId = WS_MOD_ST;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTST", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTTU: /* TCAP user */
            gaTriEntModMap[i].nModuleId = WS_MOD_TU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTTU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMA: /* MAP */
            gaTriEntModMap[i].nModuleId = WS_MOD_MA;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMA", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAU: /* MAP user */
            gaTriEntModMap[i].nModuleId = WS_MOD_AU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMG: /* MGCP MEGACO */
            gaTriEntModMap[i].nModuleId = WS_MOD_MG;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMG", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSO: /* SIP */
            gaTriEntModMap[i].nModuleId = WS_MOD_SO;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSO", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSV: /* SIP user */
            gaTriEntModMap[i].nModuleId = WS_MOD_SV;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSV", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSG: /* system manager */
            gaTriEntModMap[i].nModuleId = WS_MOD_SG;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSG", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSH: /* system agent */
            gaTriEntModMap[i].nModuleId = WS_MOD_SH;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSH", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMR: /* message router */
            gaTriEntModMap[i].nModuleId = WS_MOD_MR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTTP: /* TUP */
            gaTriEntModMap[i].nModuleId = WS_MOD_TP;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTTP", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTFM: /* SS7 fault manager */
            gaTriEntModMap[i].nModuleId = WS_MOD_FM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTFM", XOS_MM_TASK_NAME_LEN);
            break;
        case ENTPSH: /* PSH ha module */
            gaTriEntModMap[i].nModuleId = WS_MOD_PSH;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPSH", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTID:
            gaTriEntModMap[i].nModuleId = WS_MOD_ID;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTID", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMU:
            gaTriEntModMap[i].nModuleId = WS_MOD_MG;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTTS:
            gaTriEntModMap[i].nModuleId = WS_MOD_TS;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTTS", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPRM:
            gaTriEntModMap[i].nModuleId = WS_MOD_PRM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPRM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSC:
            gaTriEntModMap[i].nModuleId = WS_MOD_SC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLB:
            gaTriEntModMap[i].nModuleId = WS_MOD_LB;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLB", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLD:
            gaTriEntModMap[i].nModuleId = WS_MOD_LD;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLD", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTML:
            gaTriEntModMap[i].nModuleId = WS_MOD_ML;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTML", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTBD:
            gaTriEntModMap[i].nModuleId = WS_MOD_BD;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTBD", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTXI:
            gaTriEntModMap[i].nModuleId = WS_MOD_XI;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTXI", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTXN:
            gaTriEntModMap[i].nModuleId = WS_MOD_XN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTXN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTXG:
            gaTriEntModMap[i].nModuleId = WS_MOD_XG;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTXG", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAP:
            gaTriEntModMap[i].nModuleId = WS_MOD_AP;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAP", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTBR:
            gaTriEntModMap[i].nModuleId = WS_MOD_BR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTBR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTER:
            gaTriEntModMap[i].nModuleId = WS_MOD_ER;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTER", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMD:
            gaTriEntModMap[i].nModuleId = WS_MOD_MD;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMD", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAL:
            gaTriEntModMap[i].nModuleId = WS_MOD_AL;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAL", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTTC:
            gaTriEntModMap[i].nModuleId = WS_MOD_TC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTTC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIX:
            gaTriEntModMap[i].nModuleId = WS_MOD_IX;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIX", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTDB:
            gaTriEntModMap[i].nModuleId = WS_MOD_DB;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTDB", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIS:
            gaTriEntModMap[i].nModuleId = WS_MOD_IS;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIS", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLC:
            gaTriEntModMap[i].nModuleId = WS_MOD_LC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTOD:
            gaTriEntModMap[i].nModuleId = WS_MOD_OD;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTOD", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTHE:
            gaTriEntModMap[i].nModuleId = WS_MOD_HE;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTHE", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTCC:
            gaTriEntModMap[i].nModuleId = WS_MOD_CC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTCC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTUD:
            gaTriEntModMap[i].nModuleId = WS_MOD_UD;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTUD", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM1:
            gaTriEntModMap[i].nModuleId = WS_MOD_M1;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM1", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM2:
            gaTriEntModMap[i].nModuleId = WS_MOD_M2;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM2", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM3:
            gaTriEntModMap[i].nModuleId = WS_MOD_M3;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM3", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM4:
            gaTriEntModMap[i].nModuleId = WS_MOD_M4;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM4", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM5:
            gaTriEntModMap[i].nModuleId = WS_MOD_M5;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM5", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM6:
            gaTriEntModMap[i].nModuleId = WS_MOD_M6;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM6", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM7:
            gaTriEntModMap[i].nModuleId = WS_MOD_M7;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM7", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM8:
            gaTriEntModMap[i].nModuleId = WS_MOD_M8;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM8", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTM9:
            gaTriEntModMap[i].nModuleId = WS_MOD_M9;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTM9", XOS_MM_TASK_NAME_LEN);
            break;
            
        case ENTME:
            gaTriEntModMap[i].nModuleId = WS_MOD_ME;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTME", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAC:
            gaTriEntModMap[i].nModuleId = WS_MOD_AC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAS:
            gaTriEntModMap[i].nModuleId = WS_MOD_AS;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAS", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLDLM:
            gaTriEntModMap[i].nModuleId = WS_MOD_LDLM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLDLM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTUM:
            gaTriEntModMap[i].nModuleId = WS_MOD_UM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTUM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLR:
            gaTriEntModMap[i].nModuleId = WS_MOD_LR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLU:
            gaTriEntModMap[i].nModuleId = WS_MOD_LU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTRY:
            gaTriEntModMap[i].nModuleId = WS_MOD_RY;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTRY", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTEC:
            gaTriEntModMap[i].nModuleId = WS_MOD_EC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTEC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTFA:
            gaTriEntModMap[i].nModuleId = WS_MOD_FA;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTFA", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSR:
            gaTriEntModMap[i].nModuleId = WS_MOD_SR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTES:
            gaTriEntModMap[i].nModuleId = WS_MOD_ES;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTES", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPN:
            gaTriEntModMap[i].nModuleId = WS_MOD_PN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTQI:
            gaTriEntModMap[i].nModuleId = WS_MOD_QI;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTQI", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTWD:
            gaTriEntModMap[i].nModuleId = WS_MOD_WD;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTWD", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTWN:
            gaTriEntModMap[i].nModuleId = WS_MOD_WN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTWN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTWI:
            gaTriEntModMap[i].nModuleId = WS_MOD_WI;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTWI", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTWU:
            gaTriEntModMap[i].nModuleId = WS_MOD_WU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTWU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTWS:
            gaTriEntModMap[i].nModuleId = WS_MOD_WS;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTWS", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTWC:
            gaTriEntModMap[i].nModuleId = WS_MOD_WC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTWC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPU:
            gaTriEntModMap[i].nModuleId = WS_MOD_PU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSA:
            gaTriEntModMap[i].nModuleId = WS_MOD_SA;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSA", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTFN:
            gaTriEntModMap[i].nModuleId = WS_MOD_FN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTFN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTNV:
            gaTriEntModMap[i].nModuleId = WS_MOD_NV;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTNV", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLV:
            gaTriEntModMap[i].nModuleId = WS_MOD_LV;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLV", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTEV:
            gaTriEntModMap[i].nModuleId = WS_MOD_EV;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTEV", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPL:
            gaTriEntModMap[i].nModuleId = WS_MOD_PL;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPL", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTVM:
            gaTriEntModMap[i].nModuleId = WS_MOD_VM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTVM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAF:
            gaTriEntModMap[i].nModuleId = WS_MOD_AF;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAF", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTFR:
            gaTriEntModMap[i].nModuleId = WS_MOD_FR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTFR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMT:
            gaTriEntModMap[i].nModuleId = WS_MOD_MT;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMT", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTCV:
            gaTriEntModMap[i].nModuleId = WS_MOD_CV;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTCV", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMV:
            gaTriEntModMap[i].nModuleId = WS_MOD_MV;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMV", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIM:
            gaTriEntModMap[i].nModuleId = WS_MOD_IM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTBV:
            gaTriEntModMap[i].nModuleId = WS_MOD_BV;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTBV", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPA:
            gaTriEntModMap[i].nModuleId = WS_MOD_PA;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPA", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPV:
            gaTriEntModMap[i].nModuleId = WS_MOD_PV;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPV", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLK:
            gaTriEntModMap[i].nModuleId = WS_MOD_LK;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLK", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTL1:
            gaTriEntModMap[i].nModuleId = WS_MOD_L1;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTL1", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIA:
            gaTriEntModMap[i].nModuleId = WS_MOD_IA;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIA", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIU:
            gaTriEntModMap[i].nModuleId = WS_MOD_IU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIU", XOS_MM_TASK_NAME_LEN);
            break;
        case ENTRM:
            gaTriEntModMap[i].nModuleId = WS_MOD_RM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTRM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTRT:
            gaTriEntModMap[i].nModuleId = WS_MOD_RT;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTRT", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPQ:
            gaTriEntModMap[i].nModuleId = WS_MOD_PQ;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPQ", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMC:
            gaTriEntModMap[i].nModuleId = WS_MOD_MC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIE:
            gaTriEntModMap[i].nModuleId = WS_MOD_IE;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIE", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTBW:
            gaTriEntModMap[i].nModuleId = WS_MOD_BW;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTBW", XOS_MM_TASK_NAME_LEN);
            break;

            /*case ENTIW: */
        case ENTSI:
            gaTriEntModMap[i].nModuleId = WS_MOD_SI;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSI", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTQW:
            gaTriEntModMap[i].nModuleId = WS_MOD_QW;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTQW", XOS_MM_TASK_NAME_LEN);
            break;

            /*case ENTAW:*/
        case ENTAM:
            gaTriEntModMap[i].nModuleId = WS_MOD_AM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSF:
            gaTriEntModMap[i].nModuleId = WS_MOD_SF;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSF", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTVF:
            gaTriEntModMap[i].nModuleId = WS_MOD_VF;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTVF", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTCL:
            gaTriEntModMap[i].nModuleId = WS_MOD_CL;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTCL", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTL4:
            gaTriEntModMap[i].nModuleId = WS_MOD_L4;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTL4", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTTT:
            gaTriEntModMap[i].nModuleId = WS_MOD_TT;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTTT", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPR:
            gaTriEntModMap[i].nModuleId = WS_MOD_PR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGN:
            gaTriEntModMap[i].nModuleId = WS_MOD_GN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGG:
            gaTriEntModMap[i].nModuleId = WS_MOD_GG;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGG", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAR:
            gaTriEntModMap[i].nModuleId = WS_MOD_AR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGT:
            gaTriEntModMap[i].nModuleId = WS_MOD_GT;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGT", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGM:
            gaTriEntModMap[i].nModuleId = WS_MOD_GM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGR:
            gaTriEntModMap[i].nModuleId = WS_MOD_GR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGW:
            gaTriEntModMap[i].nModuleId = WS_MOD_GW;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGW", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGL:
            gaTriEntModMap[i].nModuleId = WS_MOD_GL;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGL", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGS:
            gaTriEntModMap[i].nModuleId = WS_MOD_GS;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGS", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGZ:
            gaTriEntModMap[i].nModuleId = WS_MOD_GZ;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGZ", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGY:
            gaTriEntModMap[i].nModuleId = WS_MOD_GY;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGY", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTHC:
            gaTriEntModMap[i].nModuleId = WS_MOD_HC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTHC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTHU:
            gaTriEntModMap[i].nModuleId = WS_MOD_HU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTHU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTHR:
            gaTriEntModMap[i].nModuleId = WS_MOD_HR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTHR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTNM:
            gaTriEntModMap[i].nModuleId = WS_MOD_NM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTNM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGB:
            gaTriEntModMap[i].nModuleId = WS_MOD_GB;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGB", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGP:
            gaTriEntModMap[i].nModuleId = WS_MOD_GP;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGP", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIQ:
            gaTriEntModMap[i].nModuleId = WS_MOD_IQ;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIQ", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTXM:
            gaTriEntModMap[i].nModuleId = WS_MOD_XM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTXM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTHG:
            gaTriEntModMap[i].nModuleId = WS_MOD_HG;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTHG", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTVO:
            gaTriEntModMap[i].nModuleId = WS_MOD_VO;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTVO", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGO:
            gaTriEntModMap[i].nModuleId = WS_MOD_GO;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGO", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGI:
            gaTriEntModMap[i].nModuleId = WS_MOD_GI;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGI", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGH:
            gaTriEntModMap[i].nModuleId = WS_MOD_GH;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGH", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGU:
            gaTriEntModMap[i].nModuleId = WS_MOD_GU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTLN:
            gaTriEntModMap[i].nModuleId = WS_MOD_LN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTLN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTFW:
            gaTriEntModMap[i].nModuleId = WS_MOD_FW;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTFW", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTRL:
            gaTriEntModMap[i].nModuleId = WS_MOD_RL;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTRL", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAL2:
            gaTriEntModMap[i].nModuleId = WS_MOD_AL2;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAL2", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTAA:
            gaTriEntModMap[i].nModuleId = WS_MOD_AA;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTAA", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTRA:
            gaTriEntModMap[i].nModuleId = WS_MOD_RA;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTRA", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTRN:
            gaTriEntModMap[i].nModuleId = WS_MOD_RN;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTRN", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTDP:
            gaTriEntModMap[i].nModuleId = WS_MOD_DP;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTDP", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTDT:
            gaTriEntModMap[i].nModuleId = WS_MOD_DT;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTDT", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTNP:
            gaTriEntModMap[i].nModuleId = WS_MOD_NP;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTNP", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTP1:
            gaTriEntModMap[i].nModuleId = WS_MOD_P1;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTP1", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTND:
            gaTriEntModMap[i].nModuleId = WS_MOD_ND;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTND", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTDM:
            gaTriEntModMap[i].nModuleId = WS_MOD_DM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTDM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSU:
            gaTriEntModMap[i].nModuleId = WS_MOD_SU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTRU:
            gaTriEntModMap[i].nModuleId = WS_MOD_RU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTRU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTQC:
            gaTriEntModMap[i].nModuleId = WS_MOD_QC;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTQC", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTCU:
            gaTriEntModMap[i].nModuleId = WS_MOD_CU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTCU", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMM:
            gaTriEntModMap[i].nModuleId = WS_MOD_MM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGA:
            gaTriEntModMap[i].nModuleId = WS_MOD_GA;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGA", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGE:
            gaTriEntModMap[i].nModuleId = WS_MOD_GE;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGE", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMW:
            gaTriEntModMap[i].nModuleId = WS_MOD_MW;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMW", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTNW:
            gaTriEntModMap[i].nModuleId = WS_MOD_NW;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTNW", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTDI:
            gaTriEntModMap[i].nModuleId = WS_MOD_DI;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTDI", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTMK:
            gaTriEntModMap[i].nModuleId = WS_MOD_MK;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTMK", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTRR:
            gaTriEntModMap[i].nModuleId = WS_MOD_RR;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTRR", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIB:
            gaTriEntModMap[i].nModuleId = WS_MOD_IB;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIB", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTPH:
            gaTriEntModMap[i].nModuleId = WS_MOD_PH;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTPH", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTSM:
            /*case ENTII: */
            gaTriEntModMap[i].nModuleId = WS_MOD_SM;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTSM", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTUL:
            gaTriEntModMap[i].nModuleId = WS_MOD_UL;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTUL", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGD:
            gaTriEntModMap[i].nModuleId = WS_MOD_GD;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGD", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTGX:
            gaTriEntModMap[i].nModuleId = WS_MOD_GX;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTGX", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTDG:
            gaTriEntModMap[i].nModuleId = WS_MOD_DG;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTDG", XOS_MM_TASK_NAME_LEN);
            break;

        case ENTIEU:
            gaTriEntModMap[i].nModuleId = WS_MOD_IEU;
            strncpy(gaTriEntModMap[i].pTaskName, "ENTIEU", XOS_MM_TASK_NAME_LEN);
            break;

        default:
            break;
        }
    }
    /*#endif*/
    /*End of modified by wallace*/

#endif

    return XSUCC;
}


/************************************************************************
������:    XosModuleGetTriId
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_MODULE_ID XosModuleGetTriId(XU16 nEntId)
{
    if ( nEntId & 0xFF00 )
    {
        return XERROR;
    }
    return gaTriEntModMap[nEntId].nModuleId;
}


/************************************************************************
������:    XosMMHeapIsLinear
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XBOOL XosMMHeapIsLinear( XOS_MM_MODULE_ID dwMdlID )
{
    XOS_MM_MODULE_TAB * pMdlEntry = NULL;
    pMdlEntry = (XOS_MM_MODULE_TAB *) XosModuleGetMdlEntry( dwMdlID );
    if(pMdlEntry)
        return pMdlEntry->platMode;
    return FALSE;
}


/************************************************************************
������:    XosMMSysMemShow
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
void XosMMSysMemShow( void )
{
}


/************************************************************************
������:    XosMMTaskIdSelf
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_TASK_ID XosMMTaskIdSelf(void)
{
    XOS_MM_TASK_ID wsTaskId;

#ifdef WS_SUPPORT_TRI_SSI
    XU16 entId, instId;
#endif

    /*
    * Get current OS-defined task id and SSI-defined task id.
    */
#ifdef XOS_VXWORKS
    WIND_TCB* osTaskId = (WIND_TCB*)taskIdSelf();
    wsTaskId = osTaskId->spare3;
#elif defined(XOS_WIN32)
    XU32 osTaskId = (XU32)GetCurrentThreadId();
    wsTaskId = XosMMTaskGetId();
#else
    XU32 osTaskId = (XU32)pthread_self();
    wsTaskId = XosMMTaskGetId();
#endif

    if (wsTaskId && wsTaskId < XOS_MM_MAX_TASKS)
    {
        if (gstWS_osTaskId[wsTaskId] == (XU32)osTaskId)
        {
            return wsTaskId;
        }
        else
        {
        /*
        * Ugh!!! In linux a thread can run before pthread_create
        * return, so gstWS_osTaskId[wsTaskId] = 0, while
        * XosMMTaskSetId has already be called.
            */
#ifdef XOS_VXWORKS
            osTaskId->spare3 = wsTaskId;
#else
            XosMMTaskSetId(wsTaskId);
#endif
            gstWS_osTaskId[wsTaskId] = (XU32)osTaskId;
            return wsTaskId;
        }
    }

    if (wsTaskId == XOS_MM_NON_SSI_TASK_ID)
        return XOS_MM_INVALID_TASK_ID;

#ifdef WS_SUPPORT_TRI_SSI
        /*
        * It is not a ws_ssi task. Check whether a trillium task.
        * A trillium task returns the entId as the taskId
    */
    TriSsGetCurTsk(&entId, &instId);
    if (entId && entId < XOS_MM_MIN_TASKS)
    {
#ifdef XOS_VXWORKS
        osTaskId->spare3 = entId;
#else
        XosMMTaskSetId(entId);
#endif
        gstWS_osTaskId[entId] = entId;
        return entId;
    }
#endif

#ifdef XOS_VXWORKS
    osTaskId->spare3 = XOS_MM_NON_SSI_TASK_ID;
#else
    XosMMTaskSetId(XOS_MM_NON_SSI_TASK_ID);
#endif

    return XOS_MM_INVALID_TASK_ID;
}


/************************************************************************
������:    XosMMTaskLibInit
���ܣ�
������
�����
���أ�
˵����
************************************************************************/
XOS_MM_STATUS XosMMTaskLibInit()
{
#ifdef XOS_VXWORKS
    gWS_taskLibStackProtRealAddr = malloc(1024*1024*2); /*2M*/
    if(gWS_taskLibStackProtRealAddr == NULL)
    {
        printf("Stack Protection memory allocate failed\n");
        return XERROR;
    }
#endif

#if (defined(XOS_WIN32) || defined(XOS_SOLARIS) || defined(XOS_LINUX))
    /*XosMMTaskInitIdKey();*/
#if defined(XOS_SOLARIS) || defined(XOS_LINUX)
    if (pthread_key_create(&gWSTaskIdKey, NULL) != 0)
        printf("[SSI] WS_taskInitIdKey failed: %s\n", strerror(errno));
#elif defined(XOS_WIN32)
    if ((gWSTaskIdKey = TlsAlloc()) == 0xFFFFFFFF)
        printf("[SSI] WS_taskInitIdKey failed: %s\n", strerror(errno));
#endif
    memset(gWSTaskIdMap, 0, sizeof(gWSTaskIdMap));

#endif
    return XSUCC;
}


/************************************************************************/
#ifdef XOS_MDLMGT_TEST
XS32  testmgtmain(HANDLE hDir,XS32 argc, XS8** argv)
{
    printf("Start test 1 module.\n");

    return MODULE_INIT_OK;
}
#endif /* XOS_MDLMGT_TEST */


/************************************************************************
������:    XosMM_XMLreadSrvCfg
���ܣ���ҵ��ģ���Ӧ��XML�����ȡģ�����������Ϣ��
������filename ---xml�ļ���ָ��
�����parsedDoc---xml�ļ�����������ͽṹָ��
���أ�XERROR--ʧ��
                  XSUCC--�ɹ�
˵����
************************************************************************/
XS32 XosMM_XMLreadSrvCfg( sys_reg_tbl_t * parsedDoc,XS8 *filename)
{
    xmlDocPtr  doc      = NULL;
    xmlNodePtr cur      = NULL;
    xmlNodePtr level1cur  = NULL;
    xmlNodePtr level2cur = NULL;
    xmlNodePtr level3cur = NULL;
    xmlChar*   pTempStr = XNULL;
    XU32 ulfilenamelen = 0;
    XCHAR szSrvCfgFName[XOS_MAX_PATHLEN]={0};

    sys_reg_key_ptr tmpRegPtr=NULL,curRegPtr=NULL,curReg1Ptr=NULL,curReg2Ptr=NULL,curReg3Ptr=NULL;
    XU8        level1firstnode,level2firstnode,level3firstnode;
#ifdef XOS_EW_START
    XCHAR szCfgFName[XOS_MAX_PATHLEN]={0};
#endif

    ulfilenamelen = (XU32)XOS_StrLen(filename);

    if(XOS_StrLen("*.xml") > ulfilenamelen 
        || (XOS_MAX_PATHLEN - XOS_StrLen(".\\")) < ulfilenamelen)//�����С�����ж�
    {
        printf("filename len(%d) err!\n",ulfilenamelen);
        return XERROR;
    }

#ifdef XOS_EW_START
    if( XSUCC != XOS_GetSysPath(szCfgFName, XOS_MAX_PATHLEN))
    {
        return XERROR;
    }
    XOS_StrCat(szCfgFName, filename);
#endif

    if ( XNULL == parsedDoc )
    {
        return XERROR;
    }

#if defined(XOS_LINUX) || defined(XOS_SOLARIS)
    XOS_StrCat(szSrvCfgFName,"./");
    XOS_StrCat(szSrvCfgFName,filename);
#ifndef XOS_EW_START
    doc = xmlParseFile(szSrvCfgFName);
#else
    doc = xmlParseFile(szCfgFName);
#endif
#endif

#if defined(XOS_WIN32) || defined(XOS_VTA)
    XOS_StrCat(szSrvCfgFName,".\\");
    XOS_StrCat(szSrvCfgFName,filename);
#ifndef XOS_EW_START
    doc = xmlParseFile(szSrvCfgFName);
#else
    doc = xmlParseFile(szCfgFName);
#endif
#endif
    if (doc == XNULL)
    {
        return( XERROR );
    }
    cur = xmlDocGetRootElement(doc);
    if (cur == XNULL)
    {
        goto ERR_OUT_LABLE;
    }
    /* �ҵ�system���ڵ� */
    if ( !XOS_StrCmp( cur->name, "System") )
    {
        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
        if(NULL==tmpRegPtr)
        {
            printf("XosMMStartDocument not enough element memory\n");
        }
        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
        XosMMParseStrCpy((XU8*)tmpRegPtr->name,(XU8*)cur->name,MAX_KEYNAME_LENGTH);
        parsedDoc->rootDir=tmpRegPtr;
    }
    else
    {
        goto ERR_OUT_LABLE;
    }
    level1cur = cur->xmlChildrenNode;
    curRegPtr = parsedDoc->rootDir;

    while ( level1cur && xmlIsBlankNode ( level1cur ) )
    {
        level1cur = level1cur -> next;
    }
    if ( level1cur == XNULL )
    {
        goto ERR_OUT_LABLE;
    }

    level1firstnode = 1;
    /* Ŀǰģ�����ֻ֧������Ƕ�״��� */
    while ( level1cur != XNULL )
    {
        if(XOS_StrLen(level1cur->name) > 0)/* ��һ������ */
        {
            /* ��һ������ */
            tmpRegPtr = NULL;
            tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
            if(NULL==tmpRegPtr)
            {
                printf("XosMMStartDocument not enough element memory\n");
                goto ERR_OUT_LABLE;
            }
            memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
            XosMMParseStrCpy((XU8*)tmpRegPtr->name,(XU8*)level1cur->name,MAX_KEYNAME_LENGTH);
            if(level1firstnode)
            {
                curRegPtr->pFirstSonKey = tmpRegPtr;
                curReg1Ptr = curRegPtr->pFirstSonKey;
                level1firstnode = 0;
            }
            else
            {
                curReg1Ptr->pNextBrotherKey = tmpRegPtr;
                curReg1Ptr = curReg1Ptr->pNextBrotherKey;
            }
            
            level2cur = level1cur->xmlChildrenNode;
            while ( level2cur && xmlIsBlankNode ( level2cur ) )
            {
                level2cur = level2cur -> next;
            }
            level2firstnode = 1;
            while ( level2cur != XNULL) /* �ڶ������� */
            {
                if(XOS_StrLen(level2cur->name) > 0)
                {
                    tmpRegPtr = NULL;
                    tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
                    if(NULL==tmpRegPtr)
                    {
                        printf("XosMMStartDocument not enough element memory\n");
                        goto ERR_OUT_LABLE;
                    }
                    memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
                    XosMMParseStrCpy((XU8*)tmpRegPtr->name,(XU8*)level2cur->name,MAX_KEYNAME_LENGTH);

                    pTempStr = xmlNodeListGetString( doc, level2cur->xmlChildrenNode, 1);
                    if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
                    {
                        XosMMParseStrCpy((XU8*)tmpRegPtr->value,(XU8*)pTempStr,MAX_KEYVALUE_LENGTH);
                    }
                    if(pTempStr != NULL) {
                        xmlFree(pTempStr);
                    }

                    if(level2firstnode)
                    {
                        curReg1Ptr->pFirstSonKey = tmpRegPtr;
                        curReg2Ptr = curReg1Ptr->pFirstSonKey;
                        level2firstnode = 0;
                    }
                    else
                    {
                        curReg2Ptr->pNextBrotherKey = tmpRegPtr;
                        curReg2Ptr = curReg2Ptr->pNextBrotherKey;
                    }
                }

                level3cur = level2cur->xmlChildrenNode;
                while ( level3cur && xmlIsBlankNode ( level3cur ) )
                {
                    level3cur = level3cur -> next;
                }
                level3firstnode = 1;
                while ( level3cur != XNULL) /* ���������� */
                {
                    pTempStr = xmlNodeListGetString( doc, level3cur->xmlChildrenNode, 1);
                    if ( XNULL != pTempStr && XOS_StrLen(pTempStr) > 0 )
                    {
                        tmpRegPtr = NULL;
                        tmpRegPtr = (sys_reg_key_ptr)malloc(sizeof(sys_reg_key_t));
                        if(NULL==tmpRegPtr)
                        {
                            printf("XosMMStartDocument not enough element memory\n");
                            goto ERR_OUT_LABLE;
                        }
                        memset(tmpRegPtr,0,sizeof(sys_reg_key_t));
                        XosMMParseStrCpy((XU8*)tmpRegPtr->name,(XU8*)level3cur->name,MAX_KEYNAME_LENGTH);
                        XosMMParseStrCpy((XU8*)tmpRegPtr->value,(XU8*)pTempStr,MAX_KEYVALUE_LENGTH);
                        if(level3firstnode)
                        {
                            curReg2Ptr->pFirstSonKey = tmpRegPtr;
                            curReg3Ptr = curReg2Ptr->pFirstSonKey;
                            level3firstnode = 0;
                        }
                        else
                        {
                            curReg3Ptr->pNextBrotherKey = tmpRegPtr;
                            curReg3Ptr = curReg3Ptr->pNextBrotherKey;
                        }
                    }
                    if(pTempStr != NULL) {
                        xmlFree(pTempStr);
                    }
                    /* ���������� */
                    level3cur = level3cur->next;
                    while ( level3cur && xmlIsBlankNode ( level3cur ) )
                    {
                        level3cur = level3cur -> next;
                    }
                }
                level2cur = level2cur->next;
                while ( level2cur && xmlIsBlankNode ( level2cur ) )
                {
                    level2cur = level2cur -> next;
                }
            }
        }
        level1cur = level1cur->next;
        while ( level1cur && xmlIsBlankNode ( level1cur ) )
        {
            level1cur = level1cur -> next;
        }
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return XSUCC;

ERR_OUT_LABLE:
    XosMMFreeRegTable(parsedDoc);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return( XERROR);
    /*  XOS Vxworks begin  */
}


/************************************************************************
������:    XosMMCreateSrvModules
���ܣ���ȡģ���ʼ������������ģ�����ݽṹ��
������moduleidx--ģ���������
�������
���أ�XSUCC--�ɹ�
                  XERROR--ʧ��
˵����ģ����Ϣ��˳�򱣴���ȫ������gMMModuleList��
************************************************************************/
XS32 XosMMCreateSrvModules(XU32 moduleidx)
{
    XS32 i = 0;
    XS32 result = XSUCC;
    XOS_MM_MODULE_T* pModule;

    MMInfo("Begin creating Srv modules, please wait...");
    printf("\n-----------------------------------------------------------\n");
    for(i = moduleidx; i <= MAX_MODULE_NUMBER;i++)
    {
        pModule = &gMMModuleList[i];
        /*�鿴ϵͳע���Ŀ¼�Ƿ���ģ��Ŀ¼������ǣ��ʹ���ģ�顣*/
        if(pModule->modHDir)
        {
            result = XosMMCreateModule(pModule->modHDir, &(gMMModuleList[i]));
            if(XSUCC!=result)
            {
                return result;
            }
            if(gMMModuleList[i].pEntryFunc==NULL)
                break;
            /* set gdCurrentLoadModuleID; modified by jeff.zheng 2005.5*/
            XosMMSetCurrentLoadModule((HANDLE)i);

            /* Start up this module */
            XosMMSetCurLdModId(gMMModuleList[i].wsModuleId);
            result = XosMMRunModule( &gMMModuleList[i]);
            
            gMMModuleList[i].module_init_flag = result;
            if( MODULE_INIT_OK != result )
            {
                goto FatalError;
            }

            XosMSMMtaskSleep(200);
            printf("\r\n----------------Continue with Next Srv Module------------------\n");
        }
    }
    XosMSMMtaskSleep(500);
    goto NormalExit;

FatalError:
    MMErr("There are fatal errors while running module \"%s\"",
                                gMMModuleList[i].modName);
    return XERROR;

NormalExit:
    return XSUCC;
}


/************************************************************************
������:    XosModuleStartSrv
���ܣ�PMģ������ҵ��ģ��ӿ�
������filename--ҵ��ģ���Ӧxml�ļ�ָ��
�������
���أ�XSUCC--�ɹ�
                  XERROR--ʧ��
˵����
************************************************************************/
XS32 XosModuleStartSrv(XCONST XS8 *filename)
{
    XS32 ret;
    HANDLE hRootDir = (HANDLE)NULL;
    HANDLE hModuleDir = (HANDLE)NULL;
    XS8 dirName[MAX_KEYNAME_LENGTH];
    XU32 tempU32Value=0;
    XOS_MM_MODULE_T* pModule=NULL;
    XU32 moduleCount=0;
    XU32 taskCount=0;
    XU32 stackTotalSize = 0;
    XU32 objTotal=0;
    XU32 result = XSUCC;
    XS8 objFileName[MAX_KEYVALUE_LENGTH+12] = "";
    XS8 objFilePath[MAX_KEYVALUE_LENGTH+12] = "";
    XS8 nameBuffer[MAX_KEYVALUE_LENGTH+MAX_KEYVALUE_LENGTH+10] = "";

    memset(dirName, 0, MAX_KEYNAME_LENGTH);

    /*��ȡ�ź���*/
    XOS_SemGet(&(gMMSystemInfo.sem));

    memset(gSysRegTable,0,sizeof(sys_reg_tbl_t));
    ret = XosMM_XMLreadSrvCfg(gSysRegTable,(XS8 *)filename);
    if(XSUCC != ret)
    {
        MMInfo("XosMM_XMLreadSrvCfg fail!");
        return XERROR;
    }
    hRootDir = XosSysRegGetRootDir();

    XosSysRegEnumDir(hRootDir, (HANDLE)NULL, dirName, &hModuleDir);
    for(moduleCount = gMMSystemInfo.moduleCount; (moduleCount < MAX_MODULE_NUMBER) && (hModuleDir!=(HANDLE)NULL);)
    {
    /*decide whether system registry table directory is MODULE directory,
        if yes, create module*/
        if(0 == strncmp(dirName, "Module",6))
        {
            pModule = &(gMMModuleList[moduleCount]);
            moduleCount++;

            pModule->modHDir = hModuleDir;
            /* ģ�������� */
            if(XSUCC==XosSysRegQueryU32KeyValue(hModuleDir, KEYNAME_TASKCOUNT,&tempU32Value))
            {
                taskCount+=tempU32Value;
            }
            /* ��ջ��С */
            if(XSUCC==XosSysRegQueryU32KeyValue(hModuleDir, KEYNAME_STACKSIZE,&tempU32Value))
            {
                stackTotalSize += tempU32Value;
            }
            strcpy(nameBuffer,"");
            /* Ŀ���ļ�·�� */
            result = XosSysRegQueryStrKeyValue(hModuleDir, OBJFILEPATHKEY, objFilePath);
            if(result==XSUCC)
            {
                strcat(nameBuffer,objFilePath);
            }
            /* Ŀ���ļ����� */
            result = XosSysRegQueryStrKeyValue(hModuleDir, OBJFILENAMEKEY, objFileName);
            if(result==XSUCC)
            {
                strcat(nameBuffer,objFileName);
            }
            result = (XU32)strlen(nameBuffer);
            if(result>=XOS_MM_FILE_NAME_LEN)
            {
                MMErr("OBJName Too Long: Max is %d, current is %d, %s",
                    XOS_MM_FILE_NAME_LEN,result,nameBuffer);
                return XERROR;
            }
            if(XSUCC!=XosMMCalObjInfo(nameBuffer, &tempU32Value,pModule))
            {
                return XERROR;
            }

#ifdef XOS_VXWORKS
#if 0
            printf("Module BssSize:%-10d,TxtSize:%-10d,DataSize:%-10d\n",
                pModule->segSizeBss,pModule->segSizeText,
                pModule->segSizeData);
#endif
#endif

            objTotal += tempU32Value;
        }
        else/* if(0 != strncmp(dirName, "Global",6))*/
        {
            MMErr("Find an invalid Module Tag:%s",dirName);
            return XERROR;
        }
        /*��һ��ģ��*/
        XosSysRegEnumDir(hRootDir,hModuleDir, dirName, &hModuleDir);
        continue;
    }
    //printf("Done\n");

    gMMSystemInfo.taskCount   += taskCount;
    gMMSystemInfo.stackTotal  += stackTotalSize;
    gMMSystemInfo.objTotal    += objTotal;
    
    if(gMMSystemInfo.moduleCount == moduleCount)
    {
        MMErr("NO Srv module Need to Start.\n");
        return XERROR;
    }

    gMMWsSsiInitPara.moduleRegionSize = gMMSystemInfo.stackTotal+gMMSystemInfo.objTotal;

    /*  �������а�ģ�� */
    ret = XosMMCreateSrvModules(gMMSystemInfo.moduleCount);
    if(ret != XSUCC)
    {
        MMInfo("begin modules failed!");
    }

    XOS_SemPut(&(gMMSystemInfo.sem));
    gMMSystemInfo.moduleCount = moduleCount;

    MMInfo("Total Module Count: %d",gMMSystemInfo.moduleCount);
    MMInfo("Total Task   Count: %d",gMMSystemInfo.taskCount);
    MMInfo("Total Stack  Size : %08x(%d)",gMMSystemInfo.stackTotal,gMMSystemInfo.stackTotal);
    MMInfo("Total Obj   Size  : %08x(%d)",gMMSystemInfo.objTotal,gMMSystemInfo.objTotal);
    printf("\r\n");
    MMInfo("Service Module Manager Finished!");

    return XSUCC;
}


/************************************************************************
������: XOS_SetCpuBind
���ܣ�  ����ƽ̨cpu��
���룺  ptCpuBind cpu�󶨵Ľṹ����Ϣ
�����
���أ� �ɹ����� 0�� ʧ�ܷ��� -1
˵���� �����ƽ̨�������߳���Ч���󶨺�����cfg.xml global����
       �����module�������ã���ģ���ڣ�ģ������ûḲ�Ǵ�ȫ������
************************************************************************/
XS32 XOS_SetCpuBind(t_CpuBindCfg *ptCpuBind)
{
    XS32 ret = 0;

    if (NULL == ptCpuBind)
    {
        printf("param err! is null\n");
        return XERROR;
    }


#ifdef XOS_LINUX
    ret = XOS_SetThreadAffinity(ptCpuBind->Cpus, ptCpuBind->cpunum);
#else
    /* ����ƽ̨�ݻ�û�����󣬺�������Ҫ����� */
    ret = 0;
#endif
    if (ret)
    {        
        MMErr("XOS_SetThreadAffinity return fail! ret:%d\n",ret);
        return XERROR; 
    }

    return XSUCC;
}

/************************************************************************
������: XOS_ParseCpuBindCfg
���ܣ�  �����������ļ��ж�ȡ�İ�cpu�������ַ������ַ����ԣ��ָ�
���룺 value  : �������ļ��н������ַ���
����� pCpuBindCfg : ���������ַ������ݷָ���','���������˽ṹ����
���أ� ��
˵����
************************************************************************/
XVOID XOS_ParseCpuBindCfg(t_CpuBindCfg* pCpuBindCfg, XS8 *value)
{
    XS8 *ptr = NULL;

    if (NULL == pCpuBindCfg || NULL == value)
    {
        return;
    }
    
    XOS_MemSet(pCpuBindCfg, 0, sizeof(t_CpuBindCfg));
    
#ifdef XOS_LINUX
    XS32 i = 0;
    ptr = strtok(value,",");
    while(NULL != ptr && i < MAX_CPUCORE_NUM)
    {
        pCpuBindCfg->Cpus[i] = (XU16)atol((char*)ptr);        
        i++;
        ptr = strtok(NULL, ",");        
    }

    pCpuBindCfg->cpunum = i;
#endif
}

#if 1
/************************************************************************
������:    XosModuleGetCfgSize
���ܣ���ȡ������Ϣ��Ҫ���ڴ��С
��������
�������
���أ�����������Ϣ��Ҫ���ڴ��С
                  XERROR--ʧ��
˵����
************************************************************************/
XS32 XosModuleGetCfgSize(XVOID)
{
    return sizeof(sys_reg_tbl_t);
}

/************************************************************************
������:    XosModuleGetCfgInfo
���ܣ���ȡ������Ϣ
������filename--ҵ��ģ���Ӧxml�ļ�ָ��
�����modInfo--������Ϣ
���أ�XSUCC--�ɹ�
                  XERROR--ʧ��
˵����
************************************************************************/
XS32 XosModuleGetCfgInfo(XCONST XS8 *filename, XU8 *modInfo)
{
    XS32 ret;
    
    if(NULL == modInfo)
    {
        MMInfo("XosModuleGetCfgInfo : para modInfo = NULL!");
        return XERROR;
    }
    
    /*��ȡ�ź���*/
    //XOS_SemGet(&(gMMSystemInfo.sem));

    memset(modInfo,0,sizeof(sys_reg_tbl_t));
    ret = XosMM_XMLreadSrvCfg((sys_reg_tbl_t*)modInfo,(XS8 *)filename);
    if(XSUCC != ret)
    {
        MMInfo("XosMM_XMLreadSrvCfg fail!");
        return XERROR;
    }
    
    //XOS_SemPut(&(gMMSystemInfo.sem));
    return XSUCC;
}

/************************************************************************
������:    XosModuleStartCfg
���ܣ�PMģ������ҵ��ģ��ӿ�
������filename--ҵ��ģ���Ӧxml�ļ�ָ��
�������
���أ�XSUCC--�ɹ�
                  XERROR--ʧ��
˵����
************************************************************************/
XS32 XosModuleStartCfg(XCONST XU8 *modInfo)
{
    XS32 ret;
    HANDLE hRootDir = (HANDLE)NULL;
    HANDLE hModuleDir = (HANDLE)NULL;
    XS8 dirName[MAX_KEYNAME_LENGTH];
    XU32 tempU32Value=0;
    XOS_MM_MODULE_T* pModule=NULL;
    XU32 moduleCount=0;
    XU32 taskCount=0;
    XU32 stackTotalSize = 0;
    XU32 objTotal=0;
    XU32 result = XSUCC;
    XS8 objFileName[MAX_KEYVALUE_LENGTH+12] = "";
    XS8 objFilePath[MAX_KEYVALUE_LENGTH+12] = "";
    XS8 nameBuffer[MAX_KEYVALUE_LENGTH+MAX_KEYVALUE_LENGTH+10] = "";

    if(NULL == modInfo)
    {
        MMInfo("XosModuleStartCfg : para modInfo = NULL!");
        return XERROR;
    }
    
    memset(dirName, 0, MAX_KEYNAME_LENGTH);

    /*��ȡ�ź���*/
    //XOS_SemGet(&(gMMSystemInfo.sem));

    memset(gSysRegTable,0,sizeof(sys_reg_tbl_t));
    XOS_MemCpy(gSysRegTable, modInfo, sizeof(sys_reg_tbl_t));

    hRootDir = XosSysRegGetRootDir();

    XosSysRegEnumDir(hRootDir, (HANDLE)NULL, dirName, &hModuleDir);
    for(moduleCount = gMMSystemInfo.moduleCount; (moduleCount < MAX_MODULE_NUMBER) && (hModuleDir!=(HANDLE)NULL);)
    {
    /*decide whether system registry table directory is MODULE directory,
        if yes, create module*/
        if(0 == strncmp(dirName, "Module",6))
        {
            pModule = &(gMMModuleList[moduleCount]);
            moduleCount++;

            pModule->modHDir = hModuleDir;
            /* ģ�������� */
            if(XSUCC==XosSysRegQueryU32KeyValue(hModuleDir, KEYNAME_TASKCOUNT,&tempU32Value))
            {
                taskCount+=tempU32Value;
            }
            /* ��ջ��С */
            if(XSUCC==XosSysRegQueryU32KeyValue(hModuleDir, KEYNAME_STACKSIZE,&tempU32Value))
            {
                stackTotalSize += tempU32Value;
            }
            strcpy(nameBuffer,"");
            /* Ŀ���ļ�·�� */
            result = XosSysRegQueryStrKeyValue(hModuleDir, OBJFILEPATHKEY, objFilePath);
            if(result==XSUCC)
            {
                strcat(nameBuffer,objFilePath);
            }
            /* Ŀ���ļ����� */
            result = XosSysRegQueryStrKeyValue(hModuleDir, OBJFILENAMEKEY, objFileName);
            if(result==XSUCC)
            {
                strcat(nameBuffer,objFileName);
            }
            result = (XU32)strlen(nameBuffer);
            if(result>=XOS_MM_FILE_NAME_LEN)
            {
                MMErr("OBJName Too Long: Max is %d, current is %d, %s",
                    XOS_MM_FILE_NAME_LEN,result,nameBuffer);
                return XERROR;
            }
            if(XSUCC!=XosMMCalObjInfo(nameBuffer, &tempU32Value,pModule))
            {
                return XERROR;
            }

#ifdef XOS_VXWORKS
#if 0
            printf("Module BssSize:%-10d,TxtSize:%-10d,DataSize:%-10d\n",
                pModule->segSizeBss,pModule->segSizeText,
                pModule->segSizeData);
#endif
#endif

            objTotal += tempU32Value;
        }
        else/* if(0 != strncmp(dirName, "Global",6))*/
        {
            MMErr("Find an invalid Module Tag:%s",dirName);
            return XERROR;
        }
        /*��һ��ģ��*/
        XosSysRegEnumDir(hRootDir,hModuleDir, dirName, &hModuleDir);
        continue;
    }
    //printf("Done\n");

    gMMSystemInfo.taskCount   += taskCount;
    gMMSystemInfo.stackTotal  += stackTotalSize;
    gMMSystemInfo.objTotal    += objTotal;
    
    if(gMMSystemInfo.moduleCount == moduleCount)
    {
        MMErr("NO Srv module Need to Start.\n");
        return XERROR;
    }

    gMMWsSsiInitPara.moduleRegionSize = gMMSystemInfo.stackTotal+gMMSystemInfo.objTotal;

    /*  �������а�ģ�� */
    ret = XosMMCreateSrvModules(gMMSystemInfo.moduleCount);
    if(ret != XSUCC)
    {
        MMInfo("begin modules failed!");
    }

    //XOS_SemPut(&(gMMSystemInfo.sem));
    gMMSystemInfo.moduleCount = moduleCount;

    MMInfo("Total Module Count: %d",gMMSystemInfo.moduleCount);
    MMInfo("Total Task   Count: %d",gMMSystemInfo.taskCount);
    MMInfo("Total Stack  Size : %08x(%d)",gMMSystemInfo.stackTotal,gMMSystemInfo.stackTotal);
    MMInfo("Total Obj   Size  : %08x(%d)",gMMSystemInfo.objTotal,gMMSystemInfo.objTotal);
    printf("\r\n");
    MMInfo("Service Module Manager Finished!");

    return XSUCC;
}
#endif

/***********************************************************************/
#ifdef __cplusplus
    }
#endif /* __cplusplus */

#endif /* XOS_MDLMGT */
