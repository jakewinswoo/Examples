LIB=-lpthread -lm
CC=gcc
CFLAGS = -Wall -std=c99 -m32 

maze: proj3.c
	$(CC) $(CFLAGS) proj3.c -o maze $(LIB)
clean:
	rm maze
