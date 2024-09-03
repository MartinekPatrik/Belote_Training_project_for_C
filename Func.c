#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

#define Player_Count 4

extern int a;

#ifdef _WIN32
#include <windows.h>
#define CLEAR() system("cls")
#else
#include <unistd.h>
#define CLEAR() system("clear")
#endif

typedef struct Cards {
    char Color;
    char Type;
} Cards;

typedef struct Player {
    int ID;
    Cards* Hand;   // Now dynamically allocated
    int isBot;
    int cardCount;
} Player;

static char TrumpColor; // Variable to store the current trump color

int getTrumpRank(char type);
int getNonTrumpRank(char type);
int getCardPoints(Cards card);
int compareCardsBelot(Cards cardA, Cards cardB, char leadSuit);
void Shuffle(Cards* Deck);
int compareCards(const void* a, const void* b);
void DealHand(Cards* Deck, Player* Players);
void RemoveCardFromHand(Player* player, int cardIndex);
int HasSuit(Player* player, char suit);
void ChooseTrumpCard(Player* player);
Cards HumanTurn(Player* player, char* leadSuit);
Cards BotTurn(Player* player, char* leadSuit);
int DetermineRoundWinner(Player* Players, Cards* playedCards, char leadSuit);
int getCardValue(char type);
bool hasSequence(Player* player, int* sequenceLength);
bool hasBelote(Player* player);
bool hasFourOfAKind(Player* player);
int calculateCombinedCall(Player* player);
int compareTeamCalls(Player* Players);
void PlayGame(Player* Players);
void FreeMemory(Player* Players, Cards* Deck);
void test();
void countToTen(int n);

int game() {
    test();
    countToTen(1);
    // Allocate memory for the deck dynamically
    Cards* Deck = (Cards*)calloc(32, sizeof(Cards));
    if (!Deck) {
        perror("Alokacija memorije za deck neuspjela");
        return 1;
    }

    // Allocate memory for players dynamically
    Player* Players = (Player*)calloc(Player_Count, sizeof(Player));
    if (!Players) {
        perror("Alokacija memorije za igrace neuspjela");
        free(Deck);
        return 1;
    }

    // Initialize player hands dynamically
    for (int i = 0; i < Player_Count; i++) {
        Players[i].Hand = (Cards*)calloc(8, sizeof(Cards));
        if (!Players[i].Hand) {
            perror("Alokacija memorije za ruku neuspjela");
            FreeMemory(Players, Deck);  // Free previously allocated memory
            return 1;
        }
    }

    // Initialize the deck
    memcpy(Deck, (Cards[32]) {
        {'Z', '7'}, { 'Z', '8' }, { 'Z', '9' }, { 'Z', '0' }, { 'Z', 'J' }, { 'Z', 'Q' }, { 'Z', 'K' }, { 'Z', 'A' },
        { 'S', '7' }, { 'S', '8' }, { 'S', '9' }, { 'S', '0' }, { 'S', 'J' }, { 'S', 'Q' }, { 'S', 'K' }, { 'S', 'A' },
        { 'L', '7' }, { 'L', '8' }, { 'L', '9' }, { 'L', '0' }, { 'L', 'J' }, { 'L', 'Q' }, { 'L', 'K' }, { 'L', 'A' },
        { 'K', '7' }, { 'K', '8' }, { 'K', '9' }, { 'K', '0' }, { 'K', 'J' }, { 'K', 'Q' }, { 'K', 'K' }, { 'K', 'A' }
    }, 32 * sizeof(Cards));

    Players[0].isBot = 0;
    Players[1].isBot = 1;
    Players[2].isBot = 1;
    Players[3].isBot = 1;

    Shuffle(Deck);
    DealHand(Deck, Players);
    PlayGame(Players);

    FreeMemory(Players, Deck);  // Free all dynamically allocated memory
    return 0;
}

void countToTen(int n) {
    if (n > 10) {
        return; // Base case: stop recursion when n is greater than 10
    }
    printf("%d\n", n); // Print the current number
    countToTen(n + 1); // Recursive call: count to the next number
}

void FreeMemory(Player* Players, Cards* Deck) {
    // Free each player's hand
    for (int i = 0; i < Player_Count; i++) {
        free(Players[i].Hand);
    }
    // Free players and deck
    free(Players);
    free(Deck);
}

// Get the rank of a card in the trump suit
int getTrumpRank(char type) {
    switch (type) {
    case 'J': return 7; // Jack is the highest
    case '9': return 6; // Nine is the second highest in trump
    case 'A': return 5;
    case '0': return 4; // 10 is higher than King
    case 'K': return 3;
    case 'Q': return 2;
    case '8': return 1;
    case '7': return 0;
    default: return -1;
    }
}

