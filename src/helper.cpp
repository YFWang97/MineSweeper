#include "helper.h"

SDL_Renderer* gRenderer;
SDL_Window* gWindow;
TTF_Font* gFont = NULL;

SDL_Texture* mineTexture;
SDL_Texture* mineRevealTexture;
SDL_Texture* mineWrongTexture;
SDL_Texture* plainTexture;
SDL_Texture* flagTexture;
SDL_Texture* baseTexture;
SDL_Texture* overTexture;
SDL_Texture* winTexture;
SDL_Texture* numTexture[8];
SDL_Texture* selectedTexture;
SDL_Texture* releasedTexture;

int initialize_sdl() {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_ERROR_MSG("SDL: Failed to initialize");
		return -1;
	}

	int flags=IMG_INIT_JPG|IMG_INIT_PNG;
	if (IMG_Init(flags)&flags != flags) {
		SDL_ERROR_MSG("IMG INIT: Failed to initialize jpg and png support");
		return -1;
	}

	gWindow = SDL_CreateWindow("GAME", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (gWindow == NULL) {
		SDL_ERROR_MSG("SDL: Failed to create window");
		return -1;	
	} 
	
	// Get the current display attributes 
	SDL_DisplayMode current;

	SDL_GetCurrentDisplayMode(0, &current);

	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	//gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if (gRenderer == NULL) {
		SDL_ERROR_MSG("SDL: Failed to create renderer");
		return -1;
	}

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0x00);

    if (TTF_Init() == -1) {
        SDL_ERROR_MSG("SDL: Failed to initialize ttf");
        return -1;
    }

    gFont = TTF_OpenFont("font/Ubuntu-L.ttf", 15);

    if (gFont == NULL) {
        SDL_ERROR_MSG("TTF: Failed to load font");
        return -1;
    }

    return 0;
}

int initialize_img() {
    plainTexture = load_img("img/plain.png");
    if (plainTexture == NULL) {return -1;}

    mineTexture = load_img("img/mine.png");
    if (mineTexture == NULL) {return -1;}

    mineRevealTexture = load_img("img/mineReveal.png");
    if (mineRevealTexture == NULL) {return -1;}

    mineWrongTexture = load_img("img/mineWrong.png");
    if (mineWrongTexture == NULL) {return -1;}

    flagTexture = load_img("img/flag.png");
    if (flagTexture == NULL) {return -1;}

    baseTexture = load_img("img/base.png");
    if (baseTexture == NULL) {return -1;}

    overTexture = load_img("img/over.png");
    if (overTexture == NULL) {return -1;}

    winTexture = load_img("img/win.png");
    if (winTexture == NULL) {return -1;}

    selectedTexture = load_img("img/selected.png");
    if (selectedTexture == NULL) {return -1;}
    
    releasedTexture = load_img("img/released.png");
    if (releasedTexture == NULL) {return -1;}

    if (load_num()) {return -1;}
    return 0;
}

SDL_Texture* load_img(char* name) {
    SDL_Surface* surface = IMG_Load(name);

    if (surface == NULL) {
        SDL_ERROR_MSG("IMG: Failed to load image to surface");
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface);

    if (texture == NULL) {
        SDL_ERROR_MSG("SDL: Failed to crate texture from surface");
    }

    SDL_FreeSurface(surface);

    return texture;
}

int load_num() {
    for (int i = 0; i < 8; i++) {

        numTexture[i] = load_text(to_string(i + 1).c_str());

        if (numTexture[i] == NULL) {
            SDL_ERROR_MSG("SDL: Failed to crate texture from number surface");
            return -1;
        }
    }
    return 0;
}

SDL_Texture* load_text(const char* text) {

    SDL_Surface* surface = TTF_RenderText_Solid(gFont, text, {0, 0, 0});
    
    if (surface == NULL) {
        SDL_ERROR_MSG("TTF: Failed to load number to surface");
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(gRenderer, surface);

    if (texture == NULL) {
        SDL_ERROR_MSG("SDL: Failed to crate texture from surface");
    }

    SDL_FreeSurface(surface);

    return texture;
}

void quit() {
    IMG_Quit();
	TTF_CloseFont(gFont);
    TTF_Quit();
    SDL_DestroyWindow(gWindow);
	SDL_Quit();
}
