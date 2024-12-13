#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>

#include "parser.h"

int is_ipv4(const char* buff);
struct sockaddr_in parsed2sockadd(struct url_parsed* parsed);

#endif