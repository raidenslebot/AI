#include "../../Include/role_boundary.h"
#include <string.h>

static RoleBoundaryContext* g_ctx = NULL;

NTSTATUS RoleBoundary_Initialize(RoleBoundaryContext* ctx) {
    if (!ctx) return STATUS_INVALID_PARAMETER;
    memset(ctx, 0, sizeof(RoleBoundaryContext));
    ctx->initialized = true;
    g_ctx = ctx;
    return STATUS_SUCCESS;
}

void RoleBoundary_Shutdown(RoleBoundaryContext* ctx) {
    if (ctx) {
        ctx->initialized = false;
        ctx->stack_depth = 0;
        ctx->violation_count = 0;
        if (g_ctx == ctx) g_ctx = NULL;
    }
}

NTSTATUS RoleBoundary_Enter(RoleBoundaryContext* ctx, const char* module_name, RoleOwner owner) {
    if (!ctx || !ctx->initialized || !module_name) return STATUS_INVALID_PARAMETER;
    if (ctx->stack_depth >= ROLE_BOUNDARY_STACK_DEPTH) return STATUS_INSUFFICIENT_RESOURCES;
    size_t len = strlen(module_name);
    if (len >= ROLE_BOUNDARY_MODULE_NAME_MAX) len = ROLE_BOUNDARY_MODULE_NAME_MAX - 1;
    memcpy(ctx->stack[ctx->stack_depth].module_name, module_name, len);
    ctx->stack[ctx->stack_depth].module_name[len] = '\0';
    ctx->stack[ctx->stack_depth].owner = owner;
    ctx->stack_depth++;
    return STATUS_SUCCESS;
}

NTSTATUS RoleBoundary_Exit(RoleBoundaryContext* ctx, const char* module_name) {
    if (!ctx || !ctx->initialized) return STATUS_INVALID_PARAMETER;
    if (ctx->stack_depth == 0) return STATUS_SUCCESS;
    if (module_name && strcmp(ctx->stack[ctx->stack_depth - 1].module_name, module_name) != 0)
        ctx->violation_count++;
    ctx->stack_depth--;
    return STATUS_SUCCESS;
}

static void role_boundary_self_correct(RoleBoundaryContext* ctx) {
    if (ctx) ctx->stack_depth = 0;
}

bool RoleBoundary_AssertCursor(RoleBoundaryContext* ctx) {
    if (!ctx || !ctx->initialized) return true;
    if (ctx->stack_depth == 0) {
        ctx->violation_count++;
        role_boundary_self_correct(ctx);
        return false;
    }
    if (ctx->stack[ctx->stack_depth - 1].owner != ROLE_OWNER_CURSOR) {
        ctx->violation_count++;
        role_boundary_self_correct(ctx);
        return false;
    }
    return true;
}

bool RoleBoundary_AssertRaijin(RoleBoundaryContext* ctx) {
    if (!ctx || !ctx->initialized) return true;
    if (ctx->stack_depth == 0) {
        ctx->violation_count++;
        role_boundary_self_correct(ctx);
        return false;
    }
    if (ctx->stack[ctx->stack_depth - 1].owner != ROLE_OWNER_RAIJIN) {
        ctx->violation_count++;
        role_boundary_self_correct(ctx);
        return false;
    }
    return true;
}

uint32_t RoleBoundary_GetViolationCount(const RoleBoundaryContext* ctx) {
    return ctx ? ctx->violation_count : 0;
}

bool RoleBoundary_Validate(const RoleBoundaryContext* ctx) {
    return ctx && ctx->violation_count == 0;
}

void RoleBoundary_Reset(RoleBoundaryContext* ctx) {
    if (ctx) {
        ctx->stack_depth = 0;
        ctx->violation_count = 0;
    }
}

RoleBoundaryContext* RoleBoundary_GetGlobal(void) {
    return g_ctx;
}
