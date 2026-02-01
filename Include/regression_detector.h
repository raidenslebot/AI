#ifndef RAIJIN_REGRESSION_DETECTOR_H
#define RAIJIN_REGRESSION_DETECTOR_H

#include "raijin_ntstatus.h"
#include "dominance_metrics.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define REGRESSION_WINDOW_MIN 16
#define REGRESSION_DEGENERATION_THRESHOLD 0.15

typedef enum {
    REGRESSION_NONE = 0,
    REGRESSION_FITNESS_DROP = 1,
    REGRESSION_LOSS_INCREASE = 2,
    REGRESSION_COHERENCE_DROP = 3,
    REGRESSION_DOMINANCE_DROP = 4,
    DEGENERATION_SUSTAINED = 5
} RegressionType;

typedef struct RegressionEvent {
    RegressionType type;
    double severity;       /* 0-1 */
    double baseline_value;
    double current_value;
    uint64_t step_count;
    uint64_t timestamp_ms;
    char description[256];
} RegressionEvent;

typedef struct RegressionDetector {
    DominanceMetrics* dominance_metrics;
    RegressionEvent last_event;
    double* fitness_history;
    double* loss_history;
    double* dominance_history;
    uint32_t history_count;
    uint32_t history_capacity;
    uint32_t consecutive_regressions;
    bool regression_detected;
    bool degeneration_detected;
    bool initialized;
} RegressionDetector;

NTSTATUS RegressionDetector_Initialize(RegressionDetector* rd, DominanceMetrics* dm);
NTSTATUS RegressionDetector_Shutdown(RegressionDetector* rd);

NTSTATUS RegressionDetector_Update(RegressionDetector* rd);

bool RegressionDetector_IsRegressionDetected(const RegressionDetector* rd);
bool RegressionDetector_IsDegenerationDetected(const RegressionDetector* rd);
void RegressionDetector_GetLastEvent(const RegressionDetector* rd, RegressionEvent* out);

NTSTATUS RegressionDetector_Reset(RegressionDetector* rd);

#endif
