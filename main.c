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
#include <ctype.h>

/**
 * A simple HTTP server implementation in C using RFC2616 (https://tools.ietf.org/html/rfc2616).
 *
 * <B>FEATURES</B>:
 * - Static resources serving (capable of serving static file resources like html, css, js, jpeg and svg files).
 * - Multi-threaded handling of requests.
 *
 * <B>TO-DO</B>:
 * - Fix memory leaks (with valgrind) and ensure all dynamic memory is deallocated when unnecesary.
 * - Fix multithreading issues to allow multiple threads to perform I/O at the same time.
 *   (https://docs.oracle.com/cd/E19455-01/806-5257/6je9h033b/index.html)
 *   "When multiple threads are performing I/O operations at the same time with the same file
 *   descriptor, you might discover that the traditional UNIX I/O interface is not thread safe."
 * - Add operative system default MIME file loading in cache or fallback to this software defaults.
 * - Improve the design and create separate files for responsibilities.
 * - Add custom handlers that could be loaded to a manager to perform custom route handling (like low level controllers).
 * - Improve the structures, like header storage by using a hashmap to allow O(1) time complexity for header lookup
 *   by the given header name.
 * - Abstract away and use a string library.
 *
 * And a lot of more stuff which will probably get refactored whenever I feel like I wanting to suffer :D
 */

#define PUBLIC_FOLDER "/home/server/public"
#define PORT_NUMBER 8080
#define BUFFER_SIZE 4096
#define MAX_CONNECTIONS 20

typedef struct header HttpHeader;
typedef struct request HttpRequest;
typedef struct mime HttpMimeType;

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
    char * body;
};

struct mime {
    char * extension;
    char * mime;
    bool binary;
};


// GLOBAL VARIABLES

// Keeps track of the current number of connections
int current_connections = 0;

// Controls threading actions (i.e. no multiple threads accesing disk)
sem_t lock;


/**
 * Returns a pointer to a new allocated <B>HttpHeader</B> structure, with
 * all its fields initialized to <I>NULL</I>.
 *
 * The returned structure and its contents should be freed by the client.
 *
 * @return a pointer to a new allocated <B>HttpHeader</B> structure, or
 *         <I>NULL</I> if there is no enough space for allocation
 */
HttpHeader * create_http_header() {
    HttpHeader * header = malloc(sizeof(HttpHeader));
    if(header == NULL) {
        fprintf(stderr, "Failed to allocate memory for http header: %s\n", strerror(errno));
        fflush(stderr);
        return NULL;
    }
    header->name = NULL;
    header->value = NULL;
    return header;
}

/**
 * Frees the given <B>HttpHeader</B> structure and its contents. If the given
 * header or any of its fields is <I>NULL</I>, no freeing is performed and
 * that variable is just ignored.
 *
 * @param header a pointer to a <B>HttpHeader</B>
 */
void free_http_header(HttpHeader * header) {
    if(header == NULL) return;
    if(header->name != NULL) free(header->name);
    if(header->value != NULL) free(header->value);
    free(header);
}

/**
 * Returns a pointer to a new allocated <B>HttpRequest</B> structure, with
 * all its fields initialized to <I>NULL</I>.
 *
 * The returned structure and its contents should be freed by the client.
 *
 * @return a pointer to a new allocated <B>HttpRequest</B> structure, or
 *         <I>NULL</I> if there is no enough space for allocation
 */
HttpRequest * create_http_request() {
    HttpRequest * request = malloc(sizeof(HttpRequest));
    if(request == NULL) {
        fprintf(stderr, "Failed to allocate memory for http request: %s\n", strerror(errno));
        fflush(stderr);
        return NULL;
    }
    request->method = NULL;
    request->uri = NULL;
    request->headers = NULL;
    request->body = NULL;
    return request;
}

/**
 * Frees the given <B>HttpRequest</B> structure and its contents. If
 * the given header or any of its fields is <I>NULL</I>, no freeing
 * is performed and that variable is just ignored.
 *
 * @param request a pointer to a <B>HttpRequest</B>
 */
