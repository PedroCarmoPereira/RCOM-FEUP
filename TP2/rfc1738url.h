#ifndef RFC1738PARSER_H
#define RFC1738PARSER_H

typedef struct rfc1738url_t{
    char protocol[255];
    char username[255];
    char password[255];
    char hostname[255];
    char url_path[255];
} rfc1738url;

int parse_url(const char * url_str, rfc1738url * params);

#endif
