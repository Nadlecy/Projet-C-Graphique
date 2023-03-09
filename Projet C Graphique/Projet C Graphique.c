#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

typedef int bool;
#define true 1
#define false 0
bool isRunning;


struct box
{
	char * content;
	int isBomb;
	int nearbyBombs;
	char* color;
};

struct gameSettings
{
	int width;
	int height;
	int bombTotal;
	int flags;
	int unopenedBoxes;
	int isGameDone;
	char* indexTexture[13];
};

//SETUP FUNCTIONS
//sets a pre-made 
void Texturize(SDL_Renderer* renderer, struct gameSettings* rules, char* path, int index)
{
	SDL_Surface* image;
	image = IMG_Load(path);
	SDL_Texture* oneImage = SDL_CreateTextureFromSurface(renderer, image);
	rules->indexTexture[index] = oneImage;
}

//fills an empty grid with empty boxes, with the same dimensions as the player input
void initialize(struct box* tab, struct gameSettings* rules, SDL_Renderer* renderer)
{
	SDL_Surface* image;
	char* newLink[14];
	for (int j = 0; j <= 12; j++)
	{
		switch (j)
		{
		case 0:
			Texturize(renderer, rules, "darkGrass.jpg", j);
			break;
		case 9:
			Texturize(renderer, rules, "flag.jpg", j);
			break;
		case 10:
			Texturize(renderer, rules, "question.jpg", j);
			break;
		case 11:
			Texturize(renderer, rules, "bomb.jpg", j);
			break;
		case 12:
			Texturize(renderer, rules, "grass.jpg", j);
			break;
		default:
			sprintf_s(newLink, 14, "nearBomb%d.jpg", j);

			Texturize(renderer, rules, newLink, j);
			printf("%s", rules->indexTexture[j]);
			break;
		}
	}
	struct box element = { rules->indexTexture[0], 0, 0};
	for (int i = 0; i < rules->height; i++)
	{
		for (int u = 0; u < rules->width; u++)
		{
			tab[i * rules->height + u] = element;
		}
	}
}

//puts bombs on the field (the number of which are input by the player)
void bombPlacing(struct box* tab, struct gameSettings* rules)
{
	//setting the minimum and maximum index values of tab on which bombs will be put
	int lower = 0;
	int upper = rules->width * rules->height - 1;
	//setting time variable, necessary for different generations
	time_t t1;
	srand((unsigned)time(&t1));

	//making a full array
	int* randTab = (int*)malloc(sizeof(int) * rules->width * rules->height);
	for (int i = 0; i < rules->width * rules->height; i++)
	{
		randTab[i] = i;
	}

	//for each bomb to put down
	for (int i = 0; i < rules->bombTotal; i++)
	{
		int length = (rules->width * rules->height) - i;

		//getting a random number
		int indice = rand() % length;
		tab[randTab[indice]].isBomb = 1;
		//editing array to remove the currently added value
		memcpy(randTab + indice, randTab + indice + 1, sizeof(int) * (length - indice - 1));
	}
}

//updates every box on the grid to tell how many bombs are nearby
void bombRadar(struct box* tab, struct gameSettings* rules)
{
	for (int i = 0; i < rules->width * rules->height; i++)
	{
		if (tab[i].isBomb != 1)
		{
			//horizontals 
			//(looking left)
			if (i > 0 && i % rules->width != 0 && tab[i - 1].isBomb)
			{
				tab[i].nearbyBombs++;
			}

			//(looking right)
			if (i % rules->width != rules->width - 1 && tab[i + 1].isBomb)
			{
				tab[i].nearbyBombs++;
			}
			//verticals
			//(looking up)
			if (i > rules->width - 1 && tab[i - rules->width].isBomb)
			{
				tab[i].nearbyBombs++;
			}
			//(looking down)
			if (i < (rules->height * rules->width - rules->width) && tab[i + rules->width].isBomb)
			{
				tab[i].nearbyBombs++;
			}
			//diagonals 
			//(upper left)
			if (i > rules->width - 1 && i % rules->width > 0 && tab[i - 1 - rules->width].isBomb)
			{
				tab[i].nearbyBombs++;
			}
			//(upper right)
			if (i > rules->width - 1 && i % rules->width != rules->width - 1 && tab[i + 1 - rules->width].isBomb)
			{
				tab[i].nearbyBombs++;
			}
			//(lower left)
			if (i < (rules->height * rules->width - rules->width) && i % rules->width > 0 && tab[i + rules->width - 1].isBomb)
			{
				tab[i].nearbyBombs++;
			}
			//(lower right)
			if (i < (rules->height * rules->width - rules->width) && i % rules->width != rules->width - 1 && tab[i + rules->width + 1].isBomb)
			{
				tab[i].nearbyBombs++;
			}
		}
		else
		{
			tab[i].nearbyBombs = 9;
		}
		//setting boxes' colors depending on the number of nearby bombs
		switch (tab[i].nearbyBombs)
		{
		case 1:
			tab[i].color = "\x1b[34m"; //blue value
			break;
		case 2:
			tab[i].color = "\x1b[32m"; //green value
			break;
		case 3:
			tab[i].color = "\x1b[31m"; //red value
			break;
		case 4:
			tab[i].color = "\x1b[35m"; //magenta/purple value
			break;
		case 5:
			tab[i].color = "\x1b[36m"; //cyan value
			break;
		case 6:
			tab[i].color = "\x1b[36m"; //cyan value
			break;
		case 7:
			tab[i].color = "\x1b[36m"; //cyan value
			break;
		case 8:
			tab[i].color = "\x1b[33m"; //yellow value
			break;
		default:
			tab[i].color = "\x1b[0m"; //white value
		}
	}
}

