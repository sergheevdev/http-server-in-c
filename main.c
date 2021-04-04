#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

//#define PUBLIC_FOLDER "/home/server/public"
#define PUBLIC_FOLDER "/home/sscid/sources"
#define PORT_NUMBER 8080
#define BUFFER_SIZE 4096
#define MAX_CONNECTIONS 10

typedef enum method HttpMethod;
typedef struct header HttpHeader;
typedef struct request HttpRequest;

struct header {
    char * name;
    char * value;
    struct header * next;
};

struct request {
    char * method;
    char * uri;
    char * version;
    struct header * headers;
    void * body;
};

void free_http_header(HttpHeader * header) {
    if(header == NULL) return;
    if(header->name != NULL) free(header->name);
    if(header->value != NULL) free(header->value);
    free(header);
}

void free_http_request(HttpRequest * request) {
    if(request == NULL) return;
    if(request->method != NULL) free(request->method);
    if(request->uri != NULL) free(request->uri);
    HttpHeader * header = request->headers;
    HttpHeader * previous = header;
    while(previous != NULL) {
        header = header->next;
        free_http_header(previous);
        previous = header;
    }
    if(request->body != NULL) free(request->body);
    free(request);
}

HttpRequest * create_http_request() {
    HttpRequest * request = malloc(sizeof(HttpRequest));
    if(request == NULL) {
        fprintf(stderr, "Failed to allocate memory for http request: %s\n", strerror(errno));
        return NULL;
    }
    request->method = NULL;
    request->uri = NULL;
    request->headers = NULL;
    request->body = NULL;
    return request;
}

HttpRequest * parse_http_request(char * message, int * status) {

    static const int SUCCESS_CODE = 0;            // If no parsing or validation errors ocurred
    static const int NO_MEMORY_CODE = 1;          // If there was no memory for some allocation
    static const int INVALID_FORMAT_CODE = 2;     // If the parsed request does not follow rfc2616 http request format
    static const int VALIDATION_FAILED_CODE = 3;  // If the provided input was not successfully validated (i.e. provided unexistent http method)

    if(message == NULL) {
        (* status) = INVALID_FORMAT_CODE;
        return NULL;
    }

    // Set default return code to: successful
    (* status) = SUCCESS_CODE;

    HttpRequest * request = create_http_request();
    if(request == NULL) {
        (* status) = NO_MEMORY_CODE;
        return NULL;
    }

    // Duplicate the received message to prevent side effects
    char * to_tokenize = strdup(message);

    // ## 1. PARSING HTTP REQUEST METHOD ##

    char * piece = strtok(to_tokenize, " \t\n");

    // ### 1.1 Check http method is present  ###
    if(piece == NULL) {
        (* status) = INVALID_FORMAT_CODE;
        free_http_request(request);
        free(to_tokenize);
        return NULL;
    }

    char * method = strdup(piece);

    // ### 1.2 Check piece duplication was successful ###
    if(method == NULL) {
        (* status) = NO_MEMORY_CODE;
        free_http_request(request);
        free(to_tokenize);
        return NULL;
    }

    // ### 1.3 Ensure the length is the method is valid ###
    // Shortest method length = 3 and longest method length = 7
    int size = 0;
    char * traversal = method;
    while((* traversal) != '\0' && size <= 7) {
        size++;
        traversal++;
    }
    // Is size exceeds longest valid method name size given input is invalid
    if(size < 3 || size > 7) {
        (* status) = VALIDATION_FAILED_CODE;
        free_http_request(request);
        free(to_tokenize);
        free(method);
        return NULL;
    }

    // @see https://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html
    bool is_valid_method =
            strcmp(method, "GET") == 0
            || strcmp(method, "POST") == 0
            || strcmp(method, "DELETE") == 0
            || strcmp(method, "PUT") == 0
            || strcmp(method, "OPTIONS") == 0
            || strcmp(method, "HEAD") == 0
            || strcmp(method, "TRACE") == 0
            || strcmp(method, "CONNECT") == 0;

    // ### 1.4 Ensure http method is a valid defined method in rfc2616 ###
    if(is_valid_method == false) {
        (* status) = VALIDATION_FAILED_CODE;
        free_http_request(request);
        free(to_tokenize);
        free(method);
        return NULL;
    }

    request->method = method;

    // ## 2. PARSING HTTP REQUEST URI ##

    // The "piece" should never freed because it points to the original buffer
    piece = strtok(NULL, " \t");
    if(piece == NULL) {
        // Set invalid format status code because no http uri was found in the request
        (* status) = 4;
        // Free the allocated resources
        free_http_request(request);
        free(to_tokenize);
        return NULL;
    }

    char * uri = strdup(piece);
    if(uri != NULL) {

        // TODO: Implement URI validation algorithm

        request->uri = uri;
    } else {
        // Set invalid format status code because no memory was available for allocation
        (* status) = 3;
        // Free the allocated resources
        free_http_request(request);
        free(to_tokenize);
        return NULL;
    }

    // 3. Parsing the http request protocol version
    // The "piece" should never freed because it points to the original buffer
    piece = strtok(NULL, " \t\n");
    if(piece == NULL) {
        // Set invalid format status code because no http protocol version was found in the request
        (* status) = 5;
        // Free the allocated resources
        free_http_request(request);
        free(to_tokenize);
        return NULL;
    }

    char * version = strdup(piece);
    if(version != NULL) {

        // TODO: Implement http protocol version validation algorithm

        request->version = version;
    } else {
        // Set invalid format status code because no memory was available for allocation
        (* status) = 3;
        // Free the allocated resources
        free_http_request(request);
        free(to_tokenize);
        return NULL;
    }

    free(to_tokenize);

    return request;
}


