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

int main(int argc, char ** argv){
	/*application app;
	if ((argc < 2) || (strcmp("send", argv[1])!=0 && strcmp("receive", argv[1])!=0) ||
  	     ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0) )) {
      printf("Usage:\tnserial send_or_recieve SerialPort\n\tex: nserial send <port> <file>\n or: nserial receive <port>\n");
      exit(1);
    }

    parseArgs(&app, argc, argv);
	*/
	/*if (argc <= 1){
		puts("WRONG NO. OF ARGS");
		exit(-1);
	}
	control_packet test;
	build_control_packet(2, &test, argv[1]);
	printf("CONTROL PACKET\nTYPE: %d\nTLV0: %d, %d, %s\nTLV1: %d, %d, %s\n", test.c, test.tlvs[0].type, test.tlvs[0].length, test.tlvs[0].value, test.tlvs[1].type, test.tlvs[1].length, test.tlvs[1].value);
	free_control_packet(&test);*/
	return 0;
}