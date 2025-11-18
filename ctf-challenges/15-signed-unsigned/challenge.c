#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void win() {
    printf("🎉 You win!\n");
    system("/bin/cat flag.txt");
}

void vuln() {
    char buffer[128];
    int size;

    printf("===== Signed/Unsigned Confusion =====\n");
    printf("How many bytes to copy? ");
    fflush(stdout);

    scanf("%d", &size);  // Read as signed int

    // Validation (but flawed!)
    if (size > sizeof(buffer)) {
        printf("Too large!\n");
        return;
    }

    if (size <= 0) {
        printf("Must be positive!\n");
        return;
    }

    printf("Copying %d bytes...\n", size);

    char source[256];
    printf("Enter data: ");
    fflush(stdout);

    // Read into source buffer
    read(0, source, sizeof(source));

    // Bug: memcpy takes size_t (unsigned), but size is signed
    // If we pass -1, it fails the checks, but... wait, we check for <= 0
    // But: what if we pass a value that's valid as signed but large as unsigned?

    // Actually, the bug is different:
    // We passed checks, but memcpy interprets size as unsigned size_t
    memcpy(buffer, source, size);

    printf("Done!\n");
}

// Better vulnerability:
void vuln2() {
    char buffer[64];
    char source[256];

    printf("\n===== Try Again =====\n");
    printf("Size: ");
    fflush(stdout);

    int size;
    scanf("%d", &size);

    // Checks size as signed
    if (size > 64) {
        printf("Too big!\n");
        return;
    }

    printf("Data: ");
    fflush(stdout);
    read(0, source, sizeof(source));

    // Bug: read expects unsigned size_t
    // If size is negative (e.g., -1), it passes check (−1 < 64)
    // But read() sees it as huge unsigned: 0xFFFFFFFF
    read(0, buffer, size);  // DANGEROUS!

    printf("Copied!\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln2();

    return 0;
}