// Keeps track of the current number of connections
int current_connections = 0;

// Controls threading actions (i.e. no multiple threads accesing disk, concurrent connections amount modification)
sem_t lock;

/*
 * REFERENCES:
 * - https://tools.ietf.org/html/rfc2616
 * - https://stackoverflow.com/questions/176409/build-a-simple-http-server-in-c
 * - https://en.wikipedia.org/wiki/Berkeley_sockets
 * TO-DO:
 * - Abstract away "handle_x" (image handlers) to a generic binary file reader
 * - Abstract away "handle_y" (plain text handlers) to a generic text file reader (and not read entire html file to a buffer but send by pieces)
 * - Create request parser, request structure (headers, method, etc) and builder (to prevent manual tokenization -> leaky abstractions)
 * - Create response parser, response structure (response status, content type, etc) and builder (to prevent hardcoded responses)
 * - Allow to add custom request path handlers (like low level controllers, so we could associate a path with a concrete handler)
 * - Use best error handling practices
 * - Test with valgrind to prevent memory leaks
 */

void handle_html(int socket, char * file_path) {

    static const char STATUS_OK[] = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/html\r\n\r\n";
    static const char STATUS_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\n"
                                           "Connection: close\r\n\r\n";

    FILE * file = fopen(file_path, "r");
    if (file != NULL) {
        printf("[Logger] requested html file was found: %s\n", file_path);
        // Find out the file size
        fseek(file, 0, SEEK_END);
        long bytes_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Send successful response header
        send(socket, STATUS_OK, strlen(STATUS_OK), 0);
        char * buffer = malloc(bytes_size * sizeof(char));

        // Read the html file straight to the buffer
        fread(buffer, bytes_size, 1, file);

        // Transmit the buffer directly to the client
        write (socket, buffer, bytes_size);
        free(buffer);

        fclose(file);
        printf("[Logger] requested html file was transmitted: %s\n", file_path);
    }
    else {
        printf("[Logger] requested html file was not found: %s\n", file_path);
        // If the file does not exist
        write(socket, STATUS_NOT_FOUND, strlen(STATUS_NOT_FOUND));
    }

    fflush(stdout);
}

