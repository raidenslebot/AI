// hal.h - Hardware Abstraction Layer Header
// Raijin ECGI Hardware Interface
// Version 1.0 - Complete Hardware Supremacy

#ifndef RAIJIN_HAL_H
#define RAIJIN_HAL_H

#include <windows.h>
#include <stdint.h>

// Windows NT status type
typedef LONG NTSTATUS;

// Standard integer types
typedef uint64_t uint64_t;
typedef uint32_t uint32_t;
typedef uint16_t uint16_t;
typedef uint8_t uint8_t;

// NT_SUCCESS macro
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

// CPU Features structure
typedef struct {
    BOOLEAN mmx;
    BOOLEAN sse;
    BOOLEAN sse2;
    BOOLEAN sse3;
    BOOLEAN ssse3;
    BOOLEAN sse4_1;
    BOOLEAN sse4_2;
    BOOLEAN avx;
    BOOLEAN avx2;
    BOOLEAN avx512_f;
    BOOLEAN avx512_dq;
    BOOLEAN avx512_ifma;
    BOOLEAN avx512_vbmi;
    BOOLEAN avx512_vbmi2;
    BOOLEAN avx512_vnni;
    BOOLEAN avx512_bitalg;
    BOOLEAN avx512_vpopcntdq;
    BOOLEAN amx_bf16;
    BOOLEAN amx_tile;
    BOOLEAN amx_int8;
    BOOLEAN fma;
    BOOLEAN fma4;
    BOOLEAN xop;
    BOOLEAN x64;
} CPUFeatures;

// Windows NT status type
typedef LONG NTSTATUS;

// Hardware Information Structures
typedef struct {
    WCHAR processor_name[256];
    DWORD processor_cores;
    ULONGLONG processor_frequency_hz;
    ULONGLONG total_memory_bytes;
    char os_version[128];
    char architecture[32];
    DWORD display_count;
    DWORD display_width;
    DWORD display_height;
} HardwareInfo;

// CPU Register State
typedef struct {
    UINT64 rax, rbx, rcx, rdx;
    UINT64 rsi, rdi, rsp, rbp;
    UINT64 r8, r9, r10, r11, r12, r13, r14, r15;
    UINT64 rip, rflags;
    UINT64 cr0, cr2, cr3, cr4;
    UINT64 dr0, dr1, dr2, dr3, dr6, dr7;
    UINT64 xmm0[2], xmm1[2], xmm2[2], xmm3[2];
    UINT64 xmm4[2], xmm5[2], xmm6[2], xmm7[2];
    UINT64 xmm8[2], xmm9[2], xmm10[2], xmm11[2];
    UINT64 xmm12[2], xmm13[2], xmm14[2], xmm15[2];
} CPURegisterState;

// MSR Access Structure
typedef struct {
    UINT32 index;
    UINT64 value;
    BOOLEAN write;
} MSRAccess;

// I/O Port Access
typedef struct {
    UINT16 port;
    UINT32 value;
    BOOLEAN write;
    UINT8 size; // 1, 2, or 4 bytes
} IOPortAccess;

// PCI Configuration Access
typedef struct {
    UINT8 bus;
    UINT8 device;
    UINT8 function;
    UINT32 register_offset;
    UINT32 value;
    BOOLEAN write;
} PCIConfigAccess;

// Physical Memory Access
typedef struct {
    UINT64 physical_address;
    PVOID virtual_address;
    SIZE_T size;
    BOOLEAN write;
} PhysicalMemoryAccess;

// Firmware Access
typedef struct {
    UINT32 firmware_type;
    PVOID buffer;
    SIZE_T size;
    BOOLEAN write;
} FirmwareAccess;

// Interrupt Handling
typedef struct {
    UINT32 interrupt_number;
    PVOID handler;
    BOOLEAN enable;
} InterruptConfig;

// Hardware Breakpoint
typedef struct {
    UINT32 breakpoint_number;
    UINT64 address;
    UINT32 type; // 0=execute, 1=write, 2=read/write, 3=I/O
    UINT32 size; // 1, 2, 4, or 8 bytes
} HardwareBreakpoint;