void free_http_request(HttpRequest * request) {
    if(request == NULL) return;
    if(request->method != NULL) free(request->method);
    if(request->uri != NULL) free(request->uri);
    if(request->body != NULL) free(request->body);
    // Free the linked list of headers
    HttpHeader * header = request->headers;
    HttpHeader * previous = header;
    while(header != NULL) {
        header = header->next;
        free_http_header(previous);
        previous = header;
    }
    free_http_header(previous);
    free(request);
}

/**
 * Attempts to parse the given message into a http request, performing
 * the necessary validations and checks.
 *
 * The returned structure and its contents should be freed by the client.
 *
 * @param message an http message to be parsed
 * @param status the result status code of the parsing
 *
 * @return a new allocated pointer with the parsed <B>HttpRequest</B>
 *         or <I>NULL</I> if there is no enough space for allocation,
 *         or the validation or parsing failed
 */
HttpRequest * parse_http_request(char * message, int * status) {

    // If the parsing and validation passsed successfully
    static const int SUCCESS_CODE = 0;

    // If there was no memory for some allocation (i.e. could not allocate memory for request body, or request method, etc)
    static const int NO_MEMORY_CODE = 1;

    // If the parsed request does not follow rfc2616 http request format (i.e. some piece like request method is missing)
    static const int INVALID_FORMAT_CODE = 2;

    // If any of the values provided in the request was not successfully validated (i.e. invalid http method like "HELLO")
    static const int VALIDATION_FAILED_CODE = 3;

    if(message == NULL) {
        (* status) = INVALID_FORMAT_CODE;
        return NULL;
    }

    // Set default return code to successful
    (* status) = SUCCESS_CODE;

    HttpRequest * http_request = create_http_request();
    if(http_request == NULL) {
        (* status) = NO_MEMORY_CODE;
        return NULL;
    }

    // Duplicate the received message to prevent side effects
    char * to_tokenize = strdup(message);

    // ## 1. PARSING HTTP REQUEST METHOD ##

    char * message_context = to_tokenize;
    char * piece = strtok_r(to_tokenize, " \t\n", &message_context);

    // ### 1.1 Check http method is present  ###
    if(piece == NULL) {
        (* status) = INVALID_FORMAT_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        return NULL;
    }

    char * method = strdup(piece);

    // ### 1.2 Check piece duplication was successful ###
    if(method == NULL) {
        (* status) = NO_MEMORY_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        return NULL;
    }

    // ### 1.3 Ensure the length of the http method is valid ###
    int size = 0;
    char * traversal = method;
    while((* traversal) != '\0' && size <= 7) {
        size++;
        traversal++;
    }
    // Shortest method length = 3 and longest method length = 7, so if the
    // size exceeds longest valid method name size, or is smaller than the
    // shortest method length given input is invalid.
    if(size < 3 || size > 7) {
        (* status) = VALIDATION_FAILED_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        free(method);
        return NULL;
    }

    // RFC2616 request methods: https://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html
    bool is_valid_method =
            strcmp(method, "GET") == 0
            || strcmp(method, "POST") == 0
            || strcmp(method, "DELETE") == 0
            || strcmp(method, "PUT") == 0
            || strcmp(method, "OPTIONS") == 0
            || strcmp(method, "HEAD") == 0
            || strcmp(method, "TRACE") == 0
            || strcmp(method, "CONNECT") == 0;

    // ### 1.4 Ensure http method is a valid defined method in RFC2616 ###
    if(is_valid_method == false) {
        (* status) = VALIDATION_FAILED_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        free(method);
        return NULL;
    }

    http_request->method = method;

    // ## 2. PARSING HTTP REQUEST URI ##

    piece = strtok_r(NULL, " \t", &message_context);

    // ### 2.1 Check http uri is present ###
    if(piece == NULL) {
        (* status) = INVALID_FORMAT_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        return NULL;
    }

    char * uri = strdup(piece);

    // ### 2.2 Check piece duplication was successful ###
    if(uri == NULL) {
        (* status) = NO_MEMORY_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        return NULL;
    }

    // Basic domain and security validations are performed, for security hardening add extra layer of validation
    // @see https://stackoverflow.com/questions/4669692/valid-characters-for-directory-part-of-a-url-for-short-links
    // Valid characters for the uri/path: "a-z A-Z 0-9 . - _ ~ ! $ & ' ( ) * + , ; = : @ % /"
    traversal = uri;
    bool is_valid_uri = true;
    char previous_char = '\0';
    while((* traversal) != '\0' && is_valid_uri) {
        is_valid_uri =
                isalnum((* traversal))
                || (* traversal) == '.'
                || (* traversal) == '-'
                || (* traversal) == '_'
                || (* traversal) == '~'
                || (* traversal) == '!'
                || (* traversal) == '$'
                || (* traversal) == '&'
                || (* traversal) == '\''
                || (* traversal) == '('
                || (* traversal) == ')'
                || (* traversal) == '*'
                || (* traversal) == '+'
                || (* traversal) == ','
                || (* traversal) == ';'
                || (* traversal) == '='
                || (* traversal) == ':'
                || (* traversal) == '@'
                || (* traversal) == '%'
                || (* traversal) == '/';
        // Prevent two dots in a row (most common vulnerability is trying to access unauthorized dirs with ../../)
        is_valid_uri = !(previous_char == '.' && previous_char == (* traversal));
        previous_char = (* traversal);
        traversal++;
    }

    // ### 2.3 Ensure http path components are valid ###
    if(is_valid_uri == false) {
        (* status) = VALIDATION_FAILED_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        free(uri);
        return NULL;
    }

    http_request->uri = uri;

    // ## 3. PARSING HTTP REQUEST PROTOCOL VERSION ##

    piece = strtok_r(NULL, " \t\n", &message_context);

    // ### 3.1 Check http version is present  ###
    if(piece == NULL) {
        (* status) = INVALID_FORMAT_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        return NULL;
    }

    char * version = strdup(piece);

    // ### 3.2 Check piece duplication was successful ###
    if(version == NULL) {
        (* status) = NO_MEMORY_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        return NULL;
    }

    bool is_valid_version = strncmp(version, "HTTP/1.1", 8) == 0 || strncmp(version, "HTTP/1.0", 8) == 0;

    // ### 3.3 Ensure http version is allowed ###
    if(is_valid_version == false) {
        (* status) = VALIDATION_FAILED_CODE;
        free_http_request(http_request);
        free(to_tokenize);
        free(version);
        return NULL;
    }

    http_request->version = version;

    // ## 4. PARSING HTTP REQUEST HEADERS ##

    piece = strtok_r(NULL, "\t\n", &message_context);

    // While there are more headers and no http_request body starting character is found
    while(piece != NULL && strcmp("\r", piece) != 0) {

        char * header = strdup(piece);

        // ### 4.1 Check piece duplication was successful ###
        if(header == NULL) {
            (* status) = NO_MEMORY_CODE;
            free_http_request(http_request);
            free(to_tokenize);
            return NULL;
        }

        // Check header should have only one separator and should be ": "
        int separator_appearances = 0;
        traversal = header;
        previous_char = '\0';
        while((* traversal) != '\0') {
            if(previous_char == ':' && (* traversal) == ' ') {
                separator_appearances++;
            }
            previous_char = (* traversal);
            traversal++;
        }

        // ### 4.2 Validate header has only one separator ###
        bool is_not_header = separator_appearances != 1;
        if(is_not_header) {
            (* status) = VALIDATION_FAILED_CODE;
            free_http_request(http_request);
            free(to_tokenize);
            free(header);
            return NULL;
        }

        // ### 4.3 Ensure header structure allocation was successful ###
        HttpHeader * http_header = create_http_header();
        if(http_header == NULL) {
            (* status) = NO_MEMORY_CODE;
            free_http_request(http_request);
            free(to_tokenize);
            free(header);
            return NULL;
        }

        char * header_context = header;
        char * header_piece = strtok_r(piece, ":", &header_context);

        char * header_name = strdup(header_piece);

        // ### 4.4 Ensure header name duplication was successful ###
        if(header_name == NULL) {
            (* status) = NO_MEMORY_CODE;
            free_http_request(http_request);
            free(to_tokenize);
            free(header);
            return NULL;
        }

        // Check header name contains only: "a-z", "A-Z", "-" characters
        traversal = header_name;
        bool is_header_name_valid = true;
        while((* traversal) != '\0' && is_header_name_valid) {
            is_header_name_valid =
                    ((* traversal) >= 'a' && (* traversal) <= 'z')     // Is lowecase letter
                    || ((* traversal) >= 'A' && (* traversal) <= 'Z')  // Is uppercase letter
                    || ((* traversal) == '-');                         // Standard word separator
            traversal++;
        }

        // ### 4.5 Ensure header name is valid ###
        if(!is_header_name_valid) {
            (* status) = VALIDATION_FAILED_CODE;
            free_http_request(http_request);
            free_http_header(http_header);
            free(to_tokenize);
            free(header);
            free(header_name);
            return NULL;
        }

        http_header->name = header_name;

        header_piece = strtok_r(NULL, " ", &header_context);

        char * header_value = strdup(header_piece);

        // ### 4.6 Ensure header value duplication was successful ###
        if(header_value == NULL) {
            (* status) = NO_MEMORY_CODE;
            free_http_request(http_request);
            free_http_header(http_header);
            free(to_tokenize);
            free(header);
            free(header_name);
            return NULL;
        }

        http_header->value = header_value;

        // Add http header to headers linked list in the http request
        HttpHeader * previous_header = http_request->headers;
        http_header->next = previous_header;
        http_request->headers = http_header;

        free(header);

        piece = strtok_r(NULL, "\t\n", &message_context);
    }

    piece = strtok_r(NULL, "", &message_context);

    // ## 5. PARSING HTTP REQUEST BODY ##

    // ### 5.1 Check if the body is present ###
    if(piece != NULL) {
        char * body = strdup(piece);

        // ### 5.2 Check piece duplication was successful ###
        if(body == NULL) {
            (* status) = NO_MEMORY_CODE;
            free_http_request(http_request);
            free(to_tokenize);
            free(body);
            return NULL;
        }

        http_request->body = body;
    } else {

        // ### 5.3 Assign empty string when body not present ###
        char * empty = malloc(sizeof(char));

        // ### 5.4 Error handling for failed memory allocation ###
        if(empty == NULL) {
            fprintf(stderr, "Failed to allocate memory for http empty body: %s\n", strerror(errno));
            fflush(stderr);
            (* status) = NO_MEMORY_CODE;
            free_http_request(http_request);
            free(to_tokenize);
            return NULL;
        }

        (* empty) = '\0';

        http_request->body = empty;
    }

    free(to_tokenize);

    return http_request;
}