//ACTION FUNCTIONS
//checks a box's contents and reveals it on the display
void dig(struct box* tab, int X, int Y, struct gameSettings* rules)
{
	// ckeck if there is a flag already placed or not on the tile
	if (X > 0 && X < rules->height + 1 && Y>0 && Y < rules->width + 1 && tab[X - 1 + rules->width * (Y - 1)].content != rules->indexTexture[9])
	{
		//if there is a bomb on the tile
		if (tab[X - 1 + rules->width * (Y - 1)].isBomb)
		{
			//you lose, so the game is done
			rules->isGameDone++;

		}
		//if the tile has not yet been revealed
		else if (tab[X - 1 + rules->width * (Y - 1)].content == rules->indexTexture[0])
		{
			//if there is one (or more) bomb(s) near the tile 
			if ((tab[X - 1 + rules->width * (Y - 1)].nearbyBombs))
			{
				//transforms the nearbyBombs pointer (int) into a string 
				tab[X - 1 + rules->width * (Y - 1)].content = rules->indexTexture[tab[X - 1 + rules->width * (Y - 1)].nearbyBombs];
				//removes a number from the unopened boxes, because we just opened one
				rules->unopenedBoxes--;
			}
			//if there are no bombs around the current box, check every neighboring box 
			else
			{
				rules->unopenedBoxes--;
				tab[X - 1 + rules->width * (Y - 1)].content = rules->indexTexture[12];
				dig(tab, X - 1, Y, rules); //left
				dig(tab, X + 1, Y, rules); //right
				dig(tab, X - 1, Y + 1, rules); //down left
				dig(tab, X + 1, Y + 1, rules); //down right
				dig(tab, X - 1, Y - 1, rules); //up left
				dig(tab, X + 1, Y - 1, rules); //up left
				dig(tab, X, Y - 1, rules); //up
				dig(tab, X, Y + 1, rules); //down

			}
		}
	}
}

//puts or removes flags and question marks on boxes
void flag(struct box* tab, int X, int Y, struct gameSettings* rules)
{
	if (X > 0 && X < rules->width + 1 && Y>0 && Y < rules->height + 1)
	{
		//if the box is empty
		if (tab[X - 1 + rules->width * (Y - 1)].content == rules->indexTexture[0])
		{
			//add a flag
			tab[X - 1 + rules->width * (Y - 1)].content = rules->indexTexture[9];
			//decrease how many flags left the player has to put
			rules->flags--;
		}
		//if the box has a flag
		else if (tab[X - 1 + rules->width * (Y - 1)].content == rules->indexTexture[9])
		{
			//replace the flag by a question mark
			tab[X - 1 + rules->width * (Y - 1)].content = rules->indexTexture[10];
			//increase how many flags left the player has to put 
			rules->flags++;
		}
		//if the box has a question mark
		else if (tab[X - 1 + rules->width * (Y - 1)].content == rules->indexTexture[10])
		{
			//empty the box
			tab[X - 1 + rules->width * (Y - 1)].content = rules->indexTexture[0];
		}
	}
}

