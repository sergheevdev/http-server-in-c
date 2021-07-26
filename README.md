# :computer: Simple HTTP Server in C

[![MIT License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](https://github.com/sergheevdev/http-server-in-c/blob/main/LICENSE)

## Announcement

This server has memory leaks and is pending to be refactored completely whenever I have time, I already designed a new architecture and just need time to implemented, anyway you can check the draft architecture at the **develop** branch of this current project if you are intrested in progress.

## Introduction

**Simple HTTP Server**: is a simple HTTP server implementation attempt in C trying to apply the [RFC2616](https://tools.ietf.org/html/rfc2616) standard.

## Features
- Static resources serving (capable of serving static file resources like html, css, js, jpeg and svg files).
- Multi-threaded handling of requests.

### Warning
- This web server is not production ready, bug free, nor memory leaks free.
- This implementation was just coded for fun, not intendeed to be really efficient.
- The design of the structures and general domain is not the best.
- Memory leaks are present in the current implementation (will be fixed soon).

### Story

This project started as a coding kata for implementing a really simple web server to see
the inner workings of sockets and how the HTTP protocol is implemented.

## Getting started

1. Clone the repository to your local computer.
2. Add execution permissions to **compile.sh**.
3. Run the script.
4. Enjoy our new vulnerable but fast http server :D!

**NOTE**: make sure you have the **gcc** essential compilation packages installed and also **valgrind** (used to check memory leaks)

## License

[MIT](LICENSE) &copy; Serghei Sergheev
