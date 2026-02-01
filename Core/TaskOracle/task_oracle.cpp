#include "../../Include/task_oracle.h"
#include <string.h>
#include <math.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

double TaskOracle_ScoreFromLoss(double loss) {
    if (loss <= 0.0) return 1.0;
    if (loss >= 1.0) return 0.0;
    return 1.0 - loss;
}

uint32_t TaskOracle_PropertyCheck(const uint8_t* output, size_t size) {
    if (!output) return 1;
    if (size == 0) return 0;
    uint32_t violations = 0;
    uint8_t first = output[0];
    size_t same = 0;
    for (size_t i = 0; i < size; i++) {
        if (output[i] == first) same++;
    }
    if (same == size) violations++;
    return violations;
}

NTSTATUS TaskOracle_Evaluate(CurriculumTaskType task_type,
    const uint8_t* output, const uint8_t* target, size_t size,
    double loss, TaskOracleResult* out) {
    if (!out) return STATUS_INVALID_PARAMETER;
    memset(out, 0, sizeof(TaskOracleResult));
    (void)task_type;
    (void)target;

    uint32_t prop = TaskOracle_PropertyCheck(output, size);
    out->property_violations = prop;

    double loss_score = TaskOracle_ScoreFromLoss(loss);
    double prop_penalty = (prop > 0) ? (1.0 / (1.0 + (double)prop)) : 1.0;
    out->score = loss_score * prop_penalty;
    if (out->score > 1.0) out->score = 1.0;
    if (out->score < 0.0) out->score = 0.0;

    out->passed = (loss <= TASK_ORACLE_LOSS_PASS_THRESHOLD && prop == 0);
    if (loss <= TASK_ORACLE_LOSS_PASS_THRESHOLD && prop > 0)
        out->passed = (out->score >= 0.5);

    return STATUS_SUCCESS;
}
