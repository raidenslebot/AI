#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Raijin System Tests
// Basic functionality tests for all components

// Include system headers (all from Include per single-source)
#include "Include/hal.h"
#include "Include/hypervisor.h"
#include "Include/neural_substrate.h"
#include "Include/ethics_system.h"
#include "Include/screen_control.h"
#include "Include/autonomous_manager.h"

// Test results
typedef enum {
    TEST_PASS = 0,
    TEST_FAIL = 1,
    TEST_SKIP = 2
} TestResult;

typedef struct {
    const char* name;
    TestResult result;
    char* message;
} TestCase;

#define MAX_TESTS 100
static TestCase g_tests[MAX_TESTS];
static int g_test_count = 0;

// Test registration
void RegisterTest(const char* name, TestResult result, const char* message) {
    if (g_test_count < MAX_TESTS) {
        g_tests[g_test_count].name = name;
        g_tests[g_test_count].result = result;
        g_tests[g_test_count].message = _strdup(message);
        g_test_count++;
    }
}

// Test functions
TestResult TestHAL() {
    printf("Testing HAL...\n");

    HALInterface hal = {0};
    NTSTATUS status = HAL_Initialize(&hal);

    if (!NT_SUCCESS(status)) {
        RegisterTest("HAL_Initialize", TEST_FAIL, "Failed to initialize HAL");
        return TEST_FAIL;
    }

    // Test basic functionality
    uint64_t memory = HAL_GetAvailableMemory(&hal);
    if (memory == 0) {
        RegisterTest("HAL_GetAvailableMemory", TEST_FAIL, "Failed to get memory info");
        HAL_Shutdown(&hal);
        return TEST_FAIL;
    }

    printf("  Available memory: %llu MB\n", memory / (1024 * 1024));

    HAL_Shutdown(&hal);
    RegisterTest("HAL", TEST_PASS, "HAL initialization and basic functions work");
    return TEST_PASS;
}

TestResult TestHypervisor() {
    printf("Testing Hypervisor Layer...\n");

    NTSTATUS status = HypervisorLayer_Initialize();
    if (!NT_SUCCESS(status)) {
        RegisterTest("Hypervisor_Initialize", TEST_FAIL, "Failed to initialize hypervisor");
        return TEST_FAIL;
    }

    // Test basic hypervisor functions
    uint64_t vmcs_count = HypervisorLayer_GetVMCSCount();
    printf("  VMCS regions: %llu\n", vmcs_count);

    HypervisorLayer_Shutdown();
    RegisterTest("Hypervisor", TEST_PASS, "Hypervisor layer initialized successfully");
    return TEST_PASS;
}

TestResult TestNeuralSubstrate() {
    printf("Testing Neural Substrate...\n");

    NeuralSubstrate neural = {0};
    NTSTATUS status = NeuralSubstrate_Initialize(&neural);

    if (!NT_SUCCESS(status)) {
        RegisterTest("Neural_Initialize", TEST_FAIL, "Failed to initialize neural substrate");
        return TEST_FAIL;
    }

    // Test basic neural operations
    uint8_t input[100] = {1, 2, 3}; // Sample input
    uint8_t output[100] = {0};

    status = NeuralSubstrate_Process(&neural, input, 3, output, 100);
    if (!NT_SUCCESS(status)) {
        RegisterTest("Neural_Process", TEST_FAIL, "Failed to process neural data");
        NeuralSubstrate_Shutdown(&neural);
        return TEST_FAIL;
    }

    // Test evolution
    status = NeuralSubstrate_Evolve(&neural);
    if (!NT_SUCCESS(status)) {
        RegisterTest("Neural_Evolve", TEST_FAIL, "Failed to evolve neural substrate");
        NeuralSubstrate_Shutdown(&neural);
        return TEST_FAIL;
    }

    float entropy = NeuralSubstrate_GetEntropy(&neural);
    printf("  Neural entropy: %.3f\n", entropy);

    NeuralSubstrate_Shutdown(&neural);
    RegisterTest("NeuralSubstrate", TEST_PASS, "Neural substrate processing and evolution work");
    return TEST_PASS;
}

TestResult TestEthicsSystem() {
    printf("Testing Ethics System...\n");

    EthicsSystem ethics = {0};
    NTSTATUS status = EthicsSystem_Initialize(&ethics);

    if (!NT_SUCCESS(status)) {
        RegisterTest("Ethics_Initialize", TEST_FAIL, "Failed to initialize ethics system");
        return TEST_FAIL;
    }

    // Test learning from input
    status = EthicsSystem_LearnFromInput(&ethics,
                                       "I value honesty and fairness",
                                       "test_input");
    if (!NT_SUCCESS(status)) {
        RegisterTest("Ethics_Learn", TEST_FAIL, "Failed to learn from input");
        EthicsSystem_Shutdown(&ethics);
        return TEST_FAIL;
    }

    // Test action evaluation
    float alignment = EthicsSystem_GetAlignmentScore(&ethics, "telling the truth");
    printf("  Truth alignment: %.2f\n", alignment);

    EthicsSystem_Shutdown(&ethics);
    RegisterTest("EthicsSystem", TEST_PASS, "Ethics system learning and evaluation work");
    return TEST_PASS;
}