// Get the rank of a card in a non-trump suit
int getNonTrumpRank(char type) {
    switch (type) {
    case 'A': return 7; // Ace is the highest
    case '0': return 6; // 10 is the second highest
    case 'K': return 5;
    case 'Q': return 4;
    case 'J': return 3; // Jack is 5th in non-trump suits
    case '9': return 2;
    case '8': return 1;
    case '7': return 0;
    default: return -1;
    }
}

// Get the points value of a card based on its suit
int getCardPoints(Cards card) {
    if (card.Color == TrumpColor) {
        switch (card.Type) {
        case 'J': return 20;
        case '9': return 14;
        case 'A': return 11;
        case '0': return 10;
        case 'K': return 4;
        case 'Q': return 3;
        default: return 0;
        }
    }
    else {
        switch (card.Type) {
        case 'A': return 11;
        case '0': return 10;
        case 'K': return 4;
        case 'Q': return 3;
        case 'J': return 2;
        default: return 0;
        }
    }
}

// Determine the strength of two cards based on Belote rules
int compareCardsBelot(Cards cardA, Cards cardB, char leadSuit) {
    if (cardA.Color == TrumpColor && cardB.Color != TrumpColor) return 1;
    if (cardB.Color == TrumpColor && cardA.Color != TrumpColor) return -1;

    if (cardA.Color == TrumpColor && cardB.Color == TrumpColor) {
        return getTrumpRank(cardA.Type) - getTrumpRank(cardB.Type);
    }

    if (cardA.Color == leadSuit && cardB.Color == leadSuit) {
        return getNonTrumpRank(cardA.Type) - getNonTrumpRank(cardB.Type);
    }

    if (cardA.Color == leadSuit && cardB.Color != leadSuit) return 1;
    if (cardB.Color == leadSuit && cardA.Color != leadSuit) return -1;

    return 0;
}

// Shuffle the deck of cards
void Shuffle(Cards* Deck) {
    srand(time(NULL));
    for (int i = 31; i > 0; i--) {
        int j = rand() % (i + 1);
        Cards temp = Deck[i];
        Deck[i] = Deck[j];
        Deck[j] = temp;
    }
}

// Compare cards during sorting
int compareCards(const void* a, const void* b) {
    Cards* cardA = (Cards*)a;
    Cards* cardB = (Cards*)b;

    if (cardA->Color < cardB->Color) return -1;
    if (cardA->Color > cardB->Color) return 1;

    int rankA = getTrumpRank(cardA->Type);
    int rankB = getNonTrumpRank(cardB->Type);

    return rankB - rankA;
}

// Deal the cards to the players
void DealHand(Cards* Deck, Player* Players) {
    int cardIndex = 0;
    for (int i = 0; i < Player_Count; i++) {
        for (int j = 0; j < 8; j++) {
            Players[i].Hand[j] = Deck[cardIndex++];
        }
        Players[i].ID = i + 1;
        Players[i].cardCount = 8;
        qsort(Players[i].Hand, 8, sizeof(Cards), compareCards);
    }
}

// Remove a card from a player's hand after it is played
void RemoveCardFromHand(Player* player, int cardIndex) {
    for (int i = cardIndex; i < player->cardCount - 1; i++) {
        player->Hand[i] = player->Hand[i + 1];
    }
    player->cardCount--;
}

// Check if a player has a card of the specified suit
int HasSuit(Player* player, char suit) {
    for (int i = 0; i < player->cardCount; i++) {
        if (player->Hand[i].Color == suit) {
            return 1;
        }
    }
    return 0;
}

// Choose the trump suit at the start of the game
void ChooseTrumpCard(Player* player) {
    if (!player->isBot) {
        printf("%d. igrac. Vaša ruka je:\n", player->ID);
        for (int i = 0; i < player->cardCount - 2; i++) {
            printf(" %c%c", player->Hand[i].Color, player->Hand[i].Type);
        }
        printf("\n%d. igrac bira adut (Z/L/K/S): ", player->ID);
        do {
            printf("Izaberi adut (Z, L, K, S): ");
            scanf_s(" %c", &TrumpColor, 1);

            TrumpColor = toupper(TrumpColor);  // Convert to uppercase to ensure consistency
        } while (TrumpColor != 'Z' && TrumpColor != 'L' && TrumpColor != 'K' && TrumpColor != 'S');

    }
    else {
        TrumpColor = player->Hand[rand() % player->cardCount].Color;
        printf("Bot %d bira adut %c.\n", player->ID, TrumpColor);
    }
}

