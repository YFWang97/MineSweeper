#ifndef DEFINE_H
#define DEFINE_H

#include <SDL2/SDL.h>
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

#endif
