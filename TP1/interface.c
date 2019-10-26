#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "interface.h"
#include "datalink.h"

int fd;

int try_counter = 0;

struct termios oldtio,newtio;

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
        //sleep(1);
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
}

int llopen(char * port, int t_or_r){
	if (!t_or_r) llopen_sender(port);
	else llopen_reciever(port);

}

int llwrite(int fd, char* buffer, int length) {
    try_counter = 0;
    state state = START;
    char rec[SUP_SIZE];

    while(try_counter < TRIES && state != END){
        state = START;
        int try = try_counter;
        printf("%d\n", try_counter);
        send_frame(fd, buffer, length);
        sleep(1);
        alarm(3);

        int i = 0;
        while(state != END){
            read(fd, &rec[i++], 1);
            sender_read_response_sm(&state, rec[i]);
            if (try != try_counter){
                printf("exiting try %d\n", try);
                break;
            }
        }
    }
    int ret = -1;
    if (state == END)
        ret = analyze_response(rec);

    
    return ret;
}

int llread(int fd, char* buffer) {
    char *rec = malloc(MAX_FRAME_SIZE);
    enum state state = START;

    int frame_length = 0;
    while(state != END){
        int r = read(fd, &rec[frame_length++], 1);
        read_frame_sm(&state, rec);
    }

    puts(" Read");

    char *destuffed_frame = malloc(MAX_FRAME_SIZE);
    int destuffed_frame_length = destuff_frame(rec, frame_length, destuffed_frame);

    puts(" Frame destuffed\n")
      
    int result = analyze_frame(destuffed_frame, destuffed_frame_length);

    printf(" Frame analyzed; result=%d", result);

    int data_to_save = 0;
    if (result == 0)
        data_to_save = get_frame_data(destuffed_frame, destuffed_frame_length, buffer);

    char *response = malloc(SUP_SIZE); 
    build_response(response, result);
    send_response(fd, response);

    free(rec);
    free(destuffed_frame);
    free(response);

    return data_to_save;    
}

int llclose_sender(int fd){
    tcflush(fd, 0);
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
    }
}

int llclose_reciever(int fd){
	state state0 = START;
	char rec;
    puts("DISCONNECTING RECEIVER");
	while(state0 != END){
		read(fd, &rec, 1);
		disc_sm(&state0, rec, 1);
	}
    sleep(1);
	send_disc(fd, 0, 1);
    sleep(1);
    (void) signal(SIGALRM, handler);
	if(state0 == END) {
        state state1 = START;
        try_counter = 0;
        int try = 0;
        while (try_counter < TRIES && state1 != END){
            try = try_counter;  
            read(fd, &rec, 1);
            ua_sm(&state1, rec, 1);
            if (try != try_counter){
                printf("FAILED ATTEMPT NO:%d\n", try);
                break;
            }
        }
        
        if (state1 == END)  puts("RECEIVER DISCONNECTED");
        else puts("RECEIVER FAILED TO DISCONNECT PROPERLY");
    }
}