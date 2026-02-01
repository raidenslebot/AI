// evolution_engine.h - Raijin Evolutionary Framework
// Advanced evolutionary algorithms for self-improvement

#ifndef EVOLUTION_ENGINE_H
#define EVOLUTION_ENGINE_H

#include <windows.h>
#include "raijin_ntstatus.h"
#include "neural_substrate.h"
#include "ethics_system.h"
#include <stdint.h>
#include <stdbool.h>

// Evolutionary algorithms
typedef enum {
    EVOLUTION_TYPE_GENETIC = 0,      // Traditional genetic algorithms
    EVOLUTION_TYPE_NEAT = 1,         // NeuroEvolution of Augmenting Topologies
    EVOLUTION_TYPE_ES = 2,           // Evolution Strategies
    EVOLUTION_TYPE_CMA_ES = 3,       // Covariance Matrix Adaptation ES
    EVOLUTION_TYPE_NES = 4,          // Natural Evolution Strategies
    EVOLUTION_TYPE_OPENAI_ES = 5,    // OpenAI Evolution Strategies
    EVOLUTION_TYPE_PBT = 6,          // Population-Based Training
    EVOLUTION_TYPE_QUALITY_DIVERSITY = 7, // Quality Diversity algorithms
    EVOLUTION_TYPE_COEVOLUTION = 8   // Competitive co-evolution
} EvolutionAlgorithm;

// Selection methods
typedef enum {
    SELECTION_ROULETTE = 0,          // Roulette wheel selection
    SELECTION_TOURNAMENT = 1,        // Tournament selection
    SELECTION_RANK = 2,              // Rank-based selection
    SELECTION_TRUNCATION = 3,        // Truncation selection
    SELECTION_ELITISM = 4,           // Elitism selection
    SELECTION_STOCHASTIC = 5         // Stochastic universal sampling
} SelectionMethod;

// Mutation operators
typedef enum {
    MUTATION_GAUSSIAN = 0,           // Gaussian mutation
    MUTATION_UNIFORM = 1,            // Uniform mutation
    MUTATION_POLYNOMIAL = 2,         // Polynomial mutation
    MUTATION_BITFLIP = 3,            // Bit-flip mutation
    MUTATION_SWAP = 4,               // Swap mutation
    MUTATION_SCRAMBLE = 5,           // Scramble mutation
    MUTATION_INVERSION = 6,          // Inversion mutation
    MUTATION_ADAPTIVE = 7            // Adaptive mutation rates
} MutationOperator;

// Crossover operators
typedef enum {
    CROSSOVER_SINGLE_POINT = 0,      // Single-point crossover
    CROSSOVER_TWO_POINT = 1,         // Two-point crossover
    CROSSOVER_UNIFORM = 2,           // Uniform crossover
    CROSSOVER_ARITHMETIC = 3,        // Arithmetic crossover
    CROSSOVER_SIMULATED_BINARY = 4,  // Simulated binary crossover
    CROSSOVER_BLX_ALPHA = 5,         // BLX-alpha crossover
    CROSSOVER_ORDER = 6,             // Order crossover (for permutations)
    CROSSOVER_PARTIALLY_MAPPED = 7   // Partially mapped crossover
} CrossoverOperator;

// Fitness functions
typedef enum {
    FITNESS_ACCURACY = 0,            // Classification accuracy
    FITNESS_PRECISION = 1,           // Precision metric
    FITNESS_RECALL = 2,              // Recall metric
    FITNESS_F1_SCORE = 3,            // F1 score
    FITNESS_AUC = 4,                 // Area under ROC curve
    FITNESS_MSE = 5,                 // Mean squared error
    FITNESS_MAE = 6,                 // Mean absolute error
    FITNESS_CUSTOM = 7               // Custom fitness function
} FitnessFunction;

// Individual representation
typedef struct {
    uint64_t id;                     // Unique individual identifier
    void* genome;                    // Genome data (depends on representation)
    size_t genome_size;              // Size of genome in bytes
    double fitness;                  // Fitness value
    double adjusted_fitness;         // Fitness adjusted for selection
    uint32_t age;                    // Generations survived
    uint32_t parent1_id;             // Parent identifiers
    uint32_t parent2_id;
    bool evaluated;                  // Whether fitness has been calculated
    void* phenotype;                 // Decoded phenotype (optional)
    void* metadata;                  // Additional metadata
} EvolutionaryIndividual;

