//#ifdef   XOS_LINUX || XOS_VXWORKS || XOS_WIN32

#ifdef XOS_VXWORKS
#include <timers.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "snprintf_test.h"

XS8 SnprintfTestInit( XVOID* argv1, XVOID* argv2);
XS8 SnprintfTestClose(XVOID *Para1, XVOID *Para2);



void StrErrorCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

t_XOSFIDLIST g_dispatcherSnprintfTestFid ={
    { "FID_SNPRINTF_TEST",  XNULL, FID_SNPRINTF_TEST,},
    { SnprintfTestInit, NULL, SnprintfTestClose,},
    { NULL, NULL,}, eXOSMode, NULL
};


// ----------------------------- MyTest -----------------------------


XS32 SnprintfTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
	t_XOSLOGINLIST stLoginList;
	XS32 ret = XSUCC;
	XS32 promptID;

	XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
	stLoginList.stack = &g_dispatcherSnprintfTestFid;
	XOS_StrNcpy(stLoginList.taskname, "Tsk_SNPRINTF_TEST", MAX_TID_NAME_LEN);
	stLoginList.TID = FID_SNPRINTF_TEST;
	stLoginList.prio = TSK_PRIO_NORMAL;
	stLoginList.quenum = MAX_MSGS_IN_QUE;
	stLoginList.stacksize = 1024*10;
	ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

	promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "snprintfTest", "Snprintf test", "");
    XOS_RegistCommand(promptID, SnprintfTestCmd, "snprintf_test", "test snprintf function", "");
    XOS_RegistCommand(promptID, StrErrorCmd, "strerror", "test snprintf function", "");
	
	return XSUCC;
}

XS8 SnprintfTestInit( XVOID* argv1, XVOID* argv2)
{
    MMInfo("SnprintfTestInit ");

    return XSUCC;
}

XS8 SnprintfTestClose(XVOID *Para1, XVOID *Para2)
{
    MMInfo("SnprintfTestClose ");
	return 0;
}


XS32 mytestfunc (int num, XCHAR *buffer, XS32 Maxlen, const XCHAR *format, ...)
{
    XS32 length;

    va_list ap;
    va_start(ap, format);

    if ( XNULL == buffer || XNULL == format )
    {
        printf("%d--arg null--return\n", num);
        return XNULL;
    }
    /*对maxlen进行0判断，因为下面处理可变参数时，maxlen是要进行-1处理的*/
    if ( 0 >= Maxlen )
    {
        printf("%d--MAXlen <= 0--return\n", num);
        return XNULL;
    }

#ifdef XOS_WIN32
    length =  _vsnprintf(buffer,Maxlen,format, ap);
#endif
#if ( defined(XOS_LINUX) || defined(XOS_SOLARIS) )
    length = vsnprintf(buffer, Maxlen,format, ap);
#endif
#if ( defined(XOS_VXWORKS) || defined(XOS_VTA) )
    length = vx_vsnprintf(buffer,Maxlen,format,ap);
#endif  
    va_end(ap);

    /*va_end( ap );*/
    if(length < 0)
    {
    /* 当输入str的len > maxlen时，win32 返回-1，在此改正为，最后一个字节替换为'\0',返回buf字符串实际长度 */
#ifdef XOS_WIN32
        buffer[Maxlen-1] = '\0';
        length = strlen(buffer);
#endif
        return length;
    }   

    if((XU32)length >= Maxlen)
    {
    /* 这里只会出现len == maxlen的情况，如果len > maxlen，_vsnprintf返回-1，不会走到这里 */
#ifdef XOS_WIN32   
        buffer[Maxlen-1] = '\0';        
#endif
        length = strlen(buffer);
    }

    
	printf("[test%d]:%s||ret:%d\n",num,buffer,length);
    XOS_MemSet(buffer,0,Maxlen);
    return length;
}

void StrErrorCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    unsigned int i;
    char *p;
    char *file;
    FILE *pf;

#ifdef XOS_VXWORKS
    file = "/ata0a/vx_strerror.txt";
#elif defined XOS_WIN32
    file = "F:\win_strerror.txt";
#elif defined XOS_LINUX
    file = "/home/spj/linux_strerror.txt";
#endif


    
    pf = fopen(file,"w+");
    if (NULL == pf)
    {
        printf("fopen fial!\n");
        return;
    }
    
    for (i = 0; i < 520000; i++)
    {
        p = XOS_StrError(i);
        //fprintf(pf,"case %8d: szErrDes = \"%s\"; break;\n", i,p); 
        fprintf(pf,"no:%8d,str:%s\n", i,p); 
    }
#if 0
#ifdef XOS_VXWORKS
    FILE * pf;
    pf = fopen("/ata0a/vx_strerror.txt","w+");
    if (NULL == pf)
    {
        printf("fopen fial!\n");
        return;
    }
    
    for (hi = 0; hi < 150; hi++)
    {
        printf("hi:%d\n",hi);
        for (lo = 0; lo <= 0xFFFF; lo++)
        {
            i = hi << 16 | lo;
            p = XOS_StrError(i,buf,256);
            //printf("strlen:%d, err[%d]:%s\n",strlen(buf),i,p);
            
            if (strncmp(p, "errno =", strlen("errno =")))
                fprintf(pf,"case %8d: szErrDes = \"%s\"; break;\n", i,p);
        }
        sleep(1);
        fflush(pf);
    }

    fclose(pf);
