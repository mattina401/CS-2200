#
# Homework 3 Makefile
#

CC     = gcc
CFLAGS = -g -Wall -pedantic
LIBS   = -lpthread
ALL    = hw3

hw3:	hw3.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(ALL) *.o
