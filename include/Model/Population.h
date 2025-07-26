#pragma once

#include <unordered_map>
#include <unordered_set>
#include <Utils/RandomUtils.h>
#include <Model/Model.h>
#include <queue>
#include "SnakeGame/ModelInputProvider.h"
#include "SnakeGame/Game.h"

struct Individual {
public:
    Individual(int inputs, int outputs) :
            model_(std::make_unique<Model>(inputs, outputs)),
            game_(800, 800, nullptr, nullptr),
            fitness_(0) {
        game_ = Game(800, 800,
                     nullptr,
                     std::make_unique<ModelInputProvider>(model_.get(), false));
    };

    Individual(std::unique_ptr<Model> model) :
            game_(800, 800, nullptr, nullptr),
            model_(std::move(model)),
            fitness_(0) {
        game_ = Game(800, 800,
                     nullptr,
                     std::make_unique<ModelInputProvider>(model_.get(), false));
    }

    Individual(std::unique_ptr<Model> model, double fitness) :
            game_(800, 800, nullptr, nullptr),
            model_(std::move(model)),
            fitness_(fitness) {
        game_ = Game(800, 800,
                     nullptr,
                     std::make_unique<ModelInputProvider>(model_.get(), false));
    }

    std::unique_ptr<Individual> clone() {
        auto modelClone = model_->clone();
        auto clonedIndividual = std::make_unique<Individual>(std::move(modelClone),
                                                             fitness_);
        return clonedIndividual;
    }

    [[nodiscard]] double getFitness() const { return fitness_; };

    void train(double epsilon) {
        fitness_ = 0;
        int numTrain = 2;

        double totalScore = 0.0;
        for (int i = 0; i < numTrain; ++i) {
            game_.start(epsilon);
            totalScore += game_.getScore();
        }

        fitness_ = totalScore / numTrain;
    }

    void play(Renderer *renderer) {
        Game game(800, 800,
                  renderer,
                  std::make_unique<ModelInputProvider>(model_.get(), true));
        game.start(0);
    }

    Model *getModel() { return model_.get(); }

    void save(std::ostream &out) const { model_->save(out); }

    void load(std::istream &in) { model_->load(in); }

    double checkCompatibility(Individual *other) { return model_->getCompatibilityDistance(other->getModel()); }

private:
    Game game_;
    std::unique_ptr<Model> model_;
    double fitness_;
};

struct Species {
    std::unique_ptr<Individual> representative = nullptr;
    std::vector<Individual *> members;

    double maxFitness = -std::numeric_limits<double>::infinity();
    int stagnantGenerations = 0;

    explicit Species(Individual *repr)
            : representative(repr->clone()) {
        members.push_back(repr);
        maxFitness = repr->getFitness();
    }

    void addMember(Individual *ind) {
        members.push_back(ind);
    }

    void clear() {
        members.clear();
    }

    double getBestFitness() const {
        double best = -std::numeric_limits<double>::infinity();
        for (const auto *m: members) {
            if (m->getFitness() > best) best = m->getFitness();
        }
        return best;
    }

    double getTotalAdjustedFitness() const {
        double total = 0.0;
        for (const auto *m : members) {
            total += m->getFitness() / members.size();  // shared fitness
        }
        return total;
    }

    void chooseNewRepresentative() {
        if (!members.empty()) {
            // Random or fitness-based
            int idx = rand() % members.size(); // or use std::mt19937
            representative = members[idx]->clone();
        }
    }
};

class Population {
public:
    Population(int size);

    void train(Renderer *renderer);

    [[nodiscard]] Individual *getFittest();

    void crossover();

    void speciate();

private:
    int size_;
    int generation_{0};
    std::vector<Individual> individuals_;
    std::vector<Species> species_;
    int currMaxSpecies_{0};
    double compatibilityThreshold_ = 0.1;
    double maxSpecies_ = 10, stagnationThreshold_ = 10000;
};
