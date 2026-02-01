#include "../../Include/training_pipeline.h"
#include "../../Include/neural_substrate.h"
#include "../../Include/evolution_engine.h"
#include "../../Include/role_boundary.h"
#include "../../Include/raijin_ntstatus.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static void GeneratePattern(uint8_t* buf, size_t size, uint32_t seed) {
    uint32_t s = seed;
    for (size_t i = 0; i < size; i++) {
        s = s * 1103515245 + 12345;
        buf[i] = (uint8_t)((s >> 16) & 0xFF);
    }
}

static void GenerateCodeLike(uint8_t* buf, size_t size, uint32_t seed) {
    static const uint8_t tokens[] = {
        ' ', '\n', '\t', '(', ')', '{', '}', '[', ']', ';', ',', '.', '=', '+', '-', '*', '/',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        '_', '"', '\'', '<', '>', ':', '!', '&', '|', '%', '#'
    };
    const size_t n = sizeof(tokens);
    uint32_t s = seed;
    for (size_t i = 0; i < size; i++) {
        s = s * 1103515245 + 12345;
        buf[i] = tokens[(s >> 8) % n];
    }
}

static double ComputeLoss(const uint8_t* output, const uint8_t* target, size_t size) {
    double sum = 0.0;
    for (size_t i = 0; i < size; i++) {
        double d = (double)(output[i]) - (double)(target[i]);
        sum += d * d;
    }
    return sum / (size * 65536.0);
}

