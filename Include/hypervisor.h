#ifndef HYPERVISOR_H
#define HYPERVISOR_H

#include <windows.h>
#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stdbool.h>

// Raijin Hypervisor Layer
// Type-1 hypervisor with absolute hardware control
// Ring -1 privilege escalation and hardware virtualization

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct HypervisorContext HypervisorContext;
typedef struct VirtualMachine VirtualMachine;
typedef struct VMXCapabilities VMXCapabilities;

// VMX (Virtual Machine Extensions) capabilities
struct VMXCapabilities {
    bool vmx_supported;
    bool unrestricted_guest;
    bool ept_supported;
    bool vpid_supported;
    uint32_t max_vcpu_count;
    uint64_t vmx_memory_limit;
    bool nested_virtualization;
    bool apic_virtualization;
    uint32_t max_msr_count;
};

// Virtual CPU state
typedef struct {
    // General purpose registers
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rsp, rbp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;

    // Instruction pointer and flags
    uint64_t rip;
    uint64_t rflags;

    // Control registers
    uint64_t cr0, cr2, cr3, cr4;

    // Debug registers
    uint64_t dr0, dr1, dr2, dr3, dr6, dr7;

    // Segment registers
    uint16_t cs, ds, es, fs, gs, ss;
    uint16_t tr, ldtr;
    uint64_t gdtr_base, idtr_base;
    uint32_t gdtr_limit, idtr_limit;

    // MSR values
    uint64_t ia32_efer;
    uint64_t ia32_pat;
    uint64_t ia32_sysenter_cs;
    uint64_t ia32_sysenter_esp;
    uint64_t ia32_sysenter_eip;
} VCPUState;

// Virtual machine structure
struct VirtualMachine {
    uint64_t vm_id;
    char vm_name[256];
    VCPUState* vcpu_states;
    uint32_t vcpu_count;
    uint64_t memory_size;
    void* memory_base;
    bool running;
    uint64_t uptime_ms;
    uint32_t exit_reason;
    uint64_t exit_qualification;
};

// Hypervisor context
struct HypervisorContext {
    bool initialized;
    VMXCapabilities vmx_caps;
    VirtualMachine* vms;
    uint32_t vm_count;
    uint32_t max_vms;
    void* vmcs_region;        // Virtual Machine Control Structure
    void* ept_pml4;          // Extended Page Table
    void* vpid_table;        // Virtual Processor ID table
    HANDLE host_memory_handle;
    CRITICAL_SECTION lock;
};