/**
 * Returns a pointer to a new allocated <B>HttpMimeType</B> structure, with
 * all its fields initialized to <I>NULL</I> or its default values.
 *
 * The returned structure and its contents should be freed by the client.
 *
 * @return a pointer to a new allocated <B>HttpMimeType</B> structure, or
 *         <I>NULL</I> if there is no enough space for allocation
 */
HttpMimeType * create_http_mime_type() {
    HttpMimeType * mime_type = malloc(sizeof(HttpMimeType));
    if(mime_type == NULL) {
        fprintf(stderr, "Failed to allocate memory for http mime type: %s\n", strerror(errno));
        fflush(stderr);
        return NULL;
    }
    mime_type->extension = NULL;
    mime_type->mime = NULL;
    mime_type->binary = false;
    return mime_type;
}

/**
 * Frees the given <B>HttpMimeType</B> structure and its contents. If
 * the given mime type or any of its fields is <I>NULL</I>, no freeing
 * is performed and that variable is just ignored.
 *
 * @param mime_type a pointer to a <B>HttpMimeType</B>
 */
void free_http_mime_type(HttpMimeType * mime_type) {
    if(mime_type == NULL) return;
    if(mime_type->extension != NULL) free(mime_type->extension);
    if(mime_type->mime != NULL) free(mime_type->mime);
    free(mime_type);
}

