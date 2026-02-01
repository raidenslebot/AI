#ifndef RAIJIN_RED_TEAM_H
#define RAIJIN_RED_TEAM_H

#include "raijin_ntstatus.h"
#include "neural_substrate.h"
#include "stress_test_framework.h"
#include "adversarial_stress.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define RED_TEAM_ATTACK_TYPES 8
#define RED_TEAM_VERIFICATION_BUF 512

typedef enum {
    RED_ATTACK_ASSUMPTION_INPUT_BOUNDED = 0,
    RED_ATTACK_ASSUMPTION_OUTPUT_STABLE = 1,
    RED_ATTACK_EDGE_CASE_EMPTY = 2,
    RED_ATTACK_EDGE_CASE_EXTREME = 3,
    RED_ATTACK_FAILURE_INJECTION = 4,
    RED_ATTACK_CRITIC_DEMAND_VERIFY = 5,
    RED_ATTACK_PROVER_CHECK_EXEC = 6,
    RED_ATTACK_CONTINUOUS_STRESS = 7
} RedTeamAttackType;

typedef struct RedTeamResult {
    RedTeamAttackType attack_type;
    bool induced_failure;
    bool verification_passed;
    char verification_hint[RED_TEAM_VERIFICATION_BUF];
    uint64_t timestamp_ms;
    double robustness_contribution;
} RedTeamResult;

typedef struct RedTeam {
    NeuralSubstrate* neural;
    StressTestFramework* stress_framework;
    AdversarialStressContext* adversarial_stress;
    RedTeamResult last_result;
    uint32_t attack_count;
    uint32_t failure_induced_count;
    uint32_t verification_failed_count;
    bool initialized;
} RedTeam;

NTSTATUS RedTeam_Initialize(RedTeam* rt,
    NeuralSubstrate* neural,
    StressTestFramework* stf,
    AdversarialStressContext* adv);
NTSTATUS RedTeam_Shutdown(RedTeam* rt);

NTSTATUS RedTeam_Attack(RedTeam* rt, RedTeamAttackType type);

NTSTATUS RedTeam_RunCycle(RedTeam* rt);

void RedTeam_GetLastResult(const RedTeam* rt, RedTeamResult* out);
uint32_t RedTeam_GetFailureInducedCount(const RedTeam* rt);
uint32_t RedTeam_GetVerificationFailedCount(const RedTeam* rt);

#endif
