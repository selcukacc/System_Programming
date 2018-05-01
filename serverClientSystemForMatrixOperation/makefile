CC = gcc
CFLAGS = -Wall

.c.o:
	$(CC) $(CFLAGS) -c $< 

all: timeServer seeWhat

timeServer: timeServer.o
	$(CC) -o timeServer timeServer.o

seeWhat: seeWhat.o
	$(CC) -o seeWhat seeWhat.o -lm

clean:
	rm -f timeServer *.o

	
