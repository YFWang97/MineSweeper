#include "defines.h"
#include "helper.h"
#include "block.h"
#include "button.h"
#include <SDL2/SDL_render.h>

vector<vector<Block*>> blockVec;
vector<Block*> mineVec;

vector<Block*> pendingRevealVec;

//Game Status Params
int remainingTotal;
int flagedNum;
int flagedNumOld;

int gameLevel;
int prevGameLevel;
char* levelInfo[NUM_OF_BUTTONS] = {"Easy", "Medium", "Hard", "Restart"};
int mineTotal[NUM_OF_BUTTONS - 1] = {10, 40, 99};
pair<int, int> levelSize[NUM_OF_BUTTONS - 1] = {make_pair(9, 9), make_pair(16, 16), make_pair(16, 30)};
pair<int, int> windowSize[NUM_OF_BUTTONS - 1] = {make_pair(300, 180), make_pair(440, 320), make_pair(720, 320)};

SDL_Rect screenRect;
SDL_Rect counterRect;
int counterH, counterW;
SDL_Texture* flagedNumTexture;

int timer;
int timerOld;
int initTime;
bool timerRunning;
SDL_Rect timerRect;
int timerH, timerW;
SDL_Texture* timerTexture;

queue<MouseEvent> mouseEventQ;
void collect_mouse_event() {
	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		MouseEvent newMouseEvent;

		//For event other than SDL_MOUSEMOTION, set coord and state to 0
		newMouseEvent.coord = {0, 0};
		newMouseEvent.state = 0;

		if (e.type & SDL_MOUSEMOTION) {
			uint32_t mouseState = SDL_GetMouseState(&newMouseEvent.coord.x, &newMouseEvent.coord.y);
			newMouseEvent.state = mouseState;
		}
		newMouseEvent.type = e.type;

		//Ignore events outside the gamewindow
		//Use SDL_WINDOWEVENT as a mask
		if (!(newMouseEvent.type & SDL_WINDOWEVENT)) {
			mouseEventQ.push(newMouseEvent);
		}
	}
}


uint32_t prevMouseState = 0;
//Mouse Commands - Execution
//These signals are asserted by mouse processing
//and deasserted by action logic after action performed
bool mouseClicked;
bool mouseRevealOneTile;
bool mouseFlagOneTile;
bool mouseRevealSurrTiles;

//Mouse Commands - Display
//These signals are asserted and deasserted by mouse processing
//Since they control display unit directly
bool mouseGrayOneTile;
bool mouseGraySurrTiles;
//Both real target changes or a click on a target can assert this signal
//This signal controls when we gray tiles
bool mouseTileTargetChanged;

bool mouseButtonAction;
bool mouseWaitingFirstReveal;
bool mouseFirstReveal;
Coordinate firstRevealCoord;

Coordinate revealedMineCoord;

//Game Status Control
bool gameQuit;
bool gameOver;
bool gameWin;
bool gameNewBoard;

//Tile Selection Coordinate
Coordinate tileCoord;
Coordinate prevTileCoord = {-1, -1};
Coordinate mouseCoord;

void proccess_mouse_event() {
	if (!mouseEventQ.empty()) {
		mouseEvent newMouseEvent = mouseEventQ.front();
		mouseEventQ.pop();
		uint32_t newMouseState = newMouseEvent.state;
		uint32_t newMouseType = newMouseEvent.type;
		mouseCoord = newMouseEvent.coord;

		//cout << newMouseEvent << endl;

		//If type is motion only
		//Preserve all signals
		//Other control logic change base on state change
		//anynum -> 5	: mouseGraySurrTiles
		//0 -> 1		: mouseGrayOneTile
		//5 -> anynum	: mouseRevealSurrTiles
		//0 -> 4		: mouseFlagOneTIle
		//1 -> 0		: mouseRevealOneTile
		//
		//When assert an execution command, need to deassert all display command.
		if (mouseCoord.x > BLOCK_WIDTH * levelSize[gameLevel].second) {
		   	
			if (prevMouseState == 0 && 
				newMouseState == 1) {
				mouseClicked = false;
				mouseButtonAction = true;
			}
			mouseGrayOneTile = false;
			mouseGraySurrTiles = false;
			mouseTileTargetChanged = true;
			prevMouseState = 0;
			newMouseState = 0;
		} else {
			if (newMouseType == SDL_QUIT) {
				gameQuit = true;
			} else {
				//Record the tileCoord only in game region
				tileCoord = mouseCoord.translate();
				if (newMouseType == SDL_MOUSEMOTION) {
					if (!(tileCoord == prevTileCoord)) {
						mouseTileTargetChanged = true;
						prevTileCoord = tileCoord;
					} else {
						mouseTileTargetChanged = false;
					}
					mouseClicked = false;	
				} else if (newMouseType == SDL_MOUSEBUTTONDOWN || newMouseType == SDL_MOUSEBUTTONUP) {
					mouseClicked = true;
					mouseTileTargetChanged = true;
					prevTileCoord = tileCoord;
					if (newMouseState == 5) {
						mouseGraySurrTiles = true;
					} else if (prevMouseState == 0 && newMouseState == 1) {
						mouseGrayOneTile = true;
					} else {
						mouseGraySurrTiles = false;
						mouseGrayOneTile = false;
						if (prevMouseState == 5) {
							mouseRevealSurrTiles = true;
						} else if (prevMouseState == 0 && newMouseState == 4) {
							mouseFlagOneTile = true;
						} else if (prevMouseState == 1 && newMouseState == 0) {
							mouseRevealOneTile = true;
							if (mouseWaitingFirstReveal) {
								mouseWaitingFirstReveal = false;
								firstRevealCoord = tileCoord;
								mouseFirstReveal = true;
								initTime = SDL_GetTicks();
								timerRunning = true;
							}
						}
					}
				}
				prevMouseState = newMouseState;
			}
		}
	}
}

