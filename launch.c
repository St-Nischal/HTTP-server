#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "Server.h"
#include "HTTPRequest.h"
#include <sys/socket.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <ctype.h>
#include <pthread.h>


#define RESPONSE_BUFFER_SIZE 2048

char response[RESPONSE_BUFFER_SIZE];


void send_file_response(const char *filename, char *response_buffer) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        snprintf(response_buffer, 2048,
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n"
            "Failed to open file.");
        return;
    }

    char body[1500];
    size_t bytes_read = fread(body, 1, sizeof(body) - 1, file);
    body[bytes_read] = '\0';
    fclose(file);

    snprintf(response_buffer, 2048,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", strlen(body), body);
}


void save_to_json_file(const char *name, const char *value) {
    FILE *file;
    struct stat st;
    cJSON *root_array;

    // Check if file exists
    if (stat("data.json", &st) == 0) {
        file = fopen("data.json", "r");
        if (!file) {
            perror("Failed to open data.json");
            return;
        }

        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        rewind(file);

        char *data = malloc(length + 1);
        fread(data, 1, length, file);
        data[length] = '\0';
        fclose(file);

        root_array = cJSON_Parse(data);
        free(data);
        if (!root_array) {
            root_array = cJSON_CreateArray(); // fallback if corrupted
        }
    } else {
        root_array = cJSON_CreateArray(); // file doesn’t exist yet
    }

    // Create new entry
    cJSON *entry = cJSON_CreateObject();
    cJSON_AddStringToObject(entry, "name", name);
    cJSON_AddStringToObject(entry, "value", value);

    cJSON_AddItemToArray(root_array, entry);

    // Save updated JSON
    char *json_string = cJSON_Print(root_array);
    file = fopen("data.json", "w");
    if (!file) {
        perror("Failed to write to data.json");
        cJSON_Delete(root_array);
        free(json_string);
        return;
    }

    fputs(json_string, file);
    fclose(file);

    cJSON_Delete(root_array);
    free(json_string);
}

int edit_json_entry(const char *name_to_edit, const char *new_value) {
    FILE *file = fopen("data.json", "r");
    if (!file) {
        perror("Failed to open data.json");
        return 0;
    }

    // Read file contents into a buffer
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    // Parse JSON
    cJSON *json_array = cJSON_Parse(buffer);
    free(buffer);

    if (!json_array || !cJSON_IsArray(json_array)) {
        cJSON_Delete(json_array);
        return 0;
    }

    int updated = 0;
    cJSON *entry = NULL;
    cJSON_ArrayForEach(entry, json_array) {
        cJSON *name_item = cJSON_GetObjectItem(entry, "name");
        if (name_item && strcmp(name_item->valuestring, name_to_edit) == 0) {
            cJSON *value_item = cJSON_GetObjectItem(entry, "value");
            if (value_item) {
                cJSON_SetValuestring(value_item, new_value);
                updated = 1;
                break; // assuming unique names
            }
        }
    }

    if (!updated) {
        cJSON_Delete(json_array);
        return 0;  // No match found
    }

    // Write back to file
    file = fopen("data.json", "w");
    if (!file) {
        perror("Failed to open data.json for writing");
        cJSON_Delete(json_array);
        return 0;
    }

    char *new_json = cJSON_Print(json_array);
    if (!new_json) {
        fclose(file);
        cJSON_Delete(json_array);
        return 0;
    }

    fputs(new_json, file);
    fclose(file);
    free(new_json);
    cJSON_Delete(json_array);

    return 1;  // Success
}



void send_404_response(char *response_buffer) {
    snprintf(response_buffer, 512,
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 44\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>404 Not Found</h1></body></html>");
}


int delete_json_entry(const char *name_to_delete) {
    FILE *file = fopen("data.json", "r");
    if (!file) {
        perror("Failed to open data.json");
        return 0;
    }

    // Read file contents into a buffer
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return 0;
    }
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    // Parse JSON
    cJSON *json_array = cJSON_Parse(buffer);
    free(buffer);

    if (!json_array || !cJSON_IsArray(json_array)) {
        cJSON_Delete(json_array);
        return 0;
    }

    int deleted = 0;
    int array_size = cJSON_GetArraySize(json_array);

    // Loop through array and find entry to delete
    for (int i = 0; i < array_size; i++) {
        cJSON *entry = cJSON_GetArrayItem(json_array, i);
        cJSON *name_item = cJSON_GetObjectItem(entry, "name");

        if (name_item && strcmp(name_item->valuestring, name_to_delete) == 0) {
            cJSON_DeleteItemFromArray(json_array, i);
            deleted = 1;
            break; // assuming unique names
        }
    }

    if (!deleted) {
        cJSON_Delete(json_array);
        return 0;  // No match found
    }

    // Write back to file
    file = fopen("data.json", "w");
    if (!file) {
        perror("Failed to open data.json for writing");
        cJSON_Delete(json_array);
        return 0;
    }

    char *new_json = cJSON_Print(json_array);
    if (!new_json) {
        fclose(file);
        cJSON_Delete(json_array);
        return 0;
    }

    fputs(new_json, file);
    fclose(file);
    free(new_json);
    cJSON_Delete(json_array);

    return 1;  // Success
}

