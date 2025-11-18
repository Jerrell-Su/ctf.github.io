#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void vuln() {
    char buffer[128];

    printf("===== Ret2libc Challenge =====\n");
    printf("printf is at: %p\n", printf);  // Leak libc address
    printf("Enter your payload: ");
    fflush(stdout);

    read(0, buffer, 512);  // Buffer overflow
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    return 0;
}
