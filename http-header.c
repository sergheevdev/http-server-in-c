#include "http-header.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

HttpHeader * http_header_create(char * name, char * value) {
    HttpHeader * http_header = malloc(sizeof(HttpHeader));
    if(http_header == NULL) {
        fprintf(stderr, "Unable to allocate memory for http_header struct\n");
        return NULL;
    }
    if(name == NULL) {
        free(http_header);
        fprintf(stderr, "Provided name to http_header_create must not be NULL\n");
        return NULL;
    }
    if(value == NULL) {
        free(http_header);
        fprintf(stderr, "Provided value to http_header_create must not be NULL\n");
        return NULL;
    }
    http_header->name = name;
    http_header->value = value;
    return http_header;
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