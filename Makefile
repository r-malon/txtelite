VERSION = 1.5.0
PROG = txtelite
SRCS = txtelite.c
OBJS = $(SRCS:.c=.o)
CFLAGS = -std=c99 -Wall -Wextra -pedantic -O2
LDFLAGS = -s

all: $(PROG)

txtelite.o: txtelite.h

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

$(PROG): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	-rm -f $(OBJS) $(PROG)

.PHONY: all clean
