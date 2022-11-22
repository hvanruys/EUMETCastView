# Microsoft Developer Studio Project File - Name="DISE" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DISE - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DISE.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DISE.mak" CFG="DISE - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DISE - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DISE - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DISE - Win32 Release"

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
# ADD CPP /nologo /W3 /GR /GX /O1 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x0400 /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "DISE - Win32 Debug"

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
# ADD CPP /nologo /W3 /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x0400 /YX /FD /GZ /c
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

# Name "DISE - Win32 Release"
# Name "DISE - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CxRITAnnotation.cpp
# End Source File
# Begin Source File

SOURCE=.\CxRITFile.cpp
# End Source File
# Begin Source File

SOURCE=.\CxRITFileHeaderRecords.cpp
# End Source File
# Begin Source File

SOURCE=.\ErrorHandling.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CDataField.h
# End Source File
# Begin Source File

SOURCE=.\CSpacecraftID.h
# End Source File
# Begin Source File

SOURCE=.\CSpectralChannelID.h
# End Source File
# Begin Source File

SOURCE=.\CxRITAnnotation.h
# End Source File
# Begin Source File

SOURCE=.\CxRITFile.h
# End Source File
# Begin Source File

SOURCE=.\CxRITFileCompressed.h
# End Source File
# Begin Source File

SOURCE=.\CxRITFileDecompressed.h
# End Source File
# Begin Source File

SOURCE=.\CxRITFileHeaderRecords.h
# End Source File
# Begin Source File

SOURCE=.\ErrorHandling.h
# End Source File
# Begin Source File

SOURCE=.\GSDS_Volume_F.h
# End Source File
# Begin Source File

SOURCE=.\GSDS_Volume_F_Impl.h
# End Source File
# Begin Source File

SOURCE=.\GSDS_Volume_F_NBO.h
# End Source File
# Begin Source File

SOURCE=.\GSDS_Volume_F_NBO_impl.h
# End Source File
# Begin Source File

SOURCE=.\MSGTime.h
# End Source File
# Begin Source File

SOURCE=.\MSGTime_impl.h
# End Source File
# Begin Source File

SOURCE=.\SmartPtr.h
# End Source File
# Begin Source File

SOURCE=.\Types.h
# End Source File
# Begin Source File

SOURCE=.\YYYYMMDDhhmm.h
# End Source File
# End Group
# End Target
# End Project
