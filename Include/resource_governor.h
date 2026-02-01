#ifndef RAIJIN_RESOURCE_GOVERNOR_H
#define RAIJIN_RESOURCE_GOVERNOR_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RESOURCE_SUBSYSTEM_COUNT 6
#define RESOURCE_BUDGET_SLOTS 32

typedef enum {
    RESOURCE_CPU = 0,
    RESOURCE_RAM_MB = 1,
    RESOURCE_DISK_MB = 2,
    RESOURCE_LATENCY_MS = 3,
    RESOURCE_THERMAL = 4,
    RESOURCE_VRAM_MB = 5
} ResourceType;

typedef enum {
    SUBSYSTEM_THINKING = 0,
    SUBSYSTEM_LEARNING = 1,
    SUBSYSTEM_MEMORY = 2,
    SUBSYSTEM_STRESS = 3,
    SUBSYSTEM_TRAINING = 4,
    SUBSYSTEM_EVOLUTION = 5
} SubsystemId;

typedef struct ResourceSample {
    double cpu_percent;
    uint64_t ram_mb;
    uint64_t disk_mb;
    uint64_t latency_ms;
    double thermal;   /* 0-1 normalized */
    uint64_t vram_mb;
    uint64_t timestamp_ms;
} ResourceSample;

typedef struct SubsystemBudget {
    SubsystemId id;
    double cpu_budget_percent;
    uint64_t ram_budget_mb;
    uint64_t latency_budget_ms;
    double current_utilization;  /* 0-1 */
    bool throttled;
    bool exceeded;
} SubsystemBudget;

typedef struct ResourceGovernor {
    ResourceSample last_sample;
    SubsystemBudget budgets[RESOURCE_SUBSYSTEM_COUNT];
    double throttle_factor;     /* 0-1: 1 = no throttle, 0 = max throttle */
    uint32_t degradation_mode;  /* 0=normal, 1=reduced, 2=minimal */
    uint64_t total_ram_mb;
    uint64_t total_disk_mb;
    uint32_t sample_count;
    bool initialized;
} ResourceGovernor;

NTSTATUS ResourceGovernor_Initialize(ResourceGovernor* rg);
NTSTATUS ResourceGovernor_Shutdown(ResourceGovernor* rg);

NTSTATUS ResourceGovernor_Sample(ResourceGovernor* rg);

NTSTATUS ResourceGovernor_AllocateBudget(ResourceGovernor* rg, SubsystemId id,
    double cpu_percent, uint64_t ram_mb, uint64_t latency_ms);

NTSTATUS ResourceGovernor_ReportConsumption(ResourceGovernor* rg, SubsystemId id,
    double cpu_used, uint64_t ram_used_mb, uint64_t latency_used_ms);

double ResourceGovernor_GetThrottleFactor(const ResourceGovernor* rg);
uint32_t ResourceGovernor_GetDegradationMode(const ResourceGovernor* rg);
NTSTATUS ResourceGovernor_GetSample(const ResourceGovernor* rg, ResourceSample* out);

NTSTATUS ResourceGovernor_ApplyThrottling(ResourceGovernor* rg);

NTSTATUS ResourceGovernor_ResetThrottle(ResourceGovernor* rg);

#endif