NTSTATUS TrainingPipeline_Initialize(TrainingPipeline* pipeline,
    NeuralSubstrate* neural,
    EvolutionEngine* evolution) {
    if (!pipeline || !neural) return STATUS_INVALID_PARAMETER;

    memset(pipeline, 0, sizeof(TrainingPipeline));
    pipeline->neural = neural;
    pipeline->evolution = evolution;
    pipeline->evolution_interval = 5;

    pipeline->synthetic_input = (uint8_t*)malloc(TRAINING_INPUT_SIZE);
    pipeline->synthetic_target = (uint8_t*)malloc(TRAINING_TARGET_SIZE);
    pipeline->output_buffer = (uint8_t*)malloc(TRAINING_TARGET_SIZE);

    if (!pipeline->synthetic_input || !pipeline->synthetic_target || !pipeline->output_buffer) {
        if (pipeline->synthetic_input) free(pipeline->synthetic_input);
        if (pipeline->synthetic_target) free(pipeline->synthetic_target);
        if (pipeline->output_buffer) free(pipeline->output_buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

NTSTATUS TrainingPipeline_Shutdown(TrainingPipeline* pipeline) {
    if (!pipeline) return STATUS_INVALID_PARAMETER;

    free(pipeline->synthetic_input);
    pipeline->synthetic_input = NULL;
    free(pipeline->synthetic_target);
    pipeline->synthetic_target = NULL;
    free(pipeline->output_buffer);
    pipeline->output_buffer = NULL;

    return STATUS_SUCCESS;
}

NTSTATUS TrainingPipeline_GenerateSyntheticBatch(TrainingPipeline* pipeline,
    uint32_t seed) {
    if (!pipeline || !pipeline->synthetic_input) return STATUS_INVALID_PARAMETER;

    if (seed % 3 == 0) {
        GenerateCodeLike(pipeline->synthetic_input, TRAINING_INPUT_SIZE, seed);
        GenerateCodeLike(pipeline->synthetic_target, TRAINING_TARGET_SIZE, seed + 1);
    } else {
        GeneratePattern(pipeline->synthetic_input, TRAINING_INPUT_SIZE, seed);
        GeneratePattern(pipeline->synthetic_target, TRAINING_TARGET_SIZE, seed + 1);
    }

    return STATUS_SUCCESS;
}

static uint32_t SeedFromCurriculumTask(const CurriculumTask* t) {
    if (!t) return 0;
    uint32_t h = (uint32_t)t->type * 31u + t->difficulty * 17u;
    for (uint32_t i = 0; i < t->spec_len && i < CURRICULUM_TASK_BUF; i++)
        h = h * 31u + (uint8_t)t->spec[i];
    h += (uint32_t)(t->timestamp_ms & 0xFFFFFFFFu);
    return h ? h : 1u;
}

void TrainingPipeline_SetCurriculumTask(TrainingPipeline* pipeline, const CurriculumTask* task) {
    if (!pipeline || !task) return;
    memcpy(&pipeline->curriculum_task, task, sizeof(CurriculumTask));
    pipeline->curriculum_task_valid = true;
}

void TrainingPipeline_ClearCurriculumTask(TrainingPipeline* pipeline) {
    if (pipeline) pipeline->curriculum_task_valid = false;
}

NTSTATUS TrainingPipeline_TrainStep(TrainingPipeline* pipeline) {
    if (!pipeline || !pipeline->neural) return STATUS_INVALID_PARAMETER;
    {
        RoleBoundaryContext* rbc = RoleBoundary_GetGlobal();
        if (rbc) RoleBoundary_AssertRaijin(rbc);
    }
    uint64_t t0 = GetTickCount64();

    uint32_t seed;
    if (pipeline->curriculum_task_valid) {
        seed = SeedFromCurriculumTask(&pipeline->curriculum_task);
        pipeline->curriculum_task_valid = false;
    } else {
        seed = (uint32_t)(pipeline->total_steps * 31 + (uint64_t)time(NULL));
    }
    NTSTATUS status = TrainingPipeline_GenerateSyntheticBatch(pipeline, seed);
    if (!NT_SUCCESS(status)) return status;

    status = NeuralSubstrate_Process(pipeline->neural,
        pipeline->synthetic_input, TRAINING_INPUT_SIZE,
        pipeline->output_buffer, TRAINING_TARGET_SIZE);
    if (!NT_SUCCESS(status)) return status;

    status = NeuralSubstrate_Learn(pipeline->neural,
        pipeline->synthetic_target, TRAINING_TARGET_SIZE);
    if (!NT_SUCCESS(status)) return status;

    pipeline->last_metrics.loss = ComputeLoss(
        pipeline->output_buffer,
        pipeline->synthetic_target,
        TRAINING_TARGET_SIZE);
    pipeline->last_metrics.entropy = NeuralSubstrate_GetEntropy(pipeline->neural);
    pipeline->last_metrics.step_count = ++pipeline->total_steps;
    pipeline->last_metrics.batch_time_ms = GetTickCount64() - t0;

    if (pipeline->evolution) {
        pipeline->last_metrics.fitness = pipeline->evolution->population.best_fitness;
        pipeline->last_metrics.generation = pipeline->evolution->population.generation;
    }

    return STATUS_SUCCESS;
}

NTSTATUS TrainingPipeline_EvolutionStep(TrainingPipeline* pipeline) {
    if (!pipeline || !pipeline->evolution) return STATUS_INVALID_PARAMETER;
    {
        RoleBoundaryContext* rbc = RoleBoundary_GetGlobal();
        if (rbc) RoleBoundary_AssertRaijin(rbc);
    }
    NTSTATUS status = EvolutionEngine_EvaluatePopulation(pipeline->evolution);
    if (!NT_SUCCESS(status)) return status;

    EvolutionaryIndividual* new_pop = (EvolutionaryIndividual*)malloc(
        pipeline->evolution->population.max_size * sizeof(EvolutionaryIndividual));
    if (!new_pop) return STATUS_INSUFFICIENT_RESOURCES;
    memset(new_pop, 0, pipeline->evolution->population.max_size * sizeof(EvolutionaryIndividual));

    uint32_t elite_count = (uint32_t)(pipeline->evolution->params.elitism_rate *
        pipeline->evolution->population.max_size);
    if (elite_count > pipeline->evolution->population.max_size)
        elite_count = pipeline->evolution->population.max_size;

    for (uint32_t i = 0; i < elite_count; i++) {
        EvolutionaryIndividual* src = &pipeline->evolution->population.individuals[i];
        EvolutionaryIndividual* dst = &new_pop[i];
        dst->id = src->id;
        dst->genome_size = src->genome_size;
        dst->fitness = src->fitness;
        dst->adjusted_fitness = src->adjusted_fitness;
        dst->age = src->age + 1;
        dst->parent1_id = src->parent1_id;
        dst->parent2_id = src->parent2_id;
        dst->evaluated = true;
        dst->genome = malloc(src->genome_size);
        if (dst->genome)
            memcpy(dst->genome, src->genome, src->genome_size);
        dst->phenotype = NULL;
        dst->metadata = NULL;
    }

    EvolutionaryIndividual* parents[2];
    for (uint32_t i = elite_count; i < pipeline->evolution->population.max_size; i++) {
        EvolutionEngine_SelectParents(pipeline->evolution, parents, 2);
        if (parents[0] && parents[1])
            EvolutionEngine_CreateOffspring(pipeline->evolution,
                parents[0], parents[1], &new_pop[i]);
    }

    EvolutionEngine_FreePopulation(pipeline->evolution);
    pipeline->evolution->population.individuals = new_pop;
    pipeline->evolution->population.size = pipeline->evolution->population.max_size;
    pipeline->evolution->population.generation++;

    if (pipeline->neural && pipeline->evolution->population.generation % 50 == 0)
        NeuralSubstrate_Evolve(pipeline->neural);

    EvolutionEngine_EvaluatePopulation(pipeline->evolution);

    pipeline->last_metrics.generation = pipeline->evolution->population.generation;
    pipeline->last_metrics.fitness = pipeline->evolution->population.best_fitness;

    return STATUS_SUCCESS;
}

void TrainingPipeline_GetMetrics(const TrainingPipeline* pipeline,
    TrainingMetrics* out) {
    if (pipeline && out)
        memcpy(out, &pipeline->last_metrics, sizeof(TrainingMetrics));
}
