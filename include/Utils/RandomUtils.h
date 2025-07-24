#pragma once

#include <random>
#include <unordered_map>
#include <iterator>
#include <stdexcept>

// Thread-safe random pair selection from a map
template <typename MapType>
const typename MapType::value_type& getRandomPair(const MapType& map) {
    if (map.empty()) {
        throw std::runtime_error("Map is empty!");
    }

    thread_local static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, map.size() - 1);

    auto it = map.begin();
    std::advance(it, dist(rng));
    return *it;
}

// Hash function for pair<int, int>
struct PairHash {
    size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

// Thread-safe random choice between two values
template <typename T>
T pickRandom(const T& a, const T& b) {
    thread_local static std::mt19937 gen(std::random_device{}());
    std::bernoulli_distribution dist(0.5);  // Recreate each call (cheap)

    return dist(gen) ? a : b;
}

// Thread-safe random double in [0, 1)
inline double randomDouble01() {
    thread_local static std::mt19937 rng(std::random_device{}());
    thread_local static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}
