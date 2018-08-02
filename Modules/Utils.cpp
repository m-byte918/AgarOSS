#include "Utils.hpp"
#include <cmath>           // toSize()
#include <random>          // rand()
#include <algorithm>       // splitStr()
#include <cctype>          // splitStr()

namespace utils {

Vector2::Vector2() noexcept : 
    x(0.0), 
    y(0.0) {
}
Vector2::Vector2(double _x, double _y) noexcept : 
    x(_x), 
    y(_y) {
}
Vector2::Vector2(const Vector2& other) noexcept : 
    x(other.x), 
    y(other.y) {
}

double Vector2::squared() const noexcept {
    return x * x + y * y;
}
double Vector2::distance() const noexcept {
    return std::sqrt(squared());
}

void Vector2::operator=(const Vector2 &other) noexcept {
    x = other.x;
    y = other.y;
}
void Vector2::operator=(double val) noexcept {
    x = val;
    y = val;
}
bool Vector2::operator==(const Vector2 &other) const noexcept {
    return x == other.x && y == other.y;
}
bool Vector2::operator!=(const Vector2 &other) const noexcept {
    return x != other.x || y != other.y;
}
const Vector2 &Vector2::operator+=(const Vector2 &other) noexcept {
    x += other.x;
    y += other.y;
    return *this;
}
const Vector2 &Vector2::operator+=(double val) noexcept {
    x += val;
    y += val;
    return *this;
}
const Vector2 &Vector2::operator-=(const Vector2 &other) noexcept {
    x -= other.x;
    y -= other.y;
    return *this;
}
const Vector2 &Vector2::operator-=(double val) noexcept {
    x -= val;
    y -= val;
    return *this;
}
const Vector2 &Vector2::operator*=(const Vector2 &other) noexcept {
    x *= other.x;
    y *= other.y;
    return *this;
}
const Vector2 &Vector2::operator*=(double val) noexcept {
    x *= val;
    y *= val;
    return *this;
}
const Vector2 &Vector2::operator/=(const Vector2 &other) noexcept {
    x /= other.x;
    y /= other.y;
    return *this;
}
const Vector2 &Vector2::operator/=(double val) noexcept {
    x /= val;
    y /= val;
    return *this;
}
Vector2 Vector2::operator*(const Vector2 &other) const noexcept {
    return { x * other.x, y * other.y };
}
Vector2 Vector2::operator*(double val) const noexcept {
    return { x * val, y * val };
}
Vector2 Vector2::operator/(const Vector2 &other) const noexcept {
    return { x / other.x, y / other.y };
}
Vector2 Vector2::operator/(double val) const noexcept {
    return { x / val, y / val };
}
Vector2 Vector2::operator+(const Vector2 &other) const noexcept {
    return { x + other.x, y + other.y };
}
Vector2 Vector2::operator+(double val) const noexcept {
    return { x + val, y + val };
}
Vector2 Vector2::operator-(const Vector2 &other) const noexcept {
    return { x - other.x, y - other.y };
}
Vector2 Vector2::operator-(double val) const noexcept {
    return { x - val, y - val };
}

Color::Color():
    Color(0, 0, 0) {
}
Color::Color(const json &j) {
    *this = j;
}
Color::Color(unsigned char _r, unsigned char _g, unsigned char _b):
    r(_r),
    g(_g),
    b(_b) {
}

void Color::operator=(const json &j) {
    r = j[0];
    g = j[1];
    b = j[2];
}
bool Color::operator==(const Color &other) {
    return r == other.r && g == other.g && b == other.b;
}
bool Color::operator!=(const Color &other) {
    return r != other.r || g != other.g || b != other.b;
}

// Returns a vector of strings from provided string
std::vector<std::string> splitStr(const std::string &str) {
    std::vector<std::string> ret;
    std::string::const_iterator i = str.begin();

    while (i != str.end()) {
        // ignore leading blanks
        i = std::find_if(i, str.end(), [](char c) { return !isspace(c); });
        // find end of next word
        std::string::const_iterator j = std::find_if(i, str.end(), [](char c) { return isspace(c); });
        // copy the characters in [i, j)
        if (i != str.end())
            ret.push_back(std::string(i, j));
        i = j;
    }
    return ret;
}

// nlohmann::json might have deprecated these.
extern bool isull(const std::string &str) {
    char* p;
    strtoull(str.c_str(), &p, 10);
    return *p == 0;
}
extern bool isll(const std::string &str) {
    char* p;
    strtoll(str.c_str(), &p, 10);
    return *p == 0;
}

extern inline double rand(double min, double max) {
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<> distr(min, max);
    return distr(eng);
}

extern inline int rand(int min, int max) {
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(min, max);
    return distr(eng);
}

extern inline Color getRandomColor() noexcept {
    unsigned char RGB[3] = {
        255,
        7,
        (unsigned char)rand(0, 256)
    };
    std::shuffle(&RGB[0], &RGB[3], std::random_device{});
    return { RGB[0], RGB[2], RGB[1] };
}

extern inline Vector2 getRandomPosition() noexcept {
    double halfWidth = config["game"]["mapWidth"].get<double>() * 0.5f;
    double halfHeight = config["game"]["mapHeight"].get<double>() * 0.5f;
    return {
        rand(-halfWidth,  halfWidth),
        rand(-halfHeight, halfHeight)
    };
}

extern inline double toSize(double mass) noexcept {
    return std::sqrt(mass * 100);
}
extern inline double toMass(double size) noexcept {
    return size * size / 100;
}

} // namespace utils