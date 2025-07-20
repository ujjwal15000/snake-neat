#include <unistd.h>
#include <iostream>
#include "SnakeGame/Game.h"
#include "SnakeGame/InputProvider.h"
#include "SnakeGame/SDLInputProvider.h"

Game::Game(int gridWidth, int gridHeight, Renderer* renderer, InputProvider* inputProvider)
        : snake_(0, 0),
          food_(0, 0),
          inputProvider_(inputProvider),
          renderer_(renderer),
          gridW_(gridWidth),
          gridH_(gridHeight),
          score_(0),
          steps_(0)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));  // Seed RNG once

    int snakeX = std::rand() % (gridW_ / CellSize_);
    int snakeY = std::rand() % (gridH_ / CellSize_);
    snake_ = Snake(snakeX, snakeY);
    generateFood();
}


void Game::start() {
    bool running = true;
    while (running){
        auto head = snake_.getHead();
        if(head.first == food_.first && head.second == food_.second) {
            snake_.grow();
            generateFood();
        }
        Direction dir = inputProvider_->getInput();

        if (dir != Direction::None) {
            if(dir == Direction::Left)
                snake_.turnLeft();
            else if(dir == Direction::Right)
                snake_.turnRight();
        }
        snake_.move();

        if(snake_.checkCollision(gridW_ / CellSize_, gridH_ / CellSize_))
            running = false;

        if(renderer_ != nullptr) {
            render();
            usleep(16000 * 5);  // Sleep for 16ms
        }
    }
}

void Game::generateFood() {
    int foodX = std::rand() % (gridW_ / CellSize_);
    int foodY = std::rand() % (gridH_ / CellSize_);
    food_ = std::pair(foodX, foodY);
}

void Game::render() {
    renderer_->clear();

    // Draw Snake Body
    const auto& body = snake_.getBody();
    for (const auto& segment : body) {
        int x = segment.first * CellSize_;
        int y = segment.second * CellSize_;
        renderer_->drawRect(x, y, CellSize_, CellSize_);
    }

    // Draw Food
    int foodX = food_.first * CellSize_;
    int foodY = food_.second * CellSize_;
    renderer_->drawRect(foodX, foodY, CellSize_, CellSize_);

    renderer_->present();
}

