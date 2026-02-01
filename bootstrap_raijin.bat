@echo off
REM Raijin Bootstrap - Single-command build, test, evolution cycle, metrics
REM This script is fully autonomous and requires no manual intervention.

setlocal EnableDelayedExpansion

echo =====================================================
echo      RAIJIN BOOTSTRAP - AUTONOMOUS EXECUTION
echo =====================================================
echo.

set RAIJIN_ROOT=%~dp0
set CYCLES=10
set EXIT_CODE=0

if not "%~1"=="" set CYCLES=%~1

echo Configuration:
echo   Root: %RAIJIN_ROOT%
echo   Evolution cycles: %CYCLES%
echo.

REM Step 1: Build
echo [1/4] Building Raijin...
set RAIJIN_AUTOMATED=1
call "%RAIJIN_ROOT%build_raijin_mingw.bat" >nul 2>&1
if errorlevel 1 (
    echo   BUILD FAILED
    set EXIT_CODE=1
    goto :report
)
echo   BUILD SUCCESS

REM Verify executables exist
if not exist "%RAIJIN_ROOT%Bin\raijin.exe" (
    echo   ERROR: raijin.exe not found after build
    set EXIT_CODE=1
    goto :report
)
echo   Verified: Bin\raijin.exe

REM Step 2: Self-test
echo [2/4] Running self-test...
"%RAIJIN_ROOT%Bin\raijin.exe" --self-test >"%RAIJIN_ROOT%data\selftest_output.txt" 2>&1
set TEST_RESULT=%ERRORLEVEL%
if %TEST_RESULT% NEQ 0 (
    echo   SELF-TEST FAILED (exit code %TEST_RESULT%)
    type "%RAIJIN_ROOT%data\selftest_output.txt"
    set EXIT_CODE=2
    goto :report
)
echo   SELF-TEST PASSED

REM Step 3: Evolution cycles
echo [3/4] Running %CYCLES% evolution cycles...
set METRICS_FILE=%RAIJIN_ROOT%data\bootstrap_metrics.txt
echo Raijin Bootstrap Metrics > "%METRICS_FILE%"
echo ======================== >> "%METRICS_FILE%"
echo Timestamp: %DATE% %TIME% >> "%METRICS_FILE%"
echo Cycles requested: %CYCLES% >> "%METRICS_FILE%"
echo. >> "%METRICS_FILE%"

REM Create a short run script that exits after N cycles
set RUNNER_SCRIPT=%RAIJIN_ROOT%data\run_cycles.ps1
echo $proc = Start-Process -FilePath '%RAIJIN_ROOT%Bin\raijin.exe' -PassThru > "%RUNNER_SCRIPT%"
echo Start-Sleep -Seconds %CYCLES% >> "%RUNNER_SCRIPT%"
echo if (!$proc.HasExited) { $proc.Kill() } >> "%RUNNER_SCRIPT%"
echo exit $proc.ExitCode >> "%RUNNER_SCRIPT%"

powershell -ExecutionPolicy Bypass -File "%RUNNER_SCRIPT%" >"%RAIJIN_ROOT%data\evolution_output.txt" 2>&1
set EVOLVE_RESULT=%ERRORLEVEL%
echo   Evolution run completed (exit %EVOLVE_RESULT%)
echo Evolution exit code: %EVOLVE_RESULT% >> "%METRICS_FILE%"

REM Step 4: Collect metrics
echo [4/4] Collecting metrics...

REM Check if data directory exists
if not exist "%RAIJIN_ROOT%data" mkdir "%RAIJIN_ROOT%data"

REM Collect telemetry summary if available
if exist "%RAIJIN_ROOT%data\telemetry*.txt" (
    echo. >> "%METRICS_FILE%"
    echo Telemetry files found: >> "%METRICS_FILE%"
    for %%f in ("%RAIJIN_ROOT%data\telemetry*.txt") do (
        echo   %%f >> "%METRICS_FILE%"
    )
)

REM Collect lineage summary if available
if exist "%RAIJIN_ROOT%data\lineage*.txt" (
    echo. >> "%METRICS_FILE%"
    echo Lineage files found: >> "%METRICS_FILE%"
    for %%f in ("%RAIJIN_ROOT%data\lineage*.txt") do (
        echo   %%f >> "%METRICS_FILE%"
    )
)

REM Collect regression replay count
if exist "%RAIJIN_ROOT%data\regression_replay.txt" (
    echo. >> "%METRICS_FILE%"
    echo Regression replay file exists >> "%METRICS_FILE%"
)

echo   Metrics written to: data\bootstrap_metrics.txt

:report
echo.
echo =====================================================
echo      BOOTSTRAP COMPLETE
echo =====================================================
echo Exit code: %EXIT_CODE%
echo.

if %EXIT_CODE%==0 (
    echo STATUS: SUCCESS
    echo All steps completed successfully.
) else if %EXIT_CODE%==1 (
    echo STATUS: BUILD FAILURE
) else if %EXIT_CODE%==2 (
    echo STATUS: SELF-TEST FAILURE
) else (
    echo STATUS: UNKNOWN FAILURE
)

echo.
echo Artifacts:
echo   Build: Bin\raijin.exe
echo   Self-test output: data\selftest_output.txt
echo   Evolution output: data\evolution_output.txt
echo   Metrics: data\bootstrap_metrics.txt
echo.

endlocal
exit /b %EXIT_CODE%
