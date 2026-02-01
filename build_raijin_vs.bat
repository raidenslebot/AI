@echo off
REM Raijin Complete Build System
REM Comprehensive compilation for all Raijin components

echo ========================================
echo    R A I J I N   B U I L D   S Y S T E M
echo ========================================
echo.

REM Set up Visual Studio environment
echo Setting up Visual Studio 2022 environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 goto :no_vs

echo Visual Studio environment configured.
echo.

REM Create output directories (Bin per README)
if not exist Bin mkdir Bin
if not exist obj mkdir obj

echo Building Raijin components...
echo.

REM Set compiler flags (/IInclude for headers)
set CFLAGS=/c /O2 /MT /W3 /nologo /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN /IInclude /ICore
set CXXFLAGS=%CFLAGS% /std:c++17 /EHsc /IInclude /ICore
set LDFLAGS=/OUT:Bin\raijin.exe /SUBSYSTEM:CONSOLE /MACHINE:X64 kernel32.lib user32.lib advapi32.lib ws2_32.lib

echo [1/9] Compiling HAL (Hardware Abstraction Layer)...
cl.exe %CXXFLAGS% Core\HAL\hal_13700k.cpp /Fo:obj\hal_13700k.obj
if errorlevel 1 goto :build_error

echo [2/9] Compiling Hypervisor Layer...
cl.exe %CFLAGS% Core\Hypervisor\hypervisor_layer.c /Fo:obj\hypervisor_layer.obj
if errorlevel 1 goto :build_error

echo [3/9] Compiling Neural Substrate...
cl.exe %CXXFLAGS% Core\Neural\neural_substrate.cpp /Fo:obj\neural_substrate.obj
if errorlevel 1 goto :build_error

echo [4/9] Compiling Ethics System...
cl.exe %CXXFLAGS% Core\Ethics\ethics_system.cpp /Fo:obj\ethics_system.obj
if errorlevel 1 goto :build_error

echo [5/9] Compiling Screen Control...
cl.exe %CXXFLAGS% Core\ScreenControl\screen_control.cpp /Fo:obj\screen_control.obj
if errorlevel 1 goto :build_error

echo [6/9] Compiling Internet Acquisition...
cl.exe %CXXFLAGS% Core\InternetAcquisition\internet_acquisition.cpp /Fo:obj\internet_acquisition.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\InternetAcquisition\http_client.cpp /Fo:obj\http_client.obj
if errorlevel 1 goto :build_error

echo [7/10] Compiling Training Pipeline...
cl.exe %CXXFLAGS% Core\Training\training_pipeline.cpp /Fo:obj\training_pipeline.obj
if errorlevel 1 goto :build_error

echo [8/10] Compiling Programming Domination...
cl.exe %CXXFLAGS% Core\ProgrammingDomination\programming_domination.cpp /Fo:obj\programming_domination.obj
if errorlevel 1 goto :build_error

echo [8/12] Compiling Autonomous Manager and Evolution Engine...
cl.exe %CXXFLAGS% Core\Autonomous\autonomous_manager.cpp /Fo:obj\autonomous_manager.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\Evolution\evolution_engine.cpp /Fo:obj\evolution_engine.obj
if errorlevel 1 goto :build_error

echo [9/12] Compiling Training, Telemetry, Memory, SelfTest...
cl.exe %CXXFLAGS% Core\Training\training_pipeline.cpp /Fo:obj\training_pipeline.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\Telemetry\telemetry.cpp /Fo:obj\telemetry.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\Memory\long_term_memory.cpp /Fo:obj\long_term_memory.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\SelfTest\self_test.cpp /Fo:obj\self_test.obj
if errorlevel 1 goto :build_error

echo [10/20] Compiling Dominance Metrics, Regression Detector, Anomaly Detector...
cl.exe %CXXFLAGS% Core\DominanceMetrics\dominance_metrics.cpp /Fo:obj\dominance_metrics.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\RegressionDetector\regression_detector.cpp /Fo:obj\regression_detector.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\AnomalyDetector\anomaly_detector.cpp /Fo:obj\anomaly_detector.obj
if errorlevel 1 goto :build_error

echo [11/21] Compiling Lineage Tracker, Versioning Rollback, Self-Healing, Fitness Ledger...
cl.exe %CXXFLAGS% Core\LineageTracker\lineage_tracker.cpp /Fo:obj\lineage_tracker.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\VersioningRollback\versioning_rollback.cpp /Fo:obj\versioning_rollback.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\SelfHealing\self_healing.cpp /Fo:obj\self_healing.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\FitnessLedger\fitness_ledger.cpp /Fo:obj\fitness_ledger.obj
if errorlevel 1 goto :build_error

