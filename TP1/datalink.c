#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "datalink.h"

extern int STOP0;
extern int STOP1;
extern int STOP2;

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

int send_set(int fd, int debug){
    char BCC = CE_RR ^ SET;
    char set[5] = {SFD, CE_RR, SET, BCC, SFD};
    int w = write(fd, set, SUP_SIZE);
    if(debug){
        if (w == -1) {
            printf("Write failed with errno:%d\n",errno);
            return -1;
        }
        else {
            puts("Sent set:");
            printf("Wrote: %d bytes \n", w);
        }
    }

    sleep(1);
    return 0;
}

int send_ua(int fd, int debug){
    char ua[5] = {SFD, CE_RR, UA, CE_RR ^ UA, SFD};
    int w = write(fd, ua, SUP_SIZE);
    if(debug){
        if (w == -1) {
            printf("Write failed with errno:%d\n",errno);
            return -1;
        }
        else {
            puts("Sent set:");
            printf("Wrote: %d bytes \n", w);
        }
    }

    return 0;
}

void sender_set_sm(state *state, char rec){
    
    switch (*state){
        case START:
            if(rec == SFD) *state = FLAG_RCV;
            break;
        case FLAG_RCV:
            if(rec == CE_RR) *state = A_RCV;
            else if (rec != SFD) *state = START;
            break;
        case A_RCV:
            if(rec == UA) *state = C_RCV;
            else if (rec == SFD) *state = FLAG_RCV;
            else *state = START;
            break;
        case C_RCV:
            if (rec == CE_RR ^ UA) *state = BCC_RCV;
            else if (rec == SFD) *state = FLAG_RCV;
            else *state = START; 
            break;
        case BCC_RCV:
            if(rec == SFD) *state = END;
            else *state = START;
            break;
        case END:
            STOP0 = 2;
            alarm(0);
            break;
        default:
            STOP0 = 1;
            break;
    }
}


void reciever_set_sm(state *state, char rec){

     switch (*state){
        case START:
            if(rec == SFD) *state = FLAG_RCV;
            break;
        case FLAG_RCV:
            if(rec == CE_RR) *state = A_RCV;
            else if (rec != SFD) *state = START;                   
            break;
        case A_RCV:
            if(rec == SET) *state = C_RCV;
            else if (rec == SFD) *state = FLAG_RCV;
            else *state = START;
            break;
        case C_RCV:
            if (rec == CE_RR ^ SET) *state = BCC_RCV;
            else if (rec == SFD) *state = FLAG_RCV;
            else *state = START;
            break;
        case BCC_RCV:
            if(rec == SFD) *state = END;
            else *state = START;
            break;
        case END:
            STOP0 = 1;
            puts("SET RECIEVED");
            break;
        default:
            STOP0 = 1;
            break;
    }
}

int send_disc(int fd, int debug, int t_or_r){
    char A;
    if(!t_or_r) A = CE_RR;
    else A = CR_RE;
    char BCC = A ^ SET;
    char disc[5] = {SFD, A, DISC, BCC, SFD};
    int w = write(fd, disc, SUP_SIZE);
    if(debug){
        if (w == -1) {
            printf("Write failed with errno:%d\n",errno);
            return -1;
        }
        else {
            puts("Sent set:");
            printf("Wrote: %d bytes \n", w);
        }
    }

    sleep(1);
    return 0;
}

void disc_sm(state *state, char rec, int t_or_r){
    char A;
    if(!t_or_r) A = CR_RE;
    else A = CE_RR;
    switch (*state){
        case START:
            if(rec == SFD) *state = FLAG_RCV;
            break;
        case FLAG_RCV:
            if(rec == A) *state = A_RCV;
            else if (rec != SFD) *state = START;                   
            break;
        case A_RCV:
            if(rec == DISC) *state = C_RCV;
            else if (rec == SFD) *state = FLAG_RCV;
            else *state = START;
            break;
        case C_RCV:
            if (rec == A ^ DISC) *state = BCC_RCV;
            else if (rec == SFD) *state = FLAG_RCV;
            else *state = START;
            break;
        case BCC_RCV:;
            if(rec == SFD) *state = END;
            else *state = START;
            break;
        case END:
            if(!t_or_r) STOP1 = 2;
            else STOP2 = 2;
            puts("DISC RECIEVED");
            break;
        default:
            if(!t_or_r) STOP1 = 1;
            else STOP2 = 1;
            break;
    }
}