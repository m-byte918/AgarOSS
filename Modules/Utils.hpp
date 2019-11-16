#pragma once
#pragma warning(push, 0)        
#include "json.hpp"           // json, toRadius, splitStr, smart pointers
#pragma warning(pop)
#include <random>             // rand()
#include "Vec2.hpp"           // randomPosition()
#include <condition_variable> // Multithreading

// Stuff I dont feel like typing out
using json = nlohmann::json;
template <class T>
using sptr = std::shared_ptr<T>;
using e_ptr = sptr<class Entity>;

// Constants
#define MATH_PI 3.141592653589793238462643383279502884L
#define INV_SQRT_2 1.0f / std::sqrt(2.0f)

// Multithreading
extern std::condition_variable conVar;

// Externals
extern json config;

// Utilities
namespace utils {

struct Color {
    unsigned char r, g, b;

    Color();
    Color(const json::value_type &j);
    Color(const json &j0, const json &j1, const json &j2);
    Color(unsigned char _r, unsigned char _g, unsigned char _b);

    std::string toString();

    bool operator==(const Color &other);
    bool operator!=(const Color &other);
};

enum CellTypeFlags {
    nothing     = 0x01,
    food        = 0x02,
    viruses     = 0x04,
    ejected     = 0x08,
    mothercells = 0x10,
    playercells = 0x20
};
enum CellStateFlags {
    isSpiked        = 0x01, // Cell has spikes on its outline
    isAgitated      = 0x02, // Cell has waves on its outline
    isRemoved       = 0x04, // Cell was removed from map
    needsUpdate     = 0x08, // Cell needs updating on client side
    ignoreCollision = 0x10  // Whether or not to ignore collision with self
};

extern unsigned char getFlagFrom(const json::value_type &j);

extern std::vector<std::string> splitStr(const std::string &str, char delimiter);

extern bool checkFlagStr(std::string str, const std::string &flagChars);

extern Color randomColor() noexcept;

extern Vec2 randomPosition() noexcept;

extern inline double rand(double min, double max) {
    static std::random_device rd;
    static std::mt19937 eng(rd());
    std::uniform_real_distribution<> distr(min, max);
    return distr(eng);
}
extern inline int rand(int min, int max) {
    static std::random_device rd;
    static std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(min, max);
    return distr(eng);
}

extern inline float toRadius(float mass) noexcept {
    return std::sqrt(mass * 100.0f);
}
extern inline float toMass(float radius) noexcept {
    return radius * radius / 100.0f;
}

} // namespace utils

using namespace utils;