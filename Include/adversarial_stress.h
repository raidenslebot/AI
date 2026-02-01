#ifndef RAIJIN_ADVERSARIAL_STRESS_H
#define RAIJIN_ADVERSARIAL_STRESS_H

#include "raijin_ntstatus.h"
#include "neural_substrate.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    ADVERSARIAL_MALFORMED_NULL_INPUT = 0,
    ADVERSARIAL_MALFORMED_ZERO_SIZE = 1,
    ADVERSARIAL_RANDOM_NOISE = 2,
    ADVERSARIAL_EXTREME_VALUES = 3,
    ADVERSARIAL_OVERSIZED_INPUT = 4,
    ADVERSARIAL_CORRUPT_ALIGNMENT = 5,
    ADVERSARIAL_STRESS_TYPE_COUNT = 6
} AdversarialStressType;

typedef struct AdversarialStressResult {
    AdversarialStressType type;
    bool process_returned_success;
    bool output_bounded;
    uint64_t timestamp_ms;
    double robustness_contribution;
} AdversarialStressResult;

typedef struct AdversarialStressContext {
    NeuralSubstrate* neural;
    uint8_t* hostile_input_buffer;
    uint8_t* output_buffer;
    size_t buffer_size;
    uint32_t injection_count_per_type[ADVERSARIAL_STRESS_TYPE_COUNT];
    uint32_t survival_count_per_type[ADVERSARIAL_STRESS_TYPE_COUNT];
    double robustness_score;
    AdversarialStressResult last_result;
    bool initialized;
} AdversarialStressContext;

NTSTATUS AdversarialStress_Initialize(AdversarialStressContext* ctx,
    NeuralSubstrate* neural, size_t buffer_size);
NTSTATUS AdversarialStress_Shutdown(AdversarialStressContext* ctx);

NTSTATUS AdversarialStress_Inject(AdversarialStressContext* ctx, AdversarialStressType type);

NTSTATUS AdversarialStress_RunCycle(AdversarialStressContext* ctx);

void AdversarialStress_GetRobustnessScore(const AdversarialStressContext* ctx, double* score);
void AdversarialStress_GetLastResult(const AdversarialStressContext* ctx, AdversarialStressResult* out);

#endif
