#ifndef BLOCK_H
#define BLOCK_H

#include "defines.h"

class Block {
	protected:
		SDL_Texture* texture;
		Coordinate topleft;
		SDL_Rect rect;
        int num;
        bool mine;
        bool marked;
        bool clicked;

	public:
		Block(SDL_Texture* texture_, Coordinate topleft_, int row_, int col_);

        int row;
        int col;

        void set_texture(SDL_Texture* texture_);

        void set_mine();
        void set_num();
        int get_mine();
        int get_num();

        void click();
        bool get_clicked();

        void toggle_mark();
        bool get_marked();

        void reset();

		void draw();		
        void draw_num();
};

#endif
