#include <stdio.h>
#include <unistd.h>

// Minimal binary - no system(), no libc functions we can abuse
void vuln() {
    char buffer[128];

    write(1, "===== Ret2Syscall Challenge =====\n", 35);
    write(1, "No system() for you!\n", 21);
    write(1, "Payload: ", 9);

    read(0, buffer, 400);
}

int main() {
    vuln();
    return 0;
}
