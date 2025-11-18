#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    void (*print)(struct User*);
    char name[32];
    int id;
} User;

void normal_print(User* u) {
    printf("User: %s (ID: %d)\n", u->name, u->id);
}

void admin_print(User* u) {
    printf("🎉 Admin user detected!\n");
    system("/bin/cat flag.txt");
}

User* user = NULL;

void create_user() {
    if (user != NULL) {
        printf("User already exists!\n");
        return;
    }

    user = malloc(sizeof(User));
    user->print = normal_print;
    user->id = rand() % 1000;

    printf("Enter name: ");
    fflush(stdout);
    read(0, user->name, sizeof(user->name) - 1);

    printf("User created!\n");
}

void delete_user() {
    if (user == NULL) {
        printf("No user!\n");
        return;
    }

    free(user);
    printf("User deleted!\n");
    // Bug: forgot to set user = NULL!
}

void show_user() {
    if (user == NULL) {
        printf("No user!\n");
        return;
    }

    // Use-after-free: user might be freed!
    user->print(user);
}

void edit_user() {
    if (user == NULL) {
        printf("No user!\n");
        return;
    }

    printf("New name: ");
    fflush(stdout);
    // Use-after-free: writing to freed memory
    read(0, user->name, sizeof(user->name) - 1);
    printf("Updated!\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    printf("===== Use-After-Free Challenge =====\n");
    printf("admin_print: %p\n", admin_print);

    while (1) {
        printf("\n[1] Create user\n[2] Delete user\n[3] Show user\n[4] Edit user\n[5] Exit\n");
        printf("Choice: ");
        fflush(stdout);

        int choice;
        scanf("%d", &choice);
        getchar();  // Consume newline

        switch (choice) {
            case 1:
                create_user();
                break;
            case 2:
                delete_user();
                break;
            case 3:
                show_user();
                break;
            case 4:
                edit_user();
                break;
            case 5:
                return 0;
            default:
                printf("Invalid!\n");
        }
    }

    return 0;
}
