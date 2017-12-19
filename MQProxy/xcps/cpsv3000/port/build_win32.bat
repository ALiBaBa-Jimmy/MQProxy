echo off

echo ####### set variables for makefile system  #######

REM ������Ŀ¼�����ã�ע��:�뱣��ƽ̨��·����������ֻ���޸� 1) 2) 3) 4) 5) 6) ���ı���

REM echo ###��Ŀ¼�����ã�###��������### ========== 1)
set SRC_ROOT=%cd%\..\..

REM ƽ̨�ĸ�Ŀ¼����cpsrules��cpsv3000����һ��Ŀ¼������Ӧ�ò�����ƵĶ����Լ��ĸ�Ŀ¼### ========== 2)
set CPS_ROOT=%SRC_ROOT%
set XCPS_ROOT=%SRC_ROOT%


rem echo ###ƽ̨Դ�ļ�cpsv3000��Ŀ¼��###��������###���벻Ҫ�Ķ���
set V3000_ROOT=%CPS_ROOT%\cpsv3000
rem ƽ̨��������Ŀ¼�������õĻ���Ĭ����ƽ̨����v3000��ͬһ��Ŀ¼
set RULES_ROOT=%CPS_ROOT%\cpsrules


REM echo ##### set variables for microsoft Visual C++ ##### ========== VC�İ�װ·�� 3��
rem set MICR_BASE=C:\PROGRA~1\MICROS~3
set MICR_BASE=C:\PROGRA~1\MICROS~4
call %MICR_BASE%\vc98\bin\vcvars32.bat

REM echo ###Tornado_202��·����2.02�汾 ===================== 4)
set WIND_BASE=c:\Tornado2.02


REM echo ###������Ҫ����Ŀ���ļ���·����Ĭ������ƽ̨Ŀ¼cpsv3000\obj�� (���������趨)=============== 5)
SET OBJS_DIR=%V3000_ROOT%\obj\win32
rem ����Ŀ���ļ�Ŀ¼
if not exist %OBJS_DIR% md %OBJS_DIR%

REM ��Ҫƽ̨����Ŀ���ļ������ͣ��磺lib��o��
set OBJTYPE=lib

set ENABLE_PR_XOS_OUT=xosrun


REM echo ###�����ǻ��������Ķ��壨��Ӧ��Ӧ�ı���꣩����Ҫ�Ļ���ǰ���"rem"ȥ�� ========== 6)


REM ##### gcc �İ汾��Ŀǰ���÷��ǣ�ֻҪ�����˴˻�����������Ϊ��ʹ��solaris10�汾��gcc ####
rem set XOS_GCC_VER=1

REM ### ��xos��rules�ļ���includeҵ���ṩ�ı���궨���ļ���                             ####
REM ### �ļ���·�����ļ��������� EXT_RULE_FILE ������,����˻���������include��Ӧ���ļ� ####
rem set EXT_RULE_FILE=

REM ### �������˻���������FD_SETSIZE=4096������Ĭ��FD_SETSIZE=256
rem set BIG_POLLING_FD_SETSIZE=1

set XOS_MDLMGT=1
set XOS_DEBUG=1
set XOS_TELNETD=1
set XOS_NEED_MAIN=1
rem set XOS_LOGWRITE_TEST=1
rem set XOS_LOG_DEBUG=1
set XOS_FTP_CLIENT=1
rem set XOS_UDT_MODULE=0
set MEM_FID_DEBUG=1
rem set XOS_FM=1

rem set XOS_TIMER_FOURFUNC=1
set XOS_HIGHTIMER=1
set XOS_HASH_DEBUG=1
set XOS_ARRAY_DEBUG=1
set INPUT_PAR_CHECK=1

rem set XOS_NOTC_MEM=1
rem set XOS_MSGMEM_TRACE=1
rem set XOS_ATT=1


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
set XOS_MSGPRINT_DEBUG=1
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


REM Must define CPU ******
SET CPU=PENTIUM

if /i "%PROCESSOR_IDENTIFIER:~0,3%" == "X86" goto x86
if /i "%PROCESSOR_IDENTIFIER:~0,3%" NEQ "X86" goto x86_64

:x86
SET CPU=PENTIUM
goto cpuend

:x86_64
echo "windows 64 not support"
goto endall

:cpuend

if .%1 == .lib goto win
if .%1 == .exe goto win
if .%1 == .clean goto clean
if .%1 == .cleanexe goto cleanxos
goto endall

REM Must define MAKE_PLATFORM ******
:win
SET MAKE_PLATFORM=WIN32
goto domake


echo ########### RUN  ###########


:domake
if .%1 == .lib goto makewin
if .%1 == .exe goto makexos
goto endall


:makewin
make -C %OBJS_DIR% -f  %V3000_ROOT%\makefile
goto endall

:clean
make -C %OBJS_DIR% -f  %V3000_ROOT%\makefile clean
goto endall


:makexos
rem set OBJTYPE=o
make -C %OBJS_DIR% -f  %V3000_ROOT%\port\xos_exe.mak
goto endall

:cleanxos
make -C %OBJS_DIR% -f  %V3000_ROOT%\port\xos_exe.mak clean
goto endall


:endall
SET PATH=C:\WINNT\system32;C:\WINNT;C:\WINNT\System32\Wbem
@echo All Done!