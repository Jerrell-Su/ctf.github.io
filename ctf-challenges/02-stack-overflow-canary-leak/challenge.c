#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void win() {
    printf("🎉 You win!\n");
    system("/bin/cat flag.txt");
}

void vuln() {
    char buffer[64];
    char canary_leak[16];

    printf("===== Stack Canary Challenge =====\n");
    printf("Hint: Stack address: %p\n", &buffer);

    // First interaction: leak the canary
    printf("What's your name? ");
    fflush(stdout);
    read(0, buffer, 80);  // Slight overflow to leak canary
    printf("Welcome, %s\n", buffer);  // Leaks canary if no null byte

    // Second interaction: exploit
    printf("Enter your message: ");
    fflush(stdout);
    read(0, buffer, 200);  // Big overflow
    printf("Message received!\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    printf("Goodbye!\n");
    return 0;
}
