#!/usr/bin/make -f

CC = g++ -Wall -g -std=c++14
LIBS = /usr/lib/x86_64-linux-gnu/libsqlite3.so
INCLUDE = ./include

munch: munch.o database.o auxiliary.o
	$(CC) -o munch munch.o database.o auxiliary.o $(LIBS)

munch.o: src/munch.cpp $(INCLUDE)/munch.hpp
	$(CC) -c src/munch.cpp -I $(INCLUDE)

database.o: src/database.cpp $(INCLUDE)/munch.hpp
	$(CC) -c src/database.cpp -I $(INCLUDE)

auxiliary.o: src/auxiliary.cpp $(INCLUDE)/munch.hpp
	$(CC) -c src/auxiliary.cpp -I $(INCLUDE)

clean:
	rm *.o
