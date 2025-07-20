#pragma once
#include <deque>
#include <utility>
#include <unordered_set>

struct PairHash {
    size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

class Snake {
public:
    Snake(int startX, int startY);

    void move();
    void grow();
    void turnRight();
    void turnLeft();
    bool checkCollision(int gridWidth, int gridHeight) const;

    const std::deque<std::pair<int, int> >& getBody() const;
    const std::pair<int, int>& getHead() const;

private:
    std::deque<std::pair<int, int> > body_;
    std::unordered_set<std::pair<int, int>, PairHash> positions_;
    int dirX_, dirY_;
};