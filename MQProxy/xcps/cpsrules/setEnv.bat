@echo off

if "%1"=="help" goto help
if "%1"=="-h" goto help
if "%1"=="" goto default
set OBJS_DIR=%1
if "%2"=="" goto no_param
set PRODUCT=%2
if "%3"=="" goto no_param
set CPU=%3
if "%4"=="" goto no_param
set MAKE_PLATFORM=%4
if "%5"=="" goto local_src
set SRC_ROOT=%5
goto set_over
:local_src
cd ..
set SRC_ROOT=%cd%
cd build
goto set_over


:help
@echo ********************************************************************************
@echo *                          === SETENV H E L P ===                              *
@echo * Syntax:                                                                      *
@echo *    setenv [OBJS_DIR] [PRODUCT] [CPU] [MAKE_PLATFORM] [SRC_ROOT]              *
@echo * Description:                                                                 *
@echo *  [OBJS_DIR]:                                                                 *
@echo *            To specify a directory where to put the object and depend files   *
@echo *  [PRODUCT]:                                                                  *
@echo *            To define which product you want to build.                        *
@echo *            The possible values are : ISUN_SS, ISUN_HLR                       *
@echo *  [CPU]:                                                                      *
@echo *            To specify the cpu type used.                                     *
@echo *            The possible values are : PENTIUM PPC860                          *
@echo *  [MAKE_PLATFORM]:                                                            *
@echo *            To define operation system used                                   *
@echo *                VXWORKS WIN32 SOLARIS LINUX SCO                               *
@echo *  [SRC_ROOT]:                                                                 *
@echo *            To specify a directory where the source files exist.              *
@echo *            the Batch file can compute the SRC_ROOT by the current directory. *
@echo *  When you only type "setenv", they will be set to the following              *
@echo *  default values:                                                             *
@echo *             OBJS_DIR      = c:\OBJS_DIR                                      *
@echo *             PRODUCT       = ISUN_SS                                          *
@echo *             CPU           = PENTIUM                                          *
@echo *             MAKE_PLATFORM = VXWORKS                                          *
@echo *             SRC_ROOT = the upper layer above the directory                   *
@echo *                        where this batch file exists                          *
@echo ********************************************************************************

GOTO exit_to_cmd

:no_param
@echo Incorrect parameters!
@echo Please type:%0 [OBJS_DIR] [PRODUCT] [CPU] [MAKE_PLATFORM]  
@echo or type "%0 help" for more information
GOTO exit_to_cmd

:default
@echo off
if not defined OBJS_DIR      (@set OBJS_DIR=c:\objs_dir)
if not defined PRODUCT       (@set PRODUCT=ISUN_SS)
if not defined CPU           (@set CPU=PENTIUM)
if not defined MAKE_PLATFORM (@set MAKE_PLATFORM=VXWORKS)
cd ..
@set SRC_ROOT=%cd%
cd build
if not defined OPTIMIZE      (@set OPTIMIZE=0)
@echo You may type "%0 help" for more information
goto set_over

:set_over
if not defined OPTIMIZE      (@set OPTIMIZE=0)
set CPU
set OBJS_DIR
set SRC_ROOT
set MAKE_PLATFORM
if defined PRODUCT set PRODUCT
set OPTIMIZE
@echo off
if not defined WIND_HOST_TYPE (@set WIND_HOST_TYPE=x86-win32)
if not defined WIND_BASE goto setwindbase    
goto end

:setwindbase
if "%CPU%" == "PPC860" goto set_wind_base_ppc
@set WIND_BASE=%SRC_ROOT%\windriver\tornado202_x86
goto end
:set_wind_base_ppc
@set WIND_BASE=%SRC_ROOT%\windriver\tornado20.ppc
goto end

:end
PATH %WIND_BASE%\host\%WIND_HOST_TYPE%\bin;%path%
set WIND_HOST_TYPE
set WIND_BASE
:exit_to_cmd

