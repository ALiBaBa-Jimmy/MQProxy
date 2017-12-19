# Microsoft Developer Studio Project File - Name="xos" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=xos - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xos_vc6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xos_vc6.mak" CFG="xos - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xos - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "xos - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "xos"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xos - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "./bdy/inc" /I "./common" /I "./inc" /I "./log4cplus/inc" /I "./xosxml/inc" /I "./port" /I "./os/win32/inc" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "xos - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./obj/windows/xos/vc6"
# PROP Intermediate_Dir "./obj/windows/xos/vc6"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "./bdy/inc" /I "./common" /I "./inc" /I "./log4cplus/inc" /I "./xosxml/inc" /I "./port" /I "./os/win32/inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "XOS_WIN32" /D "XOS_MDLMGT" /D "XOS_NEED_MAIN" /D "XOS_FTP_CLIENT" /D "HAVE_WINDOWS_H" /D "XOS_EW_START" /D "USE_SA" /D "USE_FM" /D "USE_APP" /D "MEM_FID_DEBUG" /D "XOS_NOTC_MEM" /D "XOS_TELNETD" /D "XOS_DEBUG" /D "XOS_ModMem_Check" /D "USE_PM" /D "XOS_NEED_CHK" /D "WPCAP" /D "XOS_HIGHTIMER" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"./lib/xos_vc6.lib"

!ENDIF 

# Begin Target

# Name "xos - Win32 Release"
# Name "xos - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "cpsv3000"

# PROP Default_Filter ""
# Begin Group "bdy"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\bdy\src\cliatt.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\clicmds.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\clishell.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\clitelnet.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\clitelnetd.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\cmtimer.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosarray.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xoscfg.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosenc.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosfilesys.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosfm.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosftp.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosha.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xoshash.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosinet.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosipc.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xoslist.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xoslog.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosmd5.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosmem.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosmmgt.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosmodule.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosmon.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosmtp2.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosntl.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosntpc.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosos.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosphonenum.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xospub.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosqueue.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosroot.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xossctp.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xossctp_win.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xostrace.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xostrie.c
# End Source File
# Begin Source File

SOURCE=.\bdy\src\xosxml.c
# End Source File
# End Group
# Begin Group "os"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\os\win32\src\os.c
# End Source File
# End Group
# Begin Group "port"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\port\xosport.c
# End Source File
# End Group
# Begin Group "xosxml"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\xosxml\src\xmlencoding.c
# End Source File
# Begin Source File

SOURCE=.\xosxml\src\xmlentities.c
# End Source File
# Begin Source File

SOURCE=.\xosxml\src\xmlIO.c
# End Source File
# Begin Source File

SOURCE=.\xosxml\src\xmlparser.c
# End Source File
# Begin Source File

SOURCE=.\xosxml\src\xmlsave.c
# End Source File
# Begin Source File

SOURCE=.\xosxml\src\xmlstring.c
# End Source File
# Begin Source File

SOURCE=.\xosxml\src\xmltree.c
# End Source File
# Begin Source File

SOURCE=.\xosxml\src\xwriter.c
# End Source File
# End Group
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
