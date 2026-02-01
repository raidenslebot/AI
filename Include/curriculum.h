#ifndef RAIJIN_CURRICULUM_H
#define RAIJIN_CURRICULUM_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define CURRICULUM_TASK_BUF 256
#define CURRICULUM_MAX_TASKS 64
#define CURRICULUM_DIFFICULTY_LEVELS 10

typedef enum {
    CURRICULUM_CODING = 0,
    CURRICULUM_REASONING = 1,
    CURRICULUM_PLANNING = 2,
    CURRICULUM_DEBUGGING = 3,
    CURRICULUM_REFACTORING = 4,
    CURRICULUM_LONG_HORIZON = 5,
    CURRICULUM_TASK_TYPE_COUNT = 6
} CurriculumTaskType;

typedef struct CurriculumTask {
    CurriculumTaskType type;
    uint32_t difficulty;  /* 0..CURRICULUM_DIFFICULTY_LEVELS-1 */
    char spec[CURRICULUM_TASK_BUF];
    uint32_t spec_len;
    uint64_t timestamp_ms;
} CurriculumTask;

typedef struct Curriculum {
    CurriculumTask* tasks;
    uint32_t task_count;
    uint32_t task_capacity;
    double last_performance_delta;
    uint32_t current_difficulty;
    uint32_t degradation_mode;  /* 0=normal, 1=reduced, 2=minimal; caps difficulty */
    uint32_t seed;
    bool initialized;
} Curriculum;

NTSTATUS Curriculum_Initialize(Curriculum* cu);
NTSTATUS Curriculum_Shutdown(Curriculum* cu);

NTSTATUS Curriculum_UpdateFromPerformance(Curriculum* cu, double performance_delta);
void Curriculum_SetDegradationMode(Curriculum* cu, uint32_t mode);

NTSTATUS Curriculum_SynthesizeTask(Curriculum* cu, CurriculumTaskType type,
    CurriculumTask* out);

NTSTATUS Curriculum_GetNextTask(Curriculum* cu, CurriculumTask* out);

uint32_t Curriculum_GetCurrentDifficulty(const Curriculum* cu);  /* respects degradation cap */

#endif
