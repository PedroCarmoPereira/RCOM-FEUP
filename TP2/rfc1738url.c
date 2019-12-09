#include <stdio.h>
#include <string.h>

#include "rfc1738url.h"

int parse_url(const char * url_str, rfc1738url * params){
    size_t length = strlen(url_str);
    size_t i = 0;
    while(url_str[i] != ':' && i < length){
        params->protocol[i] = url_str[i];
        i++;
    }

    if(i >= length) return -1;
    
    params->protocol[i] = '\0';
    i++;

    if (url_str[++i] != '/' || url_str[i++] != '/') return -2;

    char tmp[512];
    int j = 0;
    while(url_str[i] != ':' && url_str[i] != '/' && i < length){
        tmp[j] = url_str[i];
        j++;i++;
    }
    tmp[j] = '\0';

    if(i >= length) return -3;

    if(url_str[i] == '/') {
        strcpy(params->hostname, tmp);
        i++; j = 0;
        while (i < length){
            params->url_path[j] = url_str[i];
            i++; j++;
        }
        params->url_path[j] = '\0';  
        params->username[0] = '\0';
        params->password[0] = '\0'; 
    }

    else {
        strcpy(params->username, tmp);
        i++; j = 0;
        while(url_str[i] != '@' && i < length) {
            params->password[j] = url_str[i];
            i++; j++;
        }
        params->password[j] = '\0';
        if(i >= length) return -4;

        j = 0; i++;
        while(url_str[i] != '/' && i < length){
            params->hostname[j] = url_str[i];
            i++; j++;
        }
        params->hostname[j] = '\0';
        if(i >= length) return -5;

        j = 0; i++;
        while(i < length){
            params->url_path[j] = url_str[i];
            j++; i++;
        }
    }

    return 0;
}