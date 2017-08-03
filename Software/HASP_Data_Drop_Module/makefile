CC=g++

CFLAGS = -I. -Wall -Wextra -O2 -std=c++11 -c

#-I/usr/local/include -lraspicam -L/opt/vc/lib -lmmal -lmmal_core -lmmal_util

all: camera_test read_test full_test

camera_test: camera_test.o
	$(CC) -o camera_test camera_test.o -I/usr/local/include -lraspicam -L/opt/vc/lib -lmmal -lmmal_core -lmmal_util

read_test: read_test.o RS232.o
	$(CC) -o read_test read_test.o RS232.o -lwiringPi

full_test: full_test.o RS232.o
	$(CC) -o full_test full_test.o RS232.o -I/usr/local/include -lraspicam -L/opt/vc/lib -lmmal -lmmal_core -lmmal_util -lwiringPi

RS232.o : RS232.cpp
	$(CC) RS232.cpp -o RS232.o  $(CFLAGS) 

camera_test.o: camera_test.cpp
	$(CC) camera_test.cpp -o camera_test.o $(CFLAGS)

read_test.o: read_test.cpp
	$(CC) read_test.cpp RS232.o -o read_test.o $(CFLAGS)

full_test.o: full_test.cpp
	$(CC) full_test.cpp RS232.cpp -o full_test.o $(CFLAGS)

clean:
	rm -r *.o

purge: clean
	rm DataPigeon

.PHONY: clean purge
