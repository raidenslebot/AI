#include "../../Include/self_healing.h"
#include "../../Include/resource_governor.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

NTSTATUS SelfHealing_Initialize(SelfHealing* sh,
    RegressionDetector* rd,
    VersioningRollback* vr,
    DominanceMetrics* dm,
    void* resource_governor_optional) {
    if (!sh || !rd || !vr || !dm) return STATUS_INVALID_PARAMETER;
    memset(sh, 0, sizeof(SelfHealing));
    sh->regression_detector = rd;
    sh->versioning_rollback = vr;
    sh->dominance_metrics = dm;
    sh->resource_governor = (ResourceGovernor*)resource_governor_optional;
    sh->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS SelfHealing_Shutdown(SelfHealing* sh) {
    if (!sh) return STATUS_INVALID_PARAMETER;
    sh->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS SelfHealing_Evaluate(SelfHealing* sh) {
    if (!sh || !sh->initialized) return STATUS_INVALID_PARAMETER;

    if (RegressionDetector_IsDegenerationDetected(sh->regression_detector)) {
        sh->healing_trigger_count++;
        sh->rollback_trigger_count++;
        NTSTATUS status = VersioningRollback_RollbackToBest(sh->versioning_rollback);
        if (NT_SUCCESS(status)) {
            RegressionDetector_Reset(sh->regression_detector);
        }
        return status;
    }

    if (RegressionDetector_IsRegressionDetected(sh->regression_detector)) {
        RegressionEvent ev;
        RegressionDetector_GetLastEvent(sh->regression_detector, &ev);
        if (ev.severity >= 0.7) {
            sh->healing_trigger_count++;
            sh->rollback_trigger_count++;
            NTSTATUS status = VersioningRollback_RollbackToBest(sh->versioning_rollback);
            if (NT_SUCCESS(status)) {
                RegressionDetector_Reset(sh->regression_detector);
            }
            return status;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS SelfHealing_SoftRepair(SelfHealing* sh) {
    if (!sh || !sh->initialized) return STATUS_INVALID_PARAMETER;
    sh->healing_trigger_count++;
    RegressionDetector_Reset(sh->regression_detector);
    if (sh->resource_governor && sh->resource_governor->initialized)
        ResourceGovernor_ResetThrottle(sh->resource_governor);
    return STATUS_SUCCESS;
}

NTSTATUS SelfHealing_TriggerRecovery(SelfHealing* sh) {
    return SelfHealing_HardRepair(sh);
}

NTSTATUS SelfHealing_HardRepair(SelfHealing* sh) {
    if (!sh || !sh->initialized) return STATUS_INVALID_PARAMETER;
    sh->healing_trigger_count++;
    sh->rollback_trigger_count++;
    NTSTATUS status = VersioningRollback_RollbackToBest(sh->versioning_rollback);
    if (NT_SUCCESS(status)) {
        RegressionDetector_Reset(sh->regression_detector);
    }
    return status;
}

uint32_t SelfHealing_GetHealingTriggerCount(const SelfHealing* sh) {
    return sh ? sh->healing_trigger_count : 0;
}

uint32_t SelfHealing_GetRollbackTriggerCount(const SelfHealing* sh) {
    return sh ? sh->rollback_trigger_count : 0;
}
