# Microsoft Developer Studio Project File - Name="py" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=py - Win32 Threads Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "py.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "py.mak" CFG="py - Win32 Threads Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "py - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "py - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "py - Win32 Threads Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "py - Win32 Threads Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "py"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "py - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pd-msvc\r"
# PROP Intermediate_Dir "pd-msvc\r"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PY_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext\source" /I "C:\Programme\prog\Python22\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D FLEXT_SYS=2 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib pd.lib flext-pdwin.lib /nologo /dll /machine:I386 /libpath:"c:/programme/audio/pd/bin" /libpath:"..\flext\pd-msvc" /libpath:"C:\Programme\prog\Python22\libs"

!ELSEIF  "$(CFG)" == "py - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pd-msvc\d"
# PROP Intermediate_Dir "pd-msvc\d"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PY_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /ZI /Od /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext\source" /I "C:\Programme\prog\Python22\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D FLEXT_SYS=2 /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib pd.lib flext_d-pdwin.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"c:/programme/audio/pd/bin" /libpath:"..\flext\pd-msvc" /libpath:"f:\prog\packs\Python-2.2.1\PCbuild"

!ELSEIF  "$(CFG)" == "py - Win32 Threads Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "py___Win32_Threads_Release"
# PROP BASE Intermediate_Dir "py___Win32_Threads_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pd-msvc\tr"
# PROP Intermediate_Dir "pd-msvc\tr"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GR /GX /O2 /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext" /I "C:\Programme\prog\Python22\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PD" /D "NT" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /O2 /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext\source" /I "C:\Programme\prog\Python22\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D FLEXT_SYS=2 /D "FLEXT_THREADS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib pd.lib flext-pdwin.lib /nologo /dll /machine:I386 /libpath:"c:/programme/audio/pd/bin" /libpath:"..\flext\msvc" /libpath:"C:\Programme\prog\Python22\libs"
# ADD LINK32 kernel32.lib user32.lib pd.lib flext_t-pdwin.lib pthreadVC.lib /nologo /dll /machine:I386 /out:"pd-msvc\py.dll" /libpath:"c:/programme/audio/pd/bin" /libpath:"..\flext\pd-msvc" /libpath:"C:\Programme\prog\Python22\libs"

!ELSEIF  "$(CFG)" == "py - Win32 Threads Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "py___Win32_Threads_Debug"
# PROP BASE Intermediate_Dir "py___Win32_Threads_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pd-msvc\td"
# PROP Intermediate_Dir "pd-msvc\td"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext" /I "C:\Programme\prog\Python22\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PD" /D "NT" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "c:\programme\audio\pd\src" /I "f:\prog\max\flext\source" /I "C:\Programme\prog\Python22\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D FLEXT_SYS=2 /D "FLEXT_THREADS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib pd.lib flext-pdwin.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"c:/programme/audio/pd/bin" /libpath:"..\flext\msvc-debug" /libpath:"f:\prog\packs\Python-2.2.1\PCbuild"
# ADD LINK32 kernel32.lib user32.lib pd.lib flext_td-pdwin.lib pthreadVC.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"c:/programme/audio/pd/bin" /libpath:"..\flext\pd-msvc" /libpath:"f:\prog\packs\Python-2.2.1\PCbuild"

!ENDIF 

# Begin Target

# Name "py - Win32 Release"
# Name "py - Win32 Debug"
# Name "py - Win32 Threads Release"
# Name "py - Win32 Threads Debug"
# Begin Group "scripts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\scripts\script.py
# End Source File
# Begin Source File

SOURCE=.\scripts\sendrecv.py
# End Source File
# Begin Source File

SOURCE=.\scripts\simple.py
# End Source File
# Begin Source File

SOURCE=.\scripts\tcltk.py
# End Source File
# Begin Source File

SOURCE=.\scripts\threads.py
# End Source File
# End Group
# Begin Group "doc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gpl.txt
# End Source File
# Begin Source File

SOURCE=.\license.txt
# End Source File
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Group
# Begin Source File

SOURCE=.\source\bound.cpp
# End Source File
# Begin Source File

SOURCE=.\source\clmeth.cpp
# End Source File
# Begin Source File

SOURCE=.\source\main.cpp
# End Source File
# Begin Source File

SOURCE=.\source\main.h
# End Source File
# Begin Source File

SOURCE=.\source\modmeth.cpp
# End Source File
# Begin Source File

SOURCE=.\source\py.cpp
# End Source File
# Begin Source File

SOURCE=.\source\pyargs.cpp
# End Source File
# Begin Source File

SOURCE=.\source\pyext.cpp
# End Source File
# Begin Source File

SOURCE=.\source\pyext.h
# End Source File
# Begin Source File

SOURCE=.\source\register.cpp
# End Source File
# End Target
# End Project
