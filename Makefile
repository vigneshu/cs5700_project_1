default: all

all: client.o sever.o
client.o:
	gcc -pthread -o client client.c 
sever.o:
	gcc -pthread -o server server.c 

clean:
	rm -rf *.o
