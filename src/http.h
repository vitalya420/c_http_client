#ifndef HTTP_H
#define HTTP_H

#include <pthread.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


typedef struct {
    int sockfd;
    char* host;
    unsigned short port;
    char connected;
    SSL_CTX* ssl_ctx;
    SSL *ssl;
} http_connection;

typedef struct {
    int status_code;
    char* status_message;
    char** headers;
    int header_count;
    char* body;
    size_t body_length;
} HTTPResponse;

static SSL_CTX* _ctx;
static pthread_mutex_t ctx_mutex;

static SSL_CTX* create_context();
http_connection* http_connect(const char* host, unsigned short port, unsigned short secure);

void http_close(http_connection* http); // Closes sockets and does clean up of allocated memory
void http_connection_cleanup(http_connection* http); // does not close socket
void http_connect_cb(const char* host, unsigned short port, unsigned short secure, void (*callback)(http_connection*));
HTTPResponse* http_recv(int sockfd);
char* read_line(int sockfd);
void free_http_response(HTTPResponse* response);

#endif