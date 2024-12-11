#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"


struct url_parsed* url_parse(const char* url){
    if (url == NULL) {
        return NULL;
    }

    struct url_parsed* parsed_url = malloc(sizeof(struct url_parsed));
    if (parsed_url == NULL) {
        return NULL;
    }

    parsed_url->proto = NULL;
    parsed_url->domain = NULL;

    const char* proto_end = strstr(url, "://");

        if (proto_end) {
        size_t proto_len = proto_end - url;
        parsed_url->proto = malloc(proto_len + 1);
        if (parsed_url->proto == NULL) {
            free(parsed_url);
            return NULL;
        }
        strncpy(parsed_url->proto, url, proto_len);
        parsed_url->proto[proto_len] = '\0';

        url = proto_end + 3;
    } else {
        parsed_url->proto = malloc(5);
        if (parsed_url->proto == NULL) {
            free(parsed_url);
            return NULL;
        }
        strcpy(parsed_url->proto, "http");
    }

    if (strcmp(parsed_url->proto, "https") == 0) {
        parsed_url->port = 443;
    } else {
        parsed_url->port = 80;
    }

    const char* port_start = strchr(url, ':');
    const char* path_start = strchr(url, '/');

    size_t domain_len;
    if (port_start && (!path_start || port_start < path_start)) {
        domain_len = port_start - url;
        parsed_url->port = (unsigned short)atoi(port_start + 1);
    } else if (path_start) {
        domain_len = path_start - url;
    } else {
        domain_len = strlen(url);
    }

    parsed_url->domain = malloc(domain_len + 1);
    if (parsed_url->domain == NULL) {
        free(parsed_url->proto);
        free(parsed_url);
        return NULL;
    }
    strncpy(parsed_url->domain, url, domain_len);
    parsed_url->domain[domain_len] = '\0';

    return parsed_url;
}


void url_parsed_free(struct url_parsed* parsed_url) {
    if (parsed_url) {
        free(parsed_url->proto);
        free(parsed_url->domain);
        free(parsed_url);
    }
    parsed_url = NULL;
}
