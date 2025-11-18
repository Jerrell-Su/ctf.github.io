#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Useful functions that are never called
void gadget1() {
    asm("pop %rdi; ret");
}

void gadget2() {
    asm("pop %rsi; ret");
}

void read_flag(int fd, char *buf) {
    // This function exists but is never called
    // fd should be file descriptor for flag.txt
    // buf should be a writable buffer
    FILE *f = fdopen(fd, "r");
    if (f) {
        fgets(buf, 64, f);
        printf("Flag: %s\n", buf);
        fclose(f);
    }
}

void vuln() {
    char buffer[64];

    printf("===== Basic ROP Challenge =====\n");
    printf("read_flag is at: %p\n", read_flag);
    printf("BSS segment at: %p\n", &buffer);

    printf("\nEnter your input: ");
    fflush(stdout);

    read(0, buffer, 300);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    return 0;
}
