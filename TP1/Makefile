CFLAGS	= -Wall	-Wextra
OBJECTS1	=	application.o

all:	nserial

nserial: $(OBJECTS1)
	gcc $(OBJECTS1) $(CFLAGS) -o nserial



application.o: application.h

clean:
	rm *.o nserial