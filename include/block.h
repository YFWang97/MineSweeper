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
        bool flaged;
        bool revealed;
		bool exploded;
		bool initialized;

	public:
		Block(SDL_Texture* texture_, Coordinate topleft_, int row_, int col_);

        int row;
        int col;

        void set_texture(SDL_Texture* texture_);

        void set_mine();
        void set_num();
		bool get_mine();
        int get_num();

		void set_init();
		bool get_init();

		void explode(bool mode);

        void reveal();
        bool get_revealed();

        void toggle_flag();
        bool get_flaged();

		void set_pendingReveal();
		void unset_pendingReveal();

        void reset();

		void draw();		
        void draw_num();
};

#endif
