#include <windows.h>
#include "raijin_ntstatus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

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
#ifndef STATUS_NOT_FOUND
#define STATUS_NOT_FOUND ((NTSTATUS)0xC0000225L)
#endif

// Raijin Screen Control System
// Complete Visual Interface Domination
// Version 1.0 - Visual Consciousness

// Screen control structure
struct ScreenControlSystem {
    HDC screen_dc;
    int screen_width;
    int screen_height;
    int mouse_x;
    int mouse_y;
    BOOL is_initialized;
    char* target_program_name;
};

// Global screen control instance
static ScreenControlSystem* g_screen_system = NULL;

extern "C" NTSTATUS ScreenControl_Initialize(void** screen_context) {
    if (!screen_context) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)malloc(sizeof(ScreenControlSystem));
    if (!screen) return STATUS_INSUFFICIENT_RESOURCES;

    memset(screen, 0, sizeof(ScreenControlSystem));

    // Get screen dimensions
    screen->screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen->screen_height = GetSystemMetrics(SM_CYSCREEN);

    // Get screen device context
    screen->screen_dc = GetDC(NULL);
    if (!screen->screen_dc) {
        free(screen);
        return STATUS_UNSUCCESSFUL;
    }

    // Get initial mouse position
    POINT mouse_pos;
    if (GetCursorPos(&mouse_pos)) {
        screen->mouse_x = mouse_pos.x;
        screen->mouse_y = mouse_pos.y;
    }

    screen->is_initialized = TRUE;
    *screen_context = screen;

    printf("Screen Control initialized: %dx%d resolution\n", screen->screen_width, screen->screen_height);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_Shutdown(void* screen_context) {
    if (!screen_context) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    if (screen->target_program_name) {
        free(screen->target_program_name);
        screen->target_program_name = NULL;
    }

    if (screen->screen_dc) {
        ReleaseDC(NULL, screen->screen_dc);
        screen->screen_dc = NULL;
    }

    free(screen);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_CaptureScreen(void* screen_context, void** image_data, size_t* data_size) {
    if (!screen_context || !image_data || !data_size) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    if (!screen->is_initialized) return STATUS_UNSUCCESSFUL;

    // Create compatible DC and bitmap for screen capture
    HDC mem_dc = CreateCompatibleDC(screen->screen_dc);
    if (!mem_dc) return STATUS_UNSUCCESSFUL;

    HBITMAP bitmap = CreateCompatibleBitmap(screen->screen_dc, screen->screen_width, screen->screen_height);
    if (!bitmap) {
        DeleteDC(mem_dc);
        return STATUS_UNSUCCESSFUL;
    }

    HBITMAP old_bitmap = (HBITMAP)SelectObject(mem_dc, bitmap);

    // Copy screen to memory DC
    if (!BitBlt(mem_dc, 0, 0, screen->screen_width, screen->screen_height,
                screen->screen_dc, 0, 0, SRCCOPY)) {
        SelectObject(mem_dc, old_bitmap);
        DeleteObject(bitmap);
        DeleteDC(mem_dc);
        return STATUS_UNSUCCESSFUL;
    }

    // Calculate bitmap data size
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = screen->screen_width;
    bmi.bmiHeader.biHeight = -screen->screen_height; // Negative for top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    size_t bitmap_size = screen->screen_width * screen->screen_height * 4; // 32-bit RGBA

    void* bitmap_data = malloc(bitmap_size);
    if (!bitmap_data) {
        SelectObject(mem_dc, old_bitmap);
        DeleteObject(bitmap);
        DeleteDC(mem_dc);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Get bitmap data
    if (!GetDIBits(mem_dc, bitmap, 0, screen->screen_height, bitmap_data, &bmi, DIB_RGB_COLORS)) {
        free(bitmap_data);
        SelectObject(mem_dc, old_bitmap);
        DeleteObject(bitmap);
        DeleteDC(mem_dc);
        return STATUS_UNSUCCESSFUL;
    }

    // Cleanup GDI objects
    SelectObject(mem_dc, old_bitmap);
    DeleteObject(bitmap);
    DeleteDC(mem_dc);

    *image_data = bitmap_data;
    *data_size = bitmap_size;

    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_MoveMouse(void* screen_context, int x, int y) {
    if (!screen_context) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    if (!screen->is_initialized) return STATUS_UNSUCCESSFUL;

    // Clamp coordinates to screen bounds
    x = std::max(0, std::min(x, screen->screen_width - 1));
    y = std::max(0, std::min(y, screen->screen_height - 1));

    // Move mouse cursor
    if (!SetCursorPos(x, y)) {
        return STATUS_UNSUCCESSFUL;
    }

    screen->mouse_x = x;
    screen->mouse_y = y;

    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_ClickMouse(void* screen_context, int button) {
    if (!screen_context) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    if (!screen->is_initialized) return STATUS_UNSUCCESSFUL;

    DWORD mouse_down, mouse_up;

    switch (button) {
        case 0: // Left button
            mouse_down = MOUSEEVENTF_LEFTDOWN;
            mouse_up = MOUSEEVENTF_LEFTUP;
            break;
        case 1: // Right button
            mouse_down = MOUSEEVENTF_RIGHTDOWN;
            mouse_up = MOUSEEVENTF_RIGHTUP;
            break;
        case 2: // Middle button
            mouse_down = MOUSEEVENTF_MIDDLEDOWN;
            mouse_up = MOUSEEVENTF_MIDDLEUP;
            break;
        default:
            return STATUS_INVALID_PARAMETER;
    }

    // Send mouse down event
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = mouse_down;

    if (SendInput(1, &input, sizeof(INPUT)) != 1) {
        return STATUS_UNSUCCESSFUL;
    }

    // Small delay
    Sleep(10);

    // Send mouse up event
    input.mi.dwFlags = mouse_up;

    if (SendInput(1, &input, sizeof(INPUT)) != 1) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

// Additional screen control functions

extern "C" NTSTATUS ScreenControl_GetMousePosition(void* screen_context, int* x, int* y) {
    if (!screen_context || !x || !y) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    if (!screen->is_initialized) return STATUS_UNSUCCESSFUL;

    POINT mouse_pos;
    if (!GetCursorPos(&mouse_pos)) {
        return STATUS_UNSUCCESSFUL;
    }

    screen->mouse_x = mouse_pos.x;
    screen->mouse_y = mouse_pos.y;

    *x = mouse_pos.x;
    *y = mouse_pos.y;

    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_TypeText(void* screen_context, const char* text) {
    if (!screen_context || !text) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    if (!screen->is_initialized) return STATUS_UNSUCCESSFUL;

    size_t len = strlen(text);
    INPUT* inputs = (INPUT*)malloc(sizeof(INPUT) * len * 2); // Key down and key up for each character

    if (!inputs) return STATUS_INSUFFICIENT_RESOURCES;

    int input_count = 0;

    for (size_t i = 0; i < len; i++) {
        // Key down
        inputs[input_count].type = INPUT_KEYBOARD;
        inputs[input_count].ki.wVk = 0;
        inputs[input_count].ki.wScan = text[i];
        inputs[input_count].ki.dwFlags = KEYEVENTF_UNICODE;
        input_count++;

        // Key up
        inputs[input_count].type = INPUT_KEYBOARD;
        inputs[input_count].ki.wVk = 0;
        inputs[input_count].ki.wScan = text[i];
        inputs[input_count].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        input_count++;
    }

    UINT sent = SendInput(input_count, inputs, sizeof(INPUT));
    free(inputs);

    if (sent != (UINT)input_count) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_CaptureWindow(void* screen_context, HWND window_handle, void** image_data, size_t* data_size) {
    if (!screen_context || !window_handle || !image_data || !data_size) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    if (!screen->is_initialized) return STATUS_UNSUCCESSFUL;

    // Get window dimensions
    RECT window_rect;
    if (!GetWindowRect(window_handle, &window_rect)) {
        return STATUS_UNSUCCESSFUL;
    }

    int width = window_rect.right - window_rect.left;
    int height = window_rect.bottom - window_rect.top;

    if (width <= 0 || height <= 0) return STATUS_INVALID_PARAMETER;

    // Create compatible DC and bitmap
    HDC window_dc = GetWindowDC(window_handle);
    if (!window_dc) return STATUS_UNSUCCESSFUL;

    HDC mem_dc = CreateCompatibleDC(window_dc);
    if (!mem_dc) {
        ReleaseDC(window_handle, window_dc);
        return STATUS_UNSUCCESSFUL;
    }

    HBITMAP bitmap = CreateCompatibleBitmap(window_dc, width, height);
    if (!bitmap) {
        DeleteDC(mem_dc);
        ReleaseDC(window_handle, window_dc);
        return STATUS_UNSUCCESSFUL;
    }

    HBITMAP old_bitmap = (HBITMAP)SelectObject(mem_dc, bitmap);

    // Copy window to memory DC
    if (!BitBlt(mem_dc, 0, 0, width, height, window_dc, 0, 0, SRCCOPY)) {
        SelectObject(mem_dc, old_bitmap);
        DeleteObject(bitmap);
        DeleteDC(mem_dc);
        ReleaseDC(window_handle, window_dc);
        return STATUS_UNSUCCESSFUL;
    }

    // Get bitmap data (similar to screen capture)
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    size_t bitmap_size = width * height * 4;
    void* bitmap_data = malloc(bitmap_size);

    if (!bitmap_data) {
        SelectObject(mem_dc, old_bitmap);
        DeleteObject(bitmap);
        DeleteDC(mem_dc);
        ReleaseDC(window_handle, window_dc);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!GetDIBits(mem_dc, bitmap, 0, height, bitmap_data, &bmi, DIB_RGB_COLORS)) {
        free(bitmap_data);
        SelectObject(mem_dc, old_bitmap);
        DeleteObject(bitmap);
        DeleteDC(mem_dc);
        ReleaseDC(window_handle, window_dc);
        return STATUS_UNSUCCESSFUL;
    }

    // Cleanup
    SelectObject(mem_dc, old_bitmap);
    DeleteObject(bitmap);
    DeleteDC(mem_dc);
    ReleaseDC(window_handle, window_dc);

    *image_data = bitmap_data;
    *data_size = bitmap_size;

    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_FindWindowByTitle(void* screen_context, const char* title, HWND* window_handle) {
    if (!screen_context || !title || !window_handle) return STATUS_INVALID_PARAMETER;

    *window_handle = FindWindowA(NULL, title);

    if (*window_handle == NULL) {
        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_SetTargetProgram(void* screen_context, const char* program_name) {
    if (!screen_context || !program_name) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    if (screen->target_program_name) {
        free(screen->target_program_name);
        screen->target_program_name = NULL;
    }

    size_t len = strlen(program_name) + 1;
    screen->target_program_name = (char*)malloc(len);
    if (!screen->target_program_name) return STATUS_INSUFFICIENT_RESOURCES;

    strcpy_s(screen->target_program_name, len, program_name);
    return STATUS_SUCCESS;
}

extern "C" NTSTATUS ScreenControl_GetScreenDimensions(void* screen_context, int* width, int* height) {
    if (!screen_context || !width || !height) return STATUS_INVALID_PARAMETER;

    ScreenControlSystem* screen = (ScreenControlSystem*)screen_context;

    *width = screen->screen_width;
    *height = screen->screen_height;

    return STATUS_SUCCESS;
}