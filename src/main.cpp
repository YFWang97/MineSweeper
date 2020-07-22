#include "defines.h"
#include "helper.h"
#include "block.h"
#include "button.h"
#include <SDL2/SDL_render.h>

vector<vector<Block*>> blockVec;
vector<Block*> mineVec;

int remainingTotal;
int markedNum;
int markedNumOld;

char* levelInfo[3] = {"Easy", "Medium", "Hard"};
int mineTotal[3] = {20, 50, 80};

int level = 1;

/**
 * @param   command: 0 or LEFT_BUTTON or RIGHT_BUTTON
 *          colNum & rowNum
 * @return
 *          -1: colNum or rowNum out of range
 *          -2: game over (hitting on mine)
 */

int action(int command, int colNum, int rowNum) {
    bool leftButton = (command == LEFT_BUTTON);
    bool rightButton = (command == RIGHT_BUTTON);
    if (rowNum >= MAP_DIMENSION || colNum >= MAP_DIMENSION) return -1;

    Block* curBlock = blockVec[rowNum][colNum];

    if (rightButton) {

        if (markedNum < mineTotal[level] && 
            curBlock->get_clicked() == false &&
            curBlock->get_marked() == false) {

            markedNum++;
            if (curBlock->get_mine()) remainingTotal--;
        }

        if (curBlock->get_marked()  == true) {
            markedNum--;
            if (curBlock->get_mine()) remainingTotal++;
        }

        curBlock->toggle_mark();
    }

    if (leftButton) {
        if (curBlock->get_mine()) {
            for (auto& mine : mineVec) {
                mine->set_texture(mineTexture);
            }
            return -2;
        }

        if (curBlock->get_marked()) {
            markedNum--;
        }

        queue<Block*> pendingQ;
        pendingQ.push(curBlock);
        curBlock->click();
        while (!pendingQ.empty()) {
            Block* head = pendingQ.front();
            if (head->get_num() == 0) {
                for (int row = -1; row <= 1; row++) {
                    int newRow = head->row + row;
                    if (newRow < 0 || newRow >= MAP_DIMENSION) continue;
                    for (int col = -1; col <= 1; col++) {
                        int newCol = head->col + col;
                        if (newCol < 0 || newCol >= MAP_DIMENSION) continue;
                        if (newCol == col && newRow == row) continue;
                        if (blockVec[newRow][newCol]->get_clicked() == false &&
                            blockVec[newRow][newCol]->get_mine() == false &&
                            blockVec[newRow][newCol]->get_marked() == false) {

                            pendingQ.push(blockVec[newRow][newCol]);
                            blockVec[newRow][newCol]->click();
                        }
                    }
                }
            }
            pendingQ.pop();
        }
    }
    return 0;
}

