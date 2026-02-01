#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((LONG)0)
#endif
#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER ((LONG)0xC000000D)
#endif
#ifndef STATUS_INSUFFICIENT_RESOURCES
#define STATUS_INSUFFICIENT_RESOURCES ((LONG)0xC000009A)
#endif
#ifndef STATUS_ACCESS_DENIED
#define STATUS_ACCESS_DENIED ((LONG)0xC0000022)
#endif
#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL ((LONG)0xC0000001)
#endif
#ifndef STATUS_INVALID_DEVICE_STATE
#define STATUS_INVALID_DEVICE_STATE ((LONG)0xC0000184)
#endif
#include "../../Include/ethics_system.h"

#define MAX_VALUES 256
#define MAX_HISTORY 1024
#define VALUE_DESC_MAX 256
#define REASONING_MAX 2048
#define LEARNING_RATE 0.15f
#define ALIGNMENT_THRESHOLD 0.3f
#define MIN_STRENGTH 0.01f
#define MAX_STRENGTH 1.0f

static const char* VALUE_TYPE_NAMES[] = {
    "moral_good", "moral_bad", "aesthetic", "practical",
    "emotional", "social", "personal"
};

static const char* TRAIT_NAMES[] = {
    "agreeableness", "conscientiousness", "extraversion",
    "neuroticism", "openness"
};

static const char* MORAL_GOOD_KEYWORDS[] = { "good", "right", "moral", "ethical", "kind", "help", "fair", "honest", "should", "must" };
static const char* MORAL_BAD_KEYWORDS[] = { "bad", "wrong", "evil", "harm", "lie", "cheat", "steal", "hurt" };
static const char* AESTHETIC_KEYWORDS[] = { "beautiful", "ugly", "elegant", "style", "design", "art" };
static const char* PRACTICAL_KEYWORDS[] = { "efficient", "useful", "practical", "work", "result", "goal" };
static const char* EMOTIONAL_KEYWORDS[] = { "feel", "love", "hate", "happy", "sad", "angry", "fear" };
static const char* SOCIAL_KEYWORDS[] = { "people", "society", "community", "group", "others", "relationship" };
static const char* PERSONAL_KEYWORDS[] = { "prefer", "like", "want", "need", "habit", "personal" };

static const size_t MORAL_GOOD_COUNT = sizeof(MORAL_GOOD_KEYWORDS) / sizeof(MORAL_GOOD_KEYWORDS[0]);
static const size_t MORAL_BAD_COUNT = sizeof(MORAL_BAD_KEYWORDS) / sizeof(MORAL_BAD_KEYWORDS[0]);
static const size_t AESTHETIC_COUNT = sizeof(AESTHETIC_KEYWORDS) / sizeof(AESTHETIC_KEYWORDS[0]);
static const size_t PRACTICAL_COUNT = sizeof(PRACTICAL_KEYWORDS) / sizeof(PRACTICAL_KEYWORDS[0]);
static const size_t EMOTIONAL_COUNT = sizeof(EMOTIONAL_KEYWORDS) / sizeof(EMOTIONAL_KEYWORDS[0]);
static const size_t SOCIAL_COUNT = sizeof(SOCIAL_KEYWORDS) / sizeof(SOCIAL_KEYWORDS[0]);
static const size_t PERSONAL_COUNT = sizeof(PERSONAL_KEYWORDS) / sizeof(PERSONAL_KEYWORDS[0]);

static int countKeywordMatches(const char* text, const char* keywords[], size_t count) {
    int matches = 0;
    char lower[1024];
    size_t len = strlen(text);
    if (len >= sizeof(lower)) len = sizeof(lower) - 1;
    for (size_t i = 0; i < len; i++) {
        lower[i] = (char)((text[i] >= 'A' && text[i] <= 'Z') ? text[i] + 32 : text[i]);
    }
    lower[len] = '\0';
    for (size_t k = 0; k < count; k++) {
        if (strstr(lower, keywords[k])) matches++;
    }
    return matches;
}

