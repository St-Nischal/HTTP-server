//
// Created by Nischal Poudel on 02/08/2025.
//

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#define MAX_HEADERS 20

enum HTTPMethod {
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    OPTIONS,
    PATCH,
    CONNECT,
    TRACE
};


struct Header {
    char *key;
    char *value;
};

struct HTTPRequest {
    int method;
    char *URI;
    float HTTPVersion;
    struct Header headers[MAX_HEADERS];
    int header_count;
    char *BODY;
};

struct HTTPRequest http_request_constructor( char *request_string);

// Add this function declaration to HTTPRequest.h
void http_request_destructor(struct HTTPRequest *request);


#endif //HTTPREQUEST_H
