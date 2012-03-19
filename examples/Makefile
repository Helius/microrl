#DEBUG     = -g

CC        = gcc
CPPFLAGS  = $(DEBUG) 
CCFLAGS   = -Wall $(DEBUG) -std=gnu99 
LDFLAGS   = 

TARGET=microrl_test

all: microrl_test


microrl_test: example.o ../src/microrl.o  unix_misc/unix_misc.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< $(CCFLAGS) -o $(*).o

clean:
	rm -f unix_misc/*.o ../src/*.o *.o $(TARGET)*
