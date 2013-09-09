// Blackjack Project.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cctype>
#include <stdlib.h>
#include <Windows.h>
#include "blackjack.h"

void main()
{
	struct hand dealerHand;
	struct user player;
	FILE *fptr;
	char result = ' ';
	int cardsDealt, dealerBlackjack, continuePlaying, i, playerBust;
	cardsDealt = dealerBlackjack = continuePlaying = 0;
	player.balance = 1000;

	fptr = openFile("\\deck.dat","wb+");							// opens the deck 

	setupDeck(fptr);												// inserts cards into deck
	shuffleDeck(fptr);												// shuffles deck
	rewind(fptr);  

	resetUser(&player);												// resets players hands
	continuePlaying = continueBetting(&player);						// gets user bet or user quits

	while(continuePlaying)											
	{  
		resetHand(&dealerHand);												
		i = 0;														// resets the index of the current hand the player is using

		dealCard(&dealerHand, fptr);								
		dealCard(&player.hands[i], fptr);

		do
		{
			do
			{
				dealCard(&player.hands[i], fptr);
				printTable(dealerHand, player, i);

				if(canInsure(dealerHand, player, i))						// offers player insurance against dealer blackjack
					player.hasInsurance = offerInsurance(&player);			

				getHandStatus(&player, i);									// updates the status of the players hand since the new card was dealt
			
				if((player.hands[i].status != 'b') && (player.hands[i].status != 'j') && (player.hands[i].status != 's'))
					player.hands[i].status = getChoice(player, i);						// getting choice since the player's hand is still active

				if(player.hands[i].status == 'p')							// if the player wants to split his hand
				{
					player.numberOfSplits++;
					splitTheHand(&player, i, player.numberOfSplits);  
				}
			
				if(player.hands[i].status == 'd')							// if the player has doubled down
				{
					player.balance -= player.bet[i];						
					player.bet[i] += player.bet[i];
				}
			}while((player.hands[i].status == 'h') || (player.hands[i].status == 'd') || (player.hands[i].status == 'p')); // while hand is still active
			i++;	// moves the player on to their next hand
		}while(i <= player.numberOfSplits);			// stays in the loop the player is finishing with their last hand

		playerBust = 1;
		for(i = 0; i <= player.numberOfSplits; i++)
			playerBust *= isBust(player.hands[i]);				// if any of the hands are not bust (0) then the playerBust will be (0) 			

		/*  deals cards to the dealer until he is over 16 or bust. Doesnt deal if the player is already bust unless he has insurance   */
		while((dealerHand.total < 17) && ((dealerHand.altTotal < 17) || (dealerHand.altTotal > 21)) && ((!playerBust) || player.hasInsurance))
		{
			dealCard(&dealerHand, fptr);
			printTable(dealerHand, player, i);
		}

		dealerBlackjack = hasGotBlackjack(dealerHand);

		printf("\n\n");

		for(i = 0; i <= player.numberOfSplits; i++)
		{
			result = getResult(dealerHand, player, dealerBlackjack, i);						// gets the result of each hand
			changeBalance(result, &player, dealerBlackjack, i);								// updates balance 
			printResult(result);															// prints result
			cardsDealt+= player.hands[i].numberOfCards;										// adds the cards to the number of cards dealt
		}
		printf("\n\n\n--------------------------------------------------------------------------------");
		Sleep(800);
		cardsDealt+= dealerHand.numberOfCards;												// adds the dealer's cards to the number of cards dealt

		printf("\n\nBalance: %d", player.balance);											// print the plater's new balance

		if (cardsDealt > 26)																// shuffles the deck once if half the deck is dealt
		{
			shuffleDeck(fptr);
			rewind(fptr);  
			cardsDealt = 0;																	// resets the number of cards dealt					
		}
		resetUser(&player);																	// resets player hands
		continuePlaying = continueBetting(&player);											// gets user bet or user quits or starts new game
	}
	closeFile(fptr);
}
