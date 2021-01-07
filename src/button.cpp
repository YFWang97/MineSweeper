#include "button.h"
#include "defines.h"
#include "helper.h"

Button::Button(char* text_, Coordinate coord_) {
    textureInfo = load_text(text_);
    rect.x = coord_.x;
    rect.y = coord_.y;
    rect.h = BUTTON_HEIGHT;
    rect.w = BUTTON_WIDTH;
    texture = releasedTexture;
}

bool Button::inside(int x, int y) {
    if (x >= rect.x && x <= rect.x + rect.w &&
        y >= rect.y && y <= rect.y + rect.h) {
        return true;
    }
    return false;
}

void Button::set_pos_x(int x) {
	rect.x = x;
}

void Button::click() {
    texture = selectedTexture;
}

void Button::release() {
    texture = releasedTexture;
}

void Button::draw() {
	if (SDL_RenderCopy(gRenderer, texture, NULL, &rect)) {
		SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
	}

    int w, h;
    SDL_QueryTexture(textureInfo, NULL, NULL, &w, &h);
    SDL_Rect infoRect = {rect.x + ((rect.w - w) / 2), rect.y + ((rect.h - h) / 2), w, h};
    if (SDL_RenderCopy(gRenderer, textureInfo, NULL, &infoRect)) {
        SDL_ERROR_MSG("SDL: Failed to copy texture to renderer");
    }
}
