#pragma once
#include <vector>   // splitStr
#include "json.hpp" // json

// Settings
using json = nlohmann::json;
extern json config;

// Utilities
namespace utils {

struct Vector2 {
    double x, y;

    Vector2() noexcept;
    Vector2(double _x, double _y) noexcept;
    Vector2(const Vector2& other) noexcept;

    double squared() const noexcept;
    double distance() const noexcept;
    void operator=(const Vector2 &other) noexcept;
    void operator=(double val) noexcept;
    bool operator==(const Vector2 &other) const noexcept;
    bool operator!=(const Vector2 &other) const noexcept;
    const Vector2 &operator+=(const Vector2 &other) noexcept;
    const Vector2 &operator+=(double val) noexcept;
    const Vector2 &operator-=(const Vector2 &other) noexcept;
    const Vector2 &operator-=(double val) noexcept;
    const Vector2 &operator*=(const Vector2 &other) noexcept;
    const Vector2 &operator*=(double val) noexcept;
    const Vector2 &operator/=(const Vector2 &other) noexcept;
    const Vector2 &operator/=(double val) noexcept;
    Vector2 operator*(const Vector2 &other) const noexcept;
    Vector2 operator*(double val) const noexcept;
    Vector2 operator/(const Vector2 &other) const noexcept;
    Vector2 operator/(double val) const noexcept;
    Vector2 operator+(const Vector2 &other) const noexcept;
    Vector2 operator+(double val) const noexcept;
    Vector2 operator-(const Vector2 &other) const noexcept;
    Vector2 operator-(double val) const noexcept;

    friend std::ostream &operator<<(std::ostream &os, const Vector2 &other) {
        os << "{ " << other.x << ", " << other.y << " }";
        return os;
    }
    friend std::string operator+(std::string &str, const Vector2 &other) {
        return str + "{ " + std::to_string((int)other.x) + ", " + std::to_string((int)other.y) + " }";
    }
    friend std::string operator+(const char *str, const Vector2 &other) {
        return std::string(str) + other;
    }
};
struct Color {
    unsigned char r, g, b;

    Color();
    Color(const json &j);
    Color(unsigned char _r, unsigned char _g, unsigned char _b);

    void operator=(const json &j);
    bool operator==(const Color &other);
    bool operator!=(const Color &other);
};
// canEat, canBeEatenBy, cannotSpawnNear
enum CellFlags {
    nothing = 0x00,
    food = 0x01,
    viruses = 0x02,
    ejected = 0x04,
    mothercells = 0x08,
    playercells = 0x10
};

extern std::vector<std::string> splitStr(const std::string &str, char delimiter);

extern inline double rand(double min, double max);

extern inline int rand(int min, int max);

extern inline Color randomColor() noexcept;

extern inline Vector2 randomPosition() noexcept;

extern inline double toRadius(double mass) noexcept;
extern inline double toMass(double radius) noexcept;

} // namespace utils

using namespace utils;