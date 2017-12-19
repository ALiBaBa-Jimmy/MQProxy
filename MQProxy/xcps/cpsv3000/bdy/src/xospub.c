
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "xospub.h"
#include "xosencap.h"
#include <limits.h>



#ifdef LINUX
#include <sys/times.h>
const sys_value2string g_elfCpuType[] = {
    {EM_NONE         ,"EM_NONE         An unknown machine."},
    {EM_M32          ,"EM_M32          AT&T WE 32100."},
    {EM_SPARC        ,"EM_SPARC        Sun Microsystems SPARC."},
    {EM_386          ,"EM_386          Intel 80386."},
    {EM_68K          ,"EM_68K          Motorola 68000."},
    {EM_88K          ,"EM_88K          Motorola 88000."},
    {EM_860          ,"EM_860          Intel 80860."},
    {EM_MIPS         ,"EM_MIPS         MIPS RS3000 (big-endian only)."},
    {EM_PARISC       ,"EM_PARISC       HP/PA."},
    {EM_SPARC32PLUS  ,"EM_SPARC32PLUS  SPARC with enhanced instruction set."},
    {EM_PPC          ,"EM_PPC          PowerPC."},
    {EM_PPC64        ,"EM_PPC64        PowerPC 64-bit."},
    {EM_S390         ,"EM_S390         IBM S/390"},
    {EM_ARM          ,"EM_ARM          Advanced RISC Machines"},
    {EM_SH           ,"EM_SH           Renesas SuperH"},
    {EM_SPARCV9      ,"EM_SPARCV9      SPARC v9 64-bit."},
    {EM_IA_64        ,"EM_IA_64        Intel Itanium"},
    {EM_X86_64       ,"EM_X86_64       AMD x86-64"},
    {EM_VAX          ,"EM_VAX          DEC Vax."}
};

const sys_value2string g_elfFileType[] = {
    {ET_NONE   ,"ET_NONE  An unknown type."},
    {ET_REL    ,"ET_REL   A relocatable file."},
    {ET_EXEC   ,"ET_EXEC  An executable file."},
    {ET_DYN    ,"ET_DYN   A shared object."},
    {ET_CORE   ,"ET_CORE  A core file."}
};

const sys_value2string g_elfSectionType[] = {
    {(s32 )SHT_NULL       ,"SHT_NULL"     },
    {(s32 )SHT_PROGBITS   ,"SHT_PROGBITS" },
    {(s32 )SHT_SYMTAB     ,"SHT_SYMTAB"   },
    {(s32 )SHT_STRTAB     ,"SHT_STRTAB"   },
    {(s32 )SHT_RELA       ,"SHT_RELA"     },
    {(s32 )SHT_HASH       ,"SHT_HASH"     },
    {(s32 )SHT_DYNAMIC    ,"SHT_DYNAMIC"  },
    {(s32 )SHT_NOTE       ,"SHT_NOTE"     },
    {(s32 )SHT_NOBITS     ,"SHT_NOBITS"   },
    {(s32 )SHT_REL        ,"SHT_REL"      },
    {(s32 )SHT_SHLIB      ,"SHT_SHLIB"    },
    {(s32 )SHT_DYNSYM     ,"SHT_DYNSYM"   },
    {(s32 )SHT_LOPROC     ,"SHT_LOPROC"   },
    {(s32 )SHT_HIPROC     ,"SHT_HIPROC"   },
    {(s32 )SHT_LOUSER     ,"SHT_LOUSER"   },
    {(s32 )SHT_HIUSER     ,"SHT_HIUSER"   },
    {0, 0}
};

const sys_value2string g_elfPayloadType[] = {
    {(s32 )PT_NULL          ,"PT_NULL"          },
    {(s32 )PT_LOAD          ,"PT_LOAD"          },
    {(s32 )PT_DYNAMIC       ,"PT_DYNAMIC"       },
    {(s32 )PT_INTERP        ,"PT_INTERP"        },
    {(s32 )PT_NOTE          ,"PT_NOTE"          },        
    {(s32 )PT_SHLIB         ,"PT_SHLIB"         },
    {(s32 )PT_PHDR          ,"PT_PHDR"          },        
    {(s32 )PT_TLS           ,"PT_TLS"           },
    {(s32 )PT_LOOS          ,"PT_LOOS"          },        
    {(s32 )PT_HIOS          ,"PT_HIOS"          },
    {(s32 )PT_LOPROC        ,"PT_LOPROC"        },
    {(s32 )PT_HIPROC        ,"PT_HIPROC"        },
    {(s32 )PT_GNU_EH_FRAME  ,"PT_GNU_EH_FRAME"  },
    {(s32 )PT_GNU_STACK     ,"PT_GNU_STACK"     },
    {0, 0}
};

