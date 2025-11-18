#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

void vuln() {
    char buffer[256];

    printf("===== Shellcode Challenge =====\n");
    printf("Buffer is at: %p\n", buffer);
    printf("Send your shellcode: ");
    fflush(stdout);

    read(0, buffer, 512);
    printf("Executing...\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    // Make stack executable for this challenge
    void *stack_page = (void *)((unsigned long)&buffer & ~0xFFF);
    mprotect(stack_page, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);

    vuln();

    return 0;
}
