#include "http-body.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

HttpBody * http_body_create_empty() {
    char * empty = malloc(sizeof(char));
    if(empty == NULL) {
        fprintf(stderr, "Unable to allocate memory for empty string\n");
        return NULL;
    }
    (* empty) = '\0';
    HttpBody * http_body = http_body_create(empty);
    if(http_body == NULL) free(empty);
    return http_body;
}

HttpBody * http_body_create(char * value) {
    if(value == NULL) {
        fprintf(stderr, "Provided value to http_body_create must not be NULL\n");
        return NULL;
    }
    HttpBody * http_body = malloc(sizeof(HttpBody));
    if(http_body == NULL) {
        fprintf(stderr, "Unable to allocate memory for http_body struct\n");
        return NULL;
    }
    http_body->value = value;
    return http_body;
}

void http_body_free(HttpBody * body) {
    if(body == NULL) return;
    if(body->value != NULL) free(body->value);
    free(body);
}

const char * http_body_get_value(const HttpBody * body) {
    return body->value;
}