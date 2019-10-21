#ifndef INTERFACE_H
#define INTERFACE_H

typedef enum status {
    TRANSMITTER,
    RECEIVER
} status;

int llopen_sender(char * port);

int llopen_receiver(char * port);

int llopen(char * port, int t_or_r);

//int llwrite(int fd, char *buffer, int length);

//int llread(int fd, char *buffer);

int llclose_sender(int fd);

int llclose_reciever(int fd);

//int llclose(int fd, int t_or_r);

#endif
