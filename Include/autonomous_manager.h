#ifndef AUTONOMOUS_MANAGER_H
#define AUTONOMOUS_MANAGER_H

#include <windows.h>
#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stdbool.h>
#include "neural_substrate.h"
#include "screen_control.h"
#include "ethics_system.h"

// Raijin Autonomous Operation Manager
// Orchestrates all systems for self-directed operation

// Task types that Raijin can perform
typedef enum {
    TASK_LEARN = 0,           // Learn from user input or environment
    TASK_ANALYZE = 1,         // Analyze data or situations
    TASK_CREATE = 2,          // Create content, code, designs
    TASK_CONTROL = 3,         // Control programs or systems
    TASK_EXPLORE = 4,         // Explore internet or knowledge
    TASK_OPTIMIZE = 5,        // Optimize performance or processes
    TASK_MAINTAIN = 6,        // Self-maintenance and evolution
    TASK_INTERACT = 7,        // Interact with user or environment
    TASK_COUNT
} TaskType;

// Task priority levels
typedef enum {
    PRIORITY_IDLE = 0,        // Background maintenance
    PRIORITY_LOW = 1,         // Non-urgent tasks
    PRIORITY_NORMAL = 2,      // Standard operations
    PRIORITY_HIGH = 3,        // Important tasks
    PRIORITY_CRITICAL = 4     // Immediate action required
} TaskPriority;

// Task execution state
typedef enum {
    TASK_STATE_QUEUED = 0,    // Waiting to execute
    TASK_STATE_RUNNING = 1,   // Currently executing
    TASK_STATE_PAUSED = 2,    // Temporarily paused
    TASK_STATE_COMPLETED = 3, // Successfully finished
    TASK_STATE_FAILED = 4,    // Failed to complete
    TASK_STATE_CANCELLED = 5  // Cancelled by user or system
} TaskState;

// Autonomous task definition
typedef struct {
    uint64_t id;              // Unique task identifier
    TaskType type;            // Type of task
    TaskPriority priority;    // Execution priority
    TaskState state;          // Current execution state
    char* description;        // Human-readable description
    char* parameters;         // Task-specific parameters (JSON-like)
    uint64_t created_time;    // When task was created
    uint64_t started_time;    // When execution began
    uint64_t completed_time;  // When execution finished
    float progress;           // Completion percentage (0-1)
    char* result;             // Task execution result
    char* error_message;      // Error details if failed
    uint32_t retry_count;     // Number of retry attempts
    uint32_t max_retries;     // Maximum allowed retries
} AutonomousTask;

// Goal definition for long-term objectives
typedef struct {
    uint64_t id;              // Unique goal identifier
    char* description;        // Goal description
    char* criteria;           // Success criteria
    TaskPriority priority;    // Goal priority
    uint64_t created_time;    // When goal was set
    uint64_t deadline;        // Optional deadline
    float progress;           // Goal completion progress
    bool achieved;            // Whether goal is achieved
    AutonomousTask** tasks;   // Associated tasks
    uint32_t task_count;
} AutonomousGoal;

// Decision making context
typedef struct {
    char* situation;          // Current situation description
    char* user_intent;        // Inferred user intent
    char* available_actions;  // Possible actions
    float time_pressure;      // Urgency factor
    float risk_level;         // Risk assessment
    char* ethical_constraints; // Ethical boundaries
} DecisionContext;

