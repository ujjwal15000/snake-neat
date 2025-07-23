#pragma once

#include <SDL.h>

class Renderer {
public:
    Renderer(int screenWidth, int screenHeight);
    ~Renderer();

    void clear();
    void drawRect(int x, int y, int w, int h, int r, int g, int b, int a);
    void present();

private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
};
