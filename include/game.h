#ifndef GAME_H
#define GAME_H

#include "defines.h"
#include "helper.h"
#include "block.h"
#include "button.h"

////////// FUNCTIONS //////////


void collect_mouse_event();

Command proccess_mouse_event();

void button_action(Command& command);

vector<RevealedBlock> tile_action(Command command);

void set_new_board(Command command);

void set_mines(Command command);

void initialize_game();

void game_status_check();

void display(Command command);

void quit_game();

#endif
