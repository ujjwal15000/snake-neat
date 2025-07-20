#pragma once

#include <deque>
#include <utility>
#include "Layer.h"

class Model {
public:
    Model(int inputs, int outputs);

    std::vector<double> feedForward(std::vector<double>& inputs);
private:
    std::vector<Layer> layers_;
    int inputs_, outputs_;
    int id_ = 0;

    void tanh_act(std::vector<std::pair<int, double>>& vector1);
};