// Handle the turn for a human player
Cards HumanTurn(Player* player, char* leadSuit) {
    printf("%d. igrac. Vaša ruka je:\n", player->ID);
    for (int i = 0; i < player->cardCount; i++) {
        printf(" %c%c", player->Hand[i].Color, player->Hand[i].Type);
    }

    char input[3]; // Expecting "CA", "H0", etc.
    Cards playedCard;
    while (1) {
        printf("\nUpisite kartu koju zelite igrati, a imate u ruci (npr., ZJ, S0): ");
        scanf_s("%2s", input, (unsigned)_countof(input)); // Read 2-character input

        // Convert input to uppercase
        input[0] = toupper(input[0]);
        input[1] = toupper(input[1]);

        playedCard.Color = input[0];
        playedCard.Type = input[1];

        // Find the card in the player's hand
        int cardIndex = -1;
        for (int i = 0; i < player->cardCount; i++) {
            if (player->Hand[i].Color == playedCard.Color && player->Hand[i].Type == playedCard.Type) {
                cardIndex = i;
                break;
            }
        }

        // Validate choice based on lead suit
        if (cardIndex != -1 && (playedCard.Color == *leadSuit || !HasSuit(player, *leadSuit))) {
            RemoveCardFromHand(player, cardIndex);
            break;
        }

        printf("Kriva karta, pratite boju: %c ili odaberite adut.\n", *leadSuit);
    }

    if (*leadSuit == '\0') {
        *leadSuit = playedCard.Color; // Set the lead suit if it's the first card played
    }

    printf("%d. igrac igra %c%c.\n", player->ID, playedCard.Color, playedCard.Type);
    return playedCard;
}

// Handle the turn for a bot player
Cards BotTurn(Player* player, char* leadSuit) {
    int choice = -1;

    // If it's the bot's first move, set the lead suit
    if (*leadSuit == '\0') {
        choice = 0; // Bot plays the first card (can be randomized for more complexity)
        *leadSuit = player->Hand[choice].Color;
    }
    else {
        // Choose a card based on lead suit
        for (int i = 0; i < player->cardCount; i++) {
            if (player->Hand[i].Color == *leadSuit) {
                choice = i;
                break;
            }
        }

        if (choice == -1) {
            choice = rand() % player->cardCount; // Choose a random card if no lead suit
        }
    }

    Cards playedCard = player->Hand[choice];
    printf("Bot %d igra %c%c.\n", player->ID, playedCard.Color, playedCard.Type);
    RemoveCardFromHand(player, choice); // Remove the chosen card from the hand
    return playedCard;
}

// Determine the winner of the round based on the cards played
int DetermineRoundWinner(Player* Players, Cards playedCards[Player_Count], char leadSuit) {
    int winningIndex = 0;

    for (int i = 1; i < Player_Count; i++) {
        if (compareCardsBelot(playedCards[i], playedCards[winningIndex], leadSuit) > 0) {
            winningIndex = i;
        }
    }

    printf("%d. igrac pobjeduje rundu.\n", Players[winningIndex].ID);
    return winningIndex;
}

// Get the numeric value of a card's type
int getCardValue(char type) {
    switch (type) {
    case '7': return 0;
    case '8': return 1;
    case '9': return 2;
    case '0': return 3; // 10 is represented as '0'
    case 'J': return 4;
    case 'Q': return 5;
    case 'K': return 6;
    case 'A': return 7;
    default: return -1;
    }
}

// Detect sequences of 3 or more cards in line
bool hasSequence(Player* player, int* sequenceLength) {
    int counts[4][8] = { 0 }; // 4 suits, 8 possible values

    // Count occurrences of each card value per suit
    for (int i = 0; i < player->cardCount; i++) {
        int suitIndex = 0;
        switch (player->Hand[i].Color) {
        case 'H': suitIndex = 0; break;
        case 'S': suitIndex = 1; break;
        case 'L': suitIndex = 2; break;
        case 'C': suitIndex = 3; break;
        }
        int cardValue = getCardValue(player->Hand[i].Type);
        counts[suitIndex][cardValue]++;
    }

    // Check for sequences of 3 or more in each suit
    for (int suit = 0; suit < 4; suit++) {
        int currentSequence = 0;
        for (int value = 0; value < 8; value++) {
            if (counts[suit][value] > 0) {
                currentSequence++;
                if (currentSequence >= 3) {
                    *sequenceLength = currentSequence;
                    return true;
                }
            }
            else {
                currentSequence = 0;
            }
        }
    }
    return false;
}

