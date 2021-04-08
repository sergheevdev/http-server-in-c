#include "http-version.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

bool http_version_is_valid_value(char * value);

HttpVersion * http_version_create(char * value) {
    if(value == NULL) {
        fprintf(stderr, "Provided value to http_version_create must not be NULL\n");
        return NULL;
    }
    if(http_version_is_valid_value(value) == false) {
        fprintf(stderr, "Provided value to http_version_create is not valid\n");
        return NULL;
    }
    HttpVersion * http_version = malloc(sizeof(HttpVersion));
    if(http_version == NULL) {
        fprintf(stderr, "Unable to allocate memory for http_version struct\n");
        return NULL;
    }
    http_version->value = value;
    return http_version;
}

bool http_version_is_valid_value(char * value) {
    char * current_char = value;
    if((* current_char) != 'H') return false;
    current_char++;
    if((* current_char) != 'T') return false;
    current_char++;
    if((* current_char) != 'T') return false;
    current_char++;
    if((* current_char) != 'P') return false;
    current_char++;
    if((* current_char) != '/') return false;
    current_char++;
    if((* current_char) < '0' && (* current_char) > '9') return false;
    current_char++;
    // If minor is not present, still correct (minor is optional)
    if((* current_char) == '\0') return true;
    // Version minor
    if((* current_char) != '.') return false;
    current_char++;
    if((* current_char) < '0' && (* current_char) > '9') return false;
    current_char++;
    if((* current_char) == '\0') return true;
}

void http_version_free(HttpVersion * version) {
    if(version == NULL) return;
    if(version->value != NULL) free(version->value);
    free(version);
}

const char * http_version_get_value(const HttpVersion * version) {
    return version->value;
}