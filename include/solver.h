#ifndef SOLVER_H
#define SOLVER_H

#include "defines.h"
#include <set>

void initialize_solver();
Command solver(vector<RevealedBlock> revealedBlocksVec, Command command);

#define OPEN_TILE	0xFF
#define FLAGED_TILE 0xCC

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

#endif
