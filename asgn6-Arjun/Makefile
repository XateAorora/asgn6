CC = clang
CFLAGS = -Wall -g
LD = clang
LDFLAGS =

all: mush

mush: mush.o parseline.o
	$(LD) $(LDFLAGS) -o mush mush.o parseline.o

mush.o: mush.c
	$(CC) $(CFLAGS) -o mush.o -c mush.c

parseline.o: parseline.c
	$(CC) $(CFLAGS) -o parseline.o -c parseline.c

clean:
	rm mush *.o
