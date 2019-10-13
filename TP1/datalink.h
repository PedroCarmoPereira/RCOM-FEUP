#ifndef DATALINK_H
#define DATALINK_H

#include <termios.h>


#define BAUDRATE B38400
/*AQUI VAI SE FAZER O QUE SE FAZIA N0 READER.C E WRITER.C*/
int termios_setup(int fd, struct termios *oldtio);

int termios_reset(int fd, struct termios *oldtio);

#endif