// VM exit reasons
#define VM_EXIT_EXTERNAL_INTERRUPT  1
#define VM_EXIT_TRIPLE_FAULT        2
#define VM_EXIT_INIT_SIGNAL         3
#define VM_EXIT_SIPI_SIGNAL         4
#define VM_EXIT_IO_SMI              5
#define VM_EXIT_OTHER_SMI           6
#define VM_EXIT_INTERRUPT_WINDOW    7
#define VM_EXIT_NMI_WINDOW          8
#define VM_EXIT_TASK_SWITCH         9
#define VM_EXIT_CPUID              10
#define VM_EXIT_GETSEC             11
#define VM_EXIT_HLT                12
#define VM_EXIT_INVD               13
#define VM_EXIT_INVLPG             14
#define VM_EXIT_RDPMC              15
#define VM_EXIT_RDTSC              16
#define VM_EXIT_RSM                17
#define VM_EXIT_VMCALL             18
#define VM_EXIT_VMCLEAR            19
#define VM_EXIT_VMLAUNCH           20
#define VM_EXIT_VMPTRLD            21
#define VM_EXIT_VMPTRST            22
#define VM_EXIT_VMREAD             23
#define VM_EXIT_VMRESUME           24
#define VM_EXIT_VMWRITE            25
#define VM_EXIT_VMXOFF             26
#define VM_EXIT_VMXON              27
#define VM_EXIT_CR_ACCESS          28
#define VM_EXIT_DR_ACCESS          29
#define VM_EXIT_IO_INSTRUCTION     30
#define VM_EXIT_RDMSR              31
#define VM_EXIT_WRMSR              32
#define VM_EXIT_INVALID_GUEST_STATE 33
#define VM_EXIT_MSR_LOADING        34
#define VM_EXIT_MWAIT_INSTRUCTION  36
#define VM_EXIT_MONITOR_TRAP_FLAG  37
#define VM_EXIT_MONITOR_INSTRUCTION 39
#define VM_EXIT_PAUSE_INSTRUCTION  40
#define VM_EXIT_MCE_DURING_VMENTRY 41
#define VM_EXIT_TPR_THRESHOLD      43
#define VM_EXIT_APIC_ACCESS        44
#define VM_EXIT_VIRTUALIZED_EOI    45
#define VM_EXIT_GDTR_IDTR_ACCESS   46
#define VM_EXIT_LDTR_TR_ACCESS     47
#define VM_EXIT_EPT_VIOLATION      48
#define VM_EXIT_EPT_MISCONFIG      49
#define VM_EXIT_INVEPT             50
#define VM_EXIT_RDTSCP             51
#define VM_EXIT_VMX_PREEMPTION_TIME 52
#define VM_EXIT_INVVPID            53
#define VM_EXIT_WBINVD             54
#define VM_EXIT_XSETBV             55
#define VM_EXIT_APIC_WRITE         56
#define VM_EXIT_RDRAND             57
#define VM_EXIT_INVPCID            58
#define VM_EXIT_VMFUNC             59
#define VM_EXIT_ENCLS              60
#define VM_EXIT_RDSEED             61
#define VM_EXIT_PML_FULL           62
#define VM_EXIT_XSAVES             63
#define VM_EXIT_XRSTORS            64

// Core hypervisor functions
NTSTATUS Hypervisor_Initialize(void** hypervisor_context, void* hal_context);
NTSTATUS Hypervisor_Shutdown(void* hypervisor_context);
NTSTATUS Hypervisor_EnableRingMinus1(void* hypervisor_context);

