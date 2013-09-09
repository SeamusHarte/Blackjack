#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cctype>
#include <stdlib.h>
#include <Windows.h>
#include "blackjack.h"


FILE * openFile(char *fileName, char *mode)
{
	FILE *fptr = fopen(fileName, mode);

	if (fptr == NULL)
		printf("Error opening file ! \n");

	return fptr;
}

/* setupDeck writes 52 cards to a binary file */
void setupDeck(FILE *fptr)
{
	char suits[4] = {0,1,2,3};														// array of 4 chars
	char faces[13][3] = {"A","2","3","4","5","6","7","8","9","10","J","Q","K"};		// array of 13 strings
	int values[13] = {1,2,3,4,5,6,7,8,9,10,10,10,10};								// array of 13 ints
	struct card aCard;																// creates a temp card to store the face, suit and value 
	
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 13; j++)
		{
			aCard.suit = suits[i]+3;
			aCard.value = values[j];
			strcpy(aCard.face, faces[j]);
			fwrite(&aCard, sizeof(struct card), 1, fptr);							// writes the temp card to the file
		}
}

/* shuffleDeck shuffles the order of the cards stored in the file
 * n is set to the last card in the deck
 * a random card between card0 and card50 inclusive is swapped with card51
 * card 51 is now shuffled 
 * n is decremented so that card50 can be swapped with a random card between 0 and card49
 * card 50 is now shuffled
 *this continues until the last 2 cards remaining are swapped
 */
void shuffleDeck(FILE *fptr)
{
	int i, j, n;
	
	for(j = 0; j < 7; j++)																// shuffling 7 times to insure the deck is properly shuffled
	{
		srand(time(NULL));
		for(n = 51 ; n > 0; n--) 
		{
			i = (rand() % n);															// i is a random number between 0 and n 
			swapCard(i, n, fptr);                                                       // swap card[i] with card[n]
		}
	}

	printf("\n\nShuffling Deck\n\n");
}	

/* SwapCArd swaps two cards in the Deck */
void swapCard(int i, int n, FILE *fptr)
{
	struct card card_i, card_n;														// 2 temp cards to store the cards read in from the deck
	rewind(fptr);
	fseek(fptr, i * sizeof(struct card), SEEK_SET);										// find card i
	fread(&card_i, sizeof(struct card), 1, fptr);										// store it
	rewind(fptr);	
	fseek(fptr, n * sizeof(struct card), SEEK_SET);										// find card n
	fread(&card_n, sizeof(struct card), 1, fptr);										// store it
	rewind(fptr);
	fseek(fptr, n * sizeof(struct card), SEEK_SET);										// find where card n was
	fwrite(&card_i, sizeof(struct card), 1, fptr);										// write card i there
	rewind(fptr);
	fseek(fptr, i * sizeof(struct card), SEEK_SET);										// find where card i was
	fwrite(&card_n, sizeof(struct card), 1, fptr);										// write card n there
}

// reseta all the user variables except balance
void resetUser(struct user *aPlayer)
{
	aPlayer->hasInsurance = aPlayer->numberOfSplits = 0;
	for(int n = 0; n < MAX_NUMBER_OF_HANDS; n++)
	{
		aPlayer->bet[n] = 0;
		resetHand(&aPlayer->hands[n]);
	}
}

//	resetHand resets all the elements contained in hand
void resetHand(struct hand *aHand)
{
	aHand->hasAnAce = aHand->numberOfCards = aHand->total = aHand->altTotal = 0;
	aHand->status = ' ';
}

