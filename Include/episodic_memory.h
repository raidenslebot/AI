#ifndef RAIJIN_EPISODIC_MEMORY_H
#define RAIJIN_EPISODIC_MEMORY_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define EPISODIC_ENTRY_DIM 128
#define EPISODIC_MAX_ENTRIES 2048
#define EPISODIC_CONSOLIDATION_THRESHOLD 64
#define EPISODIC_RETRIEVAL_K 8

typedef struct EpisodicEntry {
    float* embedding;
    uint32_t dim;
    uint64_t timestamp_ms;
    uint64_t step_id;
    float utility;   /* retention earned by utility under tests */
    bool consolidated;
} EpisodicEntry;

typedef struct EpisodicMemory {
    EpisodicEntry* entries;
    uint32_t entry_count;
    uint32_t entry_capacity;
    uint64_t consolidation_cycle;
    uint32_t semantic_count;
    uint32_t procedural_count;
    bool initialized;
} EpisodicMemory;

NTSTATUS EpisodicMemory_Initialize(EpisodicMemory* em);
NTSTATUS EpisodicMemory_Shutdown(EpisodicMemory* em);

NTSTATUS EpisodicMemory_Store(EpisodicMemory* em, const float* embedding, uint32_t dim,
    uint64_t step_id);

NTSTATUS EpisodicMemory_Consolidate(EpisodicMemory* em);

NTSTATUS EpisodicMemory_Retrieve(EpisodicMemory* em, const float* query, uint32_t query_dim,
    float* out_embeddings, uint32_t* out_indices, uint32_t k);

NTSTATUS EpisodicMemory_UpdateUtility(EpisodicMemory* em, uint32_t index, float utility);

NTSTATUS EpisodicMemory_ForgetLowUtility(EpisodicMemory* em, float threshold);

uint32_t EpisodicMemory_GetEntryCount(const EpisodicMemory* em);

#endif
