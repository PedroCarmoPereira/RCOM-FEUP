/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "macros.h"
#include "datalink.h"

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int fd;

int llopen(){
    char rec;
    enum state state = START;
    while(!STOP){
        read(fd, &rec, 1);
        switch (state){
                case START:
                    if(rec == SFD) state = FLAG_RCV;
                    break;
                case FLAG_RCV:
                    if(rec == CE_RR) state = A_RCV;
                    else if (rec != SFD) state = START;                   
            break;
                case A_RCV:
                    if(rec == SET) state = C_RCV;
                    else if (rec == SFD) state = FLAG_RCV;
                    else state = START;
                    break;
                case C_RCV:
                    if (rec == CE_RR ^ SET) state = BCC_RCV;
                    else if (rec == SFD) state = FLAG_RCV;
                    else state = START;
                    break;
                case BCC_RCV:
                    if(rec == SFD) state = END;
                    else state = START;
                    break;
                case END:
                    STOP = 1;
                    puts("Acabou como devia");
                    break;
                default:
                    STOP= 1;
                    break;
            }
    }

    char buf[5] = {SFD, CE_RR, UA, CE_RR ^ UA, SFD};
    write(fd, buf, SUP_SIZE);
    return 0;
}

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

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
      
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    int ret;
    if ((ret = termios_setup(fd, &oldtio)) != 0){
        printf("termios_setup failed with error code:%d\n", ret);
        exit(-1);
    }
	printf("New termios structure set\n");
    llopen();
    sleep(1);
    
    if((ret = termios_reset(fd, &oldtio)) != 0){
        printf("termios_reset failed with error code:%d\n", ret);
        exit(-1);
    }
    close(fd);
    return 0;
}
