#ifndef RAIJIN_ROLE_BOUNDARY_H
#define RAIJIN_ROLE_BOUNDARY_H

#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stdbool.h>

#define ROLE_BOUNDARY_STACK_DEPTH 16
#define ROLE_BOUNDARY_MODULE_NAME_MAX 64

typedef enum RoleOwner {
    ROLE_OWNER_CURSOR = 0,
    ROLE_OWNER_RAIJIN = 1
} RoleOwner;

typedef struct RoleBoundaryFrame {
    char module_name[ROLE_BOUNDARY_MODULE_NAME_MAX];
    RoleOwner owner;
} RoleBoundaryFrame;

typedef struct RoleBoundaryContext {
    RoleBoundaryFrame stack[ROLE_BOUNDARY_STACK_DEPTH];
    uint32_t stack_depth;
    uint32_t violation_count;
    bool initialized;
} RoleBoundaryContext;

NTSTATUS RoleBoundary_Initialize(RoleBoundaryContext* ctx);
void RoleBoundary_Shutdown(RoleBoundaryContext* ctx);

NTSTATUS RoleBoundary_Enter(RoleBoundaryContext* ctx, const char* module_name, RoleOwner owner);
NTSTATUS RoleBoundary_Exit(RoleBoundaryContext* ctx, const char* module_name);

bool RoleBoundary_AssertCursor(RoleBoundaryContext* ctx);
bool RoleBoundary_AssertRaijin(RoleBoundaryContext* ctx);

uint32_t RoleBoundary_GetViolationCount(const RoleBoundaryContext* ctx);
bool RoleBoundary_Validate(const RoleBoundaryContext* ctx);
void RoleBoundary_Reset(RoleBoundaryContext* ctx);

RoleBoundaryContext* RoleBoundary_GetGlobal(void);

#endif
