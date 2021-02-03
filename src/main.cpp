#include "helper.h"
#include "game.h"
#include "solver.h"

int main (int argc, char**argv) {

    if (initialize_sdl() || initialize_img()) {
		ERROR_MSG("PROG: Program quits. SDL Initialization failed");
        return 0;
	}

	bool solverEnabled = false;

	if (argc == 2 && strcmp(argv[1], "solver") == 0) {
		solverEnabled = true;
	}

	initialize_game();

	if (solverEnabled) {
		initialize_solver();
	}
	
	vector<RevealedBlock> newRevealedBlocks;

	//Main Game Loop
	while (!gameQuit) {

		collect_mouse_event();
		Command command = proccess_mouse_event();

		if (solverEnabled == 0) {
			button_action(command);
			
			set_mines(command);

			set_new_board(command);

			tile_action(command);
		} else {
			Command solverCommand = solver(newRevealedBlocks, command);
			
			button_action(solverCommand);
			
			set_mines(solverCommand);

			set_new_board(solverCommand);

			newRevealedBlocks = tile_action(solverCommand);
		}

		game_status_check();        

		SDL_RenderClear(gRenderer);
       
		display(command);

		SDL_RenderPresent(gRenderer);

		if (solverEnabled) SDL_Delay(100);

	}

	quit_game();

	quit();

	return 0;
}
