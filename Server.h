//
// Created by Nischal Poudel on 01/08/2025.
//

// Server.h
#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

struct Server {
    int domain;
    int service;
    int protocol;
    u_long interface;
    int port;
    int backlog;
    int socket;
    struct sockaddr_in address;

    void (*launch)(struct Server *);
};

struct Server server_constructor(int domain, int service, int protocol, u_long interface, int port, int backlog, void (*launch)(struct Server *));

#endif //SERVER_H
