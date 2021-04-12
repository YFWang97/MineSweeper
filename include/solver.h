#ifndef SOLVER_H
#define SOLVER_H

#include "defines.h"

void initialize_solver();
Command solver(vector<RevealedBlock> revealedBlocksVec, Command command);

//Initial OPEN_TILE is 0XFF
//However all 0xFx can be considered as open tile
//As when it has some flags in the surrounding tiles
//the potential is decreased even is not opened
//When the tile actually opens, this potential decrese will be
//translated to the new potential number 

#define OPEN_TILE		0xFF
#define OPEN_TILE_MASK	0xF0
#define FLAGGED_TILE	0xCC
#define GUESS_NOT_MINE  0xE0
#define GUESS_MINE		0xEE

typedef struct information {
	vector<Coordinate> coordVec;
	uint8_t numOfMines;

	information(vector<Coordinate> coordVec_, uint8_t numOfMines_) {
		coordVec = coordVec_;
		numOfMines = numOfMines_;
	}

	information() {};

	bool operator==(const struct information& info) {
		if (numOfMines != info.numOfMines) {
			return false;
		}
		if (coordVec.size() != info.coordVec.size()) {
			return false;
		}
		for (int i = 0; i < coordVec.size(); i++) {
			if (coordVec[i] != info.coordVec[i]) {
				return false;
			}
		}
		return true;
	}
} Information;

typedef struct potentialMineTile {
	Coordinate coord;
	bool guessIsMine;
} PotentialMineTile;

/* Based on the current board situation
 * check if the input mine vector is a valid combination
 */
bool board_checker(vector<PotentialMineTile> potentialMineVec);

#endif
