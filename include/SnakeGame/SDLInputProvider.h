#pragma once
#include "InputProvider.h"
#include <SDL.h>

class SDLInputProvider : public InputProvider {
public:
    SDLInputProvider() = default;

    Direction getInput() override;

private:
    Direction mapKey(SDL_Keycode key);
};
