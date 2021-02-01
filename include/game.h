#ifndef GAME_H
#define GMAE_H

#include "defines.h"
#include "helper.h"
#include "block.h"
#include "button.h"

////////// FUNCTIONS //////////


void collect_mouse_event();

Command proccess_mouse_event();

void button_action(Command command);

void tile_action(Command command);

void set_new_board();

void set_mines(Command command);

void initialize_game();

void game_status_check();

void display(Command command);

////////// GLOBAL VARIABLES //////////

extern bool gameQuit;

////////// PARAMS & VARIABLES //////////

////////// STATUS & COMMANDS DEFINES //////////
#define STATUS_GAME_OVER			0b0001
#define STATUS_GAME_WIN				0b0010
#define STATUS_GAME_NEW_BOARD		0b0100


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

#define CMD_DISP_GRAY_ONE_TILE		0b00000001
#define CMD_DISP_GRAY_SURR_TILES	0b00000010

//Both real target changes or a click on a target can assert this signal
//This signal controls when we gray tiles
#define CMD_DISP_TARGET_CHANGED		0b00000100

#endif
