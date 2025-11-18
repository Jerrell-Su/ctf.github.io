CC = gcc
CFLAGS = -m64 -fstack-protector -no-pie

all: challenge

challenge: challenge.c
	$(CC) $(CFLAGS) -o challenge challenge.c

clean:
	rm -f challenge

.PHONY: all clean
