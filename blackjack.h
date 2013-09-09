#define MAX_NUMBER_OF_HANDS 16

struct card{
	char suit;
	char face[3];
	int value;
};

struct hand{
	int hasAnAce;						// 1 if the hand contains an Ace, O otherwise
	int numberOfCards;
	int total;							// total value of the cards in a hand
	int altTotal;						// alternative total value of the cards in a hand 
	struct card cards[10];
	char status;
};

struct user{
	int balance;
	int bet[MAX_NUMBER_OF_HANDS];
	int hasInsurance;
	int numberOfSplits;
	struct hand hands[MAX_NUMBER_OF_HANDS];
};

FILE * openFile(char *fileName, char *mode);
void setupDeck(FILE *fptr);
void shuffleDeck(FILE *fptr);
void swapCard(int i, int n, FILE *fptr);
void resetUser(struct user *aPlayer);
void resetHand(struct hand *aHand);
int continueBetting(struct user *aPlayer);
void dealCard(struct hand *aHand, FILE *fptr);
void printTable(struct hand dealersHand, struct user player, int activeHand);
void printHand(struct hand aHand);
void printValueOfHand(struct hand aHand);
int canInsure(struct hand dealersHand, struct user player, int n);
int offerInsurance(struct user *aPlayer);
void getHandStatus(struct user *aPlayer, int n);
int hasGotBlackjack(struct hand aHand);
char getChoice(struct user aPlayer, int n);
int canDoubleDown(struct user aPlayer, int n);
int canBeSplit(struct user aPlayer, int n);
void splitTheHand(struct user *player, int n, int i);
int isBust(struct hand aHand);
char getResult(struct hand dealersHand, struct user aPlayer, int dealerBlackjack, int n);
void changeBalance(int result, struct user *aPlayer, int dealerHasBlackjack, int n);
void printResult(char result);
void closeFile(FILE *fptr);
