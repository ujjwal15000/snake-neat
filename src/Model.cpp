#include "Model/Model.h"
#include "Model/Neuron.h"
#include <ctime>
#include <random>
#include <vector>
#include <unordered_map>

Model::Model(int inputs, int outputs)
        : inputs_(inputs),
          outputs_(outputs) {
    static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
    static std::uniform_real_distribution<double> dist(-1.0, 1.0);

    auto emptyWeights = std::unordered_map<int, Weight>{};
    auto inputNeurons = std::vector<Neuron>{};
    for (int i = 0; i < inputs; ++i) {
        inputNeurons.emplace_back(emptyWeights, 0.0, id_++);
    }

    auto outputNeurons = std::vector<Neuron>();
    outputNeurons.reserve(outputs);
    for (int i = 0; i < outputs; ++i) {
        auto weights = std::unordered_map<int, Weight>{};
        for (const Neuron& neuron: inputNeurons) {
            weights.insert(std::pair<int, Weight>(neuron.getId(),
                                                  Weight(dist(rng), neuron.getId())));
        }
        outputNeurons.emplace_back(weights, id_++);
    }

    layers_ = std::vector<Layer>();
    layers_.emplace_back(inputNeurons);
    layers_.emplace_back(outputNeurons);
}

std::vector<double> Model::feedForward(std::vector<double> &inputs) {
    std::vector<std::pair<int, double>> outputs;

    auto neurons = layers_[0].getNeurons();
    for (int i = 0; i < neurons.size(); ++i) {
        auto &neuron = neurons[i];
        outputs.emplace_back(neuron.getId(), inputs[i]);
    }

    for (int i = 1; i < layers_.size(); ++i) {
        outputs = layers_[i].activate(outputs);
        tanh_act(outputs);
    }

    std::vector<double> finalOutputs;
    finalOutputs.reserve(outputs.size());
    for (auto &out: outputs) {
        finalOutputs.push_back(out.second);
    }

    return finalOutputs;
}


void Model::tanh_act(std::vector<std::pair<int, double>>& vector1) {
    for (auto& d : vector1) {
        d.second = std::tanh(d.second);
    }
}
