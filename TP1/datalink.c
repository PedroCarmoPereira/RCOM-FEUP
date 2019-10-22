#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "datalink.h"

extern int STOP0;
extern int STOP1;
extern int STOP2;
char CONTROL_FIELD = 0x00;

int termios_setup_writer(int fd, struct termios * oldtio){

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

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused*/ 
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received*/ 


    tcflush(fd, TCIFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
    	perror("tcsetattr");
    	return -2;
    }

    return 0;
}

int termios_setup_reader(int fd, struct termios * oldtio){

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

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused*/ 
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

int send_frame(int fd, char* data, int data_length, int s) {
    char* data_bcc = malloc(1);
    get_data_bcc(data, data_length, data_bcc);

    char *byte_stuffed_data = malloc(2 * data_length);
    int new_length = byte_stuffer(data, data_length, byte_stuffed_data);

    printf("new length: %d \n", new_length);
    
    int frame_size = sizeof(char) * 6 + new_length;
    printf("frame size: %d \n", frame_size);
    char* frame = malloc(frame_size);
    
    if (build_frame(frame, frame_size, byte_stuffed_data, new_length, data_bcc) != 0){
        //error
        free(frame);
        free(byte_stuffed_data);
        free(data_bcc);

        return -1;
    }    

    int w = write(fd, frame, frame_size);
  
    for (int i = 0; i < frame_size; i++) {
        printf("%x  ", frame[i]);
    }

   if (CONTROL_FIELD == 0x00){
       CONTROL_FIELD = 0x40;
   } else if (CONTROL_FIELD == 0x40) {
       CONTROL_FIELD = 0x00;
   }

    free(frame);
    free(byte_stuffed_data);
    free(data_bcc);

    return w;
}

build_frame(char *frame, int frame_size, char *data, int data_size, char *data_bcc) {
    /* frame - FLAG | Endereço | Controlo | DADOS | FLAG */
    if (frame_size > data_size + 6)
        return -1;

    frame[0] = SFD;
    frame[1] = CE_RR;
    frame[2] = CONTROL_FIELD;
    frame[3] = CE_RR ^ CONTROL_FIELD;

    int i = 4;
    for (int j = 0; j < data_size; j++) {
        frame[i] = data[j];
        i++;
    }
    
    frame[i] =  *data_bcc;
    frame[++i] = SFD;
    
    return 0;
}

void get_data_bcc(char *buffer, int length, char *bcc) {
    *bcc = buffer[0];
    for (int i = 1; i < length; i++)
        *bcc = *bcc ^ buffer[i];
    
}

int byte_stuffer(char *buffer, int length, char *newBuffer) {

    int j = 0;
    for (int i = 0; i < length; i++){
        if (buffer[i] == SFD){
            newBuffer[j] = ESC;
            newBuffer[++j] = SFD_XOR;
        }
        else if (buffer[i] == ESC){
            newBuffer[j] = ESC;
            newBuffer[++j] = ESC_XOR;
        }
        else newBuffer[j] = buffer[i];

        j++;
    }
    
    return j;
}

int byte_destuffer(char *buffer, int length, char* newBuffer){
    int j = 0;
    for (int i = 0; i < length; i++) {
        if (buffer[i] == ESC) {

        }

    }
    return j;
}


int sender_response_sm(state *state, char rec) {

    switch (*state){
        case START:
            if(rec == SFD) *state = FLAG_RCV;
            break;
        case FLAG_RCV:
            if(rec == CE_RR) *state = A_RCV;
            else if (rec != SFD) *state = START;                   
            break;
        case A_RCV:
            if(rec == RR0 || rec == RR1 || rec == ) *state = C_RCV;
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