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
};

//SETUP FUNCTIONS
//fills an empty grid with empty boxes, with the same dimensions as the player input
void initialize(struct box* tab, struct gameSettings* rules)
{
	struct box element = { "darkGrass.jpg", 0, 0 };
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
	if (X > 0 && X < rules->height + 1 && Y>0 && Y < rules->width + 1 && tab[X - 1 + rules->width * (Y - 1)].content != "flag.jpg")
	{
		//if there is a bomb on the tile
		if (tab[X - 1 + rules->width * (Y - 1)].isBomb)
		{
			//you lose, so the game is done
			rules->isGameDone++;

		}
		//if the tile has not yet been revealed
		else if (tab[X - 1 + rules->width * (Y - 1)].content == "darkGrass.jpg")
		{
			//if there is one (or more) bomb(s) near the tile 
			if ((tab[X - 1 + rules->width * (Y - 1)].nearbyBombs))
			{
				char newDisplay[14];
				//transforms the nearbyBombs pointer (int) into a string 
				sprintf_s(newDisplay, 14, "nearBomb%d.jpg", tab[X - 1 + rules->width * (Y - 1)].nearbyBombs);
				tab[X - 1 + rules->width * (Y - 1)].content = newDisplay;
				//removes a number from the unopened boxes, because we just opened one
				rules->unopenedBoxes--;
			}
			//if there are no bombs around the current box, check every neighboring box 
			else
			{
				rules->unopenedBoxes--;
				tab[X - 1 + rules->width * (Y - 1)].content = "grass.jpg";
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
	//tell the player if the area they're trying to dig is flagged
	else if (X > 0 && X < rules->height + 1 && Y>0 && Y < rules->width + 1 && tab[X - 1 + rules->width * (Y - 1)].content == "flag.jpg")
	{
		printf("\nCan't dig a flagged area !");
	}
}

//puts or removes flags and question marks on boxes
void flag(struct box* tab, int X, int Y, struct gameSettings* rules)
{
	if (X > 0 && X < rules->width + 1 && Y>0 && Y < rules->height + 1)
	{
		//if the box is empty
		if (tab[X - 1 + rules->width * (Y - 1)].content == "darkGrass.jpg")
		{
			//add a flag
			tab[X - 1 + rules->width * (Y - 1)].content = "flag.jpg";
			//decrease how many flags left the player has to put
			rules->flags--;
		}
		//if the box has a flag
		else if (tab[X - 1 + rules->width * (Y - 1)].content == "flag.jpg")
		{
			//replace the flag by a question mark
			tab[X - 1 + rules->width * (Y - 1)].content = "question.jpg";
			//increase how many flags left the player has to put 
			rules->flags++;
		}
		//if the box has a question mark
		else if (tab[X - 1 + rules->width * (Y - 1)].content == "question.jpg")
		{
			//empty the box
			tab[X - 1 + rules->width * (Y - 1)].content = "darkGrass.jpg";
		}
	}
}

//VISUAL FUNCTION
//gives a visual representation of the minefield
void displayGrid(struct box* tab, struct gameSettings* rules,SDL_Renderer* renderer, int endDisplay)
{

	//Makes a basic play area/info bar
	SDL_Rect position;
	SDL_Surface* image;
	for (int i = 0; i < rules->height * rules->width; i++)
	{
		//if the game has ended and the box contains a mine, reveal the mine
		if (tab[i].isBomb && endDisplay)
		{
			image = IMG_Load("bomb.jpg");
		}
		else
		{
			image = IMG_Load(tab[i].content);
		}
		SDL_Texture* readyImage = SDL_CreateTextureFromSurface(renderer, image);
		position.x = (i % rules->width) * 32 + 16 ;//the horizontal position
		position.y = ((i - (i % rules->width)) / rules->height) * 32 + 16; //the vertical position
		SDL_QueryTexture(readyImage, NULL, NULL, &position.w, &position.h);
		SDL_RenderCopy(renderer, readyImage, NULL, &position);
	}

	SDL_Rect flagInfo;
	flagInfo.x = 16;
	flagInfo.y = rules->height * 32 + 32;
	image = IMG_Load("flag.jpg");
	SDL_Texture* readyImage = SDL_CreateTextureFromSurface(renderer, image);
	SDL_QueryTexture(readyImage, NULL, NULL, &flagInfo.w, &flagInfo.h);
	SDL_RenderCopy(renderer, readyImage, NULL, &flagInfo);
	SDL_Rect flagTextInfo;
	flagTextInfo.x = flagInfo.w + 16;
	flagTextInfo.y = flagInfo.y; 
	char newDisplay[25];
	//transforms the nearbyBombs pointer (int) into a string 
	sprintf_s(newDisplay, 25, "Flags left to place : %d", rules->flags);
	printf(newDisplay);
	char* text = newDisplay;
	TTF_Init();
	TTF_Font* font = NULL;
	font = TTF_OpenFont("font/comic-sans-ms_fr.ttf", 12);

	if (font != 0) {
		SDL_Color noir = { 0, 0, 0 }; //attention ce n'est pas un Uint32
		SDL_Surface* textSurf = TTF_RenderText_Blended(font, text, noir);


		if (textSurf == NULL) {
			printf("Ton texte il est NULL mec.");
			TTF_CloseFont(font);
			TTF_Quit();
		}

		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurf);
		if (texture == NULL) {
			printf("Ta texture elle est NULL mec.");
			TTF_CloseFont(font);
			TTF_Quit();
		}
		SDL_QueryTexture(texture, NULL, NULL, &flagTextInfo.w, &flagTextInfo.h);
		SDL_RenderCopy(renderer, texture, NULL, &flagTextInfo);
		
		printf("\n");
	}
	else 
	{
		printf("mange tes morts mek.");
	}
	/*
	//puts a number indicator for each column
	for (int o = 0; o < rules->width + 1; o++)
	{
		//maintains the width of the shown string so that double digit numbers don't mess up the display
		if (o < 10)
		{
			printf(" %d ", o);
		}
		else
		{
			printf(" %d", o);
		}
	}
	//displaying each line
	for (int i = 0; i < rules->height; i++)
	{
		//starting with a number, same method as previously but only once per line
		if (i < 9)
		{
			printf("\n %d ", i + 1);
		}
		else
		{
			printf("\n %d", i + 1);
		}
		//displaying each box of the line
		for (int u = 0; u < rules->width; u++)
		{
			//if the game has ended and the box contains a mine, reveal the mine as an X
			if (tab[i * rules->height + u].isBomb && endDisplay)
			{
				tab[i * rules->height + u].content = "bomb.jpg";
			}

			//shows the adequate content in the correspopnding color if it has been discovered via dig
			if (tab[i * rules->height + u].content != ' ' && tab[i * rules->height + u].content != "flag.jpg" && tab[i * rules->height + u].content != '?')
			{
				printf("%s""[%c]\x1b[0m", tab[i * rules->height + u].color, tab[i * rules->height + u].content);
			}
			//otherwise just show its content with no color
			else
			{
				printf("[%c]", tab[i * rules->height + u].content);
			}

		}
	}*/
	SDL_RenderPresent(renderer);
}

//FUNCTIONS FOR OPTIMIZATION
//returns the number entered by the user after a custom query
int numQuery(char* numberIs, char* numberPurpose, int numberLimit)
{
	int Z = 0;
	while (Z <= 0 || Z > numberLimit)
	{
		printf("\nPlease choose a valid %s you would like to %s : ", numberIs, numberPurpose);
		scanf_s("%d", &Z);
		if (Z <= 0 || Z > numberLimit)
		{
			printf("\nThis %s isn't valid...", numberIs);
			while (getchar() != '\n');
		}
	}
	return Z;
}

//returns a character entered by the user after a query
char actionQuery()
{
	char ans = ' ';
	while (ans != 'D' && ans != 'd' && ans != 'F' && ans != 'f')
	{
		//asking the user what to do, dig or put a flag on the tile
		printf("\nWhat would you like to do here ? Dig or place a Flag (D/F): ");
		scanf_s("%c", &ans, 1);
		if (ans != 'D' && ans != 'd' && ans != 'F' && ans != 'f')
		{
			//if the answer is not valid, then we can't do anything, so we raise an error then
			printf("\nYou cannot do this here...");
			while (getchar() != '\n');
		}
	}
	return ans;
}

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
			printf("\nflags left:%d, unopened boxes:%d", rules->flags, rules->unopenedBoxes);
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
					printf("Au moins un clic gauche\n");
				}
				break;
			}


			/*
			int X = numQuery("column", "play with", rules->width);
			int Y = numQuery("line", "play with", rules->height);

			//Emptying the stdin
			while (getchar() != '\n');
			//Selecting the next action to take
			char action = actionQuery();

			//Emptying the stdin again
			while (getchar() != '\n');

			//comitting the chosen action
			if (action == 'D' || action == 'd')
			{
				dig(tab, X, Y, rules);
			}
			else
			{
				flag(tab, X, Y, rules);

			}*/
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
int playAgain() {
	char ans = ' ';
	while (ans != 'Y' && ans != 'y' && ans != 'N' && ans != 'n')
	{
		//asking the user if they want to play again
		printf("\nWould you like to play again ? (Y/N)\n");
		scanf_s("%c", &ans, 1);
		if (ans != 'Y' && ans != 'y' && ans != 'N' && ans != 'n')
		{
			//if the answer is incorrect, then we raise an error
			printf("\nThat is not a valid answer.");
			while (getchar() != '\n');
		}
	}
	if (ans == 'Y' || ans == 'y')
	{
		//if they want to play again, then they play again
		return 1;
	}
	else if (ans == 'N' || ans == 'n')
	{
		//if not, the game ends here
		return 0;
	}
}

