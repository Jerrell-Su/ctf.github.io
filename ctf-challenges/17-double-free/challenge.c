#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NOTES 5

typedef struct {
    char* content;
    int is_free;
} Note;

Note notes[MAX_NOTES];
int note_count = 0;

void win() {
    printf("🎉 Shell obtained!\n");
    system("/bin/sh");
}

void create_note() {
    if (note_count >= MAX_NOTES) {
        printf("Too many notes!\n");
        return;
    }

    int idx = note_count++;
    notes[idx].content = malloc(128);
    notes[idx].is_free = 0;

    printf("Enter note content: ");
    fflush(stdout);
    read(0, notes[idx].content, 128);

    printf("Note %d created!\n", idx);
}

void delete_note() {
    printf("Note index: ");
    fflush(stdout);

    int idx;
    scanf("%d", &idx);
    getchar();

    if (idx < 0 || idx >= note_count) {
        printf("Invalid index!\n");
        return;
    }

    if (notes[idx].is_free) {
        printf("Already freed!\n");
        return;
    }

    free(notes[idx].content);
    notes[idx].is_free = 1;
    printf("Note %d deleted!\n", idx);
}

void delete_all() {
    printf("Deleting all notes...\n");

    for (int i = 0; i < note_count; i++) {
        // Bug: doesn't check is_free flag!
        free(notes[i].content);
        printf("Freed note %d\n", i);
    }

    note_count = 0;
    printf("All notes deleted!\n");
}

void show_note() {
    printf("Note index: ");
    fflush(stdout);

    int idx;
    scanf("%d", &idx);
    getchar();

    if (idx < 0 || idx >= note_count) {
        printf("Invalid index!\n");
        return;
    }

    printf("Content: %s\n", notes[idx].content);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    printf("===== Double-Free Challenge =====\n");
    printf("win() at: %p\n", win);

    while (1) {
        printf("\n[1] Create note\n[2] Delete note\n[3] Delete all\n[4] Show note\n[5] Exit\n");
        printf("Choice: ");
        fflush(stdout);

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                create_note();
                break;
            case 2:
                delete_note();
                break;
            case 3:
                delete_all();
                break;
            case 4:
                show_note();
                break;
            case 5:
                return 0;
            default:
                printf("Invalid!\n");
        }
    }

    return 0;
}
