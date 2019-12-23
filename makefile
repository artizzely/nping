# the compiler to use
CC=gcc
# options which you pass to the compiler
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=nping.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=nping

all: $(SOURCES) $(EXECUTABLE)

${EXECUTABLE}: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -f *.o ${EXECUTABLE}