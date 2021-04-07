#include "http-method.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

bool http_method_is_valid_value(const char * value);

HttpMethod * http_method_create(char * value) {
    if(value == NULL) {
        fprintf(stderr, "Provided value to http_method_create must not be NULL\n");
        return NULL;
    }
    if(http_method_is_valid_value(value) == false) {
        fprintf(stderr, "Provided value to http_method_create is not valid\n");
        return NULL;
    }
    HttpMethod * http_method = malloc(sizeof(HttpMethod));
    if(http_method == NULL) {
        fprintf(stderr, "Unable to allocate memory for http_method struct\n");
        return NULL;
    }
    http_method->value = value;
    return http_method;
}

bool http_method_matches_value(const HttpMethod * method, const char * value) {
    if(value == NULL) {
        fprintf(stderr, "Provided value to http_method_matches_value must not be NULL\n");
        return false;
    }
    if(http_method_is_valid_value(value) == false) {
        fprintf(stderr, "Provided value to http_method_matches_value is not valid\n");
        return false;
    }
    return strcmp(value, method->value) == 0;
}

bool http_method_is_valid_value(const char * value) {
    return strcmp(value, "OPTIONS") == 0 ||
           strcmp(value, "GET") == 0 ||
           strcmp(value, "HEAD") == 0 ||
           strcmp(value, "POST") == 0 ||
           strcmp(value, "PUT") == 0 ||
           strcmp(value, "DELETE") == 0 ||
           strcmp(value, "TRACE") == 0 ||
           strcmp(value, "CONNECT") == 0;
}

void http_method_free(HttpMethod * method) {
    if(method == NULL) return;
    if(method->value != NULL) free(method->value);
    free(method);
}

const char * http_method_get_value(const HttpMethod * method) {
    return method->value;
}