#include "helper.h"
#include "game.h"

int main (void) {

    if (initialize_sdl() || initialize_img()) {
		ERROR_MSG("PROG: Program quits. SDL Initialization failed");
        return 0;
	}


	initialize_game();

	//Main Game Loop
	while (!gameQuit) {

		collect_mouse_event();
		Command command = proccess_mouse_event();

		set_mines(command);

		button_action(command);

		set_new_board();

		tile_action(command);

		game_status_check();        

		SDL_RenderClear(gRenderer);
       
		display(command);

		SDL_RenderPresent(gRenderer);

        //SDL_Delay(1000);
	}

	quit_game();

	return 0;
}
