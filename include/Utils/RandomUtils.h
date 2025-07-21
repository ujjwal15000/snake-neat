#pragma once

#include <random>
#include <unordered_map>
#include <random>
#include <iterator>

template <typename MapType>
const typename MapType::value_type& getRandomPair(const MapType& map) {
    if (map.empty()) {
        throw std::runtime_error("Map is empty!");
    }

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, map.size() - 1);

    auto it = map.begin();
    std::advance(it, dist(rng));
    return *it;
}

struct PairHash {
    size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

inline double randomDouble01() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}