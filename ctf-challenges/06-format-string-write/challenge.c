#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void win() {
    printf("🎉 You win!\n");
    system("/bin/cat flag.txt");
}

void vuln() {
    char buffer[256];

    printf("===== Format String Write Challenge =====\n");
    printf("win() is at: %p\n", win);
    printf("puts@GOT is at: %p\n", &puts);

    while (1) {
        printf("\nEnter format string (or 'quit'): ");
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;

        if (strncmp(buffer, "quit", 4) == 0)
            break;

        printf(buffer);  // Format string vulnerability

        puts("Try again!");
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    return 0;
}
