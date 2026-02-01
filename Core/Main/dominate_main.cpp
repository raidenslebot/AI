#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../Include/programming_domination.h"

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

static int RunStats(void) {
    ProgrammingDominationSystem* system = NULL;
    NTSTATUS status = ProgrammingDomination_Initialize(&system, NULL, NULL, NULL);
    if (!NT_SUCCESS(status) || !system) {
        printf("Failed to initialize Programming Domination (0x%08X)\n", (unsigned)status);
        return 1;
    }
    char stats[2048] = {0};
    status = ProgrammingDomination_GetStatistics(system, stats, sizeof(stats));
    ProgrammingDomination_Shutdown(system);
    if (!NT_SUCCESS(status)) {
        printf("Failed to get statistics (0x%08X)\n", (unsigned)status);
        return 1;
    }
    printf("%s", stats);
    return 0;
}

static void PrintUsage(const char* exe) {
    printf("Raijin Programming Domination CLI\n");
    printf("Usage:\n");
    printf("  %s analyze <code> [--lang <language>]\n", exe);
    printf("  %s generate <description> [--lang <language>]\n", exe);
    printf("  %s stats\n", exe);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 1;
    }
    const char* cmd = argv[1];
    if (strcmp(cmd, "analyze") == 0) {
        return ProgrammingDomination_CLI_Analyze(argc, argv);
    }
    if (strcmp(cmd, "generate") == 0) {
        return ProgrammingDomination_CLI_Generate(argc, argv);
    }
    if (strcmp(cmd, "stats") == 0) {
        return RunStats();
    }
    PrintUsage(argv[0]);
    return 1;
}
