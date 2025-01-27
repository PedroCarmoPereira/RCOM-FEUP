#ifndef APPLICATION_H
#define APPLICATION_H

#include "interface.h"

#define DATA_PACKET_SIZE 1012

typedef struct appicationLayer {
	int fd; /*SERIAL PORT FILE DESCRIPTOR*/
	char * port; //PORT
	status stat; /*TRANSMITTER | RECEIVER*/
} application;

typedef enum packet_type_t{
	ZERO, 
	DATA,
	START,
	END
} packet_type;

typedef enum tlv_type {
	SIZE,
	NAME,
	OTHER
} tlv_type;

typedef struct tlv_t {
	tlv_type type;
	char length;
	char *value;
} tlv;

typedef struct control_p_t {
	packet_type c;
	tlv tlvs[2];
} control_packet;


typedef struct file_info_t {
	int size;
	char * name;
} file_info;

typedef struct data_packet_t {
	packet_type c;
	unsigned char sequence_number;
	unsigned char l1, l2;
	char data[DATA_PACKET_SIZE];
} data_packet;


int parseArgs(application * app, int argc, char ** argv);

int build_control_packet(packet_type type, control_packet * packet, char * filename, int filesize);

int free_control_packet(control_packet * packet);

int validate_start_packet(control_packet packet, file_info *fi);

int validate_end_packet(control_packet packet, file_info * fi);

int send_control_packet(control_packet packet, application *app);

int receive_control_packet(control_packet *p, application * app);

int build_data_packet(data_packet * packet, char * buff, int size);

int free_data_packet(data_packet * packet);

//void send_data_packet(data_packet * packet, application *app);

#endif
