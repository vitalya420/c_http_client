#ifndef SOCK_UTILS_H
#define SOCK_UTILS_H

#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <stddef.h>

/**
 * Set a socket to non-blocking mode.
 * @param sockfd The socket file descriptor.
 * @return 0 on success, -1 on error (with errno set).
 */
int set_sock_nonblock(int sockfd);

/**
 * Attempt to connect to a socket in non-blocking mode.
 * @param addr Pointer to the socket address structure.
 * @param len Length of the address structure.
 * @return 0 on success, -1 on error (with errno set).
 */
int nonblock_connect(const struct sockaddr *addr, socklen_t len);

/**
 * Connect to a domain using the specified port.
 * @param domain The domain name to connect to.
 * @param port The port number to connect to.
 * @param secure Indicates whether to use a secure connection (1 for secure, 0 for insecure).
 * @return 0 on success, -1 on error (with errno set).
 */
int domain_connect(const char* domain, unsigned short port, char secure);

#endif // SOCK_UTILS_H