/**
 * Returns a pointer to a new allocated <B>HttpMimeType</B> structure,
 * with all the MIME type information of a given extension.
 *
 * The returned structure and its contents should be freed by the client.
 *
 * @return a pointer to a new allocated <B>HttpMimeType</B> structure,
 *         or <I>NULL</I> if there is no enough space for allocation,
 *         or there is no registered mime for that file extension
 */
HttpMimeType * from_extension_mime_type(char * extension) {
    if(strcmp("html", extension) == 0) {
        HttpMimeType * mime_type = create_http_mime_type();
        if(mime_type == NULL) return NULL;
        mime_type->extension = strdup(extension);
        mime_type->mime = strdup("text/html");
        mime_type->binary = false;
        return mime_type;
    }
    else if(strcmp("css", extension) == 0) {
        HttpMimeType * mime_type = create_http_mime_type();
        if(mime_type == NULL) return NULL;
        mime_type->extension = strdup(extension);
        mime_type->mime = strdup("text/css");
        mime_type->binary = false;
        return mime_type;
    }
    else if(strcmp("js", extension) == 0) {
        HttpMimeType * mime_type = create_http_mime_type();
        if(mime_type == NULL) return NULL;
        mime_type->extension = strdup(extension);
        mime_type->mime = strdup("application/javascript");
        mime_type->binary = false;
        return mime_type;
    }
    else if(strcmp("svg", extension) == 0) {
        HttpMimeType * mime_type = create_http_mime_type();
        if(mime_type == NULL) return NULL;
        mime_type->extension = strdup(extension);
        mime_type->mime = strdup("image/svg+xml");
        mime_type->binary = true;
        return mime_type;
    }
    else if(strcmp("jpeg", extension) == 0 || strcmp("jpg", extension) == 0) {
        HttpMimeType * mime_type = create_http_mime_type();
        if(mime_type == NULL) return NULL;
        mime_type->extension = strdup(extension);
        mime_type->mime = strdup("image/jpeg");
        mime_type->binary = true;
        return mime_type;
    }
    return NULL;
}

