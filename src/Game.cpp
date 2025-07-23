#include <unistd.h>
#include <iostream>
#include "SnakeGame/Game.h"
#include "SnakeGame/InputProvider.h"
#include "SnakeGame/SDLInputProvider.h"

Game::Game(int gridWidth, int gridHeight, Renderer* renderer, std::unique_ptr<InputProvider> inputProvider)
        : snake_(0, 0),
          food_(0, 0),
          inputProvider_(std::move(inputProvider)),
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
    reset();
    bool running = true;
    std::vector<double> inputs;
    int steps = 0;
    while (running){
        auto head = snake_.getHead();
        if(head.first == food_.first && head.second == food_.second) {
            snake_.grow();
            score_+=1;
            generateFood();
        }

        getInputs(inputs);
        Direction dir = inputProvider_->getInput(inputs);

        if (dir != Direction::None) {
            if(dir == Direction::Left)
                snake_.turnLeft();
            else if(dir == Direction::Right)
                snake_.turnRight();
        }
        snake_.move();

        if(steps++ >= 1000 || snake_.checkCollision(gridW_ / CellSize_, gridH_ / CellSize_)) {
            running = false;
            break;
        }

        if(renderer_ != nullptr) {
            render();
            usleep(16000);  // Sleep for 16ms
        }
    }
}

void Game::generateFood() {
    int maxX = gridW_ / CellSize_;
    int maxY = gridH_ / CellSize_;

    std::pair<int, int> pos;
    const auto& body = snake_.getBody();

    do {
        int foodX = std::rand() % maxX;
        int foodY = std::rand() % maxY;
        pos = {foodX, foodY};
    } while (std::find(body.begin(), body.end(), pos) != body.end());

    food_ = pos;
}

void Game::render() {
    renderer_->clear();

    // Draw Snake Body
    const auto& body = snake_.getBody();

    bool first = true;
    for (const auto& segment : body) {
        int x = segment.first * CellSize_;
        int y = segment.second * CellSize_;

        if (first) {
            // Head in red
            renderer_->drawRect(x, y, CellSize_, CellSize_, 255, 0, 0, 0);
            first = false;
        } else {
            // Body in green
            renderer_->drawRect(x, y, CellSize_, CellSize_, 0, 255, 0, 0);
        }
    }


    // Draw Food
    int foodX = food_.first * CellSize_;
    int foodY = food_.second * CellSize_;
    renderer_->drawRect(foodX, foodY, CellSize_, CellSize_, 255, 0, 0, 0);

    renderer_->present();
}

void Game::getInputs(std::vector<double>& inputs) {
    inputs.clear();

    auto head = snake_.getHead();
    auto dir = snake_.getDir();  // Direction vector (dx, dy)
    auto& body = snake_.getBody();
    std::unordered_set<std::pair<int, int>, PairHash> occupied(body.begin() + 1, body.end());

    int gridCols = gridW_ / CellSize_;
    int gridRows = gridH_ / CellSize_;

    // --- Snake direction (unit vector) ---
    double dirMag = std::hypot(dir.first, dir.second);
    inputs.push_back(dirMag > 0 ? dir.first / dirMag : 0.0);
    inputs.push_back(dirMag > 0 ? dir.second / dirMag : 0.0);

    // --- Food direction (unit vector) ---
    int dx = food_.first - head.first;
    int dy = food_.second - head.second;
    double foodDist = std::hypot(dx, dy);
    inputs.push_back(foodDist > 0 ? dx / foodDist : 0.0);
    inputs.push_back(foodDist > 0 ? dy / foodDist : 0.0);

    // --- Relative angle to food (sin, cos) ---
    double angleToFood = std::atan2(dy, dx);
    double angleSnake = std::atan2(dir.second, dir.first);
    double relAngle = angleToFood - angleSnake;
    if (relAngle > M_PI) relAngle -= 2 * M_PI;
    if (relAngle < -M_PI) relAngle += 2 * M_PI;

    inputs.push_back(std::sin(relAngle));
    inputs.push_back(std::cos(relAngle));

    // --- Distances to wall and body in four directions ---
    auto scan = [&](int dx, int dy, int maxSteps) {
        for (int i = 1; i <= maxSteps; ++i) {
            int x = head.first + i * dx;
            int y = head.second + i * dy;
            if (x < 0 || x >= gridCols || y < 0 || y >= gridRows) return i - 1;               // wall
            if (occupied.count({x, y})) return i - 1;                                         // body
        }
        return maxSteps;
    };

    int up    = scan(0, -1, head.second);                   // max steps up
    int down  = scan(0,  1, gridRows - head.second - 1);    // max steps down
    int left  = scan(-1, 0, head.first);                    // max steps left
    int right = scan(1,  0, gridCols - head.first - 1);     // max steps right

    inputs.push_back((double)up / (gridRows - 1));
    inputs.push_back((double)down / (gridRows - 1));
    inputs.push_back((double)left / (gridCols - 1));
    inputs.push_back((double)right / (gridCols - 1));
}


void Game::reset() {
    int snakeX = std::rand() % (gridW_ / CellSize_);
    int snakeY = std::rand() % (gridH_ / CellSize_);
    snake_ = Snake(snakeX, snakeY);

    generateFood();
    score_ = 0;
    steps_ = 0;
}

