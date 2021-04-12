#include "solver.h"

int totalGameRun, totalWin;
int totalFlagged;

//blockArry stores the potential number of mines surrounding this tile.
//This number will differ from what AI initially sees.
//Every time AI flags one mine surrounding it, the number will deducted by one.
uint8_t* blockArry;
int totalNumRow, totalNumCol;

queue<Coordinate>* pendingFlagQ;
queue<Coordinate>* pendingRevealQ;

unordered_set<Coordinate> edgeTilesSet;

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

bool board_checker(vector<PotentialMineTile> potentialMineVec) {
	uint8_t* localBlockArry = (uint8_t*) malloc(totalNumRow * totalNumCol * sizeof(uint8_t));
	memcpy(localBlockArry, blockArry, totalNumRow * totalNumCol * sizeof(uint8_t));
	vector<Coordinate> affectedRevealedTilesVec;
	for (auto potentialMine : potentialMineVec) {
		localBlockArry[potentialMine.coord.y * totalNumCol + potentialMine.coord.x] = potentialMine.guessIsMine ? 0xEE : 0xE0;

		//Brodcase this guess to all surrounding tiles
		//Add surrounding tiles to the affectedRTilesVec
		//If the guess is mine then decrement
		for (int row = -1; row <= 1; row++) {
			int newRow = potentialMine.coord.y + row;
			if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
			for (int col = -1; col <= 1; col++) {
				int newCol = potentialMine.coord.x + col;
				if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
				uint8_t tileNum = localBlockArry[newRow * totalNumCol + newCol]; 
				//If the tile already has no potential mines surrounding
				//and the new coming guess is also a mine
				//it means this combination is impossible
				if (potentialMine.guessIsMine == true && tileNum == 0) {
					//cout << "MORE MINES FALSE" << endl;
					//cout << "on tile: " << newRow << " " << newCol << endl;
					//for (auto potentialMineTile : potentialMineVec) {
					//	cout << "Coord: " << potentialMineTile.coord.y << " " << potentialMineTile.coord.x << " ";
					//	cout << "Guess: " << potentialMineTile.guessIsMine << endl;
					//}
					delete (localBlockArry);
					return false;
				}
				//Don't care about the open tiles, flagged tiles and newly guessed tiles
				if ((tileNum & OPEN_TILE_MASK) == OPEN_TILE_MASK) continue;
				if (tileNum == FLAGGED_TILE) continue;
				if (tileNum == GUESS_NOT_MINE || tileNum == GUESS_MINE) continue;

				if (potentialMine.guessIsMine == true) {
					localBlockArry[newRow * totalNumCol + newCol] = tileNum - 1;
				}

				affectedRevealedTilesVec.push_back({newCol, newRow});
			}
		}	
	}
	//After this combination set
	//All the revealed tiles on the edge should have a potential mines of 0
	//If the potential mines is not 0
	//The acceptable case is that there are some more open tiles
	for (auto affectedRTile : affectedRevealedTilesVec) {
		uint8_t affectedRTileNum = localBlockArry[affectedRTile.y * totalNumCol + affectedRTile.x];
		uint8_t numOfRemainOpenTiles = 0;
		for (int row = -1; row <= 1; row++) {
			int newRow = affectedRTile.y + row;
			if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
			for (int col = -1; col <= 1; col++) {
				int newCol = affectedRTile.x + col;
				if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
				if ((localBlockArry[newRow * totalNumCol + newCol] & OPEN_TILE_MASK) == OPEN_TILE_MASK) {
					numOfRemainOpenTiles++;
				}
			}
		}	
		if (affectedRTileNum != 0 && affectedRTileNum > numOfRemainOpenTiles) {
			//cout << "NO MORE SPACE FOR MINE FALSE" << endl;
			//for (auto potentialMineTile : potentialMineVec) {
			//	cout << "Coord: " << potentialMineTile.coord.y << " " << potentialMineTile.coord.x << " ";
			//	cout << "Guess: " << potentialMineTile.guessIsMine << endl;
			//}
			delete (localBlockArry);
			return false;
		}
	}
	delete (localBlockArry);
	return true;
}

