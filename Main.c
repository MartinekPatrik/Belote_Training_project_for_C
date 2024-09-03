#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "func.h"
#include "leaderboard.h"

// Enum to represent menu options
typedef enum {
    PLAY,
    VIEW,
    DELETE,
    SEARCH,
    EXIT,
    INVALID
} MenuOption;

void displayMenu() {
    printf("Menu:\n");
    printf("play - Play Game\n");
    printf("view - View Leaderboard\n");
    printf("delete - Delete Leaderboard\n");
    printf("search - Search by Points\n");
    printf("exit - Exit\n");
    printf("Enter your choice: ");
}

// Convert string to corresponding enum value
MenuOption getMenuOption(char* str) {
    if (strcmp(str, "play") == 0) return PLAY;
    if (strcmp(str, "view") == 0) return VIEW;
    if (strcmp(str, "delete") == 0) return DELETE;
    if (strcmp(str, "search") == 0) return SEARCH;
    if (strcmp(str, "exit") == 0) return EXIT;
    return INVALID;
}

// Convert string to lowercase
void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int getValidPoints() {
    int points;
    while (1) {
        if (scanf("%d", &points) != 1) {  // If input is not a valid integer
            printf("Invalid input! Please enter a valid integer: ");
            while (getchar() != '\n');  // Clear the invalid input
        }
        else {
            return points;
        }
    }
}

// Function to clear the terminal
void clear() {
#ifdef _WIN32
    system("cls");  // For Windows
#else
    system("clear");  // For Unix/Linux/MacOS
#endif
}

int main() {
    char choice[10];
    MenuOption option;

    while (1) {
        displayMenu();
        scanf("%s", choice);

        // Convert input to lowercase
        toLowerCase(choice);

        // Map the input string to the corresponding enum value
        option = getMenuOption(choice);

        // Handle the user's choice using a switch statement
        switch (option) {
        case PLAY:
            clear();
            game();
            clear();
            break;
        case VIEW:
            clear();
            viewLeaderboard();
            printf("\n");
            break;
        case DELETE:
            clear();
            deleteLeaderboard();
            printf("\n");
            break;
        case SEARCH:
            clear();
            printf("Enter the points to search for: ");
            int points = getValidPoints();
            searchByPoints(points);
            printf("\n");
            break;
        case EXIT:
            printf("Exiting the game. Goodbye!\n");
            return 0;
        case INVALID:
        default:
            clear();
            printf("Invalid choice! Please try again.\n");
            break;
        }

        printf("Press Enter to continue...\n");
        while (getchar() != '\n');  // Wait for Enter key
    }
}
