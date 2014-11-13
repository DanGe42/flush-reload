CC=gcc
CFLAGS=-Wall

all: probe

probe: probe.c
	$(CC) $(CFLAGS) -o $@ $?
