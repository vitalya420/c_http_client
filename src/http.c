#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
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

void http_connect(const char* host, unsigned short port, char secure){
    /*
        Connect to http server by domain.
        For example connect to httpbin.org

        http_connect("httpbin.org", 80, 0);

        or with SSL

        http_connect("httpbin.org", 443, 1);
    */
    
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

void http_cleanup(http_connection* conn){
    ;
}