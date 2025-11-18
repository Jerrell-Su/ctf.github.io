#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char name[32];
    int health;
    int coins;
    int has_flag;
} Player;

Player player;

void print_flag() {
    if (player.has_flag) {
        printf("🎉 You found the secret!\n");
        system("/bin/cat flag.txt");
    } else {
        printf("You don't have the flag yet!\n");
    }
}

void shop() {
    printf("\n=== Shop ===\n");
    printf("1. Health potion (10 coins)\n");
    printf("2. Magic sword (50 coins)\n");
    printf("3. Secret flag (1000 coins)\n");
    printf("Your coins: %d\n", player.coins);

    printf("Buy: ");
    fflush(stdout);

    int choice;
    scanf("%d", &choice);
    getchar();

    switch (choice) {
        case 1:
            if (player.coins >= 10) {
                player.coins -= 10;
                player.health += 20;
                printf("Bought health potion!\n");
            } else {
                printf("Not enough coins!\n");
            }
            break;

        case 2:
            if (player.coins >= 50) {
                player.coins -= 50;
                printf("Bought magic sword!\n");
            } else {
                printf("Not enough coins!\n");
            }
            break;

        case 3:
            if (player.coins >= 1000) {
                player.coins -= 1000;
                player.has_flag = 1;
                printf("Bought the secret flag!\n");
            } else {
                printf("Not enough coins! You need 1000.\n");
            }
            break;

        default:
            printf("Invalid choice!\n");
    }
}

void battle() {
    printf("\n=== Battle Arena ===\n");
    printf("You encounter a goblin!\n");

    char action[64];
    printf("What do you do? ");
    fflush(stdout);

    // Format string vulnerability!
    fgets(action, sizeof(action), stdin);
    printf("You try to: ");
    printf(action);  // BUG: Format string!

    printf("\nYou defeated the goblin and earned 5 coins!\n");
    player.coins += 5;
}

void save_game() {
    printf("\n=== Save Game ===\n");
    printf("Enter save name: ");
    fflush(stdout);

    // Buffer overflow vulnerability!
    char save_name[32];
    gets(save_name);  // BUG: Buffer overflow!

    printf("Game saved as: %s\n", save_name);
}

void show_stats() {
    printf("\n=== Player Stats ===\n");
    printf("Name: %s\n", player.name);
    printf("Health: %d\n", player.health);
    printf("Coins: %d\n", player.coins);
    printf("Has flag: %s\n", player.has_flag ? "Yes" : "No");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);

    printf("===== PWN ADVENTURE =====\n");
    printf("Welcome to the adventure!\n");

    printf("Enter your name: ");
    fflush(stdout);
    fgets(player.name, sizeof(player.name), stdin);
    player.name[strcspn(player.name, "\n")] = 0;

    player.health = 100;
    player.coins = 0;
    player.has_flag = 0;

    printf("\nHello, %s!\n", player.name);
    printf("Player struct at: %p\n", &player);
    printf("has_flag offset: %ld\n", (long)(&player.has_flag - (int*)&player));

    while (1) {
        printf("\n[1] Battle\n[2] Shop\n[3] Show stats\n[4] Print flag\n[5] Save game\n[6] Exit\n");
        printf("Choice: ");
        fflush(stdout);

        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                battle();
                break;
            case 2:
                shop();
                break;
            case 3:
                show_stats();
                break;
            case 4:
                print_flag();
                break;
            case 5:
                save_game();
                break;
            case 6:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice!\n");
        }
    }

    return 0;
}
