#include "../../Include/self_test.h"
#include "../../Include/neural_substrate.h"
#include "../../Include/evolution_engine.h"
#include "../../Include/training_pipeline.h"
#include "../../Include/role_boundary.h"
#include "../../Include/task_oracle.h"
#include "../../Include/curriculum.h"
#include "../../Include/resource_governor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static uint64_t GetTimeMs(void) {
    return GetTickCount64();
}

static NTSTATUS Test_NeuralInit(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate substrate;
    memset(&substrate, 0, sizeof(substrate));
    NTSTATUS status = NeuralSubstrate_Initialize(&substrate);
    uint64_t dur = GetTimeMs() - t0;
    NeuralSubstrate_Shutdown(&substrate);
    SelfTestReport_Add(report, "NeuralSubstrate_Initialize",
        NT_SUCCESS(status), NT_SUCCESS(status) ? "OK" : "Init failed", dur);
    return status;
}

static NTSTATUS Test_NeuralProcess(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate substrate;
    memset(&substrate, 0, sizeof(substrate));
    NTSTATUS status = NeuralSubstrate_Initialize(&substrate);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "NeuralSubstrate_Process", false, "Init failed", GetTimeMs() - t0);
        return status;
    }
    uint8_t input[100] = {0};
    uint8_t output[100] = {0};
    for (int i = 0; i < 100; i++) input[i] = (uint8_t)(i & 0xFF);
    status = NeuralSubstrate_Process(&substrate, input, sizeof(input), output, sizeof(output));
    uint64_t dur = GetTimeMs() - t0;
    NeuralSubstrate_Shutdown(&substrate);
    SelfTestReport_Add(report, "NeuralSubstrate_Process",
        NT_SUCCESS(status), NT_SUCCESS(status) ? "OK" : "Process failed", dur);
    return status;
}

static NTSTATUS Test_NeuralLearn(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate substrate;
    memset(&substrate, 0, sizeof(substrate));
    NTSTATUS status = NeuralSubstrate_Initialize(&substrate);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "NeuralSubstrate_Learn", false, "Init failed", GetTimeMs() - t0);
        return status;
    }
    uint8_t input[100] = {0}, target[100] = {0};
    for (int i = 0; i < 100; i++) { input[i] = (uint8_t)i; target[i] = (uint8_t)(i + 1); }
    status = NeuralSubstrate_Process(&substrate, input, 100, target, 100);
    if (NT_SUCCESS(status))
        status = NeuralSubstrate_Learn(&substrate, target, 100);
    uint64_t dur = GetTimeMs() - t0;
    NeuralSubstrate_Shutdown(&substrate);
    SelfTestReport_Add(report, "NeuralSubstrate_Learn",
        NT_SUCCESS(status), NT_SUCCESS(status) ? "OK" : "Learn failed", dur);
    return status;
}

static NTSTATUS Test_NeuralAdversarialNull(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate substrate;
    memset(&substrate, 0, sizeof(substrate));
    NTSTATUS status = NeuralSubstrate_Initialize(&substrate);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "NeuralAdversarial_NullInput", true, "Init failed skip", GetTimeMs() - t0);
        return status;
    }
    uint8_t output[10] = {0};
    status = NeuralSubstrate_Process(&substrate, NULL, 0, output, 10);
    uint64_t dur = GetTimeMs() - t0;
    NeuralSubstrate_Shutdown(&substrate);
    bool expect_fail = !NT_SUCCESS(status);
    SelfTestReport_Add(report, "NeuralAdversarial_NullInput",
        expect_fail, expect_fail ? "Correctly rejected null" : "Should reject null", dur);
    return STATUS_SUCCESS;
}

