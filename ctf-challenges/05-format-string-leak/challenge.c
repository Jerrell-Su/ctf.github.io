#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char flag[] = "flag{format_string_leak_secret}";
int secret = 0xdeadbeef;

void vuln() {
    char buffer[128];

    printf("===== Format String Leak Challenge =====\n");
    printf("Can you leak the secret?\n");
    printf("Secret is at: %p\n", &secret);
    printf("Flag is at: %p\n", flag);

    printf("Enter format string: ");
    fflush(stdout);

    fgets(buffer, sizeof(buffer), stdin);
    printf("You entered: ");
    printf(buffer);  // Format string vulnerability!
    printf("\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    printf("Secret value: 0x%x\n", secret);
    return 0;
}
