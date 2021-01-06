CC = gcc
CFLAGS = -Wall -std=c11 -g -Iinclude
LDFLAGS= -L.

all: parser

parser: CalendarParser.o LinkedListAPI.o HelperFunctions.o
	$(CC) -shared -o ../libcal.so bin/CalendarParser.o bin/LinkedListAPI.o bin/HelperFunctions.o

CalendarParser.o: src/CalendarParser.c include/CalendarParser.h
	$(CC) $(CFLAGS) -c -fpic src/CalendarParser.c -o bin/CalendarParser.o

LinkedListAPI.o: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic src/LinkedListAPI.c -o bin/LinkedListAPI.o

HelperFunctions.o:	src/HelperFunctions.c include/HelperFunctions.h
	$(CC) $(CFLAGS) -c -fpic src/HelperFunctions.c -o bin/HelperFunctions.o

clean:
	rm -rf bin/*.o bin/*.so
