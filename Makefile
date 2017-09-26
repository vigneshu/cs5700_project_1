default: all

all: client.o

client.o:
	gcc -o client client.c


clean:
	rm -rf *.o
