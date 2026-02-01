#ifndef RAIJIN_TASK_ORACLE_H
#define RAIJIN_TASK_ORACLE_H

#include "raijin_ntstatus.h"
#include "curriculum.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define TASK_ORACLE_LOSS_PASS_THRESHOLD 0.5
#define TASK_ORACLE_CONSISTENCY_WINDOW 16

typedef struct TaskOracleResult {
    double score;
    bool passed;
    uint32_t property_violations;
} TaskOracleResult;

NTSTATUS TaskOracle_Evaluate(CurriculumTaskType task_type,
    const uint8_t* output, const uint8_t* target, size_t size,
    double loss, TaskOracleResult* out);

double TaskOracle_ScoreFromLoss(double loss);
uint32_t TaskOracle_PropertyCheck(const uint8_t* output, size_t size);

#endif
