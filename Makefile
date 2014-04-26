CC=gcc
CFLAGS=`sdl-config --cflags` -Wall -g
LIBS=`sdl-config --libs` -lm
OBJS=u9bmp-linux.o flxx.o

default: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o u9bmp-linux $(LIBS)

clean:
	rm -rf u9bmp-linux
	rm -rf *.o

u9bmp-linux.o: u9bmp-linux.c flxx.c flxx.h
	$(CC) $(CFLAGS) -c u9bmp-linux.c

flxx.o: flxx.c flxx.h
	$(CC) $(CFLAGS) -c flxx.c

