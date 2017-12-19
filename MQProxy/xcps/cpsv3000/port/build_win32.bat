echo off

echo ####### set variables for makefile system  #######

REM 下面是目录的设置，注意:请保留平台的路径变量，您只需修改 1) 2) 3) 4) 5) 6) 处的变量

REM echo ###根目录的设置，###必须设置### ========== 1)
set SRC_ROOT=%cd%\..\..

REM 平台的根目录（即cpsrules和cpsv3000的上一层目录），各应用层可类似的定义自己的根目录### ========== 2)
set CPS_ROOT=%SRC_ROOT%
set XCPS_ROOT=%SRC_ROOT%


rem echo ###平台源文件cpsv3000的目录，###必须设置###（请不要改动）
set V3000_ROOT=%CPS_ROOT%\cpsv3000
rem 平台编译规则的目录，不设置的话则默认与平台代码v3000在同一级目录
set RULES_ROOT=%CPS_ROOT%\cpsrules


REM echo ##### set variables for microsoft Visual C++ ##### ========== VC的安装路径 3）
rem set MICR_BASE=C:\PROGRA~1\MICROS~3
set MICR_BASE=C:\PROGRA~1\MICROS~4
call %MICR_BASE%\vc98\bin\vcvars32.bat

REM echo ###Tornado_202的路径，2.02版本 ===================== 4)
set WIND_BASE=c:\Tornado2.02


REM echo ###下面是要生成目标文件的路径，默认是在平台目录cpsv3000\obj下 (可以自行设定)=============== 5)
SET OBJS_DIR=%V3000_ROOT%\obj\win32
rem 建立目标文件目录
if not exist %OBJS_DIR% md %OBJS_DIR%

REM 需要平台生成目标文件的类型（如：lib或o）
set OBJTYPE=lib

set ENABLE_PR_XOS_OUT=xosrun


REM echo ###下面是环境变量的定义（对应相应的编译宏），需要的话把前面的"rem"去掉 ========== 6)


REM ##### gcc 的版本，目前的用法是：只要定义了此环境变量就认为是使用solaris10版本的gcc ####
rem set XOS_GCC_VER=1

REM ### 在xos的rules文件中include业务提供的编译宏定义文件，                             ####
REM ### 文件的路径和文件名保存在 EXT_RULE_FILE 变量中,定义此环境变量就include相应的文件 ####
rem set EXT_RULE_FILE=

REM ### 如果定义此环境变量，FD_SETSIZE=4096，否则默认FD_SETSIZE=256
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