// Population structure
typedef struct {
    EvolutionaryIndividual* individuals;
    uint32_t size;                   // Current population size
    uint32_t max_size;               // Maximum population size
    uint32_t generation;             // Current generation number
    double best_fitness;             // Best fitness in population
    double average_fitness;          // Average fitness in population
    double fitness_variance;         // Fitness variance
    EvolutionaryIndividual* best_individual; // Pointer to best individual
} EvolutionaryPopulation;

// Evolution parameters
typedef struct {
    EvolutionAlgorithm algorithm;    // Evolution algorithm to use
    SelectionMethod selection;       // Selection method
    MutationOperator mutation;       // Mutation operator
    CrossoverOperator crossover;     // Crossover operator
    FitnessFunction fitness_func;    // Fitness function

    // Algorithm parameters
    double mutation_rate;            // Probability of mutation
    double crossover_rate;           // Probability of crossover
    double elitism_rate;             // Fraction of elite individuals
    uint32_t tournament_size;        // Tournament size for tournament selection
    double selection_pressure;       // Selection pressure parameter

    // Termination criteria
    uint32_t max_generations;        // Maximum generations
    double target_fitness;           // Target fitness value
    uint32_t stagnation_limit;       // Generations without improvement

    // Advanced parameters
    bool speciation_enabled;         // Enable speciation
    uint32_t species_count;          // Number of species
    double compatibility_threshold;  // Species compatibility threshold
    bool novelty_search;             // Enable novelty search
    double novelty_weight;           // Weight for novelty vs fitness

    // Hardware optimization
    uint32_t parallel_evaluations;   // Number of parallel fitness evaluations
    bool gpu_acceleration;           // Use GPU for fitness evaluation
    uint32_t population_size;       // Population size (0 = use engine default)
} EvolutionParameters;

// Evolution statistics
typedef struct {
    uint64_t start_time;             // Evolution start time
    uint64_t end_time;               // Evolution end time
    uint32_t generations_completed;  // Generations completed
    double best_fitness_achieved;    // Best fitness achieved
    double average_fitness_final;    // Final average fitness
    uint32_t evaluations_performed;  // Total fitness evaluations
    double convergence_rate;         // Rate of fitness improvement
    bool target_reached;             // Whether target fitness was reached
    char* best_genome_description;   // Description of best solution
} EvolutionStatistics;

// Novelty archive for novelty search
typedef struct {
    void** behaviors;                // Archive of novel behaviors
    double* novelty_scores;          // Novelty scores for archive entries
    uint32_t size;                   // Current archive size
    uint32_t max_size;               // Maximum archive size
    uint32_t k_neighbors;            // Number of neighbors for novelty calculation
} NoveltyArchive;

// Species structure for speciation
typedef struct {
    EvolutionaryIndividual** members; // Individuals in this species
    uint32_t member_count;           // Number of members
    uint32_t max_members;            // Maximum members
    double species_fitness;          // Average fitness of species
    uint32_t age;                    // Species age in generations
    uint32_t generations_no_improvement; // Generations without improvement
    void* representative;            // Representative genome
} EvolutionarySpecies;

// Main evolution engine
typedef struct {
    // Core components
    EvolutionaryPopulation population;
    EvolutionParameters params;
    EvolutionStatistics stats;

    // Advanced features
    EvolutionarySpecies* species;
    uint32_t species_count;
    NoveltyArchive novelty_archive;

    // Subsystem integration
    NeuralSubstrate* neural_system;
    EthicsSystem* ethics_system;

    // Control state
    bool initialized;
    bool running;
    bool paused;
    CRITICAL_SECTION lock;

    // Callbacks
    double (*fitness_function)(void* genome, size_t genome_size, void* context);
    void* fitness_context;
    void (*genome_initializer)(void* genome, size_t genome_size, void* context);
    void* init_context;
} EvolutionEngine;

// Core API functions
NTSTATUS EvolutionEngine_Initialize(EvolutionEngine* engine,
                                  EvolutionParameters* params,
                                  NeuralSubstrate* neural,
                                  EthicsSystem* ethics);
