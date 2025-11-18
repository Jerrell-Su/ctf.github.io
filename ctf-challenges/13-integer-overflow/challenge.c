#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void win() {
    printf("🎉 Access granted!\n");
    system("/bin/cat flag.txt");
}

void vuln() {
    unsigned short size;  // 16-bit: max 65535
    char buffer[64];

    printf("===== Integer Overflow Challenge =====\n");
    printf("How many bytes to read? ");
    fflush(stdout);

    scanf("%hu", &size);  // Read unsigned short

    // Validation check (bypassable!)
    if (size > sizeof(buffer)) {
        printf("Too large! Adjusting...\n");
        size = size;  // Dummy operation - demonstrates developer's attempt
    }

    // The bug: size + 16 can overflow!
    unsigned short total = size + 16;

    printf("Reading %u bytes (with %u byte header)...\n", size, total);

    if (total <= sizeof(buffer)) {
        printf("Safety check passed!\n");
        printf("Enter data: ");
        fflush(stdout);
        read(0, buffer, size);  // Use original size!
    } else {
        printf("Failed safety check!\n");
        return;
    }

    printf("Data received.\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    return 0;
}