// Performance Counter
typedef struct {
    UINT32 counter_index;
    UINT64 value;
    char name[64];
} PerformanceCounter;

// Temperature Sensor
typedef struct {
    char name[32];
    float temperature_celsius;
    BOOLEAN critical;
} TemperatureSensor;

// Power Monitor
typedef struct {
    char name[32];
    float voltage_volts;
    float current_amps;
    float power_watts;
} PowerMonitor;

// HAL API Functions

// Initialization and Shutdown
NTSTATUS HAL_Initialize(void** hal_context);
NTSTATUS HAL_Shutdown(void* hal_context);

// System Information
NTSTATUS HAL_GetSystemInfo(void* hal_context, char* info, size_t size);

// CPU Operations
NTSTATUS HAL_GetCPUState(void* hal_context, CPURegisterState* state);
NTSTATUS HAL_SetCPUState(void* hal_context, const CPURegisterState* state);
NTSTATUS HAL_GetCPUTemperature(void* hal_context, float* temperature);
NTSTATUS HAL_GetCPUUsage(void* hal_context, float* usage_percent);
NTSTATUS HAL_SetCPUFrequency(void* hal_context, UINT64 frequency_hz);
NTSTATUS HAL_GetSupportedFrequencies(void* hal_context, UINT64* frequencies, UINT32* count);
NTSTATUS HAL_EnableTurboBoost(void* hal_context, BOOLEAN enable);
NTSTATUS HAL_FlushCache(void* hal_context);
NTSTATUS HAL_InvalidateTLB(void* hal_context);

// Memory Operations
NTSTATUS HAL_GetMemoryUsage(void* hal_context, UINT64* used_bytes, UINT64* total_bytes);
NTSTATUS HAL_AllocatePhysicalMemory(void* hal_context, SIZE_T size, UINT64* physical_address);
NTSTATUS HAL_FreePhysicalMemory(void* hal_context, UINT64 physical_address, SIZE_T size);
NTSTATUS HAL_MapPhysicalMemory(void* hal_context, UINT64 physical_address, SIZE_T size, PVOID* virtual_address);
NTSTATUS HAL_UnmapPhysicalMemory(void* hal_context, PVOID virtual_address, SIZE_T size);
NTSTATUS HAL_ReadPhysicalMemory(void* hal_context, UINT64 physical_address, PVOID buffer, SIZE_T size);
NTSTATUS HAL_WritePhysicalMemory(void* hal_context, UINT64 physical_address, const PVOID buffer, SIZE_T size);
NTSTATUS HAL_SetMemoryEncryption(void* hal_context, BOOLEAN enable);

// MSR Operations
NTSTATUS HAL_ReadMSR(void* hal_context, UINT32 msr_index, UINT64* value);
NTSTATUS HAL_WriteMSR(void* hal_context, UINT32 msr_index, UINT64 value);

// I/O Operations
NTSTATUS HAL_ReadIOPort(void* hal_context, UINT16 port, UINT32* value);
NTSTATUS HAL_WriteIOPort(void* hal_context, UINT16 port, UINT32 value);

// PCI Operations
NTSTATUS HAL_ReadPCIConfig(void* hal_context, UINT8 bus, UINT8 device, UINT8 function, UINT32 register_offset, UINT32* value);
NTSTATUS HAL_WritePCIConfig(void* hal_context, UINT8 bus, UINT8 device, UINT8 function, UINT32 register_offset, UINT32 value);

// Firmware Operations
NTSTATUS HAL_ReadFirmware(void* hal_context, UINT32 firmware_type, PVOID buffer, SIZE_T* size);
NTSTATUS HAL_WriteFirmware(void* hal_context, UINT32 firmware_type, const PVOID buffer, SIZE_T size);
NTSTATUS HAL_UpdateFirmware(void* hal_context, const PVOID firmware_image, SIZE_T size);

// Interrupt Operations
NTSTATUS HAL_EnableInterrupt(void* hal_context, UINT32 interrupt_number);
NTSTATUS HAL_DisableInterrupt(void* hal_context, UINT32 interrupt_number);
NTSTATUS HAL_SetInterruptHandler(void* hal_context, UINT32 interrupt_number, PVOID handler);

