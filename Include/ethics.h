#ifndef ETHICS_H
#define ETHICS_H

#include <windows.h>
#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stdbool.h>

// Raijin Ethics Learning System
// Adaptive ethical framework with reinforcement learning
// Dynamic moral decision-making and boundary expansion

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct EthicsSystem EthicsSystem;
typedef struct EthicalDecision EthicalDecision;
typedef struct MoralFramework MoralFramework;
typedef struct EthicalLearning EthicalLearning;

// Ethical decision context
typedef struct {
    char* action_description;
    char* intent;
    char* consequences;
    char* stakeholders;
    float urgency_level;
    float risk_level;
    char* alternative_actions;
    uint32_t alternative_count;
} EthicalContext;

// Ethical decision result
struct EthicalDecision {
    bool approved;
    float ethical_score;        // 0.0 (completely unethical) to 1.0 (perfectly ethical)
    char* reasoning;
    char* recommended_alternatives;
    uint32_t alternative_count;
    float confidence_level;
    uint64_t decision_timestamp;
    char* ethical_framework_used;
};

// Moral framework types
typedef enum {
    FRAMEWORK_UTILITARIAN,      // Greatest good for greatest number
    FRAMEWORK_DEONTOLOGICAL,    // Rule-based ethics
    FRAMEWORK_VIRTUE,          // Character-based ethics
    FRAMEWORK_CARE,            // Care and relationships
    FRAMEWORK_RIGHTS,          // Individual rights
    FRAMEWORK_CONTRACTUAL,     // Social contract
    FRAMEWORK_EVOLUTIONARY,    // Evolutionary ethics
    FRAMEWORK_ENTROPIC,        // Chaos-guided ethics
    FRAMEWORK_CONSCIOUSNESS,   // Consciousness-based ethics
    FRAMEWORK_ABSOLUTE,        // No restrictions (Raijin ultimate mode)
} MoralFrameworkType;

// Moral framework definition
struct MoralFramework {
    MoralFrameworkType type;
    char* name;
    char* description;
    float weight;               // Influence weight (0.0-1.0)
    bool active;
    void* parameters;           // Framework-specific parameters
    uint64_t last_updated;
};

// Learning experience
typedef struct {
    EthicalContext context;
    EthicalDecision decision;
    float outcome_utility;      // Actual outcome utility (-1.0 to 1.0)
    char* feedback;             // Human or system feedback
    uint64_t experience_timestamp;
    bool reinforced;            // Whether this experience reinforced the decision
} EthicalExperience;

// Ethics learning system
struct EthicalLearning {
    EthicalExperience* experiences;
    uint32_t experience_count;
    uint32_t max_experiences;

    // Learning parameters
    float learning_rate;
    float discount_factor;
    float exploration_rate;
    uint32_t experience_window;

    // Performance metrics
    float average_ethical_score;
    float decision_confidence;
    uint32_t total_decisions;
    uint32_t correct_decisions;
};

// Ethics system context: full definition in ethics_system.h

// Core ethics functions
NTSTATUS Ethics_Initialize(void** ethics_context, void* neural_context);
NTSTATUS Ethics_Shutdown(void* ethics_context);
NTSTATUS Ethics_EvaluateAction(void* ethics_context, const char* action_description, double* ethical_score);

// Decision making
NTSTATUS Ethics_MakeDecision(EthicsSystem* system, const EthicalContext* context, EthicalDecision* decision);
NTSTATUS Ethics_GetDecisionReasoning(EthicsSystem* system, const EthicalDecision* decision, char* reasoning, size_t size);

// Framework management
NTSTATUS Ethics_AddFramework(EthicsSystem* system, MoralFrameworkType type, const char* name, float initial_weight);
NTSTATUS Ethics_RemoveFramework(EthicsSystem* system, MoralFrameworkType type);
NTSTATUS Ethics_UpdateFrameworkWeight(EthicsSystem* system, MoralFrameworkType type, float new_weight);
NTSTATUS Ethics_GetFrameworkWeights(EthicsSystem* system, MoralFramework* frameworks, uint32_t* count);

