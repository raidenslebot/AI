#ifndef RAIJIN_DOMINANCE_METRICS_H
#define RAIJIN_DOMINANCE_METRICS_H

#include "raijin_ntstatus.h"
#include "neural_substrate.h"
#include "evolution_engine.h"
#include "training_pipeline.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define DOMINANCE_METRICS_HISTORY_LEN 1024
#define DOMINANCE_CRITIQUE_MAX 4096

typedef struct DominanceSnapshot {
    double dominance;       /* Internal dominance score (0-1+) */
    double efficiency;     /* Resource efficiency (0-1+) */
    double coherence;      /* Internal state coherence (0-1) */
    double adaptability;   /* Rate of successful adaptation (0-1+) */
    double fitness;
    double loss;
    float entropy;
    float stress_robustness; /* -1 = not set; 0-1 from adversarial stress tests */
    uint64_t step_count;
    uint64_t generation;
    uint64_t timestamp_ms;
} DominanceSnapshot;

typedef struct DominanceMetrics {
    NeuralSubstrate* neural;
    EvolutionEngine* evolution;
    TrainingPipeline* pipeline;
    DominanceSnapshot* history;
    uint32_t history_count;
    uint32_t history_capacity;
    DominanceSnapshot current;
    double last_stress_robustness; /* -1 = not set; set by RecordStressResult, consumed by next Update */
    double dominance_trend;    /* Slope over recent history */
    double efficiency_trend;
    double coherence_trend;
    double adaptability_trend;
    bool initialized;
} DominanceMetrics;

NTSTATUS DominanceMetrics_Initialize(DominanceMetrics* dm,
    NeuralSubstrate* neural,
    EvolutionEngine* evolution,
    TrainingPipeline* pipeline);
NTSTATUS DominanceMetrics_Shutdown(DominanceMetrics* dm);

NTSTATUS DominanceMetrics_Update(DominanceMetrics* dm,
    double loss, double fitness, float entropy,
    uint64_t step_count, uint64_t generation,
    uint64_t batch_time_ms, uint64_t memory_mb);

void DominanceMetrics_GetCurrent(const DominanceMetrics* dm, DominanceSnapshot* out);
void DominanceMetrics_RecordStressResult(DominanceMetrics* dm, double robustness_score);
void DominanceMetrics_GetTrends(const DominanceMetrics* dm,
    double* dominance_trend, double* efficiency_trend,
    double* coherence_trend, double* adaptability_trend);

NTSTATUS DominanceMetrics_Serialize(const DominanceMetrics* dm, char* buf, size_t buf_size);

#endif
