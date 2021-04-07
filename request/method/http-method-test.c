#include "http-method.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void assert(int condition, char * message) {
    if(condition != 1) {
        printf("%s\n", message);
        exit(1);
    }
}

void http_method_ok_test() {
    char * input = strdup("GET");
    char * http_method = http_method_from_string(input);
    if(http_method == NULL) {
        fprintf(stderr, "[http_method_not_ok] Fatal: Unable to allocate memory for the http_method variable");
        exit(1);
    }
    assert(http_method != NULL, "[http_method_ok] Expected: http_method != NULL");
    free(input);
    free(http_method);
    printf("Test http_method_ok passed!\n");
}

void http_method_not_ok_test() {
    char * input = strdup("PATCH");
    char * http_method = http_method_from_string(input);
    assert(http_method == NULL, "[http_method_not_ok] Expected: http_method == NULL");
    free(input);
    free(http_method);
    printf("Test http_method_not_ok passed!\n");
}

int main() {
    fclose(stderr);
    http_method_ok_test();
    http_method_not_ok_test();
}