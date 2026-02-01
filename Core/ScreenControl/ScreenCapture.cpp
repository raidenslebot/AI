// ScreenCapture.cpp - DirectX-based screen capture for Raijin
#include "screen_capture.h"
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <time.h>
#include <stdio.h>

// DirectX interface pointers (hidden from header)
typedef struct {
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGIOutput* output;
    IDXGIOutput1* output1;
    IDXGIOutputDuplication* duplication;
    ID3D11Texture2D* texture;
    DXGI_OUTDUPL_DESC desc;
} DXGIResources;

// Memory management helpers
NTSTATUS AllocateCaptureMemory(size_t size, void** buffer) {
    *buffer = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (*buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    return STATUS_SUCCESS;
}

void FreeCaptureMemory(void* buffer) {
    if (buffer) {
        VirtualFree(buffer, 0, MEM_RELEASE);
    }
}

// Check if screen capture is supported on this system
bool ScreenCapture_IsSupported(void) {
    HMODULE dxgi = LoadLibraryA("dxgi.dll");
    if (!dxgi) return false;

    // Check for DXGI 1.2 support (required for desktop duplication)
    typedef HRESULT(WINAPI* CreateDXGIFactory1Func)(REFIID, void**);
    CreateDXGIFactory1Func createFactory = (CreateDXGIFactory1Func)GetProcAddress(dxgi, "CreateDXGIFactory1");

    if (!createFactory) {
        FreeLibrary(dxgi);
        return false;
    }

    IDXGIFactory1* factory = NULL;
    HRESULT hr = createFactory(__uuidof(IDXGIFactory1), (void**)&factory);

    if (FAILED(hr)) {
        FreeLibrary(dxgi);
        return false;
    }

    // Enumerate adapters and check for outputs
    IDXGIAdapter* adapter = NULL;
    bool supported = false;

    for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
        IDXGIOutput* output = NULL;
        if (SUCCEEDED(adapter->EnumOutputs(0, &output))) {
            // Check if output supports duplication
            IDXGIOutput1* output1 = NULL;
            if (SUCCEEDED(output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1))) {
                output1->Release();
                supported = true;
            }
            output->Release();
        }
        adapter->Release();
        if (supported) break;
    }

    factory->Release();
    FreeLibrary(dxgi);
    return supported;
}

// Get adapter information
NTSTATUS ScreenCapture_GetAdapterInfo(char* info, size_t info_size) {
    if (!info) return STATUS_INVALID_PARAMETER;

    HMODULE dxgi = LoadLibraryA("dxgi.dll");
    if (!dxgi) return STATUS_UNSUCCESSFUL;

    typedef HRESULT(WINAPI* CreateDXGIFactory1Func)(REFIID, void**);
    CreateDXGIFactory1Func createFactory = (CreateDXGIFactory1Func)GetProcAddress(dxgi, "CreateDXGIFactory1");

    if (!createFactory) {
        FreeLibrary(dxgi);
        return STATUS_UNSUCCESSFUL;
    }

    IDXGIFactory1* factory = NULL;
    HRESULT hr = createFactory(__uuidof(IDXGIFactory1), (void**)&factory);

    if (FAILED(hr)) {
        FreeLibrary(dxgi);
        return STATUS_UNSUCCESSFUL;
    }

    sprintf_s(info, info_size, "DirectX Screen Capture Support:\n");

    IDXGIAdapter* adapter = NULL;
    char buffer[1024] = {0};

    for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
        DXGI_ADAPTER_DESC desc;
        if (SUCCEEDED(adapter->GetDesc(&desc))) {
            sprintf_s(buffer, sizeof(buffer), "Adapter %u: %ls (VRAM: %llu MB)\n",
                     i, desc.Description, desc.DedicatedVideoMemory / (1024 * 1024));
            strncat_s(info, info_size, buffer, _TRUNCATE);
        }

        IDXGIOutput* output = NULL;
        if (SUCCEEDED(adapter->EnumOutputs(0, &output))) {
            DXGI_OUTPUT_DESC outDesc;
            if (SUCCEEDED(output->GetDesc(&outDesc))) {
                sprintf_s(buffer, sizeof(buffer), "  Output: %ls (%ux%u)\n",
                         outDesc.DeviceName, outDesc.DesktopCoordinates.right - outDesc.DesktopCoordinates.left,
                         outDesc.DesktopCoordinates.bottom - outDesc.DesktopCoordinates.top);
                strncat_s(info, info_size, buffer, _TRUNCATE);
            }
            output->Release();
        }
        adapter->Release();
    }

    factory->Release();
    FreeLibrary(dxgi);
    return STATUS_SUCCESS;
}

