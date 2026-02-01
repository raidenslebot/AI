#include "../../Include/provenance.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

static void SimpleHash(const void* data, size_t len, char* out, size_t out_size) {
    const uint8_t* p = (const uint8_t*)data;
    uint64_t h = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= 0x100000001b3ULL;
    }
    snprintf(out, out_size, "%016llx", (unsigned long long)h);
}

NTSTATUS Provenance_Initialize(Provenance* pv, const char* base_dir) {
    if (!pv) return STATUS_INVALID_PARAMETER;
    memset(pv, 0, sizeof(Provenance));
    if (base_dir && base_dir[0])
        snprintf(pv->base_dir, sizeof(pv->base_dir), "%s", base_dir);
    else
        snprintf(pv->base_dir, sizeof(pv->base_dir), "data");
    snprintf(pv->log_path, sizeof(pv->log_path), "%s/provenance.log", pv->base_dir);
    snprintf(pv->pinned_deps_path, sizeof(pv->pinned_deps_path), "%s/pinned_deps.json", pv->base_dir);
    CreateDirectoryA(pv->base_dir, NULL);
    pv->deterministic_mode = true;
    pv->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS Provenance_Shutdown(Provenance* pv) {
    if (!pv) return STATUS_INVALID_PARAMETER;
    pv->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS Provenance_ComputeFileHash(const char* path, char* hash_out, size_t hash_size) {
    if (!path || !hash_out || hash_size < 32) return STATUS_INVALID_PARAMETER;
    FILE* f = fopen(path, "rb");
    if (!f) return STATUS_NOT_FOUND;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0 || sz > 1024 * 1024 * 64) {
        fclose(f);
        return STATUS_INVALID_PARAMETER;
    }
    char* buf = (char*)malloc((size_t)sz);
    if (!buf) { fclose(f); return STATUS_INSUFFICIENT_RESOURCES; }
    size_t nr = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (nr != (size_t)sz) { free(buf); return STATUS_UNSUCCESSFUL; }
    SimpleHash(buf, nr, hash_out, hash_size);
    free(buf);
    return STATUS_SUCCESS;
}

NTSTATUS Provenance_LogBuild(Provenance* pv, const char* build_artifact_path,
    const char* config_json, uint32_t config_len, uint32_t seed) {
    if (!pv || !pv->initialized) return STATUS_INVALID_PARAMETER;
    ProvenanceRecord* r = &pv->last_record;
    memset(r, 0, sizeof(ProvenanceRecord));
    r->timestamp_ms = GetTimeMs();
    r->seed = seed;
    if (build_artifact_path && build_artifact_path[0]) {
        Provenance_ComputeFileHash(build_artifact_path, r->build_hash, sizeof(r->build_hash));
    } else {
        snprintf(r->build_hash, sizeof(r->build_hash), "none");
    }
    if (config_json && config_len > 0 && config_len < PROVENANCE_CONFIG_MAX) {
        SimpleHash(config_json, config_len, r->config_hash, sizeof(r->config_hash));
        memcpy(r->config_snapshot, config_json, config_len);
        r->config_len = config_len;
    }
    pv->record_count++;
    FILE* f = fopen(pv->log_path, "a");
    if (f) {
        fprintf(f, "%llu %u %s %s\n", (unsigned long long)r->timestamp_ms, r->seed, r->build_hash, r->config_hash);
        fclose(f);
    }
    return STATUS_SUCCESS;
}

NTSTATUS Provenance_SetPinnedDeps(Provenance* pv, const char* deps_json) {
    if (!pv || !pv->initialized || !deps_json) return STATUS_INVALID_PARAMETER;
    FILE* f = fopen(pv->pinned_deps_path, "w");
    if (!f) return STATUS_UNSUCCESSFUL;
    fputs(deps_json, f);
    fclose(f);
    return STATUS_SUCCESS;
}

void Provenance_GetLastRecord(const Provenance* pv, ProvenanceRecord* out) {
    if (!pv || !out) return;
    *out = pv->last_record;
}

bool Provenance_IsDeterministic(const Provenance* pv) {
    return pv && pv->deterministic_mode;
}
