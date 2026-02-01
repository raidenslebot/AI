#ifndef RAIJIN_FITNESS_LEDGER_H
#define RAIJIN_FITNESS_LEDGER_H

#include "raijin_ntstatus.h"
#include "dominance_metrics.h"
#include "regression_detector.h"
#include "lineage_tracker.h"
#include "versioning_rollback.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define FITNESS_LEDGER_SCORE_COUNT 7

typedef struct FitnessScore {
    double correctness;   /* test pass rate 0-1 */
    double robustness;    /* adversarial/stress 0-1 */
    double efficiency;   /* time/memory 0-1 */
    double recovery;     /* rollback success 0-1 */
    double regression_rate; /* inverse: 1 - regression_freq */
    double learning_velocity; /* fitness delta over time */
    double coherence;    /* internal coherence 0-1 */
    double composite;    /* weighted composite */
    uint64_t version_id;
    uint64_t step_count;
    uint64_t timestamp_ms;
} FitnessScore;

typedef struct FitnessLedger {
    DominanceMetrics* dominance_metrics;
    RegressionDetector* regression_detector;
    LineageTracker* lineage;
    VersioningRollback* versioning_rollback;
    FitnessScore current;
    FitnessScore best;
    FitnessScore worst;
    uint32_t promotion_count;
    uint32_t demotion_count;
    bool initialized;
} FitnessLedger;

NTSTATUS FitnessLedger_Initialize(FitnessLedger* fl,
    DominanceMetrics* dm, RegressionDetector* rd,
    LineageTracker* lt, VersioningRollback* vr);
NTSTATUS FitnessLedger_Shutdown(FitnessLedger* fl);

NTSTATUS FitnessLedger_Update(FitnessLedger* fl,
    double test_pass_rate, double adversarial_robustness,
    uint64_t step_count, uint64_t version_id);

void FitnessLedger_GetCurrent(const FitnessLedger* fl, FitnessScore* out);
void FitnessLedger_GetBest(const FitnessLedger* fl, FitnessScore* out);

NTSTATUS FitnessLedger_PromoteBest(FitnessLedger* fl);
NTSTATUS FitnessLedger_DemoteWorst(FitnessLedger* fl);

uint32_t FitnessLedger_GetPromotionCount(const FitnessLedger* fl);
uint32_t FitnessLedger_GetDemotionCount(const FitnessLedger* fl);

#endif
