CFLAGS	= -Wall	-Wextra -g

OBJECTS1 	= main.c rfc1738url.o ftp.o

all: download

download: $(OBJECTS1)
	gcc $(OBJECTS1) $(CFLAGS) -o download

rfc1738url.o : rfc1738url.h

ftp.o : ftp.h

clean: 
	rm download *.o