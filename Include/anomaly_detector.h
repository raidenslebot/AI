#ifndef RAIJIN_ANOMALY_DETECTOR_H
#define RAIJIN_ANOMALY_DETECTOR_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ANOMALY_HISTORY_LEN 256
#define ANOMALY_Z_SCORE_THRESHOLD 3.0

typedef struct AnomalyEvent {
    uint32_t metric_id;    /* 0=loss, 1=fitness, 2=entropy, 3=latency, 4=memory */
    double value;
    double expected_mean;
    double z_score;
    uint64_t timestamp_ms;
    char description[128];
} AnomalyEvent;

typedef struct AnomalyDetector {
    double* loss_history;
    double* fitness_history;
    double* entropy_history;
    double* latency_history;
    double* memory_history;
    uint32_t count;
    uint32_t capacity;
    AnomalyEvent last_anomaly;
    bool anomaly_detected;
    bool initialized;
} AnomalyDetector;

NTSTATUS AnomalyDetector_Initialize(AnomalyDetector* ad);
NTSTATUS AnomalyDetector_Shutdown(AnomalyDetector* ad);

NTSTATUS AnomalyDetector_Record(AnomalyDetector* ad,
    double loss, double fitness, float entropy,
    uint64_t batch_time_ms, uint64_t memory_mb);

NTSTATUS AnomalyDetector_Update(AnomalyDetector* ad);

bool AnomalyDetector_IsAnomalyDetected(const AnomalyDetector* ad);
void AnomalyDetector_GetLastAnomaly(const AnomalyDetector* ad, AnomalyEvent* out);

#endif
