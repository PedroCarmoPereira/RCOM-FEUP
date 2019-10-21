#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "interface.h"
#include "datalink.h"

volatile int STOP0=FALSE;
volatile int STOP1=FALSE;
volatile int STOP2=FALSE;

int fd;

int try_counter = 0;

struct termios oldtio,newtio;

void handler(){try_counter++;}

int llopen_sender(char * port){
	
	fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd <0) {perror(port); exit(-1); }

    int ret;
    if ((ret = termios_setup(fd, &oldtio)) != 0){
        printf("termios_setup failed with error code:%d\n", ret);
        exit(-1);
    }
    (void) signal(SIGALRM, handler);
    while(try_counter < TRIES && STOP0 != 2){
        STOP0 = 0;
        int try = try_counter;
        printf("%d\n", try_counter);
        send_set(fd, 0);
        sleep(1);
        alarm(3);
        //Re-lê
        enum state state = START;
        char rec = 0;
        while(!STOP0){
            read(fd, &rec, 1);
            sender_set_sm(&state, rec);
            if (try != try_counter){
                printf("exiting try %d\n", try);
                STOP0 = 1;
                break;
            }
        }
    }

    if (STOP0 != 2){
        puts("FAILED TO ESTABLISH CONNECTION, EXITING");
        return -1;;
    }

    puts("CONNECTION ESTABLISHED");
}

int llopen_reciever(char *port){
	fd = open(port, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(port); exit(-1); }

    int ret;
    if ((ret = termios_setup(fd, &oldtio)) != 0){
        printf("termios_setup failed with error code:%d\n", ret);
        exit(-1);
    }
    char rec;
    enum state state = START;
    while(!STOP0){
        read(fd, &rec, 1);
        reciever_set_sm(&state, rec);
    }

    send_ua(fd, 0);
    sleep(1);
}

int llopen(char * port, int t_or_r){
	if (!t_or_r) llopen_sender(port);
	else llopen_reciever(port);

}

int llclose_sender(int fd){
    tcflush(fd, 0);
    (void) signal(SIGALRM, handler);
	state state = START;
    while (try_counter < TRIES && STOP1 != 2){
	    char rec;
	    STOP1 = 0;
        try_counter = 0;
        int try = try_counter;
        printf("%d\n", try_counter);
        send_disc(fd, 0, 0);
        alarm(3);
        while(!STOP1){
		read(fd, &rec, 1);
		disc_sm(&state, rec, 0);
        if (try != try_counter){
                printf("exiting try %d\n", try);
                STOP1 = 1;
                break;
            }
	    }
    }

	if(STOP1 == 2) puts("SENDER DISCONNECTED");
}

int llclose_reciever(int fd){
	STOP2 = 0;
	state state = START;
	char rec;
	while(!STOP2){
		read(fd, &rec, 1);
		disc_sm(&state, rec, 1);
	}
    sleep(1);
	send_disc(fd, 0, 1);
	if(STOP2 == 2) puts("RECIEVER DISCONNECTED");
}