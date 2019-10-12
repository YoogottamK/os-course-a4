CFLAGS = -g -Wall -Wextra -pthread

all: rider.o server.o cab.o main.o
	gcc $(CFLAGS) -o q3 cab.o rider.o main.o server.o
	ctags -R .

cab.o: cab.h cab.c main.h
	gcc $(CFLAGS) -c cab.c

rider.o: rider.h rider.c main.h
	gcc $(CFLAGS) -c rider.c

main.o: main.h main.c cab.h server.h rider.h
	gcc $(CFLAGS) -c main.c

server.o: server.h server.c main.h
	gcc $(CFLAGS) -c server.c

clean:
	rm *.o q3
