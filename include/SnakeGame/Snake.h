#pragma once
#include <deque>
#include <utility>
#include <unordered_set>
#include "Utils/RandomUtils.h"

class Snake {
public:
    Snake(int startX, int startY);

    void move();
    void grow();
    void turnRight();
    void turnLeft();
    bool checkCollision(int gridWidth, int gridHeight) const;
    std::pair<int, int> getDir();

    [[nodiscard]] const std::deque<std::pair<int, int> >& getBody() const;
    [[nodiscard]] const std::pair<int, int>& getHead() const;

private:
    std::deque<std::pair<int, int> > body_;
    std::unordered_set<std::pair<int, int>, PairHash> positions_;
    int dirX_, dirY_, growAmount_{0};;
};