#ifndef INTERFACE_H
#define INTERFACE_H

int llopen(int porta, int t_or_r);

int llwrite(int fd, char *buffer, int length);

int llread(int fd, char *buffer);

int llclose(int fd);

#endif
