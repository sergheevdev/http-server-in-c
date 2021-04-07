#include "http-method.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void assert(int condition, char * message, char * caller) {
    if(condition != 1) {
        printf("[%s] %s\n", caller, message);
        exit(1);
    }
}

void http_method_create_ok_test() {
    char * value = strdup("POST");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpMethod * http_method = http_method_create(value);
    assert(http_method != NULL, "Expected: http_method != NULL", __func__);
    http_method_free(http_method);
    printf("Test %s passed!\n", __func__);
}

void http_method_create_not_ok_test() {
    char * value = strdup("PATCH");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpMethod * http_method = http_method_create(value);
    assert(http_method == NULL, "Expected: http_method == NULL", __func__);
    free(value);
    printf("Test %s passed!\n", __func__);
}

void http_method_matches_value_ok_test() {
    char * value = strdup("PUT");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpMethod * http_method = http_method_create(value);
    assert(http_method_matches_value(http_method, "PUT") == true, "Expected: http_method->matches_value('PUT') == true", __func__);
    http_method_free(http_method);
    printf("Test %s passed!\n", __func__);
}

void http_method_matches_value_not_ok_test() {
    char * value = strdup("GET");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpMethod * http_method = http_method_create(value);
    assert(http_method_matches_value(http_method, "POST") == false, "Expected: http_method->matches_value('PUT') == false", __func__);
    http_method_free(http_method);
    printf("Test %s passed!\n", __func__);
}

void http_method_get_value_test() {
    char * value = strdup("POST");
    if(value == NULL) {
        fprintf(stderr, "[%s] Fatal: Unable to allocate memory for the value variable", __func__);
        exit(1);
    }
    HttpMethod * http_method = http_method_create(value);
    assert(strcmp(value, http_method_get_value(http_method)) == 0, "Expected: value == http_method->get_value()", __func__);
    http_method_free(http_method);
    printf("Test %s passed!\n", __func__);
}

int main() {
    fclose(stderr);
    http_method_create_ok_test();
    http_method_create_not_ok_test();
    http_method_matches_value_ok_test();
    http_method_matches_value_not_ok_test();
    http_method_get_value_test();
}