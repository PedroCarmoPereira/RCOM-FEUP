#include <stdlib.h>
#include <stdio.h>
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>

#include "rfc1738url.h"
#include "ftp.h"


int getHostInfo(rfc1738url * url){
    
    host = gethostbyname(url->hostname);
    
    if(host == NULL){
        printf("Could not resolver hostame:%s", url->hostname);
        return -1;
    }

    return 0;
}

int rcvFTPReply(char *reply){
    
    char *tmp;
    tmp = NULL;

    int bytes_read = 0;

    while(bytes_read < 4 || tmp[0] < '1' || tmp[0] > '5' || tmp[3] == '-') {
        free(tmp);
        tmp = NULL;
        size_t trash = 0;
        bytes_read = getline(&tmp, &trash, control_socket_stream); // keeps \r\n
        if (bytes_read <= 0) {
            printf("Failed to read from socket:%d\n", bytes_read);
            return -1;
        }

        reply[0] = tmp[0];
        reply[1] = tmp[1];
        reply[2] = tmp[2];
        reply[3] = '\0';
    }

    return 0;
}

int openControlSocket(rfc1738url * url){
    
    char reply[4];

    int r;
    if((r = getHostInfo(url))) return r;
      
    char * ipstr = inet_ntoa(*((struct in_addr *)host->h_addr));
    printf("Host name  : %s\n", host->h_name);
    printf("IP Address : %s\n", ipstr);

    control_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (control_socket_fd == -1){
        printf("Failed to open ctrl socket, exiting;");
        return  -2;
    }

    control_socket_stream = fdopen(control_socket_fd, "r");

    struct sockaddr_in in_addr = {0};
    in_addr.sin_family = AF_INET; // IPv4 address
    in_addr.sin_port = htons(CONTROL_PORT); // host byte order -> network byte order
    in_addr.sin_addr.s_addr = inet_addr(ipstr); // to network format

    r = connect(control_socket_fd, (struct sockaddr*)&in_addr, sizeof(in_addr));
    if (r != 0){
        printf("Failed to connect ctrl socket, exiting;");
        return  -3;
    }

    rcvFTPReply(reply);
    if (strcmp(reply, SYSREADY_C) != 0) {
        printf("Unexpected reply from server: %s\n", reply);
        return -4;
    }

    printf("Control socket connected\n");

    return 0;
}