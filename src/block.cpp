#include "block.h"
#include "defines.h"

Block::Block(SDL_Texture* texture_, Coordinate topleft_, int row_, int col_) {
    if (texture_ == NULL) {
        ERROR_MSG("Block: Invalid texture input");
    }
    
	texture = texture_;

    SDL_QueryTexture(texture, NULL, NULL, &(rect.w), &(rect.h));

    rect.x = topleft_.x;
    rect.y = topleft_.y;

    row = row_;
    col = col_;

    num = 0;
    mine = false;
    flaged = false;
    revealed = false;
	exploded = false;
	initialized = false;
}

void Block::set_texture(SDL_Texture* texture_) {texture = texture_;}

void Block::set_mine() {mine = true;}

void Block::set_num() {num++;}

bool Block::get_mine() {return mine;}

int Block::get_num() {return num;}

void Block::set_init() {
	initialized = true;
}

bool Block::get_init() {return initialized;}

void Block::explode(bool mode) {
	exploded = true;
	if (!flaged) {
		if (mode == 0) {
			set_texture(mineTexture);
		} else {
			set_texture(mineRevealTexture);
		}	
	}
}

bool Block::get_exploded() {return exploded;}


void Block::reveal() {
    revealed = true;
    flaged = false;
    set_texture(baseTexture);
}

bool Block::get_revealed() {return revealed;}

void Block::toggle_flag() {
    if (!revealed) {
        flaged = !flaged;

        if (flaged) {
            set_texture(flagTexture);
		} else {
            set_texture(plainTexture);
        }
    }
}

bool Block::get_flaged() {return flaged;}

void Block::reset() {
    num = 0;
    mine = false;
    flaged = false;
    revealed = false;
	exploded = false;
	initialized = false;
    set_texture(plainTexture);
}

void Block::set_pendingReveal() {
	if (!flaged && !revealed) {
		set_texture(baseTexture);	
	}
}

void Block::unset_pendingReveal() {
	if (!revealed) {
		set_texture(plainTexture);
	}
}

void Block::draw() {
	if (SDL_RenderCopy(gRenderer, texture, NULL, &rect)) {
		SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
	}
	draw_num();
}

void Block::draw_num() {
	if (num > 0 && revealed) {
		SDL_Texture* numText = numTexture[num - 1];
		int w, h;
		SDL_QueryTexture(numText, NULL, NULL, &w, &h);
		SDL_Rect numRect = {rect.x + ((rect.w - w) / 2), rect.y + ((rect.h - h) / 2), w, h};
		if (SDL_RenderCopy(gRenderer, numTexture[num - 1], NULL, &numRect)) {
			SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
		}
	}
}