// Initialize screen capture system
NTSTATUS ScreenCapture_Initialize(ScreenCapture** capture, const CaptureConfig* config) {
    if (!capture) return STATUS_INVALID_PARAMETER;

    *capture = (ScreenCapture*)malloc(sizeof(ScreenCapture));
    if (!*capture) return STATUS_INSUFFICIENT_RESOURCES;

    memset(*capture, 0, sizeof(ScreenCapture));

    // Set default configuration if none provided
    if (config) {
        memcpy(&(*capture)->config, config, sizeof(CaptureConfig));
    } else {
        (*capture)->config.target_fps = 30;
        (*capture)->config.capture_cursor = true;
        (*capture)->config.high_quality = true;
        (*capture)->config.max_width = 1920;
        (*capture)->config.max_height = 1080;
    }

    // Allocate DXGI resources
    DXGIResources* dxgi = (DXGIResources*)malloc(sizeof(DXGIResources));
    if (!dxgi) {
        free(*capture);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    memset(dxgi, 0, sizeof(DXGIResources));

    // Create D3D11 device
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags,
                                  featureLevels, ARRAYSIZE(featureLevels),
                                  D3D11_SDK_VERSION, &dxgi->device, NULL, &dxgi->context);

    if (FAILED(hr)) {
        free(dxgi);
        free(*capture);
        return STATUS_UNSUCCESSFUL;
    }

    // Get DXGI device
    IDXGIDevice* dxgiDevice = NULL;
    hr = dxgi->device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) {
        dxgi->context->Release();
        dxgi->device->Release();
        free(dxgi);
        free(*capture);
        return STATUS_UNSUCCESSFUL;
    }

    // Get DXGI adapter
    IDXGIAdapter* adapter = NULL;
    hr = dxgiDevice->GetAdapter(&adapter);
    dxgiDevice->Release();

    if (FAILED(hr)) {
        dxgi->context->Release();
        dxgi->device->Release();
        free(dxgi);
        free(*capture);
        return STATUS_UNSUCCESSFUL;
    }

    // Get primary output
    hr = adapter->EnumOutputs(0, &dxgi->output);
    adapter->Release();

    if (FAILED(hr)) {
        dxgi->context->Release();
        dxgi->device->Release();
        free(dxgi);
        free(*capture);
        return STATUS_UNSUCCESSFUL;
    }

    // Get output1 interface for duplication
    hr = dxgi->output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&dxgi->output1);
    if (FAILED(hr)) {
        dxgi->output->Release();
        dxgi->context->Release();
        dxgi->device->Release();
        free(dxgi);
        free(*capture);
        return STATUS_UNSUCCESSFUL;
    }

    // Create desktop duplication
    hr = dxgi->output1->DuplicateOutput(dxgi->device, &dxgi->duplication);
    if (FAILED(hr)) {
        dxgi->output1->Release();
        dxgi->output->Release();
        dxgi->context->Release();
        dxgi->device->Release();
        free(dxgi);
        free(*capture);
        return STATUS_UNSUCCESSFUL;
    }

    // Get duplication description
    dxgi->duplication->GetDesc(&dxgi->desc);

    // Store DXGI resources
    (*capture)->dxgi_device = dxgi->device;
    (*capture)->dxgi_context = dxgi->context;
    (*capture)->dxgi_output = dxgi->output;
    (*capture)->dxgi_output1 = dxgi->output1;
    (*capture)->dxgi_duplication = dxgi->duplication;

    (*capture)->initialized = true;

    // Free temporary structure (resources are now stored in capture)
    free(dxgi);

    return STATUS_SUCCESS;
}

// Shutdown screen capture system
NTSTATUS ScreenCapture_Shutdown(ScreenCapture* capture) {
    if (!capture) return STATUS_INVALID_PARAMETER;

    if (capture->dxgi_duplication) {
        ((IDXGIOutputDuplication*)capture->dxgi_duplication)->Release();
    }

    if (capture->dxgi_output1) {
        ((IDXGIOutput1*)capture->dxgi_output1)->Release();
    }

    if (capture->dxgi_output) {
        ((IDXGIOutput*)capture->dxgi_output)->Release();
    }

    if (capture->dxgi_context) {
        ((ID3D11DeviceContext*)capture->dxgi_context)->Release();
    }

    if (capture->dxgi_device) {
        ((ID3D11Device*)capture->dxgi_device)->Release();
    }

    free(capture);
    return STATUS_SUCCESS;
}

