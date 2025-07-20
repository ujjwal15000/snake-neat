#include <vector>
#include <Model/Layer.h>
#include <Model/Neuron.h>

Layer::Layer(std::vector<Neuron> &neurons) : neurons_(neurons) {
}

std::vector<std::pair<int, double> > Layer::activate(std::vector<std::pair<int, double> > &inputs) {
    std::vector<std::pair<int, double> > outputs;
    outputs.reserve(neurons_.size());
    for (Neuron neuron: neurons_) {
        outputs.emplace_back(neuron.getId(), neuron.activate(inputs));
    }
    return outputs;
}

const std::vector<Neuron> &Layer::getNeurons() const {
    return neurons_;
}
