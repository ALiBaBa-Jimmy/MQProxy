# Microsoft Developer Studio Project File - Name="test_xos" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=test_xos - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "test_xos.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "test_xos.mak" CFG="test_xos - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "test_xos - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test_xos - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "test_xos - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "./../../bdy/inc" /I "./../../common" /I "./../../inc" /I "./../../log4cplus/inc" /I "./../../xosxml/inc" /I "./../../port" /I "./../../os/win32/inc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "test_xos - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./../../obj/windows/test_xos/vc6"
# PROP Intermediate_Dir "./../../obj/windows/test_xos/vc6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "./../../../bdy/inc" /I "./../../../common" /I "./../../../inc" /I "./../../../log4cplus/inc" /I "./../../../xosxml/inc" /I "./../../../port" /I "./../../../os/win32/inc" /I "../ftptest/" /I "../md5/" /I "../memtest/" /I "../ntltest/" /I "../timer/" /I "../udttest/" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "XW_RBTC" /D "XOS_WIN32" /D "XOS_MDLMGT" /D "XOS_NEED_MAIN" /D "XOS_FTP_CLIENT" /D "HAVE_WINDOWS_H" /D "XOS_EW_START" /D "USE_SA" /D "USE_FM" /D "USE_APP" /D "MEM_FID_DEBUG" /D "XOS_NOTC_MEM" /D "XOS_TELNETD" /D "XOS_DEBUG" /D "XOS_ModMem_Check" /D "USE_PM" /D "XOS_NEED_CHK" /D "WPCAP" /D "_XW_rbtc" /D "XOS_HIGHTIMER" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib xos_vc6.lib dbghelp.lib Winmm.lib sctp_vc.lib gio-2.0.lib glib-2.0.lib gmodule-2.0.lib gobject-2.0.lib gthread-2.0.lib /nologo /subsystem:console /debug /machine:I386 /out:"./autotest.exe" /pdbtype:sept /libpath:"./../../../lib" /libpath:"./../../../" /libpath:"./../../../libsctp/win"

!ENDIF 

# Begin Target

# Name "test_xos - Win32 Release"
# Name "test_xos - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=auto_test.c
# End Source File
# Begin Source File

SOURCE=..\sctp_client\sctp_client.cpp
# End Source File
# Begin Source File

SOURCE=..\sctp_server\sctp_server.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\auto_test.h
# End Source File
# Begin Source File

SOURCE=..\sctp_client\sctp_client.h
# End Source File
# Begin Source File

SOURCE=..\sctp_server\sctp_server.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
