#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "interface.h"
#include "datalink.h"

volatile int STOP=FALSE;

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
    while(try_counter < TRIES && STOP != 2){
        STOP = 0;
        int try = try_counter;
        printf("%d\n", try_counter);
        send_set(fd, 0);
        sleep(1);
        alarm(3);
        //Re-lê
        enum state state = START;
        char rec = 0;
        while(!STOP){
            read(fd, &rec, 1);
            sender_sm(&state, rec);
            if (try != try_counter){
                printf("exiting try %d\n", try);
                STOP = 1;
                break;
            }
        }
    }

    if (STOP != 2){
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
    while(!STOP){
        read(fd, &rec, 1);
        reciever_sm(&state, rec);
    }

    send_ua(fd, 0);
    sleep(1);
}

int llopen(char * port, int t_or_r){
	if (!t_or_r) llopen_sender(port);
	else llopen_reciever(port);

}