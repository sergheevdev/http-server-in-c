#include "http-method.h"
#include <stdio.h>
#include <string.h>

const char * HTTP_METHOD_OPTIONS = "OPTIONS";
const char * HTTP_METHOD_GET = "GET";
const char * HTTP_METHOD_HEAD = "HEAD";
const char * HTTP_METHOD_POST = "POST";
const char * HTTP_METHOD_PUT = "PUT";
const char * HTTP_METHOD_DELETE = "DELETE";
const char * HTTP_METHOD_TRACE = "TRACE";
const char * HTTP_METHOD_CONNECT = "CONNECT";

char * http_method_from_string(char * method) {
    if(strcmp(HTTP_METHOD_OPTIONS, method) == 0) return strdup(method);
    if(strcmp(HTTP_METHOD_GET, method) == 0) return strdup(method);
    if(strcmp(HTTP_METHOD_HEAD, method) == 0) return strdup(method);
    if(strcmp(HTTP_METHOD_POST, method) == 0) return strdup(method);
    if(strcmp(HTTP_METHOD_PUT, method) == 0) return strdup(method);
    if(strcmp(HTTP_METHOD_DELETE, method) == 0) return strdup(method);
    if(strcmp(HTTP_METHOD_TRACE, method) == 0) return strdup(method);
    if(strcmp(HTTP_METHOD_CONNECT, method) == 0) return strdup(method);
    fprintf(stderr, "Unable parse provided method using http_method_from_string: %s", method);
    return NULL;
}
