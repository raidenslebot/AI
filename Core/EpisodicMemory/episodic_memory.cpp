#include "../../Include/episodic_memory.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

#define EPISODIC_CAP 2048

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

static float Dot(const float* a, const float* b, uint32_t n) {
    float sum = 0.f;
    for (uint32_t i = 0; i < n; i++) sum += a[i] * b[i];
    return sum;
}

static float Norm(const float* a, uint32_t n) {
    float sum = 0.f;
    for (uint32_t i = 0; i < n; i++) sum += a[i] * a[i];
    return sqrtf(sum);
}

NTSTATUS EpisodicMemory_Initialize(EpisodicMemory* em) {
    if (!em) return STATUS_INVALID_PARAMETER;
    memset(em, 0, sizeof(EpisodicMemory));
    em->entry_capacity = EPISODIC_CAP;
    em->entries = (EpisodicEntry*)malloc(sizeof(EpisodicEntry) * em->entry_capacity);
    if (!em->entries) return STATUS_INSUFFICIENT_RESOURCES;
    memset(em->entries, 0, sizeof(EpisodicEntry) * em->entry_capacity);
    em->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS EpisodicMemory_Shutdown(EpisodicMemory* em) {
    if (!em) return STATUS_INVALID_PARAMETER;
    if (em->entries) {
        for (uint32_t i = 0; i < em->entry_count; i++) {
            free(em->entries[i].embedding);
            em->entries[i].embedding = NULL;
        }
        free(em->entries);
        em->entries = NULL;
    }
    em->entry_count = 0;
    em->entry_capacity = 0;
    em->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS EpisodicMemory_Store(EpisodicMemory* em, const float* embedding, uint32_t dim,
    uint64_t step_id) {
    if (!em || !em->initialized || !embedding) return STATUS_INVALID_PARAMETER;
    uint32_t copy_dim = (dim < EPISODIC_ENTRY_DIM) ? dim : EPISODIC_ENTRY_DIM;
    if (em->entry_count >= em->entry_capacity) {
        uint32_t drop = em->entry_capacity / 4;
        if (drop == 0) drop = 1;
        for (uint32_t i = 0; i < drop; i++) {
            free(em->entries[i].embedding);
            em->entries[i].embedding = NULL;
        }
        memmove(em->entries, em->entries + drop, (em->entry_count - drop) * sizeof(EpisodicEntry));
        em->entry_count -= drop;
    }
    EpisodicEntry* e = &em->entries[em->entry_count];
    e->embedding = (float*)malloc(sizeof(float) * EPISODIC_ENTRY_DIM);
    if (!e->embedding) return STATUS_INSUFFICIENT_RESOURCES;
    memset(e->embedding, 0, sizeof(float) * EPISODIC_ENTRY_DIM);
    memcpy(e->embedding, embedding, copy_dim * sizeof(float));
    e->dim = copy_dim;
    e->timestamp_ms = GetTimeMs();
    e->step_id = step_id;
    e->utility = 0.5f;
    e->consolidated = false;
    em->entry_count++;
    return STATUS_SUCCESS;
}

NTSTATUS EpisodicMemory_Consolidate(EpisodicMemory* em) {
    if (!em || !em->initialized) return STATUS_INVALID_PARAMETER;
    for (uint32_t i = 0; i < em->entry_count; i++) {
        if (!em->entries[i].consolidated && em->entries[i].utility > 0.3f)
            em->entries[i].consolidated = true;
    }
    em->consolidation_cycle++;
    em->semantic_count = em->entry_count / 2;
    em->procedural_count = em->entry_count - em->semantic_count;
    return STATUS_SUCCESS;
}

NTSTATUS EpisodicMemory_Retrieve(EpisodicMemory* em, const float* query, uint32_t query_dim,
    float* out_embeddings, uint32_t* out_indices, uint32_t k) {
    if (!em || !em->initialized || !query || !out_embeddings || !out_indices) return STATUS_INVALID_PARAMETER;
    uint32_t dim = (query_dim < EPISODIC_ENTRY_DIM) ? query_dim : EPISODIC_ENTRY_DIM;
    if (em->entry_count == 0 || k == 0) return STATUS_SUCCESS;
    if (k > em->entry_count) k = em->entry_count;
    if (k > EPISODIC_RETRIEVAL_K) k = EPISODIC_RETRIEVAL_K;
    float qn = Norm(query, dim);
    if (qn < 1e-10f) return STATUS_SUCCESS;
    float best_sims[EPISODIC_RETRIEVAL_K];
    uint32_t best_idx[EPISODIC_RETRIEVAL_K];
    for (uint32_t i = 0; i < k; i++) { best_sims[i] = -2.f; best_idx[i] = 0; }
    for (uint32_t i = 0; i < em->entry_count; i++) {
        float* L = em->entries[i].embedding;
        float sim = Dot(query, L, dim) / (qn * Norm(L, em->entries[i].dim) + 1e-10f);
        sim *= (0.5f + 0.5f * em->entries[i].utility);
        uint32_t j = 0;
        while (j < k && sim <= best_sims[j]) j++;
        if (j < k) {
            for (uint32_t t = k - 1; t > j; t--) {
                best_sims[t] = best_sims[t - 1];
                best_idx[t] = best_idx[t - 1];
            }
            best_sims[j] = sim;
            best_idx[j] = i;
        }
    }
    for (uint32_t j = 0; j < k; j++) {
        out_indices[j] = best_idx[j];
        memcpy(out_embeddings + j * EPISODIC_ENTRY_DIM, em->entries[best_idx[j]].embedding,
            EPISODIC_ENTRY_DIM * sizeof(float));
    }
    return STATUS_SUCCESS;
}

NTSTATUS EpisodicMemory_UpdateUtility(EpisodicMemory* em, uint32_t index, float utility) {
    if (!em || !em->initialized || index >= em->entry_count) return STATUS_INVALID_PARAMETER;
    em->entries[index].utility = (utility < 0.f) ? 0.f : ((utility > 1.f) ? 1.f : utility);
    return STATUS_SUCCESS;
}

NTSTATUS EpisodicMemory_ForgetLowUtility(EpisodicMemory* em, float threshold) {
    if (!em || !em->initialized) return STATUS_INVALID_PARAMETER;
    uint32_t w = 0;
    for (uint32_t i = 0; i < em->entry_count; i++) {
        if (em->entries[i].utility >= threshold) {
            if (w != i) em->entries[w] = em->entries[i];
            w++;
        } else {
            free(em->entries[i].embedding);
            em->entries[i].embedding = NULL;
        }
    }
    em->entry_count = w;
    return STATUS_SUCCESS;
}

uint32_t EpisodicMemory_GetEntryCount(const EpisodicMemory* em) {
    return em ? em->entry_count : 0;
}
