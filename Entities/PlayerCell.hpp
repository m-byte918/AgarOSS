#pragma once
#include "Entity.hpp"

class PlayerCell : public Entity {
public:
    PlayerCell(const Vec2&, double radius, const Color&) noexcept;
    void move() noexcept;
    void autoSplit() noexcept;
    void pop() noexcept;
    void split(double angle, double radius) noexcept;
    void update(unsigned long long tick) noexcept;
    void consume(e_ptr &_prey) noexcept;
    void onDespawned() const noexcept;
    ~PlayerCell();
};