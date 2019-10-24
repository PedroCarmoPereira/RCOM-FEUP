#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
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
    (void) signal(SIGALRM, handler);
    state state = START;
    while(try_counter < TRIES && state != END){
        state = START;
        int try = try_counter;
        printf("%d\n", try_counter);
        send_set(fd, 0);
        //sleep(1);
        alarm(3);
        //Re-lê
        char rec = 0;
        while(state != END){
            read(fd, &rec, 1);
            sender_set_sm(&state, rec);
            if (try != try_counter){
                printf("exiting try %d\n", try);
                break;
            }
        }
    }

    if (state != END){
        puts("FAILED TO ESTABLISH CONNECTION, EXITING");
        return -1;
    }

    puts("CONNECTION ESTABLISHED");
}

int llopen_reciever(char *port){
	fd = open(port, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(port); exit(-1); }

    int ret;
    if ((ret = termios_setup_reader(fd, &oldtio)) != 0){
        printf("termios_setup failed with error code:%d\n", ret);
        exit(-1);
    }
    char rec;
    enum state state = START;
    while(state != END){
        read(fd, &rec, 1);
        reciever_set_sm(&state, rec);
    }

    send_ua(fd, 0);
}

int llopen(char * port, int t_or_r){
	if (!t_or_r) llopen_sender(port);
	else llopen_reciever(port);

}

int llwrite(int fd, char* buffer, int length) {
    /*try_counter = 0;
    STOP = FALSE;

    while(try_counter < TRIES && STOP != 2){
        STOP = 0;
        int try = try_counter;
        printf("%d\n", try_counter);
        send_frame(fd, buffer, length);
        sleep(1);
        alarm(3);

        char rec = 0;
        while(!STOP){
            read(fd, &rec, 1);
            
            if (try != try_counter){
                printf("exiting try %d\n", try);
                STOP = 1;
                break;
            }
        }
    }
    
    */
    return 0;
}

/*int llread(int fd, char* buffer) {
    
    return 0;    
}*/

int llclose_sender(int fd){
    tcflush(fd, 0);
    (void) signal(SIGALRM, handler);
	state state = START;
    while (try_counter < TRIES && state != END){
	    char rec;
	    state = START;
        try_counter = 0;
        int try = try_counter;
        printf("%d\n", try_counter);
        send_disc(fd, 0, 0);
        alarm(3);
        while(state != END){
		read(fd, &rec, 1);
		disc_sm(&state, rec, 0);
        if (try != try_counter){
                printf("exiting try %d\n", try);
                break;
            }
	    }
    }

	if(state == END) puts("SENDER DISCONNECTED");
}

int llclose_reciever(int fd){
	state state = START;
	char rec;
	while(state != END){
		read(fd, &rec, 1);
		disc_sm(&state, rec, 1);
	}
    sleep(1);
	send_disc(fd, 0, 1);
	if(state == END) puts("RECIEVER DISCONNECTED");
}