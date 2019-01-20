#include "Vec2.hpp"
#include <sstream> // toString()
#include <cmath>   // angle(), length()

Vec2::Vec2() noexcept :
    x(0.0),
    y(0.0) {
}
Vec2::Vec2(double _x, double _y) noexcept :
    x(_x),
    y(_y) {
}
Vec2::Vec2(const Vec2& other) noexcept :
    x(other.x),
    y(other.y) {
}

double Vec2::angle() const noexcept {
    return std::atan2(y, x);
}
double Vec2::squared() const noexcept {
    return x * x + y * y;
}
double Vec2::length() const noexcept {
    return std::sqrt(squared());
}
Vec2 Vec2::round() const noexcept {
    return Vec2((int)x, (int)y);
}
Vec2 Vec2::normalize() noexcept {
    return *this /= length();
}
std::string Vec2::toString() const noexcept {
    std::stringstream ss;
    ss << "{ " << (int)x << ", " << (int)y << " }";
    return ss.str();
}
double Vec2::dot(const Vec2 &other) const noexcept {
    return x * other.x + y * other.y;
}

void Vec2::operator=(const Vec2 &other) noexcept {
    x = other.x;
    y = other.y;
}
void Vec2::operator=(double val) noexcept {
    x = val;
    y = val;
}
bool Vec2::operator==(const Vec2 &other) const noexcept {
    return x == other.x && y == other.y;
}
bool Vec2::operator!=(const Vec2 &other) const noexcept {
    return x != other.x || y != other.y;
}

const Vec2 &Vec2::operator+=(const Vec2 &other) noexcept {
    x += other.x;
    y += other.y;
    return *this;
}
const Vec2 &Vec2::operator+=(double val) noexcept {
    x += val;
    y += val;
    return *this;
}
const Vec2 &Vec2::operator-=(const Vec2 &other) noexcept {
    x -= other.x;
    y -= other.y;
    return *this;
}
const Vec2 &Vec2::operator-=(double val) noexcept {
    x -= val;
    y -= val;
    return *this;
}
const Vec2 &Vec2::operator*=(const Vec2 &other) noexcept {
    x *= other.x;
    y *= other.y;
    return *this;
}
const Vec2 &Vec2::operator*=(double val) noexcept {
    x *= val;
    y *= val;
    return *this;
}
const Vec2 &Vec2::operator/=(const Vec2 &other) noexcept {
    x /= other.x;
    y /= other.y;
    return *this;
}
const Vec2 &Vec2::operator/=(double val) noexcept {
    x /= val;
    y /= val;
    return *this;
}

Vec2 Vec2::operator*(const Vec2 &other) const noexcept {
    return { x * other.x, y * other.y };
}
Vec2 Vec2::operator*(double val) const noexcept {
    return { x * val, y * val };
}
Vec2 Vec2::operator/(const Vec2 &other) const noexcept {
    return { x / other.x, y / other.y };
}
Vec2 Vec2::operator/(double val) const noexcept {
    return { x / val, y / val };
}
Vec2 Vec2::operator+(const Vec2 &other) const noexcept {
    return { x + other.x, y + other.y };
}
Vec2 Vec2::operator+(double val) const noexcept {
    return { x + val, y + val };
}
Vec2 Vec2::operator-(const Vec2 &other) const noexcept {
    return { x - other.x, y - other.y };
}
Vec2 Vec2::operator-(double val) const noexcept {
    return { x - val, y - val };
}