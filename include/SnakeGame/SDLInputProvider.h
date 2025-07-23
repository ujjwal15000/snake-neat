#pragma once

#include "vector"
#include "InputProvider.h"
#include <SDL.h>

class SDLInputProvider : public InputProvider {
public:
    SDLInputProvider() = default;

    Direction getInput(std::vector<double>& inputs) override;

private:
    Direction mapKey(SDL_Keycode key);
};