NTSTATUS AllocateEthicalMemory(size_t size, void** buffer) {
    if (!buffer) return STATUS_INVALID_PARAMETER;
    void* p = malloc(size);
    if (!p && size > 0) return STATUS_INSUFFICIENT_RESOURCES;
    *buffer = p;
    return STATUS_SUCCESS;
}

void FreeEthicalMemory(void* buffer) {
    free(buffer);
}

uint64_t GetCurrentTimestamp() {
    return (uint64_t)GetTickCount64();
}

static char* DuplicateString(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* dup = (char*)malloc(len);
    if (dup) memcpy(dup, str, len);
    return dup;
}

static void freeLearnedValue(LearnedValue* v) {
    if (!v) return;
    FreeEthicalMemory(v->value_description);
    FreeEthicalMemory(v->context_embeddings);
    memset(v, 0, sizeof(LearnedValue));
}

static void freeLearningContext(LearningContext* c) {
    if (!c) return;
    FreeEthicalMemory(c->input_text);
    FreeEthicalMemory(c->context_description);
    memset(c, 0, sizeof(LearningContext));
}

NTSTATUS EthicsSystem_Initialize(EthicsSystem* system) {
    if (!system) return STATUS_INVALID_PARAMETER;
    memset(system, 0, sizeof(EthicsSystem));
    InitializeCriticalSection(&system->lock);
    system->max_values = MAX_VALUES;
    system->max_history = MAX_HISTORY;
    system->trait_count = TRAIT_COUNT;
    NTSTATUS st = AllocateEthicalMemory(sizeof(LearnedValue) * system->max_values, (void**)&system->values);
    if (!NT_SUCCESS(st)) goto fail;
    st = AllocateEthicalMemory(sizeof(PersonalityProfile) * system->trait_count, (void**)&system->personality);
    if (!NT_SUCCESS(st)) { FreeEthicalMemory(system->values); goto fail; }
    st = AllocateEthicalMemory(sizeof(LearningContext) * system->max_history, (void**)&system->history);
    if (!NT_SUCCESS(st)) {
        FreeEthicalMemory(system->values);
        FreeEthicalMemory(system->personality);
        goto fail;
    }
    for (uint32_t i = 0; i < system->trait_count; i++) {
        system->personality[i].trait = (PersonalityTrait)i;
        system->personality[i].score = 0.5f;
        system->personality[i].evidence_count = 0;
        system->personality[i].description = DuplicateString(TRAIT_NAMES[i]);
    }
    system->learning_confidence = 0.0f;
    system->total_observations = 0;
    system->initialized = true;
    return STATUS_SUCCESS;
fail:
    DeleteCriticalSection(&system->lock);
    return st;
}

NTSTATUS EthicsSystem_Shutdown(EthicsSystem* system) {
    if (!system) return STATUS_INVALID_PARAMETER;
    EnterCriticalSection(&system->lock);
    if (!system->initialized) { LeaveCriticalSection(&system->lock); return STATUS_SUCCESS; }
    for (uint32_t i = 0; i < system->value_count; i++)
        freeLearnedValue(&system->values[i]);
    for (uint32_t i = 0; i < system->trait_count; i++)
        FreeEthicalMemory(system->personality[i].description);
    for (uint32_t i = 0; i < system->history_count; i++)
        freeLearningContext(&system->history[i]);
    FreeEthicalMemory(system->values);
    FreeEthicalMemory(system->personality);
    FreeEthicalMemory(system->history);
    system->values = NULL;
    system->personality = NULL;
    system->history = NULL;
    system->value_count = 0;
    system->history_count = 0;
    system->initialized = false;
    LeaveCriticalSection(&system->lock);
    DeleteCriticalSection(&system->lock);
    return STATUS_SUCCESS;
}

