#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

#include "sock_utils.h"

#define h_addr h_addr_list[0]


int set_sock_nonblock(int sockfd){
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1){
        perror("fcntl get");
        return -1;
    }
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}


int nonblock_connect(const struct sockaddr *addr, socklen_t len){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("Failed to create socket");
        return -1;
    }

    if (set_sock_nonblock(sockfd) < 0){
        return -1;
    }

    int err = connect(sockfd, addr, len);

    if (err < 0 && errno != EINPROGRESS) {
        perror("Failed to connect");
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}


int hostname_connect(const char* hostname, unsigned short port){
    if (hostname == NULL){
        return -1;
    }

    struct hostent* host = gethostbyname(hostname);

    if (host == NULL) {
        perror("Failed to resolve hostname");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    int sockfd = nonblock_connect((struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sockfd < 0) {
        return -1;
    }

    return sockfd;
}