// Capture a single frame
NTSTATUS ScreenCapture_CaptureFrame(ScreenCapture* capture, CaptureResult* result) {
    if (!capture || !result || !capture->initialized) {
        return STATUS_INVALID_PARAMETER;
    }

    IDXGIOutputDuplication* duplication = (IDXGIOutputDuplication*)capture->dxgi_duplication;
    ID3D11DeviceContext* context = (ID3D11DeviceContext*)capture->dxgi_context;
    ID3D11Device* device = (ID3D11Device*)capture->dxgi_device;

    uint64_t start_time = GetTickCount64();
    capture->stats.captures_attempted++;

    // Try to acquire next frame
    IDXGIResource* resource = NULL;
    DXGI_OUTDUPL_FRAME_INFO frameInfo = {0};

    HRESULT hr = duplication->AcquireNextFrame(100, &frameInfo, &resource);

    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        // No new frame available
        result->success = false;
        return STATUS_SUCCESS;
    }

    if (FAILED(hr)) {
        result->success = false;
        return STATUS_UNSUCCESSFUL;
    }

    // Get the texture from the resource
    ID3D11Texture2D* texture = NULL;
    hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);
    resource->Release();

    if (FAILED(hr)) {
        duplication->ReleaseFrame();
        result->success = false;
        return STATUS_UNSUCCESSFUL;
    }

    // Get texture description
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    // Create a readable texture for CPU access
    D3D11_TEXTURE2D_DESC readableDesc = desc;
    readableDesc.Usage = D3D11_USAGE_STAGING;
    readableDesc.BindFlags = 0;
    readableDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    readableDesc.MiscFlags = 0;

    ID3D11Texture2D* readableTexture = NULL;
    hr = device->CreateTexture2D(&readableDesc, NULL, &readableTexture);

    if (FAILED(hr)) {
        texture->Release();
        duplication->ReleaseFrame();
        result->success = false;
        return STATUS_UNSUCCESSFUL;
    }

    // Copy texture to readable format
    context->CopyResource(readableTexture, texture);

    // Map the texture for CPU access
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = context->Map(readableTexture, 0, D3D11_MAP_READ, 0, &mapped);

    if (FAILED(hr)) {
        readableTexture->Release();
        texture->Release();
        duplication->ReleaseFrame();
        result->success = false;
        return STATUS_UNSUCCESSFUL;
    }

    // Allocate memory for pixel data
    size_t dataSize = desc.Height * mapped.RowPitch;
    void* pixelData = NULL;

    if (AllocateCaptureMemory(dataSize, &pixelData) != STATUS_SUCCESS) {
        context->Unmap(readableTexture, 0);
        readableTexture->Release();
        texture->Release();
        duplication->ReleaseFrame();
        result->success = false;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Copy pixel data
    memcpy(pixelData, mapped.pData, dataSize);

    // Unmap and release resources
    context->Unmap(readableTexture, 0);
    readableTexture->Release();
    texture->Release();
    duplication->ReleaseFrame();

    // Fill result structure
    result->pixel_data = pixelData;
    result->width = desc.Width;
    result->height = desc.Height;
    result->stride = mapped.RowPitch;
    result->timestamp = GetTickCount64();
    result->success = true;

    // Update statistics
    capture->stats.captures_successful++;
    capture->stats.bytes_captured += dataSize;

    uint64_t capture_time = GetTickCount64() - start_time;
    if (capture->stats.captures_successful == 1) {
        capture->stats.average_capture_time_ms = capture_time;
    } else {
        capture->stats.average_capture_time_ms =
            (capture->stats.average_capture_time_ms * (capture->stats.captures_successful - 1) + capture_time) /
            capture->stats.captures_successful;
    }

    return STATUS_SUCCESS;
}

// Get screen information
NTSTATUS ScreenCapture_GetScreenInfo(ScreenCapture* capture, uint32_t* width, uint32_t* height) {
    if (!capture || !width || !height || !capture->initialized) {
        return STATUS_INVALID_PARAMETER;
    }

    IDXGIOutputDuplication* duplication = (IDXGIOutputDuplication*)capture->dxgi_duplication;
    DXGI_OUTDUPL_DESC desc;
    duplication->GetDesc(&desc);

    *width = desc.ModeDesc.Width;
    *height = desc.ModeDesc.Height;

    return STATUS_SUCCESS;
}

// Configure capture settings
NTSTATUS ScreenCapture_Configure(ScreenCapture* capture, const CaptureConfig* config) {
    if (!capture || !config) return STATUS_INVALID_PARAMETER;

    memcpy(&capture->config, config, sizeof(CaptureConfig));
    return STATUS_SUCCESS;
}

// Get capture statistics
NTSTATUS ScreenCapture_GetStatistics(ScreenCapture* capture, CaptureStats* stats) {
    if (!capture || !stats) return STATUS_INVALID_PARAMETER;

    memcpy(stats, &capture->stats, sizeof(CaptureStats));
    return STATUS_SUCCESS;
}

// Test capture functionality
NTSTATUS ScreenCapture_TestCapture(ScreenCapture* capture) {
    if (!capture || !capture->initialized) return STATUS_INVALID_PARAMETER;

    CaptureResult result = {0};
    NTSTATUS status = ScreenCapture_CaptureFrame(capture, &result);

    if (result.success && result.pixel_data) {
        FreeCaptureMemory(result.pixel_data);
    }

    return status;
}
