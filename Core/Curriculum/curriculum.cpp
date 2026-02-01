#include "../../Include/curriculum.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

#define CURRICULUM_CAP 64

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

static const char* TASK_SPEC_TEMPLATES[][3] = {
    {"implement function hash_map", "optimize loop nest level %u", "parse expression grammar"},
    {"deduce invariant from trace", "derive contradiction from premises", "rank hypotheses by likelihood"},
    {"schedule tasks with deps", "allocate resources under budget", "plan steps to goal %u"},
    {"locate null deref in trace", "find off-by-one in loop", "identify race condition"},
    {"extract function from block", "rename and inline safely", "split module by cohesion"},
    {"multi-step codebase change %u", "refactor and test pipeline", "migrate API across modules"}
};

NTSTATUS Curriculum_Initialize(Curriculum* cu) {
    if (!cu) return STATUS_INVALID_PARAMETER;
    memset(cu, 0, sizeof(Curriculum));
    cu->task_capacity = CURRICULUM_CAP;
    cu->tasks = (CurriculumTask*)malloc(sizeof(CurriculumTask) * cu->task_capacity);
    if (!cu->tasks) return STATUS_INSUFFICIENT_RESOURCES;
    memset(cu->tasks, 0, sizeof(CurriculumTask) * cu->task_capacity);
    cu->current_difficulty = 2;
    cu->seed = (uint32_t)GetTimeMs();
    cu->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS Curriculum_Shutdown(Curriculum* cu) {
    if (!cu) return STATUS_INVALID_PARAMETER;
    if (cu->tasks) {
        free(cu->tasks);
        cu->tasks = NULL;
    }
    cu->task_count = 0;
    cu->task_capacity = 0;
    cu->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS Curriculum_UpdateFromPerformance(Curriculum* cu, double performance_delta) {
    if (!cu || !cu->initialized) return STATUS_INVALID_PARAMETER;
    cu->last_performance_delta = performance_delta;
    if (performance_delta > 0.1 && cu->current_difficulty < CURRICULUM_DIFFICULTY_LEVELS - 1) {
        cu->current_difficulty++;
    } else if (performance_delta < -0.1 && cu->current_difficulty > 0) {
        cu->current_difficulty--;
    }
    return STATUS_SUCCESS;
}

void Curriculum_SetDegradationMode(Curriculum* cu, uint32_t mode) {
    if (cu) cu->degradation_mode = (mode > 2u) ? 2u : mode;
}

static uint32_t Curriculum_EffectiveDifficulty(const Curriculum* cu) {
    if (!cu) return 0;
    uint32_t d = cu->current_difficulty;
    if (cu->degradation_mode >= 2u) {
        if (d > 2u) d = 2u;
    } else if (cu->degradation_mode >= 1u) {
        if (d > 5u) d = 5u;
    }
    return d;
}

NTSTATUS Curriculum_SynthesizeTask(Curriculum* cu, CurriculumTaskType type,
    CurriculumTask* out) {
    if (!cu || !cu->initialized || !out) return STATUS_INVALID_PARAMETER;
    memset(out, 0, sizeof(CurriculumTask));
    out->type = type;
    out->difficulty = Curriculum_EffectiveDifficulty(cu);
    out->timestamp_ms = GetTimeMs();
    uint32_t t = (uint32_t)type % 6;
    uint32_t tmpl = cu->seed % 3;
    const char* spec = TASK_SPEC_TEMPLATES[t][tmpl];
    int n = snprintf(out->spec, sizeof(out->spec), spec, out->difficulty);
    if (n > 0 && (size_t)n < sizeof(out->spec))
        out->spec_len = (uint32_t)n;
    else
        out->spec_len = (uint32_t)strlen(spec);
    return STATUS_SUCCESS;
}

NTSTATUS Curriculum_GetNextTask(Curriculum* cu, CurriculumTask* out) {
    if (!cu || !cu->initialized || !out) return STATUS_INVALID_PARAMETER;
    CurriculumTaskType t = (CurriculumTaskType)((cu->seed + cu->task_count) % CURRICULUM_TASK_TYPE_COUNT);
    return Curriculum_SynthesizeTask(cu, t, out);
}

uint32_t Curriculum_GetCurrentDifficulty(const Curriculum* cu) {
    return cu ? Curriculum_EffectiveDifficulty(cu) : 0;
}
