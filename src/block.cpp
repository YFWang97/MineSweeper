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
    marked = false;
    clicked = false;
}

void Block::set_texture(SDL_Texture* texture_) {texture = texture_;}

void Block::set_mine() {mine = true;}

void Block::set_num() {num++;}

int Block::get_mine() {return mine;}

int Block::get_num() {return num;}

void Block::click() {
    clicked = true;
    marked = false;
    set_texture(baseTexture);
}

bool Block::get_clicked() {return clicked;}

void Block::toggle_mark() {
    if (!clicked) {
        marked = !marked;

        if (marked) {
            set_texture(flagTexture);
        } else {
            set_texture(plainTexture);
        }
    }
}

bool Block::get_marked() {return marked;}

void Block::reset() {
    num = 0;
    mine = false;
    marked = false;
    clicked = false;
    set_texture(plainTexture);
}

void Block::draw() {
	if (SDL_RenderCopy(gRenderer, texture, NULL, &rect)) {
		SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
	}
}

void Block::draw_num() {
    SDL_Texture* numText = numTexture[num - 1];
    int w, h;
    SDL_QueryTexture(numText, NULL, NULL, &w, &h);
    SDL_Rect numRect = {rect.x + ((rect.w - w) / 2), rect.y + ((rect.h - h) / 2), w, h};
    if (SDL_RenderCopy(gRenderer, numTexture[num - 1], NULL, &numRect)) {
        SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
    }
}
