#include "SnakeGame/Snake.h"

Snake::Snake(int startX, int startY)
    : dirX_(1), dirY_(0) {
    body_.emplace_back(startX, startY);
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
    if(dirX_ == 1 && dirY_ == 0){
        dirX_ = 0;
        dirY_ = -1;
    } else if(dirX_ == -1 && dirY_ == 0){
        dirX_ = 0;
        dirY_ = 1;
    } else if(dirX_ == 0 && dirY_ == 1){
        dirY_ = 0;
        dirX_ = 1;
    } else{
        dirY_ = 0;
        dirX_ = -1;
    }
}

void Snake::turnRight() {
    if(dirX_ == 1 && dirY_ == 0){
        dirX_ = 0;
        dirY_ = 1;
    } else if(dirX_ == -1 && dirY_ == 0){
        dirX_ = 0;
        dirY_ = -1;
    } else if(dirX_ == 0 && dirY_ == 1){
        dirY_ = 0;
        dirX_ = -1;
    } else{
        dirY_ = 0;
        dirX_ = 1;
    }
}

void Snake::grow() {
    growAmount_++;
}

bool Snake::checkCollision(int gridWidth, int gridHeight) const {
    auto head = getHead();
    if (head.first < 0 || head.first >= gridWidth || head.second < 0 || head.second >= gridHeight)
        return true;

    return std::count(body_.begin() + 1, body_.end(), head) > 0;
}

const std::deque<std::pair<int, int> > &Snake::getBody() const {
    return body_;
}

const std::pair<int, int> &Snake::getHead() const {
    return body_.front();
}

std::pair<int, int> Snake::getDir(){
    return std::make_pair(dirX_, dirY_);
}
