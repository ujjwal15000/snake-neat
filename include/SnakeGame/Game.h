#pragma once
#include <deque>
#include <utility>
#include <SnakeGame/Snake.h>
#include "InputProvider.h"
#include "Renderer.h"

class Game {
public:
    Game(int gridWidth, int gridHeight, Renderer* renderer, InputProvider* inputProvider);

    void start();
    void render();

private:
    Snake snake_;
    std::pair<int, int> food_;
    InputProvider* inputProvider_;
    int gridW_, gridH_, score_, steps_;
    Renderer* renderer_;
    static const int CellSize_ = 20;

    void generateFood();
};