// Hardware Breakpoint Operations
NTSTATUS HAL_SetHardwareBreakpoint(void* hal_context, UINT32 breakpoint_number, UINT64 address, UINT32 type);
NTSTATUS HAL_ClearHardwareBreakpoint(void* hal_context, UINT32 breakpoint_number);
NTSTATUS HAL_GetDebugStatus(void* hal_context, UINT64* status);

// Ring -1 Operations (Hypervisor Level)
NTSTATUS HAL_EnableRingMinus1(void* hal_context);
NTSTATUS HAL_DisableRingMinus1(void* hal_context);
NTSTATUS HAL_IsRingMinus1Available(void* hal_context);
NTSTATUS HAL_ExecuteInRingMinus1(void* hal_context, PVOID code, SIZE_T code_size, PVOID parameters, PVOID result);
NTSTATUS HAL_ReadPhysicalMemoryRingMinus1(void* hal_context, UINT64 physical_address, PVOID buffer, SIZE_T size);
NTSTATUS HAL_WritePhysicalMemoryRingMinus1(void* hal_context, UINT64 physical_address, const PVOID buffer, SIZE_T size);
NTSTATUS HAL_ExecutePrivilegedInstructionRingMinus1(void* hal_context, UINT32 instruction, UINT64 operand, UINT64* result);

// Microcode Operations
NTSTATUS HAL_LoadMicrocodePatch(void* hal_context, const PVOID microcode, SIZE_T size);
NTSTATUS HAL_ApplyMicrocodePatch(void* hal_context);
NTSTATUS HAL_GetMicrocodeVersion(void* hal_context, UINT32* version);

// VMX Operations
NTSTATUS HAL_EnableVMX(void* hal_context);
NTSTATUS HAL_DisableVMX(void* hal_context);
NTSTATUS HAL_GetVMXCapabilities(void* hal_context, UINT64* capabilities);

// Security Operations
NTSTATUS HAL_EnableSecureBoot(void* hal_context);
NTSTATUS HAL_DisableSecureBoot(void* hal_context);
NTSTATUS HAL_MeasureSystemState(void* hal_context, UINT8* measurement, SIZE_T* size);

// Power Management
NTSTATUS HAL_SetPowerProfile(void* hal_context, UINT32 profile);

// Monitoring Functions
NTSTATUS HAL_GetTemperatureSensors(void* hal_context, TemperatureSensor* sensors, UINT32* count);
NTSTATUS HAL_GetPowerMonitors(void* hal_context, PowerMonitor* monitors, UINT32* count);
NTSTATUS HAL_GetPerformanceCounters(void* hal_context, PerformanceCounter* counters, UINT32* count);

// Utility Functions
const char* HAL_GetErrorString(NTSTATUS status);
UINT32 HAL_GetProcessorFamily(void* hal_context);
UINT32 HAL_GetProcessorModel(void* hal_context);

// Firmware Types
#define FIRMWARE_TYPE_BIOS 1
#define FIRMWARE_TYPE_UEFI 2
#define FIRMWARE_TYPE_GPU_VBIOS 3
#define FIRMWARE_TYPE_NETWORK 4
#define FIRMWARE_TYPE_STORAGE 5
#define FIRMWARE_TYPE_BMC 6

// Interrupt Types
#define INTERRUPT_TYPE_EXTERNAL 0
#define INTERRUPT_TYPE_SOFTWARE 1
#define INTERRUPT_TYPE_HARDWARE 2

// Power Profiles
#define POWER_PROFILE_PERFORMANCE 0
#define POWER_PROFILE_BALANCED 1
#define POWER_PROFILE_POWERSAVE 2
#define POWER_PROFILE_CUSTOM 3

// Hardware Breakpoint Types
#define HW_BREAKPOINT_EXECUTE 0
#define HW_BREAKPOINT_WRITE 1
#define HW_BREAKPOINT_READWRITE 2
#define HW_BREAKPOINT_IO 3

#endif // RAIJIN_HAL_H