#include "game.h"


Button* buttons[NUM_OF_BUTTONS];
vector<vector<Block*>> blockVec;
vector<Block*> mineVec;

vector<Block*> pendingRevealVec;

//Game Status Control
bool gameQuit;

uint8_t gameStatus;

queue<MouseEvent> mouseEventQ;
uint32_t prevMouseState = 0;
bool mouseWaitingFirstReveal;
bool mousePressed = false;

//Previous Tile Selection Coordinate
Coordinate prevTileCoord = {-1, -1};

//Game Status Vars
int remainingTotal;
int flagedNum;
int flagedNumOld;

int timer;
int timerOld;
int initTime;
bool timerRunning;

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

SDL_Rect timerRect;
int timerH, timerW;
SDL_Texture* timerTexture;

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

Command proccess_mouse_event() {
	Command newCommand;
	newCommand.commandDisplay = 0;
	newCommand.commandExecution = 0;
	if (!mouseEventQ.empty()) {
		mouseEvent newMouseEvent = mouseEventQ.front();
		mouseEventQ.pop();
		uint32_t newMouseState = newMouseEvent.state;
		uint32_t newMouseType = newMouseEvent.type;
		Coordinate mouseCoord = newMouseEvent.coord;

		newCommand.mouseCoord = mouseCoord;
		newCommand.tileCoord = mouseCoord.translate();
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
				//In button action, all other execution commands will be deasserted
				newCommand.commandExecution = CMD_EXEC_BUTTON_ACTION;
			}
			newCommand.commandDisplay = CMD_DISP_TARGET_CHANGED;
			prevMouseState = 0;
			newMouseState = 0;
		} else {
			if (newMouseType == SDL_QUIT) {
				gameQuit = true;
			} else {
				if (newMouseType == SDL_MOUSEMOTION) {
					if (!(newCommand.tileCoord == prevTileCoord)) {
						newCommand.commandDisplay |= CMD_DISP_TARGET_CHANGED; 
						prevTileCoord = newCommand.tileCoord;
					}
					if (newMouseState == 5) newCommand.commandDisplay |= CMD_DISP_GRAY_SURR_TILES;
					if (newMouseState == 1) newCommand.commandDisplay |= CMD_DISP_GRAY_ONE_TILE;
				} else if (newMouseType == SDL_MOUSEBUTTONDOWN || newMouseType == SDL_MOUSEBUTTONUP) {
					newCommand.commandDisplay |= CMD_DISP_TARGET_CHANGED;
					prevTileCoord = newCommand.tileCoord;
					if (newMouseState == 5) {
						newCommand.commandDisplay |= CMD_DISP_GRAY_SURR_TILES;
						mousePressed = true;
					} else if (prevMouseState == 0 && newMouseState == 1) {
						newCommand.commandDisplay |= CMD_DISP_GRAY_ONE_TILE;
						mousePressed = true;
					} else {
						if (prevMouseState == 5) {
							newCommand.commandExecution |= CMD_EXEC_REVEAL_SURR_TILES;
						} else if (prevMouseState == 0 && newMouseState == 4) {
							newCommand.commandExecution |= CMD_EXEC_FLAG_ONE_TILE;
						} else if (prevMouseState == 1 && newMouseState == 0) {
							newCommand.commandExecution |= CMD_EXEC_REVEAL_ONE_TILE;
							if (mouseWaitingFirstReveal) {
								mouseWaitingFirstReveal = false;
								newCommand.commandExecution |= CMD_EXEC_FIRST_REVEAL;
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
	return newCommand;
}

void button_action(Command command) {
	//Process button action only when hitting button area
	if ((command.commandExecution & CMD_EXEC_BUTTON_ACTION) == 0) return;

	//mouseButtonAction = false;
	for (int i = 0; i < NUM_OF_BUTTONS; i++) {
		if (buttons[i]->inside(command.mouseCoord.x, command.mouseCoord.y)) {
			gameStatus = STATUS_GAME_NEW_BOARD;
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

void tile_action(Command command) {
	if (command.commandExecution == 0 ||
		command.commandExecution == CMD_EXEC_BUTTON_ACTION ||
		gameStatus == STATUS_GAME_OVER ||
		gameStatus == STATUS_GAME_WIN) {
		return;
	}

	//mouseClicked = false;
	int rowNum = command.tileCoord.y;
	int colNum = command.tileCoord.x;
	if (rowNum >= levelSize[gameLevel].first || colNum >= levelSize[gameLevel].second) return;

	Block* curBlock = blockVec[rowNum][colNum];

	if ((command.commandExecution & CMD_EXEC_FLAG_ONE_TILE) == CMD_EXEC_FLAG_ONE_TILE) {
		//mouseFlagOneTile = false;

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

	if ((command.commandExecution & CMD_EXEC_REVEAL_ONE_TILE) == CMD_EXEC_REVEAL_ONE_TILE ||
		(command.commandExecution & CMD_EXEC_REVEAL_SURR_TILES) == CMD_EXEC_REVEAL_SURR_TILES) {
		deque<Block*> pendingQ;
		if ((command.commandExecution & CMD_EXEC_REVEAL_ONE_TILE) == CMD_EXEC_REVEAL_ONE_TILE) {
			if (curBlock->get_mine()) {
				gameStatus = STATUS_GAME_OVER;
				curBlock->explode(1);
				return;
			}

			if (curBlock->get_flaged()) {
				flagedNum--;
			}
			pendingQ.push_back(curBlock);
		} else if ((command.commandExecution & CMD_EXEC_REVEAL_SURR_TILES) == CMD_EXEC_REVEAL_SURR_TILES) {
			int numOfSurrFlags = 0;
			bool hasUnflagedMine = false;
			Block* unflagedMine;
			//If this is not a revealed tile with a number geater than 0, do nothing
			if (curBlock->get_num() == 0 || !curBlock->get_revealed()) {
				return;
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
						unflagedMine = tempBlock;
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
					gameStatus = STATUS_GAME_OVER;
					unflagedMine->explode(1);
					return;
				}
			} else { //Number does not match, do nothing
				return;
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
    return;
}

void set_new_board() {
	//Only set new board when needed
	if ((gameStatus & STATUS_GAME_NEW_BOARD) == 0) return;

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
	gameStatus = 0;

	remainingTotal = mineTotal[gameLevel];
	flagedNum = 0;

	timerRunning = false;
	timer = 0;
}

void set_mines(Command command) {
	//Only set mines at the first reveal
	if ((command.commandExecution & CMD_EXEC_FIRST_REVEAL) == 0) return;

	mineVec.clear();

	int setNum = mineTotal[gameLevel];
	while (setNum > 0) {
		int mineRow = rand() % levelSize[gameLevel].first;
		int mineCol = rand() % levelSize[gameLevel].second;

		bool aroundFirstReveal = (abs(mineRow - command.tileCoord.y) <= 1) && (abs(mineCol - command.tileCoord.x) <= 1);

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
	gameStatus = STATUS_GAME_NEW_BOARD;
	mouseWaitingFirstReveal = true;

	timerRunning = false;

	set_new_board();
}


void game_status_check() {
	if (remainingTotal == 0) gameStatus = STATUS_GAME_WIN;
	if ((gameStatus & STATUS_GAME_NEW_BOARD) == STATUS_GAME_NEW_BOARD) {
		set_new_board();
	}
	if ((gameStatus & STATUS_GAME_OVER) == STATUS_GAME_OVER) {
		for (auto& mine : mineVec) {
			if (mine->get_exploded() == false) {
				mine->explode(0);
			}	
		}
	}
	if ((gameStatus & STATUS_GAME_WIN) == STATUS_GAME_WIN ||
		(gameStatus & STATUS_GAME_OVER) == STATUS_GAME_OVER) {
		timerRunning = false;
	}

	if (timerRunning) {
		timer = (SDL_GetTicks() - initTime) / 1000;
	}

}

void display(Command command) {
	//Update pending reveal list rules:
	//First need to ensure the mouse is pressed
	//Second it has a TARGET CHANGED
	//In that case, can refresh the pending reveal list and append new if applicable
	//Based on DISP_GRAY_ONE_TILE and DISP_GRAY_SURR_TILES CMD
	if ((command.commandDisplay & CMD_DISP_TARGET_CHANGED) == CMD_DISP_TARGET_CHANGED &&
		mousePressed &&
		((gameStatus & STATUS_GAME_WIN) == 0) &&
		((gameStatus & STATUS_GAME_OVER) == 0)) {
		for (auto& block : pendingRevealVec) {
			block->unset_pendingReveal();
		}
		pendingRevealVec.clear();

		if ((command.commandDisplay & CMD_DISP_GRAY_ONE_TILE) == CMD_DISP_GRAY_ONE_TILE) {
			Block* curBlock = blockVec[command.tileCoord.y][command.tileCoord.x];
			curBlock->set_pendingReveal();
			pendingRevealVec.push_back(curBlock);
		} 

		if ((command.commandDisplay & CMD_DISP_GRAY_SURR_TILES) == CMD_DISP_GRAY_SURR_TILES) {
			for (int row = -1; row <= 1; row++) {
				int newRow = command.tileCoord.y + row;
				if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
				for (int col = -1; col <= 1; col++) {
					int newCol = command.tileCoord.x + col;
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
			if ((gameStatus & STATUS_GAME_OVER) == STATUS_GAME_OVER && 
				curBlock->get_flaged() && !curBlock->get_mine()) {
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

    if ((gameStatus & STATUS_GAME_OVER) == STATUS_GAME_OVER) {
        if (SDL_RenderCopy(gRenderer, overTexture, NULL, &screenRect)) {
            SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
        }
    }

    if ((gameStatus & STATUS_GAME_WIN) == STATUS_GAME_WIN) {
        if (SDL_RenderCopy(gRenderer, winTexture, NULL, &screenRect)) {
            SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
        }
    } 

}
