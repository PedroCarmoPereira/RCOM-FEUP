#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rfc1738parser.h"

int main(int argc, char const *argv[]){

    if (argc != 2) {
        printf("Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        return -1;
    }

    ftp_params test;
    int r = parse_url(argv[1], &test);
    printf("PARSER RET:%d\n", r);    
    printf("PROTOCOL:%s\n", test.protocol);
    if(test.username[0] |= '\0') printf("USERNAME:%s\n", test.username);
    if(test.password[0] |= '\0') printf("PASSWORD:%s\n", test.password);
    printf("HOSTNAME:%s\n", test.hostname);
    printf("URL PATH:%s\n", test.url_path);

    return 0;
}
