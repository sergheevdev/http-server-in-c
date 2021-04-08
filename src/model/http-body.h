/* http-body.h */
#ifndef HTTP_BODY_H
#define HTTP_BODY_H

struct body {
    char * value;
};

typedef struct body HttpBody;

HttpBody * http_body_create_empty();
HttpBody * http_body_create(char * value);
void http_body_free(HttpBody * body);
const char * http_body_get_value(const HttpBody * body);

#endif /* HTTP_BODY_H */