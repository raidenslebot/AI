#include "../../Include/runtime_config.h"
#include <string.h>
#include <time.h>

static const uint32_t DEFAULT_EVOLUTION_INTERVAL = 5;
static const uint32_t DEFAULT_SAVE_INTERVAL = 100;
static const uint32_t DEFAULT_SELF_TEST_INTERVAL = 500;
static const uint32_t DEFAULT_STRESS_INTERVAL = 30;

static double clamp_double(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void RuntimeConfig_GetDefault(RuntimeConfig* cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(RuntimeConfig));
    cfg->evolution_interval = DEFAULT_EVOLUTION_INTERVAL;
    cfg->save_interval = DEFAULT_SAVE_INTERVAL;
    cfg->self_test_interval = DEFAULT_SELF_TEST_INTERVAL;
    cfg->stress_interval = DEFAULT_STRESS_INTERVAL;
    cfg->seed = (uint32_t)time(NULL);
    cfg->initialized = true;
}

void RuntimeConfig_Update(RuntimeConfig* cfg, double loss, double fitness, uint64_t step_count) {
    (void)step_count;
    if (!cfg || !cfg->initialized) return;

    if (cfg->metric_count < RUNTIME_CONFIG_METRIC_HISTORY) {
        cfg->metric_loss[cfg->metric_count] = loss;
        cfg->metric_fitness[cfg->metric_count] = fitness;
        cfg->metric_count++;
    } else {
        cfg->metric_loss[cfg->metric_index] = loss;
        cfg->metric_fitness[cfg->metric_index] = fitness;
        cfg->metric_index = (cfg->metric_index + 1) % RUNTIME_CONFIG_METRIC_HISTORY;
    }

    if (cfg->metric_count < 4) return;

    double sum_loss = 0.0, sum_fitness = 0.0;
    uint32_t n = cfg->metric_count;
    for (uint32_t i = 0; i < n; i++) {
        sum_loss += cfg->metric_loss[i];
        sum_fitness += cfg->metric_fitness[i];
    }
    double avg_loss = sum_loss / n;
    (void)avg_loss;

    double loss_slope = 0.0;
    double fitness_slope = 0.0;
    if (n >= 2) {
        uint32_t recent_idx = (cfg->metric_count >= RUNTIME_CONFIG_METRIC_HISTORY)
            ? (cfg->metric_index + RUNTIME_CONFIG_METRIC_HISTORY - 1) % RUNTIME_CONFIG_METRIC_HISTORY
            : n - 1;
        uint32_t old_idx = (cfg->metric_count >= RUNTIME_CONFIG_METRIC_HISTORY)
            ? cfg->metric_index
            : 0;
        double recent_loss = cfg->metric_loss[recent_idx];
        double old_loss = cfg->metric_loss[old_idx];
        double recent_fitness = cfg->metric_fitness[recent_idx];
        double old_fitness = cfg->metric_fitness[old_idx];
        loss_slope = recent_loss - old_loss;
        fitness_slope = recent_fitness - old_fitness;
    }

    if (fitness_slope > 0.01) {
        cfg->evolution_interval = (cfg->evolution_interval > RUNTIME_CONFIG_MIN_EVOLUTION_INTERVAL)
            ? cfg->evolution_interval - 1
            : RUNTIME_CONFIG_MIN_EVOLUTION_INTERVAL;
    } else if (fitness_slope < -0.01) {
        cfg->evolution_interval = (cfg->evolution_interval < RUNTIME_CONFIG_MAX_EVOLUTION_INTERVAL)
            ? cfg->evolution_interval + 1
            : RUNTIME_CONFIG_MAX_EVOLUTION_INTERVAL;
    }

    if (loss_slope > 0.05 || avg_loss > 0.8) {
        cfg->save_interval = (cfg->save_interval > RUNTIME_CONFIG_MIN_SAVE_INTERVAL)
            ? cfg->save_interval - 10
            : RUNTIME_CONFIG_MIN_SAVE_INTERVAL;
    } else if (loss_slope < -0.02 && avg_loss < 0.3) {
        cfg->save_interval = (cfg->save_interval < RUNTIME_CONFIG_MAX_SAVE_INTERVAL)
            ? cfg->save_interval + 20
            : RUNTIME_CONFIG_MAX_SAVE_INTERVAL;
    }

    cfg->save_interval = (uint32_t)clamp_double((double)cfg->save_interval,
        RUNTIME_CONFIG_MIN_SAVE_INTERVAL, RUNTIME_CONFIG_MAX_SAVE_INTERVAL);
    cfg->evolution_interval = (uint32_t)clamp_double((double)cfg->evolution_interval,
        RUNTIME_CONFIG_MIN_EVOLUTION_INTERVAL, RUNTIME_CONFIG_MAX_EVOLUTION_INTERVAL);
    cfg->self_test_interval = (uint32_t)clamp_double((double)cfg->self_test_interval,
        RUNTIME_CONFIG_MIN_SELF_TEST_INTERVAL, RUNTIME_CONFIG_MAX_SELF_TEST_INTERVAL);

    if (fitness_slope < -0.02) {
        cfg->stress_interval = (cfg->stress_interval > RUNTIME_CONFIG_MIN_STRESS_INTERVAL)
            ? cfg->stress_interval - 2
            : RUNTIME_CONFIG_MIN_STRESS_INTERVAL;
    } else if (fitness_slope > 0.02) {
        cfg->stress_interval = (cfg->stress_interval < RUNTIME_CONFIG_MAX_STRESS_INTERVAL)
            ? cfg->stress_interval + 5
            : RUNTIME_CONFIG_MAX_STRESS_INTERVAL;
    }
    cfg->stress_interval = (uint32_t)clamp_double((double)cfg->stress_interval,
        RUNTIME_CONFIG_MIN_STRESS_INTERVAL, RUNTIME_CONFIG_MAX_STRESS_INTERVAL);
}