vector<Coordinate> openTileCoordsForTree;
vector<PotentialMineTile> potentialMineTileVecInTree;
int numOfMinesInCombInTree;
vector<vector<PotentialMineTile>> validMineTileCombinationVec;
void treeExpansion(int level, bool guess) {
	Coordinate curCoord = openTileCoordsForTree[level];
	PotentialMineTile curPotentialMineTile = {curCoord, guess};
	potentialMineTileVecInTree.push_back(curPotentialMineTile);
	if (guess == true) numOfMinesInCombInTree++;

	if (board_checker(potentialMineTileVecInTree)) {
		if (level == openTileCoordsForTree.size() - 1) {
			if ((mineTotal[gameLevel] - totalFlagged) >= numOfMinesInCombInTree) 
			{ 
				validMineTileCombinationVec.push_back(potentialMineTileVecInTree);
			}
			//cout << "VALID RESULT" << endl;
			//for (auto potentialMineTile : potentialMineTileVecInTree) {
			//	cout << "Coord: " << potentialMineTile.coord.y << " " << potentialMineTile.coord.x << " ";
			//	cout << "Guess: " << potentialMineTile.guessIsMine << endl;
			//}
			potentialMineTileVecInTree.pop_back();
			if (guess == true) numOfMinesInCombInTree--;
			return;
		} else {
			treeExpansion(level + 1, 0);
			treeExpansion(level + 1, 1);
		}
	}

	potentialMineTileVecInTree.pop_back();
	if (guess == true) numOfMinesInCombInTree--;
	return;
}	

