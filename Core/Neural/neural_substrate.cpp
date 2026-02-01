#include "neural_substrate.h"
#include "../../Include/role_boundary.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include <intrin.h>

#define NEURAL_CHECKPOINT_MAGIC "RAIJIN_NEURAL_V1"

// Raijin Neural Substrate Implementation
// Biological Computational Fusion with Hardware Constraints

// Global state (for backwards compatibility with legacy code)
static NeuralSubstrate* g_substrate = NULL;

// Hardware-aware memory allocation (fits within 32GB RAM constraint)
NTSTATUS AllocateNeuralMemory(size_t size, void** buffer) {
    // Use VirtualAlloc for large allocations with hardware-specific alignment
    *buffer = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (*buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Prefault pages to avoid allocation delays during computation
    memset(*buffer, 0, size);
    return STATUS_SUCCESS;
}

void FreeNeuralMemory(void* buffer) {
    if (buffer) {
        VirtualFree(buffer, 0, MEM_RELEASE);
    }
}

// Chaotic computation primitives
float GenerateChaos(float seed, float entropy) {
    // XORShift with entropy modulation for controlled chaos
    static uint32_t x = (uint32_t)(seed * 123456789);
    static uint32_t y = (uint32_t)(entropy * 362436069);
    static uint32_t z = (uint32_t)(seed * entropy * 521288629);
    static uint32_t w = (uint32_t)(entropy * 88675123);

    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    w = w ^ (w >> 19) ^ (t ^ (t >> 8));

    // Convert to float and modulate with entropy
    float chaos = (float)w / (float)UINT32_MAX;
    return chaos * entropy + (1.0f - entropy) * seed;
}

void EntropicActivation(float* values, size_t count, float chaos_level) {
    for (size_t i = 0; i < count; i++) {
        float chaos = GenerateChaos(values[i], chaos_level);
        // Chaotic activation: tanh with entropy modulation
        values[i] = tanhf(values[i] + chaos * 0.1f);
    }
}

float ComputeEmbeddingSimilarity(const HyperEmbedding* a, const HyperEmbedding* b) {
    if (a->size != b->size) return 0.0f;

    float dot_product = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;

    for (uint32_t i = 0; i < a->size; i++) {
        dot_product += a->dimensions[i] * b->dimensions[i];
        norm_a += a->dimensions[i] * a->dimensions[i];
        norm_b += b->dimensions[i] * b->dimensions[i];
    }

    norm_a = sqrtf(norm_a);
    norm_b = sqrtf(norm_b);

    if (norm_a == 0.0f || norm_b == 0.0f) return 0.0f;

    return dot_product / (norm_a * norm_b);
}

// Hyper-dimensional operations
void InitializeHyperEmbedding(HyperEmbedding* embedding, uint32_t dimensions) {
    embedding->size = dimensions;
    embedding->entropy = 0.0f;

    NTSTATUS status = AllocateNeuralMemory(dimensions * sizeof(float), (void**)&embedding->dimensions);
    if (!NT_SUCCESS(status)) {
        embedding->dimensions = NULL;
        return;
    }

    // Initialize with random values in hyper-dimensional space
    srand((unsigned int)time(NULL));
    for (uint32_t i = 0; i < dimensions; i++) {
        embedding->dimensions[i] = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
    }
}

void DestroyHyperEmbedding(HyperEmbedding* embedding) {
    if (embedding->dimensions) {
        FreeNeuralMemory(embedding->dimensions);
        embedding->dimensions = NULL;
    }
    embedding->size = 0;
    embedding->entropy = 0.0f;
}

void BindEmbeddings(HyperEmbedding* result, const HyperEmbedding* a, const HyperEmbedding* b) {
    if (a->size != b->size || result->size != a->size) return;

    for (uint32_t i = 0; i < a->size; i++) {
        // Circular convolution in hyper-dimensional space
        result->dimensions[i] = a->dimensions[i] * b->dimensions[i];
    }

    // Update entropy
    result->entropy = (a->entropy + b->entropy) * 0.5f;
}

void UnbindEmbeddings(HyperEmbedding* result, const HyperEmbedding* a, const HyperEmbedding* b) {
    if (a->size != b->size || result->size != a->size) return;

    for (uint32_t i = 0; i < a->size; i++) {
        // Approximate unbinding through correlation
        if (fabsf(b->dimensions[i]) > 1e-6f) {
            result->dimensions[i] = a->dimensions[i] / b->dimensions[i];
        } else {
            result->dimensions[i] = 0.0f;
        }
    }

    result->entropy = fabsf(a->entropy - b->entropy);
}

// Sparse operations (memory efficient for 86B neurons)
void SparseMatrixVectorMultiply(const float* matrix, const uint64_t* indices, uint32_t count, const float* vector, float* result) {
    memset(result, 0, count * sizeof(float));

    for (uint32_t i = 0; i < count; i++) {
        uint64_t idx = indices[i];
        if (idx < count) {
            result[i] += matrix[i] * vector[idx];
        }
    }
}

void UpdateSparseConnections(SparseNeuron* neuron, const float* gradients, float learning_rate) {
    for (uint32_t i = 0; i < neuron->input_count; i++) {
        neuron->weights[i] -= learning_rate * gradients[i] * neuron->plasticity;
        // Apply weight constraints
        neuron->weights[i] = std::max(-1.0f, std::min(1.0f, neuron->weights[i]));
    }
}

// Evolutionary algorithms
void MutateNeuralWeights(SparseNeuron* neuron, float mutation_rate) {
    for (uint32_t i = 0; i < neuron->input_count; i++) {
        if ((float)rand() / RAND_MAX < mutation_rate) {
            neuron->weights[i] += ((float)rand() / RAND_MAX - 0.5f) * 0.2f;
            neuron->weights[i] = std::max(-1.0f, std::min(1.0f, neuron->weights[i]));
        }
    }

    // Mutate threshold and plasticity
    neuron->threshold += ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
    neuron->plasticity += ((float)rand() / RAND_MAX - 0.5f) * 0.05f;
    neuron->plasticity = std::max(0.001f, std::min(1.0f, neuron->plasticity));
}

void CrossoverNeurons(const SparseNeuron* parent1, const SparseNeuron* parent2, SparseNeuron* child) {
    // Uniform crossover for weights
    for (uint32_t i = 0; i < child->input_count; i++) {
        child->weights[i] = (rand() % 2) ? parent1->weights[i] : parent2->weights[i];
    }

    // Blend other parameters
    child->threshold = (parent1->threshold + parent2->threshold) * 0.5f;
    child->plasticity = (parent1->plasticity + parent2->plasticity) * 0.5f;
    child->entropy_level = (parent1->entropy_level + parent2->entropy_level) * 0.5f;
}

float EvaluateNeuronFitness(const SparseNeuron* neuron, const float* targets) {
    // Simple fitness based on output stability and entropy
    float fitness = 1.0f - fabsf(neuron->membrane_potential); // Prefer stable outputs
    fitness += neuron->entropy_level * 0.1f; // Reward some chaos
    return fitness;
}

// Biological inspiration functions
void SimulateNeuralHomeostasis(NeuralFabric* fabric) {
    // Adjust global parameters to maintain stability
    float avg_activation = 0.0f;
    uint64_t active_count = 0;

    // Sample a subset of neurons for efficiency
    for (uint64_t i = 0; i < fabric->active_neuron_count && i < 10000; i++) {
        SparseNeuron* neuron = &fabric->neurons[i];
        if (fabsf(neuron->membrane_potential) > 0.1f) {
            avg_activation += fabsf(neuron->membrane_potential);
            active_count++;
        }
    }

    if (active_count > 0) {
        avg_activation /= active_count;

        // Adjust learning temperature based on activation level
        if (avg_activation > 0.8f) {
            fabric->learning_temperature *= 0.99f; // Cool down if too active
        } else if (avg_activation < 0.2f) {
            fabric->learning_temperature *= 1.01f; // Heat up if too quiet
        }
    }
}

void ApplySynapticPruning(NeuralFabric* fabric, float pruning_threshold) {
    for (uint64_t i = 0; i < fabric->active_neuron_count; i++) {
        SparseNeuron* neuron = &fabric->neurons[i];

        uint32_t pruned = 0;
        for (uint32_t j = 0; j < neuron->input_count; j++) {
            if (fabsf(neuron->weights[j]) < pruning_threshold) {
                neuron->weights[j] = 0.0f;
                pruned++;
            }
        }

        // Update connection count
        neuron->input_count -= pruned;
    }
}

void GenerateNeuralOscillations(NeuralFabric* fabric, float frequency) {
    float time = (float)clock() / CLOCKS_PER_SEC;
    float oscillation = sinf(time * frequency * 2.0f * 3.14159f);

    // Apply oscillation to entropy levels
    for (uint64_t i = 0; i < fabric->active_neuron_count && i < 10000; i++) {
        SparseNeuron* neuron = &fabric->neurons[i];
        neuron->entropy_level += oscillation * 0.01f;
        neuron->entropy_level = std::max(0.0f, std::min(1.0f, neuron->entropy_level));
    }
}

// Neural operations implementation
static void NeuralFabric_Initialize(NeuralFabric* fabric) {
    // Initialize with sparse representation
    // Start with 10K neurons (safe default, ~120MB memory)
    // Scale up dynamically based on available resources
    fabric->active_neuron_count = 10000;
    fabric->total_connections = (uint64_t)(fabric->active_neuron_count * SPARSITY_FACTOR * fabric->active_neuron_count);
    fabric->global_entropy = 0.5f;
    fabric->learning_temperature = 1.0f;
    fabric->neurons = NULL;
    fabric->knowledge_base = NULL;
    fabric->entropic_engine = NULL;

    // Allocate neurons (sparse representation)
    NTSTATUS status = AllocateNeuralMemory(fabric->active_neuron_count * sizeof(SparseNeuron), (void**)&fabric->neurons);
    if (!NT_SUCCESS(status) || !fabric->neurons) {
        fabric->active_neuron_count = 0;
        return;
    }
    memset(fabric->neurons, 0, fabric->active_neuron_count * sizeof(SparseNeuron));

    // Initialize neurons
    srand((unsigned int)time(NULL));
    uint64_t successful_neurons = 0;
    for (uint64_t i = 0; i < fabric->active_neuron_count; i++) {
        SparseNeuron* neuron = &fabric->neurons[i];
        neuron->id = i;
        neuron->type = (NeuronType)(rand() % 4);
        neuron->activation = (ActivationFunction)(rand() % 4);
        neuron->membrane_potential = 0.0f;
        neuron->threshold = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
        neuron->entropy_level = (float)rand() / RAND_MAX;
        neuron->plasticity = PLASTICITY_RATE;
        neuron->weights = NULL;
        neuron->input_ids = NULL;

        // Sparse connections (only a few inputs per neuron)
        neuron->input_count = (uint32_t)(SPARSITY_FACTOR * fabric->active_neuron_count);
        if (neuron->input_count < 1) neuron->input_count = 1;
        neuron->output_count = neuron->input_count;

        // Allocate sparse weights and indices with null checks
        status = AllocateNeuralMemory(neuron->input_count * sizeof(float), (void**)&neuron->weights);
        if (!NT_SUCCESS(status) || !neuron->weights) {
            neuron->input_count = 0;
            continue;
        }
        status = AllocateNeuralMemory(neuron->input_count * sizeof(uint64_t), (void**)&neuron->input_ids);
        if (!NT_SUCCESS(status) || !neuron->input_ids) {
            FreeNeuralMemory(neuron->weights);
            neuron->weights = NULL;
            neuron->input_count = 0;
            continue;
        }

        // Initialize sparse connections
        for (uint32_t j = 0; j < neuron->input_count; j++) {
            neuron->weights[j] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
            neuron->input_ids[j] = rand() % fabric->active_neuron_count;
        }
        successful_neurons++;
    }

    // Initialize knowledge base
    fabric->knowledge_base = (HyperEmbedding*)malloc(sizeof(HyperEmbedding));
    if (fabric->knowledge_base) {
        memset(fabric->knowledge_base, 0, sizeof(HyperEmbedding));
        InitializeHyperEmbedding(fabric->knowledge_base, EMBEDDING_DIMENSIONS);
    }

    // Initialize entropic engine
    status = AllocateNeuralMemory(fabric->active_neuron_count * sizeof(float), (void**)&fabric->entropic_engine);
    if (!NT_SUCCESS(status)) {
        fabric->entropic_engine = NULL;
    }
}

static void NeuralFabric_Activate(NeuralFabric* fabric, const float* inputs, float* outputs) {
    // Forward pass through sparse neural network
    float* activations = fabric->entropic_engine;

    // Copy inputs to first layer
    memcpy(activations, inputs, std::min(fabric->active_neuron_count, (uint64_t)1000) * sizeof(float));

    // Process through network layers (simplified for demonstration)
    for (uint64_t i = 0; i < fabric->active_neuron_count; i++) {
        SparseNeuron* neuron = &fabric->neurons[i];

        // Compute weighted sum from sparse inputs
        float sum = 0.0f;
        for (uint32_t j = 0; j < neuron->input_count; j++) {
            uint64_t input_idx = neuron->input_ids[j];
            if (input_idx < fabric->active_neuron_count) {
                sum += neuron->weights[j] * activations[input_idx];
            }
        }

        // Add bias and entropy
        sum += neuron->threshold;
        sum += GenerateChaos(sum, neuron->entropy_level) * fabric->global_entropy;

        // Apply activation
        switch (neuron->activation) {
            case ACTIVATION_TANH:
                neuron->membrane_potential = tanhf(sum);
                break;
            case ACTIVATION_SIGMOID:
                neuron->membrane_potential = 1.0f / (1.0f + expf(-sum));
                break;
            case ACTIVATION_RELU:
                neuron->membrane_potential = std::max(0.0f, sum);
                break;
            case ACTIVATION_ENTROPIC:
                neuron->membrane_potential = tanhf(sum + GenerateChaos(sum, fabric->global_entropy));
                break;
        }

        activations[i] = neuron->membrane_potential;
    }

    // Copy outputs
    memcpy(outputs, activations, std::min(fabric->active_neuron_count, (uint64_t)1000) * sizeof(float));
}

static void NeuralFabric_Learn(NeuralFabric* fabric, const float* targets, float learning_rate) {
    // Simplified backpropagation for sparse network
    float* gradients = (float*)malloc(fabric->active_neuron_count * sizeof(float));

    // Compute output layer gradients
    for (uint64_t i = 0; i < std::min(fabric->active_neuron_count, (uint64_t)1000); i++) {
        SparseNeuron* neuron = &fabric->neurons[i];
        gradients[i] = targets[i] - neuron->membrane_potential;
    }

    // Backpropagate through network
    for (uint64_t i = 0; i < fabric->active_neuron_count; i++) {
        SparseNeuron* neuron = &fabric->neurons[i];

        // Compute gradients for this neuron
        float neuron_gradient = gradients[i];

        // Apply activation derivative
        switch (neuron->activation) {
            case ACTIVATION_TANH:
                neuron_gradient *= (1.0f - neuron->membrane_potential * neuron->membrane_potential);
                break;
            case ACTIVATION_SIGMOID:
                neuron_gradient *= neuron->membrane_potential * (1.0f - neuron->membrane_potential);
                break;
            case ACTIVATION_RELU:
                neuron_gradient *= (neuron->membrane_potential > 0.0f) ? 1.0f : 0.0f;
                break;
            case ACTIVATION_ENTROPIC:
                neuron_gradient *= (1.0f - neuron->membrane_potential * neuron->membrane_potential);
                break;
        }

        // Update weights
        UpdateSparseConnections(neuron, &neuron_gradient, learning_rate);

        // Propagate gradients backward
        for (uint32_t j = 0; j < neuron->input_count; j++) {
            uint64_t input_idx = neuron->input_ids[j];
            if (input_idx < fabric->active_neuron_count) {
                gradients[input_idx] += neuron_gradient * neuron->weights[j];
            }
        }
    }

    free(gradients);
}

static void NeuralFabric_Evolve(NeuralFabric* fabric, EvolutionaryParams* params) {
    // Evolutionary algorithm for neural architecture
    uint32_t elite_count = (uint32_t)(params->population_size * 0.1f);

    // Evaluate fitness (simplified)
    float* fitness = (float*)malloc(params->population_size * sizeof(float));
    for (uint32_t i = 0; i < params->population_size; i++) {
        fitness[i] = EvaluateNeuronFitness(&fabric->neurons[i % fabric->active_neuron_count], NULL);
    }

    // Selection and reproduction
    for (uint32_t i = elite_count; i < params->population_size; i++) {
        // Tournament selection
        uint32_t parent1 = rand() % params->population_size;
        uint32_t parent2 = rand() % params->population_size;

        if (fitness[parent2] > fitness[parent1]) parent1 = parent2;

        // Crossover
        SparseNeuron* child = &fabric->neurons[i % fabric->active_neuron_count];
        CrossoverNeurons(&fabric->neurons[parent1 % fabric->active_neuron_count],
                        &fabric->neurons[parent2 % fabric->active_neuron_count], child);

        // Mutation
        MutateNeuralWeights(child, params->mutation_rate);
    }

    free(fitness);
    params->generation++;
}

static void NeuralFabric_EmbedConcept(NeuralFabric* fabric, const void* data, size_t size, HyperEmbedding* embedding) {
    // Create hyper-dimensional embedding from arbitrary data
    InitializeHyperEmbedding(embedding, EMBEDDING_DIMENSIONS);

    // Hash data into embedding dimensions
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        uint32_t dim = (bytes[i] * 2654435761) % EMBEDDING_DIMENSIONS;
        embedding->dimensions[dim] += (float)bytes[i] / 255.0f;
    }

    // Normalize
    float norm = 0.0f;
    for (uint32_t i = 0; i < EMBEDDING_DIMENSIONS; i++) {
        norm += embedding->dimensions[i] * embedding->dimensions[i];
    }
    norm = sqrtf(norm);
    if (norm > 0.0f) {
        for (uint32_t i = 0; i < EMBEDDING_DIMENSIONS; i++) {
            embedding->dimensions[i] /= norm;
        }
    }

    // Add entropy
    embedding->entropy = GenerateChaos((float)size, fabric->global_entropy);
}

