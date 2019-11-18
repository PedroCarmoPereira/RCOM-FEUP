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

    rfc1738url url;
    int r = parse_url(argv[1], &url);

    if(r){
        printf("Unexpected url format, error_code:%d\n", r);
        return -2;
    }

    openControlSocket(&url);
    if (login(&url) != 0)
        return -1;
    if (passive() != 0)
        return -1;
    if (retrieve(url.url_path) != 0)
        return -1;
    if (receiveData(url.url_path) != 0)
        return -1;

    return 0;
}
