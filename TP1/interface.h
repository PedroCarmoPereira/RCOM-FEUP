#ifndef INTERFACE_H
#define INTERFACE_H

#define DATA_FRAME_SIZE	1018

typedef enum status {
    TRANSMITTER,
    RECEIVER
} status;

int llopen_sender(char * port);

int llopen_receiver(char * port);

int llopen(char * port, int t_or_r);

int llwrite(char *buffer, int length);

int llread(char *buffer);

int llclose_sender(int fd);

int llclose_reciever(int fd);

int llclose(int t_or_r);

int get_max_frame_size();

#endif
