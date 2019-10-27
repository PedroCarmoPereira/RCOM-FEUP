#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "application.h"
#include "interface.h"

int max_frame_size = 64; //NOTA: ESTE VALOR DEVE SER INTRODUZIDO PELO USER, PARA JA ESTAMOS A USAR O MIN_SIZE DAS FRAMES DE INTERNET IRL
int sequence_number = 0;

int parseArgs(application * app, int argc,  char ** argv){
	app->fileDescriptor = open(argv[2], O_RDWR | O_NOCTTY);
	if (strcmp("send", argv[1]) == 0) app->stat = TRANSMITTER;
	else app->stat = RECEIVER;
	if (argc < 3 && app->stat == TRANSMITTER) {
		puts("ERROR: NO INPUT FILE SPECIFIED");
		return -1;
	}
	
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
	/*application app;
	if ((argc < 2) || (strcmp("send", argv[1])!=0 && strcmp("receive", argv[1])!=0) ||
  	     ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0) )) {
      printf("Usage:\tnserial send_or_recieve SerialPort\n\tex: nserial send <port> <file>\n or: nserial receive <port>\n");
      exit(1);
    }

    parseArgs(&app, argc, argv);
	*/
	if (argc && argv[0]) puts("PLACEHOLDER");
	data_packet p;
	char buf[255] = "LOREM IPSUM SIN DOLOR";
	build_data_packet(&p, buf, strlen(buf));
	printf("DATA PACKET:\nTYPE:%d\tSEQ_NO:%d\tL1L2:%d %d\nDATA: %s \n", p.c, p.sequence_number, p.l1, p.l2, p.data);
	free_data_packet(&p);
	return 0;
}