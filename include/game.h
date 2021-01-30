#ifndef GAME_H
#define GMAE_H

#include "defines.h"
#include "helper.h"
#include "block.h"
#include "button.h"

////////// FUNCTIONS //////////


void collect_mouse_event();

void proccess_mouse_event();

void button_action();

void tile_action();

void set_new_board();

void set_mines();

void initialize_game();

void game_status_check();

void display();

////////// GLOBAL VARIABLES //////////

extern bool gameQuit;

////////// PARAMS & VARIABLES //////////

#endif
