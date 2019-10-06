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
    int stop = 0, counter = 0;
    while(!stop){
    	read(fd, &rec, 1);
    	switch (counter){
    		case 0:
    			puts("0");
    			if(rec == SFD) counter++;
    			printf("EXPECTED:%c, REC: %c\n", SFD, rec);
    			break;
    		case 1:
    			puts("1");
    			if(rec == CE_RR) counter++;
    			printf("EXPECTED:%c, REC: %c\n", CE_RR, rec);
    			break;
    		case 2:
    			puts("2");
    			if(rec == SET) counter++;
    			printf("EXPECTED:%c, REC: %c\n", SET, rec);
    			break;
            case 3:
            	puts("3");
            	printf("REC: %c\n", rec);
                counter++;
                break;
            case 4:
            	puts("4");
                if(rec == SFD) {
                	printf("EXPECTED:%c, REC: %c\n", SFD, rec);
                    counter++;
                    puts("Acabou como devia");
                }
                stop = 1;
                break;
            default:
            	puts("default");
                stop = 1;
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
