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


void initialize_board() {
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
