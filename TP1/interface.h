#ifndef INTERFACE_H
#define INTERFACE_H

int llopen_sender(char * port);

int llopen_reciever(char * port);

int llopen(char * port, int t_or_r);

//int llwrite(int fd, char *buffer, int length);

//int llread(int fd, char *buffer);

//int llclose(int fd);

#endif
