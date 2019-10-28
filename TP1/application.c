#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "application.h"
#include "interface.h"

int sequence_number = 0;

int parseArgs(application * app, int argc,  char ** argv){
	if (strcmp("send", argv[1]) == 0 && argc == 4){ 
		app->fd = open(argv[3], O_RDWR | O_NOCTTY);
		app->stat = TRANSMITTER;
		app->port = malloc(sizeof(char *));
		strcpy(app->port, argv[2]);
	}
	else if(strcmp("receive", argv[1]) == 0 && argc == 3){
		app->stat = RECEIVER;
		app->port = malloc(sizeof(char *));
		strcpy(app->port, argv[2]);
	}

	else return 1;
	
	return 0;
}

int build_control_packet(packet_type type, control_packet *packet, char * filename){
	struct stat fileStat;
	if (stat(filename, &fileStat)){
		puts("ERROR: INPUT FILE NON-EXISTENT");
		exit(-1);
	}
	packet->c = type;
	
	packet->tlvs[0].value = malloc(sizeof(char *));
	packet->tlvs[0].type = SIZE;
	packet->tlvs[0].length = sizeof(long int);
	sprintf(packet->tlvs[0].value, "%ld", fileStat.st_size);

	packet->tlvs[1].value = malloc(sizeof(char *));
	packet->tlvs[1].type = NAME;
	packet->tlvs[1].length = sizeof(char *);
	sprintf(packet->tlvs[1].value, "%s", filename);

	return 0;
}

int send_control_packet(control_packet packet, application *app){
	char *msg = malloc(sizeof(char *));
	msg[0] = packet.c;
	msg[1] = (char) packet.tlvs[0].type;
	msg[2] = (char) packet.tlvs[0].length;
	int j = 0, i;
	for(i = 3; i < packet.tlvs[0].length; i++) {
		msg[i] = packet.tlvs[0].value[j];
		j++; 
	}
	int k = j;
	msg[i++] = (char) packet.tlvs[1].type;
	msg[i++] = (char) packet.tlvs[1].length;
	
	for(j = 0; j < packet.tlvs[1].length; j++)  msg[i++] = packet.tlvs[1].value[j];

	int size = 1 + 4 + packet.tlvs[0].length + packet.tlvs[1].length;
	int stop = 0;
	for(int mi = 0; mi < size; mi++) printf("%c\n", msg[mi]);
	while(!stop){
		int ret = llwrite(msg, size);
		if (ret == -1) return ret;
		if (ret == 0) stop = 1;
	}
	free(msg);
	return 0;
}

int receive_control_packet(control_packet *p, application * app){
	char *msg = malloc(sizeof(char *));
	int ret = 0;
	do{
		ret = llread(msg);
	}while(ret == 0);
	p->c = msg[0];
	p->tlvs[0].value = malloc(sizeof(char *));
	p->tlvs[0].type = msg[1];
	p->tlvs[0].length = msg[2];
	int i;
	for(i = 0; i < p->tlvs[0].length; i++) p->tlvs[0].value[i] = msg[3 + i];

	//i += 3;
	p->tlvs[1].value = malloc(sizeof(char *));
	p->tlvs[1].type = msg[1 + i++];
	p->tlvs[1].length = msg[1 + i];
	for(int j = 0; j < p->tlvs[1].length; j++) {
		p->tlvs[1].value[j] = msg[1 + i];
		i++;
	}
	free(msg);
}

int free_control_packet(control_packet * packet){
	free(packet->tlvs[1].value);
	free(packet->tlvs[0].value);

	return 0;
}

int validate_start_packet(control_packet packet, file_info * fi){
	if(packet.c == START){
		if(packet.tlvs[0].type == SIZE){
			fi->size = atoi(packet.tlvs[0].value);
			if(packet.tlvs[1].type == NAME){
				fi->name = malloc(packet.tlvs[1].length);
				strcpy(fi->name, packet.tlvs[1].value);
				return 0;
			}
		}
	}
	return 1;
}

int validate_end_packet(control_packet packet, file_info * fi){
	if (packet.c == END)
		if(packet.tlvs[0].type == SIZE && fi->size == atoi(packet.tlvs[0].value))
			if(packet.tlvs[1].type == NAME) return strcmp(packet.tlvs[1].value, fi->name);
	
	return 1;
}

int build_data_packet(data_packet * packet, char * buff, int size){
	packet->c = DATA;
	packet->data = malloc(sizeof(char *));
	packet->sequence_number = sequence_number % 255;
	packet->l1 = size / 255;
	packet->l2 = size % 255;
	for (int i = 0; i < size; i++) packet->data[i] = buff[i];
	sequence_number++;
	return 0;
}

int free_data_packet(data_packet * packet){
	free(packet->data);
	return 0;
}

int main(int argc, char ** argv){
	application app;
	if ((argc < 2) || (strcmp("send", argv[1])!=0 && strcmp("receive", argv[1])!=0) ||
  	     ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0) )) {
      printf("Usage:\tnserial send_or_recieve SerialPort\n\tex: nserial send <port> <file>\n\tor: nserial receive <port>\n");
      exit(1);
    }

    if(parseArgs(&app, argc, argv)) {
    	puts("INCORRECT ARGUMENTS");
    	exit(-1);
    }
    int packet_size = get_max_frame_size() - 6;
    if(app.stat == TRANSMITTER){
    	llopen(app.port, 0);
    	sleep(1);
    	control_packet p;
    	build_control_packet(1, &p, argv[3]);
    	send_control_packet(p, &app);
    	puts("AFTER SENDING CONTROL PACKET");
    	/*char buffer[5] = {'A', 'B', 'C', 'D', 'E'};
    	llwrite(buffer, 5);*/
    	llclose(0);
    }

    else {
    	llopen(app.port, 1);
    	sleep(1);
		control_packet p;
		receive_control_packet(&p, &app);
		printf("PACKET RECEIVED:\nPACKET_TYPE:%d\n", p.c);
		for(int i = 0; i < 8; i++) printf("%c", p.tlvs[1].value[i]);
		puts("\nAAAA");
    	llclose(1);
    }
	return 0;
}