//VISUAL FUNCTION
//gives a visual representation of the minefield
void displayGrid(struct box* tab, struct gameSettings* rules,SDL_Renderer* renderer, int endDisplay)
{
	SDL_SetRenderDrawColor(renderer, 0, 128, 128, 255);
	SDL_RenderClear(renderer);
	//Makes a basic play area/info bar
	SDL_Rect position;
	SDL_Surface* image;
	for (int i = 0; i < rules->height * rules->width; i++)
	{
		//if the game has ended and the box contains a mine, reveal the mine
		if (tab[i].isBomb && endDisplay)
		{
			tab[i].content = rules->indexTexture[11];
		}

		position.x = (i % rules->width) * 32 + 16 ;//the horizontal position
		position.y = ((i - (i % rules->width)) / rules->height) * 32 + 16; //the vertical position
		SDL_QueryTexture(tab[i].content, NULL, NULL, &position.w, &position.h);
		SDL_RenderCopy(renderer, tab[i].content, NULL, &position);
	}

	if (endDisplay == 0)
	{
		SDL_Rect flagInfo;
		flagInfo.x = 16;
		flagInfo.y = rules->height * 32 + 32;;
		SDL_QueryTexture(rules->indexTexture[9], NULL, NULL, &flagInfo.w, &flagInfo.h);
		SDL_RenderCopy(renderer, rules->indexTexture[9], NULL, &flagInfo);
		
		//creating rectangle for the texture
		SDL_Rect flagTextInfo;
		flagTextInfo.x = flagInfo.w + 16;
		flagTextInfo.y = flagInfo.y; 
		
		//creating the text to implent in the window
		char newDisplay[25];
		//transforms the nearbyBombs pointer (int) into a string 
		sprintf_s(newDisplay, 25, "Flags left to place : %d", rules->flags);
		char* text = newDisplay;
		TTF_Init();
		TTF_Font* font = NULL;
		font = TTF_OpenFont("font/comic-sans-ms_fr.ttf", 12);
	
		if (font != 0) {
			SDL_Color black = { 0, 0, 0 }; 
			SDL_Surface* textSurf = TTF_RenderText_Blended(font, text, black);


			if (textSurf == NULL) {
				printf("Text is NULL");
				TTF_CloseFont(font);
				TTF_Quit();
			}
			SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurf);
			if (texture == NULL) {
				printf("Texture is NULL");
				TTF_CloseFont(font);
				TTF_Quit();
			}
			SDL_QueryTexture(texture, NULL, NULL, &flagTextInfo.w, &flagTextInfo.h);
			SDL_RenderCopy(renderer, texture, NULL, &flagTextInfo);
		
			printf("\n");
		}
		else 
		{
			printf("Font error");
		}
	}
	SDL_RenderPresent(renderer);
}

//FUNCTIONS FOR OPTIMIZATION
//the gameplay loop that will be repeated as long as the game is not over
void gamePlay(struct box* tab, struct gameSettings* rules, SDL_Renderer* renderer, SDL_Window* window)
{
	SDL_Event event;

	//makes a teal background
	SDL_SetRenderDrawColor(renderer, 0, 128, 128, 255);
	SDL_RenderClear(renderer);

	//checks if any game-ending condition has been met (losing/winning) every turn
	while (rules->isGameDone == 0)
	{
		//Has the player opened every box except for mines ?
		if (rules->unopenedBoxes == 0)
		{
			rules->isGameDone = 1;
		}
		else
		{
			system("cls");
			//Show the current state of the minefield
			displayGrid(tab, rules, renderer, 0);
			//Take in the coordinates of the next box the player will act upon
			SDL_WaitEvent(&event);
			switch (event.type) 
			{
			case SDL_QUIT:
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				SDL_Quit();
				rules->isGameDone = 1;
				break;

			case SDL_MOUSEBUTTONDOWN:

				if (event.button.button == SDL_BUTTON_LEFT) {
					//printf("Au moins un clic gauche\nX coord : %d\nY coord : %d\n",event.button.x,event.button.y);
					int X = (event.button.x - 16)/32 + 1;
					int Y = (event.button.y - 16)/32 + 1;
					printf("\nLEFT !\nX final coord : %d\nY final coord : %d",X,Y);
					dig(tab, X, Y, rules);
				}
				if (event.button.button == SDL_BUTTON_RIGHT) {
					//printf("Au moins un clic droit\nX coord : %d\nY coord : %d\n",event.button.x,event.button.y);
					int X = (event.button.x - 16) / 32 + 1;
					int Y = (event.button.y - 16) / 32 + 1;
					printf("\n RIGHT !\nX final coord : %d\nY final coord : %d", X, Y);
					flag(tab, X, Y, rules);
				}
				break;
			}
		}
	}

}

