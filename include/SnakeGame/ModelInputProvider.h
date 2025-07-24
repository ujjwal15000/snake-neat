#pragma once

#include "InputProvider.h"
#include "../Model/Model.h"
#include <SDL.h>

class ModelInputProvider : public InputProvider {
public:
    explicit ModelInputProvider(Model* model, bool render) : model_(model), render_(render){}

    Direction getInput(std::vector<double>& inputs) override;

private:
    Model* model_;
    bool render_;
};
