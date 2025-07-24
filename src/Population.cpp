#include "Model/Population.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <omp.h>


Population::Population(int size) {
    int inputs = 16;
    int outputs = 3;
    size_ = size;
    for (int i = 0; i < size; ++i) {
        individuals_.emplace_back(inputs, outputs);
    }
}

void Population::train(Renderer *renderer) {
    while (true) {

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < individuals_.size(); ++i) {
            individuals_[i].train();
        }
        crossover();
        if (generation_ != 0 && generation_ % 100 == 0) {
            auto fittest = getFittest();
            fittest->play(renderer);

            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::localtime(&now_c);

            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
            std::string datetime = oss.str();
            std::string folder = "runs/" + datetime;
            std::filesystem::create_directories(folder);
            std::string filename = folder + "/fittest_gen_" + std::to_string(generation_) + ".bin";
            std::ofstream out(filename, std::ios::binary);
            if (out) {
                fittest->save(out);
                out.close();
            }
        }
        std::cout << "generation: " << generation_++ << " fitness: " << getFittest()->getFitness() << std::endl;
    }
}

Individual *Population::getFittest() {
    if (individuals_.empty()) return nullptr;

    Individual *best = &individuals_[0];
    for (auto &individual: individuals_) {
        if (individual.getFitness() > best->getFitness()) {
            best = &individual;
        }
    }
    return best;
}

void Population::crossover() {
    std::sort(individuals_.begin(), individuals_.end(), [](const Individual &a, const Individual &b) {
        return a.getFitness() > b.getFitness();
    });

    int eliteCount = individuals_.size() * 0.2;
    std::vector<Individual> newGeneration;
    newGeneration.reserve(individuals_.size());

    for (int i = 0; i < eliteCount; ++i) {
        newGeneration.push_back(std::move(individuals_[i]));
    }

    std::vector<double> fitnessWeights;
    for (int i = 0; i < eliteCount; ++i) {
        fitnessWeights.push_back(individuals_[i].getFitness());
    }

    std::discrete_distribution<int> weightedDist(fitnessWeights.begin(), fitnessWeights.end());

    while (newGeneration.size() < individuals_.size()) {
//        int idx1 = rand() % eliteCount;
//        int idx2 = idx1;
        int idx1 = weightedDist(rng_);
//        int idx2 = idx1;
        int idx2 = weightedDist(rng_);
//        while (idx2 == idx1) {
//            idx2 = weightedDist(rng_);
////            idx2 = rand() % eliteCount;
//        }

        Individual &parent1 = newGeneration[idx1];
        Individual &parent2 = newGeneration[idx2];

        auto m1 = parent1.getModel();
        auto m2 = parent2.getModel();

        auto childModel = m1->crossover(m2);
        childModel->mutate();
        Individual child(std::move(childModel));
        newGeneration.push_back(std::move(child));
    }

    individuals_ = std::move(newGeneration);
}

//void Population::crossover() {
//    std::sort(individuals_.begin(), individuals_.end(), [](const Individual &a, const Individual &b) {
//        return a.getFitness() > b.getFitness();
//    });
//
//    int eliteCount = individuals_.size() / 2;
//    std::vector<Individual> newGeneration;
//    newGeneration.reserve(individuals_.size());
//
//    for (int i = 0; i < eliteCount; ++i) {
//        newGeneration.push_back(std::move(individuals_[i]));
//    }
//
//    // Precompute fitness scores of elite individuals
//    std::vector<double> fitnessScores(eliteCount);
//    double fitnessSum = 0.0;
//    for (int i = 0; i < eliteCount; ++i) {
//        fitnessScores[i] = newGeneration[i].getFitness();
//        fitnessSum += fitnessScores[i];
//    }
//
//    auto weightedIndex = [&](double sum, const std::vector<double>& scores) {
//        double r = ((double)rand() / RAND_MAX) * sum;
//        double acc = 0.0;
//        for (int i = 0; i < scores.size(); ++i) {
//            acc += scores[i];
//            if (acc >= r)
//                return i;
//        }
//        return static_cast<int>(scores.size()) - 1;
//    };
//
//    while (newGeneration.size() < individuals_.size()) {
//        int idx1 = weightedIndex(fitnessSum, fitnessScores);
//        int idx2 = idx1;
//        while (idx2 == idx1) {
//            idx2 = weightedIndex(fitnessSum, fitnessScores);
//        }
//
//        Individual &parent1 = newGeneration[idx1];
//        Individual &parent2 = newGeneration[idx2];
//
//        auto m1 = parent1.getModel();
//        auto m2 = parent2.getModel();
//
//        auto childModel = m1->crossover(m2);
//        childModel->mutate();
//        Individual child(std::move(childModel));
//        newGeneration.push_back(std::move(child));
//    }
//
//    individuals_ = std::move(newGeneration);
//}

//void Population::crossover() {
//    std::sort(individuals_.begin(), individuals_.end(), [](const Individual &a, const Individual &b) {
//        return a.getFitness() > b.getFitness();
//    });
//
//    int eliteCount = individuals_.size() / 2;
//    std::vector<Individual> newGeneration;
//    newGeneration.reserve(individuals_.size());
//
//    // Move elite individuals to new generation
//    for (int i = 0; i < eliteCount; ++i) {
//        newGeneration.push_back(std::move(individuals_[i]));
//    }
//
//    // Prepare weighted selection from remaining (use individuals_, not newGeneration)
//    std::vector<double> cumulativeFitness;
//    cumulativeFitness.reserve(eliteCount);
//    double fitnessSum = 0.0;
//    for (int i = 0; i < eliteCount; ++i) {
//        fitnessSum += individuals_[i].getFitness();
//        cumulativeFitness.push_back(fitnessSum);
//    }
//
//    auto weightedIndex = [&](double totalFitness) {
//        double r = ((double)rand() / RAND_MAX) * totalFitness;
//        return std::upper_bound(cumulativeFitness.begin(), cumulativeFitness.end(), r) - cumulativeFitness.begin();
//    };
//
//    while (newGeneration.size() < individuals_.size()) {
//        int idx1 = weightedIndex(fitnessSum);
//        int idx2 = idx1;
//        while (idx2 == idx1) {
//            idx2 = weightedIndex(fitnessSum);
//        }
//
//        Individual &parent1 = individuals_[idx1];  // from original pool
//        Individual &parent2 = individuals_[idx2];
//
//        auto m1 = parent1.getModel();
//        auto m2 = parent2.getModel();
//
//        if (!m1 || !m2) {
//            throw std::runtime_error("Null model during crossover");
//        }
//
//        auto childModel = m1->crossover(m2);
//        childModel->mutate();
//
//        newGeneration.emplace_back(std::move(childModel));
//    }
//
//    individuals_ = std::move(newGeneration);
//}



