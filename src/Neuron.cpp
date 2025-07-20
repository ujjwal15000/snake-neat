#include <deque>
#include <Model/Neuron.h>
#include <ctime>
#include <random>

Neuron::Neuron(std::unordered_map<int, Weight> &weights, int id) :
        weights_(weights),
        inputSize_(weights.size()),
        id_(id) {
    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    static std::uniform_real_distribution<double> dist(-1.0, 1.0);
    bias_ = dist(rng);
}

Neuron::Neuron(std::unordered_map<int, Weight> &weights, double bias, int id) :
        weights_(weights),
        inputSize_(weights.size()),
        id_(id),
        bias_(bias) {
}

double Neuron::activate(std::vector<std::pair<int, double> > &inputs) {
    double res = 0;
    for (std::pair<int, double> input: inputs) {
        res += input.second * (weights_.find(input.first)->second.weight);
    }
    return res + bias_;
}

const int Neuron::getId() const {
    return id_;
}
