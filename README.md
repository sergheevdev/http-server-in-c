# :computer: Simple HTTP Server in C

[![MIT License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](https://github.com/sergheevdev/http-server-in-c/blob/main/LICENSE)

**This branch is a work in progress**

Nothing stable in here... only unit tests and readable code

## TODO

Common:

- Create http version element

Request:

- Create elements that compose the http request
- Create an http request builder
- Create an http request parses that builds up the request
- Create a header that represents http methods enumeration (RFC2616 methods)

Response:

- Create elements that compose the http response
- Create a header that represents http response statuses enumeration (RFC 2616 statuses)

Basic idea:

1. Http Server -> Http Request Parser (Parse the request)
2. Http Server -> Http Router (Pass the request to the router)
3. Http Router -> Http Handler Manager (Routes passes to the handlers manager)

The Handler Manager will contain all the registered handlers

Create some default handlers like static content delivery and let developers create their own high level layer
and handle that to some controllers but wrapped in high level decorated "classes", well, here structures.