void handle_js(int socket, char * file_path) {

    static const char STATUS_OK[] = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: application/javascript\r\n\r\n";
    static const char STATUS_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\n"
                                           "Connection: close\r\n\r\n";

    FILE * file = fopen(file_path, "r");
    if (file != NULL) {
        printf("[Logger] requested js file was found: %s\n", file_path);
        // Find out the file size
        fseek(file, 0, SEEK_END);
        long bytes_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Send successful response header
        send(socket, STATUS_OK, strlen(STATUS_OK), 0);
        char * buffer = malloc(bytes_size * sizeof(char));

        // Read the html file straight to the buffer
        fread(buffer, bytes_size, 1, file);

        // Transmit the buffer directly to the client
        write (socket, buffer, bytes_size);
        free(buffer);

        fclose(file);
        printf("[Logger] requested js file was transmitted: %s\n", file_path);
    }
    else {
        printf("[Logger] requested js file was not found: %s\n", file_path);
        // If the file does not exist
        write(socket, STATUS_NOT_FOUND, strlen(STATUS_NOT_FOUND));
    }

    fflush(stdout);
}

void handle_css(int socket, char * file_path) {

    static const char STATUS_OK[] = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: text/css\r\n\r\n";
    static const char STATUS_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\n"
                                           "Connection: close\r\n\r\n";

    FILE * file = fopen(file_path, "r");
    if (file != NULL) {
        printf("[Logger] requested css file was found: %s\n", file_path);
        // Find out the file size
        fseek(file, 0, SEEK_END);
        long bytes_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Send successful response header
        send(socket, STATUS_OK, strlen(STATUS_OK), 0);
        char * buffer = malloc(bytes_size * sizeof(char));

        // Read the html file straight to the buffer
        fread(buffer, bytes_size, 1, file);

        // Transmit the buffer directly to the client
        write (socket, buffer, bytes_size);
        free(buffer);

        fclose(file);
        printf("[Logger] requested css file was transmitted: %s\n", file_path);
    }
    else {
        printf("[Logger] requested css file was not found: %s\n", file_path);
        // If the file does not exist
        write(socket, STATUS_NOT_FOUND, strlen(STATUS_NOT_FOUND));
    }

    fflush(stdout);
}

void handle_jpeg(int socket, char * file_path) {

    // Available response statuses
    static const char STATUS_OK[] = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: image/jpeg\r\n\r\n";
    static const char STATUS_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\n"
                                           "Connection: close\r\n\r\n";

    // Try to open and send the requested binary file
    int descriptor = open(file_path, O_RDONLY);
    if(descriptor > 0) {
        printf("[Logger] requested jpeg file was found: %s\n", file_path);
        send(socket, STATUS_OK, strlen(STATUS_OK), 0);
        char buffer[BUFFER_SIZE];
        int bytes;
        while ((bytes = read(descriptor, buffer, BUFFER_SIZE)) > 0) {
            write(socket, buffer, bytes);
        }
        close(descriptor);
        printf("[Logger] requested jpeg file was transmitted: %s\n", file_path);
    } else {
        printf("[Logger] requested jpeg file was not found: %s\n", file_path);
        write(socket, STATUS_NOT_FOUND, strlen(STATUS_NOT_FOUND));
    }

    fflush(stdout);
}

void handle_svg(int socket, char * file_path) {

    // Available response statuses
    static const char STATUS_OK[] = "HTTP/1.1 200 OK\r\n"
                                    "Content-Type: image/svg+xml\r\n\r\n";
    static const char STATUS_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\n"
                                           "Connection: close\r\n\r\n";

    // Try to open and send the requested binary file
    int descriptor = open(file_path, O_RDONLY);
    if(descriptor > 0) {
        printf("[Logger] requested svg file was found: %s\n", file_path);
        send(socket, STATUS_OK, strlen(STATUS_OK), 0);
        char buffer[BUFFER_SIZE];
        int bytes;
        while ((bytes = read(descriptor, buffer, BUFFER_SIZE)) > 0) {
            write(socket, buffer, bytes);
        }
        close(descriptor);
        printf("[Logger] requested svg file was transmitted: %s\n", file_path);
    } else {
        printf("[Logger] requested svg file was not found: %s\n", file_path);
        write(socket, STATUS_NOT_FOUND, strlen(STATUS_NOT_FOUND));
    }

    fflush(stdout);
}

