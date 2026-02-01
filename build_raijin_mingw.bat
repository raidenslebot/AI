@echo off
REM Raijin Complete Build System - MinGW Version
REM Comprehensive compilation for all Raijin components using MinGW

echo ========================================
echo    R A I J I N   B U I L D   S Y S T E M
echo ========================================
echo MinGW Build Version
echo.

REM Set up MinGW environment
echo Setting up MinGW environment...

REM Check if MinGW is available
where gcc >nul 2>&1
if errorlevel 1 (
    echo ERROR: MinGW GCC not found in PATH.
    echo Please ensure MinGW is installed and in PATH.
    echo.
    pause
    exit /b 1
)

echo MinGW GCC found.
echo.

REM Create output directories (Bin per README)
if not exist Bin mkdir Bin
if not exist obj mkdir obj

echo Building Raijin components...
echo.

REM Set compiler flags for MinGW (Include for headers)
set CFLAGS=-c -O2 -Wall -D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -IInclude -ICore
set CXXFLAGS=%CFLAGS% -std=c++17 -IInclude -ICore
set LDFLAGS_BASE=-static -lkernel32 -luser32 -ladvapi32 -lws2_32 -lgdi32 -lole32 -ldxgi -ld3d11 -lwinhttp

echo [1/10] Compiling HAL (Hardware Abstraction Layer)...
g++.exe %CXXFLAGS% Core/HAL/hal_13700k.cpp -o obj/hal_13700k.o
if errorlevel 1 goto :build_error

echo [2/10] Compiling Hypervisor Layer...
gcc.exe %CFLAGS% Core/Hypervisor/hypervisor_layer.c -o obj/hypervisor_layer.o
if errorlevel 1 goto :build_error

echo [3/10] Compiling Neural Substrate...
g++.exe %CXXFLAGS% Core/Neural/neural_substrate.cpp -o obj/neural_substrate.o
if errorlevel 1 goto :build_error

echo [3b/10] Compiling Role Boundary...
g++.exe %CXXFLAGS% Core/RoleBoundary/role_boundary.cpp -o obj/role_boundary.o
if errorlevel 1 goto :build_error

echo [4/10] Compiling Ethics System...
g++.exe %CXXFLAGS% Core/Ethics/ethics_system.cpp -o obj/ethics_system.o
if errorlevel 1 goto :build_error

echo [5/10] Compiling Screen Control...
g++.exe %CXXFLAGS% Core/ScreenControl/screen_control.cpp -o obj/screen_control.o
if errorlevel 1 goto :build_error

echo [6/8] Compiling Internet Acquisition...
g++.exe %CXXFLAGS% Core/InternetAcquisition/internet_acquisition.cpp -o obj/internet_acquisition.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/InternetAcquisition/http_client.cpp -o obj/http_client.o
if errorlevel 1 goto :build_error

echo [7/10] Compiling Programming Domination...
g++.exe %CXXFLAGS% Core/ProgrammingDomination/programming_domination.cpp -o obj/programming_domination.o
if errorlevel 1 goto :build_error

echo [8/12] Compiling Autonomous Manager and Evolution Engine...
g++.exe %CXXFLAGS% Core/Autonomous/autonomous_manager.cpp -o obj/autonomous_manager.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/Evolution/evolution_engine.cpp -o obj/evolution_engine.o
if errorlevel 1 goto :build_error