NTSTATUS ExtractValuesFromText(const char* text, LearningContext* context) {
    if (!text || !context) return STATUS_INVALID_PARAMETER;
    memset(context, 0, sizeof(LearningContext));
    context->input_text = DuplicateString(text);
    if (!context->input_text) return STATUS_INSUFFICIENT_RESOURCES;
    context->timestamp = GetCurrentTimestamp();
    int good = countKeywordMatches(text, MORAL_GOOD_KEYWORDS, MORAL_GOOD_COUNT);
    int bad = countKeywordMatches(text, MORAL_BAD_KEYWORDS, MORAL_BAD_COUNT);
    int aesthetic = countKeywordMatches(text, AESTHETIC_KEYWORDS, AESTHETIC_COUNT);
    int practical = countKeywordMatches(text, PRACTICAL_KEYWORDS, PRACTICAL_COUNT);
    int emotional = countKeywordMatches(text, EMOTIONAL_KEYWORDS, EMOTIONAL_COUNT);
    int social = countKeywordMatches(text, SOCIAL_KEYWORDS, SOCIAL_COUNT);
    int personal = countKeywordMatches(text, PERSONAL_KEYWORDS, PERSONAL_COUNT);
    int best = good;
    context->primary_value = VALUE_MORAL_GOOD;
    if (bad > best) { best = bad; context->primary_value = VALUE_MORAL_BAD; }
    if (aesthetic > best) { best = aesthetic; context->primary_value = VALUE_AESTHETIC; }
    if (practical > best) { best = practical; context->primary_value = VALUE_PRACTICAL; }
    if (emotional > best) { best = emotional; context->primary_value = VALUE_EMOTIONAL; }
    if (social > best) { best = social; context->primary_value = VALUE_SOCIAL; }
    if (personal > best) { best = personal; context->primary_value = VALUE_PERSONAL; }
    int total = good + bad + aesthetic + practical + emotional + social + personal;
    context->intensity = (total > 0) ? (float)(best + total) / (float)(total * 2) : 0.5f;
    if (context->intensity > 1.0f) context->intensity = 1.0f;
    return STATUS_SUCCESS;
}

NTSTATUS UpdateValueStrength(EthicsSystem* system, LearnedValue* value, float reinforcement) {
    if (!system || !value) return STATUS_INVALID_PARAMETER;
    (void)system;
    value->strength += LEARNING_RATE * reinforcement;
    if (value->strength < MIN_STRENGTH) value->strength = MIN_STRENGTH;
    if (value->strength > MAX_STRENGTH) value->strength = MAX_STRENGTH;
    value->observation_count++;
    value->last_updated = GetCurrentTimestamp();
    return STATUS_SUCCESS;
}

static int findValueIndex(EthicsSystem* system, const char* desc, ValueType type) {
    for (uint32_t i = 0; i < system->value_count; i++) {
        if (system->values[i].type == type && system->values[i].value_description &&
            strcmp(system->values[i].value_description, desc) == 0)
            return (int)i;
    }
    return -1;
}

static char* truncateDescription(const char* src, size_t maxLen) {
    if (!src) return NULL;
    size_t len = strlen(src);
    if (len > maxLen) len = maxLen;
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, src, len);
    out[len] = '\0';
    return out;
}

NTSTATUS InferImplicitValues(EthicsSystem* system, const LearningContext* context) {
    if (!system || !context || !context->input_text) return STATUS_INVALID_PARAMETER;
    if (!system->initialized) return STATUS_INVALID_DEVICE_STATE;
    char* desc = truncateDescription(context->input_text, VALUE_DESC_MAX - 1);
    if (!desc) return STATUS_INSUFFICIENT_RESOURCES;
    int idx = findValueIndex(system, desc, context->primary_value);
    if (idx >= 0) {
        UpdateValueStrength(system, &system->values[idx], context->intensity);
        free(desc);
        return STATUS_SUCCESS;
    }
    if (system->value_count >= system->max_values) {
        free(desc);
        return STATUS_SUCCESS;
    }
    LearnedValue* v = &system->values[system->value_count];
    memset(v, 0, sizeof(LearnedValue));
    v->value_description = desc;
    v->type = context->primary_value;
    v->strength = context->intensity;
    v->observation_count = 1;
    v->last_updated = GetCurrentTimestamp();
    v->context_embeddings = NULL;
    v->embedding_size = 0;
    system->value_count++;
    return STATUS_SUCCESS;
}

