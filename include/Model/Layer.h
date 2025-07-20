#pragma once

#include <vector>
#include <utility>
#include "Neuron.h"

class Layer {
public:
    Layer(std::vector<Neuron> &neurons);

    std::vector<std::pair<int, double> > activate(std::vector<std::pair<int, double> > &inputs);

    [[nodiscard]] const std::vector<Neuron> &getNeurons() const;

private:
    std::vector<Neuron> neurons_;
};