#include "../../Include/lineage_tracker.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

#define LINEAGE_FILE "data/lineage.json"
#define LINEAGE_MAGIC "RAIJIN_LINEAGE_V1"


NTSTATUS LineageTracker_Initialize(LineageTracker* lt, const char* base_dir) {
    if (!lt) return STATUS_INVALID_PARAMETER;
    memset(lt, 0, sizeof(LineageTracker));
    if (base_dir && base_dir[0]) {
        snprintf(lt->base_dir, sizeof(lt->base_dir), "%s", base_dir);
        snprintf(lt->lineage_path, sizeof(lt->lineage_path), "%s/lineage.json", base_dir);
    } else {
        snprintf(lt->base_dir, sizeof(lt->base_dir), "data");
        snprintf(lt->lineage_path, sizeof(lt->lineage_path), LINEAGE_FILE);
    }
    CreateDirectoryA(lt->base_dir, NULL);
    lt->entry_capacity = LINEAGE_MAX_ENTRIES;
    lt->entries = (LineageEntry*)malloc(sizeof(LineageEntry) * lt->entry_capacity);
    if (!lt->entries) return STATUS_INSUFFICIENT_RESOURCES;
    memset(lt->entries, 0, sizeof(LineageEntry) * lt->entry_capacity);
    lt->next_version_id = 1;
    lt->initialized = true;
    LineageTracker_Load(lt);
    return STATUS_SUCCESS;
}

NTSTATUS LineageTracker_Shutdown(LineageTracker* lt) {
    if (!lt) return STATUS_INVALID_PARAMETER;
    if (lt->entries) {
        LineageTracker_Save(lt);
        free(lt->entries);
        lt->entries = NULL;
    }
    lt->entry_count = 0;
    lt->entry_capacity = 0;
    lt->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS LineageTracker_Record(LineageTracker* lt,
    uint64_t step_count, uint64_t generation,
    double loss, double fitness, float dominance,
    const char* checkpoint_path) {
    if (!lt || !lt->initialized) return STATUS_INVALID_PARAMETER;

    if (lt->entry_count >= lt->entry_capacity) {
        memmove(lt->entries, lt->entries + 1, (lt->entry_capacity - 1) * sizeof(LineageEntry));
        lt->entry_count--;
    }

    LineageEntry* e = &lt->entries[lt->entry_count];
    e->version_id = lt->next_version_id++;
    e->step_count = step_count;
    e->generation = generation;
    e->loss = loss;
    e->fitness = fitness;
    e->dominance = dominance;
    e->timestamp_ms = GetTickCount64();
    if (checkpoint_path) {
        size_t len = strlen(checkpoint_path);
        if (len >= sizeof(e->checkpoint_path)) len = sizeof(e->checkpoint_path) - 1;
        memcpy(e->checkpoint_path, checkpoint_path, len);
        e->checkpoint_path[len] = '\0';
    } else
        snprintf(e->checkpoint_path, sizeof(e->checkpoint_path), "%s/checkpoint_v%llu.bin", lt->base_dir, (unsigned long long)e->version_id);
    lt->entry_count++;
    return LineageTracker_Save(lt);
}

NTSTATUS LineageTracker_Load(LineageTracker* lt) {
    if (!lt || !lt->initialized) return STATUS_INVALID_PARAMETER;
    lt->entry_count = 0;
    FILE* f = fopen(lt->lineage_path, "rb");
    if (!f) return STATUS_SUCCESS;
    char magic[32];
    if (fread(magic, 1, strlen(LINEAGE_MAGIC), f) != strlen(LINEAGE_MAGIC) ||
        memcmp(magic, LINEAGE_MAGIC, strlen(LINEAGE_MAGIC)) != 0) {
        fclose(f);
        return STATUS_SUCCESS;
    }
    uint32_t n = 0;
    if (fread(&n, sizeof(uint32_t), 1, f) != 1 || n > lt->entry_capacity) {
        fclose(f);
        return STATUS_SUCCESS;
    }
    if (fread(&lt->next_version_id, sizeof(uint64_t), 1, f) != 1) {
        fclose(f);
        return STATUS_SUCCESS;
    }
    for (uint32_t i = 0; i < n; i++) {
        if (fread(&lt->entries[i], sizeof(LineageEntry), 1, f) != 1) break;
        lt->entry_count = i + 1;
    }
    fclose(f);
    return STATUS_SUCCESS;
}

NTSTATUS LineageTracker_Save(LineageTracker* lt) {
    if (!lt || !lt->initialized) return STATUS_INVALID_PARAMETER;
    FILE* f = fopen(lt->lineage_path, "wb");
    if (!f) return STATUS_UNSUCCESSFUL;
    fwrite(LINEAGE_MAGIC, 1, strlen(LINEAGE_MAGIC), f);
    fwrite(&lt->entry_count, sizeof(uint32_t), 1, f);
    fwrite(&lt->next_version_id, sizeof(uint64_t), 1, f);
    for (uint32_t i = 0; i < lt->entry_count; i++)
        fwrite(&lt->entries[i], sizeof(LineageEntry), 1, f);
    fclose(f);
    return STATUS_SUCCESS;
}

uint32_t LineageTracker_GetEntryCount(const LineageTracker* lt) {
    return lt ? lt->entry_count : 0;
}

NTSTATUS LineageTracker_GetEntry(const LineageTracker* lt, uint32_t index, LineageEntry* out) {
    if (!lt || !out || index >= lt->entry_count) return STATUS_INVALID_PARAMETER;
    *out = lt->entries[index];
    return STATUS_SUCCESS;
}

NTSTATUS LineageTracker_GetBestEntry(const LineageTracker* lt, LineageEntry* out) {
    if (!lt || !out || lt->entry_count == 0) return STATUS_INVALID_PARAMETER;
    LineageEntry* best = &lt->entries[0];
    for (uint32_t i = 1; i < lt->entry_count; i++) {
        if (lt->entries[i].fitness > best->fitness)
            best = &lt->entries[i];
    }
    *out = *best;
    return STATUS_SUCCESS;
}

NTSTATUS LineageTracker_GetEntryByVersion(const LineageTracker* lt, uint64_t version_id, LineageEntry* out) {
    if (!lt || !out) return STATUS_INVALID_PARAMETER;
    for (uint32_t i = 0; i < lt->entry_count; i++) {
        if (lt->entries[i].version_id == version_id) {
            *out = lt->entries[i];
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}
