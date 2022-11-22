# Microsoft Developer Studio Project File - Name="T4" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=T4 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "T4.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "T4.mak" CFG="T4 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "T4 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "T4 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "T4 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "T4___Win32_Release"
# PROP BASE Intermediate_Dir "T4___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O1 /I ".\inc" /I "..\Inc" /I "..\..\DISE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x0400 /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "T4 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "T4___Win32_Debug"
# PROP BASE Intermediate_Dir "T4___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GR /GX /ZI /Od /I ".\inc" /I "..\Inc" /I "..\..\DISE" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x0400 /FD /GZ /c
# SUBTRACT CPP /FA<none> /YX
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "T4 - Win32 Release"
# Name "T4 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Src\CBitBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CompressT4.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\T4Coder.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\T4Codes.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\T4Decoder.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Inc\CBitBuffer.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CompressT4.h
# End Source File
# Begin Source File

SOURCE=.\Inc\T4Coder.h
# End Source File
# Begin Source File

SOURCE=.\Inc\T4Codes.h
# End Source File
# Begin Source File

SOURCE=.\Inc\T4Decoder.h
# End Source File
# End Group
# End Target
# End Project
