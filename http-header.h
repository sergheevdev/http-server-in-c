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
const char * http_header_get_name(HttpHeader * header);
const char * http_header_get_value(HttpHeader * header);

#endif /* HTTP_HEADER_H */