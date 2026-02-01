#include "../../Include/world_model.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

#define LATENT_POOL_INIT 1024
#define NODE_CAP_INIT 512
#define TRACE_CAP_INIT 64

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

NTSTATUS WorldModel_Initialize(WorldModel* wm) {
    if (!wm) return STATUS_INVALID_PARAMETER;
    memset(wm, 0, sizeof(WorldModel));
    wm->latent_capacity = LATENT_POOL_INIT;
    wm->latent_pool = (float*)malloc(sizeof(float) * WORLD_LATENT_DIM * wm->latent_capacity);
    if (!wm->latent_pool) return STATUS_INSUFFICIENT_RESOURCES;
    memset(wm->latent_pool, 0, sizeof(float) * WORLD_LATENT_DIM * wm->latent_capacity);
    wm->node_capacity = NODE_CAP_INIT;
    wm->nodes = (WorldGraphNode*)malloc(sizeof(WorldGraphNode) * wm->node_capacity);
    if (!wm->nodes) {
        free(wm->latent_pool);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(wm->nodes, 0, sizeof(WorldGraphNode) * wm->node_capacity);
    wm->trace_capacity = TRACE_CAP_INIT;
    wm->traces = (WorldTemporalTrace*)malloc(sizeof(WorldTemporalTrace) * wm->trace_capacity);
    if (!wm->traces) {
        free(wm->nodes);
        free(wm->latent_pool);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(wm->traces, 0, sizeof(WorldTemporalTrace) * wm->trace_capacity);
    wm->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS WorldModel_Shutdown(WorldModel* wm) {
    if (!wm) return STATUS_INVALID_PARAMETER;
    if (wm->nodes) {
        for (uint32_t i = 0; i < wm->node_count; i++) {
            free(wm->nodes[i].embedding);
            free(wm->nodes[i].edges);
            free(wm->nodes[i].edge_weights);
        }
        free(wm->nodes);
        wm->nodes = NULL;
    }
    if (wm->traces) {
        for (uint32_t i = 0; i < wm->trace_count; i++)
            free(wm->traces[i].latent_offsets);
        free(wm->traces);
        wm->traces = NULL;
    }
    free(wm->latent_pool);
    wm->latent_pool = NULL;
    wm->latent_count = 0;
    wm->node_count = 0;
    wm->trace_count = 0;
    wm->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS WorldModel_InjectExperience(WorldModel* wm, const float* latent, uint32_t dim) {
    if (!wm || !wm->initialized || !latent) return STATUS_INVALID_PARAMETER;
    uint32_t copy_dim = (dim < WORLD_LATENT_DIM) ? dim : WORLD_LATENT_DIM;
    if (wm->latent_count >= wm->latent_capacity) {
        uint32_t new_cap = wm->latent_capacity * 2;
        if (new_cap < 8) new_cap = 8;
        float* new_pool = (float*)realloc(wm->latent_pool, sizeof(float) * WORLD_LATENT_DIM * new_cap);
        if (!new_pool) return STATUS_INSUFFICIENT_RESOURCES;
        wm->latent_pool = new_pool;
        memset(wm->latent_pool + WORLD_LATENT_DIM * wm->latent_capacity, 0,
            sizeof(float) * WORLD_LATENT_DIM * (new_cap - wm->latent_capacity));
        wm->latent_capacity = new_cap;
    }
    float* dst = wm->latent_pool + wm->latent_count * WORLD_LATENT_DIM;
    memcpy(dst, latent, copy_dim * sizeof(float));
    wm->latent_count++;
    return STATUS_SUCCESS;
}

NTSTATUS WorldModel_Compress(WorldModel* wm) {
    if (!wm || !wm->initialized) return STATUS_INVALID_PARAMETER;
    if (wm->latent_count < 2) return STATUS_SUCCESS;
    uint32_t keep = wm->latent_count / 2;
    if (keep == 0) keep = 1;
    memmove(wm->latent_pool, wm->latent_pool + (wm->latent_count - keep) * WORLD_LATENT_DIM,
        keep * WORLD_LATENT_DIM * sizeof(float));
    wm->latent_count = keep;
    wm->compression_cycle++;
    return STATUS_SUCCESS;
}

NTSTATUS WorldModel_Retrieve(WorldModel* wm, const float* query, uint32_t query_dim,
    float* out_latent, uint32_t out_dim, uint32_t* out_count) {
    if (!wm || !wm->initialized || !query || !out_latent || !out_count) return STATUS_INVALID_PARAMETER;
    uint32_t dim = (query_dim < WORLD_LATENT_DIM) ? query_dim : WORLD_LATENT_DIM;
    *out_count = 0;
    if (wm->latent_count == 0) return STATUS_SUCCESS;
    float best_sim = -2.f;
    uint32_t best_idx = 0;
    float qn = Norm(query, dim);
    if (qn < 1e-10f) return STATUS_SUCCESS;
    for (uint32_t i = 0; i < wm->latent_count; i++) {
        float* L = wm->latent_pool + i * WORLD_LATENT_DIM;
        float sim = Dot(query, L, dim) / (qn * Norm(L, dim) + 1e-10f);
        if (sim > best_sim) {
            best_sim = sim;
            best_idx = i;
        }
    }
    uint32_t copy_dim = (out_dim < WORLD_LATENT_DIM) ? out_dim : WORLD_LATENT_DIM;
    memcpy(out_latent, wm->latent_pool + best_idx * WORLD_LATENT_DIM, copy_dim * sizeof(float));
    *out_count = 1;
    return STATUS_SUCCESS;
}

uint32_t WorldModel_GetLatentCount(const WorldModel* wm) {
    return wm ? wm->latent_count : 0;
}

uint32_t WorldModel_GetNodeCount(const WorldModel* wm) {
    return wm ? wm->node_count : 0;
}
