#DEBUG     = -g

CC        = gcc
CPPFLAGS  = $(DEBUG) 
CCFLAGS   = -Wall $(DEBUG) -std=gnu99 
LDFLAGS   = 


all: microrl_test


microrl_test: microrl_tst.o microrl.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -f *.o microrl_test
