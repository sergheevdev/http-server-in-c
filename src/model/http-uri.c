#include "http-uri.h"
#include "../http-validator.h"
#include <stdlib.h>
#include <stdio.h>

bool http_uri_is_valid_value(char * value);

HttpUri * http_uri_create(char * value) {
    if(value == NULL) {
        fprintf(stderr, "Provided value to http_uri_create must not be NULL\n");
        return NULL;
    }
    if(http_uri_is_valid_value(value) == false) {
        fprintf(stderr, "Provided value to http_uri_create is not valid\n");
        return NULL;
    }
    HttpUri * http_uri = malloc(sizeof(HttpUri));
    if(http_uri == NULL) {
        fprintf(stderr, "Unable to allocate memory for http_uri struct\n");
        return NULL;
    }
    http_uri->value = value;
    return http_uri;
}

bool http_uri_is_valid_value(char * value) {
    bool is_valid = true;
    char * current_char = value;
    while((*current_char) != '\0' && is_valid) {
        is_valid = http_validator_is_uri((* current_char));
        current_char++;
    }
    return is_valid;
}

void http_uri_free(HttpUri * uri) {
    if(uri == NULL) return;
    if(uri->value != NULL) free(uri->value);
    free(uri);
}

const char * http_uri_get_value(const HttpUri * uri) {
    return uri->value;
}