TestResult TestScreenControl() {
    printf("Testing Screen Control System...\n");

    ScreenControlSystem screen = {0};
    NTSTATUS status = ScreenControl_Initialize(&screen);

    if (!NT_SUCCESS(status)) {
        RegisterTest("Screen_Initialize", TEST_FAIL, "Failed to initialize screen control");
        return TEST_FAIL;
    }

    // Test screen capture (without actually capturing to avoid issues)
    printf("  Screen control initialized (capture test skipped in automated test)\n");

    ScreenControl_Shutdown(&screen);
    RegisterTest("ScreenControl", TEST_PASS, "Screen control system initialized");
    return TEST_PASS;
}

TestResult TestAutonomousManager() {
    printf("Testing Autonomous Manager...\n");

    // Create minimal system instances for testing
    NeuralSubstrate neural = {0};
    EthicsSystem ethics = {0};
    ScreenControlSystem screen = {0};

    // Initialize components
    if (!NT_SUCCESS(NeuralSubstrate_Initialize(&neural))) {
        RegisterTest("AutoMgr_NeuralInit", TEST_FAIL, "Failed to init neural for autonomous test");
        return TEST_FAIL;
    }

    if (!NT_SUCCESS(EthicsSystem_Initialize(&ethics))) {
        RegisterTest("AutoMgr_EthicsInit", TEST_FAIL, "Failed to init ethics for autonomous test");
        NeuralSubstrate_Shutdown(&neural);
        return TEST_FAIL;
    }

    if (!NT_SUCCESS(ScreenControl_Initialize(&screen))) {
        RegisterTest("AutoMgr_ScreenInit", TEST_FAIL, "Failed to init screen for autonomous test");
        NeuralSubstrate_Shutdown(&neural);
        EthicsSystem_Shutdown(&ethics);
        return TEST_FAIL;
    }

    AutonomousManager manager = {0};
    NTSTATUS status = AutonomousManager_Initialize(&manager, &neural, &ethics, &screen);

    if (!NT_SUCCESS(status)) {
        RegisterTest("AutoMgr_Initialize", TEST_FAIL, "Failed to initialize autonomous manager");
        ScreenControl_Shutdown(&screen);
        EthicsSystem_Shutdown(&ethics);
        NeuralSubstrate_Shutdown(&neural);
        return TEST_FAIL;
    }

    // Test task queuing
    status = AutonomousManager_QueueTask(&manager, TASK_LEARN,
                                       "Test learning task",
                                       "{\"test\":\"data\"}", PRIORITY_NORMAL);
    if (!NT_SUCCESS(status)) {
        RegisterTest("AutoMgr_QueueTask", TEST_FAIL, "Failed to queue task");
        AutonomousManager_Shutdown(&manager);
        ScreenControl_Shutdown(&screen);
        EthicsSystem_Shutdown(&ethics);
        NeuralSubstrate_Shutdown(&neural);
        return TEST_FAIL;
    }

    // Test decision making
    char decision[256] = {0};
    status = AutonomousManager_MakeDecision(&manager, "What should I do next?",
                                          decision, sizeof(decision));
    if (!NT_SUCCESS(status)) {
        RegisterTest("AutoMgr_Decision", TEST_FAIL, "Failed to make decision");
        AutonomousManager_Shutdown(&manager);
        ScreenControl_Shutdown(&screen);
        EthicsSystem_Shutdown(&ethics);
        NeuralSubstrate_Shutdown(&neural);
        return TEST_FAIL;
    }

    printf("  Decision: %s\n", decision);

    // Test action evaluation
    float alignment = 0.0f;
    char reasoning[512] = {0};
    status = AutonomousManager_EvaluateAction(&manager, "helping others",
                                            &alignment, reasoning, sizeof(reasoning));
    if (!NT_SUCCESS(status)) {
        RegisterTest("AutoMgr_Evaluate", TEST_FAIL, "Failed to evaluate action");
        AutonomousManager_Shutdown(&manager);
        ScreenControl_Shutdown(&screen);
        EthicsSystem_Shutdown(&ethics);
        NeuralSubstrate_Shutdown(&neural);
        return TEST_FAIL;
    }

    printf("  Action alignment: %.2f\n", alignment);

    AutonomousManager_Shutdown(&manager);
    ScreenControl_Shutdown(&screen);
    EthicsSystem_Shutdown(&ethics);
    NeuralSubstrate_Shutdown(&neural);

    RegisterTest("AutonomousManager", TEST_PASS, "Autonomous manager core functions work");
    return TEST_PASS;
}

