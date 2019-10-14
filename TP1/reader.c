/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include "interface.h"
#include "datalink.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */

extern int fd;

extern struct termios oldtio,newtio;

/*
int llopen(){
    char rec;
    enum state state = START;
    while(!STOP){
        read(fd, &rec, 1);
        reciever_sm(&state, rec);
    }

    send_ua(fd, 0);
    return 0;
}*/

int main(int argc, char** argv)
{
    int c, res;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    llopen(argv[1], 1);
    int ret;
    if((ret = termios_reset(fd, &oldtio)) != 0){
        printf("termios_reset failed with error code:%d\n", ret);
        exit(-1);
    }
    close(fd);
    return 0;
}
