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
#include "http.h"


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




int main(void){

    struct url_parsed* parsed = url_parse("https://httpbin.org");

    
    http_connect(parsed->domain, parsed->port, strcmp(parsed->proto, "https") == 0 ? 1 : 0);

    url_parsed_free(parsed);


    return 0;
}