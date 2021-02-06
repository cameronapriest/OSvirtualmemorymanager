# makefile

CC = gcc
CFLAGS = -Wall

virtual: 
	$(CC) $(CFLAGS) virtual.c -o virtual

clean:
	rm -rf virtual
	rm -rf output.txt

all: 
	$(CC) $(CFLAGS) virtual.c -o virtual
