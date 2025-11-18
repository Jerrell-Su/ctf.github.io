#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int is_admin = 0;
char data_store[256];

void check_admin() {
    if (is_admin) {
        printf("🎉 Admin access granted!\n");
        system("/bin/cat flag.txt");
        exit(0);
    }
}

void vuln() {
    int index;
    char value;

    printf("===== Array Index Challenge =====\n");
    printf("Data store at: %p\n", data_store);
    printf("is_admin at: %p\n", &is_admin);
    printf("Offset: %ld\n", (long)((char*)&is_admin - data_store));

    while (1) {
        printf("\n[1] Read\n[2] Write\n[3] Check admin\n[4] Exit\n");
        printf("Choice: ");
        fflush(stdout);

        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1:  // Read
                printf("Index: ");
                scanf("%d", &index);

                if (index >= 0 && index < sizeof(data_store)) {
                    printf("Value: 0x%02x\n", (unsigned char)data_store[index]);
                } else {
                    printf("Invalid index!\n");
                }
                break;

            case 2:  // Write
                printf("Index: ");
                scanf("%d", &index);

                // Bug: only checks if index < size, not if index < 0!
                if (index < sizeof(data_store)) {
                    printf("Value (0-255): ");
                    int val;
                    scanf("%d", &val);
                    data_store[index] = (char)val;
                    printf("Written!\n");
                } else {
                    printf("Invalid index!\n");
                }
                break;

            case 3:  // Check
                check_admin();
                printf("Not admin!\n");
                break;

            case 4:  // Exit
                return;

            default:
                printf("Invalid choice!\n");
        }
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    memset(data_store, 0, sizeof(data_store));

    vuln();

    return 0;
}
