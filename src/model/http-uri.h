/* http-uri.h */
#ifndef HTTP_URI_H
#define HTTP_URI_H

struct uri {
    char * value;
};

typedef struct uri HttpUri;

HttpUri * http_uri_create(char * value);
void http_uri_free(HttpUri * uri);
const char * http_uri_get_value(const HttpUri * uri);

#endif /* HTTP_URI_H */