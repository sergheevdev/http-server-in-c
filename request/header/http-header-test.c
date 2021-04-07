#include "http-header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void assert(int condition, char * message, char * caller) {
    if(condition != 1) {
        printf("[%s] %s\n", caller, message);
        exit(1);
    }
}

void http_header_create_ok_test() {
    char * name = strdup("Accept");
    if(name == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the name variable", __func__);
        exit(1);
    }
    char * value = strdup("text/plain");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header != NULL, "Expected: http_header != NULL", __func__);
    assert(name == http_header->name, "Expected: name == http_header->name", __func__);
    assert(value == http_header->value, "Expected: name == http_header->name", __func__);
    http_header_free(http_header);
    printf("Test %s passed!\n", __func__);
}

void http_header_create_name_null_test() {
    char * name = NULL;
    char * value = strdup("text/plain");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header == NULL, "Expected: http_header == NULL", __func__);
    free(value);
    printf("Test %s passed!\n", __func__);
}

void http_header_create_name_invalid_test() {
    // Forced on purpose invalid header name containing invalid character
    char * name = strdup("\x01\x02\x03\x04");
    if(name == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the name variable", __func__);
        exit(1);
    }
    char * value = strdup("text/plain");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header == NULL, "Expected: http_header == NULL", __func__);
    free(name);
    free(value);
    printf("Test %s passed!\n", __func__);
}

void http_header_create_value_null_test() {
    char * name = strdup("Accept");
    if(name == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the name variable", __func__);
        exit(1);
    }
    char * value = NULL;
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header == NULL, "Expected: http_header == NULL", __func__);
    free(name);
    printf("Test %s passed!\n", __func__);
}

void http_header_create_all_null_test() {
    char * name = NULL;
    char * value = NULL;
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header == NULL, "Expected: http_header == NULL", __func__);
    printf("Test %s passed!\n", __func__);
}

void http_header_get_name_test() {
    char * name = strdup("Accept");
    if(name == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the name variable", __func__);
        exit(1);
    }
    char * value = strdup("text/plain");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpHeader * http_header = http_header_create(name, value);
    assert(strcmp(http_header->name, name) == 0, "Expected: http_header->name === name", __func__);
    http_header_free(http_header);
    printf("Test %s passed!\n", __func__);
}

void http_header_get_value_test() {
    char * name = strdup("Accept");
    if(name == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the name variable", __func__);
        exit(1);
    }
    char * value = strdup("text/plain");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpHeader * http_header = http_header_create(name, value);
    assert(strcmp(http_header->value, value) == 0, "Expected: http_header->value === value", __func__);
    http_header_free(http_header);
    printf("Test %s passed!\n", __func__);
}

int main() {
    fclose(stderr);
    http_header_create_ok_test();
    http_header_create_name_null_test();
    http_header_create_name_invalid_test();
    http_header_create_value_null_test();
    http_header_create_all_null_test();
    http_header_get_name_test();
    http_header_get_value_test();
}