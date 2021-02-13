#include "solver.h"

uint8_t* blockArry;
int totalNumRow, totalNumCol;

queue<Coordinate> pendingFlagQ;
queue<Coordinate> pendingRevealQ;

//Push the information in the queue
//For the trivial ones (remaining open tiles == remaining mines) push front
//Process these ones first
deque<pair<vector<Coordinate>, uint8_t>>* infoArry[9];

void initialize_solver() {

	totalNumRow = levelSize[gameLevel].first;
	totalNumCol = levelSize[gameLevel].second;
	blockArry = (uint8_t*) malloc(totalNumRow * totalNumCol);
	for (int i = 0; i < 9; i++) {
		infoArry[i] = new(deque<pair<vector<Coordinate>, uint8_t>>);
	}
}

Command solver(vector<RevealedBlock> revealedBlocksVec, Command command) {
	Command newCommand;
	newCommand.commandExecution = 0;
	newCommand.commandDisplay = 0;

	//If it's a button action. Need to pass the command from mouse to all the next step
	//The user input has the highest priority
	if ((command.commandExecution & CMD_EXEC_BUTTON_ACTION) == CMD_EXEC_BUTTON_ACTION) {
		return command;
	}

	if ((gameStatus & STATUS_GAME_OVER) == STATUS_GAME_OVER ||
		(gameStatus & STATUS_GAME_WIN) == STATUS_GAME_WIN) {
		if ((gameStatus & STATUS_GAME_WIN) == STATUS_GAME_WIN) {
			cout << "WIN A GAME" << endl;
		}
		gameStatus = STATUS_GAME_WAIT_TO_START;
		newCommand.commandExecution |= CMD_EXEC_NEW_BOARD;
		//In the case the game wins or over. Send the command to the board setting part
		//Reset boards and reset status to WAIT_TO_START
		//Don't give any tile selection
		return newCommand;
	}


	//First tile selection
	if ((gameStatus & STATUS_GAME_WAIT_TO_START) == STATUS_GAME_WAIT_TO_START) {
		gameStatus &= (~STATUS_GAME_WAIT_TO_START);

		for (int i = 0; i < 9; i++) {
			delete(infoArry[i]);
			infoArry[i] = new(deque<pair<vector<Coordinate>, uint8_t>>);
		}

		free(blockArry);

		totalNumRow = levelSize[gameLevel].first;
		totalNumCol = levelSize[gameLevel].second;

		blockArry = (uint8_t*) malloc(totalNumRow * totalNumCol);
		memset(blockArry, 0xFF, totalNumRow * totalNumCol);
		
		newCommand.commandExecution = CMD_EXEC_FIRST_REVEAL | CMD_EXEC_REVEAL_ONE_TILE;
		newCommand.tileCoord.x = rand() % totalNumCol;
		newCommand.tileCoord.y = rand() % totalNumRow;

		return newCommand;
	} 

	//General case selection

	//Iterate only new information packet are aquired
	bool newInfo = false;

	if (revealedBlocksVec.size() > 0) {
		//Update the local block array first
		for (auto& revealedBlock: revealedBlocksVec) {
			int curRow = revealedBlock.coord.y;
			int curCol = revealedBlock.coord.x;
			blockArry[curRow * totalNumCol + curCol] = revealedBlock.num;
		}
		
		//Start to gather information
		for (auto& revealedBlock: revealedBlocksVec) {
			int curRow = revealedBlock.coord.y;
			int curCol = revealedBlock.coord.x;

			uint8_t numOfOpenTiles = 0;
			uint8_t numOfRemainMines = revealedBlock.num;
			vector<Coordinate> openTileVec;
			for (int row = -1; row <= 1; row++) {
				int newRow = curRow + row;
				if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
				for (int col = -1; col <= 1; col++) {
					int newCol = curCol + col;
					if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
					uint8_t tileNum = blockArry[newRow * totalNumCol + newCol]; 
					if (tileNum == OPEN_TILE) {
						openTileVec.push_back({newCol, newRow});
						numOfOpenTiles++;
					}
					if (tileNum == FLAGED_TILE) {
						numOfRemainMines--;
					}
				}
			}
			if (numOfOpenTiles > 0) {
				if (numOfOpenTiles == numOfRemainMines) {
					infoArry[numOfOpenTiles]->push_front(make_pair(openTileVec, numOfRemainMines));
				} else {
					infoArry[numOfOpenTiles]->push_back(make_pair(openTileVec, numOfRemainMines));
				}
			}
		}

		newInfo = true;
	}	

	if (newInfo) {
	
		int numOfCoords;
		for (numOfCoords = 0; numOfCoords < 8; numOfCoords++) {
			deque<pair<vector<Coordinate>, uint8_t>>* infoQ = infoArry[numOfCoords];
			int numOfInfo = infoQ->size();
			for (int i = 0; i < numOfInfo; i++) {
				pair<vector<Coordinate>, uint8_t> info = infoQ->front();
				infoQ->pop_front();	
				//If the remaining mines becomes 0, it's safe to open all the tiles in the vector
				if (info.second == 0) {
					for (auto& coord : info.first) {
						pendingRevealQ.push(coord);
					}
				} else { 
					//If the remaining mines == size of vector
					//It means all the tiles in the vector are mines
					if (info.first.size() == info.second) {
						for (auto& coord : info.first) {
							//Don't do repetitive flag. So skip when it's been flaged.
							if (blockArry[coord.y * totalNumCol + coord.x] != OPEN_TILE) {
								continue;
							}
							blockArry[coord.y * totalNumCol + coord.x] = FLAGED_TILE;
							pendingFlagQ.push(coord);
						}
					} else {
						//In the case that vector size greater than remaining mines
						//Need to reconstruct the information struct.
						//If a tile is still open to flag, add it back to the vec
						//If it's flaged, update the remaining mines number and don't add the tile to vec
						vector<Coordinate> newOpenTileVec;
						uint8_t newNumOfRemainMines = info.second;
						for (auto& coord : info.first) {
							if (blockArry[coord.y * totalNumCol + coord.x] == OPEN_TILE) {
								newOpenTileVec.push_back(coord);
							}
							if (blockArry[coord.y * totalNumCol + coord.x] == FLAGED_TILE) {
								newNumOfRemainMines--;
							}

						}
						uint8_t newNumOfOpenTiles = newOpenTileVec.size();
						if (newNumOfOpenTiles == newNumOfRemainMines) {
							infoArry[newNumOfOpenTiles]->push_front(make_pair(newOpenTileVec, newNumOfRemainMines));
						} else {
							infoArry[newNumOfOpenTiles]->push_back(make_pair(newOpenTileVec, newNumOfRemainMines));
						}

						//Only go back when the information first vector got updated
						//So it's the case that some tiles been elminated already
						bool infoUpdated = false;
						if (newNumOfOpenTiles != info.first.size()) {
							infoUpdated = true;
						}

						//If the information got updated, and no available selection yet
						//Will need to go back in the array and update again
						//If already has something to select. No need to go back
						if (newNumOfOpenTiles <= numOfCoords &&
							infoUpdated &&
							pendingFlagQ.empty() &&
							pendingRevealQ.empty()) {
							numOfCoords = newNumOfOpenTiles - 1;
						}
					}
				}
			}
		}
	} 


	//bool choose = true;
	//
	//	
	//int newRow, newCol;

	//while (choose) {
	//	newRow = rand() % totalNumRow;
	//	newCol = rand() % totalNumCol;

	//	if (blockArry[newRow * totalNumCol + newCol] == OPEN_TILE) {
	//		choose = false;
	//		newCommand.tileCoord.x = newCol;
	//		newCommand.tileCoord.y = newRow;
	//	}
	//}

	//int flag = rand() % 2;
	//if (flag) {
	//	newCommand.commandExecution = CMD_EXEC_FLAG_ONE_TILE;
	//	blockArry[newRow * totalNumCol + newCol] = FLAGED_TILE;
	//} else {

	//	newCommand.commandExecution = CMD_EXEC_REVEAL_ONE_TILE;
	//}

	//Final decision rule:
	//Reveal more tiles first
	//If no available tiles immediately, flag one
	//If none of them available, randomly select one open tile
	if (!pendingRevealQ.empty()) {
		newCommand.commandExecution = CMD_EXEC_REVEAL_ONE_TILE;
		newCommand.tileCoord = pendingRevealQ.front();
		pendingRevealQ.pop();
	} else if (!pendingFlagQ.empty()) {
		newCommand.commandExecution = CMD_EXEC_FLAG_ONE_TILE;
		newCommand.tileCoord = pendingFlagQ.front();
		pendingFlagQ.pop();
	} else {
		//TODO If cannot find anything, can go deeper in simulation
		//TODO Random selection or select in the region
		newCommand.commandExecution = CMD_EXEC_REVEAL_ONE_TILE;
		bool choose = true;
		int newRandRow, newRandCol;

		while (choose) {
			newRandRow = rand() % totalNumRow;
			newRandCol = rand() % totalNumCol;

			if (blockArry[newRandRow * totalNumCol + newRandCol] == OPEN_TILE) {
				choose = false;
				newCommand.tileCoord.x = newRandCol;
				newCommand.tileCoord.y = newRandRow;
			}
		}
	}	

	return newCommand;
}
