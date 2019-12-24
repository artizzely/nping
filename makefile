# the compiler
CC=gcc
# options for the compiler
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