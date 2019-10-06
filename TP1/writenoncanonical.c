/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include "macros.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int try_counter = 0;
int ping = 0;

volatile int STOP=FALSE;

int setUp(int fd){

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    ping = 1;
    int x = 0;
    if(try_counter < TRIES){
        try_counter++;
        printf("%d\n", try_counter);
        char BCC = CE_RR ^ SET;
        char msg[5] = {SFD, CE_RR, SET, BCC, SFD};
        x = write(fd, msg, SUP_SIZE);
        if (x==-1) printf("FUCK: %d\n",errno);
        printf("wrote %d bytes \n", x);
        sleep(1);
        alarm(3);
        //Re-lê
        int counter = 0;
        char rec = 0;
        int response_recieved = 0;
        while(!STOP){
            read(fd, &rec, 1);
            if (ping){
                ping = 0;
                close(fd);
                break;
            } 
            switch (counter){
                case 0:
                    if(rec == SFD) counter++;
                    break;
                case 1:
                    if(rec == CE_RR) counter++;
                    break;
                case 2:
                    if(rec == UA) counter++;
                    break;
                case 3:
                    counter++;
                    break;
                case 4:
                    if(rec == SFD) {
                    counter++;
                    puts("FINISHED AS EXPECTED");
                    }
                    STOP = 1;
                    break;
                default:
                    STOP= 1;
                    break;
            }
        }

    }

    else{
        puts("FAILED TO ESTABLISH CONNECTION, EXITING");
        exit(0);
    }
}

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */


    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
    (void) signal(SIGALRM, setUp);
    setUp(fd);
    sleep(1);
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    return 0;
}
