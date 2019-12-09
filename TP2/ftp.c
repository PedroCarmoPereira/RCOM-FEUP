#include <stdlib.h>
#include <stdio.h>
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h> 

#include "rfc1738url.h"
#include "ftp.h"

int getHostInfo(rfc1738url * url){
    
    host = gethostbyname(url->hostname);
    
    if(host == NULL){
        printf("Could not resolve hostame:%s", url->hostname);
        return -1;
    }

    return 0;
}

int sendFTPCmd(char * cmd){

    char *ptr = cmd;
    size_t length = strlen(ptr);
    while (length > 0){
        int i = send(control_socket_fd, ptr, length, 0);
        if (i < 1) {
            printf("Failed to write to socket:%d\n", i);
            return i;
        }
        ptr += i;
        length -= i;
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

/*AUX: SHOULDN'T THIS BE DEFINED SOMEWHERE? LIKE STRING.H?*/
void strrev(char * str){
    char tmp;
    int n = strlen(str);
    for(int i = 0; i < n / 2; i++){
        tmp = str[i];
        str[i] = str[n - i - 1];
        str[n - i - 1] = tmp;
    }
}

void getFilenameFromURL(rfc1738url *url, char * filename){
    int length = strlen(url->url_path);
    int i = length - 1;
    int j = 0;
    while(i >= 0 && url->url_path[i] != '/'){
        filename[j] = url->url_path[i];
        i--; j++;
    }
    filename[j] = '\0';
    strrev(filename);
}

int openDataSocket(char *ip, int port){

    int data_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (data_socket_fd == -1){
        printf("Failed to open ctrl socket, exiting;");
        return  -2;
    }
    struct sockaddr_in in_addr = {0};
    in_addr.sin_family = AF_INET; // IPv4 address
    in_addr.sin_port = htons(port); // host byte order -> network byte order
    in_addr.sin_addr.s_addr = inet_addr(ip); // to network format
    puts("Attempting to connect");
    int r = connect(data_socket_fd, (struct sockaddr*)&in_addr, sizeof(in_addr));
    if (r != 0){
        printf("Failed to connect data socket, returned %d; exiting;\n", r);
        return  -3;
    }
    puts("Connected");
    return data_socket_fd;
}


int login(rfc1738url *url){

    char reply[4];

    char username[255];
    char password[255];
    if(url->username[0] != '\0') strcpy(username, url->username);
    else {
        printf("USERNAME:");
        scanf("%s", username);
    }

    char *user = malloc(20 + strlen(username));
    sprintf(user, "USER %s\r\n", username);
    int r = sendFTPCmd(user);

    free(user);
    if(r) return -1;
    

    rcvFTPReply(reply);
    if (strcmp(reply, REQPASSW_C) != 0){
        printf("Unexpected reply from server on username: %s\n", reply);
        return -2;
    }

    if(url->password[0] != '\0') strcpy(password, url->username);
    else {
        printf("PASSWORD:");
        scanf("%s", password);
    }

    char *pass = malloc(20 + strlen(username));
    sprintf(pass, "PASS %s\r\n", password);
    r = sendFTPCmd(pass);
    free(pass);
    if(r) return -3;

    rcvFTPReply(reply);
    if (strcmp(reply, LOGIN_C) != 0){
        printf("Unexpected reply from server on password: %s\n", reply);
        return -2;
    }

    printf("LOGGED IN\n");

    return 0;
}

int passive() {
    char pasv[7];
    char reply[256];

    char ip[64];
    int port;

    sprintf(pasv, "pasv\r\n");

    if (sendFTPCmd(pasv) != 0)
        return -1;
    puts("pasv");

    /*if (rcvFTPReply(reply) != 0)
        return -1;*/

    int rd = read(control_socket_fd, reply, 256);
    reply[rd] = '\0';
    printf("Reply: %s\n", reply);
    int ret;
    unsigned int ipfield1, ipfield2, ipfield3, ipfield4;
    int portFactor, portRemainder;
    sscanf(reply, "%d Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ret, &ipfield1, &ipfield2, &ipfield3, &ipfield4, &portFactor, &portRemainder);
    
    /*if (ret != PASV_C)
        return -1;*/

    sprintf(ip, "%d.%d.%d.%d", ipfield1, ipfield2, ipfield3, ipfield4);
   
    port = portFactor * 256 + portRemainder;

    printf("Ret: %d \nIp: %s \nPort: %d\n", ret, ip, port);
    data_socket_fd = openDataSocket(ip, port);  

    if (data_socket_fd <= 0)
        return -1;  
    
    return 0;
}

int retrieve(char *filename) {
    char retrieve[1024];

    sprintf(retrieve, "retr %s\r\n", filename);
    
    if (sendFTPCmd(retrieve) != 0)
        return -1;
    
    puts("retr");

    return 0;
}

int receiveData(char *filename) {
    FILE* file = fopen(filename, "w");
    int r;
    char buffer[1024];
    do {
        r = read(data_socket_fd, buffer, 1024);
        fwrite(buffer, r, 1, file);
        printf("R:%d\n", r);
    } while (r != 0);

    fclose(file);

    return 0;
}
