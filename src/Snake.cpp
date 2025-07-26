#include "SnakeGame/Snake.h"


Snake::Snake(int startX, int startY) {
    static const std::array<std::pair<int, int>, 4> directions = {
            std::make_pair(1, 0),   // Right
            std::make_pair(-1, 0),  // Left
            std::make_pair(0, -1),  // Up
            std::make_pair(0, 1)    // Down
    };

    // Pick a random direction
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 3);
    auto dir = directions[dist(rng)];

    dirX_ = dir.first;
    dirY_ = dir.second;

    // Initialize 3 blocks in the opposite direction of movement
    for (int i = 0; i < 4; ++i) {
        int x = startX - i * dirX_;
        int y = startY - i * dirY_;
        body_.emplace_back(x, y);
        positions_.insert({x, y});
    }
}

void Snake::move() {
    auto head = body_.front();
    head.first += dirX_;
    head.second += dirY_;

    body_.push_front(head);
    positions_.insert(head);

    if (growAmount_ > 0) {
        growAmount_--;
    } else {
        auto tail = body_.back();
        body_.pop_back();
        positions_.erase(tail);
    }
}

void Snake::turnLeft() {
    if (dirX_ == 1 && dirY_ == 0) {
        dirX_ = 0;
        dirY_ = -1;
    } else if (dirX_ == -1 && dirY_ == 0) {
        dirX_ = 0;
        dirY_ = 1;
    } else if (dirX_ == 0 && dirY_ == 1) {
        dirY_ = 0;
        dirX_ = 1;
    } else {
        dirY_ = 0;
        dirX_ = -1;
    }
}

void Snake::turnRight() {
    if (dirX_ == 1 && dirY_ == 0) {
        dirX_ = 0;
        dirY_ = 1;
    } else if (dirX_ == -1 && dirY_ == 0) {
        dirX_ = 0;
        dirY_ = -1;
    } else if (dirX_ == 0 && dirY_ == 1) {
        dirY_ = 0;
        dirX_ = -1;
    } else {
        dirY_ = 0;
        dirX_ = 1;
    }
}

void Snake::grow() {
    growAmount_++;
}

bool Snake::checkCollision(int gridWidth, int gridHeight, bool *bodyCollided) const {
    auto head = getHead();

    if (bodyCollided) *bodyCollided = false;

    if (head.first < 0 || head.first >= gridWidth ||
        head.second < 0 || head.second >= gridHeight)
        return true;

    if (std::count(body_.begin() + 1, body_.end(), head) > 0) {
        if (bodyCollided) *bodyCollided = true;
        return true;
    }

    return false;
}

const std::deque<std::pair<int, int> > &Snake::getBody() const {
    return body_;
}

const std::pair<int, int> &Snake::getHead() const {
    return body_.front();
}

std::pair<int, int> Snake::getDir() {
    return std::make_pair(dirX_, dirY_);
}
