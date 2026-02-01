#ifndef RAIJIN_INTROSPECTION_SYSTEM_H
#define RAIJIN_INTROSPECTION_SYSTEM_H

#include "raijin_ntstatus.h"
#include "dominance_metrics.h"
#include "neural_substrate.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define INTROSPECTION_CRITIQUE_MAX 2048

typedef struct IntrospectionSystem {
    DominanceMetrics* dominance_metrics;
    NeuralSubstrate* neural;
    char last_critique[INTROSPECTION_CRITIQUE_MAX];
    double self_observation_score;
    double critique_severity;
    uint64_t observation_count;
    bool initialized;
} IntrospectionSystem;

NTSTATUS IntrospectionSystem_Initialize(IntrospectionSystem* is,
    DominanceMetrics* dm,
    NeuralSubstrate* neural);
NTSTATUS IntrospectionSystem_Shutdown(IntrospectionSystem* is);

NTSTATUS IntrospectionSystem_Observe(IntrospectionSystem* is);

NTSTATUS IntrospectionSystem_Critique(IntrospectionSystem* is, char* critique, size_t critique_size);

void IntrospectionSystem_GetLastCritique(const IntrospectionSystem* is, char* out, size_t out_size);
double IntrospectionSystem_GetSelfObservationScore(const IntrospectionSystem* is);
double IntrospectionSystem_GetCritiqueSeverity(const IntrospectionSystem* is);

#endif
