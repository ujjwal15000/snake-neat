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
        individuals_.emplace_back(std::make_unique<Individual>(inputs, outputs));
    }
}

void Population::train(Renderer *renderer) {
//    double epsilon = 0.05;
//    double minEpsilon = 0.001;
//    double decayRate = 0.995;

    while (true) {

#pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < individuals_.size(); ++i) {
            if(!individuals_[i]) {
//                throw std::runtime_error("individual null!!");
                std::cout << "null individual" << std::endl;
                continue;
            }
            individuals_[i]->train(0);
        }
//        std::cout << "training done: " << generation_ << " num_species: " << currMaxSpecies_ << std::endl;
        speciate();
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

Individual* Population::getFittest() {
    if (individuals_.empty()) {
        std::cout << "empty individuals" << std::endl;
        return nullptr;
    }

    Individual* best = individuals_[0].get();
    for (const auto& individual : individuals_) {
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

void Population::crossover() {
    std::vector<std::unique_ptr<Individual>> newGeneration;
//    newGeneration.reserve(individuals_.size());

//    // STEP 1: Compute total adjusted fitness across all species
//    std::vector<double> speciesFitness;
//    double totalAdjustedFitness = 0.0;
//    for (const auto& s : species_) {
//        double fit = s.getTotalAdjustedFitness();
//        speciesFitness.push_back(fit);
//        totalAdjustedFitness += fit;
//    }

    // STEP 2: Allocate offspring slots per species
//    std::vector<int> offspringCounts(species_.size(), 0);
//    int allocated = 0;
//
//    for (size_t i = 0; i < species_.size(); ++i) {
//        offspringCounts[i] = static_cast<int>(
//                (speciesFitness[i] / totalAdjustedFitness) * individuals_.size()
//        );
//        allocated += offspringCounts[i];
//    }
//
//    // STEP 3: Fix rounding errors to match population size exactly
//    while (allocated < individuals_.size()) {
//        // Give bonus baby to the species with highest adjusted fitness
//        size_t idx = std::distance(speciesFitness.begin(),
//                                   std::max_element(speciesFitness.begin(), speciesFitness.end()));
//        offspringCounts[idx]++;
//        allocated++;
//    }

    // STEP 4: Actual reproduction
    for (size_t i = 0; i < species_.size(); ++i) {
        auto& species = species_[i];
        if (species.members.empty()) continue;

        // Sort members descending by fitness
        std::sort(species.members.begin(), species.members.end(), [](Individual* a, Individual* b) {
            return a->getFitness() > b->getFitness();
        });

        int elites = std::max(1, static_cast<int>(species.members.size() * 0.5));

        for (int j = 0; j < elites; ++j) {
            newGeneration.emplace_back(species.members[j]->clone());
        }


        // Prepare for selection
        std::vector<double> fitnessWeights;
        for (auto* member : species.members) {
            if(!member){
                std::cout << "null individual" << std::endl;
                continue;
            }

            fitnessWeights.push_back(member->getFitness());
        }
        std::discrete_distribution<int> dist(fitnessWeights.begin(), fitnessWeights.end());

        for (int b = 0; b < size_; ++b) {
            int idx1 = dist(rng_);
            int idx2 = dist(rng_);
//            while (idx1 == idx2) idx2 = dist(rng_);

            auto* parent1 = species.members[idx1]->getModel();
            auto* parent2 = species.members[idx2]->getModel();
            if (!parent1 || !parent2) continue;

            auto childModel = parent1->crossover(parent2);
            childModel->mutate();

            newGeneration.emplace_back(std::make_unique<Individual>(std::move(childModel)));
        }
    }

    // Final sanity clamp in case something went off
//    while (newGeneration.size() > individuals_.size()) newGeneration.pop_back();
//    while (newGeneration.size() < individuals_.size()) {
//        int idx = rng_() % individuals_.size();
//        auto clone = individuals_[idx]->getModel()->clone();
//        clone->mutate();
//        newGeneration.emplace_back(std::make_unique<Individual>(std::move(clone)));
//    }

    individuals_ = std::move(newGeneration);
//    speciate();
}


//void Population::crossover() {
//    std::sort(individuals_.begin(), individuals_.end(),
//              [](const std::unique_ptr<Individual>& a, const std::unique_ptr<Individual>& b) {
//                  return a->getFitness() > b->getFitness();
//              });
//
//    int eliteCount = individuals_.size() * 0.5;
//    std::vector<std::unique_ptr<Individual>> newGeneration;
//    newGeneration.reserve(individuals_.size());
//
//    std::vector<double> fitnessWeights;
//    fitnessWeights.reserve(eliteCount);
//
//    for (int i = 0; i < eliteCount; ++i) {
//        fitnessWeights.push_back(individuals_[i]->getFitness());
//        newGeneration.push_back(std::move(individuals_[i]));
//    }
//
//    std::discrete_distribution<int> weightedDist(fitnessWeights.begin(), fitnessWeights.end());
//
//    while (newGeneration.size() < individuals_.size()) {
//        int idx1 = weightedDist(rng_);
//        int idx2 = weightedDist(rng_);
//        while (idx2 == idx1) {
//            idx2 = weightedDist(rng_);
//        }
//
//        auto& parent1 = newGeneration[idx1];
//        auto& parent2 = newGeneration[idx2];
//
//        auto m1 = parent1->getModel();
//        auto m2 = parent2->getModel();
//
//        auto childModel = m1->crossover(m2);
//        childModel->mutate();
//
//        newGeneration.emplace_back(std::make_unique<Individual>(std::move(childModel)));
//    }
//
//    individuals_ = std::move(newGeneration);
//}


void Population::speciate() {
    // Optional: parameters you may want to configure
    const int maxSpecies = maxSpecies_;
    const int stagnationThreshold = stagnationThreshold_;

    // Clear current species membership
    for (auto &s : species_)
        s.clear();

    // Assign individuals to species
    for (auto &individual : individuals_) {
        if(!individual)
            continue;
        bool added = false;

        for (auto &species : species_) {
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
        return a.getBestFitness() > b.getBestFitness();
    });

    currMaxSpecies_ = species_.size();
    if (species_.size() > maxSpecies) {
        species_.erase(species_.begin() + maxSpecies, species_.end());
    }

    // Set new representative for next generation
    for (auto &s : species_) {
        s.chooseNewRepresentative();  // Implement this method
    }
}
