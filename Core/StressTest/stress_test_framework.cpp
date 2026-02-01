#include "../../Include/stress_test_framework.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

static void GenerateCorruptedInput(uint8_t* buf, size_t size, uint32_t seed) {
    srand(seed);
    for (size_t i = 0; i < size; i++) {
        if (rand() % 10 == 0)
            buf[i] = (uint8_t)(rand() & 0xFF);
        else
            buf[i] = (uint8_t)(i & 0xFF);
    }
}

static void GenerateNoiseInjection(uint8_t* buf, size_t size, uint32_t seed, float noise_level) {
    srand(seed);
    for (size_t i = 0; i < size; i++) {
        int noise = (int)((rand() / (double)RAND_MAX - 0.5) * 2.0 * noise_level * 255.0);
        int v = (int)buf[i] + noise;
        buf[i] = (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v));
    }
}

static void GenerateExtremeValues(uint8_t* buf, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buf[i] = (i % 2 == 0) ? 0 : 255;
    }
}

static void GenerateAdversarialPerturbation(uint8_t* buf, size_t size, uint32_t seed) {
    srand(seed);
    for (size_t i = 0; i < size; i++) {
        int sign = (rand() % 2) ? 1 : -1;
        int delta = 20 + (rand() % 60);
        int v = (int)buf[i] + sign * delta;
        buf[i] = (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v));
    }
}

static double ComputeOutputStability(const uint8_t* a, const uint8_t* b, size_t size) {
    double diff = 0.0;
    for (size_t i = 0; i < size; i++) {
        double d = (double)a[i] - (double)b[i];
        diff += d * d;
    }
    diff = sqrt(diff / (size * 65536.0));
    return 1.0 / (1.0 + diff);
}

