#ifndef RFC1738PARSER_H
#define RFC1738PARSER_H

typedef struct ftp_params_t{
    char protocol[255];
    char username[255];
    char password[255];
    char hostname[255];
    char url_path[255];
} ftp_params;

int parse_url(const char * url_str, ftp_params * params);

#endif