Button* buttons[NUM_OF_BUTTONS];
void button_action() {
	mouseButtonAction = false;
	for (int i = 0; i < NUM_OF_BUTTONS; i++) {
		if (buttons[i]->inside(mouseCoord.x, mouseCoord.y)) {
			gameOver = false;
			gameWin = false;
			gameNewBoard = true;
			mouseWaitingFirstReveal = true;
			if (i != gameLevel && i != RESET) {
				buttons[gameLevel]->release();
				buttons[i]->click();
				prevGameLevel = gameLevel;
				gameLevel = i;
			}
			break;
		} 
	}
}

bool tile_action() {
	mouseClicked = false;
	int rowNum = tileCoord.y;
	int colNum = tileCoord.x;
	if (rowNum >= levelSize[gameLevel].first || colNum >= levelSize[gameLevel].second) return -1;

	Block* curBlock = blockVec[rowNum][colNum];

	if (mouseFlagOneTile) {
		mouseFlagOneTile = false;

		if (flagedNum < mineTotal[gameLevel] && 
			curBlock->get_revealed() == false &&
			curBlock->get_flaged() == false) {

			flagedNum++;
			if (curBlock->get_mine()) { 
				remainingTotal--;
			}
		}

		if (curBlock->get_flaged()  == true) {
			flagedNum--;
			if (curBlock->get_mine()) remainingTotal++;
		}

		curBlock->toggle_flag();
	}

	if (mouseRevealOneTile || mouseRevealSurrTiles) {
		deque<Block*> pendingQ;
		if (mouseRevealOneTile) {
			mouseRevealOneTile = false;
			if (curBlock->get_mine()) {
				gameOver = true;
				revealedMineCoord = tileCoord;
				curBlock->reveal();
				return -2;
			}

			if (curBlock->get_flaged()) {
				flagedNum--;
			}
			pendingQ.push_back(curBlock);
		} else if (mouseRevealSurrTiles) {
			mouseRevealSurrTiles = false;
			int numOfSurrFlags = 0;
			bool hasUnflagedMine = false;
			//If this is not a revealed tile with a number geater than 0, do nothing
			if (curBlock->get_num() == 0 || !curBlock->get_revealed()) {
				return 0;
			}
			for (int row = -1; row <= 1; row++) {
				int newRow = rowNum + row;
				if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
				for (int col = -1; col <= 1; col++) {
					int newCol = colNum + col;
					if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
					if (newCol == colNum && newRow == rowNum) continue;
					Block* tempBlock = blockVec[newRow][newCol];
					if (tempBlock->get_mine() && !tempBlock->get_flaged()) {
						hasUnflagedMine = true;
						revealedMineCoord = {newCol, newRow};
					}
					if (tempBlock->get_flaged()) {
						numOfSurrFlags++;
					}
					if (!tempBlock->get_flaged() && !tempBlock->get_revealed()) {
						pendingQ.push_back(tempBlock);
					}
				}
			}
			if (numOfSurrFlags == curBlock->get_num()) {
				if (hasUnflagedMine) {
					gameOver = true;
					return -2;
				}
			} else { //Number does not match, do nothing
				return 0;
			}
		}

		//Use revealed as a visited flga to avoid infinite loop
		for (auto& block : pendingQ) {
			block->reveal();
		}

		while (!pendingQ.empty()) {
			Block* head = pendingQ.front();
			if (head->get_num() == 0) {
				for (int row = -1; row <= 1; row++) {
					int newRow = head->row + row;
					if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
					for (int col = -1; col <= 1; col++) {
						int newCol = head->col + col;
						if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
						if (newCol == head->col && newRow == head->row) continue;
						if (blockVec[newRow][newCol]->get_revealed() == false &&
							blockVec[newRow][newCol]->get_mine() == false &&
							blockVec[newRow][newCol]->get_flaged() == false &&
							blockVec[newRow][newCol]->get_init() == true) {

							pendingQ.push_back(blockVec[newRow][newCol]);
							blockVec[newRow][newCol]->reveal();
						}
					}
				}
			}
			pendingQ.pop_front();
		}
	}
    return 0;
}

