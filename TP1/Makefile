CFLAGS	= -Wall	-Wextra -g -lm
OBJECTS1	=	application.o interface.o datalink.o error-injector.o

all:	nserial

nserial: $(OBJECTS1)
	gcc $(OBJECTS1) $(CFLAGS) -o nserial 



application.o: application.h

interface.o: interface.h

datalink.o: datalink.h

error-injector.o: error-injector.h

clean:
	rm *.o nserial