CC = gcc
CFLAGS = -pthread -c -fPIC -std=gnu99
OBJS = mapreduce.o libmapreduce.so

all: mapreduce.o libmapreduce.so main

mapreduce.o: mapreduce.c
	$(CC) $(CFLAGS) -o mapreduce.o mapreduce.c

libmapreduce.so: mapreduce.o
	$(CC) -shared -o libmapreduce.so mapreduce.o

main: testmain.c
	$(CC) -o main -L. testmain.c -lmapreduce -lpthread
clean: 
	rm -f mapreduce.o libmapreduce.so main
