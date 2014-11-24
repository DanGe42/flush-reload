IDIR=include
CC=gcc
CFLAGS=-Wall --std=gnu99 -I$(IDIR) -g

ODIR=obj

_DEPS=args.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=args.o probe.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

BDIR=bin

all: probe

probe: $(OBJ)
	mkdir -p $(BDIR)
	$(CC) $(CFLAGS) -o $(BDIR)/$@ $^

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<

measure_l1: measure_l1.c
	mkdir -p $(BDIR)
	$(CC) $(CFLAGS) -o $(BDIR)/$@ $?

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*
