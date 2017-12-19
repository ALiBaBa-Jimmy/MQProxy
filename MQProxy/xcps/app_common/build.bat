@echo off
cls


rem *********************************************************
rem define complile SRC_ROOT declaration path *********
rem *********************************************************

rem please subst the SRC_ROOT path
SET SRC_ROOT=D:\xcps_v3000_dev

if exist %SRC_ROOT% goto SRC_ROOT_DIR_OK
   echo %SRC_ROOT% not exist,please verify SRS_ROOT
goto DIR_ERR
:SRC_ROOT_DIR_OK
rem echo %SRC_ROOT%

SET XCPS_ROOT=%SRC_ROOT%\xcps
SET RULES_ROOT=%XCPS_ROOT%\cpsrules
SET OBJS_DIR=%SRC_ROOT%\obj
SET OBJTYPE=o

SET XOS_TIMER_FOURFUNC=1
SET SS_COMPILE_OAM=1
SET OAM_MSG_FROM_FILE=0
SET NEED_OAM=1
SET OAM_FROM_PLAT=0
SET PRODUCT=XW_TCF
SET XOS_NOTC_MEM=1

if exist %OBJS_DIR% goto DIR_OK2
md %OBJS_DIR%
:DIR_OK2

rem CPS compile switch options
SET DEBUG=1
SET WARNING=1
SET XOS_NEED_MAIN=1
SET XOS_EW_START=1
SET XOS_MDLMGT=1
SET XOS_ATT=1
SET XOS_TELNETD=1
SET XOS_DEBUG=1
SET MEM_FID_DEBUG=1
rem SET XOS_ModMem_Check=1
SET XOS_FTP_CLIENT=1

rem cps os compile option
SET XOS_WIN32=1
SET MAKE_PLATFORM=WIN32
SET WIND_HOST_TYPE=x86-win32

rem *********************************************************
rem define complile tornado path *********
rem *********************************************************
rem D:\tornado202 ;
SET WIND_BASE=D:\tornado202
if exist %WIND_BASE% goto WIND_BASE_DIR_OK
echo %WIND_BASE% not exist,please set tornado path
goto DIR_ERR
:WIND_BASE_DIR_OK
rem echo %WIND_BASE%

SET CPU=PENTIUM

Rem SET XOS_TIMER_FOURFUNC=1


rem agent compile switch
rem SET USE_SA=1
rem SET USE_PM=1
rem SET USE_FM=1
rem SET USE_APP=1
rem SET NEED_SA=0


SET PATH=%WIND_BASE%\host\%WIND_HOST_TYPE%\bin;%PATH%

:win
SET MICR_BASE=C:\PROGRA~1\MICROS~3

call %MICR_BASE%\vc98\bin\vcvars32.bat

if .%1 == .clean goto DoClean

make -C %OBJS_DIR% -f  %XCPS_ROOT%\app_common\makefile

goto EndAll

:DoClean
make -C %OBJS_DIR% -f  %XCPS_ROOT%\app_common\makefile clean
echo enter Y/N to delete tree %OBJS_DIR%
rd /s %OBJS_DIR%
goto EndAll

:EndAll
set PATH=%OLD_PATH%
rem echo %PATH%
echo Path=C:\WINNT\system32;C:\WINNT;C:\WINNT\System32\Wbem;
@echo All Done!

:DIR_ERR
