#include "SnakeGame/Game.h"
#include "SnakeGame/SDLInputProvider.h"
#include "Model/Model.h"
#include "Model/Population.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <execinfo.h>
#include <csignal>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <omp.h>

void printStackTrace() {
    const int maxFrames = 64;
    void* addrlist[maxFrames];

    // Get void*'s for all entries on the stack
    int addrlen = backtrace(addrlist, maxFrames);

    if (addrlen == 0) {
        std::cerr << "  <empty, possibly corrupt stack>" << std::endl;
        return;
    }

    // Print out all the frames to stderr
    char** symbollist = backtrace_symbols(addrlist, addrlen);
    for (int i = 0; i < addrlen; i++) {
        std::cerr << symbollist[i] << std::endl;
    }

    free(symbollist);
}


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

void signalHandler(int signum) {
    std::cerr << "Caught signal " << signum << std::endl;
    printStackTrace();
    exit(signum);
}

int main() {
    signal(SIGSEGV, signalHandler);
    signal(SIGABRT, signalHandler);
//    std::unique_ptr<SDLInputProvider> inputProvider = std::make_unique<SDLInputProvider>();
//    int w = 800;
//    int h = 800;
//    std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(w, h);
//    Game game(w, h, renderer.get(), std::move(inputProvider));
//    game.start(0.0);

//    Model model(4, 2);
//    std::vector<double> inputs{0.4, 0.33, 0.2, 0.8};
//    std::cout << vectorToString(model.feedForward(inputs)) << std::endl;

    Renderer renderer(800, 800);
    Population population(100);
    population.train(&renderer);
    return 0;
}
