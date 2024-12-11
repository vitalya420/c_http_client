#ifndef PARSER_H
#define PARSER_H

struct url_parsed {
    char* proto;
    char* domain;
    unsigned short port;
};

struct url_parsed* url_parse(const char* url);
void url_parsed_free(struct url_parsed* parsed_url);

#endif // PARSER_H