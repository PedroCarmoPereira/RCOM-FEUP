#ifndef FTP_H
#define FTP_H

#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "rfc1738url.h"

#define CONTROL_PORT    21

#define SYSREADY_C      "220"
#define USRLOGIN_C      "230"
#define REQPASSW_C      "331"
#define PASV_C          "227"
#define OPEN_C          "150"
#define CLOSE_C         "226"

struct hostent * host;
int control_socket_fd;
FILE * control_socket_stream;

static inline int getServerPort(int n1, int n2){
    return n1*256 + n2;
}

int getHostInfo(rfc1738url * url);

int sendFTPCmd(char *cmd);

int rcvFTPReply(char* reply);

int openControlSocket(rfc1738url * url);

#endif
