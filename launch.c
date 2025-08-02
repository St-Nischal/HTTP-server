#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "Server.h"
#include <sys/socket.h>

void launch(struct Server *server) {
    char buffer[1024];
    printf("======== Waiting For Connection =========\n");

    socklen_t address_length = sizeof(server->address);
    int new_socket = accept(server->socket, (struct sockaddr *)&server->address, &address_length);

    if (new_socket < 0) {
        perror("Failed to accept connection");
        return;
    }

    ssize_t valread = read(new_socket, buffer, sizeof(buffer)-1);
    if (valread > 0) {
        buffer[valread] = '\0'; // null terminate buffer before printing
        printf("Received request:\n%s\n", buffer);
    }

    char *hello =
        "HTTP/1.1 200 OK\r\n"
        "Date: Thu, 01 Aug 2025 10:00:00 GMT\r\n"
        "Server: Apache/2.2.14 (Win32)\r\n"
        "Last-Modified: Wed, 31 Jul 2025 18:00:00 GMT\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 106\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><title>Hello</title></head>\n"
        "<body><h1>Hello from C server</h1></body>\n"
        "</html>\n";

    write(new_socket, hello, strlen(hello));
    close(new_socket);
}

int main(void) {
    struct Server server = server_constructor(AF_INET, SOCK_STREAM, 0, INADDR_ANY, 8080, 10, launch);

    while (1) {
        server.launch(&server);  // Accept and handle one client per iteration
    }

    return 0;  // (Actually unreachable unless you add exit conditions)
}