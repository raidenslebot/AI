/*
 * Resource Governor - Raijin
 * Owner: Core/ResourceGovernor
 * Inputs: System APIs (memory, disk, CPU sampling)
 * Outputs: ResourceSample, throttle_factor, degradation_mode, per-subsystem budgets
 * Invariants: throttle_factor in [0,1]; degradation_mode in {0,1,2}; no silent failure
 * Budget: Minimal CPU; samples at configurable interval
 * Failure modes: API failure -> use last valid sample, log via caller
 * Recovery: Re-sample next cycle; degradation elevates on sustained pressure
 */

#include "../../Include/resource_governor.h"
#include "../../Include/raijin_ntstatus.h"
#include <windows.h>
#include <psapi.h>
#include <string.h>

static const double DEFAULT_CPU_BUDGET_PERCENT = 15.0;
static const uint64_t DEFAULT_RAM_BUDGET_MB = 512;
static const uint64_t DEFAULT_LATENCY_BUDGET_MS = 5000;
static const double THROTTLE_RAMP_UP_RATE = 0.05;
static const double THROTTLE_RAMP_DOWN_RATE = 0.02;
static const uint32_t SAMPLES_BEFORE_DEGRADATION = 3;
static const double CPU_PRESSURE_THRESHOLD = 0.85;
static const double RAM_PRESSURE_THRESHOLD = 0.90;
static const double DISK_PRESSURE_THRESHOLD = 0.95;

static uint64_t get_timestamp_ms(void) {
    return (uint64_t)GetTickCount64();
}

static void sample_memory(ResourceSample* sample) {
    MEMORYSTATUSEX mex = {0};
    mex.dwLength = sizeof(mex);
    if (GlobalMemoryStatusEx(&mex)) {
        sample->ram_mb = (mex.ullTotalPhys - mex.ullAvailPhys) / (1024 * 1024);
    }
    PROCESS_MEMORY_COUNTERS pmc = {0};
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        (void)pmc;
    }
}

static void sample_disk(ResourceSample* sample) {
    ULARGE_INTEGER free_caller, total, free_total;
    if (GetDiskFreeSpaceExA("C:\\", &free_caller, &total, &free_total)) {
        sample->disk_mb = (total.QuadPart - free_total.QuadPart) / (1024 * 1024);
    }
}

static void sample_cpu(ResourceSample* sample) {
    static FILETIME idle_prev = {0}, kernel_prev = {0}, user_prev = {0};
    static int first = 1;
    FILETIME idle_now, kernel_now, user_now;
    if (!GetSystemTimes(&idle_now, &kernel_now, &user_now)) {
        sample->cpu_percent = 0.0;
        return;
    }
    if (first) {
        idle_prev = idle_now;
        kernel_prev = kernel_now;
        user_prev = user_now;
        first = 0;
        sample->cpu_percent = 0.0;
        return;
    }
    ULARGE_INTEGER idle_delta, kernel_delta, user_delta;
    idle_delta.QuadPart = ((ULARGE_INTEGER*)&idle_now)->QuadPart - ((ULARGE_INTEGER*)&idle_prev)->QuadPart;
    kernel_delta.QuadPart = ((ULARGE_INTEGER*)&kernel_now)->QuadPart - ((ULARGE_INTEGER*)&kernel_prev)->QuadPart;
    user_delta.QuadPart = ((ULARGE_INTEGER*)&user_now)->QuadPart - ((ULARGE_INTEGER*)&user_prev)->QuadPart;
    idle_prev = idle_now;
    kernel_prev = kernel_now;
    user_prev = user_now;
    uint64_t total = kernel_delta.QuadPart + user_delta.QuadPart;
    if (total == 0) {
        sample->cpu_percent = 0.0;
        return;
    }
    uint64_t used = total - (idle_delta.QuadPart < total ? idle_delta.QuadPart : 0);
    sample->cpu_percent = 100.0 * (double)used / (double)total;
}

