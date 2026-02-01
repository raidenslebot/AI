#include "../../Include/versioning_rollback.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

NTSTATUS VersioningRollback_Initialize(VersioningRollback* vr,
    LineageTracker* lineage,
    LongTermMemory* ltm,
    NeuralSubstrate* neural,
    EvolutionEngine* evolution,
    const char* base_dir) {
    if (!vr || !lineage || !ltm || !neural) return STATUS_INVALID_PARAMETER;
    memset(vr, 0, sizeof(VersioningRollback));
    vr->lineage = lineage;
    vr->long_term_memory = ltm;
    vr->neural = neural;
    vr->evolution = evolution;
    if (base_dir && base_dir[0])
        snprintf(vr->checkpoint_dir, sizeof(vr->checkpoint_dir), "%s/checkpoints", base_dir);
    else
        snprintf(vr->checkpoint_dir, sizeof(vr->checkpoint_dir), "data/checkpoints");
    CreateDirectoryA("data", NULL);
    CreateDirectoryA(vr->checkpoint_dir, NULL);
    vr->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS VersioningRollback_Shutdown(VersioningRollback* vr) {
    if (!vr) return STATUS_INVALID_PARAMETER;
    vr->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS VersioningRollback_CreateCheckpoint(VersioningRollback* vr,
    uint64_t step_count, uint64_t generation,
    double loss, double fitness, float dominance) {
    if (!vr || !vr->initialized || !vr->lineage || !vr->neural) return STATUS_INVALID_PARAMETER;

    char path[LINEAGE_ENTRY_PATH_MAX];
    snprintf(path, sizeof(path), "%s/checkpoint_v%llu.bin", vr->checkpoint_dir, (unsigned long long)vr->lineage->next_version_id);

    NTSTATUS status = NeuralSubstrate_SaveState(vr->neural, path);
    if (!NT_SUCCESS(status)) return status;

    status = LineageTracker_Record(vr->lineage, step_count, generation, loss, fitness, dominance, path);
    if (!NT_SUCCESS(status)) return status;

    vr->last_checkpoint_version = vr->lineage->next_version_id - 1;
    return STATUS_SUCCESS;
}

NTSTATUS VersioningRollback_RollbackToVersion(VersioningRollback* vr, uint64_t version_id) {
    if (!vr || !vr->initialized || !vr->lineage || !vr->neural) return STATUS_INVALID_PARAMETER;

    LineageEntry entry;
    NTSTATUS status = LineageTracker_GetEntryByVersion(vr->lineage, version_id, &entry);
    if (!NT_SUCCESS(status)) return status;

    status = NeuralSubstrate_LoadState(vr->neural, entry.checkpoint_path);
    if (!NT_SUCCESS(status)) return status;

    vr->rollback_count++;
    return STATUS_SUCCESS;
}

NTSTATUS VersioningRollback_RollbackToBest(VersioningRollback* vr) {
    if (!vr || !vr->initialized || !vr->lineage) return STATUS_INVALID_PARAMETER;

    LineageEntry best;
    NTSTATUS status = LineageTracker_GetBestEntry(vr->lineage, &best);
    if (!NT_SUCCESS(status)) return status;

    return VersioningRollback_RollbackToVersion(vr, best.version_id);
}

uint32_t VersioningRollback_GetRollbackCount(const VersioningRollback* vr) {
    return vr ? vr->rollback_count : 0;
}
