/* http-request.h */
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

struct request {
    HttpMethod * method;
    HttpUri * uri;
    HttpVersion * version;
    HttpHeader * header;
    HttpBody * body;
};

typedef struct request HttpRequest;

HttpRequest * http_request_create(HttpMethod * method, HttpUri * uri, HttpVersion * version, HttpHeader * header, HttpBody * body);
void http_request_free(HttpRequest * request);
const HttpMethod * http_request_get_method(const HttpRequest * request);
const HttpUri * http_request_get_uri(const HttpRequest * request);
const HttpVersion * http_request_get_version(const HttpRequest * request);
const HttpHeader * http_request_get_headers(const HttpRequest * request);
const HttpBody * http_request_get_body(const HttpRequest * request);

#endif /* HTTP_REQUEST_H */