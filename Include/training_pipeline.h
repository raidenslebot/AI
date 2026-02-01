#ifndef RAIJIN_TRAINING_PIPELINE_H
#define RAIJIN_TRAINING_PIPELINE_H

#include "raijin_ntstatus.h"
#include "neural_substrate.h"
#include "evolution_engine.h"
#include "curriculum.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define TRAINING_BATCH_SIZE 256
#define TRAINING_INPUT_SIZE 1000
#define TRAINING_TARGET_SIZE 1000

typedef struct TrainingMetrics {
    double loss;
    double fitness;
    float entropy;
    uint64_t step_count;
    uint64_t generation;
    uint64_t batch_time_ms;
} TrainingMetrics;

typedef struct TrainingPipeline {
    NeuralSubstrate* neural;
    EvolutionEngine* evolution;
    uint8_t* synthetic_input;
    uint8_t* synthetic_target;
    uint8_t* output_buffer;
    TrainingMetrics last_metrics;
    uint64_t total_steps;
    bool run_evolution_this_step;
    uint32_t evolution_interval;
    CurriculumTask curriculum_task;
    bool curriculum_task_valid;
} TrainingPipeline;

NTSTATUS TrainingPipeline_Initialize(TrainingPipeline* pipeline,
    NeuralSubstrate* neural,
    EvolutionEngine* evolution);
NTSTATUS TrainingPipeline_Shutdown(TrainingPipeline* pipeline);

NTSTATUS TrainingPipeline_GenerateSyntheticBatch(TrainingPipeline* pipeline,
    uint32_t seed);
void TrainingPipeline_SetCurriculumTask(TrainingPipeline* pipeline, const CurriculumTask* task);
void TrainingPipeline_ClearCurriculumTask(TrainingPipeline* pipeline);
NTSTATUS TrainingPipeline_TrainStep(TrainingPipeline* pipeline);
NTSTATUS TrainingPipeline_EvolutionStep(TrainingPipeline* pipeline);

void TrainingPipeline_GetMetrics(const TrainingPipeline* pipeline,
    TrainingMetrics* out);

#endif
