#pragma once

#include <random> // rand()
#include <fstream> // config()
#include <string> // config()
#include <sstream> // config()

static unsigned int prevPlayerId = 0;

struct Position { 
    double x, y; 

    bool operator==(const Position &p) {
        return x == p.x && y == p.y;
    }
    bool operator!=(const Position &p) {
        return x != p.x || y != p.y;
    }
};

// Usage: rand(min, max);
static int rand(const int& min, const int& max) {
    std::mt19937 e{std::random_device{}()};
    std::uniform_int_distribution<> x(min, max);
    return x(e);
}
static double rand(const double& min, const double& max) {
    std::mt19937 e{std::random_device{}()};
    std::uniform_real_distribution<> x(min, max);
    return x(e);
}

template <typename T>
static T config(const std::string &name) {
    std::string line;
    std::ifstream in("Settings.ini");
    while (std::getline(in, line)) {
        std::istringstream sin(line.substr(line.find("=") + 1));

        if (line.find(name) != -1) {
            T value;
            sin >> value;
            return value;
        }
    }
}