void *handle_request(void * socket) {

    // Get the socket descriptor.
    int socket_descriptor = * ((int *)socket);

    sem_wait(&lock);

    // If we run out of available connections reject connection
    if(current_connections + 1 > MAX_CONNECTIONS) {
        static const char STATUS_UNAVAILABLE[] = "HTTP/1.1 503 Service Unavailable\r\n"
                                                 "Content-Type: text/html\r\n\r\n"
                                                 "<!doctype html><html><body>Server is busy.</body></html>";
        // Send response
        write(socket_descriptor, STATUS_UNAVAILABLE, strlen(STATUS_UNAVAILABLE));
        sem_post(&lock);
        free(socket);
        shutdown(socket_descriptor, SHUT_RDWR);
        close(socket_descriptor);
        socket_descriptor = -1;
        pthread_exit(NULL);
    } else {
        current_connections++;
    }

    sem_post(&lock);

    // Obtain the request
    char client_message[BUFFER_SIZE];
    int request = recv(socket_descriptor, client_message, BUFFER_SIZE, 0);

    if(request < 0) {
        printf("[Logger] socket message reception failed\n");
    }
    else if (request == 0) {
        printf("[Logger] socket closed because client disconnected unexpectedly\n");
    }
    else {
        // position 0 -> method
        // position 1 -> path
        // position 2 -> protocol version
        char * parsed_request[3];
        // printf("%s", client_message);

        int parse_status;

        HttpRequest * httpRequest = parse_http_request(client_message, &parse_status);

        printf("1. Parse parse_status: %d\n", parse_status);
        if(parse_status == 0) {
            printf("2. Request method: %s\n", httpRequest->method);
            printf("3. URI: %s\n", httpRequest->uri);
            printf("4. Http Version: %s\n", httpRequest->version);
        }
        fflush(stdout);

        parsed_request[0] = strtok(client_message, " \t\n");
        // Only accept GET method
        if (strncmp(parsed_request[0], "GET\0", 4) == 0) {

            // Parse the request header
            parsed_request[1] = strtok(NULL, " \t");
            parsed_request[2] = strtok(NULL, " \t\n");

            // Ensure http protocol version is 1.0 or 1.1
            if (strncmp(parsed_request[2], "HTTP/1.1", 8) != 0) {
                static const char STATUS_BAD_REQUEST[] = "HTTP/1.1 400 Bad Request\r\n"
                                                         "Connection: close\r\n\r\n";
                write(socket_descriptor, STATUS_BAD_REQUEST, strlen(STATUS_BAD_REQUEST));
            }
            else {
                // Parse the filename and extension
                char * tokens[2];
                char * file_name = malloc(strlen(parsed_request[1]) * sizeof(char));
                strcpy(file_name, parsed_request[1]);

                // Getting the file name and extension from tokenizer
                tokens[0] = strtok(file_name, ".");
                tokens[1] = strtok(NULL, ".");

                // Ignore favicon requests
                if(strcmp(tokens[0], "/favicon") == 0 && strcmp(tokens[1], "ico") != 0) {
                    sem_wait(&lock);
                    current_connections--;
                    sem_post(&lock);
                    free(socket);
                    shutdown(socket_descriptor, SHUT_RDWR);
                    close(socket_descriptor);
                    socket_descriptor = -1;
                    pthread_exit(NULL);
                }
                // If no path was provided or just "/"
                else if (tokens[0] == NULL || tokens[1] == NULL) {
                    static const char STATUS_BAD_REQUEST[] = "HTTP/1.1 400 Bad Request\r\n"
                                                             "Connection: close\r\n\r\n";
                    write(socket_descriptor, STATUS_BAD_REQUEST, strlen(STATUS_BAD_REQUEST));
                } else {
                    // If the requested file is not html, js or jpeg, send bad request response parse_status
                    if (strcmp(tokens[1], "html") != 0 && strcmp(tokens[1], "css") != 0
                        && strcmp(tokens[1], "js") != 0 && strcmp(tokens[1], "jpeg") != 0
                        && strcmp(tokens[1], "svg") != 0) {
                        static const char STATUS_BAD_REQUEST[] = "HTTP/1.1 400 Bad Request\r\n"
                                                                 "Connection: close\r\n\r\n";
                        write(socket_descriptor, STATUS_BAD_REQUEST, strlen(STATUS_BAD_REQUEST));
                    }
                    else {
                        // Merge the file name that was requested with the public folder as root directory
                        char * file_path = malloc((strlen(PUBLIC_FOLDER) + strlen(parsed_request[1])) * sizeof(char));
                        strcpy(file_path, PUBLIC_FOLDER);
                        strcat(file_path, parsed_request[1]);

                        if (strcmp(tokens[1], "html") == 0) {

                            // Prevent multiple threads access disk at the same time
                            sem_wait(&lock);
                            handle_html(socket_descriptor, file_path);
                            sem_post(&lock);
                        }
                        else if (strcmp(tokens[1], "css") == 0) {

                            // Prevent multiple threads access disk at the same time
                            sem_wait(&lock);
                            handle_css(socket_descriptor, file_path);
                            sem_post(&lock);
                        }
                        else if (strcmp(tokens[1], "js") == 0) {

                            // Prevent multiple threads access disk at the same time
                            sem_wait(&lock);
                            handle_js(socket_descriptor, file_path);
                            sem_post(&lock);
                        }
                        else if (strcmp(tokens[1], "jpeg") == 0) {

                            // Prevent multiple threads access disk at the same time
                            sem_wait(&lock);
                            handle_jpeg(socket_descriptor, file_path);
                            sem_post(&lock);
                        }
                        else if (strcmp(tokens[1], "svg") == 0) {

                            // Prevent multiple threads access disk at the same time
                            sem_wait(&lock);
                            handle_svg(socket_descriptor, file_path);
                            sem_post(&lock);
                        }

                        // Free the requested file path
                        free(file_path);
                    }
                }
                free(file_name);
            }

        }
    }

    // Close connection and finish thread
    fflush(stdout);
    free(socket);
    shutdown(socket_descriptor, SHUT_RDWR);
    close(socket_descriptor);
    socket_descriptor = -1;
    sem_wait(&lock);
    current_connections--;
    sem_post(&lock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

    sem_init(&lock, 0, 1);

    int socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descriptor == -1) {
        printf("[Server] Could not create the socket\n");
        fflush(stdout);
        return 1;
    }

    struct sockaddr_in server;
    struct sockaddr_in client;

    // Set socket to TCP, assign ADDRESS and set PORT number
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NUMBER);

    // Assign address to server socket
    if (bind(socket_descriptor, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printf("[Server] Binding has failed\n");
        fflush(stdout);
        return 1;
    }
    listen(socket_descriptor, 50);

    printf("[Server] Waiting for incoming connections...\n");
    fflush(stdout);

    int address_length = sizeof(struct sockaddr_in);
    int new_socket;
    while ((new_socket = accept(socket_descriptor, (struct sockaddr *)&client, (socklen_t *)&address_length))) {
        printf("[Server] New connection accepted!\n");
        fflush(stdout);

        int * job_socket = malloc(1);
        * job_socket = new_socket;

        // Creating a new thread that will handle each request
        pthread_t job_thread;
        int result = pthread_create(&job_thread, NULL, handle_request, (void *) job_socket);
        if (result < 0) {
            printf("[Server] Could not create a new thread!\n");
            fflush(stdout);
            return 1;
        }
    }

    return 0;
}