//"AFTER THE GAME" FUNCTIONS
//shows a message after the game has ended, depending on whether the win conditions have been met or not
void gameEnd(struct box* tab, struct gameSettings* rules)
{
	if (rules->unopenedBoxes == 0)
	{
		//winning condition : all boxes have been opened
		printf("\n Congratulations, you win !");
	}
	else
	{
		//if you landed on a bomb before opening every box, well you lose 
		printf("\n You lose !");
	}
}

//asks the player if they want to do another game
int playAgain(struct gameSettings* rules, SDL_Renderer* renderer, SDL_Window* window)
{
	SDL_Event event;
	int awaitingChoice = 1;

	SDL_Rect winLoseText;
	winLoseText.x = 16;
	winLoseText.y = rules->height * 32 + 16;
	TTF_Init();
	char* text;
	
	TTF_Font* font = NULL;
	font = TTF_OpenFont("font/comic-sans-ms_fr.ttf", 16);

	if (font != 0) {
		if (rules->unopenedBoxes == 0)
		{
			//winning condition : all boxes have been opened
			text = "                         You Win !                      Left Click to Play Again                         Right Click to leave the game";
		}
		else
		{
			//if you landed on a bomb before opening every box, well you lose 
			text = "                         You Lose !                     Left Click to Play Again                         Right Click to leave the game";
		}
		SDL_Color black = { 0, 0, 0 };
		SDL_Surface* textSurf = TTF_RenderText_Blended_Wrapped(font, text, black, 320);

		if (textSurf == NULL) {
			printf("Text is NULL");
			TTF_CloseFont(font);
			TTF_Quit();
		}
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurf);
		if (texture == NULL) {
			printf("Texture is NULL");
			TTF_CloseFont(font);
			TTF_Quit();
		}
		SDL_QueryTexture(texture, NULL, NULL, &winLoseText.w, &winLoseText.h);
		SDL_RenderCopy(renderer, texture, NULL, &winLoseText);
	}
	else
	{
		printf("Font error");
	}
	SDL_RenderPresent(renderer);

	while (awaitingChoice)
	{
		//Take in the coordinates of the player's click
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case SDL_QUIT:
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
			break;

		case SDL_MOUSEBUTTONDOWN:

			if (event.button.button == SDL_BUTTON_LEFT) {
				return 1;
			}
			if (event.button.button == SDL_BUTTON_RIGHT) {
				return 0;
			}
			break;
		}
	}
}

//VISUAL FUNCTIONS
//main operating function, runs the whole program
int main() {

	int playing = 1;
	while (playing)
	{
		int * gameValues[3];
		for (int i = 0; i < 3; i++)
		{
			gameValues[i] = 10;
		}
		//setting the grid size
		int xSize = gameValues[0];
		int ySize = gameValues[1];
		//setting the number of bombs in the grid
		int bombs = gameValues[2];

		//creating the rules based on the previous inputs
		struct gameSettings rules = { xSize, ySize, bombs, bombs, xSize * ySize - bombs, 0 };
		//beginning of game
		//memory allocation for the grid called tab
		struct box* tab = (struct box*)malloc(sizeof(struct box) * rules.width * rules.height);
		//creates a window and a renderer for the game
		SDL_Window* gameWin = SDL_CreateWindow("MineSweeper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (rules.width + 1) * 32, rules.height * 32 + 96, 0);
		SDL_Renderer* gameRender = SDL_CreateRenderer(gameWin, -1, 0);

		initialize(tab, &rules, gameRender);
		bombPlacing(tab, &rules);
		bombRadar(tab, &rules);


		//gameplay loop of interacting with the grid, also makes a playable window for the user
		gamePlay(tab, &rules, gameRender, gameWin);

		//when game ends, tell if player won or lose
		system("cls");
		gameEnd(tab, &rules);
		
		//show the grid with mines revealed
		displayGrid(tab, &rules, gameRender, 1);

		//freeing the allocated memory of the grid
		free(tab);
		//ask player if they want to play again
		playing = playAgain(&rules, gameRender, gameWin);
		
		SDL_DestroyRenderer(gameRender);
		SDL_DestroyWindow(gameWin);
	}
	return 0;
}

//handles any events that SDL noticed.
void handleEvents() {
	//the only event we'll check is the  SDL_QUIT event.
	SDL_Event event;
	SDL_PollEvent(&event);


	switch (event.type) {
	case SDL_QUIT:
		isRunning = false;
		break;
	default:
		break;
	}
}