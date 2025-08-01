#pragma once

#include <random>
#include <unordered_map>
#include <random>
#include <iterator>

struct DoubleConfig {
    double init_mean = 0.0;
    double init_stdev = 0.0;
    double min = -1.0;
    double max = 1.0;
    double mutation_rate = 0.01;
    double mutation_power = 0.2;
    double replace_rate = 0.1;
};

static DoubleConfig mutationConfig{};
static std::mt19937 rng_{std::random_device{}()};

inline double clamp(double x) {
    return std::min(mutationConfig.max, std::max(mutationConfig.min, x));
}

inline double newValue() {
    static std::normal_distribution<double> dist(mutationConfig.init_mean, mutationConfig.init_stdev);
    return clamp(dist(rng_));
}

inline double mutationDelta(double value) {
    std::normal_distribution<double> dist(0.0, mutationConfig.mutation_power);
    double delta = dist(rng_);
    return clamp(value + delta);
}