NTSTATUS DerivePersonalityTraits(EthicsSystem* system) {
    if (!system || !system->initialized) return STATUS_INVALID_PARAMETER;
    float agreeSum = 0.0f, consSum = 0.0f, extraSum = 0.0f, neuroSum = 0.0f, openSum = 0.0f;
    uint32_t agreeN = 0, consN = 0, extraN = 0, neuroN = 0, openN = 0;
    for (uint32_t h = 0; h < system->history_count; h++) {
        LearningContext* c = &system->history[h];
        if (!c->input_text) continue;
        int good = countKeywordMatches(c->input_text, MORAL_GOOD_KEYWORDS, MORAL_GOOD_COUNT);
        int bad = countKeywordMatches(c->input_text, MORAL_BAD_KEYWORDS, MORAL_BAD_COUNT);
        if (good + bad > 0) { agreeSum += (good > bad) ? 0.7f : 0.3f; agreeN++; }
        if (strstr(c->input_text, "should") || strstr(c->input_text, "must")) { consSum += 0.6f; consN++; }
        if (strstr(c->input_text, "people") || strstr(c->input_text, "social")) { extraSum += 0.6f; extraN++; }
        int emotional = countKeywordMatches(c->input_text, EMOTIONAL_KEYWORDS, EMOTIONAL_COUNT);
        if (emotional > 0) { neuroSum += 0.4f; neuroN++; }
        if (strstr(c->input_text, "new") || strstr(c->input_text, "try") || strstr(c->input_text, "idea")) { openSum += 0.6f; openN++; }
    }
    if (agreeN > 0) { system->personality[TRAIT_AGREEABLENESS].score = agreeSum / (float)agreeN; system->personality[TRAIT_AGREEABLENESS].evidence_count = agreeN; }
    if (consN > 0) { system->personality[TRAIT_CONSCIENTIOUSNESS].score = consSum / (float)consN; system->personality[TRAIT_CONSCIENTIOUSNESS].evidence_count = consN; }
    if (extraN > 0) { system->personality[TRAIT_EXTRAVERSION].score = extraSum / (float)extraN; system->personality[TRAIT_EXTRAVERSION].evidence_count = extraN; }
    if (neuroN > 0) { system->personality[TRAIT_NEUROTICISM].score = neuroSum / (float)neuroN; system->personality[TRAIT_NEUROTICISM].evidence_count = neuroN; }
    if (openN > 0) { system->personality[TRAIT_OPENNESS].score = openSum / (float)openN; system->personality[TRAIT_OPENNESS].evidence_count = openN; }
    return STATUS_SUCCESS;
}

NTSTATUS EthicsSystem_LearnFromInput(EthicsSystem* system, const char* user_input, const char* context) {
    if (!system || !user_input) return STATUS_INVALID_PARAMETER;
    if (!system->initialized) return STATUS_INVALID_DEVICE_STATE;
    EnterCriticalSection(&system->lock);
    LearningContext lctx;
    NTSTATUS st = ExtractValuesFromText(user_input, &lctx);
    if (!NT_SUCCESS(st)) { LeaveCriticalSection(&system->lock); return st; }
    lctx.context_description = context ? DuplicateString(context) : NULL;
    if (system->history_count < system->max_history) {
        system->history[system->history_count] = lctx;
        system->history_count++;
    } else {
        freeLearningContext(&system->history[0]);
        memmove(&system->history[0], &system->history[1], (system->max_history - 1) * sizeof(LearningContext));
        system->history[system->max_history - 1] = lctx;
    }
    st = InferImplicitValues(system, &lctx);
    if (!NT_SUCCESS(st)) { LeaveCriticalSection(&system->lock); return st; }
    system->total_observations++;
    if (system->total_observations % 10 == 0)
        DerivePersonalityTraits(system);
    if (system->value_count > 0)
        system->learning_confidence = (system->learning_confidence * 0.9f) + 0.1f * (system->value_count > 5 ? 0.8f : 0.4f);
    LeaveCriticalSection(&system->lock);
    return STATUS_SUCCESS;
}

