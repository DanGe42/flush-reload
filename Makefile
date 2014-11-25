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

run: probe
	- rm message.txt.gpg 2>/dev/null
	./$(GPG) -e -r "Nicholas Meyer" message.txt && echo "encrypted"
	./$(BDIR)/probe $(GPG) $(ADDR) out.txt && python graph.py &
	sleep 0.01
	./$(GPG) -d message.txt.gpg && echo "decrypted"

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(BDIR)/*
