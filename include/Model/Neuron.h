#pragma once

#include <unordered_map>
#include <deque>
#include <utility>

struct Weight {
public:
    Weight(double weight, int fromNeuron){
        this->weight = weight;
        this->fromNeuron = fromNeuron;
    }

    double weight;
    int fromNeuron;
};

class Neuron {
public:
    Neuron(std::unordered_map<int, Weight>& weights, int id);

    Neuron(std::unordered_map<int, Weight>& weights, double bias_, int id);

    double activate(std::vector<std::pair<int,double> > &inputs);
    const int getId() const;

private:
    std::unordered_map<int, Weight> weights_;
    double bias_;
    int inputSize_;
    int id_;
};