CC=gcc
CFLAGS=-Wall --std=gnu99 -g

all: probe

probe: probe.c args.c
	$(CC) $(CFLAGS) -o $@ $?

measure_l1: measure_l1.c
	$(CC) $(CFLAGS) -o $@ $?

clean:
	rm -f *.o probe measure_l1