static float NeuralFabric_ComputeEntropy(const NeuralFabric* fabric) {
    float total_entropy = 0.0f;
    for (uint64_t i = 0; i < fabric->active_neuron_count && i < 10000; i++) {
        total_entropy += fabric->neurons[i].entropy_level;
    }
    return total_entropy / std::min(fabric->active_neuron_count, (uint64_t)10000);
}

static void NeuralFabric_AdjustPlasticity(NeuralFabric* fabric, float temperature) {
    fabric->learning_temperature = temperature;
    for (uint64_t i = 0; i < fabric->active_neuron_count; i++) {
        fabric->neurons[i].plasticity = PLASTICITY_RATE * temperature;
    }
}

// Main API implementation
NTSTATUS NeuralSubstrate_Initialize(NeuralSubstrate* substrate) {
    if (!substrate) return STATUS_INVALID_PARAMETER;
    if (substrate->initialized) return STATUS_SUCCESS;

    InitializeCriticalSection(&substrate->lock);
    g_substrate = substrate;

    // Initialize neural operations
    substrate->ops.initialize = NeuralFabric_Initialize;
    substrate->ops.activate = NeuralFabric_Activate;
    substrate->ops.learn = NeuralFabric_Learn;
    substrate->ops.evolve = NeuralFabric_Evolve;
    substrate->ops.embed_concept = NeuralFabric_EmbedConcept;
    substrate->ops.compute_entropy = NeuralFabric_ComputeEntropy;
    substrate->ops.adjust_plasticity = NeuralFabric_AdjustPlasticity;

    // Initialize evolutionary parameters
    substrate->evolution.mutation_rate = 0.01f;
    substrate->evolution.crossover_rate = 0.7f;
    substrate->evolution.selection_pressure = 2.0f;
    substrate->evolution.population_size = 1000;
    substrate->evolution.generation = 0;

    // Initialize neural fabric
    substrate->ops.initialize(&substrate->fabric);

    substrate->initialized = true;
    return STATUS_SUCCESS;
}

