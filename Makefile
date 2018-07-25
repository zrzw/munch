#!/usr/bin/make -f

CC = g++ -Wall -std=c++14
LIBS = /usr/local/lib/libsqlite3.so

munch: munch.o
	$(CC) -o munch munch.o $(LIBS)

munch.o: src/munch.cpp src/munch.h
	$(CC) -c src/munch.cpp

clean:
	rm *.o
