#include "SnakeGame/Game.h"
#include "SnakeGame/SDLInputProvider.h"
#include "Model/Model.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

static std::string vectorToString(const std::vector<double>& vec) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i != vec.size() - 1)
            oss << ", ";
    }
    oss << "]";
    return oss.str();
}

int main() {
//    SDLInputProvider inputProvider{};
//    int w = 800;
//    int h = 800;
//    Renderer renderer(w, h);
//    Game game(w, h, &renderer, &inputProvider);
//    game.start();
    Model model(4, 2);
    std::vector<double> inputs{0.4, 0.33, 0.2, 0.8};
    std::cout << vectorToString(model.feedForward(inputs)) << std::endl;
    return 0;
}
