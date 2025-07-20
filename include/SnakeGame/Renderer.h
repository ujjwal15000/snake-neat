#pragma once

#include <SDL.h>

class Renderer {
public:
    Renderer(int screenWidth, int screenHeight);
    ~Renderer();

    void clear();
    void drawRect(int x, int y, int w, int h);
    void present();

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
};