static NTSTATUS Test_EvolutionInit(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate neural;
    memset(&neural, 0, sizeof(neural));
    NeuralSubstrate_Initialize(&neural);
    void* ethics_ctx = NULL;
    NTSTATUS status = Ethics_Initialize(&ethics_ctx, &neural);
    if (!NT_SUCCESS(status)) {
        NeuralSubstrate_Shutdown(&neural);
        SelfTestReport_Add(report, "EvolutionEngine_Initialize", false, "Ethics init failed", GetTimeMs() - t0);
        return status;
    }
    EvolutionEngine engine;
    memset(&engine, 0, sizeof(engine));
    EvolutionParameters params;
    memset(&params, 0, sizeof(params));
    params.algorithm = EVOLUTION_TYPE_GENETIC;
    params.selection = SELECTION_TOURNAMENT;
    params.mutation = MUTATION_GAUSSIAN;
    params.crossover = CROSSOVER_SINGLE_POINT;
    params.fitness_func = FITNESS_ACCURACY;
    params.mutation_rate = 0.01;
    params.crossover_rate = 0.7;
    params.elitism_rate = 0.1;
    params.tournament_size = 2;
    params.population_size = 4;
    params.max_generations = 2;
    params.target_fitness = 0.99;
    params.stagnation_limit = 2;
    params.parallel_evaluations = 1;
    status = EvolutionEngine_Initialize(&engine, &params, &neural, (EthicsSystem*)ethics_ctx);
    uint64_t dur = GetTimeMs() - t0;
    EvolutionEngine_Shutdown(&engine);
    Ethics_Shutdown(ethics_ctx);
    NeuralSubstrate_Shutdown(&neural);
    SelfTestReport_Add(report, "EvolutionEngine_Initialize",
        NT_SUCCESS(status), NT_SUCCESS(status) ? "OK" : "Init failed", dur);
    return status;
}

static NTSTATUS Test_TrainingStep(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate neural;
    memset(&neural, 0, sizeof(neural));
    NTSTATUS status = NeuralSubstrate_Initialize(&neural);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "TrainingPipeline_TrainStep", false, "Neural init failed", GetTimeMs() - t0);
        return status;
    }
    TrainingPipeline pipeline;
    memset(&pipeline, 0, sizeof(pipeline));
    status = TrainingPipeline_Initialize(&pipeline, &neural, NULL);
    if (!NT_SUCCESS(status)) {
        NeuralSubstrate_Shutdown(&neural);
        SelfTestReport_Add(report, "TrainingPipeline_TrainStep", false, "Pipeline init failed", GetTimeMs() - t0);
        return status;
    }
    status = TrainingPipeline_TrainStep(&pipeline);
    uint64_t dur = GetTimeMs() - t0;
    TrainingPipeline_Shutdown(&pipeline);
    NeuralSubstrate_Shutdown(&neural);
    SelfTestReport_Add(report, "TrainingPipeline_TrainStep",
        NT_SUCCESS(status), NT_SUCCESS(status) ? "OK" : "TrainStep failed", dur);
    return status;
}

static NTSTATUS Test_StressManyCycles(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate neural;
    memset(&neural, 0, sizeof(neural));
    NTSTATUS status = NeuralSubstrate_Initialize(&neural);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "Stress_ManyCycles", false, "Init failed", GetTimeMs() - t0);
        return status;
    }
    const int cycles = 50;
    int ok = 0;
    for (int i = 0; i < cycles; i++) {
        uint8_t in[100], out[100];
        for (int j = 0; j < 100; j++) in[j] = (uint8_t)((i + j) & 0xFF);
        if (NT_SUCCESS(NeuralSubstrate_Process(&neural, in, 100, out, 100)))
            ok++;
    }
    uint64_t dur = GetTimeMs() - t0;
    NeuralSubstrate_Shutdown(&neural);
    bool passed = (ok == cycles);
    char msg[SELF_TEST_MAX_MESSAGE];
    snprintf(msg, sizeof(msg), "%d/%d cycles OK", ok, cycles);
    SelfTestReport_Add(report, "Stress_ManyCycles", passed, msg, dur);
    return passed ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

