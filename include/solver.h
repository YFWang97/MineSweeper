#ifndef SOLVER_H
#define SOLVER_H

#include "defines.h"

void initialize_solver();
Command solver(vector<RevealedBlock> revealedBlocksVec, Command command);

#define OPEN_TILE	0xFF
#define FLAGED_TILE 0xCC

#endif
