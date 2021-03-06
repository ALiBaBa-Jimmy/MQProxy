rem ************set the root path of src*********************
@set currentPath=%cd%
@cd ..\src
@set rootPath=%cd%
@cd ..\..\..\
SET SRC_ROOT=%cd%
@cd %currentPath%
SET TEST_SRC_ROOT=%rootPath%

SET XOSTEST_ROOT=%TEST_SRC_ROOT%\..
SET DIR_BIN=%XOSTEST_ROOT%\bin
SET OBJS_DIR=%XOSTEST_ROOT%\obj\vxworks
SET XCPS_ROOT=%XOSTEST_ROOT%\..\..
SET V3000_ROOT=%XCPS_ROOT%\cpsv3000
SET XOS_PORT=%V3000_ROOT%\port

rem ����objĿ¼
if not exist %OBJS_DIR% md %OBJS_DIR%

SET OBJTYPE=o

set RULES_ROOT=%XCPS_ROOT%\cpsrules

SET PRODUCT=XOSTEST

SET XOS_TIMER_FOURFUNC=1
SET XOS_MDLMGT=1
SET XOS_NOTC_MEM=1
SET XOS_VTA=1
SET XOS_EW_START=1
set XOS_TELNETD=1
set XOS_FTP_CLIENT=1
set XOS_NEED_MAIN=1
SET XOS_WIN32=1
SET XOS_NOTC_MEM=1
SET MEM_FID_DEBUG=1
SET XOS_TSP=1
SET XOS_UDT_MODULE=0
SET XOS_NEED_NFS=1
SET NEED_OAM=1

SET OAM_MSG_FROM_FILE=0
SET NEED_OAM=1

SET MAKE_PLATFORM=VXWORKS
SET WIND_HOST_TYPE=x86-win32
set WIND_BASE=c:\tornado2.2
SET CPU=PPC860
SET XOS_VX_PPC860=1
set TORNADO=T22
SET XOS_T22=1
SET GCC_INIT_MODULEID=1

SET USE_OCT=0
SET USE_AMP600=1

SET DEBUG=0
SET SCC_DEBUG=1
SET GCC_INIT_MODULEID=1
SET RELEASE_VERSION=1
if .%RELEASE_VERSION% == .1 goto ReleaseScc
rem ##### define debug version macro #####
SET MEM_MSG_TRACK=1
SET XOS_ATT=1
SET SS_LOG_TRACE=1
SET CPU_HIGH_BYTE=1
SET MEM_FID_DEBUG=1
SET XOS_NOTC_MEM=1
SET XOS_VTA=1
SET XOS_DEBUG=1
SET XOS_MSGMEM_TRACE=1
SET CPPFRM_DEBUG=1
SET CPPFRM_MEM_NEW=1
SET XOS_NEED_NFS=1

:ReleaseScc
SET CPU_HIGH_BYTE=1
rem SET CPPFRM_DEBUG=1         release version
set XOS_ModMem_Check=1
rem set XOS_NEED_MAIN=0
set XOS_ARRAY_DEBUG=1
set XOS_HASH_DEBUG=1
set INPUT_PAR_CHECK=1

set PATH=%WIND_BASE%\host\%WIND_HOST_TYPE%\bin;%PATH%

set VOB_DIR=%SRC_ROOT%


set MICR_BASE=%WIND_BASE%\host
call %MICR_BASE%\x86-win32\bin\vxrm.bat

if .%1 == .exe goto domake
if .%1 == .clean goto DoClean
goto domake

:domake
make -C %OBJS_DIR% -f  %XOSTEST_ROOT%\build\xos_test.mk
rmdir /S /Q %OBJS_DIR%\cpsv3000\test\build
goto EndAll

:DoClean
rem make -C %OBJS_DIR% -f  %XOSTEST_ROOT%\build\xos_test.mk clean
rmdir /S /Q %XOSTEST_ROOT%\obj
goto EndAll


:EndAll

echo Path=C:\WINNT\system32;C:\WINNT;C:\WINNT\System32\Wbem;
@echo All Done!
