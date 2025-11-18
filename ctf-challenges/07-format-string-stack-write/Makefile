CC = gcc
CFLAGS = -m64 -fno-stack-protector -no-pie -Wno-format-security

all: challenge

challenge: challenge.c
	$(CC) $(CFLAGS) -o challenge challenge.c

clean:
	rm -f challenge

.PHONY: all clean
