SET WIND_HOST_TYPE=x86-win32
SET SRC_ROOT=d:\vss\ngn\develop\src
set WIND_BASE=d:\tornado-202
SET CPU=PENTIUM
set PATH=%WIND_BASE%\host\%WIND_HOST_TYPE%\bin;%PATH%
SET DEBUG=1

set DEBUGP=1
set PRODUCT=ISUN_SS
@echo PRODUCT: temp name

if .%1 == .win goto win
if .%1 == .vxw goto vxw
goto endall

:win
SET MAKE_PLATFORM=WIN32
SET MICR_BASE=C:\Progra~1\Micros~3
call %MICR_BASE%\vc98\bin\vcvars32.bat
SET OBJS_DIR=D:\objs_dir\win32

goto endall


:endall
@echo All Done!
