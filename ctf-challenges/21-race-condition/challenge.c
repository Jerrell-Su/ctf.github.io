#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_USERS 3

typedef struct {
    char username[32];
    int credits;
    int is_admin;
} User;

User users[MAX_USERS];
int user_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void print_flag() {
    printf("🎉 Flag unlocked!\n");
    system("/bin/cat flag.txt");
}

void create_user() {
    if (user_count >= MAX_USERS) {
        printf("Max users reached!\n");
        return;
    }

    printf("Username: ");
    fflush(stdout);

    fgets(users[user_count].username, sizeof(users[user_count].username), stdin);
    users[user_count].username[strcspn(users[user_count].username, "\n")] = 0;

    users[user_count].credits = 10;
    users[user_count].is_admin = 0;

    user_count++;
    printf("User created! You have 10 credits.\n");
}

void transfer_credits() {
    int from, to, amount;

    printf("From user index: ");
    scanf("%d", &from);
    printf("To user index: ");
    scanf("%d", &to);
    printf("Amount: ");
    scanf("%d", &amount);
    getchar();

    // Bounds checking
    if (from < 0 || from >= user_count || to < 0 || to >= user_count) {
        printf("Invalid user index!\n");
        return;
    }

    // Bug: TOCTOU - Time-of-Check-Time-of-Use
    // Check credits without lock
    if (users[from].credits < amount) {
        printf("Insufficient credits!\n");
        return;
    }

    // Small delay (simulates processing time)
    usleep(100000);  // 100ms

    // Use credits without re-checking
    // If multiple threads run simultaneously, this can be exploited!
    pthread_mutex_lock(&lock);
    users[from].credits -= amount;
    users[to].credits += amount;
    pthread_mutex_unlock(&lock);

    printf("Transfer successful!\n");
}

void buy_admin() {
    int user_idx;

    printf("User index: ");
    scanf("%d", &user_idx);
    getchar();

    if (user_idx < 0 || user_idx >= user_count) {
        printf("Invalid user!\n");
        return;
    }

    if (users[user_idx].credits >= 1000) {
        users[user_idx].credits -= 1000;
        users[user_idx].is_admin = 1;
        printf("Admin access granted!\n");
    } else {
        printf("Need 1000 credits! You have: %d\n", users[user_idx].credits);
    }
}

void show_users() {
    printf("\n=== Users ===\n");
    for (int i = 0; i < user_count; i++) {
        printf("%d. %s - Credits: %d - Admin: %s\n",
               i, users[i].username, users[i].credits,
               users[i].is_admin ? "Yes" : "No");
    }
}

void check_admin() {
    int user_idx;

    printf("User index: ");
    scanf("%d", &user_idx);
    getchar();

    if (user_idx < 0 || user_idx >= user_count) {
        printf("Invalid user!\n");
        return;
    }

    if (users[user_idx].is_admin) {
        print_flag();
    } else {
        printf("Not an admin!\n");
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    printf("===== Bank System with Race Condition =====\n");
    printf("Hint: The transfer function has a TOCTOU bug!\n");
    printf("You need 1000 credits to become admin.\n\n");

    while (1) {
        printf("\n[1] Create user\n[2] Transfer credits\n[3] Buy admin\n[4] Show users\n[5] Check admin\n[6] Exit\n");
        printf("Choice: ");
        fflush(stdout);

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                create_user();
                break;
            case 2:
                transfer_credits();
                break;
            case 3:
                buy_admin();
                break;
            case 4:
                show_users();
                break;
            case 5:
                check_admin();
                break;
            case 6:
                return 0;
            default:
                printf("Invalid choice!\n");
        }
    }

    return 0;
}