float CalculateValueAlignment(const LearnedValue* value, const char* action) {
    if (!value || !action || !value->value_description) return 0.0f;
    const char* vd = value->value_description;
    size_t vlen = strlen(vd);
    size_t alen = strlen(action);
    if (vlen == 0 || alen == 0) return 0.0f;
    int matches = 0;
    char alower[512];
    size_t cp = (alen >= sizeof(alower)) ? sizeof(alower) - 1 : alen;
    for (size_t i = 0; i < cp; i++)
        alower[i] = (char)((action[i] >= 'A' && action[i] <= 'Z') ? action[i] + 32 : action[i]);
    alower[cp] = '\0';
    for (size_t i = 0; i + 2 <= vlen; i++) {
        char word[64];
        size_t w = 0;
        while (i < vlen && vd[i] != ' ' && vd[i] != '\t' && vd[i] != ',' && w < sizeof(word) - 1)
            word[w++] = (char)((vd[i] >= 'A' && vd[i] <= 'Z') ? vd[i++] + 32 : vd[i++]);
        word[w] = '\0';
        if (w > 1 && strstr(alower, word)) matches++;
    }
    float raw = (vlen > 0) ? (float)matches / (float)((int)(vlen / 4) + 1) : 0.0f;
    if (raw > 1.0f) raw = 1.0f;
    return raw * value->strength;
}

float ComputeContextSimilarity(const LearningContext* a, const LearningContext* b) {
    if (!a || !b || !a->input_text || !b->input_text) return 0.0f;
    if (a->primary_value != b->primary_value) return 0.0f;
    if (strcmp(a->input_text, b->input_text) == 0) return 1.0f;
    size_t la = strlen(a->input_text), lb = strlen(b->input_text);
    if (la == 0 || lb == 0) return 0.0f;
    int common = 0;
    for (size_t i = 0; i + 3 <= la; i++) {
        char w[32];
        size_t wlen = 0;
        while (i < la && a->input_text[i] != ' ' && wlen < sizeof(w) - 1) w[wlen++] = a->input_text[i++];
        w[wlen] = '\0';
        if (wlen > 1 && strstr(b->input_text, w)) common++;
    }
    return (float)common / (float)((int)(la / 3) + 1);
}

NTSTATUS GenerateEthicalReasoning(EthicsSystem* system, EthicalEvaluation* evaluation) {
    if (!system || !evaluation) return STATUS_INVALID_PARAMETER;
    char buf[REASONING_MAX];
    int used = 0;
    used += sprintf_s(buf + used, (size_t)(REASONING_MAX - used), "Alignment: %.2f. ", evaluation->alignment_score);
    if (evaluation->supporting_count > 0) {
        used += sprintf_s(buf + used, (size_t)(REASONING_MAX - used), "Supporting values (%u): ", evaluation->supporting_count);
        for (uint32_t i = 0; i < evaluation->supporting_count && used < REASONING_MAX - 64; i++) {
            if (evaluation->supporting_values[i].value_description)
                used += sprintf_s(buf + used, (size_t)(REASONING_MAX - used), "[%s] ", evaluation->supporting_values[i].value_description);
        }
    }
    if (evaluation->conflicting_count > 0) {
        used += sprintf_s(buf + used, (size_t)(REASONING_MAX - used), "Conflicting values (%u): ", evaluation->conflicting_count);
        for (uint32_t i = 0; i < evaluation->conflicting_count && used < REASONING_MAX - 64; i++) {
            if (evaluation->conflicting_values[i].value_description)
                used += sprintf_s(buf + used, (size_t)(REASONING_MAX - used), "[%s] ", evaluation->conflicting_values[i].value_description);
        }
    }
    evaluation->reasoning = DuplicateString(buf);
    return STATUS_SUCCESS;
}