int continueBetting(struct user *aPlayer)
{
	int response, firstTimeInLoop;
	char choice;

	if(aPlayer->balance > 0) 
	{
		printf("\n\nHouse Rules:\n");
		printf("Dealer stands on soft 17\n");
		printf("Blackjack pays 3 to 2\n");
		printf("Double on any two cards\n");
		printf("Split on any pair\n");
		printf("Insurance pays 2 to 1\n");
		
		firstTimeInLoop = 1;
		do
		{
			if(!firstTimeInLoop)
				printf("\nInvalid input\n");

			printf("\nChoose to bet between 1 and %d, or 0 to quit\n", aPlayer->balance);
			printf("Your bet : ");
			scanf("%d", &aPlayer->bet[0]);
			fflush(stdin);
			firstTimeInLoop = 0;
		}while((aPlayer->bet[0] < 0) || (aPlayer->bet[0] > aPlayer->balance));					// checks for valid bet
		
		aPlayer->balance -= aPlayer->bet[0];
		if (aPlayer->bet[0] > 0)
			response = 1;
		else
			response = 0;
	}
	else
	{
		firstTimeInLoop = 1;
		do
		{
			if(firstTimeInLoop)
			{
				printf("\nGame Over\n\n");
				printf("New game y/n? ");
			}
			else
				printf("\nInvalid\n Re-enter Input\n : ");	// re-enter input if its not valid

			scanf("%c", &choice);
			choice = tolower(choice);
			fflush(stdin);
			firstTimeInLoop = 0;

		}while((choice != 'y') && (choice != 'n'));     // checks for valid input
	
		if (choice == 'y')
		{
			response = 1;
			aPlayer->bet[0] = 0;
			aPlayer->balance = 1000;
			response = continueBetting(aPlayer);
		}
		else
			response = 0;
	}
	return response;

}

/*	dealCard reads in a card from the deck to the next free card in the Hand.
 *	Then the other variables in the hand are updated to reflect the addition of the new card*/
void dealCard(struct hand *aHand, FILE *fptr)
{
	int i = aHand->numberOfCards;										// sets i to the next free card in the Hand
	
	fread(&aHand->cards[i], sizeof(struct card), 1, fptr);				// read in the next card from the deck to cards[i]

	aHand->numberOfCards++;												// increments the number of Cards in the Hand
	aHand->total += aHand->cards[i].value;								// add the value of the new cards to the totals
	aHand->altTotal += aHand->cards[i].value;						

	if((aHand->cards[i].value == 1) && (aHand->hasAnAce == 0))			// if new card is an ace and there isnt already an ace in the hand					
	{
		aHand->hasAnAce = 1;											// if the card is an ace then hasAnAce is set to 1
		aHand->altTotal += 10;											// adds 10 to the altTotal as the value an ace in altotal is 11
	}
	
}

/* prints the current table state
 * displays the dealer hand , the player hand(s), the user's balance and the user's bet
 */
void printTable(struct hand dealersHand, struct user player, int activeHand)
{
	int i;
	printf("\n\n\n\n--------------------------------------------------------------------------------\n\n\n\n");
	printf("Dealer : ");
	printHand(dealersHand);
	printValueOfHand(dealersHand);

	printf("\n\nPlayer : ");
	for(i = 0; i <= player.numberOfSplits; i++)
	{
		if(i > 0)
			printf("   ");											// creates a gap between the player's hands

		if(i == activeHand)
			printf("> ");											// indicates which of the players's hand is active

		printHand(player.hands[i]);
		printValueOfHand(player.hands[i]);
	}
	
	printf("\n\nBalance : %d\t\tBet : ", player.balance);

	for(i = 0; i <= player.numberOfSplits; i++)
	{
		if(i > 0)													// seperates the player's bets 
			printf("/");

		printf("%d",player.bet[i]);
	}
	Sleep(1500);                                                    // creates a delay in the program to make the action feel more natural
}

//	printHand prints out the face and suit of all the cards in the hand
void printHand(struct hand aHand)
{
	for(int i = 0; i < aHand.numberOfCards; i++)
		printf("%s%c ", aHand.cards[i].face, aHand.cards[i].suit);
}

/* prints the total value of the cards in the hand
 * determines whether to use total, altTotal or both
 */
void printValueOfHand(struct hand aHand)
{	
	if((aHand.hasAnAce) && (aHand.altTotal <= 21) && ((aHand.status == 'b') || (aHand.status == 's') || (aHand.status == 'j')))// if the hand has finished receiving cards and it has an ace
		printf("(%d)", aHand.altTotal);
	else if((aHand.hasAnAce) && (aHand.altTotal <= 21))															// if it has an ace and still could get more cards and the 11 vale
		printf("(%d/%d)", aHand.total, aHand.altTotal);
	else																										// default
		printf("(%d)", aHand.total);
}