int main (void) {

    if (initialize() || initialize_img()) {
		ERROR_MSG("PROG: Program quits. Initialization failed");
        return 0;
	}

	bool quit = false;

	SDL_Event e;
	
	bool clicked = false;


    int x, y;
    bool leftButton;
    bool rightButton;


    bool gameOver = false;
    bool win = false;
    bool newBoard = true;

    //Initlize the marked number counter
    SDL_Texture* markedNumTexture = load_text("0");
    SDL_Rect screenRect = {50, 0, 300, 300};
    int counterW, counterH;
    SDL_QueryTexture(markedNumTexture, NULL, NULL, &counterW, &counterH);
    SDL_Rect counterRect = {400 + (100 - counterW) / 2, 200, counterW, counterH};

    Button* buttons[3];
    for (int i = 0; i < 3; i++) {
        buttons[i] = new Button(levelInfo[i], {410, 40 + i * 30});
    }


    buttons[level]->click();

    for (int row = 0; row < MAP_DIMENSION; row++) {
        vector<Block*> rowVec;
        int rowPos = row * BLOCK_WIDTH;
        for (int col = 0; col < MAP_DIMENSION; col++) {
            Block* block = new Block(plainTexture, {col * BLOCK_WIDTH, rowPos}, row, col);
            rowVec.push_back(block);
        }
        blockVec.push_back(rowVec);
    }

    /* Set up the map */
    srand(SDL_GetTicks());


	while (!quit) {
        leftButton = false;
        rightButton = false;
        clicked = false;

		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
				case SDL_QUIT:
					quit = true;
					break;
                case SDL_MOUSEBUTTONDOWN: {
                    SDL_GetMouseState(&x, &y);

                    for (int i = 0; i < 3; i++) {
                        if (buttons[i]->inside(x, y)) {
                            if (i != level) {
                                buttons[level]->release();
                                buttons[i]->click();
                                level = i;
                                gameOver = false;
                                win = false;
                                newBoard = true;
                            }
                        } 
                    }


                    if (gameOver || win) {
                        gameOver = false;
                        win = false;
                        newBoard = true;
                        break;
                    } else {
                        clicked = true;
                        if (e.button.button == SDL_BUTTON_LEFT) {
                            leftButton = true;
                            rightButton = false;
                            break;
                        } else if (e.button.button == SDL_BUTTON_RIGHT) {
                            leftButton = false;
                            rightButton = true;
                            break;
                        }
                    }
                }
			}
		}
        
        if (newBoard) {
            for (int row = 0; row < MAP_DIMENSION; row++) {
                for (int col = 0; col < MAP_DIMENSION; col++) {
                    blockVec[row][col]->reset();
                }
            }

            mineVec.clear();

            int setNum = mineTotal[level];
            while (setNum > 0) {
                int mineRow = rand() % MAP_DIMENSION;
                int mineCol = rand() % MAP_DIMENSION;

                if (blockVec[mineRow][mineCol]->get_mine()) {
                    continue;
                } else {
                    blockVec[mineRow][mineCol]->set_mine();
                    mineVec.push_back(blockVec[mineRow][mineCol]);

                    for (int row = -1; row <= 1; row++) {
                        int newRow = mineRow + row;
                        if (newRow < 0 || newRow >= MAP_DIMENSION) continue;
                        for (int col = -1; col <= 1; col++) {
                            int newCol = mineCol + col;
                            if (newCol < 0 || newCol >= MAP_DIMENSION) continue;
                            blockVec[newRow][newCol]->set_num();
                        }
                    }
                    setNum--;
                }
            }
            newBoard = false;
            remainingTotal = mineTotal[level];
            markedNum = 0;
        }

        if (clicked) {

            int rowNum = floor((double)y / BLOCK_WIDTH);
            int colNum = floor((double)x / BLOCK_WIDTH);
            int command = leftButton ? LEFT_BUTTON : (rightButton ? RIGHT_BUTTON : 0);

            int actionResult = action(command, colNum, rowNum);
            if (actionResult == -1) continue;
            if (actionResult == -2) gameOver = true;
        }

        if (remainingTotal == 0) {
            win = true;
        } 
        

		SDL_RenderClear(gRenderer);
        
        for (int row = 0; row < MAP_DIMENSION; row++) {
            vector<Block*> rowVec;
            int rowPos = row * BLOCK_WIDTH;
            for (int col = 0; col < MAP_DIMENSION; col++) {
                blockVec[row][col]->draw();
                if (blockVec[row][col]->get_num() > 0 && blockVec[row][col]->get_clicked()) {
                    blockVec[row][col]->draw_num();
                }
            }
        }

        if (gameOver) {
            if (SDL_RenderCopy(gRenderer, overTexture, NULL, &screenRect)) {
                SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
            }
        }

        if (win) {
            if (SDL_RenderCopy(gRenderer, winTexture, NULL, &screenRect)) {
                SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
            }
        } 

        for (auto& button : buttons) {
            button->draw();
        }

        //When the marked number of tiles changed
        //Need to update the counter
        if (markedNum != markedNumOld) {
            markedNumOld = markedNum;
            SDL_DestroyTexture(markedNumTexture);
            markedNumTexture = load_text(to_string(markedNum).c_str());
            SDL_QueryTexture(markedNumTexture, NULL, NULL, &counterW, &counterH);
            counterRect = {400 + (100 - counterW) / 2, 200, counterW, counterH};
        }
 

        if (SDL_RenderCopy(gRenderer, markedNumTexture, NULL, &counterRect)) {
            SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
        }

		SDL_RenderPresent(gRenderer);

        //SDL_Delay(200);
	}

	quit_game();

	return 0;
}