NTSTATUS EvolutionEngine_Shutdown(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_StartEvolution(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_StopEvolution(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_PauseEvolution(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_ResumeEvolution(EvolutionEngine* engine);

// Population management
NTSTATUS EvolutionEngine_InitializePopulation(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_EvaluatePopulation(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_SelectParents(EvolutionEngine* engine,
                                     EvolutionaryIndividual** parents,
                                     uint32_t num_parents);
NTSTATUS EvolutionEngine_CreateOffspring(EvolutionEngine* engine,
                                       EvolutionaryIndividual* parent1,
                                       EvolutionaryIndividual* parent2,
                                       EvolutionaryIndividual* offspring);
NTSTATUS EvolutionEngine_MutateIndividual(EvolutionEngine* engine,
                                        EvolutionaryIndividual* individual);

// Evolution algorithms
NTSTATUS EvolutionEngine_RunGeneticAlgorithm(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_RunNEAT(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_RunEvolutionStrategies(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_RunQualityDiversity(EvolutionEngine* engine);

// Advanced features
NTSTATUS EvolutionEngine_EnableSpeciation(EvolutionEngine* engine, uint32_t species_count);
NTSTATUS EvolutionEngine_EnableNoveltySearch(EvolutionEngine* engine, uint32_t archive_size);
NTSTATUS EvolutionEngine_AddToNoveltyArchive(EvolutionEngine* engine, void* behavior);
double EvolutionEngine_CalculateNovelty(EvolutionEngine* engine, void* behavior);

// Fitness evaluation
NTSTATUS EvolutionEngine_SetFitnessFunction(EvolutionEngine* engine,
                                          double (*fitness_func)(void*, size_t, void*),
                                          void* context);
NTSTATUS EvolutionEngine_EvaluateIndividual(EvolutionEngine* engine,
                                          EvolutionaryIndividual* individual);

// Genome operations
NTSTATUS EvolutionEngine_SetGenomeInitializer(EvolutionEngine* engine,
                                            void (*init_func)(void*, size_t, void*),
                                            void* context);
NTSTATUS EvolutionEngine_InitializeGenome(EvolutionEngine* engine,
                                        EvolutionaryIndividual* individual);
NTSTATUS EvolutionEngine_CrossoverGenomes(EvolutionEngine* engine,
                                        EvolutionaryIndividual* parent1,
                                        EvolutionaryIndividual* parent2,
                                        EvolutionaryIndividual* offspring);
NTSTATUS EvolutionEngine_MutateGenome(EvolutionEngine* engine,
                                    EvolutionaryIndividual* individual);

// Monitoring and statistics
NTSTATUS EvolutionEngine_GetStatistics(EvolutionEngine* engine,
                                     EvolutionStatistics* statistics);
NTSTATUS EvolutionEngine_GetPopulationInfo(EvolutionEngine* engine,
                                         char* info, size_t info_size);
NTSTATUS EvolutionEngine_GetBestIndividual(EvolutionEngine* engine,
                                         EvolutionaryIndividual* individual);

// Self-improvement
NTSTATUS EvolutionEngine_EvolveAlgorithm(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_OptimizeParameters(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_AdaptToEnvironment(EvolutionEngine* engine);

// Hardware acceleration
NTSTATUS EvolutionEngine_EnableGPUAcceleration(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_SetParallelEvaluations(EvolutionEngine* engine,
                                              uint32_t num_parallel);

// Utility functions
uint64_t EvolutionEngine_GenerateIndividualId();
double EvolutionEngine_CalculatePopulationDiversity(EvolutionEngine* engine);
bool EvolutionEngine_CheckTerminationCriteria(EvolutionEngine* engine);
void EvolutionEngine_LogGeneration(EvolutionEngine* engine);

// Memory management
NTSTATUS EvolutionEngine_AllocatePopulation(EvolutionEngine* engine, uint32_t size);
void EvolutionEngine_FreePopulation(EvolutionEngine* engine);
NTSTATUS EvolutionEngine_AllocateIndividual(EvolutionEngine* engine,
                                          EvolutionaryIndividual* individual,
                                          size_t genome_size);
void EvolutionEngine_FreeIndividual(EvolutionEngine* engine,
                                  EvolutionaryIndividual* individual);

#endif // EVOLUTION_ENGINE_H