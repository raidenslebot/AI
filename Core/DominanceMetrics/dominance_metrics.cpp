#include "../../Include/dominance_metrics.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

static double LinearRegressionSlope(const double* y, uint32_t n) {
    if (n < 2) return 0.0;
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
    for (uint32_t i = 0; i < n; i++) {
        double x = (double)i;
        sum_x += x;
        sum_y += y[i];
        sum_xy += x * y[i];
        sum_xx += x * x;
    }
    double denom = (double)n * sum_xx - sum_x * sum_x;
    if (fabs(denom) < 1e-20) return 0.0;
    return ((double)n * sum_xy - sum_x * sum_y) / denom;
}

NTSTATUS DominanceMetrics_Initialize(DominanceMetrics* dm,
    NeuralSubstrate* neural,
    EvolutionEngine* evolution,
    TrainingPipeline* pipeline) {
    if (!dm) return STATUS_INVALID_PARAMETER;
    memset(dm, 0, sizeof(DominanceMetrics));
    dm->neural = neural;
    dm->evolution = evolution;
    dm->pipeline = pipeline;
    dm->history_capacity = DOMINANCE_METRICS_HISTORY_LEN;
    dm->history = (DominanceSnapshot*)malloc(sizeof(DominanceSnapshot) * dm->history_capacity);
    if (!dm->history) return STATUS_INSUFFICIENT_RESOURCES;
    memset(dm->history, 0, sizeof(DominanceSnapshot) * dm->history_capacity);
    dm->last_stress_robustness = -1.0;
    dm->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS DominanceMetrics_Shutdown(DominanceMetrics* dm) {
    if (!dm) return STATUS_INVALID_PARAMETER;
    if (dm->history) {
        free(dm->history);
        dm->history = NULL;
    }
    dm->history_count = 0;
    dm->history_capacity = 0;
    dm->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS DominanceMetrics_Update(DominanceMetrics* dm,
    double loss, double fitness, float entropy,
    uint64_t step_count, uint64_t generation,
    uint64_t batch_time_ms, uint64_t memory_mb) {
    if (!dm || !dm->initialized) return STATUS_INVALID_PARAMETER;

    DominanceSnapshot* cur = &dm->current;
    cur->fitness = fitness;
    cur->loss = loss;
    cur->entropy = entropy;
    cur->step_count = step_count;
    cur->generation = generation;
    cur->timestamp_ms = GetTimeMs();

    cur->dominance = fitness * (1.0 - loss);
    if (cur->dominance < 0.0) cur->dominance = 0.0;

    cur->efficiency = (batch_time_ms > 0 && memory_mb > 0)
        ? (1.0 / (1.0 + (double)batch_time_ms / 1000.0 + (double)memory_mb / 4096.0))
        : 0.5;
    if (cur->efficiency > 1.0) cur->efficiency = 1.0;

    cur->coherence = (float)(1.0 - fabs(0.5 - (double)entropy));
    if (cur->coherence < 0.0) cur->coherence = 0.0;
    if (cur->coherence > 1.0) cur->coherence = 1.0;

    cur->adaptability = (dm->history_count >= 1)
        ? (cur->fitness - dm->history[dm->history_count - 1].fitness)
        : 0.0;
    if (cur->adaptability < 0.0) cur->adaptability = 0.0;
    if (cur->adaptability > 1.0) cur->adaptability = 1.0;

    if (dm->history_count < dm->history_capacity) {
        dm->history[dm->history_count++] = *cur;
    } else {
        memmove(dm->history, dm->history + 1, (dm->history_capacity - 1) * sizeof(DominanceSnapshot));
        dm->history[dm->history_capacity - 1] = *cur;
    }

    uint32_t trend_n = (dm->history_count < 64) ? dm->history_count : 64;
    if (trend_n >= 2) {
        double dom_buf[64], eff_buf[64], coh_buf[64], adp_buf[64];
        uint32_t start = (dm->history_count >= trend_n) ? (dm->history_count - trend_n) : 0;
        for (uint32_t i = 0; i < trend_n; i++) {
            dom_buf[i] = dm->history[start + i].dominance;
            eff_buf[i] = dm->history[start + i].efficiency;
            coh_buf[i] = dm->history[start + i].coherence;
            adp_buf[i] = dm->history[start + i].adaptability;
        }
        dm->dominance_trend = LinearRegressionSlope(dom_buf, trend_n);
        dm->efficiency_trend = LinearRegressionSlope(eff_buf, trend_n);
        dm->coherence_trend = LinearRegressionSlope(coh_buf, trend_n);
        dm->adaptability_trend = LinearRegressionSlope(adp_buf, trend_n);
    }

    return STATUS_SUCCESS;
}

void DominanceMetrics_GetCurrent(const DominanceMetrics* dm, DominanceSnapshot* out) {
    if (!dm || !out) return;
    *out = dm->current;
}

void DominanceMetrics_RecordStressResult(DominanceMetrics* dm, double robustness_score) {
    if (!dm || !dm->initialized) return;
    if (robustness_score < 0.0) robustness_score = 0.0;
    if (robustness_score > 1.0) robustness_score = 1.0;
    dm->last_stress_robustness = robustness_score;
}

void DominanceMetrics_GetTrends(const DominanceMetrics* dm,
    double* dominance_trend, double* efficiency_trend,
    double* coherence_trend, double* adaptability_trend) {
    if (!dm) return;
    if (dominance_trend) *dominance_trend = dm->dominance_trend;
    if (efficiency_trend) *efficiency_trend = dm->efficiency_trend;
    if (coherence_trend) *coherence_trend = dm->coherence_trend;
    if (adaptability_trend) *adaptability_trend = dm->adaptability_trend;
}

NTSTATUS DominanceMetrics_Serialize(const DominanceMetrics* dm, char* buf, size_t buf_size) {
    if (!dm || !buf || buf_size < 256) return STATUS_INVALID_PARAMETER;
    int n = snprintf(buf, buf_size,
        "{\"dominance\":%.4f,\"efficiency\":%.4f,\"coherence\":%.4f,\"adaptability\":%.4f,"
        "\"dominance_trend\":%.6f,\"efficiency_trend\":%.6f,\"coherence_trend\":%.6f,\"adaptability_trend\":%.6f,"
        "\"step\":%llu,\"generation\":%llu}",
        dm->current.dominance, dm->current.efficiency, dm->current.coherence, dm->current.adaptability,
        dm->dominance_trend, dm->efficiency_trend, dm->coherence_trend, dm->adaptability_trend,
        (unsigned long long)dm->current.step_count, (unsigned long long)dm->current.generation);
    return (n > 0 && (size_t)n < buf_size) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}
