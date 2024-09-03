#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leaderboard.h"

// Function to view the leaderboard
void viewLeaderboard() {
    printf("\n--- Ljestvica bodova ---\n");

    FILE* file = fopen("leaderboard.txt", "r");
    if (file == NULL) {
        printf("Ljestvica bodova nije pronadena.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}

// Function to delete the leaderboard
void deleteLeaderboard() {
    if (remove("leaderboard.txt") == 0) {
        printf("Ljestvica bodova je obrisana.\n");
    }
    else {
        perror("Greška u brisanju ljestvice bodova");
    }
}

// Function to search for teams by points
void searchByPoints(int points) {
    printf("\n--- Rezultati pretrazivanja bodova: %d  ---\n", points);

    FILE* file = fopen("leaderboard.txt", "r");
    if (file == NULL) {
        printf("Ljestvica bodova nije pronadena.\n");
        return;
    }

    char line[256];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        int teamPoints;
        char teamName[256];

        // Update the sscanf format string to match the new structure "Team X: YY points"
        if (sscanf(line, "%[^:]: %d boda", teamName, &teamPoints) == 2) {
            if (teamPoints == points) {
                printf("%s\n", line);
                found = 1;
            }
        }
    }

    if (!found) {
        printf("Nije pronaden tim sa %d bodova.\n", points);
    }

    fclose(file);
}