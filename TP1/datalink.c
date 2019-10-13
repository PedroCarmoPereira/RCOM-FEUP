#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "datalink.h"

int termios_setup(int fd, struct termios * oldtio){

	if(tcgetattr(fd, oldtio) == -1){
		perror("tcgetattr");
		return -1;
	}

	struct termios newtio;
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 2;   /* inter-character timer unused*/ 
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received*/ 


    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    	perror("tcsetattr");
    	return -2;
    }

    return 0;
}

int termios_reset(int fd, struct termios * oldtio){

	if ( tcsetattr(fd,TCSANOW, oldtio) == -1) {
    	perror("tcsetattr");
    	return -2;
    }

    return 0;
}