TOP = ..
SKP = $(TOP)/libs/bin/skpgen
INCDIR = $(TOP)/libs/src
CC = gcc

CFLAGS =  -O2 -Wall -I $(INCDIR) -DDEBUG=DEBUG_TEST

%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: skpgen xirnol knight

xirnol: xirnol_parse.o xirnol_walk.o xirnol.o
	$(CC) -s -o $@ xirnol_parse.o xirnol_walk.o xirnol.o

knight: xirnol
	cp xirnol knight

xirnol_parse.c: xirnol_parse.skp
	$(SKP) xirnol_parse

skpgen:
		@cd $(TOP)/libs; make

clean:
	$(RM) *.o xirnol_parse.c xirnol_parse.h xirnol knight *.log *.t test/*.t
