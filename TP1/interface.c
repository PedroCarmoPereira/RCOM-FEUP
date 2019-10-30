#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "interface.h"
#include "datalink.h"

int fd;

int try_counter = 0;

struct termios oldtio,newtio;

extern datalink info;

void handler(){try_counter++;}

int llopen_sender(char * port){
	
	fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd <0) {perror(port); exit(-1); }

    int ret;
    if ((ret = termios_setup_writer(fd, &oldtio)) != 0){
        printf("termios_setup failed with error code:%d\n", ret);
        exit(-1);
    }
    
    puts("ESTABLISHING CONNECTION...");
    (void) signal(SIGALRM, handler);
    state state = START;
    while(try_counter < TRIES && state != END){
        state = START;
        int try = try_counter;
        send_set(fd, 0);
        alarm(3);
        //Re-lê
        char rec = 0;
        while(state != END){
            read(fd, &rec, 1);
            ua_sm(&state, rec, 0);
            if (try != try_counter){
                printf("FAILED ATTEMPT NO:%d\n", try);
                break;
            }
        }
    }

    if (state != END){
        puts("FAILED TO ESTABLISH CONNECTION, EXITING");
        termios_reset(fd, &oldtio);
        exit(-1);
    }

    puts("TRANSMITTER READY");
    return 0;
}

int llopen_reciever(char *port){
	fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {perror(port); exit(-1); }

    int ret;
    if ((ret = termios_setup_reader(fd, &oldtio)) != 0){
        printf("termios_setup failed with error code:%d\n", ret);
        exit(-1);
    }
    
    puts("ESTABLISHING CONNECTION...");
    char rec;
    enum state state = START;
    while(state != END){
        read(fd, &rec, 1);
        set_sm(&state, rec);
    }
    
    send_ua(fd, 0, 1);
    puts("RECEIVER READY");
    return 0;
}

int llopen(char * port, int t_or_r){
    set_default_settings();
	if (!t_or_r) return llopen_sender(port);
	else return llopen_reciever(port);

}

int llwrite(char* buffer, int length) {
    try_counter = 0;
    state state = START;
    char rec[SUP_SIZE];

    /*
    puts(" ");
    printf("llwrite received: ");
    for (int i = 0; i < length; i++) printf("%x ", buffer[i]);*/

    while(try_counter < TRIES && state != END){
        state = START;
        int try = try_counter;
        send_frame(fd, buffer, length);
        alarm(5);

        int i = 0;
        while(state != END) {
            int add = read(fd, &rec[i], 1);
            if (add > 0) {
                sender_read_response_sm(&state, rec[i]);
                i++;
            }
                        
            if (try != try_counter){
                printf("TIMEOUT: %d\n", try);
                break;
            }
        }

    }
    alarm(0);

    int ret = -1;
    if (state == END){
        //printf("Analyzing response FLAG:%x A:%x C:%x BCC:%x FLAG:%x\n", rec[0], rec[1], rec[2], rec[3], rec[4]);
        ret = analyze_response(rec);
        //printf("RESPONSE ANALYZED, Returned %d\n", ret);
        //puts("---------------------");
    }
    return ret;
}

int llread(char* buffer) {
    char *rec = malloc(MAX_FRAME_SIZE);
    enum state state = START;

    int frame_length = 0;

    while(state != END){
        int r = read(fd, &rec[frame_length], 1);
        if (r > 0) {
            //printf(" Char received: %x\n", rec[frame_length]);
            //puts(" Pre SM");
            read_frame_sm(&state, rec[frame_length]);
            //puts(" Post SM");
            frame_length++;
        }
    }

    //puts(" Read\n");

    char *destuffed_frame = malloc(MAX_FRAME_SIZE);
    int destuffed_frame_length = destuff_frame(rec, frame_length, destuffed_frame);

   // printf("\nDestuffed frame: %d\n", destuffed_frame_length);
    //for (int i = 0; i < destuffed_frame_length; i++) printf("%x ", destuffed_frame[i]);

    //puts(" Frame destuffed\n");
      
    int result = analyze_frame(destuffed_frame, destuffed_frame_length);

    printf(" Frame analyzed; result=%d\n", result);

    int data_to_save = 0;
    if (result == 0)
        data_to_save = get_frame_data(destuffed_frame, destuffed_frame_length, buffer);

    //printf(" Data extracted from frame\n");

    /*printf("\nData extracted from frame: %d\n", data_to_save);
    for (int i = 0; i < data_to_save; i++) printf("%x ", buffer[i]);*/


    char *response = malloc(SUP_SIZE);
    build_response(response, result);

    /*for (int i = 0; i < SUP_SIZE; i++){
        printf("%d - %x, ", i, response[i]);
    }*/

    //sleep(2);

    int i = send_response(fd, response);
    printf(" Response sent, %d writen\n", i);

    //free(rec);
    //free(destuffed_frame);
    //free(response);

    return data_to_save;    
}

int llclose_sender(int fd){
    //tcflush(fd, 0);
    (void) signal(SIGALRM, handler);
	state state = START;
    puts("DISCONNECTING TRANSMITTER");
    try_counter = 0;
    while (try_counter < TRIES && state != END){
	    char rec;
	    state = START;
        int try = try_counter;
        send_disc(fd, 0, 0);
        alarm(3);
        while(state != END){
		    read(fd, &rec, 1);
		    disc_sm(&state, rec, 0);
            if (try != try_counter){
                printf("FAILED ATTEMPT NO:%d\n", try);
                break;
            }
	    }
    }

	if(state == END) {
        send_ua(fd, 0, 0);
        puts("TRANSMITTER DISCONNECTED");
        return 0;
    }

    return -1;
}

int llclose_reciever(int fd){
	state state0 = START;
	char rec;
    int k = 0;
    puts("DISCONNECTING RECEIVER");
	while(state0 != END){
		k = read(fd, &rec, 1);
        if(k > 0) disc_sm(&state0, rec, 1);
	}
    (void) signal(SIGALRM, handler);
	if(state0 == END) {
        state state1 = START;
        try_counter = 0;
        int try = 0;
        while (try_counter < TRIES && state1 != END){
        	send_disc(fd, 0, 1);
        	alarm(3);
            try = try_counter;
            while(state1 != END){
            	k = read(fd, &rec, 1);
            	if(k > 0) ua_sm(&state1, rec, 1);
            	if (try != try_counter) break;
            }
        }
        puts("RECEIVER DISCONNECTED");
        if (state1 == END) return 0;
        else return 1;
    }

    return 2;
}

int llclose(int t_or_r){
    if(!t_or_r) return llclose_sender(fd);
    else return llclose_reciever(fd);
}

int get_max_frame_size(){return MAX_FRAME_SIZE;}