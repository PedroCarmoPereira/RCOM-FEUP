#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h> 
#include <time.h>

#include "application.h"
#include "interface.h"

int sequence_number = 0;
int fs_test_var;

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

int build_control_packet(packet_type type, control_packet *packet, char * filename, int filesize){
	packet->c = type;

	int digits = (int)floor(log10(filesize)) + 1;
	printf("Filesize %d has %d digits\n", filesize, digits);
	
	packet->tlvs[0].value = malloc(digits + 1);
	packet->tlvs[0].type = SIZE;
	sprintf(packet->tlvs[0].value,"%d\0", filesize);
	printf("\nACTUAL VALUE BEEING SENT %d\nHEX %x", filesize, filesize);
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
	char msg[(DATA_PACKET_SIZE + 10)*2];

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
		//if (ret == 1) return ret;
	}

	return 0;	
}

int receive_data_packet(data_packet * p){
	char msg[(DATA_PACKET_SIZE + 10)* 2]; //= malloc(sizeof(char *));

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

int send_file(char *filename, application *app) {
	struct stat fileStat;
	if (stat(filename, &fileStat)){
		puts("ERROR: INPUT FILE NON-EXISTENT");
		exit(-1);
	}
	int filesize = (int) fileStat.st_size;

	control_packet p;
    build_control_packet(START, &p, filename, filesize);
	printf("control packet: type:%d, TLV: %d, %d, %s ", p.c, p.tlvs[0].type, p.tlvs[0].length, p.tlvs[0].value);
	printf("TLV: %d, %d, %s \n", p.tlvs[1].type, p.tlvs[1].length, p.tlvs[1].value);

	puts("SENDING CONTROL PACKET");
    send_control_packet(p, app);
    puts("CONTROL PACKET SENT");
	puts("-------------------------");

	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		return -1;
	}

	char filePiece[DATA_PACKET_SIZE];// = malloc(DATA_PACKET_SIZE);

	int read = 0, writen = 0;
	int counter = 0;
	while (writen != filesize)
	{	
		puts("SENDING DATA...");
		read = fread(filePiece, sizeof(char), DATA_PACKET_SIZE, file);
		if(read > 0){
			counter += read;
			printf("SENDER READ %d\nCOUNTER %d\n", read, counter);
			data_packet d;
			build_data_packet(&d, filePiece, read);

			int k;
			k = send_data_packet(&d);//} while(k != 0 && k != -1);
			printf("K : %d\n", k);
			if(k == 1) continue;
			writen += read;
		}
	}
	//free(filePiece);

	if (fclose(file) != 0)
		return -1;

	control_packet end;
	build_control_packet(END, &end, filename, filesize);
	send_control_packet(end, app);

	free_control_packet(&end);
	free_control_packet(&p);
	
	return 0;
}

int receive_file(application *app){
	control_packet start;
	receive_control_packet(&start, app);
	static unsigned char expected_seq_no = 0;
	//validate_start_packet()
	char *filename = start.tlvs[1].value;
	int filesize = atoi(start.tlvs[0].value);
	fs_test_var = filesize;
	printf("\nCreating file named %s. Expected size: %d bytes\n ", filename, filesize);

	FILE* file = fopen(filename, "wb");
	if (!file){
    	puts("ERROR");
		perror("fopen");
	}

	if (file == NULL) {
		return -1;
	}

	//char *filePiece = malloc(DATA_PACKET_SIZE);
	int savedData = 0;
	
	while (savedData < filesize) {
		printf("savedData is %d and filesize is %d", savedData, filesize);
		data_packet packet;
		puts("Before receive\n");
		receive_data_packet(&packet);
		puts("After receive\n");
		if (packet.c != 1){
			printf("Data packet with wrong type");
			//free(filePiece);
			return -1;
		}
		int length = packet.l1 * 255 + packet.l2;

		if(packet.sequence_number != expected_seq_no) continue;

		printf("\nSEQUENCE_NUMBER: %d\n\n", packet.sequence_number);

		int w = fwrite(packet.data, sizeof(char), length, file);
		if (w != length){
			printf("Error: fwrite wrote %d of %d bytes", w, length);
		}
		savedData += length;
		printf("savedData is %d and was added %d", savedData, length);

		expected_seq_no++;
	}
	control_packet end;
	puts("Before receive");
	receive_control_packet(&end, app);

	puts("Leaving receive_file");

	return 0;
}

int main(int argc, char ** argv){
	application app;
	if ((argc < 2) || (strcmp("send", argv[1])!=0 && strcmp("receive", argv[1])!=0) ||
  	     ((strcmp("/dev/ttyS0", argv[2])!=0)  && (strcmp("/dev/ttyS1", argv[2])!=0) &&
		 (strcmp("/dev/ttyS5", argv[2])!=0) )) {
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
		send_file(argv[3], &app);

    	llclose(0);
    }

    else {
    	llopen(app.port, 1);
    	sleep(1);
		clock_t time = clock();
		receive_file(&app);
		puts("DISCONNECTING");
    	llclose(1);
		time = clock() - time;
		double secs = ((double)time)/CLOCKS_PER_SEC;
		printf("PROGRAM TOOK : %lf\n", secs);
		printf("R = %lf b/s\n", (fs_test_var*8)/secs);
    }
	return 0;
}