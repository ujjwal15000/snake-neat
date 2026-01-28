#include "Model/Population.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <omp.h>


Population::Population(int size) {
    inputs_ = 24;
    outputs_ = 3;
    size_ = size;
    for (int i = 0; i < size; ++i) {
        individuals_.emplace_back(std::make_unique<Individual>(inputs_, outputs_));
    }
}

void Population::train(Renderer *renderer) {
//    double epsilon = 0.05;
//    double minEpsilon = 0.001;
//    double decayRate = 0.995;

    while (true) {

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < individuals_.size(); ++i) {
            if (!individuals_[i]) {
//                throw std::runtime_error("individual null!!");
                std::cout << "null individual" << std::endl;
                continue;
            }
            individuals_[i]->train(0);
        }
//        std::cout << "training done: " << generation_ << " num_species: " << currMaxSpecies_ << std::endl;
//        speciate();
//        std::cout << "speciation done: " << generation_ << " num_species: " << currMaxSpecies_ << std::endl;
        crossover();
//        std::cout << "crossover done: " << generation_ << " num_species: " << currMaxSpecies_ << std::endl;
//        epsilon = std::max(minEpsilon, epsilon * decayRate);
        if (generation_ != 0 && generation_ % 10 == 0) {
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
        std::cout << "generation: " << generation_++ << " num_species: " << currMaxSpecies_
                  << " fitness: " << getFittest()->getFitness() << std::endl;
    }
}

Individual *Population::getFittest() {
    if (individuals_.empty()) {
        std::cout << "empty individuals" << std::endl;
        return nullptr;
    }

    Individual *best = individuals_[0].get();
    for (const auto &individual: individuals_) {
        if (!individual)
            continue;

        if (individual->getFitness() > best->getFitness()) {
            best = individual.get();  // just change the pointer, no mutation
        }
    }
    return best;
}

//void Population::crossover() {
//    std::vector<std::unique_ptr<Individual>> newGeneration;
//    newGeneration.reserve(individuals_.size());
//
//    for (auto &species: species_) {
//        if (species.members.empty()) continue;
//
//        std::sort(species.members.begin(), species.members.end(), [](Individual *a, Individual *b) {
//            return a->getFitness() > b->getFitness();
//        });
//
//        int eliteCount = std::max(1, static_cast<int>(species.members.size() * 0.5));
//        for (int i = 0; i < eliteCount; ++i) {
//            auto clone = species.members[i]->clone();
//            newGeneration.emplace_back(std::move(clone));
//        }
//
//        std::vector<double> fitnessWeights;
//        for (auto *member: species.members) {
//            fitnessWeights.push_back(member->getFitness());
//        }
//
//        std::discrete_distribution<int> dist(fitnessWeights.begin(), fitnessWeights.end());
//
//        while (newGeneration.size() < individuals_.size()) {
//            int idx1 = dist(rng_);
//            int idx2 = dist(rng_);
//            while (idx1 == idx2) idx2 = dist(rng_);
//
//            Model *parent1 = species.members[idx1]->getModel();
//            Model *parent2 = species.members[idx2]->getModel();
//
//            if (!parent1 || !parent2) {
//                std::cerr << "Null parent in crossover!" << std::endl;
//                continue;
//            }
//
//            auto childModel = parent1->crossover(parent2);
//            childModel->mutate();
//
//            newGeneration.emplace_back(std::make_unique<Individual>(std::move(childModel)));
//        }
//    }
//
//    while (newGeneration.size() < individuals_.size()) {
//        int idx = rng_() % individuals_.size();
//        Model *model = individuals_[idx]->getModel();
//        if (!model) continue;
//        auto clone = model->clone();
//        clone->mutate();
//        newGeneration.emplace_back(std::make_unique<Individual>(std::move(clone)));
//    }
//
//    individuals_ = std::move(newGeneration);
//}

