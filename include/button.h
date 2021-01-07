#ifndef BUTTON_H
#define BUTTON_H

#include "defines.h"
#include "helper.h"

class Button {
    private:

        SDL_Texture* texture;
        SDL_Texture* textureInfo;

        SDL_Rect rect;

    public:
        Button(char* text_, Coordinate coord_);

        bool inside(int x, int y);

		void set_pos_x(int x);

        void click();
        void release();

        void draw();
};

#endif
