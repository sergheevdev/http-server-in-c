#include "http-header.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool http_header_is_valid_name(char * name);

HttpHeader * http_header_create(char * name, char * value) {
    if(name == NULL) {
        fprintf(stderr, "Provided name to http_header_create must not be NULL\n");
        return NULL;
    }
    if(value == NULL) {
        fprintf(stderr, "Provided value to http_header_create must not be NULL\n");
        return NULL;
    }
    if(!http_header_is_valid_name(name)) {
        fprintf(stderr, "Provided name to http_header_create is not valid\n");
        return NULL;
    }
    HttpHeader * http_header = malloc(sizeof(HttpHeader));
    if(http_header == NULL) {
        fprintf(stderr, "Unable to allocate memory for http_header struct\n");
        return NULL;
    }
    http_header->name = name;
    http_header->value = value;
    return http_header;
}

bool http_header_is_valid_name(char * name) {
    bool is_valid = true;
    char * current_char = name;
    while((*current_char) != '\0' && is_valid) {
        is_valid =
            ((* current_char) >= 'a' && (* current_char) <= 'z')
            || ((* current_char) >= 'A' && (* current_char) <= 'Z')
            || ((* current_char) == '-');
        current_char++;
    }
    return is_valid;
}

void http_header_free(HttpHeader * header) {
    if(header == NULL) return;
    if(header->name != NULL) free(header->name);
    if(header->value != NULL) free(header->value);
    free(header);
}

const char * http_header_get_name(HttpHeader * header) {
    return header->name;
}

const char * http_header_get_value(HttpHeader * header) {
    return header->value;
}