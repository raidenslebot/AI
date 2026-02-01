@echo off
REM Raijin Test Gauntlet - build, self-test, regression replay
REM Runs automatically on every mutation; single-command bootstrap.

setlocal
set EXIT=0

echo ========================================
echo    R A I J I N   T E S T   G A U N T L E T
echo ========================================
echo.

echo [1/3] Build...
call build_raijin.bat
if errorlevel 1 (
    echo TEST GAUNTLET FAILED: Build failed.
    exit /b 1
)
echo.

echo [2/3] Self-test...
if not exist Bin\raijin.exe (
    echo TEST GAUNTLET FAILED: Bin\raijin.exe not found.
    exit /b 1
)
Bin\raijin.exe --self-test
if errorlevel 1 (
    echo TEST GAUNTLET FAILED: Self-test failed.
    exit /b 1
)
echo.

echo [3/3] Regression replay...
Bin\raijin.exe --regression-replay
if errorlevel 1 (
    echo TEST GAUNTLET FAILED: Regression replay failed.
    exit /b 1
)
echo.

echo ========================================
echo    T E S T   G A U N T L E T   P A S S E D
echo ========================================
exit /b 0
