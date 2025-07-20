#include "SnakeGame/SDLInputProvider.h"

Direction SDLInputProvider::getInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            return mapKey(event.key.keysym.sym);
        }
        if (event.type == SDL_QUIT) {
            exit(0);  // You can handle quit elsewhere if you want
        }
    }
    return Direction::None;
}

Direction SDLInputProvider::mapKey(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT: return Direction::Left;
        case SDLK_RIGHT: return Direction::Right;
        case SDLK_a: return Direction::Left;
        case SDLK_d: return Direction::Right;
        default: return Direction::None;
    }
}
