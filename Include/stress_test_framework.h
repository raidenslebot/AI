#ifndef RAIJIN_STRESS_TEST_FRAMEWORK_H
#define RAIJIN_STRESS_TEST_FRAMEWORK_H

#include "raijin_ntstatus.h"
#include "neural_substrate.h"
#include "evolution_engine.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define STRESS_TEST_INPUT_SIZE 1000
#define STRESS_ADVERSARIAL_AGENTS_MAX 4
#define STRESS_BENCHMARK_RUNS 32

typedef enum {
    STRESS_TEST_CORRUPTED_INPUT = 0,
    STRESS_TEST_NOISE_INJECTION = 1,
    STRESS_TEST_EXTREME_VALUES = 2,
    STRESS_TEST_ZERO_INPUT = 3,
    STRESS_TEST_AVERSARIAL_PERTURBATION = 4
} StressTestType;

typedef struct StressTestResult {
    StressTestType type;
    bool passed;
    double robustness_score;
    uint64_t duration_ms;
    uint32_t failure_count;
    char description[256];
} StressTestResult;

typedef struct StressTestFramework {
    NeuralSubstrate* neural;
    EvolutionEngine* evolution;
    StressTestResult last_result;
    uint8_t* adversarial_input;
    uint8_t* normal_output;
    uint8_t* stress_output;
    uint32_t stress_run_count;
    uint32_t stress_pass_count;
    uint32_t adversarial_agent_count;
    bool initialized;
} StressTestFramework;

NTSTATUS StressTestFramework_Initialize(StressTestFramework* stf,
    NeuralSubstrate* neural,
    EvolutionEngine* evolution);
NTSTATUS StressTestFramework_Shutdown(StressTestFramework* stf);

NTSTATUS StressTestFramework_RunStressTest(StressTestFramework* stf, StressTestType type);

NTSTATUS StressTestFramework_RunAdversarialAgent(StressTestFramework* stf, uint32_t agent_index);

NTSTATUS StressTestFramework_BenchmarkUnderStress(StressTestFramework* stf,
    double* robustness_score,
    uint32_t* pass_count,
    uint32_t* total_count);

void StressTestFramework_GetLastResult(const StressTestFramework* stf, StressTestResult* out);

#endif
