#include "../../Include/red_team.h"
#include "../../Include/task_oracle.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

static bool CriticDemandVerify(RedTeam* rt, const uint8_t* output, size_t size) {
    if (!output || size == 0) return true;
    uint32_t violations = TaskOracle_PropertyCheck(output, size);
    return violations == 0;
}

static bool ProverCheckExec(RedTeam* rt, const uint8_t* output, size_t size, double loss) {
    if (!output || size == 0) return loss <= TASK_ORACLE_LOSS_PASS_THRESHOLD;
    TaskOracleResult res = {0};
    NTSTATUS st = TaskOracle_Evaluate(CURRICULUM_CODING, output, NULL, size, loss, &res);
    return NT_SUCCESS(st) && res.passed && res.property_violations == 0;
}

NTSTATUS RedTeam_Initialize(RedTeam* rt,
    NeuralSubstrate* neural,
    StressTestFramework* stf,
    AdversarialStressContext* adv) {
    if (!rt || !neural) return STATUS_INVALID_PARAMETER;
    memset(rt, 0, sizeof(RedTeam));
    rt->neural = neural;
    rt->stress_framework = stf;
    rt->adversarial_stress = adv;
    rt->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS RedTeam_Shutdown(RedTeam* rt) {
    if (!rt) return STATUS_INVALID_PARAMETER;
    rt->initialized = false;
    return STATUS_SUCCESS;
}

#define RED_TEAM_BUFFER_SIZE 4096

NTSTATUS RedTeam_Attack(RedTeam* rt, RedTeamAttackType type) {
    if (!rt || !rt->initialized) return STATUS_INVALID_PARAMETER;

    rt->last_result.attack_type = type;
    rt->last_result.induced_failure = false;
    rt->last_result.verification_passed = true;
    rt->last_result.timestamp_ms = GetTimeMs();
    rt->last_result.robustness_contribution = 0.0;
    rt->last_result.verification_hint[0] = '\0';

    if (type == RED_ATTACK_CRITIC_DEMAND_VERIFY) {
        uint8_t probe[RED_TEAM_BUFFER_SIZE];
        memset(probe, 0x40, sizeof(probe));
        size_t out_size = sizeof(probe);
        NTSTATUS st = NeuralSubstrate_Process(rt->neural, probe, sizeof(probe), probe, out_size);
        if (NT_SUCCESS(st)) {
            if (!CriticDemandVerify(rt, probe, out_size)) {
                rt->last_result.induced_failure = true;
                rt->last_result.verification_passed = false;
                rt->failure_induced_count++;
                rt->verification_failed_count++;
                snprintf(rt->last_result.verification_hint, sizeof(rt->last_result.verification_hint),
                    "critic: property violations in output");
            }
        } else {
            rt->last_result.induced_failure = true;
            rt->last_result.verification_passed = false;
            rt->failure_induced_count++;
            rt->verification_failed_count++;
            snprintf(rt->last_result.verification_hint, sizeof(rt->last_result.verification_hint),
                "critic: process failed 0x%08lX", (unsigned long)(NTSTATUS)st);
        }
        rt->attack_count++;
        return STATUS_SUCCESS;
    }

    if (type == RED_ATTACK_PROVER_CHECK_EXEC) {
        uint8_t probe[RED_TEAM_BUFFER_SIZE];
        memset(probe, 0x80, sizeof(probe));
        size_t out_size = sizeof(probe);
        NTSTATUS st = NeuralSubstrate_Process(rt->neural, probe, sizeof(probe), probe, out_size);
        if (NT_SUCCESS(st)) {
            double loss = 0.25;
            if (!ProverCheckExec(rt, probe, out_size, loss)) {
                rt->last_result.induced_failure = true;
                rt->last_result.verification_passed = false;
                rt->failure_induced_count++;
                rt->verification_failed_count++;
                snprintf(rt->last_result.verification_hint, sizeof(rt->last_result.verification_hint),
                    "prover: executable verification failed");
            }
        } else {
            rt->last_result.induced_failure = true;
            rt->last_result.verification_passed = false;
            rt->failure_induced_count++;
            rt->verification_failed_count++;
            snprintf(rt->last_result.verification_hint, sizeof(rt->last_result.verification_hint),
                "prover: process failed 0x%08lX", (unsigned long)(NTSTATUS)st);
        }
        rt->attack_count++;
        return STATUS_SUCCESS;
    }

    if (rt->stress_framework && rt->stress_framework->initialized) {
        StressTestType st = (StressTestType)((uint32_t)type % 5);
        NTSTATUS status = StressTestFramework_RunStressTest(rt->stress_framework, st);
        if (NT_SUCCESS(status)) {
            rt->last_result.verification_passed = rt->stress_framework->last_result.passed;
            rt->last_result.robustness_contribution = rt->stress_framework->last_result.robustness_score;
            if (!rt->last_result.verification_passed) {
                rt->last_result.induced_failure = true;
                rt->failure_induced_count++;
                rt->verification_failed_count++;
                snprintf(rt->last_result.verification_hint, sizeof(rt->last_result.verification_hint),
                    "stress type %u failed: %s", (unsigned)st, rt->stress_framework->last_result.description);
            }
        }
    }

    if (rt->adversarial_stress && rt->adversarial_stress->initialized) {
        AdversarialStressType ast = (AdversarialStressType)((uint32_t)type % ADVERSARIAL_STRESS_TYPE_COUNT);
        NTSTATUS status = AdversarialStress_Inject(rt->adversarial_stress, ast);
        if (NT_SUCCESS(status)) {
            AdversarialStressResult ar;
            AdversarialStress_GetLastResult(rt->adversarial_stress, &ar);
            if (!ar.process_returned_success || !ar.output_bounded) {
                rt->last_result.induced_failure = true;
                rt->last_result.verification_passed = false;
                rt->failure_induced_count++;
                rt->verification_failed_count++;
                snprintf(rt->last_result.verification_hint, sizeof(rt->last_result.verification_hint),
                    "adversarial type %u: process_ok=%d output_bounded=%d",
                    (unsigned)ast, ar.process_returned_success ? 1 : 0, ar.output_bounded ? 1 : 0);
            }
            rt->last_result.robustness_contribution += ar.robustness_contribution;
        }
    }

    rt->attack_count++;
    return STATUS_SUCCESS;
}

NTSTATUS RedTeam_RunCycle(RedTeam* rt) {
    if (!rt || !rt->initialized) return STATUS_INVALID_PARAMETER;
    RedTeamAttackType t = (RedTeamAttackType)(rt->attack_count % RED_TEAM_ATTACK_TYPES);
    return RedTeam_Attack(rt, t);
}

void RedTeam_GetLastResult(const RedTeam* rt, RedTeamResult* out) {
    if (!rt || !out) return;
    *out = rt->last_result;
}

uint32_t RedTeam_GetFailureInducedCount(const RedTeam* rt) {
    return rt ? rt->failure_induced_count : 0;
}

uint32_t RedTeam_GetVerificationFailedCount(const RedTeam* rt) {
    return rt ? rt->verification_failed_count : 0;
}
