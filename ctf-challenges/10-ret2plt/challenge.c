#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void vuln() {
    char buffer[100];

    printf("===== Ret2PLT Challenge =====\n");
    printf("puts@PLT: %p\n", puts);
    printf("puts@GOT: %p\n", &puts);

    printf("\nSend payload: ");
    fflush(stdout);

    read(0, buffer, 300);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    return 0;
}
