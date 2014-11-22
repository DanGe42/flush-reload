CC=gcc
CFLAGS=-Wall --std=gnu99

all: probe

probe: probe.c
	$(CC) $(CFLAGS) -o $@ $?

clean:
	rm *.o probe