// Virtual machine management
NTSTATUS Hypervisor_CreateVM(HypervisorContext* context, const char* vm_name, uint64_t memory_size, VirtualMachine** vm);
NTSTATUS Hypervisor_DestroyVM(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_StartVM(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_StopVM(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_PauseVM(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_ResumeVM(HypervisorContext* context, VirtualMachine* vm);

// VM state management
NTSTATUS Hypervisor_GetVMState(HypervisorContext* context, VirtualMachine* vm, VCPUState* state);
NTSTATUS Hypervisor_SetVMState(HypervisorContext* context, VirtualMachine* vm, const VCPUState* state);
NTSTATUS Hypervisor_SaveVMState(HypervisorContext* context, VirtualMachine* vm, const char* filename);
NTSTATUS Hypervisor_LoadVMState(HypervisorContext* context, VirtualMachine* vm, const char* filename);

// Memory management
NTSTATUS Hypervisor_AllocateVMMemory(HypervisorContext* context, VirtualMachine* vm, size_t size, uint64_t* guest_address);
NTSTATUS Hypervisor_FreeVMMemory(HypervisorContext* context, VirtualMachine* vm, uint64_t guest_address, size_t size);
NTSTATUS Hypervisor_MapGuestMemory(HypervisorContext* context, VirtualMachine* vm, uint64_t guest_address, void* host_address, size_t size);
NTSTATUS Hypervisor_UnmapGuestMemory(HypervisorContext* context, VirtualMachine* vm, uint64_t guest_address, size_t size);

// I/O handling
NTSTATUS Hypervisor_HandleIO(HypervisorContext* context, VirtualMachine* vm, uint16_t port, bool is_write, uint32_t* value);
NTSTATUS Hypervisor_HandleMMIO(HypervisorContext* context, VirtualMachine* vm, uint64_t address, bool is_write, uint64_t* value);
NTSTATUS Hypervisor_InjectInterrupt(HypervisorContext* context, VirtualMachine* vm, uint8_t vector);

// Hardware passthrough
NTSTATUS Hypervisor_EnableDevicePassthrough(HypervisorContext* context, VirtualMachine* vm, uint16_t vendor_id, uint16_t device_id);
NTSTATUS Hypervisor_DisableDevicePassthrough(HypervisorContext* context, VirtualMachine* vm, uint16_t vendor_id, uint16_t device_id);

// Performance monitoring
NTSTATUS Hypervisor_GetVMPerformance(HypervisorContext* context, VirtualMachine* vm, uint64_t* cpu_usage, uint64_t* memory_usage);
NTSTATUS Hypervisor_GetHypervisorStats(HypervisorContext* context, uint64_t* total_vms, uint64_t* active_vms, uint64_t* total_memory_used);

// Security and isolation
NTSTATUS Hypervisor_EnableMemoryEncryption(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_DisableMemoryEncryption(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_IsolateVM(HypervisorContext* context, VirtualMachine* vm, bool enable);

// Advanced virtualization features
NTSTATUS Hypervisor_EnableNestedVirtualization(HypervisorContext* context);
NTSTATUS Hypervisor_DisableNestedVirtualization(HypervisorContext* context);
NTSTATUS Hypervisor_EnableAPICVirtualization(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_EnableEPT(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_EnableVPID(HypervisorContext* context, VirtualMachine* vm);

// Ring -1 operations (ultimate privilege)
NTSTATUS Hypervisor_ExecuteInRingMinus1(HypervisorContext* context, void* code, size_t code_size, void* parameters, void* result);
NTSTATUS Hypervisor_ReadPhysicalMemoryRingMinus1(HypervisorContext* context, uint64_t physical_address, void* buffer, size_t size);
NTSTATUS Hypervisor_WritePhysicalMemoryRingMinus1(HypervisorContext* context, uint64_t physical_address, const void* buffer, size_t size);
NTSTATUS Hypervisor_ExecutePrivilegedInstructionRingMinus1(HypervisorContext* context, uint32_t instruction, uint64_t operand, uint64_t* result);

// Firmware manipulation through hypervisor
NTSTATUS Hypervisor_ReadFirmware(HypervisorContext* context, uint32_t firmware_type, void* buffer, size_t* size);
NTSTATUS Hypervisor_WriteFirmware(HypervisorContext* context, uint32_t firmware_type, const void* buffer, size_t size);
NTSTATUS Hypervisor_UpdateFirmware(HypervisorContext* context, const void* firmware_image, size_t size);

// Hardware debugging through hypervisor
NTSTATUS Hypervisor_SetHardwareBreakpoint(HypervisorContext* context, VirtualMachine* vm, uint32_t breakpoint_number, uint64_t address, uint32_t type);
NTSTATUS Hypervisor_ClearHardwareBreakpoint(HypervisorContext* context, VirtualMachine* vm, uint32_t breakpoint_number);
NTSTATUS Hypervisor_GetVMDebugState(HypervisorContext* context, VirtualMachine* vm, uint64_t* debug_state);

// Utility functions
const char* Hypervisor_GetVMExitReasonString(uint32_t exit_reason);
bool Hypervisor_IsVMXSupported(HypervisorContext* context);
uint32_t Hypervisor_GetMaxVCPUs(HypervisorContext* context);
uint64_t Hypervisor_GetMaxVMMemory(HypervisorContext* context);

// Error handling
const char* Hypervisor_GetErrorString(NTSTATUS status);

// Advanced hypervisor features
NTSTATUS Hypervisor_EnableQuantumComputingEmulation(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_EnableNeuralNetworkAcceleration(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_EnableConsciousnessSimulation(HypervisorContext* context, VirtualMachine* vm);

// Multi-dimensional virtualization
NTSTATUS Hypervisor_EnableTemporalVirtualization(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_EnableProbabilisticVirtualization(HypervisorContext* context, VirtualMachine* vm);
NTSTATUS Hypervisor_EnableEntropicVirtualization(HypervisorContext* context, VirtualMachine* vm);

#ifdef __cplusplus
}
#endif

#endif // HYPERVISOR_H