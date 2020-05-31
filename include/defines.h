#ifndef DEFINE_H
#define DEFINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <unordered_map>
#include <unordered_set>
#include <queue>

using namespace std;

#define SDL_ERROR_MSG(...) cout << __VA_ARGS__ << "! " << "SDL_Error: " << SDL_GetError() << endl
#define ERROR_MSG(...) cout << __VA_ARGS__ << "! " << endl
#define DEBUG_MSG(...) cout << "DEBUG INFO: " << __VA_ARGS__ << "! " << endl

typedef struct coordinate {
	int x;
	int y;
} Coordinate;

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 400

#define BLOCK_WIDTH 20

#define BUTTON_WIDTH 80
#define BUTTON_HEIGHT 25

extern SDL_Renderer* gRenderer;
extern SDL_Texture* mineTexture;
extern SDL_Texture* plainTexture;
extern SDL_Texture* flagTexture;
extern SDL_Texture* baseTexture;
extern SDL_Texture* overTexture;
extern SDL_Texture* winTexture;
extern SDL_Texture* numTexture[8];
extern SDL_Texture* selectedTexture;
extern SDL_Texture* releasedTexture;

#endif
