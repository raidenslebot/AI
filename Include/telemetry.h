#ifndef RAIJIN_TELEMETRY_H
#define RAIJIN_TELEMETRY_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#define TELEMETRY_LOG_MAX 4096
#define TELEMETRY_METRICS_PATH "data/telemetry.json"
#define TELEMETRY_LOG_DIR "data/logs"

typedef enum TelemetryLevel {
    TELEMETRY_DEBUG = 0,
    TELEMETRY_INFO = 1,
    TELEMETRY_WARN = 2,
    TELEMETRY_ERROR = 3
} TelemetryLevel;

typedef struct TelemetryContext {
    char log_path[512];
    char metrics_path[512];
    FILE* log_file;
    bool initialized;
    uint64_t event_count;
} TelemetryContext;

NTSTATUS Telemetry_Initialize(TelemetryContext* ctx, const char* base_dir);
NTSTATUS Telemetry_Shutdown(TelemetryContext* ctx);

NTSTATUS Telemetry_Log(TelemetryContext* ctx, TelemetryLevel level,
    const char* component, const char* message);
NTSTATUS Telemetry_LogFormat(TelemetryContext* ctx, TelemetryLevel level,
    const char* component, const char* fmt, ...);

NTSTATUS Telemetry_RecordMetrics(TelemetryContext* ctx,
    double loss, double fitness, float entropy,
    uint64_t step_count, uint64_t generation,
    uint64_t batch_time_ms, uint64_t memory_mb);
NTSTATUS Telemetry_Flush(TelemetryContext* ctx);

#endif
