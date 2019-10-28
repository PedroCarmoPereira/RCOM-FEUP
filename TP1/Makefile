CFLAGS	= -Wall	-Wextra -g
OBJECTS1	=	application.o interface.o datalink.o

all:	nserial

nserial: $(OBJECTS1)
	gcc $(OBJECTS1) $(CFLAGS) -o nserial



application.o: application.h

interface.o: interface.h

datalink.o: datalink.h

clean:
	rm *.o nserial