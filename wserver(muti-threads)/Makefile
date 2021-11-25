# An admittedly primitive Makefile
# To compile, type "make" or make "all"
# To remove files, type "make clean"


CC = gcc
CFLAGS = -Wall -std=gnu99
LIBS = -lpthread
OBJS = wserver.o wclient.o request.o io_helper.o threadpool.o

all: wserver wclient spin.cgi

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

wserver: wserver.o request.o io_helper.o threadpool.o
	$(CC) $(CFLAGS) $(LIBS) -o wserver wserver.o request.o io_helper.o threadpool.o

wclient: wclient.o io_helper.o
	$(CC) $(CFLAGS) -o wclient wclient.o io_helper.o

spin.cgi: spin.c
	$(CC) $(CFLAGS) -o spin.cgi spin.c

clean:
	rm -f $(OBJS) wserver wclient spin.cgi

