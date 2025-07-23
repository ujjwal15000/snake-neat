#pragma once

#include <vector>

enum class Direction {
    Left,
    Right,
    None
};

class InputProvider {
public:
    virtual ~InputProvider() = default;

    virtual Direction getInput(std::vector<double>& inputs) = 0;
};
