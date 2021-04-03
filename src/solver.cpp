#include "solver.h"

int totalGameRun, totalWin;
int totalFlaged;

uint8_t* blockArry;
int totalNumRow, totalNumCol;

queue<Coordinate>* pendingFlagQ;
queue<Coordinate>* pendingRevealQ;

set<Coordinate> edgeTilesSet;

//Push the information in the queue
//For the trivial ones (remaining open tiles == remaining mines) push front
//Process these ones first
deque<Information>* infoArry[9];

void initialize_solver() {

	totalNumRow = levelSize[gameLevel].first;
	totalNumCol = levelSize[gameLevel].second;
	blockArry = (uint8_t*) malloc(totalNumRow * totalNumCol * sizeof(uint8_t));
	for (int i = 0; i <= 8; i++) {
		infoArry[i] = new(deque<Information>);
	}

	pendingFlagQ = new(queue<Coordinate>);
	pendingRevealQ = new(queue<Coordinate>);
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
			//cout << "WIN A GAME" << endl;
			totalWin++;
			gameStatus = STATUS_GAME_WAIT_TO_START;
			newCommand.commandExecution |= CMD_EXEC_NEW_BOARD;
		}
		//if (totalFlaged > 10) {
		//	gameStatus = STATUS_GAME_WAIT_TO_START;
		//	newCommand.commandExecution |= CMD_EXEC_NEW_BOARD;
		//}
		//double winRate = (double) totalWin / totalGameRun;
		//cout << "WIN RATE: " << winRate << " " << totalWin << " " << totalGameRun << " Flaged " << totalFlaged << endl;
		//gameStatus = STATUS_GAME_WAIT_TO_START;
		//newCommand.commandExecution |= CMD_EXEC_NEW_BOARD;
		//In the case the game wins or over. Send the command to the board setting part
		//Reset boards and reset status to WAIT_TO_START
		//Don't give any tile selection
		return newCommand;
	}


	//First tile selection
	if ((gameStatus & STATUS_GAME_WAIT_TO_START) == STATUS_GAME_WAIT_TO_START) {
		gameStatus &= (~STATUS_GAME_WAIT_TO_START);

		totalGameRun++;
		totalFlaged = 0;

		for (int i = 0; i <= 8; i++) {
			delete(infoArry[i]);
			infoArry[i] = new(deque<Information>);
		}

		delete(pendingFlagQ);
		delete(pendingRevealQ);

		pendingFlagQ = new(queue<Coordinate>);
		pendingRevealQ = new(queue<Coordinate>);

		free(blockArry);

		edgeTilesSet.clear();

		totalNumRow = levelSize[gameLevel].first;
		totalNumCol = levelSize[gameLevel].second;

		blockArry = (uint8_t*) malloc(totalNumRow * totalNumCol * sizeof(uint8_t));
		memset(blockArry, 0xFF, totalNumRow * totalNumCol * sizeof(uint8_t));
		
		newCommand.commandExecution = CMD_EXEC_FIRST_REVEAL | CMD_EXEC_REVEAL_ONE_TILE;
		newCommand.tileCoord.x = rand() % totalNumCol;
		newCommand.tileCoord.y = rand() % totalNumRow;

		return newCommand;
	} 

	//General case selection

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

			if (edgeTilesSet.find({curCol, curRow}) != edgeTilesSet.end()) {
				edgeTilesSet.erase({curCol, curRow});
				//cout << "Erasing just newly revealed: " << curRow << " " << curCol << endl;
			}

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
						if (edgeTilesSet.find({newCol, newRow}) == edgeTilesSet.end()) {
							edgeTilesSet.insert({newCol, newRow});
							//cout << "Inserting: " << newRow << " " << newCol << endl;
						}
					}
					if (tileNum == FLAGED_TILE) {
						numOfRemainMines--;
					}
				}
			}
			if (numOfOpenTiles > 0) {
				Information newInfo(openTileVec, numOfRemainMines);
				if (find(infoArry[numOfOpenTiles]->begin(), infoArry[numOfOpenTiles]->end(), newInfo) == infoArry[numOfOpenTiles]->end()) {
					if (numOfOpenTiles == numOfRemainMines) {
						infoArry[numOfOpenTiles]->push_front(newInfo);
					} else {
						infoArry[numOfOpenTiles]->push_back(newInfo);
					}
				}
			}
		}
	}	
	
	int numOfCoords;
	int updatedNumOfCoords = 8;
	bool infoUpdated = false;
	for (numOfCoords = 1; numOfCoords <= 8; numOfCoords++) {
		deque<Information>* infoQ = infoArry[numOfCoords];
		int numOfInfo = infoQ->size();
		for (int infoIndex = 0; infoIndex < numOfInfo; infoIndex++) {
			Information info = infoQ->front();
			infoQ->pop_front();	
			//If the remaining mines becomes 0, it's safe to open all the tiles in the vector
			if (info.numOfMines == 0) {
				for (auto const & coord : info.coordVec) {
					pendingRevealQ->push(coord);
				}
			} else { 
				//If the remaining mines == size of vector
				//It means all the tiles in the vector are mines
				if (info.coordVec.size() == info.numOfMines) {
					for (auto const & coord : info.coordVec) {
						//Don't do repetitive flag. So skip when it's been flaged.
						if (blockArry[coord.y * totalNumCol + coord.x] != OPEN_TILE) {
							continue;
						}
						blockArry[coord.y * totalNumCol + coord.x] = FLAGED_TILE;
						pendingFlagQ->push(coord);
						totalFlaged++;
					}
				} else {
					if (info.coordVec.size() < info.numOfMines) cout << "ERROR" << endl;
					//In the case that vector size greater than remaining mines
					//Need to reconstruct the information struct.
					//If a tile is still open to flag, add it back to the vec
					//If it's flaged, update the remaining mines number and don't add the tile to vec
					vector<Coordinate> newOpenTileVec;
					uint8_t newNumOfRemainMines = info.numOfMines;
					for (auto const & coord : info.coordVec) {
						if (blockArry[coord.y * totalNumCol + coord.x] == OPEN_TILE) {
							newOpenTileVec.push_back(coord);
						}
						if (blockArry[coord.y * totalNumCol + coord.x] == FLAGED_TILE) {
							newNumOfRemainMines--;
						}

					}
					uint8_t newNumOfOpenTiles = newOpenTileVec.size();
					if (newNumOfOpenTiles > 0) {
						Information newInfo(newOpenTileVec, newNumOfRemainMines);
						if (find(infoArry[newNumOfOpenTiles]->begin(), infoArry[newNumOfOpenTiles]->end(), newInfo) == infoArry[newNumOfOpenTiles]->end()) {
							if (newNumOfOpenTiles == newNumOfRemainMines) {
								infoArry[newNumOfOpenTiles]->push_front(newInfo);
							} else {
								infoArry[newNumOfOpenTiles]->push_back(newInfo);
							}
						}

						//Only go back when the information first vector got updated
						//and the new information maybe has less open tiles
						//So it's the case that some tiles been elminated already
						if (newNumOfOpenTiles != info.coordVec.size() &&
							newNumOfOpenTiles - 1 < updatedNumOfCoords) {
							infoUpdated = true;
							updatedNumOfCoords = newNumOfOpenTiles - 1;
						}
					}
				}
			}
		}
		//If the information got updated, and no available selection yet
		//Will need to go back in the array and update again
		//If already has something to select. No need to go back
		if (numOfCoords == 8 &&
			pendingFlagQ->empty() &&
			pendingRevealQ->empty()) {
			if (infoUpdated) {
				numOfCoords = updatedNumOfCoords;
				//After the first loop back, need to reset infoUpdated and updatedNumOfCoords.
				//Otherwise will get into an infinite loop
				infoUpdated = false;
				updatedNumOfCoords = 8;
			} else { 
				//In this case no new information updated by trivial elimination
				//Need to use information to deduce with other info
				//Check for superset for this information
				//Only check when the number of coords is less than 5.
				//It's only possible to have a overlap
				for (int comp1NumOfCoords = 2; comp1NumOfCoords <= 4; comp1NumOfCoords++) {
					deque<Information>* comp1InfoQ = infoArry[comp1NumOfCoords];
					int comp1NumOfInfo = comp1InfoQ->size();
					for (int comp1InfoIndex = 0; comp1InfoIndex < comp1NumOfInfo; comp1InfoIndex++) {
						Information comp1Info = comp1InfoQ->front();
						comp1InfoQ->pop_front();
						for (int comp2NumOfCoords = comp1NumOfCoords + 1; comp2NumOfCoords <=8; comp2NumOfCoords++) {
							deque<Information>* comp2InfoQ = infoArry[comp2NumOfCoords];
							int comp2NumOfInfo = comp2InfoQ->size();
							for (int comp2InfoIndex = 0; comp2InfoIndex < comp2NumOfInfo; comp2InfoIndex++) {
								//cout << "Comp1 " << comp1NumOfCoords << " Comp1Index " << comp1InfoIndex << " ";
							    //cout << "Comp2 " << comp2NumOfCoords << " Comp2Index " << comp2InfoIndex;
								//cout << endl;	
								Information comp2Info = comp2InfoQ->front();
								comp2InfoQ->pop_front();	
								
								int comp1InfoSmallestCoordArryIndex = comp1Info.coordVec.front().y * totalNumCol + comp1Info.coordVec.front().x;
								int comp1InfoLargestCoordArryIndex = comp1Info.coordVec.back().y * totalNumCol + comp1Info.coordVec.back().x;
								int comp2InfoSmallestCoordArryIndex = comp2Info.coordVec.front().y * totalNumCol + comp2Info.coordVec.front().x;
								int comp2InfoLargestCoordArryIndex = comp2Info.coordVec.back().y * totalNumCol + comp2Info.coordVec.back().x;

								//cout << "COMP1: " << unsigned(comp1Info.numOfMines) << " ";
								//for (auto& coord : comp1Info.coordVec) {
								//	cout << coord.y << "," << coord.x << " ";
								//}
								//cout << endl;
								//cout << "COMP2: " << unsigned(comp2Info.numOfMines) << " ";
								//for (auto& coord : comp2Info.coordVec) {
								//	cout << coord.y << "," << coord.x << " ";
								//}
								//cout << endl;

								//If current information is impossible to be a subset of the comparing info
								//Push back the comparing info and continue
								if ((comp1InfoSmallestCoordArryIndex < comp2InfoSmallestCoordArryIndex) ||
									(comp1InfoLargestCoordArryIndex > comp2InfoLargestCoordArryIndex)) {
									comp2InfoQ->push_back(comp2Info);
									continue;
								}	

								int comp1InfoCoordIndex = 0;
								int comp2InfoCoordIndex = 0;

								int infoNumToCompare = comp1Info.coordVec.size();
								vector<Coordinate> diffOpenTileVec;

								while (comp2InfoCoordIndex < comp2Info.coordVec.size()) {
									Coordinate comp1InfoCurCoord = comp1Info.coordVec[comp1InfoCoordIndex];
									Coordinate comp2InfoCurCoord = comp2Info.coordVec[comp2InfoCoordIndex];
									int comp1InfoCurCoordArryIndex = comp1InfoCurCoord.y * totalNumCol + comp1InfoCurCoord.x;
									int comp2InfoCurCoordArryIndex = comp2InfoCurCoord.y * totalNumCol + comp2InfoCurCoord.x;
									if (comp1InfoCurCoordArryIndex == comp2InfoCurCoordArryIndex) {
										if (comp1InfoCoordIndex < comp1Info.coordVec.size() - 1) {
											comp1InfoCoordIndex++;
										}
										comp2InfoCoordIndex++;
										infoNumToCompare--;
									} else if (comp1InfoCurCoordArryIndex > comp2InfoCurCoordArryIndex) {
										diffOpenTileVec.push_back(comp2InfoCurCoord);
										comp2InfoCoordIndex++;	
									} else {
										//infoCurCoordArryIndex < compInfoCurCoordArryIndex
										//Means compInfoCurCoordArryIndex already exceeds infoCurCoordArryIndex
										//First case is all the info find a compInfo
										//Need to collect all the remaining compInfo as diff
										//Second case is couldn't find all the compInfo  
										//There is no way that a subset can still form
										if (infoNumToCompare == 0) {
											diffOpenTileVec.push_back(comp2InfoCurCoord);
											comp2InfoCoordIndex++;	
										} else {
											break;
										}
									}
								}

								if (infoNumToCompare == 0) {
									int compNewNumOfRemainMines = comp2Info.numOfMines - comp1Info.numOfMines;	
									int compNewNumOfOpenTiles = diffOpenTileVec.size();
									if (compNewNumOfOpenTiles == 0) cout << "compNewNumOfOpenTiles 0" << endl;
										
									//cout << "DIFF: " << compNewNumOfOpenTiles << " ";
									//for (auto& coord : diffOpenTileVec) {
									//	cout << coord.y << "," << coord.x << " ";
									//}
									//cout << endl;
									Information newInfo(diffOpenTileVec, compNewNumOfRemainMines);
									if (find(infoArry[compNewNumOfOpenTiles]->begin(), infoArry[compNewNumOfOpenTiles]->end(), newInfo) == infoArry[compNewNumOfOpenTiles]->end()) {
										if (compNewNumOfOpenTiles == compNewNumOfRemainMines) {
											infoArry[compNewNumOfOpenTiles]->push_front(newInfo);
										} else {
											infoArry[compNewNumOfOpenTiles]->push_back(newInfo);
										}
									}

									if (compNewNumOfOpenTiles - 1 < updatedNumOfCoords) {
										infoUpdated = true;
										updatedNumOfCoords = compNewNumOfOpenTiles - 1;
									}
								} else {
									comp2InfoQ->push_back(comp2Info);
								}
							}
						}
						comp1InfoQ->push_back(comp1Info);
					}
				}
				if (infoUpdated) {
					numOfCoords = updatedNumOfCoords;
					//After the first loop back, need to reset infoUpdated and updatedNumOfCoords.
					//Otherwise will get into an infinite loop
					infoUpdated = false;
					updatedNumOfCoords = 8;
				}
			}
		}
	}

	if (pendingFlagQ->empty() && pendingRevealQ->empty()) {
		//cout << "EDGE TILES" << endl;
		//for (auto& edgeTile : edgeTilesSet) {
		//	cout << "Row: " << edgeTile.y << " Col: " << edgeTile.x << endl;
		//}
		//cout << edgeTilesSet.size() << endl;
		//cout << "==================" << endl;
	}

	if (pendingFlagQ->empty() && pendingRevealQ->empty()) {
		double lowestRateToBeMine = 1.0;
		Information lowestRateInfo;
		bool allQEmpty = true;
		for (numOfCoords = 1; numOfCoords <= 8; numOfCoords++) {
			deque<Information>* infoQ = infoArry[numOfCoords];
			int numOfInfo = infoQ->size();
			if (numOfInfo > 0) allQEmpty = false;
			for (int infoIndex = 0; infoIndex < numOfInfo; infoIndex++) {
				Information info = infoQ->front();
				infoQ->pop_front();	
				double rateToBeMine = (double)info.numOfMines / info.coordVec.size();
				//cout << "RATE" << rateToBeMine << endl;
				if (rateToBeMine < lowestRateToBeMine) {
					lowestRateInfo = info;
					lowestRateToBeMine = rateToBeMine;
				}
				infoQ->push_back(info);
			}
		}
		if (!allQEmpty) {
			int randCoordIndex = rand() % lowestRateInfo.coordVec.size();
			pendingRevealQ->push(lowestRateInfo.coordVec[randCoordIndex]);
		} else {
			bool choose = true;
			int newRandRow, newRandCol;
			Coordinate randCoord;

			while (choose) {
				newRandRow = rand() % totalNumRow;
				newRandCol = rand() % totalNumCol;

				if (blockArry[newRandRow * totalNumCol + newRandCol] == OPEN_TILE) {
					choose = false;
					randCoord.x = newRandCol;
					randCoord.y = newRandRow;
				}
			}
			pendingRevealQ->push(randCoord);
		}
	}

	//Final decision rule:
	//Reveal more tiles first
	//If no available tiles immediately, flag one
	//If none of them available, randomly select one open tile
