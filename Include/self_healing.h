#ifndef RAIJIN_SELF_HEALING_H
#define RAIJIN_SELF_HEALING_H

#include "raijin_ntstatus.h"
#include "regression_detector.h"
#include "versioning_rollback.h"
#include "dominance_metrics.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct ResourceGovernor;

typedef struct SelfHealing {
    RegressionDetector* regression_detector;
    VersioningRollback* versioning_rollback;
    DominanceMetrics* dominance_metrics;
    struct ResourceGovernor* resource_governor;
    uint32_t healing_trigger_count;
    uint32_t rollback_trigger_count;
    bool initialized;
} SelfHealing;

NTSTATUS SelfHealing_Initialize(SelfHealing* sh,
    RegressionDetector* rd,
    VersioningRollback* vr,
    DominanceMetrics* dm,
    void* resource_governor_optional);
NTSTATUS SelfHealing_Shutdown(SelfHealing* sh);

NTSTATUS SelfHealing_Evaluate(SelfHealing* sh);

NTSTATUS SelfHealing_SoftRepair(SelfHealing* sh);

NTSTATUS SelfHealing_TriggerRecovery(SelfHealing* sh);

NTSTATUS SelfHealing_HardRepair(SelfHealing* sh);

uint32_t SelfHealing_GetHealingTriggerCount(const SelfHealing* sh);
uint32_t SelfHealing_GetRollbackTriggerCount(const SelfHealing* sh);

#endif