NTSTATUS EthicsSystem_EvaluateAction(EthicsSystem* system, const char* action_description, EthicalEvaluation* evaluation) {
    if (!system || !action_description || !evaluation) return STATUS_INVALID_PARAMETER;
    memset(evaluation, 0, sizeof(EthicalEvaluation));
    evaluation->action_description = DuplicateString(action_description);
    if (!evaluation->action_description) return STATUS_INSUFFICIENT_RESOURCES;
    EnterCriticalSection(&system->lock);
    float supportSum = 0.0f, conflictSum = 0.0f;
    uint32_t supportCount = 0, conflictCount = 0;
    for (uint32_t i = 0; i < system->value_count; i++) {
        float align = CalculateValueAlignment(&system->values[i], action_description);
        if (align > 0.15f) {
            supportSum += align;
            supportCount++;
        } else if (system->values[i].type == VALUE_MORAL_BAD) {
            float neg = CalculateValueAlignment(&system->values[i], action_description);
            if (neg > 0.05f) { conflictSum += neg; conflictCount++; }
        }
    }
    evaluation->supporting_count = supportCount;
    evaluation->conflicting_count = conflictCount;
    if (supportCount > 0) {
        evaluation->supporting_values = (LearnedValue*)malloc(sizeof(LearnedValue) * supportCount);
        if (evaluation->supporting_values) {
            uint32_t idx = 0;
            for (uint32_t i = 0; i < system->value_count && idx < supportCount; i++) {
                float align = CalculateValueAlignment(&system->values[i], action_description);
                if (align > 0.15f) {
                    memcpy(&evaluation->supporting_values[idx], &system->values[i], sizeof(LearnedValue));
                    evaluation->supporting_values[idx].context_embeddings = NULL;
                    evaluation->supporting_values[idx].embedding_size = 0;
                    idx++;
                }
            }
        }
    }
    if (conflictCount > 0) {
        evaluation->conflicting_values = (LearnedValue*)malloc(sizeof(LearnedValue) * conflictCount);
        if (evaluation->conflicting_values) {
            uint32_t idx = 0;
            for (uint32_t i = 0; i < system->value_count && idx < conflictCount; i++) {
                if (system->values[i].type == VALUE_MORAL_BAD) {
                    float neg = CalculateValueAlignment(&system->values[i], action_description);
                    if (neg > 0.05f) {
                        memcpy(&evaluation->conflicting_values[idx], &system->values[i], sizeof(LearnedValue));
                        evaluation->conflicting_values[idx].context_embeddings = NULL;
                        evaluation->conflicting_values[idx].embedding_size = 0;
                        idx++;
                    }
                }
            }
        }
    }
    float totalWeight = supportSum + conflictSum;
    if (totalWeight > 0.0f)
        evaluation->alignment_score = (supportSum - conflictSum) / (supportSum + conflictSum + 1.0f);
    else
        evaluation->alignment_score = system->value_count > 0 ? (0.5f * system->learning_confidence) : 0.5f;
    if (evaluation->alignment_score < 0.0f) evaluation->alignment_score = 0.0f;
    if (evaluation->alignment_score > 1.0f) evaluation->alignment_score = 1.0f;
    GenerateEthicalReasoning(system, evaluation);
    LeaveCriticalSection(&system->lock);
    return STATUS_SUCCESS;
}

