#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


#include "http.h"
#include "sock_utils.h"


static SSL_CTX* _ctx = NULL;
static pthread_mutex_t ctx_mutex = PTHREAD_MUTEX_INITIALIZER;


static SSL_CTX* create_context() {
    if (_ctx != NULL) {
        return _ctx;
    }

    pthread_mutex_lock(&ctx_mutex);
    if (_ctx == NULL) {
        _ctx = SSL_CTX_new(TLS_client_method());
        if (!_ctx) {
            perror("Unable to create SSL context");
            ERR_print_errors_fp(stderr);
            pthread_mutex_unlock(&ctx_mutex);
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_unlock(&ctx_mutex);
    return _ctx;
}

http_connection* http_connect(const char* host, unsigned short port, unsigned short secure){
    /*
        Connect to http server by domain.
        For example connect to httpbin.org

        http_connect("httpbin.org", 80, 0);

        or with SSL

        http_connect("httpbin.org", 443, 1);
    */

    printf("Connect to %s %d %d\n", host, port, secure);
    if (host == NULL){
        return NULL;
    }
    
    int sockfd = hostname_connect(host, port);
    if (sockfd < 0){
        return NULL;
    }
    http_connection* conn = malloc(sizeof(http_connection));
    conn->host = (char *)malloc(strlen(host) + 1);
    strcpy(conn->host, host);
    conn->sockfd = sockfd;

    if (secure){
        conn->ssl = SSL_new(create_context());
        SSL_set_fd(conn->ssl, sockfd);
    }

    return conn;
}

int http_write(http_connection* conn, void* buff, size_t n){
    /* 
        Send bytes to http connection
    */
    if(conn->ssl != NULL){
        return SSL_write(conn->ssl, buff, n);
    }
    return send(conn->sockfd, buff, n, 0);
}

void http_close(http_connection* conn){
    if (conn->sockfd){
        close(conn->sockfd);
    }
    http_connection_cleanup(conn);
}

void http_connection_cleanup(http_connection* conn){
    if (conn != NULL){
        if (conn -> ssl != NULL){
            SSL_free(conn->ssl);
        }
        free(conn->host);
        free(conn);
    }
}


void http_connect_cb(const char* host, unsigned short port, unsigned short secure, void (*callback)(http_connection*))
{
    callback(NULL);
}

char* read_line(int sockfd) {
    char* line = NULL;
    size_t line_size = 0;
    size_t line_capacity = 0;
    int ch;

    while ((ch = recv(sockfd, NULL, 0, MSG_PEEK)) > 0) {
        char buffer[1];
        ssize_t bytes_read = recv(sockfd, buffer, 1, 0);
        
        if (bytes_read <= 0) break;

        // Expand line buffer if needed
        if (line_size + 1 >= line_capacity) {
            line_capacity = line_capacity == 0 ? 64 : line_capacity * 2;
            char* new_line = realloc(line, line_capacity);
            if (!new_line) {
                free(line);
                return NULL;
            }
            line = new_line;
        }

        line[line_size++] = buffer[0];
        
        // Check for end of line
        if (buffer[0] == '\n' && line_size > 1 && line[line_size-2] == '\r') {
            line[line_size] = '\0';
            break;
        }
    }

    return line;
}

// Function to parse HTTP response
HTTPResponse* http_recv(int sockfd) {
    HTTPResponse* response = malloc(sizeof(HTTPResponse));
    if (!response) return NULL;
    
    // Initialize response structure
    memset(response, 0, sizeof(HTTPResponse));

    // Read status line
    char* status_line = read_line(sockfd);
    if (!status_line) {
        free_http_response(response);
        return NULL;
    }

    // Parse status line
    char* protocol = strtok(status_line, " ");
    char* status_code_str = strtok(NULL, " ");
    char* status_message = strtok(NULL, "\r\n");

    if (!protocol || !status_code_str || !status_message) {
        free(status_line);
        free_http_response(response);
        return NULL;
    }

    response->status_code = atoi(status_code_str);
    response->status_message = strdup(status_message);
    free(status_line);

    // Read headers
    response->headers = malloc(sizeof(char*) * 32);  // Initial capacity
    response->header_count = 0;

    while (1) {
        char* header_line = read_line(sockfd);
        
        // Empty line indicates end of headers
        if (strcmp(header_line, "\r\n") == 0) {
            free(header_line);
            break;
        }

        // Reallocate headers if needed
        if (response->header_count >= 32) {
            free(header_line);
            break;
        }

        response->headers[response->header_count++] = header_line;
    }

    // Find Content-Length header
    size_t content_length = 0;
    for (int i = 0; i < response->header_count; i++) {
        if (strncasecmp(response->headers[i], "Content-Length:", 15) == 0) {
            content_length = atoi(response->headers[i] + 15);
            break;
        }
    }

    // Read body
    if (content_length > 0) {
        response->body = malloc(content_length + 1);
        ssize_t total_read = 0;
        
        while (total_read < content_length) {
            ssize_t bytes_read = recv(sockfd, response->body + total_read, 
                                      content_length - total_read, 0);
            if (bytes_read <= 0) break;
            total_read += bytes_read;
        }
        
        response->body[content_length] = '\0';
        response->body_length = content_length;
    }

    return response;
}

void free_http_response(HTTPResponse* response) {
    if (response) {
        free(response->status_message);
        
        // Free headers
        if (response->headers) {
            for (int i = 0; i < response->header_count; i++) {
                free(response->headers[i]);
            }
            free(response->headers);
        }
        
        free(response->body);
        free(response);
    }
}