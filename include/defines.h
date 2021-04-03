#ifndef DEFINE_H
#define DEFINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#if __linux__
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#elif __APPLE__
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#endif
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>
#include <utility>

using namespace std;

#define SDL_ERROR_MSG(...) cout << __VA_ARGS__ << "! " << "SDL_Error: " << SDL_GetError() << endl
#define ERROR_MSG(...) cout << __VA_ARGS__ << "! " << endl
#define DEBUG_MSG(...) cout << "DEBUG INFO: " << __VA_ARGS__ << "! " << endl

#define SCREEN_WIDTH 440
#define SCREEN_HEIGHT 320

#define MAP_DIMENSION 20

#define BLOCK_WIDTH 20

#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 25

#define NUM_OF_BUTTONS 4

enum GameLevel {
	EASY,
	MEDIUM,
	HARD,
	RESET
};

typedef struct coordinate {
	int x;
	int y;
	struct coordinate translate() {
		int newX = floor((double)x / BLOCK_WIDTH);
		int newY = floor((double)y / BLOCK_WIDTH);
		return {newX, newY};
	}
	bool operator==(const struct coordinate& coord) {
		return (x == coord.x && y == coord.y);
	}
	bool operator<(const struct coordinate& coord) const {
		if (y < coord.y) return true;
		if (y == coord.y && x < coord.x) return true;
		return false;
	}
	bool operator!=(const struct coordinate& coord) {
		return (x != coord.x || y != coord.y);
	}
} Coordinate;

typedef struct mouseEvent {
	Coordinate coord;
	uint32_t state;
	uint32_t type;
	friend ostream& operator<<(ostream&os, const struct mouseEvent& e) {
		os << "Coord: x:" << std::dec << e.coord.x << ", y:" << e.coord.y << endl;
		os << "State: " << e.state << endl;
		os << "Type: " << std::hex << e.type << std::dec << endl;
		return os;
	}
} MouseEvent;

typedef struct command {
	Coordinate tileCoord;
	Coordinate mouseCoord;
	uint8_t commandExecution;
	uint8_t commandDisplay;
	friend ostream& operator<<(ostream&os, const struct command& c) {
		os << "Tile Coord: x:" << std::dec << c.tileCoord.x << ", y:" << c.tileCoord.y << endl;
		os << "Mouse Coord: x:" << std::dec << c.mouseCoord.x << ", y:" << c.mouseCoord.y << endl;
		os << "EXEC: " << unsigned(c.commandExecution) << endl;
		os << "DISP: " << unsigned(c.commandDisplay) << endl;
		return os;
	}
} Command;

//Simplified version of the block info
typedef struct revealedBlock {
	Coordinate coord;
	int num;
} RevealedBlock;

extern SDL_Renderer* gRenderer;
extern SDL_Window* gWindow;
extern SDL_Texture* mineTexture;
extern SDL_Texture* mineRevealTexture;
extern SDL_Texture* mineWrongTexture;
extern SDL_Texture* plainTexture;
extern SDL_Texture* flagTexture;
extern SDL_Texture* baseTexture;
extern SDL_Texture* overTexture;
extern SDL_Texture* winTexture;
extern SDL_Texture* numTexture[8];
extern SDL_Texture* selectedTexture;
extern SDL_Texture* releasedTexture;
extern SDL_Texture* flagedNumTexture;
extern SDL_Texture* blockHighlightTexture;

////////// GLOBAL VARIABLES //////////

extern bool gameQuit;
extern uint8_t gameStatus;
extern pair<int, int> levelSize[NUM_OF_BUTTONS - 1];
extern int gameLevel;


////////// GAME STATUS DEFINES //////////
#define STATUS_GAME_OVER			0b0001
#define STATUS_GAME_WIN				0b0010
#define STATUS_GAME_WAIT_TO_START	0b0100

////////// COMMANDS DEFINES //////////

//Mouse Commands - Execution
//These signals are asserted by mouse processing
//and deasserted by action logic after action performed

//Mouse Commands - Display
//These signals are asserted and deasserted by mouse processing
//Since they control display unit directly

#define CMD_EXEC_REVEAL_ONE_TILE	0b00000001
#define CMD_EXEC_FLAG_ONE_TILE		0b00000010
#define CMD_EXEC_REVEAL_SURR_TILES	0b00000100
#define CMD_EXEC_FIRST_REVEAL		0b00001000
#define CMD_EXEC_BUTTON_ACTION		0b00010000
#define CMD_EXEC_NEW_BOARD			0b00100000

#define CMD_DISP_GRAY_ONE_TILE		0b00000001
#define CMD_DISP_GRAY_SURR_TILES	0b00000010

//Both real target changes or a click on a target can assert this signal
//This signal controls when we gray tiles
#define CMD_DISP_TARGET_CHANGED		0b00000100

#endif