/**
 * Returns a new allocated pointer to an <B>array of chars</B> that represents
 * the concatenation of the two provided strings.

 * The returned structure and its contents should be freed by the client.
 *
 * @return a pointer to a new allocated <B>array of chars</B> pointer,
 *         or <I>NULL</I> if there is no enough space for allocation
 */
char * concat_strings(char * first, char * second) {
    char * result = malloc(strlen(first) + strlen(second) + 1);
    if(result == NULL) {
        fprintf(stderr, "Failed to allocate memory for string concatenation: %s\n", strerror(errno));
        fflush(stderr);
        return NULL;
    }
    strcpy(result, first);
    strcat(result, second);
    return result;
}

/**
 * Sends an http header response and status using the specificied socket
 * descriptor. If the response does not involve sending a file pass
 * <I>NULL</I> to the mime type.
 *
 * @param socket_descriptor the descriptor of a open socket
 * @param http_status_code an http status code
 * @param mime_type a mime type that represents the content of a file to be sent
 *
 * @return 0 if the sending was successful and 1 otherwise.
 */
int send_http_header(int socket_descriptor, int http_status_code, HttpMimeType * mime_type) {
    if(http_status_code == 200) {

        // I need a variadic function for concatenation or maybe a library
        // for strings which I don't have right now, when I'll feel like
        // wanting to suffer I'll code one, but for now we have this messy
        // concatenation.

        char * response = strdup("HTTP/1.1 200 OK\r\nContent-Type: ");
        char * concat = concat_strings(response, mime_type->mime);
        free(response);
        response = concat_strings(concat, "\r\n\r\n");
        free(concat);

        send(socket_descriptor, response, strlen(response), 0);
        free(response);

        return 0;
    } else if(http_status_code == 400) {
        char * response = strdup("HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n");
        send(socket_descriptor, response, strlen(response), 0);
        free(response);

        return 0;
    } else if(http_status_code == 404) {
        char * response = strdup("HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n");
        send(socket_descriptor, response, strlen(response), 0);
        free(response);

        return 0;
    } else if(http_status_code == 503) {
        char * response = strdup("HTTP/1.1 503 Service Unavailable\r\nConnection: close\r\n\r\n");
        send(socket_descriptor, response, strlen(response), 0);
        free(response);

        return 0;
    }
    return 1;
}

/**
 * Sends a file to the specificed socket client or sends an error response
 * if the file was not found or any error ocurred, finally closes the
 * socket connection.
 *
 * @param socket_descriptor an open socket descriptor
 * @param file_path the path of the file to be sent
 * @param mime_type the mime of the file to be sent
 */
