#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <windows.h>
#include <winhttp.h>
#include "raijin_ntstatus.h"
#include <stdint.h>
#include <stdbool.h>

// Raijin HTTP Client with Stealth Capabilities
// Zero-trace web acquisition for knowledge gathering

// HTTP Client structure
struct HttpClient {
    HINTERNET session;              // WinHTTP session handle
    char** user_agents;             // Array of stealth user agents
    uint32_t user_agent_count;      // Number of user agents
    uint32_t current_user_agent;    // Current user agent index
    uint32_t min_delay_ms;          // Minimum delay between requests
    uint32_t max_delay_ms;          // Maximum delay between requests
    HINTERNET* connection_pool;     // Connection pool for efficiency
    uint32_t pool_size;             // Size of connection pool
    uint32_t pool_index;            // Current pool index
    uint64_t requests_sent;         // Statistics
    uint64_t responses_received;    // Statistics
    uint64_t bytes_downloaded;      // Statistics
};

// Core HTTP client functions
NTSTATUS HttpClient_Initialize(HttpClient** client);
NTSTATUS HttpClient_Shutdown(HttpClient* client);
NTSTATUS HttpClient_MakeRequest(HttpClient* client, const char* url, void** response_data,
                               uint64_t* response_size, char** content_type);
NTSTATUS HttpClient_MakeBatchRequests(HttpClient* client, const char** urls, uint32_t url_count,
                                    void*** responses, uint64_t** response_sizes,
                                    char*** content_types);

// Stealth and configuration functions
NTSTATUS HttpClient_ConfigureStealth(HttpClient* client, uint32_t min_delay_ms, uint32_t max_delay_ms);
const char* HttpClient_GetNextUserAgent(HttpClient* client);
void HttpClient_StealthDelay(HttpClient* client);

// Utility functions
NTSTATUS HttpClient_GetStatistics(HttpClient* client, char* stats, size_t stats_size);
NTSTATUS HttpClient_TestConnectivity(HttpClient* client, const char* test_url);

// Memory management
NTSTATUS AllocateHttpMemory(size_t size, void** buffer);
void FreeHttpMemory(void* buffer);

#endif // HTTP_CLIENT_H