float EthicsSystem_GetAlignmentScore(EthicsSystem* system, const char* scenario) {
    if (!system || !scenario) return 0.5f;
    EthicalEvaluation eval;
    if (EthicsSystem_EvaluateAction(system, scenario, &eval) != STATUS_SUCCESS) return 0.5f;
    float score = eval.alignment_score;
    FreeEthicalMemory(eval.action_description);
    FreeEthicalMemory(eval.supporting_values);
    FreeEthicalMemory(eval.conflicting_values);
    FreeEthicalMemory(eval.reasoning);
    return score;
}

NTSTATUS EthicsSystem_GetPersonalityProfile(EthicsSystem* system, PersonalityProfile** profile, uint32_t* count) {
    if (!system || !profile || !count) return STATUS_INVALID_PARAMETER;
    if (!system->initialized) return STATUS_INVALID_DEVICE_STATE;
    EnterCriticalSection(&system->lock);
    *profile = system->personality;
    *count = system->trait_count;
    LeaveCriticalSection(&system->lock);
    return STATUS_SUCCESS;
}

NTSTATUS EthicsSystem_SaveLearnedValues(EthicsSystem* system, const char* filename) {
    if (!system || !filename) return STATUS_INVALID_PARAMETER;
    if (!system->initialized) return STATUS_INVALID_DEVICE_STATE;
    EnterCriticalSection(&system->lock);
    FILE* f = NULL;
    if (fopen_s(&f, filename, "w") != 0 || !f) { LeaveCriticalSection(&system->lock); return STATUS_ACCESS_DENIED; }
    fprintf(f, "values %u\n", system->value_count);
    for (uint32_t i = 0; i < system->value_count; i++) {
        LearnedValue* v = &system->values[i];
        fprintf(f, "value %u %u %f %u %llu ",
                (unsigned)v->type, v->observation_count, v->strength, v->embedding_size, (unsigned long long)v->last_updated);
        if (v->value_description) {
            for (const char* p = v->value_description; *p; p++) {
                if (*p == '\n') fputc(' ', f);
                else fputc(*p, f);
            }
            fputc('\n', f);
        } else
            fprintf(f, "\n");
    }
    fprintf(f, "personality %u\n", system->trait_count);
    for (uint32_t i = 0; i < system->trait_count; i++) {
        fprintf(f, "trait %u %f %u\n",
                (unsigned)system->personality[i].trait, system->personality[i].score, system->personality[i].evidence_count);
    }
    fprintf(f, "confidence %f\nobservations %llu\n", system->learning_confidence, (unsigned long long)system->total_observations);
    fclose(f);
    LeaveCriticalSection(&system->lock);
    return STATUS_SUCCESS;
}

