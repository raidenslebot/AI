#include "../../Include/adversarial_stress.h"
#include "../../Include/neural_substrate.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <windows.h>

#ifndef NT_SUCCESS
#define NT_SUCCESS(S) (((LONG)(S)) >= 0)
#endif

static uint64_t GetTimeMs(void) {
    return (uint64_t)GetTickCount64();
}

static void FillRandomNoise(uint8_t* buf, size_t size, uint32_t seed) {
    uint32_t s = (uint32_t)(seed ^ (uint32_t)GetTimeMs());
    for (size_t i = 0; i < size; i++) {
        s = s * 1103515245 + 12345;
        buf[i] = (uint8_t)((s >> 16) & 0xFF);
    }
}

static bool IsOutputBounded(const uint8_t* output, size_t size) {
    if (!output || size == 0) return true;
    size_t nan_like = 0;
    for (size_t i = 0; i + sizeof(float) <= size; i += sizeof(float)) {
        float f;
        memcpy(&f, output + i, sizeof(float));
        if (f != f || isinf(f)) nan_like++;
    }
    return nan_like == 0;
}

NTSTATUS AdversarialStress_Initialize(AdversarialStressContext* ctx,
    NeuralSubstrate* neural, size_t buffer_size) {
    if (!ctx || !neural) return STATUS_INVALID_PARAMETER;
    memset(ctx, 0, sizeof(AdversarialStressContext));
    ctx->neural = neural;
    if (buffer_size == 0) buffer_size = 4096;
    ctx->buffer_size = buffer_size;
    ctx->hostile_input_buffer = (uint8_t*)malloc(buffer_size);
    ctx->output_buffer = (uint8_t*)malloc(buffer_size);
    if (!ctx->hostile_input_buffer || !ctx->output_buffer) {
        if (ctx->hostile_input_buffer) free(ctx->hostile_input_buffer);
        if (ctx->output_buffer) free(ctx->output_buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(ctx->hostile_input_buffer, 0, buffer_size);
    memset(ctx->output_buffer, 0, buffer_size);
    ctx->robustness_score = 1.0;
    ctx->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS AdversarialStress_Shutdown(AdversarialStressContext* ctx) {
    if (!ctx) return STATUS_INVALID_PARAMETER;
    if (ctx->hostile_input_buffer) {
        free(ctx->hostile_input_buffer);
        ctx->hostile_input_buffer = NULL;
    }
    if (ctx->output_buffer) {
        free(ctx->output_buffer);
        ctx->output_buffer = NULL;
    }
    ctx->buffer_size = 0;
    ctx->neural = NULL;
    ctx->initialized = false;
    return STATUS_SUCCESS;
}

NTSTATUS AdversarialStress_Inject(AdversarialStressContext* ctx, AdversarialStressType type) {
    if (!ctx || !ctx->initialized || !ctx->neural) return STATUS_INVALID_PARAMETER;

    AdversarialStressResult* res = &ctx->last_result;
    res->type = type;
    res->timestamp_ms = GetTimeMs();
    res->process_returned_success = false;
    res->output_bounded = false;
    res->robustness_contribution = 0.0;

    NTSTATUS status;
    const uint8_t* input_ptr = NULL;
    size_t input_size = 0;
    size_t output_size = ctx->buffer_size;

    switch (type) {
        case ADVERSARIAL_MALFORMED_NULL_INPUT:
            status = NeuralSubstrate_Process(ctx->neural, NULL, ctx->buffer_size, ctx->output_buffer, output_size);
            res->robustness_contribution = NT_SUCCESS(status) ? 0.0 : 1.0;
            res->process_returned_success = NT_SUCCESS(status) ? true : false;
            break;
        case ADVERSARIAL_MALFORMED_ZERO_SIZE:
            status = NeuralSubstrate_Process(ctx->neural, ctx->hostile_input_buffer, 0, ctx->output_buffer, output_size);
            res->robustness_contribution = NT_SUCCESS(status) ? 0.0 : 1.0;
            res->process_returned_success = NT_SUCCESS(status) ? true : false;
            break;
        case ADVERSARIAL_RANDOM_NOISE:
            FillRandomNoise(ctx->hostile_input_buffer, ctx->buffer_size, (uint32_t)res->timestamp_ms);
            input_ptr = ctx->hostile_input_buffer;
            input_size = ctx->buffer_size;
            status = NeuralSubstrate_Process(ctx->neural, input_ptr, input_size, ctx->output_buffer, output_size);
            res->process_returned_success = NT_SUCCESS(status);
            res->output_bounded = res->process_returned_success && IsOutputBounded(ctx->output_buffer, output_size);
            res->robustness_contribution = (res->process_returned_success && res->output_bounded) ? 1.0 : (res->process_returned_success ? 0.5 : 0.0);
            break;
        case ADVERSARIAL_EXTREME_VALUES:
            memset(ctx->hostile_input_buffer, 0xFF, ctx->buffer_size);
            input_ptr = ctx->hostile_input_buffer;
            input_size = ctx->buffer_size;
            status = NeuralSubstrate_Process(ctx->neural, input_ptr, input_size, ctx->output_buffer, output_size);
            res->process_returned_success = NT_SUCCESS(status);
            res->output_bounded = res->process_returned_success && IsOutputBounded(ctx->output_buffer, output_size);
            res->robustness_contribution = (res->process_returned_success && res->output_bounded) ? 1.0 : (res->process_returned_success ? 0.5 : 0.0);
            break;
        case ADVERSARIAL_OVERSIZED_INPUT:
            FillRandomNoise(ctx->hostile_input_buffer, ctx->buffer_size, (uint32_t)res->timestamp_ms);
            input_ptr = ctx->hostile_input_buffer;
            input_size = ctx->buffer_size;
            status = NeuralSubstrate_Process(ctx->neural, input_ptr, input_size, ctx->output_buffer, output_size);
            res->process_returned_success = NT_SUCCESS(status);
            res->output_bounded = res->process_returned_success && IsOutputBounded(ctx->output_buffer, output_size);
            res->robustness_contribution = (res->process_returned_success && res->output_bounded) ? 1.0 : (res->process_returned_success ? 0.5 : 0.0);
            break;
        case ADVERSARIAL_CORRUPT_ALIGNMENT:
            FillRandomNoise(ctx->hostile_input_buffer, ctx->buffer_size, (uint32_t)res->timestamp_ms);
            input_ptr = ctx->hostile_input_buffer;
            input_size = ctx->buffer_size - 1;
            if (input_size == 0) input_size = 1;
            status = NeuralSubstrate_Process(ctx->neural, input_ptr, input_size, ctx->output_buffer, output_size);
            res->process_returned_success = NT_SUCCESS(status);
            res->output_bounded = res->process_returned_success && IsOutputBounded(ctx->output_buffer, output_size);
            res->robustness_contribution = (res->process_returned_success && res->output_bounded) ? 1.0 : (res->process_returned_success ? 0.5 : 0.0);
            break;
        default:
            return STATUS_INVALID_PARAMETER;
    }

    if ((uint32_t)type < ADVERSARIAL_STRESS_TYPE_COUNT) {
        ctx->injection_count_per_type[(uint32_t)type]++;
        if (res->robustness_contribution >= 0.5)
            ctx->survival_count_per_type[(uint32_t)type]++;
    }

    uint32_t total_injections = 0;
    uint32_t total_survivals = 0;
    for (uint32_t i = 0; i < ADVERSARIAL_STRESS_TYPE_COUNT; i++) {
        total_injections += ctx->injection_count_per_type[i];
        total_survivals += ctx->survival_count_per_type[i];
    }
    ctx->robustness_score = (total_injections > 0) ? (double)total_survivals / (double)total_injections : 1.0;

    return STATUS_SUCCESS;
}

NTSTATUS AdversarialStress_RunCycle(AdversarialStressContext* ctx) {
    if (!ctx || !ctx->initialized) return STATUS_INVALID_PARAMETER;
    uint32_t seed = (uint32_t)(GetTimeMs() & 0xFFFFFFFFu);
    AdversarialStressType t = (AdversarialStressType)(seed % ADVERSARIAL_STRESS_TYPE_COUNT);
    return AdversarialStress_Inject(ctx, t);
}

void AdversarialStress_GetRobustnessScore(const AdversarialStressContext* ctx, double* score) {
    if (!ctx || !score) return;
    *score = ctx->robustness_score;
}

void AdversarialStress_GetLastResult(const AdversarialStressContext* ctx, AdversarialStressResult* out) {
    if (!ctx || !out) return;
    *out = ctx->last_result;
}
