#include "HTTPRequest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to parse method string to enum
int parse_method(const char *method_str) {
    if (strcmp(method_str, "GET") == 0) return GET;
    if (strcmp(method_str, "POST") == 0) return POST;
    if (strcmp(method_str, "PUT") == 0) return PUT;
    if (strcmp(method_str, "DELETE") == 0) return DELETE;
    if (strcmp(method_str, "HEAD") == 0) return HEAD;
    if (strcmp(method_str, "OPTIONS") == 0) return OPTIONS;
    if (strcmp(method_str, "PATCH") == 0) return PATCH;
    if (strcmp(method_str, "CONNECT") == 0) return CONNECT;
    if (strcmp(method_str, "TRACE") == 0) return TRACE;
    return -1; // Unknown method
}

struct HTTPRequest http_request_constructor(char *request_string) {
    struct HTTPRequest request = {0}; // Initialize all fields to 0/NULL

    if (!request_string) return request; // Safety check

    // Make a copy to work with (so we don't modify original)
    char *request_copy = strdup(request_string);
    char method_str[16], uri[512], version_str[16];

    // 1. Parse REQUEST LINE (first line)
    char *line_end = strstr(request_copy, "\r\n");
    if (line_end) {
        *line_end = '\0'; // Temporarily end first line

        // Extract method, URI, version
        if (sscanf(request_copy, "%s %s %s", method_str, uri, version_str) == 3) {
            // Parse method
            request.method = parse_method(method_str);

            // Parse URI
            request.URI = malloc(strlen(uri) + 1);
            strcpy(request.URI, uri);

            // Parse HTTP version (e.g., "HTTP/1.1" -> 1.1)
            sscanf(version_str, "HTTP/%f", &request.HTTPVersion);
        }

        *line_end = '\r'; // Restore
    }

    // 2. Find HEADERS and BODY separator
    char *headers_start = strstr(request_copy, "\r\n") + 2; // Skip first \r\n
    char *body_start = strstr(request_copy, "\r\n\r\n");

    if (body_start) {
        // 3. Extract HEADERS
        int headers_length = body_start - headers_start;
        char *raw_headers = malloc(headers_length + 1);
        strncpy(raw_headers, headers_start, headers_length);
        raw_headers[headers_length] = '\0';

        // Parse each header line
        request.header_count = 0;
        char *line = strtok(raw_headers, "\r\n");
        while (line && request.header_count < MAX_HEADERS) {
            char *colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                request.headers[request.header_count].key = strdup(line);
                char *val_start = colon + 1;
                while (*val_start == ' ') val_start++; // Trim spaces before strdup
                request.headers[request.header_count].value = strdup(val_start);
                request.header_count++;
            }
            line = strtok(NULL, "\r\n");
        }

        free(raw_headers);  // we don't need it anymore

        // 4. Extract BODY
        body_start += 4; // Skip "\r\n\r\n"
        request.BODY = malloc(strlen(body_start) + 1);
        strcpy(request.BODY, body_start);
    } else {
        // No body, just headers
        request.header_count = 0;
        char *raw_headers = strdup(headers_start);
        char *line = strtok(raw_headers, "\r\n");
        while (line && request.header_count < MAX_HEADERS) {
            char *colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                request.headers[request.header_count].key = strdup(line);
                request.headers[request.header_count].value = strdup(colon + 1);

                // Trim leading whitespace from value
                while (*(request.headers[request.header_count].value) == ' ') {
                    request.headers[request.header_count].value++;
                }

                request.header_count++;
            }
            line = strtok(NULL, "\r\n");
        }

        free(raw_headers);
        request.BODY = NULL;
    }

    return request;
}

// Helper function to free allocated memory
void http_request_destructor(struct HTTPRequest *request) {

    if (request->URI) {
        free(request->URI);
        request->URI = NULL;
    }
    for (int i = 0; i < request->header_count; ++i) {

        if (request->headers[i].key) {
            free(request->headers[i].key);
            request->headers[i].key = NULL;
        }

        if (request->headers[i].value) {
            free(request->headers[i].value);
            request->headers[i].value = NULL;
        }

    }
    if (request->BODY) {
        free(request->BODY);
        request->BODY = NULL;
    }

}


