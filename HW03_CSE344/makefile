CC = gcc

.c.o:
	$(CC) $(CFLAGS) -c $< 

all: main lsCom catCom

main: main.o
	$(CC) -o main main.o

lsCom: lsCom.o
	$(CC) -o lsCom lsCom.o

catCom: catCom.o
	$(CC) -o catCom catCom.o

clean:
	rm -f main *.o
	rm -f lsCom *.o
	rm -f catCom *.o