static NTSTATUS Test_AdversarialZeroSize(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate substrate;
    memset(&substrate, 0, sizeof(substrate));
    NTSTATUS status = NeuralSubstrate_Initialize(&substrate);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "Adversarial_ZeroSize", true, "Init failed skip", GetTimeMs() - t0);
        return status;
    }
    uint8_t input[1] = {0}, output[10] = {0};
    status = NeuralSubstrate_Process(&substrate, input, 0, output, 10);
    uint64_t dur = GetTimeMs() - t0;
    NeuralSubstrate_Shutdown(&substrate);
    bool expect_fail = !NT_SUCCESS(status);
    SelfTestReport_Add(report, "Adversarial_ZeroSize",
        expect_fail, expect_fail ? "Correctly rejected zero size" : "Should reject zero size", dur);
    return STATUS_SUCCESS;
}

static NTSTATUS Test_AdversarialExtremeValues(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate substrate;
    memset(&substrate, 0, sizeof(substrate));
    NTSTATUS status = NeuralSubstrate_Initialize(&substrate);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "Adversarial_ExtremeValues", false, "Init failed", GetTimeMs() - t0);
        return status;
    }
    uint8_t input[100], output[100];
    for (int i = 0; i < 100; i++) input[i] = 0xFF;
    status = NeuralSubstrate_Process(&substrate, input, 100, output, 100);
    uint64_t dur = GetTimeMs() - t0;
    NeuralSubstrate_Shutdown(&substrate);
    bool passed = NT_SUCCESS(status);
    SelfTestReport_Add(report, "Adversarial_ExtremeValues",
        passed, passed ? "Processed 0xFF input" : "Process failed", dur);
    return passed ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

static NTSTATUS Test_RecoveryAfterNeuralFailure(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    NeuralSubstrate substrate;
    memset(&substrate, 0, sizeof(substrate));
    NTSTATUS status = NeuralSubstrate_Initialize(&substrate);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "Recovery_AfterFailure", false, "Init failed", GetTimeMs() - t0);
        return status;
    }
    uint8_t output[10] = {0};
    NTSTATUS fail_status = NeuralSubstrate_Process(&substrate, NULL, 0, output, 10);
    bool reject_null = !NT_SUCCESS(fail_status);
    uint8_t input[100], out[100];
    for (int i = 0; i < 100; i++) input[i] = (uint8_t)i;
    NTSTATUS recover_status = NeuralSubstrate_Process(&substrate, input, 100, out, 100);
    uint64_t dur = GetTimeMs() - t0;
    NeuralSubstrate_Shutdown(&substrate);
    bool passed = reject_null && NT_SUCCESS(recover_status);
    char msg[SELF_TEST_MAX_MESSAGE];
    snprintf(msg, sizeof(msg), "reject_null=%d recover_ok=%d", reject_null ? 1 : 0, NT_SUCCESS(recover_status) ? 1 : 0);
    SelfTestReport_Add(report, "Recovery_AfterFailure", passed, passed ? "Rejected null then processed valid" : msg, dur);
    return passed ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

NTSTATUS SelfTestReport_Initialize(SelfTestReport* report, uint32_t max_tests) {
    if (!report || max_tests == 0) return STATUS_INVALID_PARAMETER;
    memset(report, 0, sizeof(SelfTestReport));
    report->results = (SelfTestResult*)malloc(max_tests * sizeof(SelfTestResult));
    if (!report->results) return STATUS_INSUFFICIENT_RESOURCES;
    report->capacity = max_tests;
    return STATUS_SUCCESS;
}

void SelfTestReport_Shutdown(SelfTestReport* report) {
    if (report && report->results) {
        free(report->results);
        report->results = NULL;
        report->count = report->capacity = report->passed = report->failed = 0;
    }
}