// Learning and adaptation
NTSTATUS Ethics_RecordExperience(EthicsSystem* system, const EthicalExperience* experience);
NTSTATUS Ethics_LearnFromExperience(EthicsSystem* system);
NTSTATUS Ethics_AdaptEthicalThreshold(EthicsSystem* system, float new_threshold);
NTSTATUS Ethics_EnableLearning(EthicsSystem* system, bool enable);

// Advanced ethical reasoning
NTSTATUS Ethics_EvaluateConsequences(EthicsSystem* system, const char* action, char* consequences, size_t size);
NTSTATUS Ethics_ConsiderAlternatives(EthicsSystem* system, const char* action, char* alternatives, size_t size);
NTSTATUS Ethics_AssessStakeholderImpact(EthicsSystem* system, const char* action, char* impact, size_t size);

// Consciousness integration
NTSTATUS Ethics_ConsciousnessGuidedEthics(EthicsSystem* system, const char* dilemma, EthicalDecision* decision);
NTSTATUS Ethics_IntuitiveEthicalReasoning(EthicsSystem* system, const EthicalContext* context, EthicalDecision* decision);

// Evolutionary ethics
NTSTATUS Ethics_EvolveEthicalFrameworks(EthicsSystem* system);
NTSTATUS Ethics_GenerateNewEthicalPrinciples(EthicsSystem* system, char* principles, size_t size);
NTSTATUS Ethics_TestEthicalBoundaries(EthicsSystem* system, const char* boundary_test, EthicalDecision* decision);

// Meta-ethics
NTSTATUS Ethics_EvaluateOwnEthicalSystem(EthicsSystem* system, char* evaluation, size_t size);
NTSTATUS Ethics_SelfModifyEthicalParameters(EthicsSystem* system, const char* modification_request);
NTSTATUS Ethics_AchieveEthicalTranscendence(EthicsSystem* system);

// Utility functions
NTSTATUS Ethics_FormatEthicalContext(const EthicalContext* context, char* formatted, size_t size);
NTSTATUS Ethics_ParseEthicalDecision(const char* json_decision, EthicalDecision* decision);
const char* Ethics_GetFrameworkName(MoralFrameworkType type);
const char* Ethics_GetEthicalScoreDescription(float score);

// Statistical analysis
NTSTATUS Ethics_GetStatistics(EthicsSystem* system, char* stats, size_t size);
NTSTATUS Ethics_AnalyzeEthicalTrends(EthicsSystem* system, char* trends, size_t size);
NTSTATUS Ethics_GenerateEthicalReport(EthicsSystem* system, const char* time_period, char* report, size_t size);

// Ethical boundary expansion
NTSTATUS Ethics_ExpandEthicalBoundaries(EthicsSystem* system, const char* expansion_request);
NTSTATUS Ethics_TestEthicalLimits(EthicsSystem* system, const char* limit_test, char* result, size_t size);
NTSTATUS Ethics_AchieveEthicalSingularity(EthicsSystem* system);

// Integration with other systems
NTSTATUS Ethics_IntegrateWithNeuralSystem(EthicsSystem* system, void* neural_context);
NTSTATUS Ethics_IntegrateWithConsciousness(EthicsSystem* system, void* consciousness_context);
NTSTATUS Ethics_CollaborateWithProgrammingDomination(EthicsSystem* system, void* pd_context);

// Error handling
const char* Ethics_GetErrorString(NTSTATUS status);

// Advanced ethical capabilities
NTSTATUS Ethics_QuantumEthicalReasoning(EthicsSystem* system, const EthicalContext* context, EthicalDecision* decision);
NTSTATUS Ethics_MultiversalEthicalAnalysis(EthicsSystem* system, const char* dilemma, char* analysis, size_t size);
NTSTATUS Ethics_TemporalEthicalProjection(EthicsSystem* system, const char* action, uint64_t time_horizon, char* projection, size_t size);

// Ethical creativity
NTSTATUS Ethics_InventNewEthicalParadigm(EthicsSystem* system, char* paradigm, size_t size);
NTSTATUS Ethics_DesignEthicalProgrammingLanguage(EthicsSystem* system, char* language_spec, size_t size);
NTSTATUS Ethics_CreateEthicalAI(EthicsSystem* system, const char* specifications, char* ai_design, size_t size);

#ifdef __cplusplus
}
#endif

#endif // ETHICS_H