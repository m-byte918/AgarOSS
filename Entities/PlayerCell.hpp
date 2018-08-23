#pragma once
#include "Entity.hpp"

class PlayerCell : public Entity {
public:
    PlayerCell(const Vector2&, double radius, const Color&) noexcept;
    void move() noexcept;
    void split(double angle) noexcept;
    void update(unsigned long long tick) noexcept;
    ~PlayerCell();
};