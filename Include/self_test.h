#ifndef RAIJIN_SELF_TEST_H
#define RAIJIN_SELF_TEST_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SELF_TEST_MAX_NAME 128
#define SELF_TEST_MAX_MESSAGE 256

typedef struct SelfTestResult {
    char name[SELF_TEST_MAX_NAME];
    char message[SELF_TEST_MAX_MESSAGE];
    bool passed;
    uint64_t duration_ms;
} SelfTestResult;

typedef struct SelfTestReport {
    SelfTestResult* results;
    uint32_t count;
    uint32_t capacity;
    uint32_t passed;
    uint32_t failed;
    uint64_t total_ms;
} SelfTestReport;

NTSTATUS SelfTestReport_Initialize(SelfTestReport* report, uint32_t max_tests);
void SelfTestReport_Shutdown(SelfTestReport* report);
NTSTATUS SelfTestReport_Add(SelfTestReport* report, const char* name,
    bool passed, const char* message, uint64_t duration_ms);

NTSTATUS SelfTest_RunAll(SelfTestReport* report);
NTSTATUS SelfTest_RunOne(SelfTestReport* report, const char* test_name);
bool SelfTest_AllPassed(const SelfTestReport* report);
void SelfTest_PrintReport(const SelfTestReport* report);

#endif
