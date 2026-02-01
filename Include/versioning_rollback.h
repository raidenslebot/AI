#ifndef RAIJIN_VERSIONING_ROLLBACK_H
#define RAIJIN_VERSIONING_ROLLBACK_H

#include "raijin_ntstatus.h"
#include "lineage_tracker.h"
#include "long_term_memory.h"
#include "neural_substrate.h"
#include "evolution_engine.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct VersioningRollback {
    LineageTracker* lineage;
    LongTermMemory* long_term_memory;
    NeuralSubstrate* neural;
    EvolutionEngine* evolution;
    char checkpoint_dir[512];
    uint64_t last_checkpoint_version;
    uint32_t rollback_count;
    bool initialized;
} VersioningRollback;

NTSTATUS VersioningRollback_Initialize(VersioningRollback* vr,
    LineageTracker* lineage,
    LongTermMemory* ltm,
    NeuralSubstrate* neural,
    EvolutionEngine* evolution,
    const char* base_dir);
NTSTATUS VersioningRollback_Shutdown(VersioningRollback* vr);

NTSTATUS VersioningRollback_CreateCheckpoint(VersioningRollback* vr,
    uint64_t step_count, uint64_t generation,
    double loss, double fitness, float dominance);

NTSTATUS VersioningRollback_RollbackToVersion(VersioningRollback* vr, uint64_t version_id);
NTSTATUS VersioningRollback_RollbackToBest(VersioningRollback* vr);

uint32_t VersioningRollback_GetRollbackCount(const VersioningRollback* vr);

#endif
