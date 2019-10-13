#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "application.h"

void parseArgs(application * app, char ** argv){
	printf("Path: %s\n",argv[2]);
	app->fileDescriptor = open(argv[2], O_RDWR | O_NOCTTY);
	printf("Error No:%d\n", errno);
	if (strcmp("send", argv[1]) == 0) app->stat = TRANSMITTER;
	else app->stat = RECIEVER;

}

int main(int argc, char ** argv){
	application app;
	if ((argc < 2) || (strcmp("send", argv[1])!=0 && strcmp("recieve", argv[1])!=0) ||
  	     ((strcmp("/dev/ttyS0", argv[2])!=0) && (strcmp("/dev/ttyS1", argv[2])!=0) )) {
      printf("Usage:\tnserial send_or_recieve SerialPort\n\tex: nserial send /dev/ttyS1\n");
      exit(1);
    }

    parseArgs(&app, argv);

    printf("After parsing:\nFD:%d, STATUS:%d\n", app.fileDescriptor, app.stat);

	return 0;
}