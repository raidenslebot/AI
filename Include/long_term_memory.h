#ifndef RAIJIN_LONG_TERM_MEMORY_H
#define RAIJIN_LONG_TERM_MEMORY_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LTM_STATE_PATH "data/raijin_state.json"
#define LTM_NEURAL_CHECKPOINT "data/neural_checkpoint.bin"
#define LTM_STATE_MAX 65536

typedef struct LongTermMemory {
    char state_path[512];
    char neural_checkpoint_path[512];
    char state_json[LTM_STATE_MAX];
    uint64_t last_save_step;
    uint64_t last_save_generation;
    double last_loss;
    double last_fitness;
    bool initialized;
} LongTermMemory;

NTSTATUS LongTermMemory_Initialize(LongTermMemory* ltm, const char* base_dir);
NTSTATUS LongTermMemory_Shutdown(LongTermMemory* ltm);

NTSTATUS LongTermMemory_Save(LongTermMemory* ltm,
    void* neural, void* evolution,
    uint64_t step_count, uint64_t generation,
    double loss, double fitness, float entropy);
NTSTATUS LongTermMemory_Load(LongTermMemory* ltm,
    void* neural,
    uint64_t* out_step, uint64_t* out_generation);

NTSTATUS LongTermMemory_SaveNeuralCheckpoint(LongTermMemory* ltm, void* neural);
NTSTATUS LongTermMemory_LoadNeuralCheckpoint(LongTermMemory* ltm, void* neural);

bool LongTermMemory_DetectDegradation(const LongTermMemory* ltm,
    double current_loss, double current_fitness);

#endif
