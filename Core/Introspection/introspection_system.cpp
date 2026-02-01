#include "../../Include/introspection_system.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

NTSTATUS IntrospectionSystem_Initialize(IntrospectionSystem* is,
    DominanceMetrics* dm,
    NeuralSubstrate* neural) {
    if (!is || !dm || !neural) return STATUS_INVALID_PARAMETER;
    memset(is, 0, sizeof(IntrospectionSystem));
    is->dominance_metrics = dm;
    is->neural = neural;
    is->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS IntrospectionSystem_Shutdown(IntrospectionSystem* is) {
    if (!is) return STATUS_INVALID_PARAMETER;
    is->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS IntrospectionSystem_Observe(IntrospectionSystem* is) {
    if (!is || !is->initialized) return STATUS_INVALID_PARAMETER;

    DominanceSnapshot cur;
    DominanceMetrics_GetCurrent(is->dominance_metrics, &cur);

    double dom_trend = 0.0, eff_trend = 0.0, coh_trend = 0.0, adp_trend = 0.0;
    DominanceMetrics_GetTrends(is->dominance_metrics, &dom_trend, &eff_trend, &coh_trend, &adp_trend);

    is->self_observation_score = (cur.dominance + cur.efficiency + cur.coherence + cur.adaptability) / 4.0;
    if (is->self_observation_score > 1.0) is->self_observation_score = 1.0;
    if (is->self_observation_score < 0.0) is->self_observation_score = 0.0;

    is->observation_count++;
    return STATUS_SUCCESS;
}

NTSTATUS IntrospectionSystem_Critique(IntrospectionSystem* is, char* critique, size_t critique_size) {
    if (!is || !is->initialized) return STATUS_INVALID_PARAMETER;

    DominanceSnapshot cur;
    DominanceMetrics_GetCurrent(is->dominance_metrics, &cur);
    double dom_trend = 0.0, eff_trend = 0.0, coh_trend = 0.0, adp_trend = 0.0;
    DominanceMetrics_GetTrends(is->dominance_metrics, &dom_trend, &eff_trend, &coh_trend, &adp_trend);

    is->critique_severity = 0.0;
    int n = 0;
    n = snprintf(is->last_critique, sizeof(is->last_critique),
        "dominance=%.3f trend=%.4f efficiency=%.3f trend=%.4f coherence=%.3f trend=%.4f adaptability=%.3f trend=%.4f",
        cur.dominance, dom_trend, cur.efficiency, eff_trend, cur.coherence, coh_trend, cur.adaptability, adp_trend);

    if (dom_trend < -0.01) is->critique_severity += 0.25;
    if (eff_trend < -0.01) is->critique_severity += 0.25;
    if (coh_trend < -0.01) is->critique_severity += 0.25;
    if (adp_trend < -0.01) is->critique_severity += 0.25;
    if (cur.dominance < 0.3) is->critique_severity += 0.2;
    if (cur.coherence < 0.3) is->critique_severity += 0.2;
    if (is->critique_severity > 1.0) is->critique_severity = 1.0;

    if (critique && critique_size > 0) {
        size_t len = strlen(is->last_critique);
        if (len >= critique_size) len = critique_size - 1;
        memcpy(critique, is->last_critique, len);
        critique[len] = '\0';
    }
    return STATUS_SUCCESS;
}

void IntrospectionSystem_GetLastCritique(const IntrospectionSystem* is, char* out, size_t out_size) {
    if (!is || !out || out_size == 0) return;
    size_t len = strlen(is->last_critique);
    if (len >= out_size) len = out_size - 1;
    memcpy(out, is->last_critique, len);
    out[len] = '\0';
}

double IntrospectionSystem_GetSelfObservationScore(const IntrospectionSystem* is) {
    return is ? is->self_observation_score : 0.0;
}

double IntrospectionSystem_GetCritiqueSeverity(const IntrospectionSystem* is) {
    return is ? is->critique_severity : 0.0;
}
