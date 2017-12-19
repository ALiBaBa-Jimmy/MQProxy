echo off

echo ####### set variables for makefile system  #######

REM echo ###Tornado_2.2��·����2.2�汾 ===================== 3)
set WIND_BASE=C:\Tornado2.2

set SRC_ROOT=%cd%\..\..
set CPS_ROOT=%SRC_ROOT%
set XCPS_ROOT=%SRC_ROOT%
set V3000_ROOT=%CPS_ROOT%\cpsv3000
set RULES_ROOT=%CPS_ROOT%\cpsrules
SET OBJS_DIR=%V3000_ROOT%\obj\vxworks

mkdir %OBJS_DIR%

set OBJTYPE=out

set ENABLE_PR_XOS_OUT=xosvx


REM echo ###�����ǻ��������Ķ��壨��Ӧ��Ӧ�ı���꣩����Ҫ�Ļ���ǰ���"rem"ȥ�� ========== 5)


REM ##### gcc �İ汾��Ŀǰ���÷��ǣ�ֻҪ�����˴˻�����������Ϊ��ʹ��solaris10�汾��gcc ####
rem set XOS_GCC_VER10=1

REM ### ��xos��rules�ļ���includeҵ���ṩ�ı���궨���ļ���                             ####
REM ### �ļ���·�����ļ��������� EXT_RULE_FILE ������,����˻���������include��Ӧ���ļ� ####
rem set EXT_RULE_FILE=

REM ### �������˻���������FD_SETSIZE=4096������Ĭ��FD_SETSIZE=256
rem set BIG_POLLING_FD_SETSIZE=1

set XOS_MDLMGT=1
set XOS_DEBUG=1
set XOS_TELNETD=1
set XOS_NEED_MAIN=1
set XOS_ModMem_Check=1
set XOS_FTP_CLIENT=1
set MEM_FID_DEBUG=1

rem set XOS_FM=1
rem set XOS_UDT_MODULE=1
rem set XOS_NEED_NFS=1

rem set XOS_TIMER_FOURFUNC=1
rem set XOS_HIGHTIMER=1
rem set XOS_HASH_DEBUG=1
rem set XOS_ARRAY_DEBUG=1
rem set INPUT_PAR_CHECK=1

rem set XOS_NOTC_MEM=1
rem set XOS_MSGMEM_TRACE=1
rem set XOS_ATT=1
rem SET XOS_WIN32=1

rem set XOS_IPC_MGNT=1
rem set XOS_IPCTEST=1
rem set XOS_VTA=1
rem set XOS_MCU_DRV=1
rem set XOS_BSI_DRV=1
rem set XOS_SDI_DRV=1
rem set XOS_SAI_DRV=1
rem set XOS_NET_DRV=1
rem set XOS_V5I_DRV=1

rem set XOS_ST_TEST=1
rem set XOS_MEM_ERROR=1
rem set XOS_MSGPRINT_DEBUG=1
rem set XOS_ST_WRITE_LOG=1



REM ################      ##################

SET DEBUG=1

set ENABLE_PR_TRISM=1
set ENABLE_PR_TRICM=1

set DEBUGP=1
set PRODUCT=XW_SS

@echo PRODUCT: XW_SS


REM echo ########### set variables for Tornado  ###########

SET WIND_HOST_TYPE=x86-win32
set PATH=%WIND_BASE%\host\%WIND_HOST_TYPE%\bin;%PATH%


REM Must define CPU �� TORNADO �İ汾 ==================== 6)
set XOS_TSP=1
SET CPU=PPC860

set TORNADO=T22

SET XOS_VX_PPC860=1
SET XOS_T22=1


if .%1 == .lib goto vx
if .%1 == .exe goto vx
if .%1 == .clean goto clean
if .%1 == .cleanexe goto cleanxos
goto endall

REM Must define MAKE_PLATFORM ******
:vx
SET MAKE_PLATFORM=VXWORKS
set MICR_BASE=%WIND_BASE%\host
call %MICR_BASE%\x86-win32\bin\vxrm.bat

goto domake


echo ########### RUN  ###########


:domake
if .%1 == .lib goto makevx
if .%1 == .exe goto makexos
goto endall


:makevx
make -C %OBJS_DIR% -f  %V3000_ROOT%\makefile
goto endall

:clean
rmdir /S /Q %OBJS_DIR%
make -C %OBJS_DIR% -f  %V3000_ROOT%\makefile clean
goto endall


:makexos
rem set OBJTYPE=o
make -C %OBJS_DIR% -f  %V3000_ROOT%\port\xos_exe.mak
if .%2 == .ram goto makeram
goto endall

:cleanxos
make -C %OBJS_DIR% -f  %V3000_ROOT%\port\xos_exe.mak clean
goto endall

:makeram
#set RAMSRC_ROOT=%SRC_ROOT%\xwhx_cps_rules\tndprj
#set RAMOBJS_DIR=%OBJS_DIR%\debug
#make -C %RAMOBJS_DIR% -f  %RAMSRC_ROOT%\makefile
goto endall

:endall
SET PATH=C:\WINNT\system32;C:\WINNT;C:\WINNT\System32\Wbem
@echo All Done!
