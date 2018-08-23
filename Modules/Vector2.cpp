#include "Vector2.hpp"
#include <sstream> // toString()
#include <cmath>   // angle(), length()

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

double Vector2::angle() const noexcept {
    return std::atan2(y, x);
}
double Vector2::squared() const noexcept {
    return x * x + y * y;
}
double Vector2::length() const noexcept {
    return std::sqrt(squared());
}
void Vector2::normalize() noexcept {
    *this /= length();
}
std::string Vector2::toString() const noexcept {
    std::stringstream ss;
    ss << "{ " << (int)x << ", " << (int)y << " }";
    return ss.str();
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