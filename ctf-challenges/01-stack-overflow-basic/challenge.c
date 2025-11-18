#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void win() {
    printf("🎉 Congratulations! You got the flag!\n");
    system("/bin/cat flag.txt");
}

void vuln() {
    char buffer[64];
    printf("Enter your name: ");
    fflush(stdout);
    gets(buffer);  // Classic buffer overflow
    printf("Hello, %s!\n", buffer);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    printf("===== Stack Overflow Challenge 01 =====\n");
    printf("Can you overflow the buffer?\n");
    vuln();

    printf("Goodbye!\n");
    return 0;
}