echo [12/21] Compiling World Model, Episodic Memory, Provenance, Curriculum, Red Team...
cl.exe %CXXFLAGS% Core\WorldModel\world_model.cpp /Fo:obj\world_model.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\EpisodicMemory\episodic_memory.cpp /Fo:obj\episodic_memory.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\Provenance\provenance.cpp /Fo:obj\provenance.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\Curriculum\curriculum.cpp /Fo:obj\curriculum.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\RedTeam\red_team.cpp /Fo:obj\red_team.obj
if errorlevel 1 goto :build_error

echo [13/22] Compiling Resource Governor, RoleBoundary, Introspection, Stress Test, Adversarial Stress...
cl.exe %CXXFLAGS% Core\ResourceGovernor\resource_governor.cpp /Fo:obj\resource_governor.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\RoleBoundary\role_boundary.cpp /Fo:obj\role_boundary.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\Introspection\introspection_system.cpp /Fo:obj\introspection_system.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\StressTest\stress_test_framework.cpp /Fo:obj\stress_test_framework.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\AdversarialStress\adversarial_stress.cpp /Fo:obj\adversarial_stress.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\TaskOracle\task_oracle.cpp /Fo:obj\task_oracle.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\RegressionReplay\regression_replay.cpp /Fo:obj\regression_replay.obj
if errorlevel 1 goto :build_error

echo [14/22] Compiling Main and Runtime Config...
cl.exe %CXXFLAGS% Core\Main\runtime_config.cpp /Fo:obj\runtime_config.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\Main\raijin_main.cpp /Fo:obj\raijin_main.obj
if errorlevel 1 goto :build_error
cl.exe %CXXFLAGS% Core\Main\dominate_main.cpp /Fo:obj\dominate_main.obj
if errorlevel 1 goto :build_error

echo.
echo Linking raijin.exe...
link.exe obj\hal_13700k.obj obj\hypervisor_layer.obj obj\neural_substrate.obj obj\ethics_system.obj obj\screen_control.obj obj\internet_acquisition.obj obj\http_client.obj obj\programming_domination.obj obj\autonomous_manager.obj obj\evolution_engine.obj obj\training_pipeline.obj obj\telemetry.obj obj\long_term_memory.obj obj\self_test.obj obj\dominance_metrics.obj obj\regression_detector.obj obj\anomaly_detector.obj obj\lineage_tracker.obj obj\versioning_rollback.obj obj\self_healing.obj obj\fitness_ledger.obj obj\regression_replay.obj obj\world_model.obj obj\episodic_memory.obj obj\provenance.obj obj\curriculum.obj obj\red_team.obj obj\resource_governor.obj obj\role_boundary.obj obj\task_oracle.obj obj\introspection_system.obj obj\stress_test_framework.obj obj\adversarial_stress.obj obj\runtime_config.obj obj\raijin_main.obj /OUT:Bin\raijin.exe /SUBSYSTEM:CONSOLE /MACHINE:X64 kernel32.lib user32.lib advapi32.lib ws2_32.lib psapi.lib
if errorlevel 1 goto :build_error

echo Linking raijin-dominate.exe...
link.exe obj\hal_13700k.obj obj\hypervisor_layer.obj obj\neural_substrate.obj obj\ethics_system.obj obj\screen_control.obj obj\internet_acquisition.obj obj\http_client.obj obj\training_pipeline.obj obj\programming_domination.obj obj\autonomous_manager.obj obj\evolution_engine.obj obj\dominance_metrics.obj obj\regression_detector.obj obj\anomaly_detector.obj obj\lineage_tracker.obj obj\versioning_rollback.obj obj\self_healing.obj obj\introspection_system.obj obj\stress_test_framework.obj obj\dominate_main.obj /OUT:Bin\raijin-dominate.exe /SUBSYSTEM:CONSOLE /MACHINE:X64 kernel32.lib user32.lib advapi32.lib ws2_32.lib
if errorlevel 1 goto :build_error

echo.
echo ========================================
echo    R A I J I N   B U I L D   S U C C E S S
echo ========================================
echo.
echo raijin.exe and raijin-dominate.exe have been created in Bin\
echo.
echo To run Raijin:
echo   Bin\raijin.exe
echo To run Programming Domination CLI:
echo   Bin\raijin-dominate.exe analyze "code" --lang python
echo   Bin\raijin-dominate.exe generate "description" --lang javascript
echo   Bin\raijin-dominate.exe stats
echo.
goto :end

:no_vs
echo.
echo ERROR: Visual Studio 2022 Community not found.
echo Please ensure Visual Studio 2022 Community is installed.
echo.
pause
exit /b 1

:build_error
echo.
echo ERROR: Build failed. Check the output above for details.
echo.
pause
exit /b 1

:end
echo Build completed successfully.
pause
