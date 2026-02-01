#include "../../Include/regression_detector.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

#define REGRESSION_HISTORY_CAP 512

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

NTSTATUS RegressionDetector_Initialize(RegressionDetector* rd, DominanceMetrics* dm) {
    if (!rd || !dm) return STATUS_INVALID_PARAMETER;
    memset(rd, 0, sizeof(RegressionDetector));
    rd->dominance_metrics = dm;
    rd->history_capacity = REGRESSION_HISTORY_CAP;
    rd->fitness_history = (double*)malloc(sizeof(double) * rd->history_capacity);
    rd->loss_history = (double*)malloc(sizeof(double) * rd->history_capacity);
    rd->dominance_history = (double*)malloc(sizeof(double) * rd->history_capacity);
    if (!rd->fitness_history || !rd->loss_history || !rd->dominance_history) {
        if (rd->fitness_history) free(rd->fitness_history);
        if (rd->loss_history) free(rd->loss_history);
        if (rd->dominance_history) free(rd->dominance_history);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(rd->fitness_history, 0, sizeof(double) * rd->history_capacity);
    memset(rd->loss_history, 0, sizeof(double) * rd->history_capacity);
    memset(rd->dominance_history, 0, sizeof(double) * rd->history_capacity);
    rd->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS RegressionDetector_Shutdown(RegressionDetector* rd) {
    if (!rd) return STATUS_INVALID_PARAMETER;
    free(rd->fitness_history);
    rd->fitness_history = NULL;
    free(rd->loss_history);
    rd->loss_history = NULL;
    free(rd->dominance_history);
    rd->dominance_history = NULL;
    rd->history_count = 0;
    rd->history_capacity = 0;
    rd->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS RegressionDetector_Update(RegressionDetector* rd) {
    if (!rd || !rd->initialized || !rd->dominance_metrics) return STATUS_INVALID_PARAMETER;

    DominanceSnapshot cur;
    DominanceMetrics_GetCurrent(rd->dominance_metrics, &cur);

    if (rd->history_count < rd->history_capacity) {
        rd->fitness_history[rd->history_count] = cur.fitness;
        rd->loss_history[rd->history_count] = cur.loss;
        rd->dominance_history[rd->history_count] = cur.dominance;
        rd->history_count++;
    } else {
        memmove(rd->fitness_history, rd->fitness_history + 1, (rd->history_capacity - 1) * sizeof(double));
        memmove(rd->loss_history, rd->loss_history + 1, (rd->history_capacity - 1) * sizeof(double));
        memmove(rd->dominance_history, rd->dominance_history + 1, (rd->history_capacity - 1) * sizeof(double));
        rd->fitness_history[rd->history_capacity - 1] = cur.fitness;
        rd->loss_history[rd->history_capacity - 1] = cur.loss;
        rd->dominance_history[rd->history_capacity - 1] = cur.dominance;
    }

    rd->regression_detected = false;
    rd->last_event.type = REGRESSION_NONE;
    rd->last_event.severity = 0.0;

    if (rd->history_count < REGRESSION_WINDOW_MIN) return STATUS_SUCCESS;

    uint32_t n = rd->history_count;
    uint32_t half = n / 2;
    double recent_fitness = 0.0, recent_loss = 0.0, recent_dom = 0.0;
    double baseline_fitness = 0.0, baseline_loss = 0.0, baseline_dom = 0.0;
    for (uint32_t i = n - half; i < n; i++) {
        recent_fitness += rd->fitness_history[i];
        recent_loss += rd->loss_history[i];
        recent_dom += rd->dominance_history[i];
    }
    recent_fitness /= half;
    recent_loss /= half;
    recent_dom /= half;
    for (uint32_t i = 0; i < half; i++) {
        baseline_fitness += rd->fitness_history[i];
        baseline_loss += rd->loss_history[i];
        baseline_dom += rd->dominance_history[i];
    }
    baseline_fitness /= half;
    baseline_loss /= half;
    baseline_dom /= half;

    double fitness_drop = baseline_fitness - recent_fitness;
    double loss_rise = recent_loss - baseline_loss;
    double dom_drop = baseline_dom - recent_dom;

    if (fitness_drop > REGRESSION_DEGENERATION_THRESHOLD) {
        rd->regression_detected = true;
        rd->last_event.type = REGRESSION_FITNESS_DROP;
        rd->last_event.severity = fmin(1.0, fitness_drop / 0.5);
        rd->last_event.baseline_value = baseline_fitness;
        rd->last_event.current_value = recent_fitness;
        rd->last_event.step_count = cur.step_count;
        rd->last_event.timestamp_ms = GetTimeMs();
        snprintf(rd->last_event.description, sizeof(rd->last_event.description),
            "Fitness regression: baseline=%.4f recent=%.4f drop=%.4f", baseline_fitness, recent_fitness, fitness_drop);
        rd->consecutive_regressions++;
    } else if (loss_rise > REGRESSION_DEGENERATION_THRESHOLD) {
        rd->regression_detected = true;
        rd->last_event.type = REGRESSION_LOSS_INCREASE;
        rd->last_event.severity = fmin(1.0, loss_rise / 0.5);
        rd->last_event.baseline_value = baseline_loss;
        rd->last_event.current_value = recent_loss;
        rd->last_event.step_count = cur.step_count;
        rd->last_event.timestamp_ms = GetTimeMs();
        snprintf(rd->last_event.description, sizeof(rd->last_event.description),
            "Loss regression: baseline=%.4f recent=%.4f rise=%.4f", baseline_loss, recent_loss, loss_rise);
        rd->consecutive_regressions++;
    } else if (dom_drop > REGRESSION_DEGENERATION_THRESHOLD) {
        rd->regression_detected = true;
        rd->last_event.type = REGRESSION_DOMINANCE_DROP;
        rd->last_event.severity = fmin(1.0, dom_drop / 0.5);
        rd->last_event.baseline_value = baseline_dom;
        rd->last_event.current_value = recent_dom;
        rd->last_event.step_count = cur.step_count;
        rd->last_event.timestamp_ms = GetTimeMs();
        snprintf(rd->last_event.description, sizeof(rd->last_event.description),
            "Dominance regression: baseline=%.4f recent=%.4f drop=%.4f", baseline_dom, recent_dom, dom_drop);
        rd->consecutive_regressions++;
    } else {
        rd->consecutive_regressions = 0;
    }

    if (rd->consecutive_regressions >= 5) {
        rd->degeneration_detected = true;
        rd->last_event.type = DEGENERATION_SUSTAINED;
        rd->last_event.severity = 1.0;
        snprintf(rd->last_event.description, sizeof(rd->last_event.description),
            "Sustained degeneration: %u consecutive regressions", rd->consecutive_regressions);
    } else {
        rd->degeneration_detected = false;
    }

    return STATUS_SUCCESS;
}

bool RegressionDetector_IsRegressionDetected(const RegressionDetector* rd) {
    return rd && rd->regression_detected;
}

bool RegressionDetector_IsDegenerationDetected(const RegressionDetector* rd) {
    return rd && rd->degeneration_detected;
}

void RegressionDetector_GetLastEvent(const RegressionDetector* rd, RegressionEvent* out) {
    if (!rd || !out) return;
    *out = rd->last_event;
}

NTSTATUS RegressionDetector_Reset(RegressionDetector* rd) {
    if (!rd) return STATUS_INVALID_PARAMETER;
    rd->regression_detected = false;
    rd->degeneration_detected = false;
    rd->consecutive_regressions = 0;
    rd->last_event.type = REGRESSION_NONE;
    rd->last_event.severity = 0.0;
    return STATUS_SUCCESS;
}
