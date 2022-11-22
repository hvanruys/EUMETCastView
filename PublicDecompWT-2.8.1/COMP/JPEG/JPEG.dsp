# Microsoft Developer Studio Project File - Name="JPEG" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=JPEG - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JPEG.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JPEG.mak" CFG="JPEG - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JPEG - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "JPEG - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "JPEG - Win32 Release"

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
# ADD CPP /nologo /W3 /GR /GX /O1 /I ".\inc" /I "..\Inc" /I "..\..\DISE" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x0400 /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "JPEG - Win32 Debug"

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
# ADD CPP /nologo /W3 /GR /GX /ZI /Od /I ".\inc" /I "..\Inc" /I "..\..\DISE" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x0400 /YX /FD /GZ /c
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

# Name "JPEG - Win32 Release"
# Name "JPEG - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Src\CHcodec.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CHOptim.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CHufftables.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CJBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CJPEGDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CJPEGLossLessCoder.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CJPEGLossyCoder.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CompressJPEG.cpp
# End Source File
# Begin Source File

SOURCE=.\Src\CQuantizationTable.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Inc\CHcodec.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CHOptim.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CHufftables.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CJBlock.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CJPEGCoder.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CJPEGDecoder.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CJPEGLossLessCoder.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CJPEGLossyCoder.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CompressJPEG.h
# End Source File
# Begin Source File

SOURCE=.\Inc\CQuantizationTable.h
# End Source File
# Begin Source File

SOURCE=.\Inc\JPEGConst.h
# End Source File
# End Group
# End Target
# End Project