NTSTATUS StressTestFramework_Initialize(StressTestFramework* stf,
    NeuralSubstrate* neural,
    EvolutionEngine* evolution) {
    if (!stf || !neural) return STATUS_INVALID_PARAMETER;
    memset(stf, 0, sizeof(StressTestFramework));
    stf->neural = neural;
    stf->evolution = evolution;
    stf->adversarial_input = (uint8_t*)malloc(STRESS_TEST_INPUT_SIZE);
    stf->normal_output = (uint8_t*)malloc(STRESS_TEST_INPUT_SIZE);
    stf->stress_output = (uint8_t*)malloc(STRESS_TEST_INPUT_SIZE);
    if (!stf->adversarial_input || !stf->normal_output || !stf->stress_output) {
        free(stf->adversarial_input);
        free(stf->normal_output);
        free(stf->stress_output);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(stf->adversarial_input, 0, STRESS_TEST_INPUT_SIZE);
    memset(stf->normal_output, 0, STRESS_TEST_INPUT_SIZE);
    memset(stf->stress_output, 0, STRESS_TEST_INPUT_SIZE);
    stf->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS StressTestFramework_Shutdown(StressTestFramework* stf) {
    if (!stf) return STATUS_INVALID_PARAMETER;
    free(stf->adversarial_input);
    free(stf->normal_output);
    free(stf->stress_output);
    stf->adversarial_input = NULL;
    stf->normal_output = NULL;
    stf->stress_output = NULL;
    stf->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS StressTestFramework_RunStressTest(StressTestFramework* stf, StressTestType type) {
    if (!stf || !stf->initialized || !stf->neural) return STATUS_INVALID_PARAMETER;

    uint8_t normal_input[STRESS_TEST_INPUT_SIZE];
    memset(normal_input, 0x80, STRESS_TEST_INPUT_SIZE);

    uint64_t t0 = GetTimeMs();
    NTSTATUS status = NeuralSubstrate_Process(stf->neural, normal_input, STRESS_TEST_INPUT_SIZE,
        stf->normal_output, STRESS_TEST_INPUT_SIZE);
    if (!NT_SUCCESS(status)) {
        stf->last_result.type = type;
        stf->last_result.passed = false;
        stf->last_result.robustness_score = 0.0;
        stf->last_result.duration_ms = GetTimeMs() - t0;
        stf->last_result.failure_count = 1;
        snprintf(stf->last_result.description, sizeof(stf->last_result.description),
            "Stress test %u: normal process failed", (unsigned)type);
        return status;
    }

    memcpy(stf->adversarial_input, normal_input, STRESS_TEST_INPUT_SIZE);
    uint32_t seed = (uint32_t)(GetTimeMs() & 0xFFFFFFFF);
    switch (type) {
        case STRESS_TEST_CORRUPTED_INPUT:
            GenerateCorruptedInput(stf->adversarial_input, STRESS_TEST_INPUT_SIZE, seed);
            break;
        case STRESS_TEST_NOISE_INJECTION:
            GenerateNoiseInjection(stf->adversarial_input, STRESS_TEST_INPUT_SIZE, seed, 0.5f);
            break;
        case STRESS_TEST_EXTREME_VALUES:
            GenerateExtremeValues(stf->adversarial_input, STRESS_TEST_INPUT_SIZE);
            break;
        case STRESS_TEST_ZERO_INPUT:
            memset(stf->adversarial_input, 0, STRESS_TEST_INPUT_SIZE);
            break;
        case STRESS_TEST_AVERSARIAL_PERTURBATION:
            GenerateAdversarialPerturbation(stf->adversarial_input, STRESS_TEST_INPUT_SIZE, seed);
            break;
        default:
            memcpy(stf->adversarial_input, normal_input, STRESS_TEST_INPUT_SIZE);
            break;
    }

    status = NeuralSubstrate_Process(stf->neural, stf->adversarial_input, STRESS_TEST_INPUT_SIZE,
        stf->stress_output, STRESS_TEST_INPUT_SIZE);
    uint64_t t1 = GetTimeMs();

    stf->last_result.type = type;
    stf->last_result.passed = NT_SUCCESS(status);
    stf->last_result.duration_ms = t1 - t0;
    stf->last_result.failure_count = NT_SUCCESS(status) ? 0 : 1;

    if (NT_SUCCESS(status)) {
        stf->last_result.robustness_score = ComputeOutputStability(
            stf->normal_output, stf->stress_output, STRESS_TEST_INPUT_SIZE);
        stf->last_result.passed = (stf->last_result.robustness_score > 0.1);
    } else {
        stf->last_result.robustness_score = 0.0;
    }

    stf->stress_run_count++;
    if (stf->last_result.passed) stf->stress_pass_count++;

    snprintf(stf->last_result.description, sizeof(stf->last_result.description),
        "Stress test %u: %s robustness=%.3f", (unsigned)type,
        stf->last_result.passed ? "PASS" : "FAIL", stf->last_result.robustness_score);

    return STATUS_SUCCESS;
}

NTSTATUS StressTestFramework_RunAdversarialAgent(StressTestFramework* stf, uint32_t agent_index) {
    if (!stf || !stf->initialized) return STATUS_INVALID_PARAMETER;
    StressTestType t = (StressTestType)(agent_index % 5);
    return StressTestFramework_RunStressTest(stf, t);
}

NTSTATUS StressTestFramework_BenchmarkUnderStress(StressTestFramework* stf,
    double* robustness_score,
    uint32_t* pass_count,
    uint32_t* total_count) {
    if (!stf || !stf->initialized) return STATUS_INVALID_PARAMETER;

    uint32_t pass = 0;
    double total_robustness = 0.0;
    for (uint32_t i = 0; i < STRESS_BENCHMARK_RUNS; i++) {
        StressTestType t = (StressTestType)(i % 5);
        NTSTATUS status = StressTestFramework_RunStressTest(stf, t);
        if (NT_SUCCESS(status)) {
            if (stf->last_result.passed) pass++;
            total_robustness += stf->last_result.robustness_score;
        }
    }

    if (robustness_score) *robustness_score = total_robustness / STRESS_BENCHMARK_RUNS;
    if (pass_count) *pass_count = pass;
    if (total_count) *total_count = STRESS_BENCHMARK_RUNS;
    return STATUS_SUCCESS;
}

void StressTestFramework_GetLastResult(const StressTestFramework* stf, StressTestResult* out) {
    if (!stf || !out) return;
    *out = stf->last_result;
}
