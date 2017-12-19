echo off

echo ####### set variables for makefile system  #######
rem setenv [OBJS_DIR] [PRODUCT] [CPU] [MAKE_PLATFORM] [SRC_ROOT]  
call setenv d:\objs_dir\win32 ISUN_SS PENTIUM WIN32

echo ##### set variables for microsoft Visual C++ #####
rem 察看C:\Program Files\Microsoft Visual Studio\VC98\Bin\vcvars32.bat
rem 我们可以得到Visual C++的dos格式目录名，比如C:\Progra~1\Micros~3
set MICR_BASE=C:\Progra~1\Micros~3
call %MICR_BASE%\vc98\bin\vcvars32.bat

echo ########### set variables for Tornado  ###########
rem 如果使用windriver下的tornado202_x86，则setenv会自动设置本环境变量。
rem 否则用户按下面的方法设置
rem @set WIND_BASE=%SRC_ROOT%\windriver\tornado202_x86
rem @set WIND_HOST_TYPE=x86-win32
rem PATH %WIND_BASE%\host\WIND_HOST_TYPE\bin;%path%


echo ########### Avoid too long PATH  ###########
if "%OLD_SETENV_PATH%" == "" goto set_old_path
set PATH=%OLD_SETENV_PATH%
goto end

:set_old_path
set OLD_SETENV_PATH=%PATH%
goto end

:end

