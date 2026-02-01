#include "../../Include/long_term_memory.h"
#include "../../Include/neural_substrate.h"
#include "../../Include/evolution_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

NTSTATUS LongTermMemory_Initialize(LongTermMemory* ltm, const char* base_dir) {
    if (!ltm) return STATUS_INVALID_PARAMETER;
    memset(ltm, 0, sizeof(LongTermMemory));
    if (base_dir && base_dir[0]) {
        snprintf(ltm->state_path, sizeof(ltm->state_path), "%s/raijin_state.json", base_dir);
        snprintf(ltm->neural_checkpoint_path, sizeof(ltm->neural_checkpoint_path),
            "%s/neural_checkpoint.bin", base_dir);
    } else {
        snprintf(ltm->state_path, sizeof(ltm->state_path), LTM_STATE_PATH);
        snprintf(ltm->neural_checkpoint_path, sizeof(ltm->neural_checkpoint_path),
            LTM_NEURAL_CHECKPOINT);
    }
    CreateDirectoryA("data", NULL);
    ltm->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS LongTermMemory_Shutdown(LongTermMemory* ltm) {
    if (!ltm) return STATUS_INVALID_PARAMETER;
    ltm->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS LongTermMemory_SaveNeuralCheckpoint(LongTermMemory* ltm, void* neural) {
    if (!ltm || !ltm->initialized || !neural) return STATUS_INVALID_PARAMETER;
    return NeuralSubstrate_SaveState((NeuralSubstrate*)neural, ltm->neural_checkpoint_path);
}

NTSTATUS LongTermMemory_LoadNeuralCheckpoint(LongTermMemory* ltm, void* neural) {
    if (!ltm || !ltm->initialized || !neural) return STATUS_INVALID_PARAMETER;
    return NeuralSubstrate_LoadState((NeuralSubstrate*)neural, ltm->neural_checkpoint_path);
}

NTSTATUS LongTermMemory_Save(LongTermMemory* ltm,
    void* neural,
    void* evolution,
    uint64_t step_count, uint64_t generation,
    double loss, double fitness, float entropy) {
    if (!ltm || !ltm->initialized) return STATUS_INVALID_PARAMETER;

    if (neural) {
        NTSTATUS status = LongTermMemory_SaveNeuralCheckpoint(ltm, neural);
        if (!NT_SUCCESS(status)) return status;
    }

    int n = snprintf(ltm->state_json, LTM_STATE_MAX,
        "{\n"
        "  \"consciousness_level\": %.6f,\n"
        "  \"evolution_generation\": %llu,\n"
        "  \"training_step_count\": %llu,\n"
        "  \"loss\": %.6f,\n"
        "  \"fitness\": %.6f,\n"
        "  \"entropy\": %.6f,\n"
        "  \"awareness_metrics\": {\n"
        "    \"self_reflection\": %.4f,\n"
        "    \"environmental_awareness\": %.4f,\n"
        "    \"predictive_capability\": %.4f,\n"
        "    \"ethical_reasoning\": %.4f,\n"
        "    \"creative_potential\": %.4f\n"
        "  },\n"
        "  \"knowledge_base\": {},\n"
        "  \"ethics_state\": {\n"
        "    \"decision_count\": %u,\n"
        "    \"maturity_level\": %.4f,\n"
        "    \"framework_size\": %u\n"
        "  },\n"
        "  \"programming_state\": {\n"
        "    \"known_languages\": 5,\n"
        "    \"generated_code_count\": 0,\n"
        "    \"analyzed_code_count\": 0\n"
        "  }\n"
        "}\n",
        (double)(entropy * 0.5 + (1.0 - loss) * 0.5),
        (unsigned long long)generation,
        (unsigned long long)step_count,
        loss, fitness, (double)entropy,
        (float)(step_count % 1000) / 1000.0f,
        (float)(generation % 1000) / 1000.0f,
        (float)(1.0 - loss),
        (float)(fitness),
        (float)(entropy),
        (unsigned)(generation % 10000),
        (float)(fitness),
        (unsigned)(generation % 1000));

    if (n <= 0 || n >= (int)LTM_STATE_MAX) return STATUS_UNSUCCESSFUL;

    FILE* f = fopen(ltm->state_path, "w");
    if (!f) return STATUS_UNSUCCESSFUL;
    fputs(ltm->state_json, f);
    fclose(f);

    ltm->last_save_step = step_count;
    ltm->last_save_generation = generation;
    ltm->last_loss = loss;
    ltm->last_fitness = fitness;
    return STATUS_SUCCESS;
}

NTSTATUS LongTermMemory_Load(LongTermMemory* ltm,
    void* neural,
    uint64_t* out_step, uint64_t* out_generation) {
    if (!ltm || !ltm->initialized) return STATUS_INVALID_PARAMETER;

    if (neural) {
        NTSTATUS status = LongTermMemory_LoadNeuralCheckpoint(ltm, neural);
        if (!NT_SUCCESS(status)) return status;
    }

    if (out_step) *out_step = ltm->last_save_step;
    if (out_generation) *out_generation = ltm->last_save_generation;
    return STATUS_SUCCESS;
}

bool LongTermMemory_DetectDegradation(const LongTermMemory* ltm,
    double current_loss, double current_fitness) {
    if (!ltm || !ltm->initialized) return false;
    if (ltm->last_save_step == 0) return false;
    const double loss_threshold = 1.05;
    const double fitness_threshold = 0.95;
    if (ltm->last_loss <= 0.0) return (current_fitness < ltm->last_fitness * fitness_threshold);
    if (ltm->last_fitness <= 0.0) return (current_loss > ltm->last_loss * loss_threshold);
    return (current_loss > ltm->last_loss * loss_threshold &&
            current_fitness < ltm->last_fitness * fitness_threshold);
}
