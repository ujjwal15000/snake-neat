#include "SnakeGame/Renderer.h"
#include <iostream>

Renderer::Renderer(int screenWidth, int screenHeight) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
    }

    window_ = SDL_CreateWindow("Snake Game",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               screenWidth,
                               screenHeight,
                               SDL_WINDOW_SHOWN);

    if (!window_) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(renderer_);
}

void Renderer::drawRect(int x, int y, int w, int h, int r, int g, int b, int a) {
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer_, r, g, b, a);
    SDL_RenderFillRect(renderer_, &rect);
}

void Renderer::present() {
    SDL_RenderPresent(renderer_);
}
