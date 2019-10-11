CFLAGS = -g -Wall -Wextra -pthread

all: main.o robot.o table.o student.o
	gcc $(CFLAGS) -o q2 main.o robot.o student.o table.o
	ctags -R .

main.o: main.h main.c robot.h table.h student.h
	gcc $(CFLAGS) -c main.c

robot.o: robot.h main.h robot.c
	gcc $(CFLAGS) -c robot.c

table.o: table.h main.h table.c
	gcc $(CFLAGS) -c table.c

student.o: student.h main.h student.c
	gcc $(CFLAGS) -c student.c

clean:
	rm *.o q2
