#ifndef RAIJIN_REGRESSION_REPLAY_H
#define RAIJIN_REGRESSION_REPLAY_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define REGRESSION_REPLAY_MAX_TESTS 256
#define REGRESSION_REPLAY_NAME_LEN 128
#define REGRESSION_REPLAY_SIGNATURE_LEN 64
#define REGRESSION_REPLAY_PATH "data/regression_replay.txt"

typedef struct RegressionReplayEntry {
    char test_name[REGRESSION_REPLAY_NAME_LEN];
    char signature[REGRESSION_REPLAY_SIGNATURE_LEN];
    uint64_t first_seen_ms;
    uint32_t replay_count;
} RegressionReplayEntry;

typedef struct RegressionReplay {
    char path[512];
    RegressionReplayEntry entries[REGRESSION_REPLAY_MAX_TESTS];
    uint32_t count;
    bool initialized;
} RegressionReplay;

NTSTATUS RegressionReplay_Initialize(RegressionReplay* rr, const char* base_dir);
NTSTATUS RegressionReplay_Shutdown(RegressionReplay* rr);

NTSTATUS RegressionReplay_Load(RegressionReplay* rr);
NTSTATUS RegressionReplay_Save(RegressionReplay* rr);

NTSTATUS RegressionReplay_AddFailure(RegressionReplay* rr,
    const char* test_name, const char* signature);

uint32_t RegressionReplay_GetCount(const RegressionReplay* rr);
NTSTATUS RegressionReplay_GetEntry(const RegressionReplay* rr, uint32_t index,
    char* test_name, size_t name_size, char* signature, size_t sig_size);

#endif