/* can Insure if the dealer's hand contains 1 Ace only and the user can afford to insure
 * and the user has only 2 cards (so that it prompts the use only once)*/
int canInsure(struct hand dealersHand, struct user player, int n)
{
	if((dealersHand.numberOfCards == 1) && (player.hands[n].numberOfCards == 2) && (dealersHand.hasAnAce) && (player.balance >= player.bet[n]))
		return 1;
	else
		return 0;
}

/* prompts user to accept or decline insurance against dealer blackjack
 * returns 1 if user accepts, returns 0 if user declines
 */
int offerInsurance(struct user *aPlayer)
{
	char choice;
	int firstTimeInTheLoop = 1;

	do
	{
		if (firstTimeInTheLoop)
			printf("\n\nInsure against dealer Blackjack? (y/n) : ");	
		else
			printf("\nInvalid\nRe-enter Input\n : ");					// re-enter input if its not valid
		
		scanf("%c", &choice);
		choice = tolower(choice);
		fflush(stdin);
		firstTimeInTheLoop = 0;

	}while((choice != 'y') && (choice != 'n'));							// checks for valid input

	if(choice == 'y')
	{
		aPlayer->balance -= aPlayer->bet[0];							// deduct the cost of insurance (same as user bet) from the balance
		return 1;
	}
	else
		return 0;
}

// determines the status of the hand
void getHandStatus(struct user *aPlayer, int n)
{
	if(isBust(aPlayer->hands[n]))
		aPlayer->hands[n].status = 'b';
	else if (hasGotBlackjack(aPlayer->hands[n]))
	{
		if(aPlayer->numberOfSplits == 0)						// split hands cannot become blackjack
			aPlayer->hands[n].status = 'j';
		else
			aPlayer->hands[n].status = 's';
	}
	else if((aPlayer->hands[n].total == 21) || (aPlayer->hands[n].altTotal == 21))      // player auto-stands once they reach 21
		aPlayer->hands[n].status = 's';
	else if(aPlayer->hands[n].status == 'd')											// if player has choosen to double their status now stand
		aPlayer->hands[n].status = 's';
	else
		;
}

/*  returns 1 if hand is blackjack (Ace and a 10 value card)*/ 
int hasGotBlackjack(struct hand aHand)
{
	if((aHand.altTotal == 21) && (aHand.numberOfCards == 2))
		return 1;
	else
		return 0;
}

/* getChoice outputs prompt to user and returns the user choice */
char getChoice(struct user aPlayer, int n)
{
	int ableToDouble = 0;
	int ableToSplit = 0;
	int firstTimeInTheLoop = 1;
	char choice;

	printf("\n\nh to Hit\n");
	printf("s to Stand\n");
	if(ableToDouble = canDoubleDown(aPlayer, n))
		printf("d to Double Down\n");														// only prints in the event the user can double down
	if(ableToSplit = canBeSplit(aPlayer, n))
		printf("p to Split hand\n");														// oply prints in the event the user can split
	printf("\n : ");

	do
	{
		if(!firstTimeInTheLoop)																
			printf("\nInvalid\n Re-Enter Input\n : ");										// re-enter input if its not valid
		scanf("%c", &choice);
		choice = tolower(choice);															// convert choice to lowercase if the user enters capitals
		fflush(stdin);
		firstTimeInTheLoop = 0;
	}while(!((choice == 'h') || (choice == 's') || ((choice == 'd') && (ableToDouble)) || ((choice == 'p') && (ableToSplit))));     // checks for valid input

	return choice;
}

/* player can Double Down on a hand if the hand contains 2 cards only and the user can afford to double their bet */
int canDoubleDown(struct user aPlayer, int n)
{
	if((aPlayer.hands[n].numberOfCards == 2) && (aPlayer.balance >= aPlayer.bet[n]))
		return 1;
	else
		return 0;
}


