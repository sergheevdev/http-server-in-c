/* http-header.h */
#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

struct header {
    char * name;
    char * value;
};

typedef struct header HttpHeader;

HttpHeader * http_header_create(char * name, char * value);
void http_header_free(HttpHeader * header);
const char * http_header_get_name(const HttpHeader * header);
const char * http_header_get_value(const HttpHeader * header);

#endif /* HTTP_HEADER_H */