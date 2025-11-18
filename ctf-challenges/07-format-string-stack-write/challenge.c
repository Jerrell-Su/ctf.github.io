#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void vuln() {
    char buffer[128];
    int auth = 0;

    printf("===== Format String Stack Write =====\n");
    printf("Auth variable is at: %p\n", &auth);
    printf("Auth value: %d\n", auth);

    printf("\nEnter your input: ");
    fflush(stdout);

    fgets(buffer, sizeof(buffer), stdin);
    printf(buffer);  // Format string vuln

    printf("\nAuth value after: %d\n", auth);

    if (auth == 0x1337c0de) {
        printf("🎉 Authentication successful!\n");
        system("/bin/cat flag.txt");
    } else {
        printf("❌ Authentication failed!\n");
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    return 0;
}
