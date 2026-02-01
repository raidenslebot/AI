#include "../../Include/anomaly_detector.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

#define ANOMALY_CAP 256

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

static void ComputeMeanStd(const double* x, uint32_t n, double* mean, double* std_dev) {
    *mean = 0.0;
    *std_dev = 0.0;
    if (n == 0) return;
    for (uint32_t i = 0; i < n; i++) *mean += x[i];
    *mean /= n;
    for (uint32_t i = 0; i < n; i++) {
        double d = x[i] - *mean;
        *std_dev += d * d;
    }
    *std_dev = (n > 1) ? sqrt(*std_dev / (n - 1)) : 0.0;
}

static double ZScore(double value, double mean, double std_dev) {
    if (std_dev < 1e-20) return 0.0;
    return fabs((value - mean) / std_dev);
}

NTSTATUS AnomalyDetector_Initialize(AnomalyDetector* ad) {
    if (!ad) return STATUS_INVALID_PARAMETER;
    memset(ad, 0, sizeof(AnomalyDetector));
    ad->capacity = ANOMALY_CAP;
    ad->loss_history = (double*)malloc(sizeof(double) * ad->capacity);
    ad->fitness_history = (double*)malloc(sizeof(double) * ad->capacity);
    ad->entropy_history = (double*)malloc(sizeof(double) * ad->capacity);
    ad->latency_history = (double*)malloc(sizeof(double) * ad->capacity);
    ad->memory_history = (double*)malloc(sizeof(double) * ad->capacity);
    if (!ad->loss_history || !ad->fitness_history || !ad->entropy_history ||
        !ad->latency_history || !ad->memory_history) {
        free(ad->loss_history);
        free(ad->fitness_history);
        free(ad->entropy_history);
        free(ad->latency_history);
        free(ad->memory_history);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(ad->loss_history, 0, sizeof(double) * ad->capacity);
    memset(ad->fitness_history, 0, sizeof(double) * ad->capacity);
    memset(ad->entropy_history, 0, sizeof(double) * ad->capacity);
    memset(ad->latency_history, 0, sizeof(double) * ad->capacity);
    memset(ad->memory_history, 0, sizeof(double) * ad->capacity);
    ad->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS AnomalyDetector_Shutdown(AnomalyDetector* ad) {
    if (!ad) return STATUS_INVALID_PARAMETER;
    free(ad->loss_history);
    free(ad->fitness_history);
    free(ad->entropy_history);
    free(ad->latency_history);
    free(ad->memory_history);
    ad->loss_history = ad->fitness_history = ad->entropy_history = NULL;
    ad->latency_history = ad->memory_history = NULL;
    ad->count = 0;
    ad->capacity = 0;
    ad->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS AnomalyDetector_Record(AnomalyDetector* ad,
    double loss, double fitness, float entropy,
    uint64_t batch_time_ms, uint64_t memory_mb) {
    if (!ad || !ad->initialized) return STATUS_INVALID_PARAMETER;

    if (ad->count < ad->capacity) {
        ad->loss_history[ad->count] = loss;
        ad->fitness_history[ad->count] = fitness;
        ad->entropy_history[ad->count] = (double)entropy;
        ad->latency_history[ad->count] = (double)batch_time_ms;
        ad->memory_history[ad->count] = (double)memory_mb;
        ad->count++;
    } else {
        memmove(ad->loss_history, ad->loss_history + 1, (ad->capacity - 1) * sizeof(double));
        memmove(ad->fitness_history, ad->fitness_history + 1, (ad->capacity - 1) * sizeof(double));
        memmove(ad->entropy_history, ad->entropy_history + 1, (ad->capacity - 1) * sizeof(double));
        memmove(ad->latency_history, ad->latency_history + 1, (ad->capacity - 1) * sizeof(double));
        memmove(ad->memory_history, ad->memory_history + 1, (ad->capacity - 1) * sizeof(double));
        ad->loss_history[ad->capacity - 1] = loss;
        ad->fitness_history[ad->capacity - 1] = fitness;
        ad->entropy_history[ad->capacity - 1] = (double)entropy;
        ad->latency_history[ad->capacity - 1] = (double)batch_time_ms;
        ad->memory_history[ad->capacity - 1] = (double)memory_mb;
    }
    return STATUS_SUCCESS;
}

NTSTATUS AnomalyDetector_Update(AnomalyDetector* ad) {
    if (!ad || !ad->initialized || ad->count < 16) return STATUS_INVALID_PARAMETER;

    ad->anomaly_detected = false;
    ad->last_anomaly.metric_id = (uint32_t)-1;

    uint32_t n = ad->count;
    double loss_mean, loss_std, fit_mean, fit_std, ent_mean, ent_std;
    double lat_mean, lat_std, mem_mean, mem_std;
    ComputeMeanStd(ad->loss_history, n, &loss_mean, &loss_std);
    ComputeMeanStd(ad->fitness_history, n, &fit_mean, &fit_std);
    ComputeMeanStd(ad->entropy_history, n, &ent_mean, &ent_std);
    ComputeMeanStd(ad->latency_history, n, &lat_mean, &lat_std);
    ComputeMeanStd(ad->memory_history, n, &mem_mean, &mem_std);

    double loss = ad->loss_history[n - 1];
    double fitness = ad->fitness_history[n - 1];
    double entropy = ad->entropy_history[n - 1];
    double latency = ad->latency_history[n - 1];
    double memory = ad->memory_history[n - 1];

    double z_loss = ZScore(loss, loss_mean, loss_std);
    double z_fit = ZScore(fitness, fit_mean, fit_std);
    double z_ent = ZScore(entropy, ent_mean, ent_std);
    double z_lat = ZScore(latency, lat_mean, lat_std);
    double z_mem = ZScore(memory, mem_mean, mem_std);

    if (z_loss >= ANOMALY_Z_SCORE_THRESHOLD) {
        ad->anomaly_detected = true;
        ad->last_anomaly.metric_id = 0;
        ad->last_anomaly.value = loss;
        ad->last_anomaly.expected_mean = loss_mean;
        ad->last_anomaly.z_score = z_loss;
        ad->last_anomaly.timestamp_ms = GetTimeMs();
        snprintf(ad->last_anomaly.description, sizeof(ad->last_anomaly.description),
            "Loss anomaly: value=%.4f mean=%.4f z=%.2f", loss, loss_mean, z_loss);
    }
    if (z_fit >= ANOMALY_Z_SCORE_THRESHOLD) {
        ad->anomaly_detected = true;
        ad->last_anomaly.metric_id = 1;
        ad->last_anomaly.value = fitness;
        ad->last_anomaly.expected_mean = fit_mean;
        ad->last_anomaly.z_score = z_fit;
        ad->last_anomaly.timestamp_ms = GetTimeMs();
        snprintf(ad->last_anomaly.description, sizeof(ad->last_anomaly.description),
            "Fitness anomaly: value=%.4f mean=%.4f z=%.2f", fitness, fit_mean, z_fit);
    }
    if (z_ent >= ANOMALY_Z_SCORE_THRESHOLD) {
        ad->anomaly_detected = true;
        ad->last_anomaly.metric_id = 2;
        ad->last_anomaly.value = entropy;
        ad->last_anomaly.expected_mean = ent_mean;
        ad->last_anomaly.z_score = z_ent;
        ad->last_anomaly.timestamp_ms = GetTimeMs();
        snprintf(ad->last_anomaly.description, sizeof(ad->last_anomaly.description),
            "Entropy anomaly: value=%.4f mean=%.4f z=%.2f", entropy, ent_mean, z_ent);
    }
    if (z_lat >= ANOMALY_Z_SCORE_THRESHOLD) {
        ad->anomaly_detected = true;
        ad->last_anomaly.metric_id = 3;
        ad->last_anomaly.value = latency;
        ad->last_anomaly.expected_mean = lat_mean;
        ad->last_anomaly.z_score = z_lat;
        ad->last_anomaly.timestamp_ms = GetTimeMs();
        snprintf(ad->last_anomaly.description, sizeof(ad->last_anomaly.description),
            "Latency anomaly: value=%.0f mean=%.0f z=%.2f", latency, lat_mean, z_lat);
    }
    if (z_mem >= ANOMALY_Z_SCORE_THRESHOLD) {
        ad->anomaly_detected = true;
        ad->last_anomaly.metric_id = 4;
        ad->last_anomaly.value = memory;
        ad->last_anomaly.expected_mean = mem_mean;
        ad->last_anomaly.z_score = z_mem;
        ad->last_anomaly.timestamp_ms = GetTimeMs();
        snprintf(ad->last_anomaly.description, sizeof(ad->last_anomaly.description),
            "Memory anomaly: value=%.0f mean=%.0f z=%.2f", memory, mem_mean, z_mem);
    }

    return STATUS_SUCCESS;
}

bool AnomalyDetector_IsAnomalyDetected(const AnomalyDetector* ad) {
    return ad && ad->anomaly_detected;
}

void AnomalyDetector_GetLastAnomaly(const AnomalyDetector* ad, AnomalyEvent* out) {
    if (!ad || !out) return;
    *out = ad->last_anomaly;
}