echo [9/12] Compiling Training, Telemetry, Memory, SelfTest...
g++.exe %CXXFLAGS% Core/Training/training_pipeline.cpp -o obj/training_pipeline.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/Telemetry/telemetry.cpp -o obj/telemetry.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/Memory/long_term_memory.cpp -o obj/long_term_memory.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/SelfTest/self_test.cpp -o obj/self_test.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/DominanceMetrics/dominance_metrics.cpp -o obj/dominance_metrics.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/RegressionDetector/regression_detector.cpp -o obj/regression_detector.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/AnomalyDetector/anomaly_detector.cpp -o obj/anomaly_detector.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/LineageTracker/lineage_tracker.cpp -o obj/lineage_tracker.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/VersioningRollback/versioning_rollback.cpp -o obj/versioning_rollback.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/SelfHealing/self_healing.cpp -o obj/self_healing.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/Introspection/introspection_system.cpp -o obj/introspection_system.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/ResourceGovernor/resource_governor.cpp -o obj/resource_governor.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/StressTest/stress_test_framework.cpp -o obj/stress_test_framework.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/AdversarialStress/adversarial_stress.cpp -o obj/adversarial_stress.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/WorldModel/world_model.cpp -o obj/world_model.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/EpisodicMemory/episodic_memory.cpp -o obj/episodic_memory.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/Provenance/provenance.cpp -o obj/provenance.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/Curriculum/curriculum.cpp -o obj/curriculum.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/RedTeam/red_team.cpp -o obj/red_team.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/TaskOracle/task_oracle.cpp -o obj/task_oracle.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/Main/runtime_config.cpp -o obj/runtime_config.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/FitnessLedger/fitness_ledger.cpp -o obj/fitness_ledger.o
if errorlevel 1 goto :build_error
g++.exe %CXXFLAGS% Core/RegressionReplay/regression_replay.cpp -o obj/regression_replay.o
if errorlevel 1 goto :build_error

echo [11/12] Compiling Main Raijin Executable...
g++.exe %CXXFLAGS% Core/Main/raijin_main.cpp -o obj/raijin_main.o
if errorlevel 1 goto :build_error

echo [10/10] Compiling Dominate CLI...
g++.exe %CXXFLAGS% Core/Main/dominate_main.cpp -o obj/dominate_main.o
if errorlevel 1 goto :build_error

echo.
echo Linking raijin.exe...
g++.exe obj/hal_13700k.o obj/hypervisor_layer.o obj/neural_substrate.o obj/role_boundary.o obj/ethics_system.o obj/screen_control.o obj/internet_acquisition.o obj/http_client.o obj/programming_domination.o obj/autonomous_manager.o obj/evolution_engine.o obj/training_pipeline.o obj/telemetry.o obj/long_term_memory.o obj/self_test.o obj/dominance_metrics.o obj/regression_detector.o obj/anomaly_detector.o obj/lineage_tracker.o obj/versioning_rollback.o obj/self_healing.o obj/fitness_ledger.o obj/regression_replay.o obj/introspection_system.o obj/stress_test_framework.o obj/adversarial_stress.o obj/resource_governor.o obj/world_model.o obj/episodic_memory.o obj/provenance.o obj/curriculum.o obj/task_oracle.o obj/red_team.o obj/runtime_config.o obj/raijin_main.o -o Bin/raijin.exe %LDFLAGS_BASE% -lpsapi
if errorlevel 1 goto :build_error

echo Linking raijin-dominate.exe...
g++.exe obj/hal_13700k.o obj/hypervisor_layer.o obj/neural_substrate.o obj/role_boundary.o obj/ethics_system.o obj/screen_control.o obj/internet_acquisition.o obj/http_client.o obj/programming_domination.o obj/autonomous_manager.o obj/evolution_engine.o obj/dominate_main.o -o Bin/raijin-dominate.exe %LDFLAGS_BASE%
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

:build_error
echo.
echo ERROR: Build failed. Check the output above for details.
echo.
if "%RAIJIN_AUTOMATED%"=="" pause
exit /b 1

:end
echo.
echo Running test gauntlet (self-test + regression-replay)...
if not exist Bin\raijin.exe goto :gauntlet_skip
Bin\raijin.exe --self-test
if errorlevel 1 (
    echo ERROR: Self-test failed.
    goto :build_error
)
Bin\raijin.exe --regression-replay
if errorlevel 1 (
    echo ERROR: Regression replay failed.
    goto :build_error
)
echo Test gauntlet passed.
:gauntlet_skip
echo Build completed successfully.
if "%RAIJIN_AUTOMATED%"=="" pause