#DEBUG     = -g

CC        = gcc
CPPFLAGS  = $(DEBUG) 
CCFLAGS   = -Wall $(DEBUG) -std=gnu99 
LDFLAGS   = 

TARGET=microrl_test

all: microrl_test


microrl_test: microrl_tst.o microrl.o unix_misc.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -f *.o $(TARGET)*
