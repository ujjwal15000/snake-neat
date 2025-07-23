#include "Model/Population.h"
#include <iostream>

Population::Population(int size){
    int inputs = 10;
    int outputs = 3;
    size_ = size;
    for (int i = 0; i < size; ++i) {
        individuals_.emplace_back(inputs, outputs);
    }
}

void Population::train(Renderer* renderer) {
    while(true) {
        for (auto& individual: individuals_) {
            individual.train();
        }
        crossover();
        if(generation_ != 0 && generation_ % 10==0){
            auto fittest = getFittest();
            fittest->play(renderer);
        }
        std::cout << "generation: " << generation_++ << " fitness: " << getFittest()->getFitness() << std::endl;
    }
}

Individual* Population::getFittest() {
    if (individuals_.empty()) return nullptr;

    Individual* best = &individuals_[0];
    for (auto& individual : individuals_) {
        if (individual.getFitness() > best->getFitness()) {
            best = &individual;
        }
    }
    return best;
}

void Population::crossover() {
    std::sort(individuals_.begin(), individuals_.end(), [](const Individual& a, const Individual& b) {
        return a.getFitness() > b.getFitness();
    });

    int eliteCount = individuals_.size() / 2;
    std::vector<Individual> newGeneration;
    newGeneration.reserve(individuals_.size());

    for (int i = 0; i < eliteCount; ++i) {
        newGeneration.push_back(std::move(individuals_[i]));
    }

    while (newGeneration.size() < individuals_.size()) {
        Individual& parent1 = newGeneration[rand() % eliteCount];
        Individual& parent2 = newGeneration[rand() % eliteCount];

        auto m1 = parent1.getModel();
        auto m2 = parent2.getModel();

        auto childModel = m1->crossover(m2);
        childModel->mutate();
        Individual child(std::move(childModel));
        newGeneration.push_back(std::move(child));
    }

    individuals_ = std::move(newGeneration);
}

