#include <stdbool.h>

/* http-method.h */

#ifndef HTTP_METHOD_H
#define HTTP_METHOD_H

struct method {
    char * value;
};

typedef struct method HttpMethod;

HttpMethod * http_method_create(char * value);
bool http_method_matches_value(const HttpMethod * method, const char * value);
void http_method_free(HttpMethod * method);
const char * http_method_get_value(const HttpMethod * method);

#endif /* HTTP_METHOD_H */