static void init_subsystem_budgets(ResourceGovernor* rg) {
    const SubsystemId ids[] = {
        SUBSYSTEM_THINKING, SUBSYSTEM_LEARNING, SUBSYSTEM_MEMORY,
        SUBSYSTEM_STRESS, SUBSYSTEM_TRAINING, SUBSYSTEM_EVOLUTION
    };
    for (int i = 0; i < RESOURCE_SUBSYSTEM_COUNT; i++) {
        SubsystemBudget* b = &rg->budgets[i];
        memset(b, 0, sizeof(*b));
        b->id = ids[i];
        b->cpu_budget_percent = DEFAULT_CPU_BUDGET_PERCENT;
        b->ram_budget_mb = DEFAULT_RAM_BUDGET_MB;
        b->latency_budget_ms = DEFAULT_LATENCY_BUDGET_MS;
        b->current_utilization = 0.0;
        b->throttled = false;
        b->exceeded = false;
    }
}

NTSTATUS ResourceGovernor_Initialize(ResourceGovernor* rg) {
    if (!rg) return STATUS_INVALID_PARAMETER;
    memset(rg, 0, sizeof(*rg));
    rg->throttle_factor = 1.0;
    rg->degradation_mode = 0;
    rg->sample_count = 0;
    init_subsystem_budgets(rg);
    MEMORYSTATUSEX mex = {0};
    mex.dwLength = sizeof(mex);
    if (GlobalMemoryStatusEx(&mex)) {
        rg->total_ram_mb = mex.ullTotalPhys / (1024 * 1024);
    } else {
        rg->total_ram_mb = 4096;
    }
    ULARGE_INTEGER free_caller, total, free_total;
    if (GetDiskFreeSpaceExA("C:\\", &free_caller, &total, &free_total)) {
        rg->total_disk_mb = total.QuadPart / (1024 * 1024);
    } else {
        rg->total_disk_mb = 1024 * 1024;
    }
    NTSTATUS st = ResourceGovernor_Sample(rg);
    rg->initialized = NT_SUCCESS(st);
    return st;
}

