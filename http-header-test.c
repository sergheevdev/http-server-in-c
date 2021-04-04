#include "http-header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void assert(int condition, char * message) {
    if(condition != 1) {
        printf("%s\n", message);
        exit(1);
    }
}

void http_header_create_ok_test() {
    char * name = strdup("Accept");
    char * value = strdup("text/plain");
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header != NULL, "[http_header_create_test_ok] Expected: http_header != NULL");
    assert(name == http_header->name, "[http_header_create_test_ok] Expected: name == http_header->name");
    assert(value == http_header->value, "[http_header_create_test_ok] Expected: name == http_header->name");
    http_header_free(http_header);
    printf("Test http_header_create_test_ok passed!\n");
}

void http_header_create_name_null_test() {
    char * name = NULL;
    char * value = strdup("text/plain");
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header == NULL, "[http_header_create_test_name_null] Expected: http_header == NULL");
    free(value);
    printf("Test http_header_create_test_name_null passed!\n");
}

void http_header_create_name_invalid_test() {
    // Forced on purpose invalid header name containing "#" invalid character
    char * name = strdup("Strange#Header-Name");
    char * value = strdup("text/plain");
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header == NULL, "[http_header_create_name_invalid_test] Expected: http_header == NULL");
    free(name);
    free(value);
    printf("Test http_header_create_name_invalid_test passed!\n");
}

void http_header_create_value_null_test() {
    char * name = strdup("Accept");
    char * value = NULL;
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header == NULL, "[http_header_create_test_value_null] Expected: http_header == NULL");
    free(name);
    printf("Test http_header_create_test_value_null passed!\n");
}

void http_header_create_all_null_test() {
    char * name = NULL;
    char * value = NULL;
    HttpHeader * http_header = http_header_create(name, value);
    assert(http_header == NULL, "[http_header_create_test_all_null] Expected: http_header == NULL");
    printf("Test http_header_create_test_all_null passed!\n");
}

void http_header_get_name_test() {
    char * name = strdup("Accept");
    char * value = strdup("text/plain");
    HttpHeader * http_header = http_header_create(name, value);
    assert(strcmp(http_header->name, name) == 0, "[http_header_get_name_test] Expected: http_header->name === name");
    http_header_free(http_header);
    printf("Test http_header_get_name_test passed!\n");
}

void http_header_get_value_test() {
    char * name = strdup("Accept");
    char * value = strdup("text/plain");
    HttpHeader * http_header = http_header_create(name, value);
    assert(strcmp(http_header->value, value) == 0, "[http_header_get_value_test] Expected: http_header->value === value");
    http_header_free(http_header);
    printf("Test http_header_get_value_test passed!\n");
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