void url_decode(char *dst, const char *src) {
    while (*src) {
        if (*src == '%') {
            if (isxdigit(*(src + 1)) && isxdigit(*(src + 2))) {
                char hex[3] = { *(src + 1), *(src + 2), '\0' };
                *dst++ = (char) strtol(hex, NULL, 16);
                src += 3;
            } else {
                *dst++ = *src++;
            }
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

void parse_form_data(const char *body, char *name, char *value) {
    char encoded_name[256] = {0};
    char encoded_value[256] = {0};

    sscanf(body, "name=%255[^&]&value=%255s", encoded_name, encoded_value);

    url_decode(name, encoded_name);
    url_decode(value, encoded_value);
}



void* handle_client(void* socket_desc) {
    int new_socket = *(int*)socket_desc;
    char buffer[1024];

    printf("======== Handling Client in Thread %p =========\n", (void *)pthread_self());

    ssize_t valread = read(new_socket, buffer, sizeof(buffer) - 1);
    if (valread > 0) {
        buffer[valread] = '\0'; // null terminate buffer before printing
        printf("Received request:\n%s\n", buffer);

        struct HTTPRequest req = http_request_constructor(buffer);

        printf("Parsed Request:\n");
        printf("Method: %d (GET=%d)\n", req.method, GET);
        printf("URI: %s\n", req.URI);
        printf("HTTP Version: %.1f\n", req.HTTPVersion);

        char response[2048];

        // === ROUTING === (SAME AS YOUR ORIGINAL CODE)
        if (req.method == GET) {
            if (strcmp(req.URI, "/") == 0) {
                send_file_response("Homepage.html", response);
            } else if (strcmp(req.URI, "/add") == 0) {
                send_file_response("Add Data.html", response);
            } else if (strcmp(req.URI, "/edit") == 0) {
                send_file_response("Edit Data.html", response);
            } else {
                send_404_response(response);
            }

        } else if (req.method == POST && strcmp(req.URI, "/api/data") == 0) {
            char name[256];
            char value[256];
            parse_form_data(req.BODY, name, value);  // Parses from x-www-form-urlencoded

            if (strlen(name) > 0 && strlen(value) > 0) {
                save_to_json_file(name, value);  // used to save to data.json
                snprintf(response, RESPONSE_BUFFER_SIZE,
                    "HTTP/1.1 201 Created\r\n"
                    "Content-Type: application/json\r\n"
                    "\r\n"
                    "{\"message\": \"Entry added\"}");
            } else {
                snprintf(response, RESPONSE_BUFFER_SIZE,
                    "HTTP/1.1 400 Bad Request\r\n"
                    "Content-Type: text/plain\r\n"
                    "\r\n"
                    "Missing name or value");
            }
        }else if (req.method == PUT && strcmp(req.URI, "/api/data/1") == 0) {
            char name[256] = {0};
            char value[256] = {0};

            // Parse form data like: name=abc&value=xyz
            parse_form_data(req.BODY, name, value);

            if (strlen(name) > 0 && strlen(value) > 0) {
                if (edit_json_entry(name, value)) {
                    snprintf(response, RESPONSE_BUFFER_SIZE,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "\r\n"
                        "{\"message\": \"Entry updated\"}");
                } else {
                    snprintf(response, RESPONSE_BUFFER_SIZE,
                        "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: application/json\r\n"
                        "\r\n"
                        "{\"error\": \"Entry not found\"}");
                }
            } else {
                snprintf(response, RESPONSE_BUFFER_SIZE,
                    "HTTP/1.1 400 Bad Request\r\n"
                    "Content-Type: text/plain\r\n"
                    "\r\n"
                    "Missing 'name' or 'value' in form data.");
            }
        }else if (req.method == DELETE && strcmp(req.URI, "/api/data") == 0) {
            char name[256] = {0};
            char value[256] = {0}; // Not used for delete, but parse_form_data expects it

            // Parse form data to get the name of entry to delete
            parse_form_data(req.BODY, name, value);

            if (strlen(name) > 0) {
                if (delete_json_entry(name)) {
                    snprintf(response, RESPONSE_BUFFER_SIZE,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "\r\n"
                        "{\"message\": \"Entry deleted successfully\"}");
                } else {
                    snprintf(response, RESPONSE_BUFFER_SIZE,
                        "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: application/json\r\n"
                        "\r\n"
                        "{\"error\": \"Entry not found\"}");
                }
            } else {
                snprintf(response, RESPONSE_BUFFER_SIZE,
                    "HTTP/1.1 400 Bad Request\r\n"
                    "Content-Type: application/json\r\n"
                    "\r\n"
                    "{\"error\": \"Missing 'name' parameter\"}");
            }

        } else {
            send_404_response(response);
        }

        write(new_socket, response, strlen(response));
        http_request_destructor(&req);
    }

    close(new_socket);
    free(socket_desc);  // Free the malloc'd socket descriptor
    printf("======== Thread %p finished =========\n", (void *)pthread_self());
    return NULL;
}

// MODIFIED: Your launch function now just accepts connections and creates threads
void launch(struct Server *server) {
    printf("======== Waiting For Connection =========\n");
    socklen_t address_length = sizeof(server->address);
    int new_socket = accept(server->socket, (struct sockaddr *)&server->address, &address_length);

    if (new_socket < 0) {
        perror("Failed to accept connection");
        return;
    }

    // Create a thread to handle this client
    int *socket_desc = malloc(sizeof(int));
    *socket_desc = new_socket;

    pthread_t thread;
    int result = pthread_create(&thread, NULL, handle_client, socket_desc);

    if (result != 0) {
        perror("Failed to create thread");
        close(new_socket);
        free(socket_desc);
        return;
    }

    // Detach the thread so we don't need to wait for it
    pthread_detach(thread);

    printf("Created thread %p for new client\n", (void *)thread);
}

// Your main function stays the same
int main(void) {
    struct Server server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 10, launch);
    while (1) {
        server.launch(&server);  // Accept and handle one client per iteration
    }
    return 0;
}