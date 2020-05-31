#ifndef HELPER_H
#define HELPER_H

#include "defines.h"

int initialize();

int initialize_img();

SDL_Texture* load_img(char* name);

int load_num();

SDL_Texture* load_text(const char* text);

void quit_game();

#endif