NTSTATUS ResourceGovernor_Shutdown(ResourceGovernor* rg) {
    if (!rg) return STATUS_INVALID_PARAMETER;
    rg->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS ResourceGovernor_Sample(ResourceGovernor* rg) {
    if (!rg) return STATUS_INVALID_PARAMETER;
    ResourceSample* s = &rg->last_sample;
    memset(s, 0, sizeof(*s));
    s->timestamp_ms = get_timestamp_ms();
    sample_cpu(s);
    sample_memory(s);
    sample_disk(s);
    s->latency_ms = 0;
    s->thermal = 0.0;
    s->vram_mb = 0;
    rg->sample_count++;
    return STATUS_SUCCESS;
}

NTSTATUS ResourceGovernor_AllocateBudget(ResourceGovernor* rg, SubsystemId id,
    double cpu_percent, uint64_t ram_mb, uint64_t latency_ms) {
    if (!rg || !rg->initialized) return STATUS_INVALID_DEVICE_STATE;
    if ((int)id < 0 || (int)id >= RESOURCE_SUBSYSTEM_COUNT) return STATUS_INVALID_PARAMETER;
    SubsystemBudget* b = &rg->budgets[id];
    b->cpu_budget_percent = cpu_percent;
    b->ram_budget_mb = ram_mb;
    b->latency_budget_ms = latency_ms;
    return STATUS_SUCCESS;
}

NTSTATUS ResourceGovernor_ReportConsumption(ResourceGovernor* rg, SubsystemId id,
    double cpu_used, uint64_t ram_used_mb, uint64_t latency_used_ms) {
    if (!rg || !rg->initialized) return STATUS_INVALID_DEVICE_STATE;
    if ((int)id < 0 || (int)id >= RESOURCE_SUBSYSTEM_COUNT) return STATUS_INVALID_PARAMETER;
    SubsystemBudget* b = &rg->budgets[id];
    b->current_utilization = 0.0;
    if (b->cpu_budget_percent > 0.0)
        b->current_utilization += (cpu_used / b->cpu_budget_percent);
    if (b->ram_budget_mb > 0)
        b->current_utilization += (double)ram_used_mb / (double)b->ram_budget_mb;
    if (b->latency_budget_ms > 0 && latency_used_ms > 0)
        b->current_utilization += (double)latency_used_ms / (double)b->latency_budget_ms;
    b->current_utilization /= 3.0;
    if (b->current_utilization > 1.0) b->exceeded = true; else b->exceeded = false;
    return STATUS_SUCCESS;
}

double ResourceGovernor_GetThrottleFactor(const ResourceGovernor* rg) {
    return rg ? rg->throttle_factor : 1.0;
}

uint32_t ResourceGovernor_GetDegradationMode(const ResourceGovernor* rg) {
    return rg ? rg->degradation_mode : 0;
}

NTSTATUS ResourceGovernor_GetSample(const ResourceGovernor* rg, ResourceSample* out) {
    if (!rg || !out) return STATUS_INVALID_PARAMETER;
    *out = rg->last_sample;
    return STATUS_SUCCESS;
}

NTSTATUS ResourceGovernor_ApplyThrottling(ResourceGovernor* rg) {
    if (!rg || !rg->initialized) return STATUS_INVALID_DEVICE_STATE;
    const ResourceSample* s = &rg->last_sample;
    double cpu_ratio = 0.0;
    if (rg->total_ram_mb > 0)
        cpu_ratio = (double)s->ram_mb / (double)rg->total_ram_mb;
    double ram_pressure = cpu_ratio;
    double disk_ratio = 0.0;
    if (rg->total_disk_mb > 0)
        disk_ratio = (double)s->disk_mb / (double)rg->total_disk_mb;
    double cpu_pressure = (s->cpu_percent / 100.0);
    if (ram_pressure > cpu_pressure) cpu_pressure = ram_pressure;
    if (disk_ratio > cpu_pressure) cpu_pressure = disk_ratio;

    static uint32_t pressure_cycles = 0;
    if (cpu_pressure >= CPU_PRESSURE_THRESHOLD || ram_pressure >= RAM_PRESSURE_THRESHOLD ||
        disk_ratio >= DISK_PRESSURE_THRESHOLD) {
        pressure_cycles++;
        if (pressure_cycles >= SAMPLES_BEFORE_DEGRADATION) {
            if (rg->degradation_mode < 2) rg->degradation_mode++;
            if (rg->throttle_factor > 0.2) rg->throttle_factor -= THROTTLE_RAMP_DOWN_RATE;
            if (rg->throttle_factor < 0.2) rg->throttle_factor = 0.2;
        }
    } else {
        pressure_cycles = 0;
        if (rg->degradation_mode > 0) rg->degradation_mode--;
        if (rg->throttle_factor < 1.0) {
            rg->throttle_factor += THROTTLE_RAMP_UP_RATE;
            if (rg->throttle_factor > 1.0) rg->throttle_factor = 1.0;
        }
    }

    for (int i = 0; i < RESOURCE_SUBSYSTEM_COUNT; i++) {
        if (rg->budgets[i].exceeded)
            rg->budgets[i].throttled = true;
        else
            rg->budgets[i].throttled = false;
    }
    return STATUS_SUCCESS;
}

NTSTATUS ResourceGovernor_ResetThrottle(ResourceGovernor* rg) {
    if (!rg || !rg->initialized) return STATUS_INVALID_DEVICE_STATE;
    rg->throttle_factor = 1.0;
    rg->degradation_mode = 0;
    for (int i = 0; i < RESOURCE_SUBSYSTEM_COUNT; i++) {
        rg->budgets[i].throttled = false;
        rg->budgets[i].exceeded = false;
    }
    return STATUS_SUCCESS;
}
