#ifndef RAIJIN_LINEAGE_TRACKER_H
#define RAIJIN_LINEAGE_TRACKER_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define LINEAGE_VERSION_ID_MAX 64
#define LINEAGE_ENTRY_PATH_MAX 512
#define LINEAGE_MAX_ENTRIES 1024

typedef struct LineageEntry {
    uint64_t version_id;
    uint64_t step_count;
    uint64_t generation;
    double loss;
    double fitness;
    float dominance;
    uint64_t timestamp_ms;
    char checkpoint_path[LINEAGE_ENTRY_PATH_MAX];
} LineageEntry;

typedef struct LineageTracker {
    char base_dir[512];
    char lineage_path[512];
    LineageEntry* entries;
    uint32_t entry_count;
    uint32_t entry_capacity;
    uint64_t next_version_id;
    bool initialized;
} LineageTracker;

NTSTATUS LineageTracker_Initialize(LineageTracker* lt, const char* base_dir);
NTSTATUS LineageTracker_Shutdown(LineageTracker* lt);

NTSTATUS LineageTracker_Record(LineageTracker* lt,
    uint64_t step_count, uint64_t generation,
    double loss, double fitness, float dominance,
    const char* checkpoint_path);

NTSTATUS LineageTracker_Load(LineageTracker* lt);
NTSTATUS LineageTracker_Save(LineageTracker* lt);

uint32_t LineageTracker_GetEntryCount(const LineageTracker* lt);
NTSTATUS LineageTracker_GetEntry(const LineageTracker* lt, uint32_t index, LineageEntry* out);
NTSTATUS LineageTracker_GetBestEntry(const LineageTracker* lt, LineageEntry* out);
NTSTATUS LineageTracker_GetEntryByVersion(const LineageTracker* lt, uint64_t version_id, LineageEntry* out);

#endif
