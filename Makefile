#!/usr/bin/make -f

SHELL = /bin/sh
CC = g++ -Wall -std=c++14

munch: munch.o
	$(CC) -o munch munch.o

munch.o: src/munch.cpp src/munch.h
	$(CC) -c src/munch.cpp

clean:
	rm *.o