void set_new_board() {
	//ScreenRect is used for WIN/GAMEOVER sign
    screenRect = {0, 0, windowSize[gameLevel].first, windowSize[gameLevel].second};
    //Initlize the flaged number counter
	string flagNumText = "Remaining:" + to_string(mineTotal[gameLevel]);
    flagedNumTexture = load_text(flagNumText.c_str());
    SDL_QueryTexture(flagedNumTexture, NULL, NULL, &counterW, &counterH);
    counterRect = {levelSize[gameLevel].second * BLOCK_WIDTH + (100 - counterW) / 2, 0, counterW, counterH};

    timerTexture = load_text("Time: 0");
    SDL_QueryTexture(timerTexture, NULL, NULL, &timerW, &timerH);
    timerRect = {levelSize[gameLevel].second * BLOCK_WIDTH + (100 - timerW) / 2, 20, timerW, timerH};

	for (int row = 0; row < levelSize[gameLevel].first; row++) {
		for (int col = 0; col < levelSize[gameLevel].second; col++) {
			blockVec[row][col]->reset();
		}
	}
	if (prevGameLevel != gameLevel) {
		for (auto& button : buttons) {
			button->set_pos_x(levelSize[gameLevel].second * BLOCK_WIDTH + 10);
		}	
		SDL_SetWindowSize(gWindow, windowSize[gameLevel].first, windowSize[gameLevel].second);
	}
	gameWin = false;
	gameOver = false;
	gameNewBoard = false;

	remainingTotal = mineTotal[gameLevel];
	flagedNum = 0;

	timerRunning = false;
	timer = 0;
}

void set_mines() {

	mineVec.clear();

	int setNum = mineTotal[gameLevel];
	while (setNum > 0) {
		int mineRow = rand() % levelSize[gameLevel].first;
		int mineCol = rand() % levelSize[gameLevel].second;

		bool aroundFirstReveal = (abs(mineRow - firstRevealCoord.y) <= 1) && (abs(mineCol - firstRevealCoord.x) <= 1);

		//Do not put on existing mine
		//Do not put on the surronding of the frst reveal
		if (blockVec[mineRow][mineCol]->get_mine()) {
			continue;
		} else if (aroundFirstReveal) {
			continue;
		} else {
			blockVec[mineRow][mineCol]->set_mine();
			mineVec.push_back(blockVec[mineRow][mineCol]);

			for (int row = -1; row <= 1; row++) {
				int newRow = mineRow + row;
				if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
				for (int col = -1; col <= 1; col++) {
					int newCol = mineCol + col;
					if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
					blockVec[newRow][newCol]->set_num();
				}
			}
			setNum--;
		}
	}
	for (int row = 0; row < levelSize[gameLevel].first; row++) {
		for (int col = 0; col < levelSize[gameLevel].second; col++) {
			blockVec[row][col]->set_init();
		}
	}
	mouseFirstReveal = false;
}


void initialize_game() {
	gameLevel = MEDIUM;

    for (int i = 0; i < NUM_OF_BUTTONS; i++) {
        buttons[i] = new Button(levelInfo[i], {levelSize[gameLevel].second * BLOCK_WIDTH + 10, 40 + i * 30});
    }


    buttons[gameLevel]->click();

    for (int row = 0; row < levelSize[HARD].first; row++) {
        vector<Block*> rowVec;
        int rowPos = row * BLOCK_WIDTH;
        for (int col = 0; col < levelSize[HARD].second; col++) {
            Block* block = new Block(plainTexture, {col * BLOCK_WIDTH, rowPos}, row, col);
            rowVec.push_back(block);
        }
        blockVec.push_back(rowVec);
    }

    /* Set up the map */
	int seed = SDL_GetTicks();
	srand(seed);

	printf("Current seed is %d\n", seed);

	//Get a new board
	gameNewBoard = true;
	mouseWaitingFirstReveal = true;
	mouseFirstReveal = false;

	timerRunning = false;

	set_new_board();
}


