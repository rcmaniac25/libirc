# Microsoft Developer Studio Project File - Name="libirc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libirc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libirc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libirc.mak" CFG="libirc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libirc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libirc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libirc - Win32 Release"

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
# ADD CPP /nologo /MT /w /W0 /GX /O2 /I "../include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libirc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /w /W0 /Gm /GX /ZI /Od /I "../include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "libirc - Win32 Release"
# Name "libirc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\ircBasicCommands.cpp
# End Source File
# Begin Source File

SOURCE=..\src\IRCClient.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ircCommands.cpp
# End Source File
# Begin Source File

SOURCE=..\src\irClientCommands.cpp
# End Source File
# Begin Source File

SOURCE=..\src\irClientEvents.cpp
# End Source File
# Begin Source File

SOURCE=..\src\IRCServer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\IRCTextUtils.cxx
# End Source File
# Begin Source File

SOURCE=..\src\IRCUserManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\libIRC.cpp
# End Source File
# Begin Source File

SOURCE=..\src\net.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TCPConnection.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "commandHandlers"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\src\ircBasicCommands.h
# End Source File
# End Group
# Begin Group "networking"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\include\net.h
# End Source File
# Begin Source File

SOURCE=..\include\TCPConnection.h
# End Source File
# End Group
# Begin Group "utils"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\include\IRCTextUtils.h
# End Source File
# Begin Source File

SOURCE=..\include\Singleton.h
# End Source File
# End Group
# Begin Group "IRC spec"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\include\ircCommands.h
# End Source File
# Begin Source File

SOURCE=..\include\IRCNumerics.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\include\IRCClient.h
# End Source File
# Begin Source File

SOURCE=..\include\IRCEvents.h
# End Source File
# Begin Source File

SOURCE=..\include\IRCServer.h
# End Source File
# Begin Source File

SOURCE=..\include\IRCUserManager.h
# End Source File
# Begin Source File

SOURCE=..\include\libIRC.h
# End Source File
# End Group
# End Target
# End Project
