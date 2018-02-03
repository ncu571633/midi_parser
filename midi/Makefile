
PICDIR = ./pic/

########################################################
CC = g++
LIBS = 

CPPFLAGS = -Wall -g3

########################################################
objects = Midi.o

all: main.o $(objects)
	$(CC) -o test main.o $(objects) $(CPPFLAGS) $(LIBS)


clean:
	rm -r -f test
	rm -r -f *.o *.p?m *.mid? *.pdf *.ps *.xml
