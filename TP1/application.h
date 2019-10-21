#ifndef APPLICATION_H
#define APPLICATION_H

typedef enum t_or_r{
	TRANSMITTER, RECIEVER
} status;


typedef struct appicationLayer {
	int fileDescriptor; /*SERIAL PORT FILE DESCRIPTOR*/
	status stat; /*TRANSMITTER | RECIEVER*/
} application;

void parseArgs(application * app, char ** argv);

#endif
