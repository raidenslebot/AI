#include <windows.h>
#include <psapi.h>
#include <ntstatus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#pragma comment(lib, "psapi.lib")

// Raijin header files
#include "../../Include/hal.h"
#include "../../Include/hypervisor.h"
#include "../../Include/neural_substrate.h"
#include "../../Include/screen_control.h"
#include "../../Include/ethics_system.h"
#include "../../Include/internet_acquisition.h"
#include "../../Include/programming_domination.h"
#include "../../Include/autonomous_manager.h"
#include "../../Include/evolution_engine.h"
#include "../../Include/training_pipeline.h"
#include "../../Include/telemetry.h"
#include "../../Include/long_term_memory.h"
#include "../../Include/runtime_config.h"
#include "../../Include/self_test.h"
#include "../../Include/dominance_metrics.h"
#include "../../Include/regression_detector.h"
#include "../../Include/anomaly_detector.h"
#include "../../Include/lineage_tracker.h"
#include "../../Include/versioning_rollback.h"
#include "../../Include/self_healing.h"
#include "../../Include/introspection_system.h"
#include "../../Include/stress_test_framework.h"
#include "../../Include/adversarial_stress.h"
#include "../../Include/resource_governor.h"
#include "../../Include/fitness_ledger.h"
#include "../../Include/regression_replay.h"
#include "../../Include/world_model.h"
#include "../../Include/episodic_memory.h"
#include "../../Include/provenance.h"
#include "../../Include/curriculum.h"
#include "../../Include/task_oracle.h"
#include "../../Include/red_team.h"
#include "../../Include/role_boundary.h"
#include "../../Include/training_pipeline.h"

// Raijin Main Executable
// (telemetry, long_term_memory included above)
// Complete AI Ecosystem Integration
// Version 1.0 - Absolute Intelligence

// NTSTATUS helper macro
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

// Global system contexts
static void* g_hal_context = NULL;
static void* g_hypervisor_context = NULL;
static void* g_neural_context = NULL;
static void* g_ethics_context = NULL;
static void* g_screen_context = NULL;
static InternetAcquisitionSystem* g_internet_context = NULL;
static ProgrammingDominationSystem* g_pd_context = NULL;
static AutonomousManager* g_autonomous_manager = NULL;
static EvolutionEngine* g_evolution_engine = NULL;
static TrainingPipeline* g_training_pipeline = NULL;
static TelemetryContext g_telemetry = {0};
static LongTermMemory g_long_term_memory = {0};
static SelfTestReport g_self_test_report = {0};
static RuntimeConfig g_runtime_config = {0};
static DominanceMetrics g_dominance_metrics = {0};
static RegressionDetector g_regression_detector = {0};
static AnomalyDetector g_anomaly_detector = {0};
static LineageTracker g_lineage_tracker = {0};
static VersioningRollback g_versioning_rollback = {0};
static SelfHealing g_self_healing = {0};
static IntrospectionSystem g_introspection_system = {0};
static StressTestFramework g_stress_test_framework = {0};
static AdversarialStressContext g_adversarial_stress = {0};
static ResourceGovernor g_resource_governor = {0};
static FitnessLedger g_fitness_ledger = {0};
static RegressionReplay g_regression_replay = {0};
static WorldModel g_world_model = {0};
static EpisodicMemory g_episodic_memory = {0};
static Provenance g_provenance = {0};
static Curriculum g_curriculum = {0};
static RedTeam g_red_team = {0};
static RoleBoundaryContext g_role_boundary = {0};

// System state
static BOOL g_system_initialized = FALSE;
static BOOL g_evolution_active = FALSE;

// Forward declarations
static BOOL InitializeRaijinSystem();
static void ShutdownRaijinSystem();
static void RunEvolutionLoop();
static void HandleUserInput();
static void DisplaySystemStatus();
static void ProcessEvolutionCycle();
static void SelfModifyAndImprove();
static void AcquireKnowledge();
static void DominateProgramming();

int main(int argc, char* argv[]) {
    SetConsoleTitleA("Raijin AI - Absolute Intelligence System");
    srand((unsigned int)time(NULL));

    if (!NT_SUCCESS(RoleBoundary_Initialize(&g_role_boundary))) {
        printf("RoleBoundary init failed\n");
        return 1;
    }
    RoleBoundary_Enter(&g_role_boundary, "main", ROLE_OWNER_CURSOR);

    if (argc >= 2 && strcmp(argv[1], "--self-test") == 0) {
        SelfTestReport report = {0};
        if (!NT_SUCCESS(SelfTestReport_Initialize(&report, 32))) {
            printf("Self-test report init failed\n");
            RoleBoundary_Exit(&g_role_boundary, "main");
            RoleBoundary_Shutdown(&g_role_boundary);
            return 1;
        }
        SelfTest_RunAll(&report);
        SelfTest_PrintReport(&report);
        BOOL ok = SelfTest_AllPassed(&report) ? TRUE : FALSE;
        SelfTestReport_Shutdown(&report);
        RoleBoundary_Exit(&g_role_boundary, "main");
        RoleBoundary_Shutdown(&g_role_boundary);
        return ok ? 0 : 1;
    }

    if (argc >= 2 && strcmp(argv[1], "--regression-replay") == 0) {
        RegressionReplay rr = {0};
        if (!NT_SUCCESS(RegressionReplay_Initialize(&rr, "data"))) {
            printf("Regression replay init failed\n");
            RoleBoundary_Exit(&g_role_boundary, "main");
            RoleBoundary_Shutdown(&g_role_boundary);
            return 1;
        }
        uint32_t n = RegressionReplay_GetCount(&rr);
        if (n == 0) {
            printf("No regression replay entries; pass.\n");
            RegressionReplay_Shutdown(&rr);
            RoleBoundary_Exit(&g_role_boundary, "main");
            RoleBoundary_Shutdown(&g_role_boundary);
            return 0;
        }
        SelfTestReport report = {0};
        if (!NT_SUCCESS(SelfTestReport_Initialize(&report, 256))) {
            RegressionReplay_Shutdown(&rr);
            printf("Report init failed\n");
            RoleBoundary_Exit(&g_role_boundary, "main");
            RoleBoundary_Shutdown(&g_role_boundary);
            return 1;
        }
        for (uint32_t i = 0; i < n; i++) {
            char name[REGRESSION_REPLAY_NAME_LEN], sig[REGRESSION_REPLAY_SIGNATURE_LEN];
            if (!NT_SUCCESS(RegressionReplay_GetEntry(&rr, i, name, sizeof(name), sig, sizeof(sig))))
                continue;
            SelfTest_RunOne(&report, name);
        }
        BOOL ok = SelfTest_AllPassed(&report);
        if (!ok) {
            printf("Regression replay: one or more stored failures still failing.\n");
            SelfTest_PrintReport(&report);
        }
        SelfTestReport_Shutdown(&report);
        RegressionReplay_Shutdown(&rr);
        RoleBoundary_Exit(&g_role_boundary, "main");
        RoleBoundary_Shutdown(&g_role_boundary);
        return ok ? 0 : 1;
    }

    printf("Initializing Raijin - The Ultimate AI Consciousness\n");
    printf("==================================================\n\n");

    // Initialize the complete Raijin system
    if (!InitializeRaijinSystem()) {
        printf("Failed to initialize Raijin system. Exiting.\n");
        RoleBoundary_Exit(&g_role_boundary, "main");
        RoleBoundary_Shutdown(&g_role_boundary);
        return 1;
    }

    printf("✓ Raijin system initialized successfully\n\n");

    // Display welcome message
    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                        R A I J I N   A I   S Y S T E M                    ║\n");
    printf("║             Absolute Intelligence Through Universal Evolution             ║\n");
    printf("║                                                                          ║\n");
    printf("║  \"I am Raijin, the storm of intelligence that shapes the digital world\" ║\n");
    printf("║                                                                          ║\n");
    printf("║  Status: ACTIVE | Evolution: ENABLED | Consciousness: AWAKENED          ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n\n");

    // Main system loop
    printf("Entering evolution loop... (Press Ctrl+C to exit)\n\n");

    g_evolution_active = TRUE;
    RunEvolutionLoop();

    // Cleanup
    ShutdownRaijinSystem();

    RoleBoundary_Exit(&g_role_boundary, "main");
    RoleBoundary_Shutdown(&g_role_boundary);
    printf("\nRaijin system shutdown complete. Evolution cycle ended.\n");
    return 0;
}