//	if (!pendingRevealQ->empty()) {
//		newCommand.commandExecution = CMD_EXEC_REVEAL_ONE_TILE;
//		newCommand.tileCoord = pendingRevealQ->front();
//		pendingRevealQ->pop();
//		if (edgeTilesSet.find(newCommand.tileCoord) != edgeTilesSet.end()) 
//			edgeTilesSet.erase(newCommand.tileCoord);
//	} else if (!pendingFlagQ->empty()) {
//		newCommand.commandExecution = CMD_EXEC_FLAG_ONE_TILE;
//		newCommand.tileCoord = pendingFlagQ->front();
//		pendingFlagQ->pop();
//		if (edgeTilesSet.find(newCommand.tileCoord) != edgeTilesSet.end()) 
//			edgeTilesSet.erase(newCommand.tileCoord);
	if (!pendingFlagQ->empty()) {
		newCommand.commandExecution = CMD_EXEC_FLAG_ONE_TILE;
		newCommand.tileCoord = pendingFlagQ->front();
		pendingFlagQ->pop();
		if (edgeTilesSet.find(newCommand.tileCoord) != edgeTilesSet.end()) {
			edgeTilesSet.erase(newCommand.tileCoord);
			//cout << "Erasing flag q: " << newCommand.tileCoord.y << " " << newCommand.tileCoord.x << endl;
		}
	} else if (!pendingRevealQ->empty()) {
		newCommand.commandExecution = CMD_EXEC_REVEAL_ONE_TILE;
		newCommand.tileCoord = pendingRevealQ->front();
		pendingRevealQ->pop();
		if (edgeTilesSet.find(newCommand.tileCoord) != edgeTilesSet.end()) {
			edgeTilesSet.erase(newCommand.tileCoord);
			//cout << "Erasing reaveal q: " << newCommand.tileCoord.y << " " << newCommand.tileCoord.x << endl;
		}
	} else {
		bool error = false;
		for (int i = 0; i <= 8; i++) {
			for (auto& info : *infoArry[i]) {
				if (info.coordVec.size() == info.numOfMines) error = true;
			}
		}
		if (error) {
		cout << "HAS" << infoUpdated << endl;
		cout << "FQ" << pendingFlagQ->empty() << endl;
	    cout << "RQ" << pendingRevealQ->empty() << endl;	
		for (int i = 0; i <= 8; i++) {
			cout << i << endl;
			for (auto& info : *infoArry[i]) {
				cout << unsigned(info.numOfMines) << " ";
				for (auto& coord : info.coordVec) {
					cout << coord.y+1 << "," << coord.x+1 << " ";
				}
			}
			cout << endl;
		}
		cout << "=====================" << endl;
		}
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