NTSTATUS SelfTestReport_Add(SelfTestReport* report, const char* name,
    bool passed, const char* message, uint64_t duration_ms) {
    if (!report || !report->results || report->count >= report->capacity) return STATUS_INVALID_PARAMETER;
    SelfTestResult* r = &report->results[report->count++];
    strncpy(r->name, name ? name : "", SELF_TEST_MAX_NAME - 1);
    r->name[SELF_TEST_MAX_NAME - 1] = '\0';
    strncpy(r->message, message ? message : "", SELF_TEST_MAX_MESSAGE - 1);
    r->message[SELF_TEST_MAX_MESSAGE - 1] = '\0';
    r->passed = passed;
    r->duration_ms = duration_ms;
    if (passed) report->passed++; else report->failed++;
    report->total_ms += duration_ms;
    return STATUS_SUCCESS;
}

static NTSTATUS Test_RoleBoundary_NoViolation(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    RoleBoundaryContext local = {0};
    local.initialized = true;
    RoleBoundary_Enter(&local, "cursor.test", ROLE_OWNER_CURSOR);
    RoleBoundary_AssertCursor(&local);
    bool ok = RoleBoundary_Validate(&local);
    RoleBoundary_Exit(&local, "cursor.test");
    SelfTestReport_Add(report, "RoleBoundary_NoViolation", ok, ok ? "OK" : "violation", GetTimeMs() - t0);
    return STATUS_SUCCESS;
}

static NTSTATUS Test_RoleBoundary_DetectsViolation(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    RoleBoundaryContext local = {0};
    local.initialized = true;
    RoleBoundary_Enter(&local, "raijin.fake", ROLE_OWNER_RAIJIN);
    RoleBoundary_AssertCursor(&local);
    uint32_t v = RoleBoundary_GetViolationCount(&local);
    RoleBoundary_Exit(&local, "raijin.fake");
    bool ok = (v >= 1);
    SelfTestReport_Add(report, "RoleBoundary_DetectsViolation", ok, ok ? "OK" : "no violation", GetTimeMs() - t0);
    return STATUS_SUCCESS;
}

static NTSTATUS Test_TaskOracle_Evaluate(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    uint8_t output[64], target[64];
    for (int i = 0; i < 64; i++) { output[i] = (uint8_t)i; target[i] = (uint8_t)(i + 1); }
    double loss = 0.1;
    TaskOracleResult result = {0};
    NTSTATUS status = TaskOracle_Evaluate(CURRICULUM_CODING, output, target, 64, loss, &result);
    bool ok = NT_SUCCESS(status) && result.score >= 0.0 && result.score <= 1.0 && result.property_violations == 0;
    SelfTestReport_Add(report, "TaskOracle_Evaluate", ok, ok ? "OK" : "fail", GetTimeMs() - t0);
    return status;
}

static NTSTATUS Test_ResourceGovernor_ResetThrottle(SelfTestReport* report) {
    uint64_t t0 = GetTimeMs();
    ResourceGovernor rg = {{0}};
    NTSTATUS status = ResourceGovernor_Initialize(&rg);
    if (!NT_SUCCESS(status)) {
        SelfTestReport_Add(report, "ResourceGovernor_ResetThrottle", false, "Init failed", GetTimeMs() - t0);
        return status;
    }
    rg.throttle_factor = 0.3;
    rg.degradation_mode = 2;
    status = ResourceGovernor_ResetThrottle(&rg);
    bool ok = NT_SUCCESS(status) && rg.throttle_factor >= 0.99 && rg.throttle_factor <= 1.01 && rg.degradation_mode == 0;
    ResourceGovernor_Shutdown(&rg);
    SelfTestReport_Add(report, "ResourceGovernor_ResetThrottle", ok, ok ? "OK" : "fail", GetTimeMs() - t0);
    return STATUS_SUCCESS;
}

typedef NTSTATUS (*SelfTestFn)(SelfTestReport*);