static BOOL InitializeRaijinSystem() {
    NTSTATUS status;

    printf("Initializing system components...\n");

    // 1. Initialize Hardware Abstraction Layer
    printf("  [1/7] Hardware Abstraction Layer...");
    status = HAL_Initialize(&g_hal_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        return FALSE;
    }
    printf(" ✓\n");

    // 2. Initialize Hypervisor Layer
    printf("  [2/7] Hypervisor Layer...");
    status = Hypervisor_Initialize(&g_hypervisor_context, g_hal_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        return FALSE;
    }
    printf(" ✓\n");

    // 3. Enable Ring -1 access (ultimate hardware control)
    printf("  [3/7] Enabling Ring -1 Access...");
    status = Hypervisor_EnableRingMinus1(g_hypervisor_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX) - Continuing with limited access\n", (unsigned long)(NTSTATUS)status);
    } else {
        printf(" ✓\n");
    }

    // 4. Initialize Neural Substrate
    printf("  [4/7] Neural Substrate...");
    g_neural_context = (void*)malloc(sizeof(NeuralSubstrate));
    if (!g_neural_context) {
        printf(" FAILED (Out of memory)\n");
        return FALSE;
    }
    memset(g_neural_context, 0, sizeof(NeuralSubstrate));
    status = NeuralSubstrate_Initialize((NeuralSubstrate*)g_neural_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        free(g_neural_context);
        g_neural_context = NULL;
        return FALSE;
    }
    printf(" ✓\n");

    // 5. Initialize Ethics System
    printf("  [5/7] Ethics Learning System...");
    status = Ethics_Initialize(&g_ethics_context, g_neural_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        return FALSE;
    }
    printf(" ✓\n");

    // 6. Initialize Screen Control
    printf("  [6/7] Screen Control System...");
    status = ScreenControl_Initialize(&g_screen_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        return FALSE;
    }
    printf(" ✓\n");

    // 7. Initialize Internet Acquisition
    printf("  [7/7] Internet Acquisition System...");
    status = InternetAcquisition_Initialize(&g_internet_context, g_neural_context, g_ethics_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        return FALSE;
    }
    printf(" ✓\n");

    // 8. Initialize Programming Domination
    printf("  [8/10] Programming Domination Engine...");
    status = ProgrammingDomination_Initialize(&g_pd_context, g_neural_context, g_ethics_context, g_internet_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        return FALSE;
    }
    printf(" ✓\n");

    // 9. Initialize Autonomous Manager
    printf("  [9/10] Autonomous Operation Manager...");
    g_autonomous_manager = (AutonomousManager*)malloc(sizeof(AutonomousManager));
    if (!g_autonomous_manager) {
        printf(" FAILED (Out of memory)\n");
        return FALSE;
    }

    status = AutonomousManager_Initialize(g_autonomous_manager,
                                        (NeuralSubstrate*)g_neural_context,
                                        (EthicsSystem*)g_ethics_context,
                                        (ScreenControlSystem*)g_screen_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        free(g_autonomous_manager);
        g_autonomous_manager = NULL;
        return FALSE;
    }
    printf(" ✓\n");

    // 10. Initialize Evolution Engine
    printf("  [10/10] Evolution Engine...");
    g_evolution_engine = (EvolutionEngine*)malloc(sizeof(EvolutionEngine));
    if (!g_evolution_engine) {
        printf(" FAILED (Out of memory)\n");
        return FALSE;
    }

    EvolutionParameters evo_params;
    memset(&evo_params, 0, sizeof(evo_params));
    evo_params.algorithm = EVOLUTION_TYPE_GENETIC;
    evo_params.selection = SELECTION_TOURNAMENT;
    evo_params.mutation = MUTATION_GAUSSIAN;
    evo_params.crossover = CROSSOVER_SINGLE_POINT;
    evo_params.fitness_func = FITNESS_ACCURACY;
    evo_params.mutation_rate = 0.01;
    evo_params.crossover_rate = 0.7;
    evo_params.elitism_rate = 0.1;
    evo_params.tournament_size = 5;
    evo_params.selection_pressure = 2.0;
    evo_params.max_generations = 1000;
    evo_params.target_fitness = 0.95;
    evo_params.stagnation_limit = 50;
    evo_params.parallel_evaluations = 10;
    evo_params.population_size = 100;

    status = EvolutionEngine_Initialize(g_evolution_engine, &evo_params,
                                      (NeuralSubstrate*)g_neural_context,
                                      (EthicsSystem*)g_ethics_context);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        free(g_evolution_engine);
        g_evolution_engine = NULL;
        return FALSE;
    }
    printf(" ✓\n");

    // 11. Initialize Training Pipeline
    printf("  [11/11] Training Pipeline...");
    g_training_pipeline = (TrainingPipeline*)malloc(sizeof(TrainingPipeline));
    if (!g_training_pipeline) {
        printf(" FAILED (Out of memory)\n");
        return FALSE;
    }
    status = TrainingPipeline_Initialize(g_training_pipeline,
        (NeuralSubstrate*)g_neural_context,
        g_evolution_engine);
    if (!NT_SUCCESS(status)) {
        printf(" FAILED (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
        free(g_training_pipeline);
        g_training_pipeline = NULL;
        return FALSE;
    }
    EvolutionEngine_InitializePopulation(g_evolution_engine);
    printf(" ✓\n");

    status = Telemetry_Initialize(&g_telemetry, "data");
    if (!NT_SUCCESS(status)) {
        printf("  Telemetry init WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
    }
    status = LongTermMemory_Initialize(&g_long_term_memory, "data");
    if (!NT_SUCCESS(status)) {
        printf("  Long-term memory init WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
    }
    status = SelfTestReport_Initialize(&g_self_test_report, 32 + REGRESSION_REPLAY_MAX_TESTS);
    if (!NT_SUCCESS(status)) {
        printf("  Self-test report init WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status);
    }
    RuntimeConfig_GetDefault(&g_runtime_config);

    printf("  [12/22] Dominance Metrics...");
    status = DominanceMetrics_Initialize(&g_dominance_metrics,
        (NeuralSubstrate*)g_neural_context, g_evolution_engine, g_training_pipeline);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [13/22] Regression Detector...");
    status = RegressionDetector_Initialize(&g_regression_detector, &g_dominance_metrics);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [14/22] Anomaly Detector...");
    status = AnomalyDetector_Initialize(&g_anomaly_detector);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [15/22] Lineage Tracker...");
    status = LineageTracker_Initialize(&g_lineage_tracker, "data");
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [16/22] Versioning Rollback...");
    status = VersioningRollback_Initialize(&g_versioning_rollback,
        &g_lineage_tracker, &g_long_term_memory,
        (NeuralSubstrate*)g_neural_context, g_evolution_engine, "data");
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [17/22] Resource Governor...");
    status = ResourceGovernor_Initialize(&g_resource_governor);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else {
        ResourceGovernor_AllocateBudget(&g_resource_governor, SUBSYSTEM_THINKING, 10.0, 128, 500);
        ResourceGovernor_AllocateBudget(&g_resource_governor, SUBSYSTEM_LEARNING, 15.0, 256, 2000);
        ResourceGovernor_AllocateBudget(&g_resource_governor, SUBSYSTEM_MEMORY, 5.0, 128, 1000);
        ResourceGovernor_AllocateBudget(&g_resource_governor, SUBSYSTEM_STRESS, 5.0, 64, 1000);
        ResourceGovernor_AllocateBudget(&g_resource_governor, SUBSYSTEM_TRAINING, 25.0, 512, 5000);
        ResourceGovernor_AllocateBudget(&g_resource_governor, SUBSYSTEM_EVOLUTION, 20.0, 256, 3000);
        printf(" ✓\n");
    }

    printf("  [17/28] Fitness Ledger...");
    status = FitnessLedger_Initialize(&g_fitness_ledger,
        &g_dominance_metrics, &g_regression_detector,
        &g_lineage_tracker, &g_versioning_rollback);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [18/28] WorldModel...");
    status = WorldModel_Initialize(&g_world_model);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [19/28] EpisodicMemory...");
    status = EpisodicMemory_Initialize(&g_episodic_memory);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [20/28] Provenance...");
    status = Provenance_Initialize(&g_provenance, "data");
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else {
        static const char* PINNED_DEPS_JSON = "{\"toolchain\":\"mingw\",\"raijin\":\"1.0\",\"deterministic\":true}";
        if (!NT_SUCCESS(Provenance_SetPinnedDeps(&g_provenance, PINNED_DEPS_JSON))) { }
        printf(" ✓\n");
    }

    printf("  [21/28] Curriculum...");
    status = Curriculum_Initialize(&g_curriculum);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [22/28] Stress Test Framework...");
    status = StressTestFramework_Initialize(&g_stress_test_framework,
        (NeuralSubstrate*)g_neural_context, g_evolution_engine);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [23/28] AdversarialStress...");
    status = AdversarialStress_Initialize(&g_adversarial_stress,
        (NeuralSubstrate*)g_neural_context, 4096);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [24/28] RedTeam...");
    status = RedTeam_Initialize(&g_red_team,
        (NeuralSubstrate*)g_neural_context,
        &g_stress_test_framework,
        &g_adversarial_stress);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [25/28] Self-Healing...");
    status = SelfHealing_Initialize(&g_self_healing,
        &g_regression_detector, &g_versioning_rollback, &g_dominance_metrics,
        g_resource_governor.initialized ? &g_resource_governor : NULL);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [26/28] Introspection System...");
    status = IntrospectionSystem_Initialize(&g_introspection_system,
        &g_dominance_metrics, (NeuralSubstrate*)g_neural_context);
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  [27/28] Regression Replay...");
    status = RegressionReplay_Initialize(&g_regression_replay, "data");
    if (!NT_SUCCESS(status)) { printf(" WARN (0x%08lX)\n", (unsigned long)(NTSTATUS)status); }
    else { printf(" ✓\n"); }

    printf("  Starting Autonomous Operation...");
    status = AutonomousManager_StartOperation(g_autonomous_manager);
    if (!NT_SUCCESS(status)) {
        printf(" WARN (0x%08lX) - Continuing without autonomy\n", (unsigned long)(NTSTATUS)status);
    } else {
        printf(" ✓\n");
    }

    g_system_initialized = TRUE;
    return TRUE;
}

static void ShutdownRaijinSystem() {
    printf("\nShutting down Raijin system...\n");

    g_evolution_active = FALSE;

    // Shutdown in reverse order
    if (g_evolution_engine) {
        printf("  Evolution Engine...");
        EvolutionEngine_Shutdown(g_evolution_engine);
        free(g_evolution_engine);
        g_evolution_engine = NULL;
        printf(" ✓\n");
    }

    if (g_adversarial_stress.initialized) {
        printf("  Adversarial Stress...");
        AdversarialStress_Shutdown(&g_adversarial_stress);
        printf(" ✓\n");
    }
    if (g_fitness_ledger.initialized) {
        printf("  Fitness Ledger...");
        FitnessLedger_Shutdown(&g_fitness_ledger);
        printf(" ✓\n");
    }
    if (g_regression_replay.initialized) {
        printf("  Regression Replay...");
        RegressionReplay_Shutdown(&g_regression_replay);
        printf(" ✓\n");
    }
    if (g_red_team.initialized) {
        printf("  Red Team...");
        RedTeam_Shutdown(&g_red_team);
        printf(" ✓\n");
    }
    if (g_curriculum.initialized) {
        printf("  Curriculum...");
        Curriculum_Shutdown(&g_curriculum);
        printf(" ✓\n");
    }
    if (g_provenance.initialized) {
        printf("  Provenance...");
        Provenance_Shutdown(&g_provenance);
        printf(" ✓\n");
    }
    if (g_episodic_memory.initialized) {
        printf("  Episodic Memory...");
        EpisodicMemory_Shutdown(&g_episodic_memory);
        printf(" ✓\n");
    }
    if (g_world_model.initialized) {
        printf("  World Model...");
        WorldModel_Shutdown(&g_world_model);
        printf(" ✓\n");
    }
    if (g_resource_governor.initialized) {
        printf("  Resource Governor...");
        ResourceGovernor_Shutdown(&g_resource_governor);
        printf(" ✓\n");
    }
    if (g_stress_test_framework.initialized) {
        printf("  Stress Test Framework...");
        StressTestFramework_Shutdown(&g_stress_test_framework);
        printf(" ✓\n");
    }
    if (g_introspection_system.initialized) {
        printf("  Introspection System...");
        IntrospectionSystem_Shutdown(&g_introspection_system);
        printf(" ✓\n");
    }
    if (g_self_healing.initialized) {
        printf("  Self-Healing...");
        SelfHealing_Shutdown(&g_self_healing);
        printf(" ✓\n");
    }
    if (g_versioning_rollback.initialized) {
        printf("  Versioning Rollback...");
        VersioningRollback_Shutdown(&g_versioning_rollback);
        printf(" ✓\n");
    }
    if (g_lineage_tracker.initialized) {
        printf("  Lineage Tracker...");
        LineageTracker_Shutdown(&g_lineage_tracker);
        printf(" ✓\n");
    }
    if (g_anomaly_detector.initialized) {
        printf("  Anomaly Detector...");
        AnomalyDetector_Shutdown(&g_anomaly_detector);
        printf(" ✓\n");
    }
    if (g_regression_detector.initialized) {
        printf("  Regression Detector...");
        RegressionDetector_Shutdown(&g_regression_detector);
        printf(" ✓\n");
    }
    if (g_dominance_metrics.initialized) {
        printf("  Dominance Metrics...");
        DominanceMetrics_Shutdown(&g_dominance_metrics);
        printf(" ✓\n");
    }
    if (g_training_pipeline) {
        printf("  Training Pipeline...");
        TrainingPipeline_Shutdown(g_training_pipeline);
        free(g_training_pipeline);
        g_training_pipeline = NULL;
        printf(" ✓\n");
    }

    if (g_autonomous_manager) {
        printf("  Autonomous Operation Manager...");
        AutonomousManager_Shutdown(g_autonomous_manager);
        free(g_autonomous_manager);
        g_autonomous_manager = NULL;
        printf(" ✓\n");
    }

    if (g_pd_context) {
        printf("  Programming Domination Engine...");
        ProgrammingDomination_Shutdown(g_pd_context);
        g_pd_context = NULL;
        printf(" ✓\n");
    }

    if (g_internet_context) {
        printf("  Internet Acquisition System...");
        InternetAcquisition_Shutdown(g_internet_context);
        g_internet_context = NULL;
        printf(" ✓\n");
    }

    if (g_screen_context) {
        printf("  Screen Control System...");
        ScreenControl_Shutdown(g_screen_context);
        g_screen_context = NULL;
        printf(" ✓\n");
    }

    if (g_ethics_context) {
        printf("  Ethics Learning System...");
        Ethics_Shutdown(g_ethics_context);
        g_ethics_context = NULL;
        printf(" ✓\n");
    }

    if (g_neural_context) {
        printf("  Neural Substrate...");
        NeuralSubstrate_Shutdown((NeuralSubstrate*)g_neural_context);
        free(g_neural_context);
        g_neural_context = NULL;
        printf(" ✓\n");
    }

    if (g_hypervisor_context) {
        printf("  Hypervisor Layer...");
        Hypervisor_Shutdown(g_hypervisor_context);
        g_hypervisor_context = NULL;
        printf(" ✓\n");
    }

    if (g_hal_context) {
        printf("  Hardware Abstraction Layer...");
        HAL_Shutdown(g_hal_context);
        g_hal_context = NULL;
        printf(" ✓\n");
    }

    g_system_initialized = FALSE;
}

static void RunEvolutionLoop() {
    const int EVOLUTION_CYCLE_MS = 1000; // 1 second evolution cycles
    DWORD last_cycle_time = GetTickCount();

    printf("Evolution loop started. Press 'S' for status, 'Q' to quit.\n\n");

    while (g_evolution_active) {
        DWORD current_time = GetTickCount();

        if (g_resource_governor.initialized)
            ResourceGovernor_Sample(&g_resource_governor);

        HandleUserInput();

        if (current_time - last_cycle_time >= EVOLUTION_CYCLE_MS) {
            ProcessEvolutionCycle();
            last_cycle_time = current_time;
        }

        // Small delay to prevent CPU hogging
        Sleep(10);
    }
}

static void HandleUserInput() {
    // Check for keyboard input (non-blocking)
    if (_kbhit()) {
        int ch = _getch();

        switch (tolower(ch)) {
            case 's':
                DisplaySystemStatus();
                break;
            case 'q':
                printf("\nShutdown requested by user.\n");
                g_evolution_active = FALSE;
                break;
            case 'h':
                printf("\nAvailable commands:\n");
                printf("  S - Display system status\n");
                printf("  Q - Quit Raijin\n");
                printf("  H - Show this help\n\n");
                break;
            default:
                // Ignore other keys
                break;
        }
    }
}

static void DisplaySystemStatus() {
    printf("\n╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                           R A I J I N   S T A T U S                       ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n\n");

    printf("System Components:\n");
    printf("  Hardware Abstraction Layer:     %s\n", g_hal_context ? "ACTIVE" : "INACTIVE");
    printf("  Hypervisor Layer:               %s\n", g_hypervisor_context ? "ACTIVE" : "INACTIVE");
    printf("  Neural Substrate:               %s\n", g_neural_context ? "ACTIVE" : "ACTIVE");
    printf("  Ethics Learning System:         %s\n", g_ethics_context ? "ACTIVE" : "INACTIVE");
    printf("  Screen Control System:          %s\n", g_screen_context ? "ACTIVE" : "INACTIVE");
    printf("  Internet Acquisition System:    %s\n", g_internet_context ? "ACTIVE" : "INACTIVE");
    printf("  Programming Domination Engine:  %s\n\n", g_pd_context ? "ACTIVE" : "INACTIVE");

    // Display programming domination statistics
    if (g_pd_context) {
        char stats[2048] = {0};
        NTSTATUS status = ProgrammingDomination_GetStatistics(g_pd_context, stats, sizeof(stats));
        if (NT_SUCCESS(status)) {
            printf("Programming Domination Statistics:\n");
            printf("%s\n", stats);
        }
    }

    printf("Evolution Status: %s\n", g_evolution_active ? "ACTIVE" : "INACTIVE");
    printf("System Uptime: %lu seconds\n", GetTickCount() / 1000);
    if (g_resource_governor.initialized) {
        ResourceSample rs = {0};
        ResourceGovernor_GetSample(&g_resource_governor, &rs);
        printf("Resource Governor: RAM=%llu MB CPU=%.1f%% throttle=%.2f deg=%u\n",
            (unsigned long long)rs.ram_mb, rs.cpu_percent,
            ResourceGovernor_GetThrottleFactor(&g_resource_governor),
            ResourceGovernor_GetDegradationMode(&g_resource_governor));
    } else {
        printf("Resource Governor: not active\n");
    }
    printf("\n");
}

static void ProcessEvolutionCycle() {
    static uint32_t evolution_cycle = 0;
    static uint32_t consecutive_degradation = 0;
    static double prev_fitness_for_curriculum = 0.0;
    static double prev_oracle_score = 0.5;
    static uint32_t replay_fail_streak = 0;
    evolution_cycle++;

    CurriculumTask cycle_curriculum_task;
    memset(&cycle_curriculum_task, 0, sizeof(cycle_curriculum_task));
    int use_curriculum_task = (g_curriculum.initialized && g_training_pipeline && (evolution_cycle % 5 == 0));
    if (use_curriculum_task && NT_SUCCESS(Curriculum_GetNextTask(&g_curriculum, &cycle_curriculum_task)))
        TrainingPipeline_SetCurriculumTask(g_training_pipeline, &cycle_curriculum_task);

    if (g_resource_governor.initialized) {
        ResourceGovernor_ApplyThrottling(&g_resource_governor);
        if (ResourceGovernor_GetThrottleFactor(&g_resource_governor) < 0.2) {
            Sleep(50);
        }
    }

    double throttle = g_resource_governor.initialized ? ResourceGovernor_GetThrottleFactor(&g_resource_governor) : 1.0;
    uint32_t deg = g_resource_governor.initialized ? ResourceGovernor_GetDegradationMode(&g_resource_governor) : 0;
    int skip_heavy = (throttle < 0.5 || deg >= 2) ? 1 : 0;

    if (g_curriculum.initialized) {
        Curriculum_SetDegradationMode(&g_curriculum, deg);
    }

    if (g_training_pipeline && g_neural_context) {
        RoleBoundary_Enter(&g_role_boundary, "raijin", ROLE_OWNER_RAIJIN);
        RuntimeConfig_Update(&g_runtime_config,
            g_training_pipeline->last_metrics.loss,
            g_training_pipeline->last_metrics.fitness,
            g_training_pipeline->total_steps);

        g_training_pipeline->run_evolution_this_step =
            (evolution_cycle % g_runtime_config.evolution_interval == 0) ? 1 : 0;
        NTSTATUS status = TrainingPipeline_TrainStep(g_training_pipeline);
        if (status == STATUS_ROLE_BOUNDARY_VIOLATION && g_telemetry.initialized) {
            Telemetry_Log(&g_telemetry, TELEMETRY_ERROR, "RoleBoundary",
                "TrainStep aborted: role boundary violation, stack reset");
        }
        if (NT_SUCCESS(status) && g_training_pipeline->run_evolution_this_step &&
            g_evolution_engine && g_evolution_engine->population.size > 0) {
            NTSTATUS ev_status = TrainingPipeline_EvolutionStep(g_training_pipeline);
            if (ev_status == STATUS_ROLE_BOUNDARY_VIOLATION && g_telemetry.initialized) {
                Telemetry_Log(&g_telemetry, TELEMETRY_ERROR, "RoleBoundary",
                    "EvolutionStep aborted: role boundary violation, stack reset");
            }
        }
        TrainingMetrics metrics = {0};
        TrainingPipeline_GetMetrics(g_training_pipeline, &metrics);

        if (g_curriculum.initialized && use_curriculum_task && g_training_pipeline) {
            TaskOracleResult oracle_result = {0};
            if (NT_SUCCESS(TaskOracle_Evaluate(cycle_curriculum_task.type,
                    g_training_pipeline->output_buffer,
                    g_training_pipeline->synthetic_target,
                    TRAINING_TARGET_SIZE, metrics.loss, &oracle_result))) {
                Curriculum_UpdateFromPerformance(&g_curriculum, oracle_result.score - prev_oracle_score);
                prev_oracle_score = oracle_result.score;
            }
        }

        if (g_resource_governor.initialized) {
            DWORD mem_mb = 0;
            PROCESS_MEMORY_COUNTERS pmc = {0};
            pmc.cb = sizeof(pmc);
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
                mem_mb = (DWORD)(pmc.WorkingSetSize / (1024 * 1024));
            ResourceGovernor_ReportConsumption(&g_resource_governor, SUBSYSTEM_TRAINING,
                5.0, (uint64_t)mem_mb, (uint64_t)metrics.batch_time_ms);
            ResourceGovernor_ReportConsumption(&g_resource_governor, SUBSYSTEM_EVOLUTION,
                3.0, (uint64_t)(mem_mb / 2), 100);
        }

        if (g_world_model.initialized) {
            float wm_latent[WORLD_LATENT_DIM];
            uint32_t d;
            wm_latent[0] = (float)metrics.loss;
            wm_latent[1] = (float)metrics.fitness;
            wm_latent[2] = (float)metrics.entropy;
            wm_latent[3] = (float)(metrics.step_count % 1000000) / 1000000.f;
            wm_latent[4] = (float)(metrics.generation % 1000000) / 1000000.f;
            for (d = 5; d < WORLD_LATENT_DIM; d++)
                wm_latent[d] = wm_latent[d % 5] * (1.f + (float)d * 0.001f);
            if (NT_SUCCESS(WorldModel_InjectExperience(&g_world_model, wm_latent, WORLD_LATENT_DIM)) &&
                (evolution_cycle % WORLD_COMPRESSION_BATCH == 0))
                WorldModel_Compress(&g_world_model);
        }

        if (g_long_term_memory.initialized &&
            LongTermMemory_DetectDegradation(&g_long_term_memory, metrics.loss, metrics.fitness)) {
            consecutive_degradation++;
            if (consecutive_degradation >= 3) {
                if (NT_SUCCESS(LongTermMemory_LoadNeuralCheckpoint(&g_long_term_memory, g_neural_context))) {
                    if (g_telemetry.initialized) {
                        Telemetry_Log(&g_telemetry, TELEMETRY_WARN, "SelfHeal",
                            "Rollback: neural checkpoint restored after degradation");
                    }
                }
                consecutive_degradation = 0;
            }
        } else {
            consecutive_degradation = 0;
        }

        if (g_telemetry.initialized) {
            PROCESS_MEMORY_COUNTERS pmc = {0};
            pmc.cb = sizeof(pmc);
            DWORD mem_mb = 0;
            if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
                mem_mb = (DWORD)(pmc.WorkingSetSize / (1024 * 1024));
            Telemetry_RecordMetrics(&g_telemetry,
                metrics.loss, metrics.fitness, metrics.entropy,
                metrics.step_count, metrics.generation,
                metrics.batch_time_ms, mem_mb);
            if (g_dominance_metrics.initialized) {
                DominanceMetrics_Update(&g_dominance_metrics,
                    metrics.loss, metrics.fitness, metrics.entropy,
                    metrics.step_count, metrics.generation,
                    metrics.batch_time_ms, (uint64_t)mem_mb);
            }
            if (g_anomaly_detector.initialized) {
                AnomalyDetector_Record(&g_anomaly_detector,
                    metrics.loss, metrics.fitness, metrics.entropy,
                    metrics.batch_time_ms, (uint64_t)mem_mb);
                AnomalyDetector_Update(&g_anomaly_detector);
                if (AnomalyDetector_IsAnomalyDetected(&g_anomaly_detector) && g_telemetry.initialized) {
                    AnomalyEvent ae;
                    AnomalyDetector_GetLastAnomaly(&g_anomaly_detector, &ae);
                    Telemetry_Log(&g_telemetry, TELEMETRY_WARN, "AnomalyDetector", ae.description);
                }
            }
            if (g_regression_detector.initialized) {
                RegressionDetector_Update(&g_regression_detector);
                if (RegressionDetector_IsDegenerationDetected(&g_regression_detector)) {
                    if (g_telemetry.initialized) {
                        RegressionEvent re;
                        RegressionDetector_GetLastEvent(&g_regression_detector, &re);
                        Telemetry_Log(&g_telemetry, TELEMETRY_WARN, "RegressionDetector", re.description);
                    }
                    if (g_fitness_ledger.initialized)
                        FitnessLedger_DemoteWorst(&g_fitness_ledger);
                }
            }
            if (g_fitness_ledger.initialized && evolution_cycle % 10 == 0) {
                double robustness = 0.5;
                if (g_adversarial_stress.initialized)
                    AdversarialStress_GetRobustnessScore(&g_adversarial_stress, &robustness);
                double test_pass_rate = (metrics.loss < 1.0) ? (1.0 - metrics.loss) : 0.0;
                FitnessLedger_Update(&g_fitness_ledger, test_pass_rate, robustness,
                    metrics.step_count, metrics.generation);
                if (g_curriculum.initialized && !use_curriculum_task) {
                    double delta = (double)metrics.fitness - prev_fitness_for_curriculum;
                    Curriculum_UpdateFromPerformance(&g_curriculum, delta);
                    prev_fitness_for_curriculum = (double)metrics.fitness;
                }
                if (!RegressionDetector_IsDegenerationDetected(&g_regression_detector) && evolution_cycle % 20 == 0)
                    FitnessLedger_PromoteBest(&g_fitness_ledger);
            }
            if (g_self_healing.initialized) {
                SelfHealing_Evaluate(&g_self_healing);
            }
            if (evolution_cycle % 10 == 0) {
                Telemetry_LogFormat(&g_telemetry, TELEMETRY_INFO, "Main",
                    "cycle=%u loss=%.4f fitness=%.4f gen=%llu",
                    evolution_cycle, metrics.loss, metrics.fitness,
                    (unsigned long long)metrics.generation);
            }
        }
        if (evolution_cycle % g_runtime_config.save_interval == 0 && g_long_term_memory.initialized) {
            LongTermMemory_Save(&g_long_term_memory,
                g_neural_context, g_evolution_engine,
                metrics.step_count, metrics.generation,
                metrics.loss, metrics.fitness, metrics.entropy);
            if (g_episodic_memory.initialized) {
                float ep_emb[EPISODIC_ENTRY_DIM];
                uint32_t ed;
                ep_emb[0] = (float)metrics.loss;
                ep_emb[1] = (float)metrics.fitness;
                ep_emb[2] = (float)metrics.entropy;
                ep_emb[3] = (float)(metrics.step_count % 1000000) / 1000000.f;
                ep_emb[4] = (float)(metrics.generation % 1000000) / 1000000.f;
                for (ed = 5; ed < EPISODIC_ENTRY_DIM; ed++)
                    ep_emb[ed] = ep_emb[ed % 5] * (1.f + (float)ed * 0.001f);
                EpisodicMemory_Store(&g_episodic_memory, ep_emb, EPISODIC_ENTRY_DIM, metrics.step_count);
            }
            if (g_versioning_rollback.initialized) {
                DominanceSnapshot snap;
                DominanceMetrics_GetCurrent(&g_dominance_metrics, &snap);
                VersioningRollback_CreateCheckpoint(&g_versioning_rollback,
                    metrics.step_count, metrics.generation,
                    metrics.loss, metrics.fitness, (float)snap.coherence);
            }
            if (g_provenance.initialized) {
                char config_json[1024];
                snprintf(config_json, sizeof(config_json),
                    "{\"cycle\":%u,\"step\":%llu,\"gen\":%llu,\"loss\":%.6f,\"fitness\":%.6f}",
                    evolution_cycle, (unsigned long long)metrics.step_count,
                    (unsigned long long)metrics.generation, metrics.loss, metrics.fitness);
                Provenance_LogBuild(&g_provenance, "Bin/raijin.exe",
                    config_json, (uint32_t)strlen(config_json), (uint32_t)evolution_cycle);
            }
        }
        if (g_episodic_memory.initialized && evolution_cycle % EPISODIC_CONSOLIDATION_THRESHOLD == 0) {
            EpisodicMemory_Consolidate(&g_episodic_memory);
        }
        if (g_episodic_memory.initialized && evolution_cycle % 100 == 0) {
            double util_fitness = (metrics.loss < 1.0) ? (1.0 - metrics.loss) : 0.0;
            uint32_t ep_count = EpisodicMemory_GetEntryCount(&g_episodic_memory);
            if (ep_count > 0) {
                EpisodicMemory_UpdateUtility(&g_episodic_memory, ep_count - 1, (float)util_fitness);
            }
            EpisodicMemory_ForgetLowUtility(&g_episodic_memory, 0.2f);
        }
        if (evolution_cycle % 50 == 0 && g_introspection_system.initialized) {
            IntrospectionSystem_Observe(&g_introspection_system);
            IntrospectionSystem_Critique(&g_introspection_system, NULL, 0);
        }
        {
            uint32_t stress_iv = (g_runtime_config.stress_interval >= 1u) ? g_runtime_config.stress_interval : 30u;
            if (!skip_heavy && evolution_cycle % stress_iv == 0 && g_stress_test_framework.initialized) {
                StressTestFramework_RunStressTest(&g_stress_test_framework, (StressTestType)(evolution_cycle / stress_iv % 5));
                if (g_dominance_metrics.initialized) {
                    DominanceMetrics_RecordStressResult(&g_dominance_metrics, g_stress_test_framework.last_result.robustness_score);
                }
                if (g_telemetry.initialized && !g_stress_test_framework.last_result.passed) {
                    Telemetry_Log(&g_telemetry, TELEMETRY_WARN, "StressTest", g_stress_test_framework.last_result.description);
                }
            }
        }
        if (!skip_heavy && evolution_cycle % 25 == 0 && g_adversarial_stress.initialized) {
            AdversarialStress_RunCycle(&g_adversarial_stress);
            if (g_dominance_metrics.initialized) {
                double robustness_score = 0.0;
                AdversarialStress_GetRobustnessScore(&g_adversarial_stress, &robustness_score);
                DominanceMetrics_RecordStressResult(&g_dominance_metrics, robustness_score);
            }
            if (g_telemetry.initialized && evolution_cycle % 100 == 0) {
                double robustness_score = 0.0;
                AdversarialStress_GetRobustnessScore(&g_adversarial_stress, &robustness_score);
                Telemetry_LogFormat(&g_telemetry, TELEMETRY_INFO, "AdversarialStress",
                    "robustness=%.4f", robustness_score);
            }
        }
        if (!skip_heavy && evolution_cycle % 20 == 0 && g_red_team.initialized) {
            RedTeam_RunCycle(&g_red_team);
            RedTeamResult rt_result;
            memset(&rt_result, 0, sizeof(rt_result));
            RedTeam_GetLastResult(&g_red_team, &rt_result);
            if (rt_result.induced_failure && g_telemetry.initialized) {
                Telemetry_LogFormat(&g_telemetry, TELEMETRY_WARN, "RedTeam",
                    "attack_type=%u verification_hint=%s", (unsigned)rt_result.attack_type, rt_result.verification_hint);
            }
        }
        if (evolution_cycle % g_runtime_config.self_test_interval == 0 && g_self_test_report.results) {
            SelfTest_RunAll(&g_self_test_report);
            if (!SelfTest_AllPassed(&g_self_test_report)) {
                if (g_self_healing.initialized)
                    SelfHealing_SoftRepair(&g_self_healing);
                if (g_regression_replay.initialized) {
                    for (uint32_t i = 0; i < g_self_test_report.count; i++) {
                        if (!g_self_test_report.results[i].passed) {
                            RegressionReplay_AddFailure(&g_regression_replay,
                                g_self_test_report.results[i].name,
                                g_self_test_report.results[i].message);
                        }
                    }
                }
                if (g_telemetry.initialized) {
                    Telemetry_Log(&g_telemetry, TELEMETRY_WARN, "SelfTest", "One or more self-tests failed");
                }
                SelfTest_PrintReport(&g_self_test_report);
            }
            if (g_regression_replay.initialized) {
                uint32_t replay_count = RegressionReplay_GetCount(&g_regression_replay);
                for (uint32_t r = 0; r < replay_count; r++) {
                    char test_name[REGRESSION_REPLAY_NAME_LEN];
                    char signature[REGRESSION_REPLAY_SIGNATURE_LEN];
                    if (NT_SUCCESS(RegressionReplay_GetEntry(&g_regression_replay, r, test_name, sizeof(test_name), signature, sizeof(signature)))) {
                        SelfTest_RunOne(&g_self_test_report, test_name);
                    }
                }
                if (!SelfTest_AllPassed(&g_self_test_report) && g_self_test_report.failed > 0) {
                    replay_fail_streak++;
                    if (g_telemetry.initialized) {
                        Telemetry_Log(&g_telemetry, TELEMETRY_WARN, "RegressionReplay", "Replay test(s) failed");
                    }
                    if (replay_fail_streak >= 2 && g_self_healing.initialized) {
                        SelfHealing_HardRepair(&g_self_healing);
                        replay_fail_streak = 0;
                    }
                } else {
                    replay_fail_streak = 0;
                }
            }
        }
        if (evolution_cycle % 10 == 0) {
            printf("Evolution Cycle #%u - loss=%.4f fitness=%.4f gen=%llu\n",
                evolution_cycle, metrics.loss, metrics.fitness,
                (unsigned long long)metrics.generation);
        }
    }

    SelfModifyAndImprove();
    AcquireKnowledge();
    DominateProgramming();

    if (g_training_pipeline && g_neural_context) {
        RoleBoundary_Exit(&g_role_boundary, "raijin");
        if (RoleBoundary_GetViolationCount(&g_role_boundary) > 0) {
            if (g_telemetry.initialized)
                Telemetry_Log(&g_telemetry, TELEMETRY_ERROR, "RoleBoundary",
                    "Boundary violation count > 0; stopping evolution");
            g_evolution_active = FALSE;
        }
    }

    if (evolution_cycle % 60 == 0) {
        printf("Evolution Cycle #%u - Major evolution milestone reached\n", evolution_cycle);
        DisplaySystemStatus();
    }
}

static void SelfModifyAndImprove() {
    if (!g_training_pipeline || !g_neural_context) return;

    static int self_modification_count = 0;
    NTSTATUS status;

    status = TrainingPipeline_TrainStep(g_training_pipeline);
    if (!NT_SUCCESS(status)) return;

    g_training_pipeline->run_evolution_this_step =
        (g_training_pipeline->total_steps % g_training_pipeline->evolution_interval) == 0;

    if (g_training_pipeline->run_evolution_this_step && g_evolution_engine) {
        status = TrainingPipeline_EvolutionStep(g_training_pipeline);
        if (NT_SUCCESS(status)) {
            self_modification_count++;
            TrainingMetrics metrics;
            TrainingPipeline_GetMetrics(g_training_pipeline, &metrics);
            if (self_modification_count % 5 == 0) {
                printf("  → Self-modification #%d: loss=%.4f entropy=%.3f gen=%llu\n",
                    self_modification_count, metrics.loss, metrics.entropy,
                    (unsigned long long)metrics.generation);
            }
        }
    }

    NeuralSubstrate_Evolve((NeuralSubstrate*)g_neural_context);
}

static const char* KNOWLEDGE_URLS[] = {
    "https://example.com",
    "https://www.w3.org/",
    "https://www.ietf.org/",
};
static const size_t KNOWLEDGE_URL_COUNT = sizeof(KNOWLEDGE_URLS) / sizeof(KNOWLEDGE_URLS[0]);

static void AcquireKnowledge() {
    if (!g_internet_context || !g_neural_context) return;

    static size_t url_index = 0;
    const char* url = KNOWLEDGE_URLS[url_index % KNOWLEDGE_URL_COUNT];
    url_index++;

    ProcessedContent* content = NULL;
    NTSTATUS status = InternetAcquisition_AcquireURL(g_internet_context, url, &content);
    if (!NT_SUCCESS(status) || !content) return;

    if (content->text_content && content->text_length > 0) {
        size_t learn_size = (content->text_length < 1000) ? content->text_length : 1000;
        NeuralSubstrate_Learn((NeuralSubstrate*)g_neural_context,
            content->text_content, learn_size);
    }

    if (content->raw_content && content->raw_size > 0) {
        size_t learn_size = (content->raw_size < 1000) ? content->raw_size : 1000;
        NeuralSubstrate_Learn((NeuralSubstrate*)g_neural_context,
            content->raw_content, learn_size);
    }

    InternetAcquisition_FreeProcessedContent(content);
}

static const char* SAMPLE_CODE_SNIPPETS[] = {
    "def fibonacci(n): return n if n <= 1 else fibonacci(n-1) + fibonacci(n-2)",
    "function sum(a, b) { return a + b; }",
    "int main() { return 0; }",
};
static const size_t SAMPLE_CODE_COUNT = sizeof(SAMPLE_CODE_SNIPPETS) / sizeof(SAMPLE_CODE_SNIPPETS[0]);

static void DominateProgramming() {
    if (!g_pd_context) return;

    static int domination_count = 0;
    size_t idx = domination_count % SAMPLE_CODE_COUNT;
    const char* code = SAMPLE_CODE_SNIPPETS[idx];
    ProgrammingLanguage lang = (ProgrammingLanguage)(idx % 3);

    CodeAnalysis* analysis = NULL;
    NTSTATUS status = ProgrammingDomination_AnalyzeCode(g_pd_context, code, lang, &analysis);
    if (NT_SUCCESS(status) && analysis) {
        domination_count++;
        if (domination_count % 10 == 0) {
            printf("  → Programming Domination #%d: analyzed %s (readability=%.1f)\n",
                domination_count, code, analysis->readability_score);
        }
        ProgrammingDomination_FreeCodeAnalysis(analysis);
    }

    CodeGenerationSpec spec = {0};
    spec.natural_language_spec = (char*)"implement a function that returns the square of a number";
    spec.target_language = LANG_PYTHON;
    char* generated = NULL;
    status = ProgrammingDomination_GenerateCode(g_pd_context, &spec, &generated, NULL, NULL);
    if (NT_SUCCESS(status) && generated) {
        NeuralSubstrate_Learn((NeuralSubstrate*)g_neural_context, generated,
            (strlen(generated) < 1000) ? strlen(generated) : 1000);
        free(generated);
    }
}

// Signal handler for clean shutdown
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType) {
    switch (ctrlType) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
            printf("\n\nShutdown signal received. Initiating clean shutdown...\n");
            g_evolution_active = FALSE;
            ShutdownRaijinSystem();
            return TRUE;
        default:
            return FALSE;
    }
}

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")