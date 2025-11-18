#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void win() {
    printf("🎉 Flag captured!\n");
    system("/bin/cat flag.txt");
}

void vuln() {
    char buffer[64];
    unsigned char size;

    printf("===== Off-by-One Challenge =====\n");
    printf("Buffer at: %p\n", buffer);
    printf("win() at: %p\n", win);

    printf("\nEnter size (0-64): ");
    fflush(stdout);

    scanf("%hhu", &size);  // Read unsigned char (0-255)
    getchar();

    // Validation
    if (size > sizeof(buffer)) {
        printf("Too large!\n");
        return;
    }

    printf("Enter data: ");
    fflush(stdout);

    // Bug: off-by-one!
    // Allows writing size+1 bytes, including null terminator
    // If size=64, reads 65 bytes total!
    read(0, buffer, size);
    buffer[size] = '\0';  // Off-by-one null byte write!

    printf("Data: %s\n", buffer);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    printf("Goodbye!\n");
    return 0;
}