/* hand can Be Split if the hand contains 2 cards of the same face only and the user can afford to split */
int canBeSplit(struct user aPlayer, int n)
{
	if((aPlayer.hands[n].numberOfCards == 2) && (aPlayer.hands[n].cards[0].value == aPlayer.hands[n].cards[1].value) && (aPlayer.balance >= aPlayer.bet[n]))
		return 1;
	else
		return 0;
}

// splits hand n to hand n+1 and makes space for n+1 by moving other hands
void splitTheHand(struct user *player, int n, int i)
{
	/* copies all the hands to the right of the hand being split one space to the right to make space for the new hand */
	while(i > n+1)										
	{
		player->hands[i] = player->hands[i-1];							
		player->bet[i] = player->bet[i-1];
		i--;
	}

	player->hands[n+1].numberOfCards = player->hands[n].numberOfCards = 1;							// number of cards in each hand is 1
	player->hands[n+1].cards[0] = player->hands[n].cards[1];										// moving the 2nd card in hand n to the 1st place in hand (n+1) 
	player->hands[n+1].total = player->hands[n].total = (player->hands[n].total / 2);				// total value of each hand is equal to half the old value
	player->hands[n+1].altTotal = player->hands[n].altTotal = (player->hands[n].altTotal / 2);		// altTotal value of each hand is equal to half the old value
	player->hands[n+1].hasAnAce = player->hands[n].hasAnAce;										 
	player->bet[n+1] = player->bet[n];
	player->balance -= player->bet[n+1];					
}

/*	isBust checks if both total values of the hand are greater than 21  */
int isBust(struct hand aHand)
{
	if((aHand.total > 21) && (aHand.altTotal > 21))
		return 1;
	else
		return 0;
}

/* compares the two hands and returns the result */
char getResult(struct hand dealersHand, struct user aPlayer, int dealerBlackjack, int n)
{
	int dealerTotal, playerTotal;
	dealerTotal = playerTotal = 0;
	
	if(dealersHand.altTotal <= 21)						// determining whether to use total or altTotal for dealer
		dealerTotal = dealersHand.altTotal;
	else
		dealerTotal = dealersHand.total;

	if(aPlayer.hands[n].altTotal <= 21)					// determining whether to use total or altTotal for player
		playerTotal = aPlayer.hands[n].altTotal;
	else
		playerTotal = aPlayer.hands[n].total;
	
	if(aPlayer.hands[n].status == 'b')										// lose if player bust
		return 'l';
	else if((aPlayer.hands[n].status == 'j') && (!dealerBlackjack))			// blackjack if player has blackjack and dealer hasnt
		return 'j';
	else if((aPlayer.hands[n].status != 'j') && (dealerBlackjack))			// lose if dealer has blackjack and player hasnt
		return 'l';
	else if(dealerTotal > 21)												// win if dealer is bust
		return 'w';
	else if(dealerTotal > playerTotal)										// lose if dealer is closer to 21 than player
		return 'l';
	else if(dealerTotal < playerTotal)										// win if player is closer to 21 than dealer
		return 'w';
	else																	// otherwise it must be a draw
		return 'd';
}

/* updates user balance */
void changeBalance(int result, struct user *aPlayer, int dealerHasBlackjack, int n)
{
	if(result == 'w')
		aPlayer->balance += (2 * aPlayer->bet[n]);					
	else if(result == 'j')
		aPlayer->balance += (2.5 * aPlayer->bet[n]);
	else if(result == 'd')
		aPlayer->balance += aPlayer->bet[n];
	else
		;

	if((dealerHasBlackjack) && (aPlayer->hasInsurance))
		aPlayer->balance += (2 * aPlayer->bet[n]);
}

/* prints win, blackjack, push or lose depend on result */
void printResult(char result)
{
	if(result == 'w')
		printf("Win\t");
	else if(result == 'j')
		printf("Blackjack\t");
	else if(result == 'd')
		printf("Push\t");
	else
		printf("Lose\t");
	Sleep(200);
}

void closeFile(FILE *fptr)
{
	fclose(fptr);
}
