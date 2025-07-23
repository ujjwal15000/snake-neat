#pragma once

#include "InputProvider.h"
#include "../Model/Model.h"
#include <SDL.h>

class ModelInputProvider : public InputProvider {
public:
    explicit ModelInputProvider(Model* model) : model_(model){}

    Direction getInput(std::vector<double>& inputs) override;

private:
    Model* model_;
};
