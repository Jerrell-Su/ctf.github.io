#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Hidden flag - not directly referenced in code flow
char secret_flag[] = "flag{blind_format_string_exploit}";

void process_input() {
    char buffer[200];

    printf("Enter data: ");
    fflush(stdout);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return;

    // Blind format string - no output shown!
    snprintf(buffer, sizeof(buffer), buffer);

    // But we can detect if we crashed or not
    printf("Processed successfully!\n");
}

void vuln() {
    int check = 0xbadc0de;

    printf("===== Blind Format String =====\n");

    for (int i = 0; i < 5; i++) {
        process_input();

        // Check if something was corrupted
        if (check != 0xbadc0de) {
            printf("❌ Corruption detected! check = 0x%x\n", check);
            printf("🎉 Here's your reward: %s\n", secret_flag);
            return;
        }
    }

    printf("No corruption detected.\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    vuln();

    return 0;
}
