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
    sleep(3);
    /** 
     * ! Testing send_frame();
     * ----------------------
    */
    /*char test[4] = {0x11, 0x7e, 0x7d, 0x02};
    char stuffed[8];
    int x = byte_stuffer(test, 4, stuffed);
    for(int i = 0; i < x; i++) printf(" %x", stuffed[i]);
    puts("");
    char destuffed[8];
    int y = byte_destuffer(stuffed, x, destuffed);
    for(int i = 0; i < y; i++) printf(" %x", destuffed[i]);
    puts("");*/
    /*------------------------*/
    
    llclose_reciever(fd);
    int ret;
    if((ret = termios_reset(fd, &oldtio)) != 0){
        printf("termios_reset failed with error code:%d\n", ret);
        exit(-1);
    }*/
    close(fd);
    return 0;
}
