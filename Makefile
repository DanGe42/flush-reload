SRC=src
IDIR=$(SRC)/include
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

$(ODIR)/%.o: $(SRC)/%.c $(DEPS)
	mkdir -p obj
	$(CC) $(CFLAGS) -c -o $@ $<

measure_l1: $(SRC)/measure_l1.c
	mkdir -p $(BDIR)
	$(CC) $(CFLAGS) -o $(BDIR)/$@ $?

timings: $(ODIR)/args.o $(ODIR)/timings.o
	mkdir -p $(BDIR)
	$(CC) $(CFLAGS) -o $(BDIR)/$@ $^

run: probe
	- rm message.txt.gpg 2>/dev/null
	$(BDIR)/probe $(GPG) $(ADDR) out.txt $(CYCLES) && python graph.py &
	sleep 0.01
	$(GPG) --sign message.txt

noise: probe
	$(BDIR)/probe $(GPG) $(ADDR) out.txt $(CYCLES) && python graph.py &

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*
	rm -rf *.dSYM
