#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "application.h"
#include "interface.h"

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

int main(int argc, char ** argv){
	/*application app;
	if ((argc < 2) || (strcmp("send", argv[1])!=0 && strcmp("receive", argv[1])!=0) ||
  	     ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0) )) {
      printf("Usage:\tnserial send_or_recieve SerialPort\n\tex: nserial send <port> <file>\n or: nserial receive <port>\n");
      exit(1);
    }

    parseArgs(&app, argc, argv);
	*/
	if (argc <= 1){
		puts("WRONG NO. OF ARGS");
		exit(-1);
	}
	control_packet test0, test1;
	file_info fi;
	build_control_packet(2, &test0, argv[1]);
	build_control_packet(3, &test1, argv[1]);
	validate_start_packet(test0, &fi);
	int r = validate_end_packet(test1, &fi);
	if(!r) printf("FILE %s, is %d bytes long!\n", fi.name, fi.size);
	else puts("BAD END PACKET");
	free_control_packet(&test0);
	return 0;
}