// Detect Belote (King and Queen in Trump Suit)
bool hasBelote(Player* player) {
    bool hasKing = false;
    bool hasQueen = false;

    for (int i = 0; i < player->cardCount; i++) {
        if (player->Hand[i].Color == TrumpColor) {
            if (player->Hand[i].Type == 'K') {
                hasKing = true;
            }
            if (player->Hand[i].Type == 'Q') {
                hasQueen = true;
            }
        }
    }

    return hasKing && hasQueen;
}

// Detect Four of a Kind for 9, 10, J, Q, K, A
bool hasFourOfAKind(Player* player) {
    int rankCounts[8] = { 0 }; // 8 possible ranks: 7, 8, 9, 10, J, Q, K, A

    for (int i = 0; i < player->cardCount; i++) {
        int rankIndex = getCardValue(player->Hand[i].Type);
        rankCounts[rankIndex]++;
    }

    // Check for four of a kind among specific ranks
    for (int i = 2; i <= 7; i++) { // 9 (index 2) to Ace (index 7)
        if (rankCounts[i] == 4) {
            return true;
        }
    }

    return false;
}

// Calculate points for sequences, Belote, and Four of a Kind
int calculateCombinedCall(Player* player) {
    int totalPoints = 0;
    int sequenceLength;

    // Check for sequences
    if (hasSequence(player, &sequenceLength)) {
        totalPoints += (sequenceLength * 10); // Sequence points
    }

    // Check for Belote
    if (hasBelote(player)) {
        totalPoints += 20; // Belote points
    }

    // Check for Four of a Kind
    if (hasFourOfAKind(player)) {
        totalPoints += 100; // Four of a Kind points
    }

    return totalPoints;
}

// Compare combined calls between Team 1 (Players 1 & 3) and Team 2 (Players 2 & 4)
int compareTeamCalls(Player* Players) {
    int team1Points = 0;
    int team2Points = 0;

    // Calculate combined call for Players 1 and 3 (Team 1)
    team1Points += calculateCombinedCall(&Players[0]);
    team1Points += calculateCombinedCall(&Players[2]);

    // Calculate combined call for Players 2 and 4 (Team 2)
    team2Points += calculateCombinedCall(&Players[1]);
    team2Points += calculateCombinedCall(&Players[3]);

    printf("Team 1 (Igraci 1 & 3) Kombinirano zvanje: %d\n", team1Points);
    printf("Team 2 (Igraci 2 & 4) Kombinirano zvanje: %d\n", team2Points);

    if (team1Points > team2Points) {
        printf("Tim 1 ima vece kombinirano zvanje.\n");
        return 1;
    }
    else if (team1Points < team2Points) {
        printf("Tim 2 ima vece kombinirano zvanje.\n");
        return 2;
    }
    else {
        printf("Oba tima imaju jednako zvanje.\n");
        return 0;
    }
}

