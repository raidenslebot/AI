#ifndef SCREEN_CAPTURE_H
#define SCREEN_CAPTURE_H

#include <windows.h>
#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stdbool.h>

// Raijin Screen Capture System
// DirectX-based screen observation and analysis

// Pixel data structure
typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} Pixel;

// Screen capture result
typedef struct {
    void* pixel_data;       // Raw pixel data (BGRA format)
    uint32_t width;         // Screen width
    uint32_t height;        // Screen height
    uint32_t stride;        // Bytes per row
    uint64_t timestamp;     // Capture timestamp
    bool success;           // Capture success flag
} CaptureResult;

// Screen capture statistics
typedef struct {
    uint64_t captures_attempted;
    uint64_t captures_successful;
    uint64_t bytes_captured;
    uint64_t average_capture_time_ms;
    uint32_t current_fps;
} CaptureStats;

// Screen capture configuration
typedef struct {
    uint32_t target_fps;            // Target capture rate
    bool capture_cursor;            // Include mouse cursor
    bool high_quality;              // High quality mode
    uint32_t max_width;             // Maximum capture width
    uint32_t max_height;            // Maximum capture height
} CaptureConfig;

// Main screen capture interface
typedef struct {
    void* dxgi_device;              // DirectX device
    void* dxgi_context;             // DirectX device context
    void* dxgi_output;              // DXGI output
    void* dxgi_output1;             // DXGI output 1
    void* dxgi_duplication;         // Desktop duplication
    CaptureConfig config;           // Capture configuration
    CaptureStats stats;             // Performance statistics
    bool initialized;               // Initialization flag
} ScreenCapture;

// Core screen capture functions
NTSTATUS ScreenCapture_Initialize(ScreenCapture** capture, const CaptureConfig* config);
NTSTATUS ScreenCapture_Shutdown(ScreenCapture* capture);
NTSTATUS ScreenCapture_CaptureFrame(ScreenCapture* capture, CaptureResult* result);
NTSTATUS ScreenCapture_GetScreenInfo(ScreenCapture* capture, uint32_t* width, uint32_t* height);

// Configuration and control functions
NTSTATUS ScreenCapture_Configure(ScreenCapture* capture, const CaptureConfig* config);
NTSTATUS ScreenCapture_GetStatistics(ScreenCapture* capture, CaptureStats* stats);
NTSTATUS ScreenCapture_TestCapture(ScreenCapture* capture);

// Memory management for capture results
NTSTATUS AllocateCaptureMemory(size_t size, void** buffer);
void FreeCaptureMemory(void* buffer);

// Utility functions
bool ScreenCapture_IsSupported(void);
NTSTATUS ScreenCapture_GetAdapterInfo(char* info, size_t info_size);

#endif // SCREEN_CAPTURE_H