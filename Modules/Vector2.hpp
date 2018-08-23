#pragma once
#include <string> // toString()

struct Vector2 {
    double x, y;

    Vector2() noexcept;
    Vector2(double _x, double _y) noexcept;
    Vector2(const Vector2& other) noexcept;

    double angle() const noexcept;
    double squared() const noexcept;
    double length() const noexcept;
    void normalize() noexcept;
    std::string toString() const noexcept;

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
};