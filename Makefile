#!/usr/bin/make -f

CC = g++ -Wall -std=c++14
LIBS = /usr/lib/x86_64-linux-gnu/libsqlite3.so

munch: munch.o
	$(CC) -o munch munch.o $(LIBS)

munch.o: src/munch.cpp src/munch.hpp
	$(CC) -c src/munch.cpp

clean:
	rm *.o
