#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char* description;
    void (*handler)(void);
} Task;

void normal_handler() {
    printf("Task executed normally.\n");
}

void admin_handler() {
    printf("🎉 Admin task executed!\n");
    system("/bin/cat flag.txt");
}

Task* task1 = NULL;
Task* task2 = NULL;

void create_tasks() {
    printf("===== Creating Tasks =====\n");

    // Allocate task1
    task1 = malloc(sizeof(Task));
    task1->description = malloc(64);  // Small buffer
    task1->handler = normal_handler;

    // Allocate task2 right after
    task2 = malloc(sizeof(Task));
    task2->description = malloc(64);
    task2->handler = normal_handler;

    printf("Task 1 at: %p\n", task1);
    printf("Task 1 description at: %p\n", task1->description);
    printf("Task 2 at: %p\n", task2);
    printf("Task 2 description at: %p\n", task2->description);
    printf("admin_handler at: %p\n", admin_handler);

    strcpy(task1->description, "Normal task 1");
    strcpy(task2->description, "Normal task 2");

    printf("Tasks created!\n");
}

void edit_task1() {
    printf("Enter new description for task 1: ");
    fflush(stdout);

    // Bug: no bounds checking!
    read(0, task1->description, 200);  // Can overflow!

    printf("Task 1 updated!\n");
}

void execute_tasks() {
    printf("\n=== Executing Tasks ===\n");

    printf("Task 1: %s\n", task1->description);
    task1->handler();

    printf("Task 2: %s\n", task2->description);
    task2->handler();  // If we overflow, we can control this!
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    create_tasks();

    while (1) {
        printf("\n[1] Edit task 1\n[2] Execute tasks\n[3] Exit\n");
        printf("Choice: ");
        fflush(stdout);

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                edit_task1();
                break;
            case 2:
                execute_tasks();
                break;
            case 3:
                return 0;
            default:
                printf("Invalid!\n");
        }
    }

    return 0;
}
