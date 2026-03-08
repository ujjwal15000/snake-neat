#include <unistd.h>
#include <iostream>
#include "SnakeGame/Game.h"
#include "SnakeGame/InputProvider.h"
#include "SnakeGame/SDLInputProvider.h"
#include <random>

Game::Game(int gridWidth, int gridHeight, Renderer *renderer, std::unique_ptr<InputProvider> inputProvider)
        : snake_(0, 0),
          food_(0, 0),
          inputProvider_(std::move(inputProvider)),
          renderer_(renderer),
          gridW_(gridWidth),
          gridH_(gridHeight),
          score_(0),
          steps_(0) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));  // Seed RNG once

    std::uniform_int_distribution<int> distX((gridW_ / CellSize_) * 0.25, (gridW_ / CellSize_) * 0.75);
    std::uniform_int_distribution<int> distY((gridH_ / CellSize_) * 0.25, (gridH_ / CellSize_) * 0.75);
    int snakeX = distX(rng_);;
    int snakeY = distY(rng_);
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

    std::deque<std::pair<int, int>> headHistory;

    bool bodyCollided = false;
    bool wallCollided = false;
    bool trapped = false;
    bool looping = false;

    int maxSteps = 10000;

    while (running) {
        auto head = snake_.getHead();

        // Track head positions
        headHistory.push_back(head);
        if (headHistory.size() > snake_.getBody().size() * 10)
            headHistory.pop_front();

        // Reward for surviving a step
//        score_ += 0.01;
        steps++;
        stepsSinceLastFood++;

        // Eat food
        if (head.first == food_.first && head.second == food_.second) {
            snake_.grow();
            score_ += 1;
            generateFood();
            stepsSinceLastFood = 0;
        }

        // Gather inputs
        getInputs(inputs);

        // ε-greedy decision
//        Direction dir;
//        if (dist(rng_) < epsilon) {
//            int action = actionDist(rng_);
//            if (action == 0) dir = Direction::Left;
//            else if (action == 1) dir = Direction::Right;
//            else dir = Direction::None;
//        } else {
//            dir = inputProvider_->getInput(inputs);
//        }

        Direction dir;
        dir = inputProvider_->getInput(inputs);

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

        // Loop/trap detection
        bool isLooping = false;
        bool isTrapped = false;

        std::unordered_map<std::pair<int, int>, int, PairHash> positionCounts;
        for (const auto &pos: headHistory) {
            positionCounts[pos]++;
        }
        for (const auto &[pos, count]: positionCounts) {
            if (count > 3) {
                isLooping = true;
                break;
            }
        }

        int hungerLimit = 100 * std::max((int) snake_.getBody().size() - 2, 1);
        if (stepsSinceLastFood > hungerLimit && isLooping) {
            isTrapped = true;
        }

        // Terminate on loop/trap
        if (isTrapped) {
            trapped = true;
            running = false;
        } else if (isLooping) {
            if (renderer_ != nullptr)
                std::cout << "snake looping" << std::endl;

            looping = true;
            running = false;
        }

        // Max steps check
        if (steps >= maxSteps) {
            running = false;
        }

        // Collision check
        if (snake_.checkCollision(gridW_ / CellSize_, gridH_ / CellSize_, &bodyCollided)) {
            wallCollided = true;
            running = false;
        }

//        distanceToFood += std::hypot(food_.first - head.first, food_.second - head.second);

        // Move snake
        if(running)
            snake_.move();

        // Optional rendering
        if (renderer_ != nullptr) {
            render();
            usleep(8000);  // Sleep ~8ms
        }
    }

    // Fitness = food^2 + efficiency_bonus
    // efficiency_bonus = food * (maxSteps - steps) / maxSteps
    // This rewards getting food quickly

    int foodEaten = snake_.getBody().size() - 4;  // Starting size is 4
    double efficiency = (double)(maxSteps - steps) / maxSteps;

    score_ = std::pow(foodEaten + 1, 2);  // Base: 1, 4, 9, 16...
    score_ += foodEaten * efficiency * 2; // Bonus for speed

    if (looping) {
        score_ *= 0.5;  // Moderate penalty (not too harsh)
    }
}


void Game::generateFood() {
    int maxX = gridW_ / CellSize_;
    int maxY = gridH_ / CellSize_;

    std::pair<int, int> pos;
    const auto &body = snake_.getBody();

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
    const auto &body = snake_.getBody();

    bool first = true;
    for (const auto &segment: body) {
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
    auto dir = snake_.getDir();
    auto& body = snake_.getBody();
    std::unordered_set<std::pair<int, int>, PairHash> occupied(body.begin() + 1, body.end());

    int gridCols = gridW_ / CellSize_;
    int gridRows = gridH_ / CellSize_;

    // Calculate left and right directions relative to snake
    std::pair<int, int> front = dir;
    std::pair<int, int> left = {dir.second, -dir.first};   // 90 deg counterclockwise
    std::pair<int, int> right = {-dir.second, dir.first};  // 90 deg clockwise

    std::vector<std::pair<int, int>> relDirs = {front, left, right};

    // Helper: scan in a direction and return (wallDist, bodyDist, foodFound)
    auto scan = [&](std::pair<int, int> d) -> std::tuple<double, double, double> {
        int x = head.first;
        int y = head.second;
        int steps = 0;
        double bodyDist = 0.0;
        double foodFound = 0.0;

        while (true) {
            x += d.first;
            y += d.second;
            steps++;

            // Wall hit
            if (x < 0 || x >= gridCols || y < 0 || y >= gridRows) {
                double wallDist = 1.0 / steps;  // Inverse: closer = higher
                return {wallDist, bodyDist, foodFound};
            }

            // Body detection (first encounter only)
            if (bodyDist == 0.0 && occupied.count({x, y})) {
                bodyDist = 1.0 / steps;  // Inverse: closer = higher
            }

            // Food detection
            if (foodFound == 0.0 && x == food_.first && y == food_.second) {
                foodFound = 1.0;
            }
        }
    };

    // Scan front, left, right
    for (auto& d : relDirs) {
        auto [wallDist, bodyDist, foodFound] = scan(d);
        inputs.push_back(wallDist);
        inputs.push_back(bodyDist);
        inputs.push_back(foodFound);
    }

    // Food angle relative to snake heading
    int dx = food_.first - head.first;
    int dy = food_.second - head.second;
    double angleToFood = std::atan2(dy, dx);
    double angleSnake = std::atan2(dir.second, dir.first);
    double relAngle = angleToFood - angleSnake;

    inputs.push_back(std::sin(relAngle));  // -1 to 1
    inputs.push_back(std::cos(relAngle));  // -1 to 1
}

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