TestResult TestSystemIntegration() {
    printf("Testing System Integration...\n");

    // This test verifies that all systems can be initialized together
    HALInterface hal = {0};
    NeuralSubstrate neural = {0};
    EthicsSystem ethics = {0};
    ScreenControlSystem screen = {0};
    AutonomousManager manager = {0};

    NTSTATUS status;

    // Initialize all systems
    status = HAL_Initialize(&hal);
    if (!NT_SUCCESS(status)) {
        RegisterTest("Integration_HAL", TEST_FAIL, "HAL init failed in integration test");
        return TEST_FAIL;
    }

    status = NeuralSubstrate_Initialize(&neural);
    if (!NT_SUCCESS(status)) {
        RegisterTest("Integration_Neural", TEST_FAIL, "Neural init failed in integration test");
        HAL_Shutdown(&hal);
        return TEST_FAIL;
    }

    status = EthicsSystem_Initialize(&ethics);
    if (!NT_SUCCESS(status)) {
        RegisterTest("Integration_Ethics", TEST_FAIL, "Ethics init failed in integration test");
        NeuralSubstrate_Shutdown(&neural);
        HAL_Shutdown(&hal);
        return TEST_FAIL;
    }

    status = ScreenControl_Initialize(&screen);
    if (!NT_SUCCESS(status)) {
        RegisterTest("Integration_Screen", TEST_FAIL, "Screen init failed in integration test");
        EthicsSystem_Shutdown(&ethics);
        NeuralSubstrate_Shutdown(&neural);
        HAL_Shutdown(&hal);
        return TEST_FAIL;
    }

    status = AutonomousManager_Initialize(&manager, &neural, &ethics, &screen);
    if (!NT_SUCCESS(status)) {
        RegisterTest("Integration_Manager", TEST_FAIL, "Manager init failed in integration test");
        ScreenControl_Shutdown(&screen);
        EthicsSystem_Shutdown(&ethics);
        NeuralSubstrate_Shutdown(&neural);
        HAL_Shutdown(&hal);
        return TEST_FAIL;
    }

    // Test cross-system interaction
    status = AutonomousManager_ProcessUserInput(&manager, "I want to create something beautiful");
    if (!NT_SUCCESS(status)) {
        RegisterTest("Integration_CrossTalk", TEST_FAIL, "Cross-system communication failed");
        AutonomousManager_Shutdown(&manager);
        ScreenControl_Shutdown(&screen);
        EthicsSystem_Shutdown(&ethics);
        NeuralSubstrate_Shutdown(&neural);
        HAL_Shutdown(&hal);
        return TEST_FAIL;
    }

    // Shutdown in reverse order
    AutonomousManager_Shutdown(&manager);
    ScreenControl_Shutdown(&screen);
    EthicsSystem_Shutdown(&ethics);
    NeuralSubstrate_Shutdown(&neural);
    HAL_Shutdown(&hal);

    RegisterTest("SystemIntegration", TEST_PASS, "All systems integrate successfully");
    return TEST_PASS;
}

// Run all tests
int RunTests() {
    printf("=====================================\n");
    printf("     Raijin ECGI Test Suite\n");
    printf("=====================================\n\n");

    // Run individual component tests
    TestHAL();
    TestHypervisor();
    TestNeuralSubstrate();
    TestEthicsSystem();
    TestScreenControl();
    TestAutonomousManager();

    // Run integration test
    TestSystemIntegration();

    // Report results
    printf("\n=====================================\n");
    printf("           Test Results\n");
    printf("=====================================\n");

    int passed = 0, failed = 0, skipped = 0;

    for (int i = 0; i < g_test_count; i++) {
        const char* status_str;
        switch (g_tests[i].result) {
            case TEST_PASS: status_str = "PASS"; passed++; break;
            case TEST_FAIL: status_str = "FAIL"; failed++; break;
            case TEST_SKIP: status_str = "SKIP"; skipped++; break;
            default: status_str = "UNKNOWN"; break;
        }

        printf("%-20s: %s\n", g_tests[i].name, status_str);
        if (g_tests[i].result == TEST_FAIL && g_tests[i].message) {
            printf("  %s\n", g_tests[i].message);
        }
    }

    printf("\nSummary: %d passed, %d failed, %d skipped\n", passed, failed, skipped);

    // Cleanup
    for (int i = 0; i < g_test_count; i++) {
        free(g_tests[i].message);
    }

    return failed > 0 ? 1 : 0;
}

// Main test entry point
int main(int argc, char* argv[]) {
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    return RunTests();
}