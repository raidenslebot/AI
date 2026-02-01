#ifndef RAIJIN_RUNTIME_CONFIG_H
#define RAIJIN_RUNTIME_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define RUNTIME_CONFIG_METRIC_HISTORY 16
#define RUNTIME_CONFIG_MIN_EVOLUTION_INTERVAL 1
#define RUNTIME_CONFIG_MAX_EVOLUTION_INTERVAL 20
#define RUNTIME_CONFIG_MIN_SAVE_INTERVAL 20
#define RUNTIME_CONFIG_MAX_SAVE_INTERVAL 500
#define RUNTIME_CONFIG_MIN_SELF_TEST_INTERVAL 100
#define RUNTIME_CONFIG_MAX_SELF_TEST_INTERVAL 2000
#define RUNTIME_CONFIG_MIN_STRESS_INTERVAL 10
#define RUNTIME_CONFIG_MAX_STRESS_INTERVAL 200

typedef struct RuntimeConfig {
    uint32_t evolution_interval;
    uint32_t save_interval;
    uint32_t self_test_interval;
    uint32_t stress_interval;
    uint32_t seed;
    double metric_loss[RUNTIME_CONFIG_METRIC_HISTORY];
    double metric_fitness[RUNTIME_CONFIG_METRIC_HISTORY];
    uint32_t metric_count;
    uint32_t metric_index;
    bool initialized;
} RuntimeConfig;

void RuntimeConfig_GetDefault(RuntimeConfig* cfg);
void RuntimeConfig_Update(RuntimeConfig* cfg, double loss, double fitness, uint64_t step_count);

#endif