#elif defined XOS_WIN32

    printf("SOCKET_ERROR    :%d\n", SOCKET_ERROR     );
    printf("WSAEINPROGRESS  :%d\n", WSAEINPROGRESS   );
    printf("WSAEISCONN      :%d\n", WSAEISCONN       );
    printf("WSAEWOULDBLOCK  :%d\n", WSAEWOULDBLOCK   );
    printf("INADDR_NONE     :%d\n", INADDR_NONE      );

    for (i = 0; i< 20000; i++)
    {
        p = XOS_StrError(i,buf,256);
        //printf("strlen:%d, err[%d]:%s\n",strlen(buf),i,p);
        if (strncmp(p, "errno =", strlen("errno =")))
            printf("case %2d: szErrDes = \"%s\"\n", i,p);
    }       
#ifdef XOS_WIN32
    system("pause");
#else
    //sleep(1);
#endif 

#else
    
        memset(buf,'1',256);
        /*
        p = XOS_StrError(i,buf,15);
        printf("strlen:%d, err[%d]:%s\n",strlen(p),i,p);
                */
        p = XOS_StrError(i,buf,256);
        //printf("strlen:%d, err[%d]:%s\n",strlen(buf),i,p);
        if (strncmp(p, "errno =", strlen("errno =")))
            printf("case %2d: szErrDes = \"%s\"\n", i,p);
            
        #ifdef XOS_WIN32
        system("pause");
        #else
        //sleep(1);
        #endif 
 #endif
#endif
}

void SnprintfTestCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    char buf1[10] = {"aaaaaaaaa"};
    char testBuf[30] = "01234567890123456789012345678";
    char buf2[10] = {"bbbbbbbbb"};
    int len = sizeof(testBuf)/sizeof(testBuf[0]);

    int inttest = 101;
    char strtest[10] = "shenzhen";
    int Hextest = 0x12345678;

    mytestfunc(1,testBuf, len, "abcefg");

    mytestfunc(2,testBuf, len, "a[%d]{%s}[0x%X]!", inttest, strtest, Hextest);
    
   
    mytestfunc(3,testBuf, len, "%s","0123456789");   /* strlen < count */
    mytestfunc(4,testBuf, len, "%s","012345678901234567890123456789");  /* strlen == count */
	mytestfunc(5,testBuf, len, "%s","0123456789012345678901234567890123456789");  /* strlen > count */

    printf("buf1:%s\n",buf1);
    printf("buf2:%s\n",buf2);
    
    mytestfunc(6,testBuf, len, "abcd",inttest, strtest, Hextest);
    //mytestfunc(7,testBuf, len, "[%d]");
    
    mytestfunc(9,testBuf, 0, "abcd[%s]","0123456789");
    mytestfunc(9,testBuf, 10, "abcd[%s]","0123456789");
    mytestfunc(10,testBuf, len,0);
    mytestfunc(10,testBuf, len,0,"adbadf");
    mytestfunc(8,0, len, "abcd");

#ifdef XOS_LINUX  /* gcov 覆盖 */
    printf("-------------------------------\n");

    mytestfunc(10,testBuf, 30, "%25.2s$", "0123456789");
    
    mytestfunc(11,testBuf, len,"% s","adbadf");
    
    mytestfunc(12,testBuf, len,"%#X",Hextest);

    mytestfunc(13,testBuf, len,"%*s%d",20,"adbadf",inttest);

    mytestfunc(14,testBuf, len,"%-30s%d","adbadf",inttest);

    mytestfunc(15,testBuf, len,"%.*+s",-10,"adbadf");

    mytestfunc(16,testBuf, len,"%hs","adbadf");

    mytestfunc(17,testBuf, len,"0x%lld",Hextest);

    mytestfunc(18,testBuf, len,"%c",41);

    mytestfunc(19,testBuf, len,"%D",inttest);

    mytestfunc(20,testBuf, len,"%i",-100);

    int* pint = &inttest;
    
    mytestfunc(21,testBuf, len,"%n",pint);
    mytestfunc(21,testBuf, len,"%hn",pint);
    mytestfunc(21,testBuf, len,"%ln",pint);

    mytestfunc(22,testBuf, len,"%o",245);
    mytestfunc(22,testBuf, len,"%O",245);

    mytestfunc(23,testBuf, len,"%p",&inttest);

    mytestfunc(24,testBuf, len,"%s",NULL);  // inttest = 0; &inttest

    mytestfunc(25,testBuf, len,"%U",100);

    mytestfunc(26,testBuf, len,"0x%0x",100);
    mytestfunc(27,testBuf, len,"%u",100);
    
    mytestfunc(28,testBuf, len,"%d",1);
    mytestfunc(29,testBuf, len,"%d",21474836470);

    mytestfunc(31,testBuf, len,"%?",100);
    mytestfunc(32,testBuf, len,"%\0",100);
    /*'' # * - + .  0  1-9 h l c D d(fushu) n O o p s(null) U u X x ?(defalt) */
#endif  
}

#ifdef __cplusplus
}
#endif

//#endif /* XOS_VXWORKS */