NTSTATUS NeuralSubstrate_Shutdown(NeuralSubstrate* substrate) {
    if (!substrate->initialized) return STATUS_SUCCESS;

    // Free neural fabric resources
    if (substrate->fabric.neurons) {
        for (uint64_t i = 0; i < substrate->fabric.active_neuron_count; i++) {
            SparseNeuron* neuron = &substrate->fabric.neurons[i];
            if (neuron->weights) FreeNeuralMemory(neuron->weights);
            if (neuron->input_ids) FreeNeuralMemory(neuron->input_ids);
        }
        FreeNeuralMemory(substrate->fabric.neurons);
    }

    if (substrate->fabric.knowledge_base) {
        DestroyHyperEmbedding(substrate->fabric.knowledge_base);
        free(substrate->fabric.knowledge_base);
    }

    if (substrate->fabric.entropic_engine) {
        FreeNeuralMemory(substrate->fabric.entropic_engine);
    }

    substrate->initialized = false;
    DeleteCriticalSection(&substrate->lock);
    return STATUS_SUCCESS;
}

NTSTATUS NeuralSubstrate_Process(NeuralSubstrate* substrate, const void* input, size_t input_size, void* output, size_t output_size) {
    if (!substrate->initialized) return STATUS_INVALID_DEVICE_STATE;
    {
        RoleBoundaryContext* rbc = RoleBoundary_GetGlobal();
        if (rbc && !RoleBoundary_AssertRaijin(rbc))
            return STATUS_ROLE_BOUNDARY_VIOLATION;
    }
    EnterCriticalSection(&substrate->lock);

    // Convert input to float array
    float* float_input = (float*)malloc(input_size * sizeof(float));
    const uint8_t* bytes = (const uint8_t*)input;
    for (size_t i = 0; i < input_size; i++) {
        float_input[i] = (float)bytes[i] / 255.0f;
    }

    float* float_output = (float*)malloc(output_size * sizeof(float));
    if (!float_output) {
        free(float_input);
        LeaveCriticalSection(&substrate->lock);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    substrate->ops.activate(&substrate->fabric, float_input, float_output);

    // Convert output to bytes
    uint8_t* byte_output = (uint8_t*)output;
    for (size_t i = 0; i < output_size; i++) {
        byte_output[i] = (uint8_t)(float_output[i] * 255.0f);
    }

    free(float_input);
    free(float_output);

    LeaveCriticalSection(&substrate->lock);
    return STATUS_SUCCESS;
}

NTSTATUS NeuralSubstrate_Learn(NeuralSubstrate* substrate, const void* target, size_t target_size) {
    if (!substrate->initialized) return STATUS_INVALID_DEVICE_STATE;
    {
        RoleBoundaryContext* rbc = RoleBoundary_GetGlobal();
        if (rbc && !RoleBoundary_AssertRaijin(rbc))
            return STATUS_ROLE_BOUNDARY_VIOLATION;
    }
    EnterCriticalSection(&substrate->lock);

    // Convert target to float array
    float* float_target = (float*)malloc(target_size * sizeof(float));
    const uint8_t* bytes = (const uint8_t*)target;
    for (size_t i = 0; i < target_size; i++) {
        float_target[i] = (float)bytes[i] / 255.0f;
    }

    // Learn from target
    substrate->ops.learn(&substrate->fabric, float_target, PLASTICITY_RATE);

    free(float_target);

    LeaveCriticalSection(&substrate->lock);
    return STATUS_SUCCESS;
}

NTSTATUS NeuralSubstrate_Evolve(NeuralSubstrate* substrate) {
    if (!substrate->initialized) return STATUS_INVALID_DEVICE_STATE;

    EnterCriticalSection(&substrate->lock);

    substrate->ops.evolve(&substrate->fabric, &substrate->evolution);

    // Periodic homeostasis
    static int homeostasis_counter = 0;
    if (++homeostasis_counter % 100 == 0) {
        SimulateNeuralHomeostasis(&substrate->fabric);
    }

    // Periodic pruning
    static int pruning_counter = 0;
    if (++pruning_counter % 1000 == 0) {
        ApplySynapticPruning(&substrate->fabric, 0.001f);
    }

    // Generate oscillations
    GenerateNeuralOscillations(&substrate->fabric, 0.1f);

    LeaveCriticalSection(&substrate->lock);
    return STATUS_SUCCESS;
}

float NeuralSubstrate_GetEntropy(const NeuralSubstrate* substrate) {
    if (!substrate->initialized) return 0.0f;
    return substrate->ops.compute_entropy(&substrate->fabric);
}

NTSTATUS NeuralSubstrate_SaveState(const NeuralSubstrate* substrate, const char* filename) {
    if (!substrate || !substrate->initialized || !filename) return STATUS_INVALID_PARAMETER;

    FILE* f = fopen(filename, "wb");
    if (!f) return STATUS_UNSUCCESSFUL;

    const NeuralFabric* fabric = &substrate->fabric;
    size_t nw;

    nw = fwrite(NEURAL_CHECKPOINT_MAGIC, 1, 16, f);
    if (nw != 16) { fclose(f); return STATUS_UNSUCCESSFUL; }

    nw = fwrite(&fabric->active_neuron_count, sizeof(uint64_t), 1, f);
    if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }

    for (uint64_t i = 0; i < fabric->active_neuron_count; i++) {
        const SparseNeuron* n = &fabric->neurons[i];
        nw = fwrite(&n->id, sizeof(uint64_t), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        uint32_t t = (uint32_t)n->type;
        uint32_t a = (uint32_t)n->activation;
        nw = fwrite(&t, sizeof(uint32_t), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nw = fwrite(&a, sizeof(uint32_t), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nw = fwrite(&n->membrane_potential, sizeof(float), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nw = fwrite(&n->threshold, sizeof(float), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nw = fwrite(&n->entropy_level, sizeof(float), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nw = fwrite(&n->input_count, sizeof(uint32_t), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nw = fwrite(&n->output_count, sizeof(uint32_t), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nw = fwrite(&n->plasticity, sizeof(float), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        if (n->input_count > 0 && n->weights) {
            nw = fwrite(n->weights, sizeof(float), n->input_count, f);
            if (nw != n->input_count) { fclose(f); return STATUS_UNSUCCESSFUL; }
            nw = fwrite(n->input_ids, sizeof(uint64_t), n->input_count, f);
            if (nw != n->input_count) { fclose(f); return STATUS_UNSUCCESSFUL; }
        }
    }

    if (fabric->knowledge_base && fabric->knowledge_base->dimensions) {
        uint32_t es = fabric->knowledge_base->size;
        nw = fwrite(&es, sizeof(uint32_t), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nw = fwrite(fabric->knowledge_base->dimensions, sizeof(float), es, f);
        if (nw != es) { fclose(f); return STATUS_UNSUCCESSFUL; }
    } else {
        uint32_t zero = 0;
        nw = fwrite(&zero, sizeof(uint32_t), 1, f);
        if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
    }

    nw = fwrite(&fabric->global_entropy, sizeof(float), 1, f);
    if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
    nw = fwrite(&fabric->learning_temperature, sizeof(float), 1, f);
    if (nw != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }

    if (fabric->entropic_engine && fabric->active_neuron_count > 0) {
        nw = fwrite(fabric->entropic_engine, sizeof(float), (size_t)fabric->active_neuron_count, f);
        if (nw != (size_t)fabric->active_neuron_count) { fclose(f); return STATUS_UNSUCCESSFUL; }
    }

    fclose(f);
    return STATUS_SUCCESS;
}

NTSTATUS NeuralSubstrate_LoadState(NeuralSubstrate* substrate, const char* filename) {
    if (!substrate || !substrate->initialized || !filename) return STATUS_INVALID_PARAMETER;

    FILE* f = fopen(filename, "rb");
    if (!f) return STATUS_UNSUCCESSFUL;

    NeuralFabric* fabric = &substrate->fabric;
    char magic[16];
    size_t nr = fread(magic, 1, 16, f);
    if (nr != 16 || memcmp(magic, NEURAL_CHECKPOINT_MAGIC, 16) != 0) {
        fclose(f);
        return STATUS_UNSUCCESSFUL;
    }

    uint64_t saved_count = 0;
    nr = fread(&saved_count, sizeof(uint64_t), 1, f);
    if (nr != 1 || saved_count != fabric->active_neuron_count) {
        fclose(f);
        return STATUS_INVALID_DEVICE_STATE;
    }

    for (uint64_t i = 0; i < fabric->active_neuron_count; i++) {
        SparseNeuron* n = &fabric->neurons[i];
        nr = fread(&n->id, sizeof(uint64_t), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        uint32_t t, a;
        nr = fread(&t, sizeof(uint32_t), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nr = fread(&a, sizeof(uint32_t), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        n->type = (NeuronType)t;
        n->activation = (ActivationFunction)a;
        nr = fread(&n->membrane_potential, sizeof(float), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nr = fread(&n->threshold, sizeof(float), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nr = fread(&n->entropy_level, sizeof(float), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        uint32_t in_count = 0, out_count = 0;
        nr = fread(&in_count, sizeof(uint32_t), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        nr = fread(&out_count, sizeof(uint32_t), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        n->input_count = in_count;
        n->output_count = out_count;
        nr = fread(&n->plasticity, sizeof(float), 1, f);
        if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
        if (in_count > 0 && n->weights && n->input_ids) {
            nr = fread(n->weights, sizeof(float), in_count, f);
            if (nr != in_count) { fclose(f); return STATUS_UNSUCCESSFUL; }
            nr = fread(n->input_ids, sizeof(uint64_t), in_count, f);
            if (nr != in_count) { fclose(f); return STATUS_UNSUCCESSFUL; }
        }
    }

    uint32_t es = 0;
    nr = fread(&es, sizeof(uint32_t), 1, f);
    if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
    if (es > 0 && fabric->knowledge_base && fabric->knowledge_base->dimensions &&
        fabric->knowledge_base->size >= es) {
        nr = fread(fabric->knowledge_base->dimensions, sizeof(float), es, f);
        if (nr != es) { fclose(f); return STATUS_UNSUCCESSFUL; }
    } else if (es > 0) {
        for (uint32_t i = 0; i < es; i++) { float dummy; fread(&dummy, sizeof(float), 1, f); }
    }

    nr = fread(&fabric->global_entropy, sizeof(float), 1, f);
    if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }
    nr = fread(&fabric->learning_temperature, sizeof(float), 1, f);
    if (nr != 1) { fclose(f); return STATUS_UNSUCCESSFUL; }

    if (fabric->entropic_engine && fabric->active_neuron_count > 0) {
        nr = fread(fabric->entropic_engine, sizeof(float), (size_t)fabric->active_neuron_count, f);
        if (nr != (size_t)fabric->active_neuron_count) { fclose(f); return STATUS_UNSUCCESSFUL; }
    }

    fclose(f);
    return STATUS_SUCCESS;
}