void game_status_check() {
	if (remainingTotal == 0) gameWin = true;
	if (gameNewBoard) {
		set_new_board();
	}
	if (gameOver) {
		for (auto& mine : mineVec) {
			if (revealedMineCoord.y == mine->row && revealedMineCoord.x == mine->col) {
				mine->explode(1);
			} else {
				mine->explode(0);
			}	
		}
	}
	if (gameWin || gameOver) {
		timerRunning = false;
	}

	if (timerRunning) {
		timer = (SDL_GetTicks() - initTime) / 1000;
	}

}

void display() {
	if (mouseTileTargetChanged && !(gameWin || gameOver)) {
		mouseTileTargetChanged = false;
		for (auto& block : pendingRevealVec) {
			block->unset_pendingReveal();
		}
		pendingRevealVec.clear();

		if (mouseGrayOneTile) {
			Block* curBlock = blockVec[tileCoord.y][tileCoord.x];
			curBlock->set_pendingReveal();
			pendingRevealVec.push_back(curBlock);
		} 

		if (mouseGraySurrTiles) {
			for (int row = -1; row <= 1; row++) {
				int newRow = tileCoord.y + row;
				if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
				for (int col = -1; col <= 1; col++) {
					int newCol = tileCoord.x + col;
					if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
					Block* curBlock = blockVec[newRow][newCol];
					if (!curBlock->get_flaged() && !curBlock->get_revealed()) {
						curBlock->set_pendingReveal();
						pendingRevealVec.push_back(curBlock);
					}
				}
			}
		}
	}


	//Display all tiles	
	for (int row = 0; row < levelSize[gameLevel].first; row++) {
		for (int col = 0; col < levelSize[gameLevel].second; col++) {
			Block* curBlock = blockVec[row][col];
			if (gameOver && curBlock->get_flaged() && !curBlock->get_mine()) {
				curBlock->set_texture(mineWrongTexture);
			}
			curBlock->draw();
		}
	}

	//Display all buttons
	for (auto& button : buttons) {
		button->draw();
	}

    //When the flaged number of tiles changed
    //Need to update the counter
    if (flagedNum != flagedNumOld) {
        flagedNumOld = flagedNum;
        SDL_DestroyTexture(flagedNumTexture);
		string flagNumText = "Remaining:" + to_string(mineTotal[gameLevel] - flagedNum);
        flagedNumTexture = load_text(flagNumText.c_str());
        SDL_QueryTexture(flagedNumTexture, NULL, NULL, &counterW, &counterH);
        counterRect = {levelSize[gameLevel].second * BLOCK_WIDTH + (100 - counterW) / 2 + 10, 0, counterW, counterH};
    }
 

    if (SDL_RenderCopy(gRenderer, flagedNumTexture, NULL, &counterRect)) {
        SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
    }

    //Need to update the timer, when change in second
    if (timer != timerOld) {
        timerOld = timer;
        SDL_DestroyTexture(timerTexture);
		string timerText = "Time:" + to_string(timer);
        timerTexture = load_text(timerText.c_str());
        SDL_QueryTexture(timerTexture, NULL, NULL, &timerW, &timerH);
        timerRect = {levelSize[gameLevel].second * BLOCK_WIDTH + (100 - timerW) / 2, 20, timerW, timerH};
    }
 

    if (SDL_RenderCopy(gRenderer, timerTexture, NULL, &timerRect)) {
        SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
    }

    if (gameOver) {
        if (SDL_RenderCopy(gRenderer, overTexture, NULL, &screenRect)) {
            SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
        }
    }

    if (gameWin) {
        if (SDL_RenderCopy(gRenderer, winTexture, NULL, &screenRect)) {
            SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
        }
    } 

}

int main (void) {

    if (initialize_sdl() || initialize_img()) {
		ERROR_MSG("PROG: Program quits. SDL Initialization failed");
        return 0;
	}


	initialize_game();


	//Main Game Loop
	while (!gameQuit) {

		collect_mouse_event();
		proccess_mouse_event();
		
		if (mouseFirstReveal) {
			set_mines();
		}

		if (mouseButtonAction) {
			button_action();
		}

		if (gameNewBoard) {
			set_new_board();
		}

		if (mouseClicked && !gameOver && !gameWin) {
			tile_action();
		}

		game_status_check();        

		SDL_RenderClear(gRenderer);
       
		display();

		SDL_RenderPresent(gRenderer);

        //SDL_Delay(1000);
	}

	quit_game();

	return 0;
}