void flagOneTile (Coordinate coord) {
	//Don't do repetitive flag. So skip when it's been flagged.
	if ((blockArry[coord.y * totalNumCol + coord.x] & OPEN_TILE_MASK) != OPEN_TILE_MASK) {
		return;
	}
	blockArry[coord.y * totalNumCol + coord.x] = FLAGGED_TILE;

	//cout << "TO BE FLAG: " << coord.y << " " << coord.x << endl;
	for (int row = -1; row <= 1; row++) {
		int newRow = coord.y + row;
		if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
		for (int col = -1; col <= 1; col++) {
			int newCol = coord.x + col;
			if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
			uint8_t tileNum = blockArry[newRow * totalNumCol + newCol]; 
			if (tileNum == 0) cout << "ERROR!!!!!!!!!!!!!!" << coord.y << " " << coord.x << endl;
			//If a tile is flagged, notify all the surrounding tiles
			//that they would expect a decrese of potential mine by 1
			//Even it's an open tile, need to record this
			blockArry[newRow * totalNumCol + newCol] = (tileNum == FLAGGED_TILE) ? FLAGGED_TILE : 
													   (tileNum - 1);
			//cout << "Row: " << newRow << " Col: " << newCol << " Tile Num: " << (int)tileNum-1 << endl;
		}
	}
	pendingFlagQ->push(coord);
	totalFlagged++;
	return;
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
			//gameStatus = STATUS_GAME_WAIT_TO_START;
			//newCommand.commandExecution |= CMD_EXEC_NEW_BOARD;
		}
		//if (totalFlagged > 10) {
		//	gameStatus = STATUS_GAME_WAIT_TO_START;
		//	newCommand.commandExecution |= CMD_EXEC_NEW_BOARD;
		//}
		//if (totalGameRun < 2) 
		{
			double winRate = (double) totalWin / totalGameRun;
			cout << "WIN RATE: " << winRate << " " << totalWin << " " << totalGameRun << " Flaged " << totalFlagged << endl;
			gameStatus = STATUS_GAME_WAIT_TO_START;
			newCommand.commandExecution |= CMD_EXEC_NEW_BOARD;
		}
		//In the case the game wins or over. Send the command to the board setting part
		//Reset boards and reset status to WAIT_TO_START
		//Don't give any tile selection
		return newCommand;
	}


	//First tile selection
	if ((gameStatus & STATUS_GAME_WAIT_TO_START) == STATUS_GAME_WAIT_TO_START) {
		gameStatus &= (~STATUS_GAME_WAIT_TO_START);

		totalGameRun++;
		//cout << "+++++++++++++++++++++ NEW GAME +++++++++++++++++++++++++" << endl;
		totalFlagged = 0;

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
		memset(blockArry, OPEN_TILE, totalNumRow * totalNumCol * sizeof(uint8_t));
		
		newCommand.commandExecution = CMD_EXEC_FIRST_REVEAL | CMD_EXEC_REVEAL_ONE_TILE;
		//newCommand.tileCoord.x = rand() % totalNumCol;
		//newCommand.tileCoord.y = rand() % totalNumRow;
		newCommand.tileCoord.x = totalNumCol / 2;
		newCommand.tileCoord.y = totalNumRow / 2;

		return newCommand;
	} 

	//General case selection

	if (revealedBlocksVec.size() > 0) {
		//Update the local block array first
		for (auto& revealedBlock: revealedBlocksVec) {
			int curRow = revealedBlock.coord.y;
			int curCol = revealedBlock.coord.x;
			uint8_t tileNum = blockArry[curRow * totalNumCol + curCol];
			if ((tileNum & OPEN_TILE_MASK) == OPEN_TILE_MASK) {
				blockArry[curRow * totalNumCol + curCol] = revealedBlock.num - (OPEN_TILE - tileNum);
			}
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
					if ((tileNum & OPEN_TILE_MASK) == OPEN_TILE_MASK) {
						openTileVec.push_back({newCol, newRow});
						numOfOpenTiles++;
						if (edgeTilesSet.find({newCol, newRow}) == edgeTilesSet.end()) {
							edgeTilesSet.insert({newCol, newRow});
							//cout << "Inserting: " << newRow << " " << newCol << endl;
						}
					}
					if (tileNum == FLAGGED_TILE) {
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
						flagOneTile(coord);
					}
				} else {
					if (info.coordVec.size() < info.numOfMines) cout << "ERROR" << endl;
					//In the case that vector size greater than remaining mines
					//Need to reconstruct the information struct.
					//If a tile is still open to flag, add it back to the vec
					//If it's flagged, update the remaining mines number and don't add the tile to vec
					vector<Coordinate> newOpenTileVec;
					uint8_t newNumOfRemainMines = info.numOfMines;
					for (auto const & coord : info.coordVec) {
						if ((blockArry[coord.y * totalNumCol + coord.x] & OPEN_TILE_MASK) == OPEN_TILE_MASK) {
							newOpenTileVec.push_back(coord);
						}
						if (blockArry[coord.y * totalNumCol + coord.x] == FLAGGED_TILE) {
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
	
	bool tankAlgo = true;
	if (pendingFlagQ->empty() && pendingRevealQ->empty() && tankAlgo) {
		//cout << "EDGE TILES" << endl;
		//for (auto& edgeTile : edgeTilesSet) {
		//	cout << "Row: " << edgeTile.y << " Col: " << edgeTile.x << endl;
		//}
		//cout << edgeTilesSet.size() << endl;
		//cout << "==================" << endl;
		
		//Seperate the edge tiles in regions
		//Tiles in one region will have correlations
		//Store in vector<vector<Coordinate>>
		//TODO Wrong Way to Separate Regions
		//May create more regions as regions will merge to one
		
		unordered_map<Coordinate, vector<Coordinate>> edgeTileToRTileCorrelationMap;
		unordered_map<Coordinate, vector<Coordinate>> RTileToEdgeTileCorrelationMap;
		
		for (auto& edgeTile : edgeTilesSet) {
			vector<Coordinate> RTiles;
			for (int row = -1; row <= 1; row++) {
				int newRow = edgeTile.y + row;
				if (newRow < 0 || newRow >= levelSize[gameLevel].first) continue;
				for (int col = -1; col <= 1; col++) {
					int newCol = edgeTile.x + col;
					if (newCol < 0 || newCol >= levelSize[gameLevel].second) continue;
					uint8_t tileNum = blockArry[newRow * totalNumCol + newCol]; 
					if ((tileNum & OPEN_TILE_MASK) == OPEN_TILE_MASK) continue;
					if (tileNum == FLAGGED_TILE) continue;
					Coordinate RTile = {newCol, newRow};
					RTiles.push_back(RTile);

					if (RTileToEdgeTileCorrelationMap.find(RTile) != RTileToEdgeTileCorrelationMap.end()) {
						RTileToEdgeTileCorrelationMap[RTile].push_back(edgeTile);
					} else {
						vector<Coordinate> tmpVec;
						tmpVec.push_back(edgeTile);
						RTileToEdgeTileCorrelationMap[RTile] = tmpVec;
					}
				}
			}

			edgeTileToRTileCorrelationMap[edgeTile] = RTiles;
		}

		vector<vector<Coordinate>> regionEdgeTilesVec;
		queue<Coordinate> regionSearchQ;
		unordered_set<Coordinate> visitedEdgeTilesSet;
		vector<Coordinate> newRegionVec;
		while (edgeTileToRTileCorrelationMap.size() > 0) {
			if (regionSearchQ.empty()) {
				Coordinate firstEdgeTile = edgeTileToRTileCorrelationMap.begin()->first;
				regionSearchQ.push(firstEdgeTile);
				newRegionVec.clear();
				visitedEdgeTilesSet.insert(firstEdgeTile);
			}

			while (regionSearchQ.size() > 0) {
				Coordinate curEdgeTile = regionSearchQ.front();
				vector<Coordinate> correlatedRTiles = edgeTileToRTileCorrelationMap[curEdgeTile];
				edgeTileToRTileCorrelationMap.erase(curEdgeTile);
				regionSearchQ.pop();
				newRegionVec.push_back(curEdgeTile);					

				for (auto RTile : correlatedRTiles) {
					if (RTileToEdgeTileCorrelationMap.find(RTile) != RTileToEdgeTileCorrelationMap.end()) {
						for (auto correlatedEdgeTile : RTileToEdgeTileCorrelationMap[RTile]) {
							if (visitedEdgeTilesSet.find(correlatedEdgeTile) == visitedEdgeTilesSet.end()) {
								regionSearchQ.push(correlatedEdgeTile);
								visitedEdgeTilesSet.insert(correlatedEdgeTile);
							}
						}
					}
				}
			}
			if (newRegionVec.size() > 0) {
				regionEdgeTilesVec.push_back(newRegionVec);
			}
		}

		//cout << "REGION NUM" << regionEdgeTilesVec.size() << endl;

		//Build up a tree based on the edgeTileSet.
		//Tree nodes are the coordinate and if it is a mine

		double curMinPossibilityOfMine = 1.0; 
		Coordinate minTimesOfValidCoord;
		for (auto regionVec : regionEdgeTilesVec) {

			openTileCoordsForTree.clear();
			potentialMineTileVecInTree.clear();
			for (auto& tmpVec : validMineTileCombinationVec) {
				tmpVec.clear();
			}
			validMineTileCombinationVec.clear();

			openTileCoordsForTree = regionVec;

			if (openTileCoordsForTree.size() > 0) {
				numOfMinesInCombInTree = 0;
				treeExpansion(0, 0);
				treeExpansion(0, 1);
				//cout << "NUM OF VALID COMBS: " << validMineTileCombinationVec.size() << endl;

				if (validMineTileCombinationVec.size() > 0) {

					//Count how many times each tiles can be a mine
					unordered_map<Coordinate, uint16_t> validMineTileTimesMap;

					for (auto tile : regionVec) {
						validMineTileTimesMap.insert(make_pair(tile, 0));	
					}

					for (auto combination : validMineTileCombinationVec) {
						for (auto guessTile : combination) {
							if (guessTile.guessIsMine) {
								validMineTileTimesMap[guessTile.coord]++;
								//printf("Increment a count %d, %d: %d\n", guessTile.coord.y, guessTile.coord.x, validMineTileTimesMap[guessTile.coord]);
							}
						}	
					}	

					for (auto tile : validMineTileTimesMap) {
						//printf("Tile Coord: %d, %d Tile Guess Mine Times: %d\n", tile.first.y, tile.first.x, (int)tile.second);
						double possibilityOfMine = (double) tile.second / (double) validMineTileCombinationVec.size();
						if (possibilityOfMine == 0) {
							pendingRevealQ->push(tile.first);
							//cout << "Certain selection of save tile from tank algo: " << tile.first.y << " " << tile.first.x << endl;
						} else if (possibilityOfMine == 1.0) {
							//cout << "Certain selection of mine from tank algo: " << tile.first.y << " " << tile.first.x << endl;
							flagOneTile(tile.first);
						} else if (possibilityOfMine < curMinPossibilityOfMine) {
							minTimesOfValidCoord = tile.first;
							curMinPossibilityOfMine = possibilityOfMine;
						}
					}
				}
			}
		}
		//cout << "Cur min: " << curMinPossibilityOfMine << endl;
		//If cannot find any suitable mine or save tiles
		//Select the least possible tile
		if (pendingFlagQ->empty() && pendingRevealQ->empty()) {
			if (curMinPossibilityOfMine != 1.0) {
				pendingRevealQ->push(minTimesOfValidCoord);
				//cout << "GUESS from tank algo: " << minTimesOfValidCoord.y << " " << minTimesOfValidCoord.x << endl;
				//printf("Appearance is %d, total valid is %d\n", curMinPossibilityOfMine, validMineTileCombinationVec.size());
			}
		}
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
			//TODO If cannot find anything, can go deeper in simulation
			//TODO Random selection or select in the region
			bool choose = true;
			int newRandRow, newRandCol;
			Coordinate randCoord;

			while (choose) {
				newRandRow = rand() % totalNumRow;
				newRandCol = rand() % totalNumCol;

				if ((blockArry[newRandRow * totalNumCol + newRandCol] & OPEN_TILE_MASK) == OPEN_TILE_MASK) {
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
	}	
	return newCommand;
}
