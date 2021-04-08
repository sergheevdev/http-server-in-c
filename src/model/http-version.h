/* http-version.h */
#ifndef HTTP_VERSION_H
#define HTTP_VERSION_H

struct version {
    char * value;
};

typedef struct version HttpVersion;

HttpVersion * http_version_create(char * value);
void http_version_free(HttpVersion * version);
const char * http_version_get_value(const HttpVersion * version);

#endif /* HTTP_VERSION_H */