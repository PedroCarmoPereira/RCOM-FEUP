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
	
	packet->tlvs[0].value = malloc(sizeof (int));
	packet->tlvs[0].type = SIZE;
	snprintf(packet->tlvs[0].value, sizeof packet->tlvs[0].value,"%d", fileStat.st_size);
	printf("\nACTUAL VALUE BEEING SENT %d\nHEX %x", fileStat.st_size, fileStat.st_size);
	printf("\nSTRING %s\n", packet->tlvs[0].value);
	packet->tlvs[0].length = strlen(packet->tlvs[0].value);

	printf("\nSIZE: %d\n", packet->tlvs[0].length);

	packet->tlvs[1].value = malloc(sizeof(char) * strlen(filename));
	packet->tlvs[1].type = NAME;
	sprintf(packet->tlvs[1].value, "%s", filename);
	packet->tlvs[1].length = strlen(packet->tlvs[1].value);

	return 0;
}

int send_control_packet(control_packet packet, application *app){
	int sz = 6 + packet.tlvs[0].length + packet.tlvs[1].length;
	char *msg = malloc(sizeof(char) * sz);
	int i = 0;
	msg[i] = packet.c;
	i++;
	msg[i] = (char) packet.tlvs[0].type;
	i++;
	msg[i] = (char) packet.tlvs[0].length;
	//printf("Length size -> %x\n", msg[i]);
	//printf("\n%d\n", packet.tlvs[0].length);
	i++;
	int j;
	for(j = 0; j < packet.tlvs[0].length; j++) {
		msg[i] = packet.tlvs[0].value[j];
		//printf("SIZE VALUE %d, %x\n", j, msg[i]);
		i++;
	}

	msg[i] = (char) packet.tlvs[1].type;
	i++;
	msg[i] = (char) packet.tlvs[1].length + 1;
	//printf("Length filename -> %x", msg[i]);
	i++;
	
	for(j = 0; j < packet.tlvs[1].length; j++)  {
		msg[i] = packet.tlvs[1].value[j];
		i++;
	}
	msg[i] = '\0';

	int size = 5 + packet.tlvs[0].length + packet.tlvs[1].length + 1;
	int stop = 0;
	puts("Putting in llwrite:");
	for(int mi = 0; mi < size; mi++) printf("%x  ", msg[mi]);
	puts(" ");
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
	int i = 0;
	p->c = msg[i];
	i++;
	p->tlvs[0].value = malloc(sizeof(char *));
	p->tlvs[0].type = msg[i];
	i++;
	p->tlvs[0].length = msg[i];
	i++;
	int k;
	for(k = 0; k < p->tlvs[0].length; k++) {
		p->tlvs[0].value[k] = msg[i];
		i++;
	}
	p->tlvs[0].value[k] = '\0';

	p->tlvs[1].value = malloc(sizeof(char *));
	p->tlvs[1].type = msg[i];
	i++;
	p->tlvs[1].length = msg[i];
	i++;
	int j;
	for(j = 0; j < p->tlvs[1].length; j++) {
		p->tlvs[1].value[j] = msg[i];
		i++;
	}
	p->tlvs[1].length = j;
	printf("%d", j);
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
	//packet->data = malloc(sizeof(char *));
	packet->sequence_number = sequence_number % 255;
	packet->l1 = size / 255;
	packet->l2 = size % 255;
	for (int i = 0; i < size; i++) packet->data[i] = buff[i];
	sequence_number++;
	return 0;
}

int send_data_packet(data_packet *packet){
	char msg[DATA_FRAME_SIZE];

	msg[0] = packet->c;
	msg[1] = packet->sequence_number;
	msg[2] = packet->l1;
	msg[3] = packet->l2;

	int j = 4;
	for (int i = 0; i < packet->l1 * 255 + packet->l2; i++) {
		msg[j] = packet->data[i];
		j++;
	}

	int stop = 0;
	while(!stop) {
		int ret = llwrite(msg, j);
		printf("LLWRITE Returned %d\n", ret);
		if (ret == -1) return ret;
		if (ret == 0) stop = 1;
	}

	return 0;	
}

int receive_data_packet(data_packet * p){
	char msg[DATA_FRAME_SIZE]; //= malloc(sizeof(char *));

	int ret = 0;

	do {
		ret = llread(msg);
	} while(ret == 0);

	p->c = msg[0];
	p->sequence_number = msg[1];
	p->l1 = msg[2];
	p->l2 = msg[3];

	int j = 4;
	for (int i = 0; i < p->l1 * 255 + p->l2; i++) {
		p->data[i] = msg[j];
		j++; 
	}
	return 0;
}

int free_data_packet(data_packet * packet){
	free(packet->data);
	return 0;
}

int sendFile(char *filename, application *app) {
	
	return 0;
}

int receiveFile(){

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
    	build_control_packet(START, &p, argv[3]);

		printf("control packet: type:%d, TLV: %d, %d, %s ", p.c, p.tlvs[0].type, p.tlvs[0].length, p.tlvs[0].value);
		printf("TLV: %d, %d, %s \n", p.tlvs[1].type, p.tlvs[1].length, p.tlvs[1].value);

		puts("SENDING CONTROL PACKET");
    	send_control_packet(p, &app);
    	puts("CONTROL PACKET SENT");

		puts("-------------------------");
		data_packet d;
		char buffer[5] = {'A', 'B', 'C', 'D', 'E'};
		build_data_packet(&d, buffer, 5);
		
		puts("\nSending data packet");
		send_data_packet(&d);
		puts("sent data packet");
		send_data_packet(&d);
		send_data_packet(&d);
		send_data_packet(&d);
		send_data_packet(&d);
    	/*char buffer[5] = {'A', 'B', 'C', 'D', 'E'};
    	llwrite(buffer, 5);*/
    	llclose(0);
    }

    else {
    	llopen(app.port, 1);
    	sleep(1);
		control_packet p;
		receive_control_packet(&p, &app);
		data_packet dp;
		puts("\nGetting data packet");
		receive_data_packet(&dp);
		puts("received data packet");
		receive_data_packet(&dp);
		receive_data_packet(&dp);
		receive_data_packet(&dp);
		receive_data_packet(&dp);
		printf("\nDATA PACKET RECEIVED: %d, %d, %d\n", dp.c, dp.l1, dp.l2);
		for(int i = 0; i < dp.l1*255 + dp.l2; i++) printf("DATA: %x\n", dp.data[i]);
		//free_control_packet(&p);
		puts("DISCONNECTING");
    	llclose(1);
    }
	return 0;
}