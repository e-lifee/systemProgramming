CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11

all: tarsau

tarsau: tarsau.c
	$(CC) $(CFLAGS) -o tarsau tarsau.c

clean:
	rm -f tarsau

