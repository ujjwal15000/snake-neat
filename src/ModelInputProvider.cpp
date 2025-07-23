#include "SnakeGame/ModelInputProvider.h"


Direction ModelInputProvider::getInput(std::vector<double> &inputs) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {}
    auto outputs = model_->feedForward(inputs);
    auto maxIt = std::max_element(outputs.begin(), outputs.end());
    double maxValue = *maxIt;
    int maxIndex = std::distance(outputs.begin(), maxIt);
    return static_cast<Direction>(maxIndex);
}