//VISUAL FUNCTIONS

//text box

//difficulty/intro window
int * startScreen()
{

	SDL_Window* startWin = SDL_CreateWindow("MineSweeper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 300, 0);
	SDL_Renderer* startRender = SDL_CreateRenderer(startWin,-1, 0);
	if (startRender) {
		SDL_SetRenderDrawColor(startRender, 0, 128, 128, 255);
		SDL_RenderClear(startRender);
		printf("Renderer created!\n");
		isRunning = true;
	}

	int gameValues[3];
	for (int i = 0; i < 3; i++)
	{
		gameValues[i] = 10;
	}

	SDL_Rect startRects[1];
	for (int i = 0; i < 6; i++)
	{
		if (i % 2)
		{
			SDL_SetRenderDrawColor(startRender, 195, 195, 195, 255);
			startRects[0].h = 70;
			startRects[0].x = 40 + (i - 1) / 2  * 175;
			startRects[0].y = 70;

		}
		else 
		{
			SDL_SetRenderDrawColor(startRender, 129, 129, 129, 255);
			startRects[0].h = 130;
			startRects[0].y = 40;
			startRects[0].x = 40 + i/2 * 175;
		}
		startRects[0].w = 70;
		SDL_RenderFillRect(startRender, startRects);
	}
	//TTF_Init();
	//TTF_Font* font = NULL;
	//font = TTF_OpenFont("comic-sans-ms_fr.ttf", 12);
	/*
	if (font != 0) {
		SDL_Color noir = { 0, 0, 0, 255 }; //attention ce n'est pas un Uint32
		SDL_Surface* texte = TTF_RenderText_Blended(font, "coucou", noir);
		//affichage
		SDL_FreeSurface(texte); //On oublie toujours pas
		TTF_CloseFont(font);
	}
	else 
	{ 
		printf("foirage à l'ouverture de times.ttf");
	}

	TTF_Quit();*/
	SDL_RenderPresent(startRender);
	/*
	while (isRunning)
	{
		//get info for width (xSize)
		//get info for height (ySize)
		//get info for bomb number
		//if <button> is clicked and EVERYTHING is set
			//break window, renderer and return values
	}
*/
	SDL_Delay(1000);
	SDL_DestroyWindow(startWin);
	SDL_DestroyRenderer(startRender);
	return gameValues;
}


//main operating function, runs the whole program
int main() {

	int playing = 1;
	while (playing)
	{
		int * inputValues = startScreen();
		//setting the grid size
		int xSize = inputValues[0];
		int ySize = inputValues[1];
		//setting the number of bombs in the grid
		int bombs = inputValues[2];

		//creating the rules based on the previous inputs
		struct gameSettings rules = { xSize, ySize, bombs, bombs, xSize * ySize - bombs, 0 };
		//beginning of game
		//memory allocation for the grid called tab
		struct box* tab = (struct box*)malloc(sizeof(struct box) * rules.width * rules.height);
		initialize(tab, &rules);
		bombPlacing(tab, &rules);
		bombRadar(tab, &rules);

		//creates a window and a renderer for the game
		SDL_Window* gameWin = SDL_CreateWindow("MineSweeper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (rules.width+1) * 32 , rules.height * 32 + 96, 0);
		SDL_Renderer* gameRender = SDL_CreateRenderer(gameWin, -1, 0);

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
		if (rules.isGameDone < 2)
		{
			playing = playAgain();
		}
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

//simple update function
void update() {
	//if things could update the code would go in here.
}