void send_file(int socket_descriptor, char * file_path, HttpMimeType * mime_type) {
    // Handle binary file sending
    if(mime_type->binary) {

        int file_descriptor = open(file_path, O_RDONLY);

        if(file_descriptor > 0) {
            send_http_header(socket_descriptor, 200, mime_type);

            char buffer[BUFFER_SIZE];
            int bytes;

            // Write the binary file in chunks (using BUFFER_SIZE as the size of the chunk)
            while ((bytes = read(file_descriptor, buffer, BUFFER_SIZE)) > 0) {
                write(socket_descriptor, buffer, bytes);
            }

            close(file_descriptor);
            close(socket_descriptor);
        } else {
            send_http_header(socket_descriptor, 404, NULL);
        }

    }
    // Handle text file sending
    else {

        FILE * file = fopen(file_path, "r");

        if(file != NULL) {
            send_http_header(socket_descriptor, 200, mime_type);

            fseek(file, 0, SEEK_END);
            long bytes_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            char * buffer = malloc(bytes_size * sizeof(char));

            // Read the html file straight to the buffer
            fread(buffer, bytes_size, 1, file);

            // Transmit the buffer directly to the client
            write(socket_descriptor, buffer, bytes_size);
            free(buffer);

            fclose(file);
            close(socket_descriptor);
        } else {
            send_http_header(socket_descriptor, 404, NULL);
        }

    }
}

/**
 * Handles the client request and sends a response.
 *
 * @param socket an open socket to whom send a response.
 *
 * @return a pointer to this method
 */
void * handle_request(void * socket) {

    // Get the socket descriptor.
    int socket_descriptor = * ((int *)socket);

    sem_wait(&lock);

    // If we run out of available connections reject connection
    if(current_connections + 1 > MAX_CONNECTIONS) {
        send_http_header(socket_descriptor, 503, NULL);
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

    char client_message[BUFFER_SIZE];

    // Read the entire client message to the buffer
    int request = recv(socket_descriptor, client_message, BUFFER_SIZE, 0);

    if(request < 0) {
        printf("[Server] Client message reception failed\n");
    }
    else if (request == 0) {
        printf("[Server] Client disconnected unexpectedly and closed the connection\n");
    }
    else {
        int parse_status;
        HttpRequest * http_request = parse_http_request(client_message, &parse_status);

        // Printing status to the console
        printf("\n");
        printf("1. Parse parse_status: %d\n", parse_status);
        if(parse_status == 0) {
            printf("2. Request method: %s\n", http_request->method);
            printf("3. URI: %s\n", http_request->uri);
            printf("4. Http Version: %s\n", http_request->version);
            printf("5. Http Headers:\n");
            HttpHeader * header = http_request->headers;
            while(header != NULL) {
                printf("   - %s : %s\n", header->name, header->value);
                header = header->next;
            }
            printf("6. Body: %s\n", http_request->body);

        }
        printf("\n");
        fflush(stdout);
        // END

        // If the parsing was successful
        if(parse_status == 0) {

            // Concat the requested file path with the public resources folder
            char * file_path = malloc((strlen(PUBLIC_FOLDER) + strlen(http_request->uri)) * sizeof(char));
            strcpy(file_path, PUBLIC_FOLDER);
            strcat(file_path, http_request->uri);

            // Extract the extension of the requested file
            char * to_tokenize = strdup(http_request->uri);
            strtok(to_tokenize, ".");
            char * extension = strtok(NULL, ".");

            // Extract the MIME information using the extracted file extension
            HttpMimeType * mime_type = from_extension_mime_type(extension);
            if(mime_type != NULL) {
                sem_wait(&lock);
                // Send the file to the client or an error response if file was not found
                send_file(socket_descriptor, file_path, mime_type);
                sem_post(&lock);
            } else {
                send_http_header(socket_descriptor, 400, NULL);
            }

            // Free allocated resources
            free(file_path);
            free(to_tokenize);
            free_http_mime_type(mime_type);
            free(http_request);

        } else {
            send_http_header(socket_descriptor, 400, NULL);
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
    listen(socket_descriptor, MAX_CONNECTIONS);

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