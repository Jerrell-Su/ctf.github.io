#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char global_buffer[512];  // Larger buffer in BSS

void win() {
    printf("🎉 Winner!\n");
    system("/bin/cat flag.txt");
}

void vuln() {
    char small_buffer[16];  // Very small stack buffer!

    printf("===== Stack Pivot Challenge =====\n");
    printf("Global buffer at: %p\n", global_buffer);
    printf("win() at: %p\n", win);

    // First, read into global buffer (lots of space)
    printf("\nSend ROP chain: ");
    fflush(stdout);
    read(0, global_buffer, sizeof(global_buffer));

    // Then, tiny overflow on stack to pivot
    printf("Overflow here: ");
    fflush(stdout);
    read(0, small_buffer, 64);  // Overflow!

    printf("Bye!\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    return 0;
}