// Play a full game consisting of multiple rounds
void PlayGame(Player* Players) {
    int TeamPoints[2] = { 0, 0 }; // Points for Team 1 and Team 2
    int roundWinner = 0;
    int saveScore = 0;

    ChooseTrumpCard(&Players[roundWinner]);
    printf("Adut: %c\n", TrumpColor);

    // Compare combined calls before the game starts
    int teamComparison = compareTeamCalls(Players);
    int callPoints[2] = { 0, 0 }; // Points for calls for each team

    // Distribute initial call points
    if (teamComparison == 1) {
        callPoints[0] += calculateCombinedCall(&Players[0]) + calculateCombinedCall(&Players[2]);
    }
    else if (teamComparison == 2) {
        callPoints[1] += calculateCombinedCall(&Players[1]) + calculateCombinedCall(&Players[3]);
    }

    // Play 8 turns in the round
    // Variables to store last round information
    Cards lastPlayedCards[Player_Count];
    int lastRoundWinner = -1; // Initialize with an invalid index

    // Play 8 turns in the round
    for (int turn = 0; turn < 8; turn++) {
        // Display last round information with a separator if it's not the first round
        if (turn > 0) {
            CLEAR();
            printf("=== Prosla runda ===\n");
            for (int i = 0; i < Player_Count; i++) {
                printf("%d. igrac igra %c%c\n", i + 1, lastPlayedCards[i].Color, lastPlayedCards[i].Type);
            }
            printf("Pobjednik prosle runde: Igrac %d\n", lastRoundWinner + 1);
            printf("===================\n\n");
        }

        // Clear the terminal output for the new round

        printf("Trump color: %c\n", TrumpColor); // Display the trump card

        Cards playedCards[Player_Count];
        char leadSuit = '\0';
        int RoundPoints = 0;

        for (int i = 0; i < Player_Count; i++) {
            Player* currentPlayer = &Players[(roundWinner + i) % Player_Count];

            // Display bots' cards before Player 1's turn if Player 1 is not the first player in the round
            if (currentPlayer->isBot && i == 0 && (roundWinner + i) % Player_Count != 0) {
                playedCards[(roundWinner + i) % Player_Count] = BotTurn(currentPlayer, &leadSuit);
                printf("Bot %d igra %c%c.\n", currentPlayer->ID, playedCards[(roundWinner + i) % Player_Count].Color, playedCards[(roundWinner + i) % Player_Count].Type);
            }
            else if (currentPlayer->isBot) {
                playedCards[(roundWinner + i) % Player_Count] = BotTurn(currentPlayer, &leadSuit);
            }
            else {
                // Player 1's turn
                playedCards[(roundWinner + i) % Player_Count] = HumanTurn(currentPlayer, &leadSuit);
            }

            RoundPoints += getCardPoints(playedCards[(roundWinner + i) % Player_Count]); // Add points for the played card
        }

        roundWinner = DetermineRoundWinner(Players, playedCards, leadSuit);

        // Assign points to the winning team
        if (roundWinner == 0 || roundWinner == 2) {
            TeamPoints[0] += RoundPoints; // Team 1 points
        }
        else {
            TeamPoints[1] += RoundPoints; // Team 2 points
        }

        // At the end of the 8th round, award 10 extra points to the round winner
        if (turn == 7) {
            if (roundWinner == 0 || roundWinner == 2) {
                TeamPoints[0] += 10; // Team 1 gets 10 extra points
            }
            else {
                TeamPoints[1] += 10; // Team 2 gets 10 extra points
            }
        }

        // Store the current round information for the next round display
        for (int i = 0; i < Player_Count; i++) {
            lastPlayedCards[i] = playedCards[i];
        }
        lastRoundWinner = roundWinner;
    }

    // Ensure total points are capped at 162
    int totalPoints = TeamPoints[0] + TeamPoints[1];
    if (totalPoints > 162) {
        int excessPoints = totalPoints - 162;
        if (TeamPoints[0] > TeamPoints[1]) {
            TeamPoints[0] -= excessPoints;
        }
        else {
            TeamPoints[1] -= excessPoints;
        }
    }

    // Add call points after final round points have been capped
    if (TeamPoints[0] > TeamPoints[1]) {
        TeamPoints[0] += callPoints[0];
    }
    else if (TeamPoints[1] > TeamPoints[0]) {
        TeamPoints[1] += callPoints[1];
    }
    printf("\n===================\n\n");
    printf("Zavrsni bodovin");
    printf("Tim 1: %d\n", TeamPoints[0]);
    printf("Tim 2: %d\n", TeamPoints[1]);
    while (1) {
        int choice;
        printf("\nGotova igra!\n");
        printf("1. Igraj ponovo\n");
        printf("2. Spremi bodove na ljestvicu\n");
        printf("3. Vrati se u glavni izbornik\n");
        printf("Odaberi: ");
        scanf("%d", &choice);

        switch (choice) {
        case 1:  // Play Again
            return game();  // Restart the game

        case 2:  // Save Score to Leaderboard
            saveScore = 1;
            break;

        case 3:  // Quit to Main Menu
            if (saveScore) {
                printf("Bodovi spremljeni na ljestvicu bodova.\n");
            }
            return 0;

        default:
            printf("Krivi odabir! Pokusaj ponovo.\n");
            continue;
        }

        if (saveScore) {
            // Save scores to leaderboard
            FILE* file = fopen("leaderboard.txt", "a");
            if (file == NULL) {
                perror("Greska pri otvaranju ljestvice");
                return 1;
            }

            // Record the final scores of both teams to the leaderboard
            fprintf(file, "Tim 1: %d boda, Tim 2: %d boda\n", TeamPoints[0], TeamPoints[1]);
            fclose(file);

            printf("Bodovi spremljeni na ljestvicu bodova.\n");
            saveScore = 0;  // Reset save flag after saving
        }
    }
}

void test() {
    FILE* file = fopen("test.txt", "a");
    if (file != NULL) {
        fclose(file);
        remove("test.txt");
    }
    else {
        printf("Failed to open the file.\n");
    }
}
