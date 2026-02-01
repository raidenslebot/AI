#include "../../Include/regression_replay.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

NTSTATUS RegressionReplay_Initialize(RegressionReplay* rr, const char* base_dir) {
    if (!rr) return STATUS_INVALID_PARAMETER;
    memset(rr, 0, sizeof(RegressionReplay));
    if (base_dir && base_dir[0]) {
        snprintf(rr->path, sizeof(rr->path), "%s/regression_replay.txt", base_dir);
    } else {
        snprintf(rr->path, sizeof(rr->path), REGRESSION_REPLAY_PATH);
    }
    rr->initialized = true;
    RegressionReplay_Load(rr);
    return STATUS_SUCCESS;
}

NTSTATUS RegressionReplay_Shutdown(RegressionReplay* rr) {
    if (!rr) return STATUS_INVALID_PARAMETER;
    rr->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS RegressionReplay_Load(RegressionReplay* rr) {
    if (!rr || !rr->initialized) return STATUS_INVALID_PARAMETER;
    rr->count = 0;
    FILE* f = fopen(rr->path, "r");
    if (!f) return STATUS_SUCCESS;
    char line[512];
    while (rr->count < REGRESSION_REPLAY_MAX_TESTS && fgets(line, sizeof(line), f)) {
        char* tab = strchr(line, '\t');
        if (tab) {
            *tab = '\0';
            size_t name_len = (size_t)(tab - line);
            if (name_len >= REGRESSION_REPLAY_NAME_LEN) name_len = REGRESSION_REPLAY_NAME_LEN - 1;
            strncpy(rr->entries[rr->count].test_name, line, name_len);
            rr->entries[rr->count].test_name[name_len] = '\0';
            tab++;
            size_t sig_len = strlen(tab);
            while (sig_len > 0 && (tab[sig_len - 1] == '\n' || tab[sig_len - 1] == '\r')) sig_len--;
            if (sig_len >= REGRESSION_REPLAY_SIGNATURE_LEN) sig_len = REGRESSION_REPLAY_SIGNATURE_LEN - 1;
            strncpy(rr->entries[rr->count].signature, tab, sig_len);
            rr->entries[rr->count].signature[sig_len] = '\0';
            rr->entries[rr->count].replay_count = 0;
            rr->entries[rr->count].first_seen_ms = 0;
            rr->count++;
        }
    }
    fclose(f);
    return STATUS_SUCCESS;
}

NTSTATUS RegressionReplay_Save(RegressionReplay* rr) {
    if (!rr || !rr->initialized) return STATUS_INVALID_PARAMETER;
    FILE* f = fopen(rr->path, "w");
    if (!f) return STATUS_UNSUCCESSFUL;
    for (uint32_t i = 0; i < rr->count; i++) {
        fprintf(f, "%s\t%s\n", rr->entries[i].test_name, rr->entries[i].signature);
    }
    fclose(f);
    return STATUS_SUCCESS;
}

NTSTATUS RegressionReplay_AddFailure(RegressionReplay* rr,
    const char* test_name, const char* signature) {
    if (!rr || !rr->initialized || !test_name) return STATUS_INVALID_PARAMETER;
    for (uint32_t i = 0; i < rr->count; i++) {
        if (strcmp(rr->entries[i].test_name, test_name) == 0) {
            rr->entries[i].replay_count++;
            return RegressionReplay_Save(rr);
        }
    }
    if (rr->count >= REGRESSION_REPLAY_MAX_TESTS) return STATUS_UNSUCCESSFUL;
    strncpy(rr->entries[rr->count].test_name, test_name, REGRESSION_REPLAY_NAME_LEN - 1);
    rr->entries[rr->count].test_name[REGRESSION_REPLAY_NAME_LEN - 1] = '\0';
    if (signature) {
        strncpy(rr->entries[rr->count].signature, signature, REGRESSION_REPLAY_SIGNATURE_LEN - 1);
        rr->entries[rr->count].signature[REGRESSION_REPLAY_SIGNATURE_LEN - 1] = '\0';
    } else {
        rr->entries[rr->count].signature[0] = '\0';
    }
    rr->entries[rr->count].first_seen_ms = GetTickCount64();
    rr->entries[rr->count].replay_count = 1;
    rr->count++;
    return RegressionReplay_Save(rr);
}

uint32_t RegressionReplay_GetCount(const RegressionReplay* rr) {
    return rr ? rr->count : 0;
}

NTSTATUS RegressionReplay_GetEntry(const RegressionReplay* rr, uint32_t index,
    char* test_name, size_t name_size, char* signature, size_t sig_size) {
    if (!rr || index >= rr->count) return STATUS_INVALID_PARAMETER;
    if (test_name && name_size > 0) {
        strncpy(test_name, rr->entries[index].test_name, name_size - 1);
        test_name[name_size - 1] = '\0';
    }
    if (signature && sig_size > 0) {
        strncpy(signature, rr->entries[index].signature, sig_size - 1);
        signature[sig_size - 1] = '\0';
    }
    return STATUS_SUCCESS;
}
