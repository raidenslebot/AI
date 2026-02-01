#ifndef SCREEN_CONTROL_H
#define SCREEN_CONTROL_H

#include <windows.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif
#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER ((NTSTATUS)0xC000000DL)
#endif
#ifndef STATUS_INSUFFICIENT_RESOURCES
#define STATUS_INSUFFICIENT_RESOURCES ((NTSTATUS)0xC000009AL)
#endif
#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#endif
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

// Raijin Screen Observation and Control System
// Full system perception and autonomous interaction

// Screen capture configuration
#define MAX_DISPLAYS 8
#define MAX_SCREEN_WIDTH 7680
#define MAX_SCREEN_HEIGHT 4320
#define SCREEN_BUFFER_SIZE (MAX_SCREEN_WIDTH * MAX_SCREEN_HEIGHT * 4) // RGBA

// Pixel analysis
typedef struct {
    uint8_t r, g, b, a;
    float brightness;
    float saturation;
    float hue;
} PixelInfo;

typedef struct {
    PixelInfo* pixels;
    uint32_t width;
    uint32_t height;
    uint64_t timestamp;
    uint32_t display_index;
} ScreenFrame;

// Object recognition
typedef enum {
    OBJECT_TYPE_WINDOW = 0,
    OBJECT_TYPE_BUTTON = 1,
    OBJECT_TYPE_TEXT = 2,
    OBJECT_TYPE_ICON = 3,
    OBJECT_TYPE_MENU = 4,
    OBJECT_TYPE_CURSOR = 5,
    OBJECT_TYPE_UNKNOWN = 6
} ObjectType;

typedef struct {
    RECT bounds;
    ObjectType type;
    char* description;
    float confidence;
    void* metadata; // Type-specific data
} ScreenObject;

typedef struct {
    ScreenObject* objects;
    uint32_t count;
    uint32_t max_objects;
} ObjectDetectionResult;

// Text recognition
typedef struct {
    RECT bounds;
    char* text;
    char* font_name;
    uint32_t font_size;
    uint32_t color;
    float confidence;
} RecognizedText;

typedef struct {
    RecognizedText* texts;
    uint32_t count;
    uint32_t max_texts;
} TextRecognitionResult;

// Activity detection
typedef enum {
    ACTIVITY_IDLE = 0,
    ACTIVITY_TYPING = 1,
    ACTIVITY_CLICKING = 2,
    ACTIVITY_SCROLLING = 3,
    ACTIVITY_DRAGGING = 4,
    ACTIVITY_WINDOW_SWITCH = 5,
    ACTIVITY_MENU_OPEN = 6
} UserActivity;

typedef struct {
    UserActivity activity;
    RECT focus_area;
    uint64_t duration_ms;
    float intensity;
} ActivityInfo;

// Program understanding
typedef struct {
    HWND window_handle;
    char* title;
    char* class_name;
    char* process_name;
    RECT bounds;
    bool is_active;
    bool is_minimized;
    void* control_tree; // UI control hierarchy
} ProgramState;

typedef struct {
    ProgramState* programs;
    uint32_t count;
    uint32_t active_program_index;
} SystemProgramsState;

// Input synthesis
typedef enum {
    INPUT_TYPE_MOUSE_MOVE = 0,
    INPUT_TYPE_MOUSE_CLICK = 1,
    INPUT_TYPE_KEY_PRESS = 2,
    INPUT_TYPE_TEXT_INPUT = 3,
    INPUT_TYPE_SCROLL = 4
} InputType;

typedef struct {
    InputType type;
    union {
        struct {
            int x, y;
            bool relative;
        } mouse_move;
        struct {
            int button; // 0=left, 1=right, 2=middle
            bool double_click;
        } mouse_click;
        struct {
            uint32_t key_code;
            bool extended;
        } key_press;
        struct {
            char* text;
            uint32_t length;
        } text_input;
        struct {
            int delta_x, delta_y;
        } scroll;
    } data;
    uint32_t delay_ms; // Delay before this input
} SynthesizedInput;

// Human-like behavior
typedef struct {
    float base_delay_ms;
    float randomness_factor;
    float acceleration_curve;
    bool add_micro_movements;
    bool simulate_fatigue;
} HumanBehaviorProfile;

// Screen control system
typedef struct {
    // Screen capture
    ScreenFrame current_frame;
    void* capture_context;

    // Analysis results
    ObjectDetectionResult objects;
    TextRecognitionResult texts;
    ActivityInfo current_activity;

    // Program understanding
    SystemProgramsState programs;

    // Input synthesis
    HumanBehaviorProfile human_profile;
    SynthesizedInput* input_queue;
    uint32_t queue_size;
    uint32_t queue_capacity;

    // Control state
    bool is_controlling;
    HWND target_window;
    RECT control_bounds;

    // Internal state
    CRITICAL_SECTION lock;
    bool initialized;
    HANDLE analysis_thread;
    HANDLE input_thread;
} ScreenControlSystem;

#ifdef __cplusplus
extern "C" {
#endif

// Core API functions (context-style: allocate and set *context)
NTSTATUS ScreenControl_Initialize(void** screen_context);
NTSTATUS ScreenControl_Shutdown(void* screen_context);
NTSTATUS ScreenControl_CaptureScreen(ScreenControlSystem* system, ScreenFrame* frame);
NTSTATUS ScreenControl_AnalyzeFrame(ScreenControlSystem* system, const ScreenFrame* frame);
NTSTATUS ScreenControl_DetectObjects(ScreenControlSystem* system, ObjectDetectionResult* result);
NTSTATUS ScreenControl_RecognizeText(ScreenControlSystem* system, TextRecognitionResult* result);
NTSTATUS ScreenControl_DetectActivity(ScreenControlSystem* system, ActivityInfo* activity);
NTSTATUS ScreenControl_UnderstandPrograms(ScreenControlSystem* system, SystemProgramsState* programs);

// Control functions
NTSTATUS ScreenControl_SetTargetProgram(ScreenControlSystem* system, const char* program_name);
NTSTATUS ScreenControl_QueueInput(ScreenControlSystem* system, const SynthesizedInput* input);
NTSTATUS ScreenControl_ExecuteInputQueue(ScreenControlSystem* system);
NTSTATUS ScreenControl_WaitForResult(ScreenControlSystem* system, const char* expected_state, uint32_t timeout_ms);

// Human-like input generation
NTSTATUS GenerateHumanMouseMove(ScreenControlSystem* system, int start_x, int start_y, int end_x, int end_y, SynthesizedInput* inputs, uint32_t* count);
NTSTATUS GenerateHumanClick(ScreenControlSystem* system, int x, int y, int button, SynthesizedInput* inputs, uint32_t* count);
NTSTATUS GenerateHumanTyping(ScreenControlSystem* system, const char* text, SynthesizedInput* inputs, uint32_t* count);

// Utility functions
NTSTATUS GetWindowUnderCursor(HWND* window, char* title, size_t title_size);
NTSTATUS GetProgramState(HWND window, ProgramState* state);
NTSTATUS FindUIElement(HWND window, const char* element_description, RECT* bounds);
bool IsPointInRect(int x, int y, const RECT* rect);
double GetDistance(int x1, int y1, int x2, int y2);

// Memory management
NTSTATUS AllocateScreenMemory(size_t size, void** buffer);
void FreeScreenMemory(void* buffer);

#ifdef __cplusplus
}
#endif

#endif // SCREEN_CONTROL_H