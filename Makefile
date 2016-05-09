CFLAGS=-Wall -g
LDLIBS=-pthread -lm

all: ht htg

clean:
	rm ht
	rm htg

