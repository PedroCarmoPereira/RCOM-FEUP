/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

#include "interface.h"
#include "datalink.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

extern int fd;

extern struct termios oldtio,newtio;

int main(int argc, char** argv)
{
    int c, res;

    char buf[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    int ret;
    llopen_sender(argv[1]);
    sleep(1);
    if((ret = termios_reset(fd, &oldtio)) != 0){
        printf("termios_reset failed with error code:%d\n", ret);
        exit(-1);
    }
    close(fd);
    return 0;
}
