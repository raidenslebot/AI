#ifndef INTERNET_ACQUISITION_H
#define INTERNET_ACQUISITION_H

#include <windows.h>
#include "raijin_ntstatus.h"
#include "http_client.h"
#include <winhttp.h>
#include <stdint.h>
#include <stdbool.h>

// Raijin Internet Acquisition & Knowledge Gathering System
// Zero-trace, multi-modal knowledge acquisition with neural integration

// Forward declarations
typedef struct InternetAcquisitionSystem InternetAcquisitionSystem;
typedef struct ContentProcessor ContentProcessor;
typedef struct KnowledgeBase KnowledgeBase;
typedef struct AcquisitionTask AcquisitionTask;

// Core system structures
struct InternetAcquisitionSystem {
    HttpClient* http_client;
    ContentProcessor* content_processor;
    KnowledgeBase* knowledge_base;

    // Configuration
    uint32_t max_concurrent_requests;
    uint32_t request_timeout_ms;
    uint32_t max_content_size_mb;
    bool stealth_mode;
    bool respect_robots_txt;

    // Statistics
    uint64_t total_requests;
    uint64_t successful_acquisitions;
    uint64_t failed_acquisitions;
    uint64_t bytes_processed;
    uint64_t concepts_extracted;

    // Threading
    HANDLE acquisition_thread;
    HANDLE processing_thread;
    CRITICAL_SECTION lock;
    bool is_active;

    // Integration
    void* neural_substrate;  // Pointer to neural system for real-time learning
    void* ethics_system;     // Pointer to ethics system for acquisition decisions
};

// Content types
typedef enum {
    CONTENT_TYPE_HTML,
    CONTENT_TYPE_JSON,
    CONTENT_TYPE_XML,
    CONTENT_TYPE_PDF,
    CONTENT_TYPE_IMAGE_JPEG,
    CONTENT_TYPE_IMAGE_PNG,
    CONTENT_TYPE_VIDEO_MP4,
    CONTENT_TYPE_AUDIO_MP3,
    CONTENT_TYPE_CODE_PYTHON,
    CONTENT_TYPE_CODE_CPP,
    CONTENT_TYPE_CODE_JAVASCRIPT,
    CONTENT_TYPE_TEXT_PLAIN,
    CONTENT_TYPE_UNKNOWN
} ContentType;

// Content metadata
typedef struct {
    ContentType type;
    char* url;
    char* title;
    char* description;
    uint64_t size_bytes;
    uint64_t timestamp;
    float credibility_score;
    char* language;
    char** keywords;
    uint32_t keyword_count;
} ContentMetadata;

// Processed content
typedef struct {
    ContentMetadata metadata;
    void* raw_content;
    uint64_t raw_size;

    // Extracted knowledge
    char* text_content;
    uint64_t text_length;

    // Multi-modal data
    void* image_data;
    void* audio_data;
    void* video_data;

    // Structured data
    void* json_data;
    void* xml_data;

    // Code analysis
    char* code_language;
    char** functions;
    uint32_t function_count;
    char** classes;
    uint32_t class_count;
} ProcessedContent;

// Content Processor
struct ContentProcessor {
    // HTML parsing
    void* html_parser;

    // Multi-modal processors
    void* image_processor;
    void* audio_processor;
    void* video_processor;

    // Code analysis
    void* code_analyzer;

    // NLP pipeline
    void* nlp_processor;

    // Statistics
    uint64_t content_processed;
    uint64_t text_extracted;
    uint64_t images_processed;
    uint64_t code_analyzed;
};

// Knowledge storage
typedef struct {
    char* concept;
    float* embedding;
    uint32_t embedding_size;
    float confidence;
    uint64_t timestamp;
    char** related_concepts;
    uint32_t relation_count;
    ContentMetadata* source;
    uint32_t observation_count;
} KnowledgeConcept;

struct KnowledgeBase {
    KnowledgeConcept* concepts;
    uint32_t concept_count;
    uint32_t max_concepts;

    // Indexing
    void* concept_index;  // Hash map for fast lookup

    // Persistence
    char* storage_path;
    bool auto_save;

    // Statistics
    uint64_t total_concepts;
    uint64_t storage_size_mb;
    float average_confidence;
};