const sys_value2string g_elfSymType[] = {
    {(s32 )STT_NOTYPE       ,"STT_NOTYPE"       },
    {(s32 )STT_OBJECT       ,"STT_OBJECT"       },
    {(s32 )STT_FUNC         ,"STT_FUNC"         },
    {(s32 )STT_SECTION      ,"STT_SECTION"      },
    {(s32 )STT_FILE         ,"STT_FILE"         },        
    {(s32 )STT_LOPROC       ,"STT_LOPROC"       },
    {(s32 )STT_HIPROC       ,"STT_HIPROC"       },  
    {0, 0}
};


const sys_value2string g_elfSymBind[] = {
    {(s32 )STB_LOCAL        ,"STB_LOCAL"        },
    {(s32 )STB_GLOBAL       ,"STB_GLOBAL"       },
    {(s32 )STB_WEAK         ,"STB_WEAK"         },
    {(s32 )STB_LOPROC       ,"STB_LOPROC"       },
    {(s32 )STB_HIPROC       ,"STB_HIPROC"       },  
    {0, 0}
};

#define max_symble_len      255

typedef struct
{
    s8 name[max_symble_len];
    Elf32_Sym  elfSymTable;    
}sym_table_st;

typedef struct 
{
    s8 *_pElfSymName;
    u32 _elfSymNameLen;
    s8 *_pElfDynSymName;
    u32 _elfDynSymNameLen;
    s8 *_pElfSectionName;
    u32 _elfSectionNameLen;

    Elf32_Ehdr *_pElfEHead;
    Elf32_Phdr *_pElfPHead;
    Elf32_Shdr *_pElfSHead;
    u32 symCn;
    sym_table_st *_pElfSymTable;
//    Elf32_Sym  *_pElfSymTable;
    u32 dynSymCn;
    Elf32_Sym  *_pElfDynSymTable;
}elf_info_st;
elf_info_st g_elfInfo;

#endif

