#include "../../Include/telemetry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static const char* LevelString(TelemetryLevel level) {
    switch (level) {
        case TELEMETRY_DEBUG: return "DEBUG";
        case TELEMETRY_INFO:  return "INFO";
        case TELEMETRY_WARN:  return "WARN";
        case TELEMETRY_ERROR: return "ERROR";
        default: return "?";
    }
}

static void Timestamp(char* buf, size_t size) {
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    if (tm)
        strftime(buf, size, "%Y-%m-%dT%H:%M:%S", tm);
    else
        buf[0] = '\0';
}

NTSTATUS Telemetry_Initialize(TelemetryContext* ctx, const char* base_dir) {
    if (!ctx) return STATUS_INVALID_PARAMETER;

    memset(ctx, 0, sizeof(TelemetryContext));

    if (base_dir && base_dir[0]) {
        snprintf(ctx->log_path, sizeof(ctx->log_path), "%s/logs", base_dir);
        snprintf(ctx->metrics_path, sizeof(ctx->metrics_path), "%s/telemetry.json", base_dir);
    } else {
        snprintf(ctx->log_path, sizeof(ctx->log_path), TELEMETRY_LOG_DIR);
        snprintf(ctx->metrics_path, sizeof(ctx->metrics_path), TELEMETRY_METRICS_PATH);
    }

    CreateDirectoryA("data", NULL);
    CreateDirectoryA(ctx->log_path, NULL);

    char path[512];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    if (tm) {
        strftime(path, sizeof(path), "%Y%m%d", tm);
        snprintf(ctx->log_path, sizeof(ctx->log_path), "%s/raijin_%s.log",
            TELEMETRY_LOG_DIR, path);
    } else {
        snprintf(ctx->log_path, sizeof(ctx->log_path), "%s/raijin.log", TELEMETRY_LOG_DIR);
    }

    ctx->log_file = fopen(ctx->log_path, "a");
    if (!ctx->log_file)
        return STATUS_UNSUCCESSFUL;

    ctx->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS Telemetry_Shutdown(TelemetryContext* ctx) {
    if (!ctx) return STATUS_INVALID_PARAMETER;
    if (ctx->log_file) {
        fclose(ctx->log_file);
        ctx->log_file = NULL;
    }
    ctx->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS Telemetry_Log(TelemetryContext* ctx, TelemetryLevel level,
    const char* component, const char* message) {
    if (!ctx || !ctx->initialized || !ctx->log_file) return STATUS_INVALID_PARAMETER;

    char ts[32];
    Timestamp(ts, sizeof(ts));
    fprintf(ctx->log_file, "%s [%s] [%s] %s\n",
        ts, LevelString(level), component ? component : "", message ? message : "");
    fflush(ctx->log_file);
    ctx->event_count++;
    return STATUS_SUCCESS;
}

NTSTATUS Telemetry_LogFormat(TelemetryContext* ctx, TelemetryLevel level,
    const char* component, const char* fmt, ...) {
    if (!ctx || !ctx->initialized || !ctx->log_file) return STATUS_INVALID_PARAMETER;

    char buf[TELEMETRY_LOG_MAX];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    char ts[32];
    Timestamp(ts, sizeof(ts));
    fprintf(ctx->log_file, "%s [%s] [%s] %s\n",
        ts, LevelString(level), component ? component : "", buf);
    fflush(ctx->log_file);
    ctx->event_count++;
    return STATUS_SUCCESS;
}

NTSTATUS Telemetry_RecordMetrics(TelemetryContext* ctx,
    double loss, double fitness, float entropy,
    uint64_t step_count, uint64_t generation,
    uint64_t batch_time_ms, uint64_t memory_mb) {
    if (!ctx) return STATUS_INVALID_PARAMETER;

    FILE* f = fopen(ctx->metrics_path, "w");
    if (!f) return STATUS_UNSUCCESSFUL;

    fprintf(f, "{\n");
    fprintf(f, "  \"loss\": %.6f,\n", loss);
    fprintf(f, "  \"fitness\": %.6f,\n", fitness);
    fprintf(f, "  \"entropy\": %.6f,\n", (double)entropy);
    fprintf(f, "  \"step_count\": %llu,\n", (unsigned long long)step_count);
    fprintf(f, "  \"generation\": %llu,\n", (unsigned long long)generation);
    fprintf(f, "  \"batch_time_ms\": %llu,\n", (unsigned long long)batch_time_ms);
    fprintf(f, "  \"memory_mb\": %llu,\n", (unsigned long long)memory_mb);
    fprintf(f, "  \"event_count\": %llu\n", (unsigned long long)ctx->event_count);
    fprintf(f, "}\n");
    fclose(f);
    return STATUS_SUCCESS;
}

NTSTATUS Telemetry_Flush(TelemetryContext* ctx) {
    if (!ctx || !ctx->log_file) return STATUS_INVALID_PARAMETER;
    fflush(ctx->log_file);
    return STATUS_SUCCESS;
}
