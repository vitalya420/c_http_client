#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <liburing.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define QUEUE_DEPTH 256
#define MAX_CONNECTIONS 1024
#define BUFFER_SIZE 1024

struct client_context {
    int socket_fd;
    char buffer[BUFFER_SIZE];
};

int setup_listening_socket(int port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket creation failed");
        exit(1);
    }

    int enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    if (listen(socket_fd, SOMAXCONN) < 0) {
        perror("listen failed");
        exit(1);
    }

    return socket_fd;
}

void event_loop(int listen_fd) {
    struct io_uring ring;
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        perror("io_uring_queue_init failed");
        exit(1);
    }

    // Prepare accept sqe
    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    struct client_context *accept_ctx = malloc(sizeof(struct client_context));
    accept_ctx->socket_fd = listen_fd;

    io_uring_prep_accept(sqe, listen_fd, NULL, NULL, 0);
    io_uring_sqe_set_data(sqe, accept_ctx);

    io_uring_submit(&ring);

    while (1) {
        struct io_uring_cqe *cqe;
        io_uring_wait_cqe(&ring, &cqe);

        struct client_context *ctx = io_uring_cqe_get_data(cqe);

        if (ctx->socket_fd == listen_fd) {
            // New connection
            int client_fd = cqe->res;
            if (client_fd >= 0) {
                // Prepare read for new client
                struct io_uring_sqe *read_sqe = io_uring_get_sqe(&ring);
                struct client_context *read_ctx = malloc(sizeof(struct client_context));
                read_ctx->socket_fd = client_fd;

                io_uring_prep_recv(read_sqe, client_fd, read_ctx->buffer, BUFFER_SIZE, 0);
                io_uring_sqe_set_data(read_sqe, read_ctx);

                // Prepare next accept
                sqe = io_uring_get_sqe(&ring);
                struct client_context *next_accept_ctx = malloc(sizeof(struct client_context));
                next_accept_ctx->socket_fd = listen_fd;

                io_uring_prep_accept(sqe, listen_fd, NULL, NULL, 0);
                io_uring_sqe_set_data(sqe, next_accept_ctx);

                io_uring_submit(&ring);
            }
        } else {
            // Client data received
            if (cqe->res <= 0) {
                // Connection closed or error
                close(ctx->socket_fd);
                free(ctx);
            } else {
                // Process received data
                printf("Received %d bytes\n", cqe->res);

                // Prepare next read
                struct io_uring_sqe *read_sqe = io_uring_get_sqe(&ring);
                io_uring_prep_recv(read_sqe, ctx->socket_fd, ctx->buffer, BUFFER_SIZE, 0);
                io_uring_sqe_set_data(read_sqe, ctx);

                io_uring_submit(&ring);
            }
        }

        io_uring_cqe_seen(&ring, cqe);
    }

    io_uring_queue_exit(&ring);
}

int main() {
    int listen_fd = setup_listening_socket(8080);
    printf("Server listening on port 8080\n");
    event_loop(listen_fd);
    return 0;
}