s32 sys_createThread(void *pFunc, void *para, u8 *threadName)
{
#ifdef WIN32
    t_XOSTASKID tid;
    tid = (t_XOSTASKID)_beginthread(pFunc, 0, para);
#endif

#ifdef LINUX
    pthread_t tid;
    pthread_create(&tid, 0, pFunc, para);
#endif

#ifdef VXWORKS
    u32 tid;
    tid = taskSpawn(threadName, 65, VX_FP_TASK, 500000, (FUNCPTR)pFunc,
                        (int)para, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif

    return (s32)tid;
}

//--------------------------------------------------------------------------------
//      获得当前时间
//  输入:
//      pTime:          时间
//  返回:
//      0:              成功
//      -1:             失败
//--------------------------------------------------------------------------------
s32 sys_getCurTime(sys_time_st *pTime)
{
#ifdef WIN32
    SYSTEMTIME time;
#endif

#if defined VXWORKS || defined LINUX

    time_t now;
    struct tm *timenow;
#endif

    if (0x00 == pTime)
        return -1;
    
#ifdef WIN32

    GetLocalTime(&time);

    pTime->year  = time.wYear;
    pTime->month = (u8)time.wMonth;
    pTime->day   = (u8)time.wDay;
    pTime->hour  = (u8)time.wHour;
    pTime->minute= (u8)time.wMinute;
    pTime->second= (u8)time.wSecond;
    pTime->missec= time.wMilliseconds;

#endif

#if defined VXWORKS || defined LINUX

    time(&now);
    timenow = (struct tm * )localtime(&now);
//    gettimeofday(&nowtimeval,0);

    pTime->year  = ((u16)(timenow->tm_year+1900));
    pTime->month = (u8 )timenow->tm_mon+1;
    pTime->day   = (u8 )timenow->tm_mday;
    pTime->hour  = (u8 )timenow->tm_hour;
    pTime->minute= (u8 )timenow->tm_min;
    pTime->second= (u8 )timenow->tm_sec;  
//    pTime->missec= ((u16 )(nowtimeval.tv_usec/1000));

#endif

    return 0x00;
}


s32 sys_logMsg(s8* file, s8 *buf, s32 len)
{
    FILE *f;
    f = fopen(file, "ab+");
    if(NULL == f)
        return -1;

    fwrite(buf, len, 1, f);

    fclose(f);
    return 0x00;
}

s32 logTime(s8* file)
{
    s8 _buf[1024] = {0};
    sys_time_st _time;
    
    sys_getCurTime(&_time);
    sprintf(_buf, "\r\nTime: %4d-%02d-%02d-%02d:%02d:%02d", 
        _time.year, _time.month, _time.day, _time.hour, _time.minute, _time.second);
    sys_logMsg(file, _buf, (XS32)strlen(_buf));
    
    return _success_;
}

/************************************************************************
函数名: XOS_MutexCreate
功能：互斥量的初始化
输入：mutex-互斥量ID
    type 互斥量类型
    (OS_LOCK_MUTEX和OS_LOCK_CRITSEC)
输出：N/A
返回：XSUCC-    成功
      XERROR-   失败
说明：
************************************************************************/
s32 sys_mutexInit(_mutexId *mutex )
{
    if(!mutex)
    {
        return _failed_;
    }
    
#ifdef VXWORKS
    *mutex = semMCreate(SEM_Q_PRIORITY);
#endif

#ifdef WIN32
    InitializeCriticalSection(mutex);
#endif

#ifdef LINUX    
    *mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
#endif

    return _success_;
}



/*---------------------------------------------------------------------
函数名: XOS_MutexLock
功能：获得互斥量
输入：mutex     互斥量ID
输出：N/A
返回：XSUCC     成功
      XERROR    失败
说明：必须是在同一进程的不同线程间使用;
      若在同一线程内可以多次lock（但最好与XOS_MutexUnlock成对出现）
------------------------------------------------------------------------*/
s32 sys_mutexLock(_mutexId *mutex)
{
    s32 ret = 0x00;
#ifdef VXWORKS
    s32 back;
#endif    
    if(!mutex)
    {
        return _failed_;
    }

#ifdef LINUX  
    ret = pthread_mutex_lock(mutex);
#endif

#ifdef WIN32
    EnterCriticalSection(mutex);
#endif

#ifdef VXWORKS
    back = intContext();
    ret = semTake(*mutex, back ? NO_WAIT:WAIT_FOREVER);
#endif

    if(ret)
    {
        printf("\r\n err: lock failure!!!");
        return _failed_;
    }

    return _success_;
}

/*---------------------------------------------------------------------
函数名: XOS_MutexUnlock
功能：互斥量解锁
输入：mutex     互斥量ID
输出：N/A
返回：XSUCC     成功
    XERROR      失败
说明：
------------------------------------------------------------------------*/
s32 sys_mutexUnlock(_mutexId *mutex)
{
    s32 ret = 0x00;
    
    if(!mutex)
    {
        return _failed_;
    }

#ifdef LINUX  
    ret = pthread_mutex_unlock(mutex);
#endif

#ifdef WIN32
    LeaveCriticalSection(mutex);
#endif

#ifdef VXWORKS
    semGive(*mutex);
#endif

    if(ret)
    {
        printf("\r\n err: unlock failure!!!");
        return _failed_;
    }

    return _success_;
}


s32 sys_listInit(sys_list_st *ent)
{
    ent->next = ent->prev = ent;
    return _success_;
}


/*---------------------------------------------------------------------
sys_listAdd     - 把一节点添加到双向链表中

参数说明：
    old         - 输入，双向链表头指针
    add         - 输入，双向链新节点

返回值: 0, 函数操作失败返回1
------------------------------------------------------------------------*/
s32 sys_listAdd(sys_list_st *old, sys_list_st *add)
{
    add->next = old->next;
    add->prev = old;
    old->next = add;
    (add->next)->prev = add;
    return _success_;
}

/**
 * sys_listEmpty - tests whether a list is empty
 * @head: the list to test.
 */
s32 sys_listEmpty(sys_list_st *head)
{
    return head->next == head;
}

void __list_splice(sys_list_st *list, sys_list_st *head)
{
    sys_list_st *first = list->next;
    sys_list_st *last = list->prev;
    sys_list_st *at = head->next;

    first->prev = head;
    head->next = first;

    last->next = at;
    at->prev = last;
}


/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
void sys_listSpliceInit(sys_list_st *slist, sys_list_st *head)
{
    if (!sys_listEmpty(slist)) {
        __list_splice(slist, head);
        sys_listInit(slist);
    }
}

sys_list_st* sys_listNext(sys_list_st *ent)
{
    sys_list_st *rt = 0x00;
    
    if(!ent)
    {
        return 0x00;
    }

    if(ent == ent->prev)
    {
        return 0x00;
    }

    rt = ent->prev;
    sys_listDel(rt);
    sys_listInit(rt);
    
    
    return rt;
}


/*---------------------------------------------------------------------
SYS_ListDel    - 从双向链表中删除一个节点

参数说明：
    psDelNode      - 输入，需要删除的节点的指针

返回值: 0, 函数操作失败返回1
------------------------------------------------------------------------*/
s32 sys_listDel(sys_list_st *ent)
{
    (ent->prev)->next = ent->next;
    (ent->next)->prev = ent->prev;
    ent->next = ent->prev = ent;
    
    return _success_;
}

/*---------------------------------------------------------------------
SYS_ListAdd    - 把一节点添加到双向链表中

参数说明：
    pstTblHeader      - 输入，双向链表头指针
    pstNewNode        - 输入，双向链新节点

返回值: 0, 函数操作失败返回1
------------------------------------------------------------------------*/
s32 sys_slistAdd(sys_list_st *ent, sys_list_st *add, _mutexId *m)
{    
    sys_mutexLock(m);

    add->next = ent->next;
    add->prev = ent;
    ent->next = add;
    (add->next)->prev = add;
    
    sys_mutexUnlock(m);
    return _success_;
    
}

/*---------------------------------------------------------------------
SYS_ListDel    - 从双向链表中删除一个节点

参数说明：
    psDelNode      - 输入，需要删除的节点的指针

返回值: 0, 函数操作失败返回1
------------------------------------------------------------------------*/
s32 sys_slistDel(sys_list_st *ent, _mutexId *m)
{
    sys_mutexLock(m);
    
    (ent->prev)->next = ent->next;
    (ent->next)->prev = ent->prev;
    ent->next = ent->prev = ent;
    
    sys_mutexUnlock(m);
    return _success_;
}


/*---------------------------------------------------------------------
SYS_ListDel    - 从双向链表中删除一个节点

参数说明：
    psDelNode      - 输入，需要删除的节点的指针

返回值: XSUCC, 函数操作失败返回FAIL
------------------------------------------------------------------------*/
sys_list_st* sys_slistGetNext(sys_list_st *ent, _mutexId *m)
{
    sys_list_st *rt;
    
    if(!ent)
    {
        return 0x00;
    }

    sys_mutexLock(m);
    if(ent == ent->prev)
    {
        sys_mutexUnlock(m);
        return 0x00;
    }

    rt = ent->prev;
    sys_listDel(rt);
    sys_listInit(rt);
    
    sys_mutexUnlock(m);
    
    return rt;
}

/*
    get the data size of netbuf
*/
s32 sys_getNetBufSize(t_SOCKFD fd, s32 *size)
{

#ifdef WIN32
    ioctlsocket(fd, FIONREAD, (u32 *)size);
#endif

#ifdef VXWORKS
    ioctl(fd, FIONREAD, (s32)size);
#endif

#ifdef LINUX
    ioctl(fd, FIONREAD, size);
#endif

    return _success_;    
}

/*
    locate cursor of console
*/
s32 sys_consoleGoxy(int x, int y)
{
#ifdef WIN32
    HANDLE   hCon;//定义一个句柄
    COORD   setps; //定义结构体变量

    hCon = GetStdHandle(STD_OUTPUT_HANDLE);   //获得输出设备的句柄
    setps.X = x;
    setps.Y = y;
    SetConsoleCursorPosition(hCon,setps);  //定位
    return _success_;
#endif

#ifdef LINUX
    /* man terminfo 

 下 面 是 Linux主 控 台 的 Terminfo 原 始 描 述 内 容 ： 
    console|  Standard Linux Console,  
 cr=^M, cud1=^J, ind=^J, bel=^G, il1=\E[L, am, cub1=^H, ed=\E[J,  
 el=\E[K, clear=\E[H\E[J, km, eo, mir, msgr, xon,  
 colors#8, pairs#64, lines#25, cols#80,  
 hpa=\E%i%p1%dG, vpa=\E%i%p1%dd, ri=\EM, hts=\EH, tbc=\E[3g,  
 smir=\E[4h, rmir=\E[4l, civis=\E[?25l, cnorm=\E[?25h,  
 sc=\E7, rc=\E8,  
 cup=\E[%i%p1%d;%p2%dH, op=\E[37m\E[40m,  
 dch=\E[%p1%dP, dl=\E[%p1%dM, home=\E[H, it#4, ich=\E[%p1%d@,  
 bold=\E[1m, rev=\E[7m, blink=\E[5m,  
 setf=\E[%p1%{30}%+%dm, setb=\E[%p1%{40}%+%dm,  
 kcuu1=\E[A, kcud1=\E[B, kcub1=\E[D, kcuf1=\E[C, khome=\E[H,  
 cuf1=\E[C, ht=^I, cuu1=\E[A,  
 smacs=\E(B\E)U^N,rmacs=\E(B\E)0^O,  
 rmul=\E[24m, smul=\E[4m, rmso=\E[0m, smso=\E[7m,  
 kf1=\E[[A, kf2=\E[[B, kf3=\E[[C, kf4=\E[[D, kf5=\E[[E,  
 kf6=\E[17~, kf7=\E[18~, kf8=\E[19~, kf9=\E[20~, kf10=\E[21~,  
 kdch1=\E[3~, kend=\E[4~, khome=\E[1~, knp=\E[6~, kpp=\E[5~,  
*/
    printf("\E[H\E[J\33[%d;%dH",y,x);
    return _success_;
#endif

    return _success_;
} 

extern unsigned long tickGet(void);
u32 sys_getTick()
{
#ifdef WIN32     
    return GetTickCount();
#endif

#ifdef LINUX
    return  times(NULL);
#endif

#ifdef VXWORKS
    return tickGet();
#endif

    return 0;
}

/*
    ms
*/
u32 sys_sleep(u32 tm)
{
#ifdef WIN32
    Sleep(tm);
#endif

#ifdef LINUX
    usleep(tm<<10);//这里认为系统时间片预设为10ms
#endif

#ifdef VXWORKS
    taskDelay(tm);
#endif

    return _success_;
}

/*--------------------------------------------------------------------------------
      检查数据序号是否在环形缓冲的开始序号和结束序号之间
  输入:
      BeginSeq:       开始序号
      EndSeq:         结束序号
      MidSeq:         需要判断的中间序号
      MaxSeq:         最大序号
  返回:
      SUCC:           BeginSeq<=MidSeq<=EndSeq
      FAIL:           不属于上述范围
--------------------------------------------------------------------------------*/
s32 sys_IsMiddleSeq(u32 beginSeq, u32 endSeq, u32 midSeq)
{
    /* 正常情况 */
    if (endSeq >= beginSeq)
    {
        if (midSeq >= beginSeq && midSeq <= endSeq)//在有效范围内
            return _success_;
        else
            return _failed_;
    }
    else//发生环回
    {
        if (midSeq < beginSeq && midSeq > endSeq)//如果在有效范围外
            return _failed_;
        else
            return _success_;
    }
}

s8* sys_getStringByValue(sys_value2string *st, s32 id)
{
    sys_value2string *p = st;
    
    if(!st)
        return 0x00;

    while(1)
    {
        if(!p->pstr )
            break;
        
        if(id == p->value)
            return (s8* )p->pstr;

        p++;
    }

    return 0x00;
}
  


s32 sys_readSymTable(s8 *name)
{    
#if defined LINUX    
    u32 i   = 0x00;
    u32 type    = 0x00;
    u32 cir     = 0x00;

    FILE *f;    
    //Elf32_Ehdr *pElfEHead;
    //Elf32_Phdr *pElfPHead;
    Elf32_Shdr *pElfSHead;
    //Elf32_Sym  *pElfSym;
    sym_table_st *pstSymTable;
#endif


#if defined LINUX  
    f = fopen(name, "rb");
    if(NULL == f)
        return -1; 

    g_elfInfo._pElfEHead = (Elf32_Ehdr* )malloc(sizeof(Elf32_Ehdr));
    fread((s8* )(g_elfInfo._pElfEHead), sizeof(Elf32_Ehdr), 1, f);

    /* 
        progess head segment 
    */
    if(g_elfInfo._pElfEHead->e_phnum)
    {
        g_elfInfo._pElfPHead = (Elf32_Phdr* )malloc(g_elfInfo._pElfEHead->e_phnum*sizeof(Elf32_Phdr));
        fread((s8* )g_elfInfo._pElfPHead, g_elfInfo._pElfEHead->e_phnum*sizeof(Elf32_Phdr), 1, f);
    }

    /* 
        section head segment 
    */
    fseek(f, g_elfInfo._pElfEHead->e_shoff, SEEK_SET);
    g_elfInfo._pElfSHead = (Elf32_Shdr* )malloc(g_elfInfo._pElfEHead->e_shnum*sizeof(Elf32_Shdr));
    fread((s8* )g_elfInfo._pElfSHead, g_elfInfo._pElfEHead->e_shnum*sizeof(Elf32_Shdr), 1, f);

    /*
        section head string
    */    
    pElfSHead = &g_elfInfo._pElfSHead[g_elfInfo._pElfEHead->e_shstrndx];
    fseek(f, pElfSHead->sh_offset, SEEK_SET);
    g_elfInfo._elfSectionNameLen = pElfSHead->sh_size;
    g_elfInfo._pElfSectionName = (s8* )malloc(pElfSHead->sh_size);
    fread((s8* )g_elfInfo._pElfSectionName, pElfSHead->sh_size, 1, f);    
    
    for(cir=0x00; cir<g_elfInfo._pElfEHead->e_shnum; cir++)
    {
        pElfSHead = &g_elfInfo._pElfSHead[cir];
        if(SHT_SYMTAB == pElfSHead->sh_type)
        {
            pElfSHead = &g_elfInfo._pElfSHead[pElfSHead->sh_link];
            fseek(f, pElfSHead->sh_offset, SEEK_SET);
            g_elfInfo._elfSymNameLen = pElfSHead->sh_size;
            g_elfInfo._pElfSymName = (s8* )malloc(pElfSHead->sh_size);
            fread((s8* )g_elfInfo._pElfSymName, pElfSHead->sh_size, 1, f);
            
            pElfSHead = &g_elfInfo._pElfSHead[cir];
            fseek(f, pElfSHead->sh_offset, SEEK_SET);
            g_elfInfo.symCn = pElfSHead->sh_size/sizeof(Elf32_Sym);
            g_elfInfo._pElfSymTable = (sym_table_st* )malloc(sizeof(sym_table_st)*g_elfInfo.symCn);
            for(i=0x00; i<g_elfInfo.symCn; i++)
            {
                fread((s8* )&(g_elfInfo._pElfSymTable[i].elfSymTable), sizeof(Elf32_Sym), 1, f);

                pstSymTable = &g_elfInfo._pElfSymTable[i];
                type = ELF32_ST_TYPE(pstSymTable->elfSymTable.st_info);
                pstSymTable->name[0x00] = 0x00;
                if(0x00 == pstSymTable->elfSymTable.st_name)
                    continue;

                if(STT_FUNC != type && STT_OBJECT != type)
                    continue;
                
                if(max_symble_len <= strlen(&g_elfInfo._pElfSymName[pstSymTable->elfSymTable.st_name]))
                    continue;

                strcpy(pstSymTable->name, &g_elfInfo._pElfSymName[pstSymTable->elfSymTable.st_name]);                 
            }
        }
        
        if(SHT_DYNSYM == pElfSHead->sh_type)
        {
            fseek(f, pElfSHead->sh_offset, SEEK_SET);
            g_elfInfo.dynSymCn = pElfSHead->sh_size/sizeof(Elf32_Sym);
            g_elfInfo._pElfDynSymTable = (Elf32_Sym* )malloc(pElfSHead->sh_size);
            fread((s8* )g_elfInfo._pElfDynSymTable, pElfSHead->sh_size, 1, f);
            
            pElfSHead = &g_elfInfo._pElfSHead[pElfSHead->sh_link];
            fseek(f, pElfSHead->sh_offset, SEEK_SET);
            g_elfInfo._elfDynSymNameLen = pElfSHead->sh_size;
            g_elfInfo._pElfDynSymName = (s8* )malloc(pElfSHead->sh_size);
            fread((s8* )g_elfInfo._pElfDynSymName, pElfSHead->sh_size, 1, f);            
        }
    }

    /*
        save the symble name list
    g_elfInfo.symString = (s8* )malloc(max_symble_len*g_elfInfo.symCn);
    for(cir=0x00; cir<g_elfInfo.symCn; cir++)
        g_elfInfo.symString[cir][0x00] = 0x00;
   
    */
    /* for(cir=0x00; cir<g_elfInfo.symCn; cir++)
    {
        pElfSym = &g_elfInfo._pElfSymTable[cir];
        type = ELF32_ST_TYPE(pElfSym->st_info);
        g_elfInfo.symString[cir][0] = 0x00;
        if(0x00 == pElfSym->st_name)
            continue;

        if(STT_FUNC != type && STT_OBJECT != type)
            continue;
        
        if(max_symble_len <= strlen(&g_elfInfo._pElfSymName[pElfSym->st_name]))
            continue;

        strcpy(g_elfInfo.symString[cir], &g_elfInfo._pElfSymName[pElfSym->st_name]);      
    }
*/

    fclose(f);
#endif

    return _success_;
}

#if defined LINUX
s32 sys_printSymTable()
{
    u32 cir = 0x00;
    u32 type= 0x00;
    Elf32_Phdr *pElfPHead;    
    Elf32_Shdr *pElfSHead;
    Elf32_Sym  *pElfSym;
#ifdef XOS_ARCH_64
    printf("\r\n elf file head: sizeof(Elf32_Ehdr): 0x%lx", sizeof(Elf32_Ehdr));
#else
    printf("\r\n elf file head: sizeof(Elf32_Ehdr): 0x%x", sizeof(Elf32_Ehdr));
#endif
    printf("\r\n e_ident:    %s",       g_elfInfo._pElfEHead->e_ident);
    printf("\r\n e_type:     %s",       sys_getStringByValue((sys_value2string* )g_elfFileType, g_elfInfo._pElfEHead->e_type));
    printf("\r\n e_machine:  %s",       sys_getStringByValue((sys_value2string* )g_elfCpuType, g_elfInfo._pElfEHead->e_machine));
    printf("\r\n e_version:  0x%04x",   g_elfInfo._pElfEHead->e_version);
    printf("\r\n e_entry:    0x%04x",   g_elfInfo._pElfEHead->e_entry);
    printf("\r\n e_phoff:    0x%04x",   g_elfInfo._pElfEHead->e_phoff);
    printf("\r\n e_shoff:    0x%04x",   g_elfInfo._pElfEHead->e_shoff);
    printf("\r\n e_flags:    0x%04x",   g_elfInfo._pElfEHead->e_flags);
    printf("\r\n e_ehsize:   0x%04x",   g_elfInfo._pElfEHead->e_ehsize);
    printf("\r\n e_phentsize:0x%04x",   g_elfInfo._pElfEHead->e_phentsize);
    printf("\r\n e_phnum:    0x%04x",   g_elfInfo._pElfEHead->e_phnum);
    printf("\r\n e_shentsize:0x%04x",   g_elfInfo._pElfEHead->e_shentsize);
    printf("\r\n e_shnum:    0x%04x",   g_elfInfo._pElfEHead->e_shnum);
    printf("\r\n e_shstrndx: 0x%04x",   g_elfInfo._pElfEHead->e_shstrndx);
    printf("\r\n");
#ifdef XOS_ARCH_64
    printf("\r\n progress list: sizeof(Elf32_Phdr): 0x%lx", sizeof(Elf32_Phdr));
#else
    printf("\r\n progress list: sizeof(Elf32_Phdr): 0x%x", sizeof(Elf32_Phdr));
#endif
    for(cir=0x00; cir<g_elfInfo._pElfEHead->e_phnum; cir++)
    {
        pElfPHead = &g_elfInfo._pElfPHead[cir];
        printf("\r\n p_type: %-16s",     sys_getStringByValue((sys_value2string* )g_elfPayloadType, pElfPHead->p_type));
        printf("\r\n p_offset: 0x%04x",   pElfPHead->p_offset);
        printf(" p_vaddr:  0x%04x",   pElfPHead->p_vaddr);
        printf(" p_paddr:  0x%04x",   pElfPHead->p_paddr);
        printf("\r\n p_filesz: 0x%04x",   pElfPHead->p_filesz);
        printf(" p_memsz:  0x%04x",   pElfPHead->p_memsz);
        printf(" p_flags:  0x%04x   ",   pElfPHead->p_flags);
        printf(" p_align:  0x%04x",   pElfPHead->p_align);
        printf("\r\n");
    }   
    
#ifdef XOS_ARCH_64
    printf("\r\n section list: sizeof(Elf32_Shdr): 0x%lx", sizeof(Elf32_Shdr));
#else
    printf("\r\n section list: sizeof(Elf32_Shdr): 0x%x", sizeof(Elf32_Shdr));
#endif
    for(cir=0x00; cir<g_elfInfo._pElfEHead->e_shnum; cir++)
    {
        pElfSHead = &g_elfInfo._pElfSHead[cir];
        printf("\r\n 0x-%02x %s",  cir, &g_elfInfo._pElfSectionName[pElfSHead->sh_name]);
        printf("\r\n sh_type: %-16s",   sys_getStringByValue((sys_value2string* )g_elfSectionType, pElfSHead->sh_type));
        printf("\r\n sh_flags: 0x%04x", pElfSHead->sh_flags);
        printf(" sh_addr:  0x%08x",     pElfSHead->sh_addr);
        printf(" sh_offset:    0x%04x",   pElfSHead->sh_offset);
        printf(" sh_size:    0x%04x",  pElfSHead->sh_size);
        printf("\r\n sh_link:  0x%04x",     pElfSHead->sh_link);
        printf(" sh_info:  0x%08x",  pElfSHead->sh_info);
        printf(" sh_addralign: 0x%04x",pElfSHead->sh_addralign);
        printf(" sh_entsize: 0x%04x",  pElfSHead->sh_entsize);
        printf("\r\n");
    }   
    
#ifdef XOS_ARCH_64
    printf("\r\n\r\n symble list: sizeof(Elf32_Sym): 0x%lx", sizeof(Elf32_Sym));
#else
    printf("\r\n\r\n symble list: sizeof(Elf32_Sym): 0x%x", sizeof(Elf32_Sym));
#endif
    printf("\r\n idx  name                      adress      size      area          type     ");
    for(cir=0x00; cir<g_elfInfo.symCn; cir++)
    {
        pElfSym = &g_elfInfo._pElfSymTable[cir].elfSymTable;
        type = ELF32_ST_TYPE(pElfSym->st_info);
        if(0x00 == pElfSym->st_name)
            continue;

        if(STT_FUNC != type && STT_OBJECT != type)
            continue;

        printf("\r\n 0x-%02x %-25s 0x%08x  0x%06x  %-10s  %s",  
            cir, g_elfInfo._pElfSymTable[cir].name, pElfSym->st_value, pElfSym->st_size,
            sys_getStringByValue((sys_value2string* )g_elfSymBind, ELF32_ST_BIND(pElfSym->st_info)), 
            sys_getStringByValue((sys_value2string* )g_elfSymType, type) );
    }

#ifdef XOS_ARCH_64
    printf("\r\n\r\n dyn symble list: sizeof(Elf32_Sym): 0x%lx", sizeof(Elf32_Sym));
#else
    printf("\r\n\r\n dyn symble list: sizeof(Elf32_Sym): 0x%x", sizeof(Elf32_Sym));
#endif
    printf("\r\n idx  name                      adress      size      area          type     ");
    for(cir=0x00; cir<g_elfInfo.dynSymCn; cir++)
    {
        pElfSym = &g_elfInfo._pElfDynSymTable[cir];
        type = ELF32_ST_TYPE(pElfSym->st_info);
        if(0x00 == pElfSym->st_name)
            continue;

        if(STT_FUNC != type && STT_OBJECT != type)
            continue;
        
        printf("\r\n 0x-%02x %-25s 0x%08x  0x%06x  %-10s  %s",  
            cir, &g_elfInfo._pElfDynSymName[pElfSym->st_name], pElfSym->st_value, pElfSym->st_size,
            sys_getStringByValue((sys_value2string* )g_elfSymBind, ELF32_ST_BIND(pElfSym->st_info)), 
            sys_getStringByValue((sys_value2string* )g_elfSymType, type));
    }
    
    
    return _success_; 
}

s32 sys_symIsFun(u32 type)
{
    if(STT_FUNC == type)
        return _success_;

    return _failed_;
}

/*此函数暂时没有使用，如果使用，需要注意pAddress的32位与64位长度*/
s32 sys_findSymbByName(s8 *name, u32 *pAddress, u32 *type)
{   
    u32 len1 = 0x00;
    u32 len2 = 0x00;
    u32 cir = 0x00;
    sym_table_st *pstSymTable;

    len1 = strlen(name);
    for(cir=0x00; cir<g_elfInfo.symCn; cir++)
    {
        pstSymTable = &g_elfInfo._pElfSymTable[cir];
        len2 = strlen(pstSymTable->name);
        if(len1 != len2)
            continue;

        if(0x00 != strcmp(name, pstSymTable->name))
            continue;

        *type =     ELF32_ST_TYPE(pstSymTable->elfSymTable.st_info);
        *pAddress = pstSymTable->elfSymTable.st_value;
        return _success_;
    }

    return _failed_;    
}
#endif

/************************************************************************
函数名: strtopointer
功能:  将字符串转化为内存地址
输入: nptr --字符串
      endptr
      base -- 进制类型
输出:
返回: 地址指针
说明:
************************************************************************/
XPOINT strtopointer(const char *nptr, char **endptr, int base)
{
    const char     *src = nptr;
    XPOINT   acc = 0;
    unsigned char   ch = 0;
    unsigned long   cutoff = 0;
    int             neg = 0, any = 0, cutlim = 0;

    if(src == NULL)
    {
        return 0;
    }
    /*
     * See strtol for comments as to the logic used.
     */
    do 
    {
        ch = *src++;
    } while (isspace(ch));

    if (ch == '-') 
    {
        neg = 1;
        ch = *src++;
    } 
    else if (ch == '+')
    {
        ch = *src++;
    }

    if ((base == 0 || base == 16) && ch == '0' && (*src == 'x' || *src == 'X')) 
    {
        ch = src[1];
        src += 2;
        base = 16;
    }

    if (base == 0)
    {
        base = ch == '0' ? 8 : 10;
    }
    
    cutoff = (unsigned long) ULONG_MAX / (unsigned long) base;
    cutlim = (unsigned long) ULONG_MAX % (unsigned long) base;
    
    for (acc = 0, any = 0;; ch = *src++) 
    {
        if (!isascii(ch))
            break;
        if (isdigit(ch))
            ch -= '0';
        else if (isalpha(ch))
            ch -= isupper(ch) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (ch >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && ch > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += ch;
        }
    }

    if (any < 0) 
    {
        acc = ULONG_MAX;
    } 
    else if (neg)
    {
        /*不支持负地址转换*/
        return 0;
    }
    
    if (endptr != 0)
    {
        *endptr = (char *) (any ? src - 1 : nptr);
    }
    
    return (acc);
}

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


