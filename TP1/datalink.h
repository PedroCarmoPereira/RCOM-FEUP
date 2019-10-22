#ifndef DATALINK_H
#define DATALINK_H

#include <termios.h>


#define BAUDRATE B38400
#define SFD 		0x7e
#define SET 		0x03
#define	DISC 		0x0b
#define SUP_SIZE	5
#define UA 			0x07
#define RR0         0x05
#define RR1         0x85
#define REJ0        0x01
#define REJ1        0x81
#define CE_RR 		0x03
#define	CR_RE 		0x01
#define ESC         0x7d
#define SFD_XOR     0x5e
#define ESC_XOR     0x5d
#define TRIES		3
#define FALSE 0
#define TRUE 1

typedef enum state {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_RCV,
    END
} state;

/*AQUI VAI SE FAZER O QUE SE FAZIA N0 READER.C E WRITER.C*/
int termios_setup_writer(int fd, struct termios *oldtio);

int termios_setup_reader(int fd, struct termios *oldtio);

int termios_reset(int fd, struct termios *oldtio);

int send_set(int fd, int debug);

int send_ua(int fd, int debug);

void sender_set_sm(state *s, char rec);

void reciever_set_sm(state *s, char rec);

int send_disc(int fd, int debug, int t_or_r);

void disc_sm(state *s, char rec, int t_or_r);

#endif