// Acquisition task types
typedef enum {
    TASK_ACQUIRE_URL,
    TASK_ACQUIRE_SEARCH,
    TASK_ACQUIRE_DOMAIN,
    TASK_EXPLORE_LINKS,
    TASK_MONITOR_FEED,
    TASK_PROCESS_CONTENT,
    TASK_EXTRACT_KNOWLEDGE
} AcquisitionTaskType;

// Acquisition task
struct AcquisitionTask {
    uint64_t id;
    AcquisitionTaskType type;
    char* target;
    char* parameters;
    uint32_t priority;
    uint32_t timeout_ms;

    // State
    bool completed;
    bool success;
    char* error_message;

    // Results
    ProcessedContent* content;
    KnowledgeConcept* concepts;
    uint32_t concept_count;

    // Timing
    uint64_t created_time;
    uint64_t started_time;
    uint64_t completed_time;
};

// API Functions

// System lifecycle
NTSTATUS InternetAcquisition_Initialize(InternetAcquisitionSystem** system,
                                      void* neural_substrate, void* ethics_system);
NTSTATUS InternetAcquisition_Shutdown(InternetAcquisitionSystem* system);

// Core acquisition
NTSTATUS InternetAcquisition_AcquireURL(InternetAcquisitionSystem* system,
                                       const char* url, ProcessedContent** content);
void InternetAcquisition_FreeProcessedContent(ProcessedContent* content);
NTSTATUS InternetAcquisition_AcquireSearch(InternetAcquisitionSystem* system,
                                          const char* query, uint32_t max_results,
                                          ProcessedContent** results, uint32_t* result_count);

// Content processing
NTSTATUS InternetAcquisition_ProcessContent(InternetAcquisitionSystem* system,
                                          const void* raw_content, uint64_t size,
                                          ContentType type, ProcessedContent** processed);

// Knowledge integration
NTSTATUS InternetAcquisition_ExtractKnowledge(InternetAcquisitionSystem* system,
                                            ProcessedContent* content,
                                            KnowledgeConcept** concepts, uint32_t* count);
NTSTATUS InternetAcquisition_IntegrateKnowledge(InternetAcquisitionSystem* system,
                                              KnowledgeConcept* concepts, uint32_t count);

// Task management
NTSTATUS InternetAcquisition_QueueTask(InternetAcquisitionSystem* system,
                                     AcquisitionTaskType type, const char* target,
                                     const char* parameters, uint32_t priority);
NTSTATUS InternetAcquisition_GetTaskStatus(InternetAcquisitionSystem* system,
                                         uint64_t task_id, AcquisitionTask** task);

// Autonomous exploration
NTSTATUS InternetAcquisition_StartExploration(InternetAcquisitionSystem* system,
                                            const char* seed_urls, uint32_t depth,
                                            const char* objectives);
NTSTATUS InternetAcquisition_StopExploration(InternetAcquisitionSystem* system);

// Stealth and ethics
NTSTATUS InternetAcquisition_SetStealthMode(InternetAcquisitionSystem* system, bool enabled);
NTSTATUS InternetAcquisition_CheckEthicalAcquisition(InternetAcquisitionSystem* system,
                                                   const char* url, bool* allowed,
                                                   char* reasoning, size_t reasoning_size);

// Statistics and monitoring
NTSTATUS InternetAcquisition_GetStatistics(InternetAcquisitionSystem* system,
                                         char* stats, size_t stats_size);
NTSTATUS InternetAcquisition_GetKnowledgeStats(InternetAcquisitionSystem* system,
                                             char* stats, size_t stats_size);

// Utility functions
ContentType InternetAcquisition_DetectContentType(const void* content, uint64_t size, const char* url);
bool InternetAcquisition_IsValidURL(const char* url);
char* InternetAcquisition_NormalizeURL(const char* url);
uint32_t InternetAcquisition_CalculateCredibility(const char* domain, const char* content);

// CLI interface functions (for direct testing)
int InternetAcquisition_CLI_Acquire(int argc, char* argv[]);
int InternetAcquisition_CLI_Process(int argc, char* argv[]);
int InternetAcquisition_CLI_Explore(int argc, char* argv[]);
int InternetAcquisition_CLI_Analyze(int argc, char* argv[]);

#endif // INTERNET_ACQUISITION_H