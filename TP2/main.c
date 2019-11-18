#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ftp.h"
#include "rfc1738url.h"

int main(int argc, char const *argv[]){

    if (argc != 2) {
        printf("Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        return -1;
    }

    rfc1738url params;
    int r = parse_url(argv[1], &params);

    if(r){
        printf("Unexpected url format, error_code:%d\n", r);
        return -2;
    }

    getHostInfo(&params);
    printf("Host name  : %s\n", host->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)host->h_addr)));


    return 0;
}
