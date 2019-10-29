#ifndef DATALINK_H
#define DATALINK_H

#include <termios.h>

#define BIT(n) (0x01<<(n))
#define BAUDRATE        B9600//B38400
#define SFD 		    0x7e
#define SET 		    0x03
#define	DISC 		    0x0b
#define SUP_SIZE	    5
#define UA 			    0x07
#define RR              0x05
#define RR0             0x05
#define RR1             0x85
#define REJ             0x01
#define REJ0            0x01
#define REJ1            0x81
#define CE_RR 		    0x03
#define	CR_RE 		    0x01
#define ESC             0x7d
#define SFD_XOR         0x5e
#define ESC_XOR         0x5d
#define MAX_FRAME_SIZE  1024    
#define TRIES		    1
#define FALSE           0
#define TRUE            1

typedef struct datalink {
    char port[12];
    unsigned int baudRate;
    unsigned int sequenceNumber;
    unsigned int timeout;
    unsigned int numRetransmitions;
} datalink;


typedef enum state {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_RCV,
    DATA_RCV,
    END
} state;

/*AQUI VAI SE FAZER O QUE SE FAZIA N0 READER.C E WRITER.C*/
void set_default_settings();

int termios_setup_writer(int fd, struct termios *oldtio);

int termios_setup_reader(int fd, struct termios *oldtio);

int termios_reset(int fd, struct termios *oldtio);

int send_set(int fd, int debug);

int send_ua(int fd, int debug, int t_or_r);

void ua_sm(state *s, char rec, int t_or_r);

void set_sm(state *s, char rec);

int send_disc(int fd, int debug, int t_or_r);

void disc_sm(state *s, char rec, int t_or_r);

int send_frame(int fd, char* data, int data_length);

int build_frame(char *frame, int frame_size, char *data, int data_size, char data_bcc);

void get_data_bcc(char *buffer, int length, char *bcc);

int byte_stuffer(char *buffer, int length, char *newBuffer);

int byte_destuffer(char *buffer, int length, char* newBuffer);

int sender_read_response_sm(state *state, char rec);

int analyze_response(char *rec);

int read_frame_sm(state *state, char rec);

int destuff_frame(char *rec, int length, char* destuffed_frame);

int analyze_frame(char *frame, int frame_length);

int get_frame_data(char *frame, int length, char *data);

int build_response(char *response, int response_type);

int send_response(int fd, char *response);

void set_default_settings();

#endif
