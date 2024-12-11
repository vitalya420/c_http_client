#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/select.h>

#include "parser.h"

#define h_addr h_addr_list[0]

typedef struct
{
    char *http_ver;
    unsigned short status_code;
    char *status_phrase;
    char *headers;
    char *body;

} http_response;


typedef struct
{
    unsigned short is_secure;
    int socket_fd;
    struct sockaddr_in server_addr;
} http_connection;


static int _set_sock_nonblock(int sockfd){
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (-1 == flags){
        perror("fcntl get");
        return -1;
    }
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

static int _do_connect(const struct sockaddr *__addr, socklen_t __len){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > sockfd){
        perror("Failed to create socket");
        return -1;
    }
    printf("Socket created\n");

    if (_set_sock_nonblock(sockfd) < 0){
        return -1;
    }
    printf("Socket non blocking\n");

    int err = connect(sockfd, __addr, __len);
    printf("err %d", err);
    if (err < 0 && errno != EINPROGRESS) {
        perror("Failed to connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int domain_connect(const char* domain, unsigned short port){
    if (NULL == domain){
        return -1;
    }
    struct hostent* host = gethostbyname(domain);

    if (host == NULL) {
        perror("Failed to resolve hostname");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    int sockfd = _do_connect((struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sockfd < 0) {
        return -1;
    }

    return sockfd;
}



http_connection* http_connect(const char* domain, unsigned short port, unsigned short secure){
    http_connection* conn = malloc(sizeof(http_connection));
    if (conn == NULL) {
        perror("Failed to allocate memory for connection");
        return NULL;
    }

    struct hostent* host = gethostbyname(domain);
    if (host == NULL) {
        perror("Failed to resolve hostname");
        free(conn);
        return NULL;
    }

    conn->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn->socket_fd < 0) {
        perror("Failed to create socket");
        free(conn);
        return NULL;
    }

    memset(&conn->server_addr, 0, sizeof(conn->server_addr));
    conn->server_addr.sin_family = AF_INET;
    conn->server_addr.sin_port = htons(port);
    memcpy(&conn->server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(conn->socket_fd, (struct sockaddr*)&conn->server_addr, sizeof(conn->server_addr)) < 0) {
        perror("Failed to connect to server");
        close(conn->socket_fd);
        free(conn);
        return NULL;
    }
    conn->is_secure = 0;

    if(secure){

        conn->is_secure = 1;
    }

    return conn;
}


int http_write(http_connection* conn, const void* request, size_t n){
    return send(conn->socket_fd, request, n, 0);
}

void http_cleanup(http_connection* _http_connection){
    free(_http_connection);
}


void async_connect(){
    
}


void selector_event_loop(fd_set* write_fds, fd_set* read_fds){

}

int main(void){

    struct url_parsed* parsed = url_parse("https://httpbin.org");


    int sockfd = domain_connect(parsed->domain, parsed->port);

    if (sockfd < 0) {
        fprintf(stderr, "Failed to connect to %s:%d\n", parsed->domain, parsed->port);
        return EXIT_FAILURE;
    }

    printf("Connected to %s:%d\n", parsed->domain, parsed->port);

    close(sockfd);
    url_parsed_free(parsed);

    // const char* req = "GET /get HTTP/1.1\r\nHost: httpbin.org\r\n\r\n";
    // printf("%s\n", req);
    // http_connection* conn = http_connect("httpbin.org", 80, 0);
    // if (conn == NULL){
    //     printf("Error while connecting\n");
    // }else{
    //     printf("Connected!\n");
    //     int sent = http_write(conn, req, strlen(req));
    //     printf("%d\n", sent);
    //     http_cleanup(conn);

    // }
    return 0;
}