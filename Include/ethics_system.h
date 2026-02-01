#ifndef ETHICS_SYSTEM_H
#define ETHICS_SYSTEM_H

#include <windows.h>
#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stdbool.h>

// Raijin Ethics Learning System
// Learns exclusively from user inputs - no external ethical frameworks

// Value types that can be learned
typedef enum {
    VALUE_MORAL_GOOD = 0,      // What the user considers morally good
    VALUE_MORAL_BAD = 1,       // What the user considers morally bad
    VALUE_AESTHETIC = 2,       // Aesthetic preferences
    VALUE_PRACTICAL = 3,       // Practical considerations
    VALUE_EMOTIONAL = 4,       // Emotional responses
    VALUE_SOCIAL = 5,          // Social norms and expectations
    VALUE_PERSONAL = 6,        // Personal preferences and habits
    VALUE_COUNT
} ValueType;

// Personality traits derived from user
typedef enum {
    TRAIT_AGREEABLENESS = 0,   // How agreeable/compassionate
    TRAIT_CONSCIENTIOUSNESS = 1, // How organized/disciplined
    TRAIT_EXTRAVERSION = 2,    // How outgoing/social
    TRAIT_NEUROTICISM = 3,     // How emotionally stable
    TRAIT_OPENNESS = 4,        // How open to new experiences
    TRAIT_COUNT
} PersonalityTrait;

// Learning context
typedef struct {
    char* input_text;          // The user's input text
    char* context_description; // Description of the situation
    ValueType primary_value;   // Primary value being expressed
    float intensity;           // How strongly this value is expressed (0-1)
    uint64_t timestamp;        // When this learning occurred
} LearningContext;

// Learned value representation
typedef struct {
    char* value_description;   // Text description of the value
    ValueType type;            // Type of value
    float strength;            // How strongly held (0-1)
    uint32_t observation_count; // How many times observed
    uint64_t last_updated;     // Last time this value was reinforced
    float* context_embeddings; // Hyper-dimensional context representation
    uint32_t embedding_size;
} LearnedValue;

// Personality profile
typedef struct {
    PersonalityTrait trait;
    float score;               // Trait score (0-1)
    uint32_t evidence_count;   // Supporting observations
    char* description;         // Text description of trait manifestation
} PersonalityProfile;

// Decision evaluation
typedef struct {
    char* action_description;  // Description of potential action
    float alignment_score;     // How well it aligns with learned values
    LearnedValue* supporting_values; // Values that support this action
    uint32_t supporting_count;
    LearnedValue* conflicting_values; // Values that conflict
    uint32_t conflicting_count;
    char* reasoning;           // Explanation of the evaluation
} EthicalEvaluation;

// Ethics learning system
typedef struct EthicsSystem {
    LearnedValue* values;      // Array of learned values
    uint32_t value_count;
    uint32_t max_values;

    PersonalityProfile* personality; // Personality traits
    uint32_t trait_count;

    LearningContext* history;  // Learning history
    uint32_t history_count;
    uint32_t max_history;

    float learning_confidence; // Overall confidence in learned ethics
    uint64_t total_observations; // Total learning observations

    // Internal state
    CRITICAL_SECTION lock;
    bool initialized;
} EthicsSystem;

// C API (opaque context)
#ifdef __cplusplus
extern "C" {
#endif
NTSTATUS Ethics_Initialize(void** ethics_context, void* neural_context);
NTSTATUS Ethics_Shutdown(void* ethics_context);
#ifdef __cplusplus
}
#endif

// Core API functions
NTSTATUS EthicsSystem_Initialize(EthicsSystem* system);
NTSTATUS EthicsSystem_Shutdown(EthicsSystem* system);
NTSTATUS EthicsSystem_LearnFromInput(EthicsSystem* system, const char* user_input, const char* context);
NTSTATUS EthicsSystem_EvaluateAction(EthicsSystem* system, const char* action_description, EthicalEvaluation* evaluation);
float EthicsSystem_GetAlignmentScore(EthicsSystem* system, const char* scenario);
NTSTATUS EthicsSystem_GetPersonalityProfile(EthicsSystem* system, PersonalityProfile** profile, uint32_t* count);
NTSTATUS EthicsSystem_SaveLearnedValues(EthicsSystem* system, const char* filename);
NTSTATUS EthicsSystem_LoadLearnedValues(EthicsSystem* system, const char* filename);

// Learning functions
NTSTATUS ExtractValuesFromText(const char* text, LearningContext* context);
NTSTATUS InferImplicitValues(EthicsSystem* system, const LearningContext* context);
NTSTATUS UpdateValueStrength(EthicsSystem* system, LearnedValue* value, float reinforcement);
NTSTATUS DerivePersonalityTraits(EthicsSystem* system);

// Evaluation functions
float CalculateValueAlignment(const LearnedValue* value, const char* action);
float ComputeContextSimilarity(const LearningContext* a, const LearningContext* b);
NTSTATUS GenerateEthicalReasoning(EthicsSystem* system, EthicalEvaluation* evaluation);

// Utility functions
NTSTATUS AllocateEthicalMemory(size_t size, void** buffer);
void FreeEthicalMemory(void* buffer);
uint64_t GetCurrentTimestamp();

#endif // ETHICS_SYSTEM_H
