#pragma once

enum class Direction {
    Left,
    Right,
    None
};

class InputProvider {
public:
    virtual ~InputProvider() = default;

    // Virtual method to be implemented by subclasses
    virtual Direction getInput() = 0;
};