//void Population::crossover() {
//    std::vector<std::unique_ptr<Individual>> newGeneration;
//
//    for (size_t i = 0; i < species_.size(); ++i) {
//        auto &species = species_[i];
//        if (species.members.empty()) continue;
//
//        // Sort members descending by fitness
//        std::sort(species.members.begin(), species.members.end(), [](Individual *a, Individual *b) {
//            return a->getFitness() > b->getFitness();
//        });
//
//        int elites = std::max(1, static_cast<int>(species.members.size() * 0.5));
//
//        for (int j = 0; j < elites; ++j) {
//            newGeneration.emplace_back(species.members[j]->clone());
//        }
//
//
//        // Prepare for selection
//        std::vector<double> fitnessWeights;
//        for (int a = 0; a < elites; a++) {
//            auto *member = species.members.at(a);
//            if (!member) {
//                std::cout << "null individual" << std::endl;
//                continue;
//            }
//
//            fitnessWeights.push_back(member->getFitness());
//        }
//        std::discrete_distribution<int> dist(fitnessWeights.begin(), fitnessWeights.end());
//
//        for (int b = elites; b < size_; ++b) {
//            int idx1 = dist(rng_);
//            int idx2 = dist(rng_);
////            while (idx1 == idx2) idx2 = dist(rng_);
//
//            auto *parent1 = species.members[idx1]->getModel();
//            auto *parent2 = species.members[idx2]->getModel();
//            if (!parent1 || !parent2) continue;
//
//            auto childModel = parent1->crossover(parent2);
//            childModel->mutate();
//
//            newGeneration.emplace_back(std::make_unique<Individual>(std::move(childModel)));
//        }
//    }
//
//    individuals_ = std::move(newGeneration);
//}

//void Population::crossover() {
//    std::vector<std::unique_ptr<Individual>> newGeneration;
//
//    // 1. Track the best genome globally
//    Individual *bestIndividual = nullptr;
//    double bestFitness = -1.0;
//
//    for (const auto &species: species_) {
//        for (auto *ind: species.members) {
//            if (ind->getFitness() > bestFitness) {
//                bestFitness = ind->getFitness();
//                bestIndividual = ind;
//            }
//        }
//    }
//
//    std::unique_ptr<Model> bestModelBackup = nullptr;
//    if (bestIndividual) {
//        // 2. Add best genome (global elitism)
//        newGeneration.emplace_back(bestIndividual->clone());
//        bestModelBackup = bestIndividual->getModel()->clone();
//    }
//
//
//    // 3. Assign offspring quota to each species
//    int totalFitness = 0;
//    std::vector<double> speciesFitness;
//    for (const auto &species: species_) {
//        double sf = 0.0;
//        for (auto *m: species.members) sf += m->getFitness();
//        speciesFitness.push_back(sf);
//        totalFitness += sf;
//    }
//
//    int remaining = size_ - 1;
//
//    for (size_t i = 0; i < species_.size(); ++i) {
//        auto &species = species_[i];
//        if (species.members.empty()) continue;
//
//        // Sort by fitness
//        std::sort(species.members.begin(), species.members.end(), [](Individual *a, Individual *b) {
//            return a->getFitness() > b->getFitness();
//        });
//
//        int elites = std::max(1, (int) (species.members.size() * 0.5));
//        int numOffspring = std::round(((speciesFitness[i]) / totalFitness) * size_);
//
//        // Clamp to remaining slots
//        numOffspring = std::min(numOffspring, remaining);
//        remaining -= numOffspring;
//
//        // 4. Copy elites
//        for (int j = 0; j < std::min(elites, numOffspring); ++j) {
//            newGeneration.emplace_back(species.members[j]->clone());
//        }
//
//        // 5. Generate offspring
//        std::vector<double> fitnessWeights;
//        for (int j = 0; j < elites; ++j) {
//            fitnessWeights.push_back(species.members[j]->getFitness());
//        }
//
//        std::discrete_distribution<int> dist(fitnessWeights.begin(), fitnessWeights.end());
//        for (int j = elites; j < numOffspring; ++j) {
//            int idx1 = dist(rng_);
//            int idx2 = dist(rng_);
//            auto *parent1 = species.members[idx1]->getModel();
//            auto *parent2 = species.members[idx2]->getModel();
//            if (!parent1 || !parent2) continue;
//
//            auto childModel = parent1->crossover(parent2);
//            childModel->mutate();
//            newGeneration.emplace_back(std::make_unique<Individual>(std::move(childModel)));
//        }
//    }
//
//    // 6. Pad remaining (in case rounding missed 1–2 slots)
//    while ((int) newGeneration.size() < size_) {
//        if (bestModelBackup) {
//            auto model = bestModelBackup->clone();
//            model->mutate();
//            newGeneration.emplace_back(std::make_unique<Individual>(std::move(model)));
//        } else{
//            newGeneration.emplace_back(std::make_unique<Individual>(inputs_, outputs_));
//        }
//    }
//
//    individuals_ = std::move(newGeneration);
//}



