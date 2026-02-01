#include "../../Include/fitness_ledger.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static const double WEIGHT_CORRECTNESS = 0.2;
static const double WEIGHT_ROBUSTNESS = 0.2;
static const double WEIGHT_EFFICIENCY = 0.15;
static const double WEIGHT_RECOVERY = 0.15;
static const double WEIGHT_REGRESSION = 0.15;
static const double WEIGHT_LEARNING = 0.1;
static const double WEIGHT_COHERENCE = 0.05;

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

NTSTATUS FitnessLedger_Initialize(FitnessLedger* fl,
    DominanceMetrics* dm, RegressionDetector* rd,
    LineageTracker* lt, VersioningRollback* vr) {
    if (!fl) return STATUS_INVALID_PARAMETER;
    memset(fl, 0, sizeof(FitnessLedger));
    fl->dominance_metrics = dm;
    fl->regression_detector = rd;
    fl->lineage = lt;
    fl->versioning_rollback = vr;
    fl->best.composite = -1.0;
    fl->worst.composite = 2.0;
    fl->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS FitnessLedger_Shutdown(FitnessLedger* fl) {
    if (!fl) return STATUS_INVALID_PARAMETER;
    fl->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS FitnessLedger_Update(FitnessLedger* fl,
    double test_pass_rate, double adversarial_robustness,
    uint64_t step_count, uint64_t version_id) {
    if (!fl || !fl->initialized) return STATUS_INVALID_PARAMETER;

    FitnessScore* cur = &fl->current;
    cur->correctness = (test_pass_rate >= 0.0 && test_pass_rate <= 1.0) ? test_pass_rate : 0.5;
    cur->robustness = (adversarial_robustness >= 0.0 && adversarial_robustness <= 1.0) ? adversarial_robustness : 0.5;
    cur->step_count = step_count;
    cur->version_id = version_id;
    cur->timestamp_ms = GetTimeMs();

    if (fl->dominance_metrics && fl->dominance_metrics->initialized) {
        DominanceSnapshot snap;
        DominanceMetrics_GetCurrent(fl->dominance_metrics, &snap);
        cur->efficiency = snap.efficiency;
        cur->coherence = snap.coherence;
        cur->learning_velocity = (snap.adaptability >= 0.0) ? snap.adaptability : 0.0;
    } else {
        cur->efficiency = 0.5;
        cur->coherence = 0.5;
        cur->learning_velocity = 0.0;
    }

    if (fl->regression_detector && fl->regression_detector->initialized) {
        cur->regression_rate = RegressionDetector_IsDegenerationDetected(fl->regression_detector) ? 0.0 : 1.0;
        cur->regression_rate = RegressionDetector_IsRegressionDetected(fl->regression_detector) ? 0.5 : cur->regression_rate;
    } else {
        cur->regression_rate = 1.0;
    }

    cur->recovery = (fl->versioning_rollback && fl->versioning_rollback->initialized)
        ? (1.0 / (1.0 + (double)VersioningRollback_GetRollbackCount(fl->versioning_rollback)))
        : 1.0;

    cur->composite = WEIGHT_CORRECTNESS * cur->correctness +
        WEIGHT_ROBUSTNESS * cur->robustness +
        WEIGHT_EFFICIENCY * cur->efficiency +
        WEIGHT_RECOVERY * cur->recovery +
        WEIGHT_REGRESSION * cur->regression_rate +
        WEIGHT_LEARNING * cur->learning_velocity +
        WEIGHT_COHERENCE * cur->coherence;

    if (cur->composite > fl->best.composite || fl->best.composite < 0.0) {
        fl->best = *cur;
    }
    if (cur->composite < fl->worst.composite) {
        fl->worst = *cur;
    }

    return STATUS_SUCCESS;
}

void FitnessLedger_GetCurrent(const FitnessLedger* fl, FitnessScore* out) {
    if (!fl || !out) return;
    *out = fl->current;
}

void FitnessLedger_GetBest(const FitnessLedger* fl, FitnessScore* out) {
    if (!fl || !out) return;
    *out = fl->best;
}

NTSTATUS FitnessLedger_PromoteBest(FitnessLedger* fl) {
    if (!fl || !fl->initialized || !fl->versioning_rollback) return STATUS_INVALID_PARAMETER;
    NTSTATUS status = VersioningRollback_RollbackToBest(fl->versioning_rollback);
    if (NT_SUCCESS(status))
        fl->promotion_count++;
    return status;
}

NTSTATUS FitnessLedger_DemoteWorst(FitnessLedger* fl) {
    if (!fl || !fl->initialized || !fl->versioning_rollback) return STATUS_INVALID_PARAMETER;
    NTSTATUS status = VersioningRollback_RollbackToBest(fl->versioning_rollback);
    if (NT_SUCCESS(status))
        fl->demotion_count++;
    return status;
}

uint32_t FitnessLedger_GetPromotionCount(const FitnessLedger* fl) {
    return fl ? fl->promotion_count : 0;
}

uint32_t FitnessLedger_GetDemotionCount(const FitnessLedger* fl) {
    return fl ? fl->demotion_count : 0;
}
