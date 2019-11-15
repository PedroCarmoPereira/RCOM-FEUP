CFLAGS	= -Wall	-Wextra -g

OBJECTS1 	= main.c rfc1738parser.o

all: download

download: $(OBJECTS1)
	gcc $(OBJECTS1) $(CFLAGS) -o download

rfc1738parser.o : rfc1738parser.h

clean: 
	rm download *.o