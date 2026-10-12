CFLAGS = -std=gnu89 -pedantic
OBJECTS = bhp.o

prog: ${OBJECTS}
	gcc ${CFLAGS} -o prog ${OBJECTS}

clean:
	rm *.o prog

bhp.o: bhp.c
