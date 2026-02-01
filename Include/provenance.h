#ifndef RAIJIN_PROVENANCE_H
#define RAIJIN_PROVENANCE_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PROVENANCE_HASH_LEN 64
#define PROVENANCE_LOG_PATH_MAX 512
#define PROVENANCE_CONFIG_MAX 4096

typedef struct ProvenanceRecord {
    char build_hash[PROVENANCE_HASH_LEN];
    char config_hash[PROVENANCE_HASH_LEN];
    uint64_t timestamp_ms;
    uint32_t seed;
    char config_snapshot[PROVENANCE_CONFIG_MAX];
    uint32_t config_len;
} ProvenanceRecord;

typedef struct Provenance {
    char base_dir[512];
    char log_path[PROVENANCE_LOG_PATH_MAX];
    char pinned_deps_path[512];
    ProvenanceRecord last_record;
    uint32_t record_count;
    bool deterministic_mode;
    bool initialized;
} Provenance;

NTSTATUS Provenance_Initialize(Provenance* pv, const char* base_dir);
NTSTATUS Provenance_Shutdown(Provenance* pv);

NTSTATUS Provenance_LogBuild(Provenance* pv, const char* build_artifact_path,
    const char* config_json, uint32_t config_len, uint32_t seed);

NTSTATUS Provenance_ComputeFileHash(const char* path, char* hash_out, size_t hash_size);

NTSTATUS Provenance_SetPinnedDeps(Provenance* pv, const char* deps_json);

void Provenance_GetLastRecord(const Provenance* pv, ProvenanceRecord* out);
bool Provenance_IsDeterministic(const Provenance* pv);

#endif
