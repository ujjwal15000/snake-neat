#include <unistd.h>
#include <iostream>
#include "SnakeGame/Game.h"
#include "SnakeGame/InputProvider.h"
#include "SnakeGame/SDLInputProvider.h"
#include <random>

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


void Game::start(double epsilon) {
    reset();
    bool running = true;
    std::vector<double> inputs;
    int steps = 0;

    int leftTurns = 0;
    int rightTurns = 0;
    int stepsSinceLastFood = 0;

    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::uniform_int_distribution<int> actionDist(0, 2);

    bool bodyCollided = false;
    while (running) {
        auto head = snake_.getHead();

        // Reward for surviving a step (small incentive to live)
        score_ += 0.01;
        steps++;
        stepsSinceLastFood++;

        // Eat food
        if (head.first == food_.first && head.second == food_.second) {
            snake_.grow();
            score_ += 10.0;  // Strong reward for eating
            generateFood();
            stepsSinceLastFood = 0;
        }

        // Gather inputs
        getInputs(inputs);

        // ε-greedy decision
        Direction dir;
        if (dist(rng_) < epsilon) {
            // Explore: random direction (Left, Forward, Right)
            int action = actionDist(rng_);
            if (action == 0) dir = Direction::Left;
            else if (action == 1) dir = Direction::Right;
            else dir = Direction::None;
        } else {
            // Exploit: NN's best choice
            dir = inputProvider_->getInput(inputs);
        }

        // Apply direction
        if (dir != Direction::None) {
            if (dir == Direction::Left) {
                snake_.turnLeft();
                leftTurns++;
            } else if (dir == Direction::Right) {
                snake_.turnRight();
                rightTurns++;
            }
        }

        // Termination conditions
        int maxSteps = std::min(std::max(static_cast<int>(score_ * 1000.0), 1000), 10000);
        int hungerLimit = 100 + 50 * snake_.getBody().size();  // Scales with length

        if (steps >= maxSteps ||
            stepsSinceLastFood > hungerLimit ||
            snake_.checkCollision(gridW_ / CellSize_, gridH_ / CellSize_, &bodyCollided)) {
            running = false;
        }

        // Move snake
        snake_.move();

        // Optional rendering
        if (renderer_ != nullptr) {
            render();
            usleep(8000);  // Sleep ~8ms
        }
    }

    // Final fitness shaping (optional, if you're using score_ as fitness)
    score_ = score_ + std::pow(snake_.getBody().size(), 1.5);
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
    auto tail = snake_.getBody().back();
    auto dir = snake_.getDir();  // Direction vector (dx, dy)
    auto& body = snake_.getBody();
    std::unordered_set<std::pair<int, int>, PairHash> occupied(body.begin() + 1, body.end());

    int gridCols = gridW_ / CellSize_;
    int gridRows = gridH_ / CellSize_;

    // Snake direction
    inputs.push_back(dir.first);
    inputs.push_back(dir.second);

    // Food direction (normalized vector)
    int dx = food_.first - head.first;
    int dy = food_.second - head.second;
    double foodDist = std::hypot(dx, dy);

    // Relative angle to food (sin, cos)
    double angleToFood = std::atan2(dy, dx);
    double angleSnake = std::atan2(dir.second, dir.first);
    double relAngle = angleToFood - angleSnake;

    // Normalize the relative angle to the range [-π, π]
    relAngle = std::atan2(std::sin(relAngle), std::cos(relAngle));

    auto isCollision = [&](int x, int y) -> bool {
        return x < 0 || x >= gridCols || y < 0 || y >= gridRows ||
               std::find(body.begin() + 1, body.end(), std::make_pair(x, y)) != body.end();
    };

//    int nextX = head.first + dir.first;
//    int nextY = head.second + dir.second;

    bool bodyInPath = false;
    auto [x, y] = head;
    while (true) {
        x += dir.first;
        y += dir.second;

        // Stop if out of bounds
        if (x < 0 || x >= gridCols || y < 0 || y >= gridRows)
            break;

        // If body is found in this path
        if (occupied.count({x, y})) {
            bodyInPath = true;
            break;
        }

        // Optional: stop if food is found, you can remove this if you want full ray trace
        if (x == food_.first && y == food_.second)
            break;
    }

    // If food is in front (within ±90°), use sin and cos; else push 0
    if (std::abs(relAngle) <= M_PI / 2 && !bodyInPath) {
        inputs.push_back(foodDist > 0 ? dx / foodDist : 0.0);
        inputs.push_back(foodDist > 0 ? dy / foodDist : 0.0);
        inputs.push_back(std::sin(relAngle));
        inputs.push_back(std::cos(relAngle));
    } else {
        inputs.push_back(0.0);
        inputs.push_back(0.0);
        inputs.push_back(0.0);
        inputs.push_back(0.0);
    }

    // Tail relative to food — weighted by proximity
    int dTx = head.first - tail.first;
    int dTy = head.second - tail.second;
    double tailDist = std::hypot(dTx, dTy);
    double weight = (tailDist > 0.0) ? 1.0 / tailDist : 1.0;

    inputs.push_back(weight * dTx);
    inputs.push_back(weight * dTy);

    // Relative angle to tail (sin, cos) — also weighted
    double angleToTail = std::atan2(dTy, dTx);
    double relTailAngle = angleToTail - angleSnake;
    inputs.push_back(weight * std::sin(relTailAngle));
    inputs.push_back(weight * std::cos(relTailAngle));

    // Direction vectors for left and right
    std::pair<int, int> left(-dir.second, dir.first);
    std::pair<int, int> right(dir.second, -dir.first);



    auto bodyDistance = [&](std::pair<int, int> delta) -> double {
        int x = head.first, y = head.second;
        double dist = 1.0;
        while (true) {
            x += delta.first;
            y += delta.second;
            if (x < 0 || x >= gridCols || y < 0 || y >= gridRows) break;
            if (std::find(body.begin() + 1, body.end(), std::make_pair(x, y)) != body.end()) return dist;
            dist += 1.0;
        }
        return -1.0;
    };

    // Danger ahead/left/right: 1 if wall/body in next cell
    inputs.push_back(isCollision(head.first + dir.first, head.second + dir.second) ? 1.0 : 0.0);   // front
    inputs.push_back(isCollision(head.first + left.first, head.second + left.second) ? 1.0 : 0.0); // left
    inputs.push_back(isCollision(head.first + right.first, head.second + right.second) ? 1.0 : 0.0); // right

    // Inverse body distance in front/left/right (0 = no obstacle)
    for (auto d : {dir, left, right}) {
        double dBody = bodyDistance(d);
        inputs.push_back(dBody < 0 ? 0.0 : 1.0 / dBody);
    }
}

