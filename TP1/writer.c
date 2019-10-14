/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

#include "datalink.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

volatile int STOP=FALSE;

int try_counter = 0;

int fd;

void handler(){try_counter++;}

int llopen(){
    int x = 0;
    while(try_counter < TRIES && STOP != 2){
        STOP = 0;
        int try = try_counter;
        printf("%d\n", try_counter);
        send_set(fd, 0);
        sleep(1);
        alarm(3);
        //Re-lê
        enum state state = START;
        char rec = 0;
        while(!STOP){
            read(fd, &rec, 1);
            sender_sm(&state, rec);
            if (try != try_counter){
                printf("exiting try %d\n", try);
                STOP = 1;
                break;
            }
        }
    }

    if (STOP != 2){
        puts("FAILED TO ESTABLISH CONNECTION, EXITING");
        return -1;;
    }

    puts("CONNECTION ESTABLISHED");
    return 0;
}

int main(int argc, char** argv)
{
    int c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd <0) {perror(argv[1]); exit(-1); }

     int ret;
    if ((ret = termios_setup(fd, &oldtio)) != 0){
        printf("termios_setup failed with error code:%d\n", ret);
        exit(-1);
    }
    (void) signal(SIGALRM, handler);
    llopen();
    sleep(1);
    if((ret = termios_reset(fd, &oldtio)) != 0){
        printf("termios_reset failed with error code:%d\n", ret);
        exit(-1);
    }
    close(fd);
    return 0;
}
