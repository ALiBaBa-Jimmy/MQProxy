SET WIND_HOST_TYPE=x86-win32




REM>>>>>>>>开始目录设置

REM============= 相关工具安装目录及设置 =============

SET WIND_BASE=C:\Tornado
SET PATH=%WIND_BASE%\host\%WIND_HOST_TYPE%\bin;%PATH%

SET MICR_BASE="D:\Program Files\Microsoft Visual Studio"




REM============= 工程相关目录 =============
SET PRG_ROOT=F:\CC\hlr\hlr_dev\hlr3000
SET SRC_ROOT=%PRG_ROOT%\src
SET OBJS_DIR=%PRG_ROOT%\obj




REM============= 生成的目标程序所在目录 =============

SET HLR_OUTPUT=%PRG_ROOT%\bin\win32\hlr.exe



REM============= Oracle库目录 =============
SET ORACLE_HOME=C:\oracle\product\ora10g
SET ORACLE_LIB=%ORACLE_HOME%\precomp\LIB\orasql10.lib %PRG_ROOT%\proj\vc\lib\charge_agt.lib

#============ HLR相关设置 =============
SET WITH_CHARGE_AGENT=YES

REM============= XOS平台、OAM和Agent相关目录、文件和环境 =============

SET OBJTYPE=o

SET XCPS_ROOT=%SRC_ROOT%\xcps
SET RULES_ROOT=%XCPS_ROOT%\cpsrules

SET CMM_SAAP=%XCPS_ROOT%\saap

SET NEED_OAM=1
SET OAM_MSG_FROM_FILE=0

SET MAKE_PLATFORM=WIN32
SET PRODUCT=XW_HLR
SET CPU=PENTIUM
SET ENABLE_PR_XOS_OUT=xoslin
SET WARNING=0
SET DEBUG=1
SET MEM_FID_DEBUG=1
SET XOS_NEED_MAIN=1
SET XOS_MDLMGT=1
SET XOS_TELNETD=1
SET XOS_NOTC_MEM=1
SET XOS_ModMem_Check=1
SET XOS_HASH_DEBUG=1
SET XOS_ARRAY_DEBUG=1
SET XOS_INPUT_PAR_CHECK=1
SET XOS_TIMER_FOURFUNC=1
SET XOS_FTP_CLIENT=1

SET XOS_EW_START=1




REM============= 协议栈相关目录、文件和设置 =============
SET PRTL_SRC=%SRC_ROOT%\protocol\protocol

SET NEED_TRI_SSI=YES

SET PRTCL_ENV_BAT=%PRTL_SRC%\stackbuild\setevent.bat




REM<<<<<<<<完成目录设置


CALL %PRTCL_ENV_BAT%
CALL %MICR_BASE%\vc98\bin\vcvars32.bat

echo on

make -C %OBJS_DIR% -f  %SRC_ROOT%\xdb\makefile_proc
make -C %OBJS_DIR% -f  %SRC_ROOT%\hlr.mak

@echo 编译结束
