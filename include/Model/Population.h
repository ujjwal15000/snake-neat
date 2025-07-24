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

    [[nodiscard]] double getFitness() const { return fitness_; };

    void train() {
        fitness_ = 0;
        int numTrain = 3;

        double totalScore = 0.0;
        for (int i = 0; i < numTrain; ++i) {
            game_.start();
            totalScore += game_.getScore();
        }

        fitness_ = totalScore / numTrain;
    }

    void play(Renderer* renderer) {
        Game game(800, 800,
                     renderer,
                     std::make_unique<ModelInputProvider>(model_.get(), true));
        game.start();
    }

    Model *getModel() { return model_.get(); }

    void save(std::ostream& out) const {
        model_->save(out);
    }

    void load(std::istream& in) {
        model_->load(in);
    }

private:
    Game game_;
    std::unique_ptr<Model> model_;
    double fitness_;
};

class Population {
public:
    Population(int size);

    void train(Renderer* renderer);

    [[nodiscard]] Individual *getFittest();

    void crossover();

private:
    int size_;
    int generation_{0};
    std::vector<Individual> individuals_;
};
