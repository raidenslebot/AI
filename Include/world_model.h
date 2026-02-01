#ifndef RAIJIN_WORLD_MODEL_H
#define RAIJIN_WORLD_MODEL_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define WORLD_LATENT_DIM 256
#define WORLD_GRAPH_NODES_MAX 4096
#define WORLD_TRACE_LEN 64
#define WORLD_COMPRESSION_BATCH 32

typedef struct WorldLatent {
    float* values;
    uint32_t dim;
    uint64_t timestamp_ms;
} WorldLatent;

typedef struct WorldGraphNode {
    uint32_t id;
    uint32_t latent_offset;
    float* embedding;
    uint32_t edge_count;
    uint32_t* edges;
    float* edge_weights;
} WorldGraphNode;

typedef struct WorldTemporalTrace {
    uint32_t* latent_offsets;
    uint32_t length;
    uint64_t start_ms;
    uint64_t end_ms;
} WorldTemporalTrace;

typedef struct WorldModel {
    float* latent_pool;
    uint32_t latent_count;
    uint32_t latent_capacity;
    WorldGraphNode* nodes;
    uint32_t node_count;
    uint32_t node_capacity;
    WorldTemporalTrace* traces;
    uint32_t trace_count;
    uint32_t trace_capacity;
    uint64_t compression_cycle;
    bool initialized;
} WorldModel;

NTSTATUS WorldModel_Initialize(WorldModel* wm);
NTSTATUS WorldModel_Shutdown(WorldModel* wm);

NTSTATUS WorldModel_InjectExperience(WorldModel* wm, const float* latent, uint32_t dim);

NTSTATUS WorldModel_Compress(WorldModel* wm);

NTSTATUS WorldModel_Retrieve(WorldModel* wm, const float* query, uint32_t query_dim,
    float* out_latent, uint32_t out_dim, uint32_t* out_count);

uint32_t WorldModel_GetLatentCount(const WorldModel* wm);
uint32_t WorldModel_GetNodeCount(const WorldModel* wm);

#endif
