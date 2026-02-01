@echo off
REM Raijin Build Dispatcher - README-referenced entry point
REM Tries Visual Studio first, then MinGW

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call build_raijin_vs.bat
    exit /b %ERRORLEVEL%
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    call build_raijin_vs.bat
    exit /b %ERRORLEVEL%
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    call build_raijin_vs.bat
    exit /b %ERRORLEVEL%
)

where g++.exe >nul 2>&1
if %ERRORLEVEL% equ 0 (
    call build_raijin_mingw.bat
    exit /b %ERRORLEVEL%
)

echo ERROR: No C++ toolchain found. Install Visual Studio 2022 (Community/Professional/Enterprise)
echo with "Desktop development with C++" or MinGW with g++ in PATH.
exit /b 1