//void Game::getInputs(std::vector<double>& inputs) {
//    inputs.clear();
//
//    auto head = snake_.getHead();
//    auto& body = snake_.getBody();
//    std::unordered_set<std::pair<int, int>, PairHash> occupied(body.begin() + 1, body.end());
//
//    int gridCols = gridW_ / CellSize_;
//    int gridRows = gridH_ / CellSize_;
//
//    // 8 directions: N, NE, E, SE, S, SW, W, NW
//    std::vector<std::pair<int, int>> dirs = {
//            {0, -1}, {1, -1}, {1, 0}, {1, 1},
//            {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}
//    };
//
//    for (auto& d : dirs) {
//        int dx = d.first;
//        int dy = d.second;
//
//        bool sawWall = false;
//        bool sawFood = false;
//        bool sawBody = false;
//
//        int x = head.first;
//        int y = head.second;
//
//        while (true) {
//            x += dx;
//            y += dy;
//
//            // Wall detection
//            if (x < 0 || x >= gridCols || y < 0 || y >= gridRows) {
//                sawWall = true;
//                break; // Stop scanning in this direction
//            }
//
//            // Food detection
//            if (!sawFood && x == food_.first && y == food_.second) {
//                sawFood = true;
//            }
//
//            // Body detection
//            if (!sawBody && occupied.count({x, y})) {
//                sawBody = true;
//            }
//
//            // Early exit if all three are detected
//            if (sawFood && sawBody)
//                break;
//        }
//
//        inputs.push_back(sawWall ? 1.0 : 0.0);
//        inputs.push_back(sawFood ? 1.0 : 0.0);
//        inputs.push_back(sawBody ? 1.0 : 0.0);
//    }
//}

