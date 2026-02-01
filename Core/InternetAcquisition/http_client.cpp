#include "internet_acquisition.h"
#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wincrypt.h>

// HTTP Client Implementation with Stealth Capabilities
// Zero-trace web acquisition for Raijin knowledge gathering

// Stealth user agents (realistic and varied)
static const char* STEALTH_USER_AGENTS[] = {
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:120.0) Gecko/20100101 Firefox/120.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.1 Safari/605.1.15",
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/119.0.0.0 Safari/537.36 Edg/119.0.0.0",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 OPR/106.0.0.0",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.15; rv:120.0) Gecko/20100101 Firefox/120.0",
    "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:120.0) Gecko/20100101 Firefox/120.0",
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Vivaldi/6.4.3160.41"
};

#define STEALTH_USER_AGENT_COUNT (sizeof(STEALTH_USER_AGENTS) / sizeof(STEALTH_USER_AGENTS[0]))

// Memory management for HTTP client
NTSTATUS AllocateHttpMemory(size_t size, void** buffer) {
    *buffer = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (*buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    memset(*buffer, 0, size);
    return STATUS_SUCCESS;
}

void FreeHttpMemory(void* buffer) {
    if (buffer) {
        VirtualFree(buffer, 0, MEM_RELEASE);
    }
}

// Initialize HTTP client with stealth capabilities
NTSTATUS HttpClient_Initialize(HttpClient** client) {
    *client = (HttpClient*)malloc(sizeof(HttpClient));
    if (!*client) return STATUS_INSUFFICIENT_RESOURCES;

    memset(*client, 0, sizeof(HttpClient));

    // Initialize WinHTTP session with stealth settings
    DWORD access_type = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
    LPCWSTR user_agent = L"Raijin/1.0";  // Will be overridden per request
    DWORD flags = WINHTTP_FLAG_ASYNC;

    (*client)->session = WinHttpOpen(user_agent, access_type, WINHTTP_NO_PROXY_NAME,
                                   WINHTTP_NO_PROXY_BYPASS, flags);

    if (!(*client)->session) {
        free(*client);
        return STATUS_UNSUCCESSFUL;
    }

    // Configure timeouts for stealth
    WinHttpSetTimeouts((*client)->session, 10000, 30000, 30000, 30000);

    // Set up user agents
    (*client)->user_agent_count = STEALTH_USER_AGENT_COUNT;
    (*client)->user_agents = (char**)malloc(sizeof(char*) * STEALTH_USER_AGENT_COUNT);
    if (!(*client)->user_agents) {
        WinHttpCloseHandle((*client)->session);
        free(*client);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    for (uint32_t i = 0; i < STEALTH_USER_AGENT_COUNT; i++) {
        (*client)->user_agents[i] = _strdup(STEALTH_USER_AGENTS[i]);
    }

    // Initialize timing for stealth
    (*client)->min_delay_ms = 1000;  // Minimum 1 second between requests
    (*client)->max_delay_ms = 5000;  // Maximum 5 seconds between requests

    // Initialize connection pool
    (*client)->pool_size = 10;
    (*client)->connection_pool = (HINTERNET*)malloc(sizeof(HINTERNET) * (*client)->pool_size);
    if (!(*client)->connection_pool) {
        // Cleanup and return error
        for (uint32_t i = 0; i < STEALTH_USER_AGENT_COUNT; i++) {
            free((*client)->user_agents[i]);
        }
        free((*client)->user_agents);
        WinHttpCloseHandle((*client)->session);
        free(*client);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    memset((*client)->connection_pool, 0, sizeof(HINTERNET) * (*client)->pool_size);

    return STATUS_SUCCESS;
}

// Shutdown HTTP client
NTSTATUS HttpClient_Shutdown(HttpClient* client) {
    if (!client) return STATUS_INVALID_PARAMETER;

    // Close all connections in pool
    for (uint32_t i = 0; i < client->pool_size; i++) {
        if (client->connection_pool[i]) {
            WinHttpCloseHandle(client->connection_pool[i]);
        }
    }

    // Close session
    if (client->session) {
        WinHttpCloseHandle(client->session);
    }

    // Free user agents
    if (client->user_agents) {
        for (uint32_t i = 0; i < client->user_agent_count; i++) {
            free(client->user_agents[i]);
        }
        free(client->user_agents);
    }

    // Free connection pool
    free(client->connection_pool);

    // Free client structure
    free(client);

    return STATUS_SUCCESS;
}

// Get next stealth user agent
const char* HttpClient_GetNextUserAgent(HttpClient* client) {
    if (!client || !client->user_agents) return STEALTH_USER_AGENTS[0];

    const char* ua = client->user_agents[client->current_user_agent];
    client->current_user_agent = (client->current_user_agent + 1) % client->user_agent_count;
    return ua;
}

// Implement stealth delay between requests
void HttpClient_StealthDelay(HttpClient* client) {
    if (!client) return;

    // Calculate random delay within configured range
    uint32_t delay_range = client->max_delay_ms - client->min_delay_ms;
    uint32_t delay = client->min_delay_ms;

    if (delay_range > 0) {
        delay += rand() % delay_range;
    }

    // Add some jitter to make timing less predictable
    delay += (rand() % 500) - 250;  // +/- 250ms jitter

    Sleep(delay);
}

// Parse URL into components
NTSTATUS ParseURL(const char* url, char** host, char** path, INTERNET_PORT* port, bool* is_https) {
    if (!url || !host || !path || !port || !is_https) {
        return STATUS_INVALID_PARAMETER;
    }

    *host = NULL;
    *path = NULL;
    *port = INTERNET_DEFAULT_HTTP_PORT;
    *is_https = false;

    // Check for HTTPS
    if (strncmp(url, "https://", 8) == 0) {
        *is_https = true;
        *port = INTERNET_DEFAULT_HTTPS_PORT;
        url += 8;
    } else if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    } else {
        return STATUS_INVALID_PARAMETER;  // Must have protocol
    }

    // Find host end
    const char* host_end = strchr(url, '/');
    const char* port_start = strchr(url, ':');

    size_t host_len;
    if (host_end) {
        host_len = host_end - url;
    } else {
        host_len = strlen(url);
    }

    // Check for port
    if (port_start && port_start < host_end) {
        // Parse port number
        char port_str[16] = {0};
        size_t port_len = host_end ? (host_end - port_start - 1) : strlen(port_start + 1);
        if (port_len < sizeof(port_str)) {
            strncpy_s(port_str, sizeof(port_str), port_start + 1, port_len);
            *port = (INTERNET_PORT)atoi(port_str);
        }
        host_len = port_start - url;
    }

    // Allocate and copy host
    *host = (char*)malloc(host_len + 1);
    if (!*host) return STATUS_INSUFFICIENT_RESOURCES;

    strncpy_s(*host, host_len + 1, url, host_len);
    (*host)[host_len] = '\0';

    // Allocate and copy path
    if (host_end) {
        size_t path_len = strlen(host_end);
        *path = (char*)malloc(path_len + 1);
        if (!*path) {
            free(*host);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        strcpy_s(*path, path_len + 1, host_end);
    } else {
        *path = _strdup("/");
        if (!*path) {
            free(*host);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return STATUS_SUCCESS;
}

// Get connection from pool or create new one
HINTERNET HttpClient_GetConnection(HttpClient* client, const char* host, INTERNET_PORT port, bool is_https) {
    if (!client || !host) return NULL;

    // Simple pool implementation - find existing or create new
    for (uint32_t i = 0; i < client->pool_size; i++) {
        uint32_t idx = (client->pool_index + i) % client->pool_size;
        if (client->connection_pool[idx]) {
            // Could check if connection matches host/port, but for simplicity reuse any
            HINTERNET conn = client->connection_pool[idx];
            client->connection_pool[idx] = NULL;
            return conn;
        }
    }

    // Create new connection
    DWORD flags = is_https ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET connection = WinHttpConnect(client->session, (LPCWSTR)host, port, 0);

    return connection;
}

// Return connection to pool
void HttpClient_ReturnConnection(HttpClient* client, HINTERNET connection) {
    if (!client || !connection) return;

    // Find empty slot in pool
    for (uint32_t i = 0; i < client->pool_size; i++) {
        uint32_t idx = (client->pool_index + i) % client->pool_size;
        if (!client->connection_pool[idx]) {
            client->connection_pool[idx] = connection;
            client->pool_index = (idx + 1) % client->pool_size;
            return;
        }
    }

    // Pool full, close connection
    WinHttpCloseHandle(connection);
}

// Core HTTP request function with stealth
NTSTATUS HttpClient_MakeRequest(HttpClient* client, const char* url, void** response_data,
                               uint64_t* response_size, char** content_type) {
    if (!client || !url || !response_data || !response_size) {
        return STATUS_INVALID_PARAMETER;
    }

    *response_data = NULL;
    *response_size = 0;
    if (content_type) *content_type = NULL;

    NTSTATUS status = STATUS_SUCCESS;
    HINTERNET connection = NULL;
    HINTERNET request = NULL;
    DWORD bytes_available = 0;
    size_t total_read = 0;
    const size_t buffer_size = 8192;
    uint8_t* buffer = NULL;
    const char* user_agent = NULL;

    // Parse URL
    char* host = NULL;
    char* path = NULL;
    INTERNET_PORT port;
    bool is_https;

    status = ParseURL(url, &host, &path, &port, &is_https);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    // Stealth delay before request
    HttpClient_StealthDelay(client);

    // Get connection
    connection = HttpClient_GetConnection(client, host, port, is_https);
    if (!connection) {
        status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    // Create request with stealth user agent
    user_agent = HttpClient_GetNextUserAgent(client);
    request = WinHttpOpenRequest(connection, L"GET", (LPCWSTR)path, NULL,
                                WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                is_https ? WINHTTP_FLAG_SECURE : 0);

    if (!request) {
        status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    // Set user agent header
    WCHAR ua_header[1024];
    swprintf_s(ua_header, sizeof(ua_header) / sizeof(WCHAR), L"User-Agent: %S", user_agent);
    WinHttpAddRequestHeaders(request, ua_header, (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

    // Add other stealth headers
    WinHttpAddRequestHeaders(request, L"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
                           (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpAddRequestHeaders(request, L"Accept-Language: en-US,en;q=0.5", (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpAddRequestHeaders(request, L"Accept-Encoding: gzip, deflate", (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpAddRequestHeaders(request, L"DNT: 1", (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpAddRequestHeaders(request, L"Connection: keep-alive", (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpAddRequestHeaders(request, L"Upgrade-Insecure-Requests: 1", (DWORD)-1, WINHTTP_ADDREQ_FLAG_ADD);

    // Send request
    if (!WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                           WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    // Receive response
    if (!WinHttpReceiveResponse(request, NULL)) {
        status = STATUS_UNSUCCESSFUL;
        goto cleanup;
    }

    // Get content type if requested
    if (content_type) {
        WCHAR content_type_w[256] = {0};
        DWORD header_size = sizeof(content_type_w);

        if (WinHttpQueryHeaders(request, WINHTTP_QUERY_CONTENT_TYPE,
                               WINHTTP_HEADER_NAME_BY_INDEX, content_type_w,
                               &header_size, WINHTTP_NO_HEADER_INDEX)) {
            // Convert wide char to char
            size_t converted_size;
            wcstombs_s(&converted_size, NULL, 0, content_type_w, 0);
            *content_type = (char*)malloc(converted_size);
            if (*content_type) {
                wcstombs_s(&converted_size, *content_type, converted_size, content_type_w, converted_size);
            }
        }
    }

    // Read response data
    buffer = (uint8_t*)malloc(buffer_size);

    if (!buffer) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    while (WinHttpQueryDataAvailable(request, &bytes_available) && bytes_available > 0) {
        if (total_read + bytes_available > 50 * 1024 * 1024) {  // 50MB limit
            break;  // Too large
        }

        uint8_t* temp_buffer = (uint8_t*)realloc(buffer, total_read + bytes_available);
        if (!temp_buffer) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            free(buffer);
            buffer = NULL;
            goto cleanup;
        }
        buffer = temp_buffer;

        DWORD bytes_read = 0;
        if (!WinHttpReadData(request, buffer + total_read, bytes_available, &bytes_read)) {
            status = STATUS_UNSUCCESSFUL;
            free(buffer);
            buffer = NULL;
            goto cleanup;
        }

        total_read += bytes_read;
    }

    // Success
    *response_data = buffer;
    *response_size = total_read;
    client->responses_received++;
    client->bytes_downloaded += total_read;

cleanup:
    if (buffer && *response_data != buffer) free(buffer);
    if (request) WinHttpCloseHandle(request);
    if (connection) HttpClient_ReturnConnection(client, connection);
    free(host);
    free(path);

    client->requests_sent++;
    return status;
}

// Batch request function for efficiency
NTSTATUS HttpClient_MakeBatchRequests(HttpClient* client, const char** urls, uint32_t url_count,
                                    void*** responses, uint64_t** response_sizes,
                                    char*** content_types) {
    if (!client || !urls || !responses || !response_sizes) {
        return STATUS_INVALID_PARAMETER;
    }

    *responses = (void**)malloc(sizeof(void*) * url_count);
    *response_sizes = (uint64_t*)malloc(sizeof(uint64_t) * url_count);

    if (content_types) {
        *content_types = (char**)malloc(sizeof(char*) * url_count);
    }

    if (!*responses || !*response_sizes || (content_types && !*content_types)) {
        free(*responses);
        free(*response_sizes);
        if (content_types) free(*content_types);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    memset(*responses, 0, sizeof(void*) * url_count);
    memset(*response_sizes, 0, sizeof(uint64_t) * url_count);

    if (content_types) {
        memset(*content_types, 0, sizeof(char*) * url_count);
    }

    NTSTATUS overall_status = STATUS_SUCCESS;

    // Process requests sequentially with stealth delays
    for (uint32_t i = 0; i < url_count; i++) {
        NTSTATUS status = HttpClient_MakeRequest(client, urls[i],
                                                &(*responses)[i], &(*response_sizes)[i],
                                                content_types ? &(*content_types)[i] : NULL);

        if (!NT_SUCCESS(status)) {
            overall_status = status;
            // Continue with other requests
        }
    }

    return overall_status;
}

// Configure stealth parameters
NTSTATUS HttpClient_ConfigureStealth(HttpClient* client, uint32_t min_delay_ms, uint32_t max_delay_ms) {
    if (!client) return STATUS_INVALID_PARAMETER;

    client->min_delay_ms = min_delay_ms;
    client->max_delay_ms = max_delay_ms;

    return STATUS_SUCCESS;
}

// Get client statistics
NTSTATUS HttpClient_GetStatistics(HttpClient* client, char* stats, size_t stats_size) {
    if (!client || !stats) return STATUS_INVALID_PARAMETER;

    sprintf_s(stats, stats_size,
              "HTTP Client Statistics:\n"
              "Requests Sent: %llu\n"
              "Responses Received: %llu\n"
              "Bytes Downloaded: %llu\n"
              "Success Rate: %.1f%%\n"
              "Current User Agent Index: %u/%u\n"
              "Connection Pool Size: %u",
              client->requests_sent,
              client->responses_received,
              client->bytes_downloaded,
              client->requests_sent > 0 ?
                  (double)client->responses_received / client->requests_sent * 100.0 : 0.0,
              client->current_user_agent,
              client->user_agent_count,
              client->pool_size);

    return STATUS_SUCCESS;
}

// Test connectivity function
NTSTATUS HttpClient_TestConnectivity(HttpClient* client, const char* test_url) {
    void* response_data = NULL;
    uint64_t response_size = 0;

    NTSTATUS status = HttpClient_MakeRequest(client, test_url ? test_url : "https://www.google.com",
                                           &response_data, &response_size, NULL);

    if (response_data) {
        free(response_data);
    }

    return status;
}