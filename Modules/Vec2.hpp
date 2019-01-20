#pragma once
#include <string> // toString()

struct Vec2 {
    double x, y;

    Vec2() noexcept;
    Vec2(double _x, double _y) noexcept;
    Vec2(const Vec2& other) noexcept;

    double angle() const noexcept;
    double squared() const noexcept;
    double length() const noexcept;
    Vec2 round() const noexcept;
    Vec2 normalize() noexcept;
    std::string toString() const noexcept;
    double dot(const Vec2 &other) const noexcept;

    void operator=(const Vec2 &other) noexcept;
    void operator=(double val) noexcept;
    bool operator==(const Vec2 &other) const noexcept;
    bool operator!=(const Vec2 &other) const noexcept;

    const Vec2 &operator+=(const Vec2 &other) noexcept;
    const Vec2 &operator+=(double val) noexcept;
    const Vec2 &operator-=(const Vec2 &other) noexcept;
    const Vec2 &operator-=(double val) noexcept;
    const Vec2 &operator*=(const Vec2 &other) noexcept;
    const Vec2 &operator*=(double val) noexcept;
    const Vec2 &operator/=(const Vec2 &other) noexcept;
    const Vec2 &operator/=(double val) noexcept;

    Vec2 operator*(const Vec2 &other) const noexcept;
    Vec2 operator*(double val) const noexcept;
    Vec2 operator/(const Vec2 &other) const noexcept;
    Vec2 operator/(double val) const noexcept;
    Vec2 operator+(const Vec2 &other) const noexcept;
    Vec2 operator+(double val) const noexcept;
    Vec2 operator-(const Vec2 &other) const noexcept;
    Vec2 operator-(double val) const noexcept;
};