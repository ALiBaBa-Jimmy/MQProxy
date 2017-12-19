rem ************set the root path of src*********************
@set currentPath=%cd%
@cd ../src
@set rootPath=%cd%
@cd %currentPath%
SET SRC_ROOT=%rootPath%

SET XOSTEST_ROOT=%SRC_ROOT%\..
SET Tornado_ROOT=c:\Tornado2.2

SET DEALWITH_C=%Tornado_ROOT%\host\x86-win32\bin
SET TORNADO_HUTILS=%Tornado_ROOT%/host/src/hutils
SET TORNADO_LINK_DIR=%Tornado_ROOT%/target/h/tool/gnu/ldscripts

SET DIR_BIN=%XOSTEST_ROOT%\bin

set Obj_File=xostest.o

@echo start c++ obj link
nmppc %DIR_BIN%\%Obj_File% | wtxtcl %TORNADO_HUTILS%/munch.tcl -c ppc > %DIR_BIN%\test.c 
ccppc  -mcpu=860 -fno-builtin -nostdinc  -DTOOL_FAMILY=gnu -DTOOL=gnu -c -o %DIR_BIN%\test.o %DIR_BIN%\test.c

echo deal with result file
rem move /Y %DEALWITH_C%\xostest.o %DIR_BIN%\xostest.o

ccppc -r -nostdlib -Wl,-X -T %TORNADO_LINK_DIR%/link.OUT -o %DIR_BIN%\xostest.out %DIR_BIN%\%Obj_File% %DIR_BIN%\test.o

@echo c++ link all done!