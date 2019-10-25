#ifndef APPLICATION_H
#define APPLICATION_H

#include "interface.h"

typedef struct appicationLayer {
	int fileDescriptor; /*SERIAL PORT FILE DESCRIPTOR*/
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

typedef struct tlv_t{
	tlv_type type;
	int length;
	char *value;
} tlv;

typedef struct control_p_t{
	packet_type c;
	tlv tlvs[2];
} control_packet;


int parseArgs(application * app, int argc, char ** argv);

int build_control_packet(packet_type type, control_packet * packet, char * filename);

int free_control_packet(control_packet * packet);

#endif
