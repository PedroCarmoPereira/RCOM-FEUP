/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "macros.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;
int main(int argc, char** argv)
{
    int fd,c, res;
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

    tcgetattr(fd,&oldtio); /* save current port settings */

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
	printf("New termios structure set\n");

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
                    if(rec == UA) state = C_RCV;
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

    buf[0] = SFD;
    buf[1] = CE_RR;
    buf[2] = UA;
    buf[3] = buf[2] ^ buf[1];
    buf[4] = SFD;

    write(fd, buf, SUP_SIZE);

    sleep(1);
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