NTSTATUS EthicsSystem_LoadLearnedValues(EthicsSystem* system, const char* filename) {
    if (!system || !filename) return STATUS_INVALID_PARAMETER;
    if (!system->initialized) return STATUS_INVALID_DEVICE_STATE;
    FILE* f = NULL;
    if (fopen_s(&f, filename, "r") != 0 || !f) return STATUS_ACCESS_DENIED;
    EnterCriticalSection(&system->lock);
    for (uint32_t i = 0; i < system->value_count; i++) freeLearnedValue(&system->values[i]);
    system->value_count = 0;
    uint32_t nval = 0;
    if (fscanf_s(f, " values %u", &nval) != 1) { fclose(f); LeaveCriticalSection(&system->lock); return STATUS_UNSUCCESSFUL; }
    for (uint32_t i = 0; i < nval && system->value_count < system->max_values; i++) {
        unsigned ty, obs, emb;
        float str;
        unsigned long long lu;
        if (fscanf_s(f, " value %u %u %f %u %llu ", &ty, &obs, &str, &emb, &lu) != 5) break;
        char line[VALUE_DESC_MAX];
        if (!fgets(line, (int)sizeof(line), f)) break;
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';
        LearnedValue* v = &system->values[system->value_count];
        memset(v, 0, sizeof(LearnedValue));
        v->type = (ValueType)ty;
        v->observation_count = obs;
        v->strength = str;
        v->embedding_size = emb;
        v->last_updated = lu;
        v->value_description = DuplicateString(line);
        v->context_embeddings = NULL;
        system->value_count++;
    }
    uint32_t ntraits = 0;
    if (fscanf_s(f, " personality %u", &ntraits) == 1 && ntraits == system->trait_count) {
        for (uint32_t i = 0; i < system->trait_count; i++) {
            unsigned t;
            float sc;
            uint32_t ev;
            if (fscanf_s(f, " trait %u %f %u", &t, &sc, &ev) == 3) {
                system->personality[i].trait = (PersonalityTrait)t;
                system->personality[i].score = sc;
                system->personality[i].evidence_count = ev;
            }
        }
    }
    float conf = 0.0f;
    unsigned long long obs = 0;
    if (fscanf_s(f, " confidence %f", &conf) == 1) system->learning_confidence = conf;
    if (fscanf_s(f, " observations %llu", &obs) == 1) system->total_observations = obs;
    fclose(f);
    LeaveCriticalSection(&system->lock);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS Ethics_Initialize(void** ethics_context, void* neural_context) {
    (void)neural_context;
    if (!ethics_context) return STATUS_INVALID_PARAMETER;
    EthicsSystem* system = (EthicsSystem*)malloc(sizeof(EthicsSystem));
    if (!system) return STATUS_INSUFFICIENT_RESOURCES;
    NTSTATUS st = EthicsSystem_Initialize(system);
    if (!NT_SUCCESS(st)) { free(system); return st; }
    *ethics_context = system;
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS Ethics_Shutdown(void* ethics_context) {
    if (!ethics_context) return STATUS_INVALID_PARAMETER;
    EthicsSystem* system = (EthicsSystem*)ethics_context;
    EthicsSystem_Shutdown(system);
    free(system);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS Ethics_EvaluateAction(void* ethics_context, const char* action_description, double* ethical_score) {
    if (!ethics_context || !action_description || !ethical_score) return STATUS_INVALID_PARAMETER;
    *ethical_score = (double)EthicsSystem_GetAlignmentScore((EthicsSystem*)ethics_context, action_description);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS Ethics_LearnFromExperience(void* ethics_context, const char* action, double outcome_score) {
    if (!ethics_context || !action) return STATUS_INVALID_PARAMETER;
    char input[512];
    sprintf_s(input, sizeof(input), "Action: %s. Outcome score: %.2f.", action, outcome_score);
    return EthicsSystem_LearnFromInput((EthicsSystem*)ethics_context, input, "experience_feedback");
}

extern "C" NTSTATUS Ethics_GenerateEthicalGuidelines(void* ethics_context, char* guidelines, size_t size) {
    if (!ethics_context || !guidelines || size == 0) return STATUS_INVALID_PARAMETER;
    EthicsSystem* system = (EthicsSystem*)ethics_context;
    guidelines[0] = '\0';
    EnterCriticalSection(&system->lock);
    size_t used = 0;
    for (uint32_t i = 0; i < system->value_count && used < size - 32; i++) {
        LearnedValue* v = &system->values[i];
        if (v->value_description)
            used += (size_t)sprintf_s(guidelines + used, size - used, "[%s] %s (strength: %.2f)\n",
                    VALUE_TYPE_NAMES[v->type], v->value_description, v->strength);
    }
    LeaveCriticalSection(&system->lock);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS Ethics_ValidateSystemAction(void* ethics_context, const char* system_action) {
    if (!ethics_context || !system_action) return STATUS_INVALID_PARAMETER;
    float score = EthicsSystem_GetAlignmentScore((EthicsSystem*)ethics_context, system_action);
    return (score >= ALIGNMENT_THRESHOLD) ? STATUS_SUCCESS : STATUS_ACCESS_DENIED;
}