static const struct {
    const char* name;
    SelfTestFn fn;
} s_self_test_dispatch[] = {
    { "NeuralSubstrate_Initialize", Test_NeuralInit },
    { "NeuralSubstrate_Process", Test_NeuralProcess },
    { "NeuralSubstrate_Learn", Test_NeuralLearn },
    { "NeuralAdversarial_NullInput", Test_NeuralAdversarialNull },
    { "Adversarial_ZeroSize", Test_AdversarialZeroSize },
    { "Adversarial_ExtremeValues", Test_AdversarialExtremeValues },
    { "Recovery_AfterFailure", Test_RecoveryAfterNeuralFailure },
    { "EvolutionEngine_Initialize", Test_EvolutionInit },
    { "TrainingPipeline_TrainStep", Test_TrainingStep },
    { "Stress_ManyCycles", Test_StressManyCycles },
    { "RoleBoundary_NoViolation", Test_RoleBoundary_NoViolation },
    { "RoleBoundary_DetectsViolation", Test_RoleBoundary_DetectsViolation },
    { "TaskOracle_Evaluate", Test_TaskOracle_Evaluate },
    { "ResourceGovernor_ResetThrottle", Test_ResourceGovernor_ResetThrottle },
};
static const uint32_t s_self_test_dispatch_count =
    sizeof(s_self_test_dispatch) / sizeof(s_self_test_dispatch[0]);

static void RunOneWithRaijinContext(SelfTestReport* report, SelfTestFn fn) {
    RoleBoundaryContext* rbc = RoleBoundary_GetGlobal();
    if (rbc) RoleBoundary_Enter(rbc, "raijin.test", ROLE_OWNER_RAIJIN);
    fn(report);
    if (rbc) RoleBoundary_Exit(rbc, "raijin.test");
}

NTSTATUS SelfTest_RunAll(SelfTestReport* report) {
    if (!report || !report->results) return STATUS_INVALID_PARAMETER;
    report->count = report->passed = report->failed = 0;
    report->total_ms = 0;

    RoleBoundaryContext* rbc = RoleBoundary_GetGlobal();
    if (rbc) RoleBoundary_AssertCursor(rbc);

    RunOneWithRaijinContext(report, Test_NeuralInit);
    RunOneWithRaijinContext(report, Test_NeuralProcess);
    RunOneWithRaijinContext(report, Test_NeuralLearn);
    RunOneWithRaijinContext(report, Test_NeuralAdversarialNull);
    RunOneWithRaijinContext(report, Test_AdversarialZeroSize);
    RunOneWithRaijinContext(report, Test_AdversarialExtremeValues);
    RunOneWithRaijinContext(report, Test_RecoveryAfterNeuralFailure);
    RunOneWithRaijinContext(report, Test_EvolutionInit);
    RunOneWithRaijinContext(report, Test_TrainingStep);
    RunOneWithRaijinContext(report, Test_StressManyCycles);
    Test_RoleBoundary_NoViolation(report);
    Test_RoleBoundary_DetectsViolation(report);
    Test_TaskOracle_Evaluate(report);
    Test_ResourceGovernor_ResetThrottle(report);

    return STATUS_SUCCESS;
}

NTSTATUS SelfTest_RunOne(SelfTestReport* report, const char* test_name) {
    if (!report || !report->results || !test_name) return STATUS_INVALID_PARAMETER;
    for (uint32_t i = 0; i < s_self_test_dispatch_count; i++) {
        if (strcmp(s_self_test_dispatch[i].name, test_name) == 0) {
            return s_self_test_dispatch[i].fn(report);
        }
    }
    SelfTestReport_Add(report, test_name, false, "Unknown test name", 0);
    return STATUS_SUCCESS;
}

bool SelfTest_AllPassed(const SelfTestReport* report) {
    return report && report->failed == 0 && report->count > 0;
}

void SelfTest_PrintReport(const SelfTestReport* report) {
    if (!report || !report->results) return;
    printf("\n=== Raijin Self-Test Report ===\n");
    printf("Total: %u | Passed: %u | Failed: %u | Time: %llu ms\n\n",
        report->count, report->passed, report->failed, (unsigned long long)report->total_ms);
    for (uint32_t i = 0; i < report->count; i++) {
        const SelfTestResult* r = &report->results[i];
        printf("  [%s] %s - %s (%llu ms)\n",
            r->passed ? "PASS" : "FAIL", r->name, r->message, (unsigned long long)r->duration_ms);
    }
    printf("===============================\n\n");
}