//void Game::getInputs(std::vector<double>& inputs) {
//    inputs.clear();
//
//    auto head = snake_.getHead();
//    auto& body = snake_.getBody();
//    std::unordered_set<std::pair<int, int>, PairHash> occupied(body.begin() + 1, body.end());
//
//    int gridCols = gridW_ / CellSize_;
//    int gridRows = gridH_ / CellSize_;
//    double maxDist = std::hypot(gridCols, gridRows);  // Used for normalization
//
//    // 8 directions: N, NE, E, SE, S, SW, W, NW
//    std::vector<std::pair<int, int>> dirs = {
//            {0, -1},  {1, -1},  {1, 0},  {1, 1},
//            {0, 1},   {-1, 1},  {-1, 0}, {-1, -1}
//    };
//
//    for (auto& d : dirs) {
//        int dx = d.first;
//        int dy = d.second;
//
//        double distToWall = 0.0;
//        double foodInDir = 0.0;
//        double distToBody = 0.0;
//
//        int x = head.first;
//        int y = head.second;
//        int steps = 0;
//
//        while (true) {
//            x += dx;
//            y += dy;
//            steps++;
//
//            // Wall detection — normalized
//            if (x < 0 || x >= gridCols || y < 0 || y >= gridRows) {
//                distToWall = steps / maxDist;  // normalized distance
//                break;
//            }
//
//            // Food detection — binary
//            if (foodInDir == 0.0 && x == food_.first && y == food_.second) {
//                foodInDir = 1.0;
//            }
//
//            // Body detection — normalized
//            if (distToBody == 0.0 && occupied.count({x, y})) {
//                distToBody = steps / maxDist;  // normalized distance
//            }
//        }
//
//        inputs.push_back(distToWall);
//        inputs.push_back(foodInDir);
//        inputs.push_back(distToBody);
//    }
//}

//void Game::getInputs(std::vector<double>& inputs) {
//    inputs.clear();
//
//    auto head = snake_.getHead();
//    auto dir = snake_.getDir();  // (dx, dy)
//    auto& body = snake_.getBody();
//    std::unordered_set<std::pair<int, int>, PairHash> occupied(body.begin() + 1, body.end());
//
//    int gridCols = gridW_ / CellSize_;
//    int gridRows = gridH_ / CellSize_;
//
//    // --- Snake direction (unit vector) ---
//    inputs.push_back(dir.first);
//    inputs.push_back(dir.second);
//
//    // --- Food direction (unit vector) ---
//    int dx = food_.first - head.first;
//    int dy = food_.second - head.second;
//    double foodDist = std::hypot(dx, dy);
//    inputs.push_back(foodDist > 0 ? dx / foodDist : 0.0);
//    inputs.push_back(foodDist > 0 ? dy / foodDist : 0.0);
//
//    // --- Relative angle to food (sin, cos) ---
//    double angleToFood = std::atan2(dy, dx);
//    double angleSnake = std::atan2(dir.second, dir.first);
//    double relAngle = angleToFood - angleSnake;
//    if (relAngle > M_PI) relAngle -= 2 * M_PI;
//    if (relAngle < -M_PI) relAngle += 2 * M_PI;
//
//    inputs.push_back(std::sin(relAngle));
//    inputs.push_back(std::cos(relAngle));
//
//    // --- Distances to walls (normalized) ---
//    double rightWall = (gridCols - head.first) / static_cast<double>(gridCols);
//    double downWall = (gridRows - head.second) / static_cast<double>(gridRows);
//    double leftWall = head.first / static_cast<double>(gridCols);
//    double upWall = head.second / static_cast<double>(gridRows);
//
//    inputs.push_back(rightWall);
//    inputs.push_back(downWall);
//    inputs.push_back(leftWall);
//    inputs.push_back(upWall);
//
//    // --- Body proximity in 8 directions (N, NE, E, SE, S, SW, W, NW) ---
//    std::vector<std::pair<int, int>> dirs = {
//            {0, -1},  {1, -1},  {1, 0},  {1, 1},
//            {0, 1},   {-1, 1},  {-1, 0}, {-1, -1}
//    };
//
//    double maxSteps = std::max(gridCols, gridRows);  // For normalization
//
//    for (auto& d : dirs) {
//        int steps = 1;
//        int x = head.first + d.first;
//        int y = head.second + d.second;
//        double found = 0.0;
//
//        while (x >= 0 && x < gridCols && y >= 0 && y < gridRows) {
//            if (occupied.count({x, y})) {
//                found = 1.0 - (steps / maxSteps);  // Closer body = higher input
//                break;
//            }
//            x += d.first;
//            y += d.second;
//            steps++;
//        }
//
//        inputs.push_back(found);  // Add one value per direction
//    }
//}


void Game::reset() {
    int snakeX = std::rand() % (gridW_ / CellSize_);
    int snakeY = std::rand() % (gridH_ / CellSize_);
    snake_ = Snake(snakeX, snakeY);

    generateFood();
    score_ = 0;
    steps_ = 0;
}