void Population::crossover() {
    std::sort(individuals_.begin(), individuals_.end(),
              [](const std::unique_ptr<Individual>& a, const std::unique_ptr<Individual>& b) {
                  return a->getFitness() > b->getFitness();
              });

    int eliteCount = individuals_.size() * 0.5;
    std::vector<std::unique_ptr<Individual>> newGeneration;
    newGeneration.reserve(individuals_.size());

    std::vector<double> fitnessWeights;
    fitnessWeights.reserve(eliteCount);

    for (int i = 0; i < eliteCount; ++i) {
        fitnessWeights.push_back(individuals_[i]->getFitness());
        newGeneration.push_back(std::move(individuals_[i]));
    }

    std::discrete_distribution<int> weightedDist(fitnessWeights.begin(), fitnessWeights.end());

    while (newGeneration.size() < individuals_.size()) {
        int idx1 = weightedDist(rng_);
        int idx2 = weightedDist(rng_);
        while (idx2 == idx1) {
            idx2 = weightedDist(rng_);
        }

        auto& parent1 = newGeneration[idx1];
        auto& parent2 = newGeneration[idx2];

        auto m1 = parent1->getModel();
        auto m2 = parent2->getModel();

        auto childModel = m1->crossover(m2);
        childModel->mutate();

        newGeneration.emplace_back(std::make_unique<Individual>(std::move(childModel)));
    }

    individuals_ = std::move(newGeneration);
}


void Population::speciate() {
    // Optional: parameters you may want to configure
    const int maxSpecies = maxSpecies_;
    const int stagnationThreshold = stagnationThreshold_;

    // Clear current species membership
    for (auto &s: species_)
        s.clear();

    // Assign individuals to species
    for (auto &individual: individuals_) {
        if (!individual)
            continue;
        bool added = false;

        for (auto &species: species_) {
            double distance = species.representative->checkCompatibility(individual.get());
            if (distance < compatibilityThreshold_) {
                species.addMember(individual.get());
                added = true;
                break;
            }
        }

        if (!added) {
            species_.emplace_back(individual.get()); // new species with this as representative
        }
    }

    // Remove stagnant species
    auto it = species_.begin();
    while (it != species_.end()) {
        if (it->members.empty()) {
            it = species_.erase(it);
            continue;
        }

        double bestFitness = it->getBestFitness();
        if (bestFitness > it->maxFitness) {
            it->maxFitness = bestFitness;
            it->stagnantGenerations = 0;
        } else {
            ++it->stagnantGenerations;
        }

        if (it->stagnantGenerations > stagnationThreshold) {
            it = species_.erase(it);
        } else {
            ++it;
        }
    }

    // Sort by max fitness and cap to top K species
    std::sort(species_.begin(), species_.end(), [](const auto &a, const auto &b) {
        return a.getTotalAdjustedFitness() > b.getTotalAdjustedFitness();
    });

    currMaxSpecies_ = species_.size();
    if (species_.size() > maxSpecies) {
        species_.erase(species_.begin() + maxSpecies, species_.end());
    }

    // Set new representative for next generation
    for (auto &s: species_) {
        s.chooseNewRepresentative();  // Implement this method
    }
}