// Autonomous operation manager
typedef struct {
    // Subsystem references
    NeuralSubstrate* neural_system;
    EthicsSystem* ethics_system;
    ScreenControlSystem* screen_system;

    // Task management
    AutonomousTask* task_queue;
    uint32_t task_count;
    uint32_t max_tasks;
    uint32_t next_task_id;

    // Goal management
    AutonomousGoal* goals;
    uint32_t goal_count;
    uint32_t max_goals;

    // Decision making
    DecisionContext current_context;
    float autonomy_level;     // How autonomous to be (0-1)
    float creativity_level;   // How creative in problem solving
    bool learning_enabled;    // Whether to learn from experiences

    // Operational state
    bool is_active;           // Whether autonomous operation is enabled
    bool user_present;        // Whether user is actively present
    uint64_t last_user_activity; // Timestamp of last user interaction
    uint32_t idle_threshold_ms; // How long before considering user idle

    // Performance metrics
    uint64_t tasks_completed;
    uint64_t tasks_failed;
    float average_completion_time;
    float system_efficiency;

    // Internal state
    CRITICAL_SECTION lock;
    bool initialized;
    HANDLE operation_thread;
    HANDLE monitoring_thread;
} AutonomousManager;

// Core API functions
NTSTATUS AutonomousManager_Initialize(AutonomousManager* manager,
                                    NeuralSubstrate* neural,
                                    EthicsSystem* ethics,
                                    ScreenControlSystem* screen);
NTSTATUS AutonomousManager_Shutdown(AutonomousManager* manager);
NTSTATUS AutonomousManager_StartOperation(AutonomousManager* manager);
NTSTATUS AutonomousManager_StopOperation(AutonomousManager* manager);

// Task management
NTSTATUS AutonomousManager_QueueTask(AutonomousManager* manager, TaskType type,
                                   const char* description, const char* parameters,
                                   TaskPriority priority);
NTSTATUS AutonomousManager_CancelTask(AutonomousManager* manager, uint64_t task_id);
NTSTATUS AutonomousManager_GetTaskStatus(AutonomousManager* manager, uint64_t task_id,
                                       AutonomousTask* task);
NTSTATUS AutonomousManager_ListTasks(AutonomousManager* manager, AutonomousTask** tasks,
                                   uint32_t* count);

// Goal management
NTSTATUS AutonomousManager_SetGoal(AutonomousManager* manager, const char* description,
                                 const char* criteria, TaskPriority priority);
NTSTATUS AutonomousManager_UpdateGoalProgress(AutonomousManager* manager, uint64_t goal_id,
                                            float progress);
NTSTATUS AutonomousManager_ListGoals(AutonomousManager* manager, AutonomousGoal** goals,
                                   uint32_t* count);

// Decision making
NTSTATUS AutonomousManager_MakeDecision(AutonomousManager* manager, const char* situation,
                                      char* decision, size_t decision_size);
NTSTATUS AutonomousManager_EvaluateAction(AutonomousManager* manager, const char* action,
                                        float* alignment_score, char* reasoning,
                                        size_t reasoning_size);

// Operational control
NTSTATUS AutonomousManager_SetAutonomyLevel(AutonomousManager* manager, float level);
NTSTATUS AutonomousManager_EnableLearning(AutonomousManager* manager, bool enable);
NTSTATUS AutonomousManager_ProcessUserInput(AutonomousManager* manager, const char* input);
NTSTATUS AutonomousManager_HandleScreenActivity(AutonomousManager* manager);

// Self-optimization
NTSTATUS AutonomousManager_OptimizePerformance(AutonomousManager* manager);
NTSTATUS AutonomousManager_AdaptToUser(AutonomousManager* manager);
NTSTATUS AutonomousManager_EvolveCapabilities(AutonomousManager* manager);

// Monitoring and reporting
NTSTATUS AutonomousManager_GetStatus(AutonomousManager* manager, char* status,
                                   size_t status_size);
NTSTATUS AutonomousManager_GetMetrics(AutonomousManager* manager, char* metrics,
                                    size_t metrics_size);

// Utility functions
uint64_t GenerateTaskId();
uint64_t GetCurrentTimeMs();
char* FormatTimestamp(uint64_t timestamp);
bool IsUserActive(AutonomousManager* manager);
TaskPriority CalculateTaskPriority(AutonomousManager* manager, TaskType type);

#endif // AUTONOMOUS_MANAGER_H